class ADD_JournalVehicles
{
    static string Prefix(EntityAI vehicle)
    {
        if (vehicle.IsInherited(BoatScript))
            return "boat";
        return "car";
    }

    static void Spawned(EntityAI vehicle)
    {
        if (!vehicle || vehicle.IsSetForDeletion())
            return;
        string act = "spawned";
        if (ADD_Journal.Loading())
            act = "loaded";
        ADD_Journal.Write(Prefix(vehicle) + "Spawn", act, null, vehicle, vector.Zero, "", "", vehicle.GetHealth("", "Health"));
    }

    static void Deleted(EntityAI vehicle)
    {
        if (ADD_Journal.ShuttingDown())
            return;
        ADD_Journal.Write(Prefix(vehicle) + "Delete", "deleted", null, vehicle, vector.Zero, "", "", 0);
    }

    static void Hit(EntityAI vehicle, TotalDamageResult result, EntityAI source, string zone, string ammo)
    {
        if (!result)
            return;
        float damage = result.GetHighestDamage("Health");
        if (damage < ADD_Config.Get().Data.journal.vehicleMinDamage)
            return;
        PlayerBase who = ADD_Journal.RootPlayer(source);
        string by = ammo;
        if (source)
            by = source.GetType();
        ADD_Journal.Write(Prefix(vehicle) + "Damage", "hit", who, vehicle, vector.Zero, by, zone, damage);
        ADD_EntityChanges.Get().Touch(vehicle);
    }

    static void Killed(EntityAI vehicle, Object killer)
    {
        PlayerBase who = ADD_Journal.RootPlayer(EntityAI.Cast(killer));
        string by;
        if (killer)
            by = killer.GetType();
        ADD_Journal.Write(Prefix(vehicle) + "Damage", "destroyed", who, vehicle, vector.Zero, by, "", 0);
        ADD_EntityChanges.Get().Changed(vehicle, "destroyed");
    }
}

modded class CarScript
{
    override void EEInit()
    {
        super.EEInit();
        if (GetGame().IsServer())
            GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(ADD_JournalSpawned, 2000, false);
    }

    void ADD_JournalSpawned()
    {
        ADD_JournalVehicles.Spawned(this);
    }

    override void EEDelete(EntityAI parent)
    {
        if (GetGame().IsServer())
            ADD_JournalVehicles.Deleted(this);
        super.EEDelete(parent);
    }

    override void EEHitBy(TotalDamageResult damageResult, int damageType, EntityAI source, int component, string dmgZone, string ammo, vector modelPos, float speedCoef)
    {
        super.EEHitBy(damageResult, damageType, source, component, dmgZone, ammo, modelPos, speedCoef);
        if (GetGame().IsServer())
            ADD_JournalVehicles.Hit(this, damageResult, source, dmgZone, ammo);
    }

    override void EEKilled(Object killer)
    {
        super.EEKilled(killer);
        if (GetGame().IsServer())
            ADD_JournalVehicles.Killed(this, killer);
    }
}

modded class BoatScript
{
    override void EEInit()
    {
        super.EEInit();
        if (GetGame().IsServer())
            GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(ADD_JournalSpawned, 2000, false);
    }

    void ADD_JournalSpawned()
    {
        ADD_JournalVehicles.Spawned(this);
    }

    override void EEDelete(EntityAI parent)
    {
        if (GetGame().IsServer())
            ADD_JournalVehicles.Deleted(this);
        super.EEDelete(parent);
    }

    override void EEHitBy(TotalDamageResult damageResult, int damageType, EntityAI source, int component, string dmgZone, string ammo, vector modelPos, float speedCoef)
    {
        super.EEHitBy(damageResult, damageType, source, component, dmgZone, ammo, modelPos, speedCoef);
        if (GetGame().IsServer())
            ADD_JournalVehicles.Hit(this, damageResult, source, dmgZone, ammo);
    }

    override void EEKilled(Object killer)
    {
        super.EEKilled(killer);
        if (GetGame().IsServer())
            ADD_JournalVehicles.Killed(this, killer);
    }
}
