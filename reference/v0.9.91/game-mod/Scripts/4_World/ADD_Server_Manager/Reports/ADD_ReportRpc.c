class ADD_ReportRpc
{
    static const int RPC_REPORT = 778210;
    protected static ref map<string, int> s_Cooldown;

    static void Ensure()
    {
        if (!s_Cooldown)
            s_Cooldown = new map<string, int>;
    }

    static bool Accept(PlayerIdentity sender, string category, string message, string targetSteam)
    {
        Ensure();
        ADD_ReportsCfg cfg = ADD_Config.Get().Data.reports;
        if (!cfg.enabled || !sender)
            return false;
        if (category != "cheat" && category != "harassment" && category != "bug" && category != "other")
            return false;
        if (!message || message.Length() < 3 || message.Length() > cfg.maxMessageLen)
            return false;
        string steam = sender.GetPlainId();
        int last;
        s_Cooldown.Find(steam, last);
        if (GetGame().GetTime() - last < cfg.cooldownSec * 1000)
            return false;
        s_Cooldown.Set(steam, GetGame().GetTime());
        PlayerBase reporter = ADD_PlayerIdentity.Find(steam);
        vector pos = "0 0 0";
        if (reporter)
            pos = reporter.GetPosition();
        string data = "{" + ADD_Json.JKey("reporter") + "{" + ADD_Json.JKey("steamId") + ADD_Json.Quote(steam) + "," + ADD_Json.JKey("name") + ADD_Json.Quote(sender.GetName()) + "}";
        if (targetSteam != "")
        {
            PlayerBase target = ADD_PlayerIdentity.Find(targetSteam);
            if (target && reporter && vector.Distance(reporter.GetPosition(), target.GetPosition()) > cfg.targetRadiusM)
                return false;
            string targetName = "";
            if (target && target.GetIdentity())
                targetName = target.GetIdentity().GetName();
            data = data + "," + ADD_Json.JKey("target") + "{" + ADD_Json.JKey("steamId") + ADD_Json.Quote(targetSteam) + "," + ADD_Json.JKey("name") + ADD_Json.Quote(targetName) + "}";
        }
        data = data + "," + ADD_Json.JKey("category") + ADD_Json.Quote(category) + "," + ADD_Json.JKey("message") + ADD_Json.Quote(message);
        data = data + "," + ADD_Json.JKey("pos") + ADD_Json.Vec(pos) + "," + ADD_Json.JKey("fromDeathScreen") + "false}";
        ADD_Publisher.Get().Publish("report", data, true);
        return true;
    }
}

modded class PlayerBase
{
    override void OnRPC(PlayerIdentity sender, int rpc_type, ParamsReadContext ctx)
    {
        super.OnRPC(sender, rpc_type, ctx);
        if (!GetGame() || !GetGame().IsServer() || rpc_type != ADD_ReportRpc.RPC_REPORT)
            return;
        Param3<string, string, string> payload;
        if (!ctx.Read(payload))
            return;
        ADD_ReportRpc.Accept(sender, payload.param1, payload.param2, payload.param3);
    }
}
