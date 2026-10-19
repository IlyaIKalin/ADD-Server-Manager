class ADD_Tracked
{
    string kind;
    EntityAI entity;
    vector lastPos;
    float lastHealth;
    bool reported;
    string label;

    void ADD_Tracked(string entityKind, EntityAI ent)
    {
        kind = entityKind;
        entity = ent;
        lastHealth = -1;
    }
}

// Every kind is registered so commands can find any entity; streams only decide what is sent.
class ADD_EntityRegistry
{
    protected static ref ADD_EntityRegistry s_Instance;
    protected ref array<ref ADD_Tracked> m_Items;

    static ADD_EntityRegistry Get()
    {
        if (!s_Instance)
            s_Instance = new ADD_EntityRegistry;
        return s_Instance;
    }

    void ADD_EntityRegistry()
    {
        m_Items = new array<ref ADD_Tracked>;
    }

    void Upsert(string kind, EntityAI entity)
    {
        if (!entity)
            return;
        if (Tracked(entity))
            return;
        m_Items.Insert(new ADD_Tracked(kind, entity));
    }

    void Remove(string kind, EntityAI entity)
    {
        for (int i = m_Items.Count() - 1; i >= 0; i--)
        {
            ADD_Tracked tracked = m_Items.Get(i);
            if (tracked.entity == entity)
            {
                if (tracked.reported)
                    ADD_EntityChanges.Get().Removed(tracked);
                m_Items.Remove(i);
            }
        }
    }

    void Note(EntityAI entity, string text)
    {
        ADD_Tracked tracked = Tracked(entity);
        if (tracked && text != "")
            tracked.label = text;
    }

    ADD_Tracked Tracked(EntityAI entity)
    {
        for (int i = 0; i < m_Items.Count(); i++)
        {
            if (m_Items.Get(i).entity == entity)
                return m_Items.Get(i);
        }
        return null;
    }

    array<ref ADD_Tracked> Items()
    {
        return m_Items;
    }

    EntityAI Find(string id)
    {
        if (id == "")
            return null;
        for (int i = 0; i < m_Items.Count(); i++)
        {
            EntityAI entity = m_Items.Get(i).entity;
            if (entity && NetworkID(entity) == id)
                return entity;
        }
        return null;
    }

    string CountsJson()
    {
        ref map<string, int> counts = new map<string, int>;
        for (int i = 0; i < m_Items.Count(); i++)
        {
            string kind = m_Items.Get(i).kind;
            int n;
            counts.Find(kind, n);
            counts.Set(kind, n + 1);
        }
        string json = "{";
        bool first = true;
        for (int k = 0; k < counts.Count(); k++)
        {
            if (!first)
                json = json + ",";
            first = false;
            json = json + ADD_Json.Quote(counts.GetKey(k)) + ":" + counts.GetElement(k).ToString();
        }
        return json + "}";
    }

    static string NetworkID(EntityAI entity)
    {
        int low, high;
        entity.GetNetworkID(low, high);
        return low.ToString() + ":" + high.ToString();
    }

    static string Row(string kind, EntityAI entity)
    {
        string row = "{" + ADD_Json.JKey("id") + ADD_Json.Quote(NetworkID(entity));
        row = row + "," + ADD_Json.JKey("kind") + ADD_Json.Quote(kind);
        string cls = entity.GetType();
        ADD_Tracked tracked = Get().Tracked(entity);
        if (tracked && tracked.label != "")
            cls = tracked.label;
        row = row + "," + ADD_Json.JKey("class") + ADD_Json.Quote(cls);
        string pid = ADD_Journal.Pid(entity);
        if (pid != "")
            row = row + "," + ADD_Json.JKey("pid") + ADD_Json.Quote(pid);
        row = row + "," + ADD_Json.JKey("pos") + ADD_Json.Vec(entity.GetPosition());
        float health = 0;
        float maxHealth = 0;
        if (kind != "crash" && kind != "gas")
        {
            health = entity.GetHealth("", "");
            maxHealth = entity.GetMaxHealth("", "");
        }
        row = row + "," + ADD_Json.JKey("health") + ADD_Json.Num(health);
        row = row + "," + ADD_Json.JKey("maxHealth") + ADD_Json.Num(maxHealth);
        if (kind == "animal")
        {
            float maxBlood = entity.GetMaxHealth("", "Blood");
            float blood = entity.GetHealth("", "Blood");
            if (maxBlood <= 0)
            {
                maxBlood = entity.GetMaxHealth("GlobalHealth", "Blood");
                blood = entity.GetHealth("GlobalHealth", "Blood");
            }
            row = row + "," + ADD_Json.JKey("blood") + ADD_Json.Num(blood);
            row = row + "," + ADD_Json.JKey("maxBlood") + ADD_Json.Num(maxBlood);
        }
        return row + "}";
    }
}
