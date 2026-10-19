class ADD_Publisher
{
    protected static ref ADD_Publisher s_Instance;
    protected ref array<string> m_Events;
    protected int m_LastFlush;
    protected int m_Size;

    // The manager rejects bodies over 1 MiB; flushing at a quarter keeps every batch accepted.
    static const int MAX_EVENTS = 200;
    static const int MAX_CHARS = 256000;

    static ADD_Publisher Get()
    {
        if (!s_Instance)
            s_Instance = new ADD_Publisher;
        return s_Instance;
    }

    void ADD_Publisher()
    {
        m_Events = new array<string>;
    }

    void Publish(string type, string dataJson, bool urgent)
    {
        string ev = "{" + ADD_Json.JKey("id") + ADD_Json.Quote(ADD_Uuid.New()) + "," + ADD_Json.JKey("type") + ADD_Json.Quote(type) + "," + ADD_Json.JKey("ts") + ADD_Clock.UnixMs().ToString() + "," + ADD_Json.JKey("data") + dataJson + "}";
        if (m_Size + ev.Length() > MAX_CHARS)
            Flush();
        m_Events.Insert(ev);
        m_Size = m_Size + ev.Length() + 1;
        if (urgent || m_Events.Count() >= MAX_EVENTS)
            Flush();
    }

    void Tick()
    {
        if (m_Events.Count() == 0)
            return;
        int every = 1500;
        ADD_ModConfigData data = ADD_Config.Get().Data;
        if (data && data.journal)
            every = data.journal.flushMs;
        if (GetGame().GetTime() - m_LastFlush >= every)
            Flush();
    }

    void Flush()
    {
        if (m_Events.Count() == 0)
            return;
        ADD_Bootstrap boot = ADD_Config.Get().Bootstrap;
        string events = "";
        for (int i = 0; i < m_Events.Count(); i++)
        {
            if (i > 0)
                events = events + ",";
            events = events + m_Events.Get(i);
        }
        m_Events.Clear();
        m_Size = 0;
        m_LastFlush = GetGame().GetTime();
        string body = "{" + ADD_Json.JKey("protocol") + "2";
        body = body + "," + ADD_Json.JKey("batchId") + ADD_Json.Quote(ADD_Uuid.New());
        body = body + "," + ADD_Json.JKey("instanceId") + ADD_Json.Quote(boot.instanceId);
        body = body + "," + ADD_Json.JKey("sentAt") + ADD_Clock.UnixMs().ToString();
        body = body + "," + ADD_Json.JKey("modVersion") + ADD_Json.Quote(ADD_ModInfo2.VERSION);
        body = body + "," + ADD_Json.JKey("events") + "[" + events + "]}";
        ADD_Http.Get().Enqueue(body, false);
    }
}
