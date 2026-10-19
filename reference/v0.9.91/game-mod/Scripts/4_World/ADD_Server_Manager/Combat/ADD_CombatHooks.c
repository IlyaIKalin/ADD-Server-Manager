modded class PlayerBase
{
    protected EntityAI m_ADD_LastSource;

    override void EEHitBy(TotalDamageResult damageResult, int damageType, EntityAI source, int component, string dmgZone, string ammo, vector modelPos, float speedCoef)
    {
        super.EEHitBy(damageResult, damageType, source, component, dmgZone, ammo, modelPos, speedCoef);
        if (!GetGame() || !GetGame().IsServer() || !damageResult)
            return;
        float damage = damageResult.GetDamage(dmgZone, "Health");
        ADD_CombatCfg combat = ADD_Config.Get().Data.combat;
        if (combat && damage < combat.hitMinDamage)
            return;
        m_ADD_LastSource = source;
        ADD_CombatClassifier.Hit(this, source, dmgZone, ammo, damage);
    }

    override void EEKilled(Object killer)
    {
        string corpseName = "";
        if (GetIdentity())
            corpseName = GetIdentity().GetName();
        if (GetGame() && GetGame().IsServer())
            ADD_CombatClassifier.Death(this, killer, m_ADD_LastSource);
        super.EEKilled(killer);
        if (GetGame() && GetGame().IsServer() && !IsAlive())
        {
            ADD_EntityRegistry reg = ADD_EntityRegistry.Get();
            reg.Upsert("corpse", this);
            reg.Note(this, corpseName);
        }
    }

    override void EEDelete(EntityAI parent)
    {
        if (GetGame() && GetGame().IsServer())
            ADD_EntityRegistry.Get().Remove("corpse", this);
        super.EEDelete(parent);
    }

    override void OnBleedingSourceAdded()
    {
        super.OnBleedingSourceAdded();
        if (GetGame().IsServer() && GetIdentity())
            ADD_Journal.Write("cuts", "cut", this, null, GetPosition(), "", "", GetBleedingSourceCount());
    }

    override void OnUnconsciousStart()
    {
        super.OnUnconsciousStart();
        if (GetGame().IsServer() && GetIdentity())
            ADD_Journal.Write("unconscious", "start", this, null, GetPosition(), "", "", 0);
    }

    override void OnUnconsciousStop(int pCurrentCommandID)
    {
        super.OnUnconsciousStop(pCurrentCommandID);
        if (GetGame().IsServer() && GetIdentity())
            ADD_Journal.Write("unconscious", "stop", this, null, GetPosition(), "", "", 0);
    }
}
