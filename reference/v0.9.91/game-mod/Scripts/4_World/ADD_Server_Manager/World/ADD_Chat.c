class ADD_Chat
{
    static void OnChat(PlayerBase player, int channel, string text)
    {
        if (!ADD_Config.Get().On("chat") || !player || !player.GetIdentity())
            return;
        if (text.Length() > 512)
            text = text.Substring(0, 512);
        string data = "{" + ADD_Json.JKey("steamId") + ADD_Json.Quote(player.GetIdentity().GetPlainId());
        data = data + "," + ADD_Json.JKey("name") + ADD_Json.Quote(player.GetIdentity().GetName());
        data = data + "," + ADD_Json.JKey("channel") + ADD_Json.Quote(channel.ToString());
        data = data + "," + ADD_Json.JKey("text") + ADD_Json.Quote(text) + "}";
        ADD_Publisher.Get().Publish("chat", data, false);
    }
}
