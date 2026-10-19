modded class MissionServer
{
    void MissionServer()
    {
        if (GetGame() && GetGame().IsDedicatedServer())
            GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(this.ADD_Start, 2000, false);
    }

    override void OnUpdate(float timeslice)
    {
        super.OnUpdate(timeslice);
        if (GetGame() && GetGame().IsServer())
            ADD_Ticker.OnUpdate(timeslice);
    }

    override void OnMissionFinish()
    {
        if (GetGame() && GetGame().IsServer())
        {
            ADD_Journal.SetShuttingDown();
            ADD_Publisher.Get().Flush();
            ADD_Http.Get().SpoolAll();
        }
        super.OnMissionFinish();
    }

    void ADD_Start()
    {
        ADD_Log.Info(ADD_ModInfo2.NAME + " " + ADD_ModInfo2.VERSION + " protocol " + ADD_ModInfo2.PROTOCOL.ToString());
        ADD_Config.Get().LoadBootstrap();
        ADD_Http.Get().FetchConfig();
        ADD_Heartbeat.Get().Start();
        ADD_Presence.Get().Start();
        ADD_Positions.Get().Start();
        ADD_CommandPoll.Get().Start();
    }

    override void OnEvent(EventType eventTypeId, Param params)
    {
        if (eventTypeId == ChatMessageEventTypeID && GetGame() && GetGame().IsServer())
        {
            ChatMessageEventParams chat = ChatMessageEventParams.Cast(params);
            if (chat && chat.param3 != "")
            {
                array<Man> players = new array<Man>;
                GetGame().GetPlayers(players);
                for (int i = 0; i < players.Count(); i++)
                {
                    PlayerBase player = PlayerBase.Cast(players.Get(i));
                    if (player && player.GetIdentity() && player.GetIdentity().GetName() == chat.param2)
                    {
                        ADD_Chat.OnChat(player, chat.param1, chat.param3);
                        break;
                    }
                }
            }
        }
        super.OnEvent(eventTypeId, params);
    }

    override void InvokeOnConnect(PlayerBase player, PlayerIdentity identity)
    {
        if (GetGame() && GetGame().IsServer() && player)
            player.ADD_HoldGear(6000);
        super.InvokeOnConnect(player, identity);
        if (GetGame() && GetGame().IsServer())
            ADD_Sessions.OnConnected(player, identity);
    }

    override void PlayerDisconnected(PlayerBase player, PlayerIdentity identity, string uid)
    {
        if (GetGame() && GetGame().IsServer())
            ADD_Sessions.OnDisconnected(identity, "disconnect");
        super.PlayerDisconnected(player, identity, uid);
    }
}
