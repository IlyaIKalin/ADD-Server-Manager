class ADD_JournalHealth
{
    static void Log(PlayerBase player, string cat, string what, bool on)
    {
        if (!GetGame().IsServer() || !player || !player.GetIdentity())
            return;
        string act = "end";
        if (on)
            act = "start";
        ADD_Journal.Write(cat, act, player, null, player.GetPosition(), what, "", 0);
    }
}
modded class PoisoningMdfr
{
    override void OnActivate(PlayerBase player)
    {
        super.OnActivate(player);
        ADD_JournalHealth.Log(player, "disease", "poisoning", true);
    }

    override void OnDeactivate(PlayerBase player)
    {
        super.OnDeactivate(player);
        ADD_JournalHealth.Log(player, "disease", "poisoning", false);
    }
}
modded class BrokenLegsMdfr
{
    override void OnActivate(PlayerBase player)
    {
        super.OnActivate(player);
        ADD_JournalHealth.Log(player, "brokenLegs", "legs", true);
    }

    override void OnDeactivate(PlayerBase player)
    {
        super.OnDeactivate(player);
        ADD_JournalHealth.Log(player, "brokenLegs", "legs", false);
    }
}
modded class BrainDiseaseMdfr
{
    override protected void OnActivate(PlayerBase player)
    {
        super.OnActivate(player);
        ADD_JournalHealth.Log(player, "disease", "brain", true);
    }

    override protected void OnDeactivate(PlayerBase player)
    {
        super.OnDeactivate(player);
        ADD_JournalHealth.Log(player, "disease", "brain", false);
    }
}
modded class CholeraMdfr
{
    override protected void OnActivate(PlayerBase player)
    {
        super.OnActivate(player);
        ADD_JournalHealth.Log(player, "disease", "cholera", true);
    }

    override protected void OnDeactivate(PlayerBase player)
    {
        super.OnDeactivate(player);
        ADD_JournalHealth.Log(player, "disease", "cholera", false);
    }
}
modded class CommonColdMdfr
{
    override protected void OnActivate(PlayerBase player)
    {
        super.OnActivate(player);
        ADD_JournalHealth.Log(player, "disease", "cold", true);
    }

    override protected void OnDeactivate(PlayerBase player)
    {
        super.OnDeactivate(player);
        ADD_JournalHealth.Log(player, "disease", "cold", false);
    }
}
modded class SalmonellaMdfr
{
    override protected void OnActivate(PlayerBase player)
    {
        super.OnActivate(player);
        ADD_JournalHealth.Log(player, "disease", "salmonella", true);
    }

    override protected void OnDeactivate(PlayerBase player)
    {
        super.OnDeactivate(player);
        ADD_JournalHealth.Log(player, "disease", "salmonella", false);
    }
}
modded class ContaminationStage1Mdfr
{
    override protected void OnActivate(PlayerBase player)
    {
        super.OnActivate(player);
        ADD_JournalHealth.Log(player, "disease", "contamination1", true);
    }

    override protected void OnDeactivate(PlayerBase player)
    {
        super.OnDeactivate(player);
        ADD_JournalHealth.Log(player, "disease", "contamination1", false);
    }
}
modded class ContaminationStage2Mdfr
{
    override protected void OnActivate(PlayerBase player)
    {
        super.OnActivate(player);
        ADD_JournalHealth.Log(player, "disease", "contamination2", true);
    }

    override protected void OnDeactivate(PlayerBase player)
    {
        super.OnDeactivate(player);
        ADD_JournalHealth.Log(player, "disease", "contamination2", false);
    }
}
modded class ContaminationStage3Mdfr
{
    override protected void OnActivate(PlayerBase player)
    {
        super.OnActivate(player);
        ADD_JournalHealth.Log(player, "disease", "contamination3", true);
    }

    override protected void OnDeactivate(PlayerBase player)
    {
        super.OnDeactivate(player);
        ADD_JournalHealth.Log(player, "disease", "contamination3", false);
    }
}
modded class HeavyMetalPhase1Mdfr
{
    override protected void OnActivate(PlayerBase player)
    {
        super.OnActivate(player);
        ADD_JournalHealth.Log(player, "disease", "heavymetal1", true);
    }

    override protected void OnDeactivate(PlayerBase player)
    {
        super.OnDeactivate(player);
        ADD_JournalHealth.Log(player, "disease", "heavymetal1", false);
    }
}
modded class HeavyMetalPhase2Mdfr
{
    override protected void OnActivate(PlayerBase player)
    {
        super.OnActivate(player);
        ADD_JournalHealth.Log(player, "disease", "heavymetal2", true);
    }

    override protected void OnDeactivate(PlayerBase player)
    {
        super.OnDeactivate(player);
        ADD_JournalHealth.Log(player, "disease", "heavymetal2", false);
    }
}
modded class HeavyMetalPhase3Mdfr
{
    override protected void OnActivate(PlayerBase player)
    {
        super.OnActivate(player);
        ADD_JournalHealth.Log(player, "disease", "heavymetal3", true);
    }

    override protected void OnDeactivate(PlayerBase player)
    {
        super.OnDeactivate(player);
        ADD_JournalHealth.Log(player, "disease", "heavymetal3", false);
    }
}
modded class InfluenzaMdfr
{
    override protected void OnActivate(PlayerBase player)
    {
        super.OnActivate(player);
        ADD_JournalHealth.Log(player, "disease", "influenza", true);
    }

    override protected void OnDeactivate(PlayerBase player)
    {
        super.OnDeactivate(player);
        ADD_JournalHealth.Log(player, "disease", "influenza", false);
    }
}
modded class PneumoniaMdfr
{
    override protected void OnActivate(PlayerBase player)
    {
        super.OnActivate(player);
        ADD_JournalHealth.Log(player, "disease", "pneumonia", true);
    }

    override protected void OnDeactivate(PlayerBase player)
    {
        super.OnDeactivate(player);
        ADD_JournalHealth.Log(player, "disease", "pneumonia", false);
    }
}
modded class WoundInfectStage1Mdfr
{
    override protected void OnActivate(PlayerBase player)
    {
        super.OnActivate(player);
        ADD_JournalHealth.Log(player, "disease", "woundinfect1", true);
    }

    override protected void OnDeactivate(PlayerBase player)
    {
        super.OnDeactivate(player);
        ADD_JournalHealth.Log(player, "disease", "woundinfect1", false);
    }
}
modded class WoundInfectStage2Mdfr
{
    override protected void OnActivate(PlayerBase player)
    {
        super.OnActivate(player);
        ADD_JournalHealth.Log(player, "disease", "woundinfect2", true);
    }

    override protected void OnDeactivate(PlayerBase player)
    {
        super.OnDeactivate(player);
        ADD_JournalHealth.Log(player, "disease", "woundinfect2", false);
    }
}
