class ADD_CombatClassifier
{
    static void Hit(PlayerBase victim, EntityAI source, string zone, string ammo, float damage)
    {
        Object attacker = Attacker(source);
        string data = "{" + ADD_Json.JKey("attacker") + Combatant(attacker, source) + "," + ADD_Json.JKey("victim") + Combatant(victim, null);
        data = data + "," + ADD_Json.JKey("ammo") + ADD_Json.Quote(ammo) + "," + ADD_Json.JKey("zone") + ADD_Json.Quote(zone);
        data = data + "," + ADD_Json.JKey("damage") + ADD_Json.Num(damage) + "," + ADD_Json.JKey("distance") + ADD_Json.Num(Distance(victim, attacker)) + "}";
        if (ADD_Config.Get().On("playerHit"))
            ADD_Publisher.Get().Publish("combat.hit", data, false);
        ADD_CombatCfg combat = ADD_Config.Get().Data.combat;
        PlayerBase shooter = PlayerBase.Cast(attacker);
        if (combat && combat.magicBullet && shooter && shooter.GetIdentity() && Distance(victim, attacker) > 800 && damage > 40)
        {
            string anomaly = "{" + ADD_Json.JKey("kind") + ADD_Json.Quote("magic_bullet") + "," + ADD_Json.JKey("steamId") + ADD_Json.Quote(shooter.GetIdentity().GetPlainId());
            anomaly = anomaly + "," + ADD_Json.JKey("name") + ADD_Json.Quote(shooter.GetIdentity().GetName()) + "," + ADD_Json.JKey("pos") + ADD_Json.Vec(victim.GetPosition()) + "}";
            ADD_Publisher.Get().Publish("security.anomaly", anomaly, true);
        }
    }

    static void Death(PlayerBase victim, Object killer, EntityAI lastSource)
    {
        ADD_Journal.Remember(victim, "death");
        if (!ADD_Config.Get().On("death"))
            return;
        Object weapon = killer;
        if (!weapon)
            weapon = lastSource;
        Object who = Attacker(EntityAI.Cast(weapon));
        if (!who)
            who = weapon;
        string cause = Classify(victim, who);
        string data = "{" + ADD_Json.JKey("victim") + Combatant(victim, null) + "," + ADD_Json.JKey("cause") + ADD_Json.Quote(cause);
        if (who)
            data = data + "," + ADD_Json.JKey("killer") + Combatant(who, EntityAI.Cast(weapon));
        data = data + "," + ADD_Json.JKey("distance") + ADD_Json.Num(Distance(victim, who));
        if (ADD_Config.Get().On("deathItems"))
            data = data + "," + ADD_Json.JKey("items") + Items(victim);
        data = data + "}";
        ADD_Publisher.Get().Publish("combat.death", data, true);
    }

    // Attacker returns the player holding a weapon, or the source itself for zombies, animals and cars.
    protected static Object Attacker(EntityAI source)
    {
        if (!source)
            return null;
        PlayerBase player = ADD_Journal.RootPlayer(source);
        if (player)
            return player;
        return source;
    }

    protected static string Items(PlayerBase victim)
    {
        return ADD_Journal.Inventory(victim);
    }

    protected static string Classify(PlayerBase victim, Object killer)
    {
        if (!killer || killer == victim)
            return "suicide";
        if (killer.IsInherited(PlayerBase))
            return "pvp";
        if (killer.IsInherited(ZombieBase))
            return "zombie";
        if (killer.IsInherited(AnimalBase))
            return "animal";
        if (killer.IsInherited(CarScript) || killer.IsInherited(BoatScript))
            return "vehicle";
        return "unknown";
    }

    protected static string Combatant(Object obj, EntityAI weapon)
    {
        if (!obj)
        {
            string q = ADD_Json.DQuote();
            return "{" + ADD_Json.JKey("name") + q + q + "}";
        }
        string res;
        PlayerBase player = PlayerBase.Cast(obj);
        if (player && player.GetIdentity())
            res = "{" + ADD_Json.JKey("steamId") + ADD_Json.Quote(player.GetIdentity().GetPlainId()) + "," + ADD_Json.JKey("name") + ADD_Json.Quote(player.GetIdentity().GetName());
        else
            res = "{" + ADD_Json.JKey("name") + ADD_Json.Quote(obj.GetType()) + "," + ADD_Json.JKey("class") + ADD_Json.Quote(obj.GetType());
        if (weapon && weapon != obj)
            res = res + "," + ADD_Json.JKey("weapon") + ADD_Json.Quote(weapon.GetType());
        return res + "," + ADD_Json.JKey("pos") + ADD_Json.Vec(obj.GetPosition()) + "}";
    }

    protected static float Distance(Object a, Object b)
    {
        if (!a || !b)
            return 0;
        return vector.Distance(a.GetPosition(), b.GetPosition());
    }
}
