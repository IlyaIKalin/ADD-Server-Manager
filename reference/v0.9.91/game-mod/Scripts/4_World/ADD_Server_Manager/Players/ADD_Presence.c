class ADD_Presence
{
    protected static ref ADD_Presence s_Instance;
    protected ref map<string, float> m_Distance;

    static ADD_Presence Get()
    {
        if (!s_Instance)
            s_Instance = new ADD_Presence;
        return s_Instance;
    }

    void ADD_Presence()
    {
        m_Distance = new map<string, float>;
    }

    void Start()
    {
        int sec = ADD_Config.Get().Data.intervalsSec.online;
        if (sec < 5)
            sec = 30;
        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(this.Publish, sec * 1000, true);
    }

    void NoteMove(string steamId, float meters)
    {
        float cur;
        m_Distance.Find(steamId, cur);
        m_Distance.Set(steamId, cur + meters);
    }

    void Publish()
    {
        array<Man> players = new array<Man>;
        GetGame().GetPlayers(players);
        string items = "";
        int n = players.Count();
        if (n > 120)
            n = 120;
        for (int i = 0; i < n; i++)
        {
            PlayerBase player = PlayerBase.Cast(players.Get(i));
            if (!player || !player.GetIdentity())
                continue;
            if (items != "")
                items = items + ",";
            string steam = player.GetIdentity().GetPlainId();
            float walked;
            m_Distance.Find(steam, walked);
            vector pos = player.GetPosition();
            items = items + "{" + ADD_Json.JKey("steamId") + ADD_Json.Quote(steam) + "," + ADD_Json.JKey("name") + ADD_Json.Quote(player.GetIdentity().GetName());
            items = items + "," + ADD_Json.JKey("pos") + ADD_Json.Vec(pos) + "," + ADD_Json.JKey("dir") + player.GetDirection()[0].ToString();
            items = items + "," + ADD_Json.JKey("health") + player.GetHealth("", "").ToString() + "," + ADD_Json.JKey("blood") + player.GetHealth("GlobalHealth", "Blood").ToString();
            items = items + "," + ADD_Json.JKey("shock") + player.GetHealth("GlobalHealth", "Shock").ToString();
            items = items + "," + ADD_Json.JKey("distanceM") + walked.ToString() + "," + ADD_Json.JKey("alive") + Alive(player) + "}";

        }
        if (ADD_Config.Get().On("online"))
            ADD_Publisher.Get().Publish("players.online", "{" + ADD_Json.JKey("players") + "[" + items + "]}", false);
    }

    protected string Alive(PlayerBase player)
    {
        if (player.IsAlive())
            return "true";
        return "false";
    }
}
