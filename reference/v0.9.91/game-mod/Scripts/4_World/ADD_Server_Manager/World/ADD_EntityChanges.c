// Vehicles, boats, bases, flags and tracked items are sent only when they change:
// first sight (state), moved or health changed (changed), destroyed, and removed from the world.
class ADD_EntityChanges
{
    protected static ref ADD_EntityChanges s_Instance;
    protected ref array<EntityAI> m_Touched;
    protected int m_Cursor;
    protected int m_NextAt;
    protected bool m_Running;

    static const int BUDGET = 200;

    static ADD_EntityChanges Get()
    {
        if (!s_Instance)
            s_Instance = new ADD_EntityChanges;
        return s_Instance;
    }

    void ADD_EntityChanges()
    {
        m_Touched = new array<EntityAI>;
    }

    static string Stream(string kind)
    {
        switch (kind)
        {
            case "transport": return "carMove";
            case "boat": return "boatMove";
            case "base": return "baseChange";
            case "flag": return "baseChange";
            case "item": return "trackedItems";
        }
        return "";
    }

    // Touch asks for the entity to be checked on the next step instead of waiting for the scan.
    void Touch(EntityAI entity)
    {
        if (entity && m_Touched.Find(entity) < 0)
            m_Touched.Insert(entity);
    }

    void Changed(EntityAI entity, string action)
    {
        ADD_Tracked tracked = ADD_EntityRegistry.Get().Tracked(entity);
        if (!tracked)
            return;
        Emit(tracked, action);
    }

    void Removed(ADD_Tracked tracked)
    {
        if (ADD_Journal.ShuttingDown())
            return;
        Emit(tracked, "deleted");
    }

    void Step()
    {
        if (ADD_Journal.Loading())
            return;
        for (int t = 0; t < m_Touched.Count(); t++)
        {
            ADD_Tracked touched = ADD_EntityRegistry.Get().Tracked(m_Touched.Get(t));
            if (touched)
                Check(touched);
        }
        m_Touched.Clear();
        int now = GetGame().GetTime();
        if (!m_Running)
        {
            if (now < m_NextAt)
                return;
            m_Running = true;
            m_Cursor = 0;
        }
        array<ref ADD_Tracked> items = ADD_EntityRegistry.Get().Items();
        int stop = m_Cursor + BUDGET;
        while (m_Cursor < items.Count() && m_Cursor < stop)
        {
            ADD_Tracked tracked = items.Get(m_Cursor);
            m_Cursor++;
            if (tracked && tracked.entity && Stream(tracked.kind) != "")
                Check(tracked);
        }
        if (m_Cursor >= items.Count())
        {
            m_Running = false;
            m_NextAt = now + ADD_Config.Get().Data.snapshots.changeScanSec * 1000;
        }
    }

    protected void Check(ADD_Tracked tracked)
    {
        EntityAI entity = tracked.entity;
        if (!entity)
            return;
        vector pos = entity.GetPosition();
        float health = entity.GetHealth("", "");
        if (!tracked.reported)
        {
            if (Emit(tracked, "state"))
            {
                tracked.reported = true;
                tracked.lastPos = pos;
                tracked.lastHealth = health;
            }
            return;
        }
        bool moved = vector.Distance(pos, tracked.lastPos) > ADD_Config.Get().Data.journal.moveMeters;
        bool hurt = Math.AbsFloat(health - tracked.lastHealth) > 0.01;
        if (!moved && !hurt)
            return;
        if (Emit(tracked, "changed"))
        {
            tracked.lastPos = pos;
            tracked.lastHealth = health;
        }
    }

    protected bool Emit(ADD_Tracked tracked, string action)
    {
        string stream = Stream(tracked.kind);
        if (stream == "" || !ADD_Config.Get().On(stream) || !tracked.entity)
            return false;
        string data = "{" + ADD_Json.JKey("entity") + ADD_EntityRegistry.Row(tracked.kind, tracked.entity);
        data = data + "," + ADD_Json.JKey("action") + ADD_Json.Quote(action) + "}";
        ADD_Publisher.Get().Publish("entity.change", data, false);
        return true;
    }
}
