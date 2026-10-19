class ADD_Journal
{
    protected static bool s_ShuttingDown;

    static void SetShuttingDown()
    {
        s_ShuttingDown = true;
    }

    // Entities are deleted in bulk when the mission ends; those deletions are not world events.
    static bool ShuttingDown()
    {
        return s_ShuttingDown;
    }

    // Loaded is true while the server is still restoring the persisted world.
    static bool Loading()
    {
        return GetGame().GetTime() < 120000;
    }

    static string Pid(EntityAI entity)
    {
        if (!entity)
            return "";
        int b1, b2, b3, b4;
        entity.GetPersistentID(b1, b2, b3, b4);
        if (b1 == 0 && b2 == 0 && b3 == 0 && b4 == 0)
            return "";
        return b1.ToString() + "-" + b2.ToString() + "-" + b3.ToString() + "-" + b4.ToString();
    }

    static PlayerBase RootPlayer(EntityAI entity)
    {
        if (!entity)
            return null;
        PlayerBase player = PlayerBase.Cast(entity);
        if (player)
            return player;
        return PlayerBase.Cast(entity.GetHierarchyRootPlayer());
    }

    static PlayerBase Nearest(vector pos, float radius)
    {
        array<Man> players = new array<Man>;
        GetGame().GetPlayers(players);
        PlayerBase best;
        float bestDistance = radius;
        for (int i = 0; i < players.Count(); i++)
        {
            PlayerBase candidate = PlayerBase.Cast(players.Get(i));
            if (!candidate || !candidate.GetIdentity())
                continue;
            float distance = vector.Distance(pos, candidate.GetPosition());
            if (distance <= bestDistance)
            {
                bestDistance = distance;
                best = candidate;
            }
        }
        return best;
    }

    static string Steam(PlayerBase player)
    {
        if (player && player.GetIdentity())
            return player.GetIdentity().GetPlainId();
        return "";
    }

    // Write publishes one journal row when its stream is on.
    // who is the player the row belongs to; other is the second player (attacker, victim, owner).
    static void Write(string cat, string act, PlayerBase who, EntityAI obj, vector pos, string target, string detail, float value, PlayerBase other = null)
    {
        if (!ADD_Config.Get().On(cat))
            return;
        string data = "{" + ADD_Json.JKey("cat") + ADD_Json.Quote(cat) + "," + ADD_Json.JKey("act") + ADD_Json.Quote(act);
        if (who && who.GetIdentity())
        {
            data = data + "," + ADD_Json.JKey("steamId") + ADD_Json.Quote(who.GetIdentity().GetPlainId());
            data = data + "," + ADD_Json.JKey("name") + ADD_Json.Quote(who.GetIdentity().GetName());
        }
        if (other && other.GetIdentity() && other != who)
        {
            data = data + "," + ADD_Json.JKey("otherId") + ADD_Json.Quote(other.GetIdentity().GetPlainId());
            data = data + "," + ADD_Json.JKey("otherName") + ADD_Json.Quote(other.GetIdentity().GetName());
        }
        if (obj)
        {
            if (pos == vector.Zero)
                pos = obj.GetPosition();
            data = data + "," + ADD_Json.JKey("class") + ADD_Json.Quote(obj.GetType());
            string pid = Pid(obj);
            if (pid != "")
                data = data + "," + ADD_Json.JKey("pid") + ADD_Json.Quote(pid);
        }
        if (pos == vector.Zero && who)
            pos = who.GetPosition();
        data = data + "," + ADD_Json.JKey("pos") + ADD_Json.Vec(pos);
        if (target != "")
            data = data + "," + ADD_Json.JKey("target") + ADD_Json.Quote(target);
        if (detail != "")
            data = data + "," + ADD_Json.JKey("detail") + ADD_Json.Quote(detail);
        if (value != 0)
            data = data + "," + ADD_Json.JKey("value") + ADD_Json.Num(value);
        data = data + "}";
        ADD_Publisher.Get().Publish("journal", data, false);
    }

    // Inventory is everything the player is carrying, including items inside containers.
    static string Inventory(PlayerBase player)
    {
        string res = "[";
        if (!player || !player.GetInventory())
            return res + "]";
        array<EntityAI> all = new array<EntityAI>;
        player.GetInventory().EnumerateInventory(InventoryTraversalType.INORDER, all);
        bool first = true;
        for (int i = 0; i < all.Count(); i++)
        {
            ItemBase item = ItemBase.Cast(all.Get(i));
            if (!item)
                continue;
            if (!first)
                res = res + ",";
            first = false;
            res = res + "{" + ADD_Json.JKey("class") + ADD_Json.Quote(item.GetType());
            string pid = Pid(item);
            if (pid != "")
                res = res + "," + ADD_Json.JKey("pid") + ADD_Json.Quote(pid);
            if (item.GetQuantityMax() > 0)
                res = res + "," + ADD_Json.JKey("qty") + ADD_Json.Num(item.GetQuantity());
            res = res + "," + ADD_Json.JKey("health") + ADD_Json.Num(item.GetHealth("", "Health")) + "}";
        }
        return res + "]";
    }

    // Poll records the same inventory list the journal already stores on join and death.
    static void Poll(PlayerBase player)
    {
        if (!player || !player.GetIdentity())
            return;
        PlayerIdentity identity = player.GetIdentity();
        string data = "{" + ADD_Json.JKey("cat") + ADD_Json.Quote("inventory");
        data = data + "," + ADD_Json.JKey("act") + ADD_Json.Quote("poll");
        data = data + "," + ADD_Json.JKey("steamId") + ADD_Json.Quote(ADD_PlayerIdentity.SteamID(identity));
        data = data + "," + ADD_Json.JKey("name") + ADD_Json.Quote(ADD_PlayerIdentity.Name(identity));
        vector pos = player.GetPosition();
        data = data + "," + ADD_Json.JKey("pos") + ADD_Json.Vec(pos);
        data = data + "," + ADD_Json.JKey("items") + Inventory(player) + "}";
        ADD_Publisher.Get().Publish("journal", data, false);
        Remember(player, "poll");
    }

    // Remember keeps a placed snapshot (slot, class, quantity, health, nesting) for restore.
    static void Remember(PlayerBase player, string source)
    {
        if (!player || !player.GetIdentity() || source == "")
            return;
        PlayerIdentity identity = player.GetIdentity();
        string data = "{" + ADD_Json.JKey("steamId") + ADD_Json.Quote(ADD_PlayerIdentity.SteamID(identity));
        data = data + "," + ADD_Json.JKey("name") + ADD_Json.Quote(ADD_PlayerIdentity.Name(identity));
        data = data + "," + ADD_Json.JKey("source") + ADD_Json.Quote(source);
        data = data + "," + ADD_Json.JKey("items") + InventoryNodes(player) + "}";
        ADD_Publisher.Get().Publish("inventory.snapshot", data, false);
    }

    // InventoryTree is the live-map view: slot, classname and nesting. No persistent id.
    static string InventoryTree(PlayerBase player)
    {
        return "{" + ADD_Json.JKey("items") + InventoryNodes(player) + "}";
    }

    protected static string InventoryNodes(PlayerBase player)
    {
        string res = "[";
        if (!player || !player.GetInventory())
            return res + "]";
        EntityAI hands = null;
        if (player.GetHumanInventory())
            hands = player.GetHumanInventory().GetEntityInHands();
        bool first = true;
        bool handsListed = false;
        GameInventory inv = player.GetInventory();
        for (int i = 0; i < inv.AttachmentCount(); i++)
        {
            EntityAI att = inv.GetAttachmentFromIndex(i);
            if (!att)
                continue;
            if (att == hands)
                handsListed = true;
            string slot = SlotOf(att);
            if (att == hands && slot == "")
                slot = "Hands";
            if (!first)
                res = res + ",";
            first = false;
            res = res + InventoryNode(att, slot);
        }
        if (hands && !handsListed)
        {
            if (!first)
                res = res + ",";
            res = res + InventoryNode(hands, "Hands");
        }
        return res + "]";
    }

    protected static string SlotOf(EntityAI item)
    {
        if (!item || !item.GetInventory())
            return "";
        InventoryLocation loc = new InventoryLocation;
        if (!item.GetInventory().GetCurrentInventoryLocation(loc))
            return "";
        int slotId = loc.GetSlot();
        if (slotId < 0)
            return "";
        return InventorySlots.GetSlotName(slotId);
    }

    protected static string InventoryNode(EntityAI item, string slot)
    {
        string res = "{" + ADD_Json.JKey("slot") + ADD_Json.Quote(slot);
        res = res + "," + ADD_Json.JKey("class") + ADD_Json.Quote(item.GetType());
        ItemBase ib = ItemBase.Cast(item);
        if (ib && ib.GetQuantityMax() > 0)
            res = res + "," + ADD_Json.JKey("qty") + ADD_Json.Num(ib.GetQuantity());
        res = res + "," + ADD_Json.JKey("health") + ADD_Json.Num(item.GetHealth("", "Health"));
        res = res + "," + ADD_Json.JKey("children") + InventoryChildren(item) + "}";
        return res;
    }

    protected static string InventoryChildren(EntityAI item)
    {
        string res = "[";
        if (!item || !item.GetInventory())
            return res + "]";
        bool first = true;
        GameInventory inv = item.GetInventory();
        for (int i = 0; i < inv.AttachmentCount(); i++)
        {
            EntityAI att = inv.GetAttachmentFromIndex(i);
            if (!att)
                continue;
            if (!first)
                res = res + ",";
            first = false;
            res = res + InventoryNode(att, SlotOf(att));
        }
        CargoBase cargo = inv.GetCargo();
        if (cargo)
        {
            for (int c = 0; c < cargo.GetItemCount(); c++)
            {
                EntityAI inside = cargo.GetItem(c);
                if (!inside)
                    continue;
                if (!first)
                    res = res + ",";
                first = false;
                res = res + InventoryNode(inside, "");
            }
        }
        return res + "]";
    }

    static bool Listed(array<string> list, string name)
    {
        if (!list || list.Count() == 0)
            return true;
        return list.Find(name) >= 0;
    }
}
