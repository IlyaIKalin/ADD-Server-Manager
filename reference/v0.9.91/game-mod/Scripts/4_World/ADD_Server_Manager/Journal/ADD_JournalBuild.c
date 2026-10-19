class ADD_JournalBuild
{
    // The base whose last part is being dismantled; its deletion is not a lifetime disappearance.
    static BaseBuildingBase s_Dismantling;

    static void Built(ActionData data)
    {
        BuildPartActionData build = BuildPartActionData.Cast(data);
        if (!build || !data.m_Player || !data.m_Target)
            return;
        BaseBuildingBase base = BaseBuildingBase.Cast(data.m_Target.GetObject());
        string part = build.m_PartType;
        ADD_Journal.Write("build", "built", data.m_Player, base, vector.Zero, part, "", 0);
        if (part == "pole")
            ADD_Journal.Write("flagBuild", "built", data.m_Player, base, vector.Zero, part, "", 0);
        if (base)
            ADD_EntityChanges.Get().Touch(base);
    }

    static string TargetPart(ActionData data)
    {
        if (!data || !data.m_Player || !data.m_Player.GetConstructionActionData())
            return "";
        ConstructionPart part = data.m_Player.GetConstructionActionData().GetTargetPart();
        if (!part)
            return "";
        return part.GetPartName();
    }

    static void Gone(BaseBuildingBase base, string act, Object killer)
    {
        PlayerBase who = ADD_Journal.RootPlayer(EntityAI.Cast(killer));
        string by;
        if (killer)
            by = killer.GetType();
        ADD_Journal.Write("baseGone", act, who, base, vector.Zero, by, "", 0);
        if (act == "destroyed")
            ADD_EntityChanges.Get().Changed(base, act);
    }
}

modded class ActionBuildPart
{
    override void OnFinishProgressServer(ActionData action_data)
    {
        super.OnFinishProgressServer(action_data);
        ADD_JournalBuild.Built(action_data);
    }
}

modded class ActionDismantlePart
{
    override void OnFinishProgressServer(ActionData action_data)
    {
        string part = ADD_JournalBuild.TargetPart(action_data);
        BaseBuildingBase base;
        if (action_data && action_data.m_Target)
            base = BaseBuildingBase.Cast(action_data.m_Target.GetObject());
        if (base)
        {
            ADD_JournalBuild.s_Dismantling = base;
            ADD_Journal.Write("dismantle", "dismantled", action_data.m_Player, base, vector.Zero, part, "", 0);
        }
        super.OnFinishProgressServer(action_data);
        ADD_JournalBuild.s_Dismantling = null;
        if (base)
            ADD_EntityChanges.Get().Touch(base);
    }
}

modded class BaseBuildingBase
{
    protected bool m_ADD_Gone;

    override void EEKilled(Object killer)
    {
        super.EEKilled(killer);
        if (GetGame().IsServer() && !m_ADD_Gone)
        {
            m_ADD_Gone = true;
            ADD_JournalBuild.Gone(this, "destroyed", killer);
        }
    }

    override void EEDelete(EntityAI parent)
    {
        if (GetGame().IsServer() && !m_ADD_Gone && !ADD_Journal.ShuttingDown())
        {
            m_ADD_Gone = true;
            string act = "gone";
            if (ADD_JournalBuild.s_Dismantling == this)
                act = "dismantled";
            ADD_JournalBuild.Gone(this, act, null);
        }
        super.EEDelete(parent);
    }
}
