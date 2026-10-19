class ADD_JournalItems
{
    // The item last reported by OnWasAttached/OnWasDetached; its following location change is the same move.
    static ItemBase s_Reported;

    static void Moved(ItemBase item, InventoryLocation oldLoc, InventoryLocation newLoc)
    {
        if (s_Reported == item)
        {
            s_Reported = null;
            return;
        }
        if (ADD_Journal.Loading() || ADD_Journal.ShuttingDown() || !ADD_Config.Get().On("itemMove"))
            return;
        EntityAI oldParent = oldLoc.GetParent();
        EntityAI newParent = newLoc.GetParent();
        if (GearQuiet(item) || GearQuiet(oldParent) || GearQuiet(newParent))
            return;
        PlayerBase oldRoot = ADD_Journal.RootPlayer(oldParent);
        PlayerBase newRoot = ADD_Journal.RootPlayer(newParent);
        if (oldRoot && !oldRoot.GetIdentity())
            oldRoot = null;
        if (newRoot && !newRoot.GetIdentity())
            newRoot = null;
        if (oldRoot && oldRoot == newRoot)
            return;
        PlayerBase who;
        PlayerBase other;
        string act;
        EntityAI place;
        if (newRoot)
        {
            who = newRoot;
            other = oldRoot;
            place = oldParent;
            act = "picked";
            if (oldParent)
                act = "took";
        }
        else if (oldRoot)
        {
            who = oldRoot;
            place = newParent;
            act = "dropped";
            if (newParent)
                act = "put";
        }
        else if (oldParent && newParent)
        {
            who = ADD_Journal.Nearest(newParent.GetPosition(), 5);
            if (!who)
                return;
            act = "moved";
            place = newParent;
        }
        else
        {
            return;
        }
        string target;
        string detail;
        if (place)
        {
            target = place.GetType();
            detail = ADD_Journal.Pid(place);
        }
        ADD_Journal.Write("itemMove", act, who, item, vector.Zero, target, detail, item.GetQuantity(), other);
    }

    static void Slot(ItemBase item, EntityAI parent, int slotId, bool attached)
    {
        if (ADD_Journal.Loading() || ADD_Journal.ShuttingDown() || !parent)
            return;
        if (GearQuiet(parent) || GearQuiet(item))
            return;
        PlayerBase who = ADD_Journal.RootPlayer(parent);
        if (!who || !who.GetIdentity())
            return;
        s_Reported = item;
        string act = "detached";
        if (attached)
            act = "attached";
        string target = parent.GetType();
        string detail = ADD_Journal.Pid(parent);
        PlayerBase body = PlayerBase.Cast(parent);
        if (body)
        {
            target = InventorySlots.GetSlotName(slotId);
            if (target == "")
                target = "player";
            detail = "";
        }
        ADD_Journal.Write("itemMove", act, who, item, vector.Zero, target, detail, 0);
    }

    // Gear restore on connect fires a move for every owned item. That is not looting.
    static bool GearQuiet(EntityAI entity)
    {
        PlayerBase player = ADD_Journal.RootPlayer(entity);
        return player && player.ADD_GearQuiet();
    }

    static bool Watched(ItemBase item)
    {
        return item.IsInherited(CarWheel) || item.IsInherited(Container_Base) || item.IsInherited(TentBase);
    }

    static void Hit(ItemBase item, TotalDamageResult result, EntityAI source, string zone, string ammo)
    {
        if (!result || (!Watched(item) && !item.IsInherited(BaseBuildingBase)))
            return;
        float damage = result.GetHighestDamage("Health");
        if (damage <= 0.5)
            return;
        PlayerBase who = ADD_Journal.RootPlayer(source);
        string weapon = ammo;
        if (source)
            weapon = source.GetType();
        ADD_Journal.Write("itemDamage", "hit", who, item, vector.Zero, weapon, zone, damage);
    }

    static void Killed(ItemBase item, Object killer)
    {
        if (!Watched(item))
            return;
        PlayerBase who = ADD_Journal.RootPlayer(EntityAI.Cast(killer));
        string by;
        if (killer)
            by = killer.GetType();
        ADD_Journal.Write("itemDamage", "destroyed", who, item, vector.Zero, by, "", 0);
    }
}

modded class ItemBase
{
    override void EEItemLocationChanged(notnull InventoryLocation oldLoc, notnull InventoryLocation newLoc)
    {
        super.EEItemLocationChanged(oldLoc, newLoc);
        if (GetGame().IsServer())
            ADD_JournalItems.Moved(this, oldLoc, newLoc);
    }

    override void OnWasAttached(EntityAI parent, int slot_id)
    {
        super.OnWasAttached(parent, slot_id);
        if (GetGame().IsServer() && ADD_Config.Get().On("itemMove"))
            ADD_JournalItems.Slot(this, parent, slot_id, true);
    }

    override void OnWasDetached(EntityAI parent, int slot_id)
    {
        super.OnWasDetached(parent, slot_id);
        if (GetGame().IsServer() && ADD_Config.Get().On("itemMove"))
            ADD_JournalItems.Slot(this, parent, slot_id, false);
    }

    override void EEHitBy(TotalDamageResult damageResult, int damageType, EntityAI source, int component, string dmgZone, string ammo, vector modelPos, float speedCoef)
    {
        super.EEHitBy(damageResult, damageType, source, component, dmgZone, ammo, modelPos, speedCoef);
        if (GetGame().IsServer() && ADD_Config.Get().On("itemDamage"))
            ADD_JournalItems.Hit(this, damageResult, source, dmgZone, ammo);
    }

    override void EEKilled(Object killer)
    {
        super.EEKilled(killer);
        if (GetGame().IsServer() && ADD_Config.Get().On("itemDamage"))
            ADD_JournalItems.Killed(this, killer);
    }
}

modded class PlayerBase
{
    protected int m_ADD_GearQuietUntil;

    void ADD_HoldGear(int ms)
    {
        int until = GetGame().GetTime() + ms;
        if (until > m_ADD_GearQuietUntil)
            m_ADD_GearQuietUntil = until;
    }

    bool ADD_GearQuiet()
    {
        return GetGame().GetTime() < m_ADD_GearQuietUntil;
    }

    override void EEInit()
    {
        if (GetGame() && GetGame().IsServer())
            ADD_HoldGear(6000);
        super.EEInit();
    }

    override bool OnStoreLoad(ParamsReadContext ctx, int version)
    {
        if (GetGame() && GetGame().IsServer())
            ADD_HoldGear(8000);
        return super.OnStoreLoad(ctx, version);
    }
}
