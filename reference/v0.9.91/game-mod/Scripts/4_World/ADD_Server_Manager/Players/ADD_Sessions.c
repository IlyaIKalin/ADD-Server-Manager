class ADD_Sessions
{
    static void OnConnected(PlayerBase player, PlayerIdentity identity)
    {
        if (!identity)
            return;
        ADD_Journal.Remember(player, "join");
        if (!ADD_Config.Get().On("session"))
            return;
        string data = "{" + ADD_Json.JKey("steamId") + ADD_Json.Quote(ADD_PlayerIdentity.SteamID(identity));
        data = data + "," + ADD_Json.JKey("name") + ADD_Json.Quote(ADD_PlayerIdentity.Name(identity));
        data = data + "," + ADD_Json.JKey("bisId") + ADD_Json.Quote(ADD_PlayerIdentity.BisID(identity));
        data = data + "," + ADD_Json.JKey("items") + ADD_Journal.Inventory(player) + "}";
        ADD_Publisher.Get().Publish("session.start", data, false);
    }

    static void OnDisconnected(PlayerIdentity identity, string reason)
    {
        if (!identity || !ADD_Config.Get().On("session"))
            return;
        string data = "{" + ADD_Json.JKey("steamId") + ADD_Json.Quote(ADD_PlayerIdentity.SteamID(identity));
        data = data + "," + ADD_Json.JKey("name") + ADD_Json.Quote(ADD_PlayerIdentity.Name(identity));
        data = data + "," + ADD_Json.JKey("reason") + ADD_Json.Quote(reason) + "}";
        ADD_Publisher.Get().Publish("session.end", data, false);
    }
}
