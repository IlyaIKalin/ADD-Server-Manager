class ADD_Positions
{
    protected static ref ADD_Positions s_Instance;
    protected ref map<string, vector> m_Last;

    static ADD_Positions Get()
    {
        if (!s_Instance)
            s_Instance = new ADD_Positions;
        return s_Instance;
    }

    void ADD_Positions()
    {
        m_Last = new map<string, vector>;
    }

    void Start()
    {
        int sec = ADD_Config.Get().Data.intervalsSec.positions;
        if (sec < 1)
            sec = 5;
        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(this.Sample, sec * 1000, true);
    }

    void Sample()
    {
        array<Man> players = new array<Man>;
        GetGame().GetPlayers(players);
        string points = "";
        for (int i = 0; i < players.Count(); i++)
        {
            PlayerBase player = PlayerBase.Cast(players.Get(i));
            if (!player || !player.GetIdentity())
                continue;
            string steam = player.GetIdentity().GetPlainId();
            vector pos = player.GetPosition();
            vector prev;
            if (m_Last.Find(steam, prev))
            {
                float moved = vector.Distance(prev, pos);
                float minMove = ADD_Config.Get().Data.positions.minMoveMeters;
                if (minMove < 0)
                    minMove = 2;
                if (moved < minMove)
                    continue;
                ADD_Presence.Get().NoteMove(steam, moved);
            }
            m_Last.Set(steam, pos);
            if (points != "")
                points = points + ",";
            points = points + "{" + ADD_Json.JKey("steamId") + ADD_Json.Quote(steam) + "," + ADD_Json.JKey("pos") + ADD_Json.Vec(pos) + "," + ADD_Json.JKey("dir") + "0}";
        }
        if (points != "" && ADD_Config.Get().On("positions"))
            ADD_Publisher.Get().Publish("positions", "{" + ADD_Json.JKey("points") + "[" + points + "]}", false);
    }
}
