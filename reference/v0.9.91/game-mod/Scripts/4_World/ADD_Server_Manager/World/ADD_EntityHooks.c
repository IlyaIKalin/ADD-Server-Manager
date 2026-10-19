modded class ZombieBase
{
    override void EEInit()
    {
        super.EEInit();
        if (GetGame() && GetGame().IsServer())
            ADD_EntityRegistry.Get().Upsert("zombie", this);
    }

    override void EEDelete(EntityAI parent)
    {
        if (GetGame() && GetGame().IsServer())
            ADD_EntityRegistry.Get().Remove("zombie", this);
        super.EEDelete(parent);
    }
}

modded class AnimalBase
{
    override void EEInit()
    {
        super.EEInit();
        if (GetGame() && GetGame().IsServer())
            ADD_EntityRegistry.Get().Upsert("animal", this);
    }

    override void EEDelete(EntityAI parent)
    {
        if (GetGame() && GetGame().IsServer())
            ADD_EntityRegistry.Get().Remove("animal", this);
        super.EEDelete(parent);
    }
}

modded class CarScript
{
    override void EEInit()
    {
        super.EEInit();
        if (GetGame() && GetGame().IsServer())
            ADD_EntityRegistry.Get().Upsert("transport", this);
    }

    override void EEDelete(EntityAI parent)
    {
        if (GetGame() && GetGame().IsServer())
            ADD_EntityRegistry.Get().Remove("transport", this);
        super.EEDelete(parent);
    }
}

modded class BoatScript
{
    override void EEInit()
    {
        super.EEInit();
        if (GetGame() && GetGame().IsServer())
            ADD_EntityRegistry.Get().Upsert("boat", this);
    }

    override void EEDelete(EntityAI parent)
    {
        if (GetGame() && GetGame().IsServer())
            ADD_EntityRegistry.Get().Remove("boat", this);
        super.EEDelete(parent);
    }
}

modded class BaseBuildingBase
{
    override void EEInit()
    {
        super.EEInit();
        if (GetGame() && GetGame().IsServer() && !IsKindOf("TerritoryFlag"))
            ADD_EntityRegistry.Get().Upsert("base", this);
    }

    override void EEDelete(EntityAI parent)
    {
        if (GetGame() && GetGame().IsServer())
            ADD_EntityRegistry.Get().Remove("base", this);
        super.EEDelete(parent);
    }
}

modded class TerritoryFlag
{
    override void EEInit()
    {
        super.EEInit();
        if (GetGame() && GetGame().IsServer())
            ADD_EntityRegistry.Get().Upsert("flag", this);
    }

    override void EEDelete(EntityAI parent)
    {
        if (GetGame() && GetGame().IsServer())
            ADD_EntityRegistry.Get().Remove("flag", this);
        super.EEDelete(parent);
    }
}

modded class ContaminatedArea_Dynamic
{
    override void EEInit()
    {
        super.EEInit();
        if (GetGame() && GetGame().IsServer())
            ADD_EntityRegistry.Get().Upsert("gas", this);
    }

    override void EEDelete(EntityAI parent)
    {
        if (GetGame() && GetGame().IsServer())
            ADD_EntityRegistry.Get().Remove("gas", this);
        super.EEDelete(parent);
    }
}

modded class CrashBase
{
    override void EEInit()
    {
        super.EEInit();
        if (GetGame() && GetGame().IsServer())
            ADD_EntityRegistry.Get().Upsert("crash", this);
    }

    override void EEDelete(EntityAI parent)
    {
        if (GetGame() && GetGame().IsServer())
            ADD_EntityRegistry.Get().Remove("crash", this);
        super.EEDelete(parent);
    }
}

modded class ItemBase
{
    override void EEInit()
    {
        super.EEInit();
        if (!GetGame() || !GetGame().IsServer())
            return;
        if (ADD_TrackedItem(this))
            ADD_EntityRegistry.Get().Upsert("item", this);
    }

    override void EEDelete(EntityAI parent)
    {
        if (GetGame() && GetGame().IsServer())
            ADD_EntityRegistry.Get().Remove("item", this);
        super.EEDelete(parent);
    }
}

bool ADD_TrackedItem(EntityAI item)
{
    if (!item)
        return false;
    array<string> list = ADD_Config.Get().Data.trackedItemClasses;
    if (!list)
        return false;
    for (int i = 0; i < list.Count(); i++)
    {
        if (item.IsKindOf(list.Get(i)))
            return true;
    }
    return false;
}
