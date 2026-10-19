// Full snapshots of moving or short-lived kinds, sent every snapshots.intervalSec.
// The same snapshot feeds the live map and the journal.
class ADD_EntitySnapshots
{
    protected static ref array<string> s_Kinds;
    protected static int s_KindIndex;
    protected static int s_Offset;
    protected static int s_NextAt;
    protected static bool s_Force;
    protected static bool s_Running;

    static void Request()
    {
        s_Force = true;
    }

    static array<string> Kinds()
    {
        if (!s_Kinds)
        {
            s_Kinds = new array<string>;
            s_Kinds.Insert("zombie");
            s_Kinds.Insert("animal");
            s_Kinds.Insert("gas");
            s_Kinds.Insert("crash");
            s_Kinds.Insert("corpse");
        }
        return s_Kinds;
    }

    static void Step()
    {
        int now = GetGame().GetTime();
        if (!s_Running)
        {
            if (!s_Force && now < s_NextAt)
                return;
            s_Force = false;
            s_Running = true;
            s_KindIndex = 0;
            s_Offset = 0;
        }
        array<string> kinds = Kinds();
        while (s_KindIndex < kinds.Count() && !ADD_Config.Get().On(kinds.Get(s_KindIndex)))
            s_KindIndex++;
        if (s_KindIndex >= kinds.Count())
        {
            s_Running = false;
            s_NextAt = now + ADD_Config.Get().Data.snapshots.intervalSec * 1000;
            return;
        }
        EmitChunk(kinds.Get(s_KindIndex));
    }

    protected static void EmitChunk(string kind)
    {
        if (kind == "corpse")
            DropLivingCorpses();
        array<ref ADD_Tracked> items = ADD_EntityRegistry.Get().Items();
        int total = 0;
        for (int n = 0; n < items.Count(); n++)
        {
            ADD_Tracked counted = items.Get(n);
            if (counted && counted.entity && counted.kind == kind)
                total++;
        }
        int chunk = 40;
        int parts = 1;
        if (total > 0)
            parts = (total + chunk - 1) / chunk;
        int part = s_Offset / chunk;
        int seen = 0;
        int emitted = 0;
        string body = "";
        for (int i = 0; i < items.Count(); i++)
        {
            ADD_Tracked tracked = items.Get(i);
            if (!tracked || !tracked.entity || tracked.kind != kind)
                continue;
            if (seen < s_Offset)
            {
                seen++;
                continue;
            }
            if (emitted >= chunk)
                break;
            if (body != "")
                body = body + ",";
            body = body + ADD_EntityRegistry.Row(kind, tracked.entity);
            emitted++;
        }
        string payload = "{" + ADD_Json.JKey("kind") + ADD_Json.Quote(kind);
        payload = payload + "," + ADD_Json.JKey("part") + part.ToString();
        payload = payload + "," + ADD_Json.JKey("parts") + parts.ToString();
        payload = payload + "," + ADD_Json.JKey("items") + "[" + body + "]}";
        ADD_Publisher.Get().Publish("entities.snapshot", payload, false);
        if (emitted == 0 || s_Offset + emitted >= total)
        {
            s_KindIndex++;
            s_Offset = 0;
        }
        else
            s_Offset = s_Offset + emitted;
    }

    // A hit must not leave a living player on the corpse layer.
    protected static void DropLivingCorpses()
    {
        array<ref ADD_Tracked> items = ADD_EntityRegistry.Get().Items();
        for (int i = items.Count() - 1; i >= 0; i--)
        {
            ADD_Tracked tracked = items.Get(i);
            if (!tracked || tracked.kind != "corpse" || !tracked.entity)
                continue;
            PlayerBase body = PlayerBase.Cast(tracked.entity);
            if (body && body.IsAlive())
                ADD_EntityRegistry.Get().Remove("corpse", body);
        }
    }
}
