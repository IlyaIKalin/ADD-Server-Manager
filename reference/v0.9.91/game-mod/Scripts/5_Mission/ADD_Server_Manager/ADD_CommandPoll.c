class ADD_CommandPoll
{
    protected static ref ADD_CommandPoll s_Instance;
    protected int m_Delay;
    protected string m_LastError;
    protected int m_Polls;
    protected int m_Received;
    protected int m_StatsAt;

    static ADD_CommandPoll Get()
    {
        if (!s_Instance)
            s_Instance = new ADD_CommandPoll;
        return s_Instance;
    }

    void ADD_CommandPoll()
    {
        m_Delay = 5000;
    }

    void Start()
    {
        ADD_Log.Info("command poll started");
        m_StatsAt = GetGame().GetTime() + 60000;
        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(this.Poll, m_Delay, false);
    }

    void Poll()
    {
        m_Polls++;
        string body = ADD_Http.Get().GetNow("commands");
        int ran = 0;
        if (body && body != "")
            ran = Dispatch(body);
        if (ran > 0)
            m_Delay = 2000;
        else if (m_Delay < 10000)
            m_Delay = m_Delay + 1000;
        if (GetGame().GetTime() >= m_StatsAt)
        {
            m_StatsAt = GetGame().GetTime() + 60000;
            string state = "ok";
            if (m_LastError != "")
                state = m_LastError;
            ADD_Log.Info("stats: polls " + m_Polls.ToString() + ", commands " + m_Received.ToString() + ", poll state " + state + "; " + ADD_Http.Get().Stats());
        }
        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(this.Poll, m_Delay, false);
    }

    protected int Dispatch(string body)
    {
        string code = ADD_Json.ErrorCode(body);
        if (code != m_LastError)
        {
            if (code != "")
                ADD_Log.Warn("commands rejected: " + code);
            else
                ADD_Log.Info("commands poll works again after: " + m_LastError);
            m_LastError = code;
        }
        if (code != "")
            return 0;
        string q = ADD_Json.DQuote();
        body.Replace(q + "class" + q + ":", q + "className" + q + ":");
        body.Replace(q + "value" + q + ":" + q, q + "text" + q + ":" + q);

        string error;
        JsonSerializer serializer = new JsonSerializer;
        ADD_CommandsBody parsed = new ADD_CommandsBody;
        if (!serializer.ReadFromString(parsed, body, error) || !parsed.commands)
        {
            int cut = body.Length();
            if (cut > 300)
                cut = 300;
            ADD_Log.Warn("commands: cannot parse response: " + error + " body: " + body.Substring(0, cut));
            return 0;
        }
        if (parsed.commands.Count() > 0)
            ADD_Log.Info("commands: received " + parsed.commands.Count().ToString());
        m_Received += parsed.commands.Count();
        for (int i = 0; i < parsed.commands.Count(); i++)
            ADD_CommandExecutor.Run(parsed.commands.Get(i));
        return parsed.commands.Count();
    }
}
