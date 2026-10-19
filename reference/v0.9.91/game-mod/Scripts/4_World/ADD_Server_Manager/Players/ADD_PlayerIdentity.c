class ADD_PlayerIdentity
{
    static string SteamID(PlayerIdentity identity)
    {
        if (!identity)
            return "";
        return identity.GetPlainId();
    }

    static string BisID(PlayerIdentity identity)
    {
        if (!identity)
            return "";
        return identity.GetId();
    }

    static string Name(PlayerIdentity identity)
    {
        if (!identity)
            return "";
        return identity.GetName();
    }

    static PlayerBase Find(string steamId)
    {
        array<Man> players = new array<Man>;
        GetGame().GetPlayers(players);
        for (int i = 0; i < players.Count(); i++)
        {
            PlayerBase player = PlayerBase.Cast(players.Get(i));
            if (!player || !player.GetIdentity())
                continue;
            if (player.GetIdentity().GetPlainId() == steamId)
                return player;
        }
        return null;
    }
}
