class ADD_CommandExecutor
{
    protected static ref map<string, string> s_Kv;
    protected static int s_WindGen;
    protected static float s_WindFrom;
    protected static float s_WindTo;
    protected static int s_WeatherGen;
    protected static float s_OverTarget;
    protected static float s_RainTarget;
    // Why the last helper returned false; becomes the ack error.
    protected static string s_Reason;

    static void Run(ADD_ModCommand command)
    {
        if (!command || command.id == "")
        {
            ADD_Log.Warn("cmd: skipped command without id");
            return;
        }
        bool ok = true;
        string error;
        string result = "{}";
        s_Reason = "";
        ADD_CmdParams params = command.params;
        if (!params)
            params = new ADD_CmdParams;
        ADD_Log.Info("cmd " + command.id + " " + command.type + Describe(params));
        if (command.type == "heal")
            ok = Heal(params.steamId);
        else if (command.type == "kill")
            ok = Kill(params.steamId);
        else if (command.type == "kick")
            ok = Kick(params.steamId);
        else if (command.type == "teleport")
            ok = Teleport(params.steamId, params.pos);
        else if (command.type == "entity.teleport")
            ok = TeleportEntity(params.id, params.pos);
        else if (command.type == "entity.kill")
            ok = KillEntity(params.id);
        else if (command.type == "spawn_item")
            ok = SpawnItem(params.steamId, params.className);
        else if (command.type == "inventory")
        {
            PlayerBase who = FindPlayer(params.steamId);
            ok = who != null;
            if (ok)
            {
                result = ADD_Journal.InventoryTree(who);
                ADD_Journal.Poll(who);
            }
        }
        else if (command.type == "inventory.restore")
            ok = RestoreGear(params.steamId, params.text);
        else if (command.type == "vehicle.spawn")
        {
            string spawned = SpawnVehicle(params.className, params.pos);
            ok = spawned != "";
            if (ok)
                result = "{" + ADD_Json.JKey("id") + ADD_Json.Quote(spawned) + "}";
        }
        else if (command.type == "vehicle.repair")
            ok = RepairVehicle(params.id);
        else if (command.type == "vehicle.refuel")
            ok = Refuel(params.id);
        else if (command.type == "vehicle.parts")
            ok = SpawnMissingParts(params.id);
        else if (command.type == "vehicle.delete")
            ok = DeleteEntity(params.id);
        else if (command.type == "vehicle.set_fluid")
            ok = SetFluid(params.id, params.fluid, params.value);
        else if (command.type == "entity.delete")
            ok = DeleteEntity(params.id);
        else if (command.type == "request_snapshot")
            ADD_EntitySnapshots.Request();
        else if (command.type == "time")
            ok = SetDateTime(params);
        else if (command.type == "weather")
            ok = SetWeather(params);
        else if (command.type == "kv.set")
        {
            Kv().Set(params.key, params.text);
            result = "{" + ADD_Json.JKey("key") + ADD_Json.Quote(params.key) + "," + ADD_Json.JKey("value") + ADD_Json.Quote(params.text) + "}";
            ok = params.key != "";
        }
        else if (command.type == "kv.delete")
        {
            Kv().Remove(params.key);
            ok = params.key != "";
        }
        else
        {
            ok = false;
            error = "unsupported";
        }
        if (!ok && error == "")
            error = s_Reason;
        if (!ok && error == "")
            error = "failed";
        if (ok)
            ADD_Log.Info("cmd " + command.id + " ok");
        else
            ADD_Log.Warn("cmd " + command.id + " failed: " + error);
        ADD_Http.Get().PostAck(command.id, ok, error, result);
    }

    protected static string Describe(ADD_CmdParams params)
    {
        string text;
        if (params.steamId != "")
            text = text + " steamId=" + params.steamId;
        if (params.id != "")
            text = text + " id=" + params.id;
        if (params.className != "")
            text = text + " class=" + params.className;
        if (params.pos && params.pos.Count() >= 3)
            text = text + " pos=" + params.pos.Get(0).ToString() + "," + params.pos.Get(1).ToString() + "," + params.pos.Get(2).ToString();
        return text;
    }

    protected static bool Reject(string reason)
    {
        s_Reason = reason;
        return false;
    }

    protected static PlayerBase FindPlayer(string steamId)
    {
        PlayerBase player = ADD_PlayerIdentity.Find(steamId);
        if (!player)
            Reject("player not found: " + steamId);
        return player;
    }

    protected static EntityAI FindEntity(string id)
    {
        EntityAI entity = ADD_EntityRegistry.Get().Find(id);
        if (!entity)
            Reject("entity not found: " + id + " (tracked " + ADD_EntityRegistry.Get().Items().Count().ToString() + ")");
        return entity;
    }

    protected static bool HasPos(array<float> pos)
    {
        if (pos && pos.Count() >= 3)
            return true;
        return Reject("pos must be [x, y, z]");
    }

    protected static map<string, string> Kv()
    {
        if (!s_Kv)
            s_Kv = new map<string, string>;
        return s_Kv;
    }

    protected static bool Heal(string steamId)
    {
        PlayerBase player = FindPlayer(steamId);
        if (!player)
            return false;
        player.SetHealth("", "", player.GetMaxHealth("", ""));
        player.SetHealth("GlobalHealth", "Blood", player.GetMaxHealth("GlobalHealth", "Blood"));
        player.SetHealth("GlobalHealth", "Shock", player.GetMaxHealth("GlobalHealth", "Shock"));
        if (player.GetBleedingManagerServer())
            player.GetBleedingManagerServer().RemoveAllSources();
        player.SetBrokenLegs(eBrokenLegs.NO_BROKEN_LEGS);
        player.RemoveAllAgents();
        player.GetStatWater().Set(player.GetStatWater().GetMax());
        player.GetStatEnergy().Set(player.GetStatEnergy().GetMax());
        return true;
    }

    protected static bool Kill(string steamId)
    {
        PlayerBase player = FindPlayer(steamId);
        if (!player)
            return false;
        if (!player.IsAlive())
            return Reject("player is already dead");
        player.SetHealth("", "", 0);
        return true;
    }

    protected static bool Kick(string steamId)
    {
        PlayerBase player = FindPlayer(steamId);
        if (!player)
            return false;
        PlayerIdentity identity = player.GetIdentity();
        if (!identity)
            return Reject("player has no identity");
        GetGame().DisconnectPlayer(identity);
        return true;
    }

    protected static bool KillEntity(string id)
    {
        EntityAI entity = FindEntity(id);
        if (!entity)
            return false;
        if (!entity.IsAlive())
            return Reject("already dead");
        entity.SetHealth("", "", 0);
        return true;
    }

    // The map has no height: y <= 0 means "put on the ground".
    protected static vector Target(array<float> pos, float lift)
    {
        float x = pos.Get(0);
        float z = pos.Get(2);
        float y = pos.Get(1);
        if (y <= 0)
            y = GetGame().SurfaceY(x, z) + lift;
        return Vector(x, y, z);
    }

    protected static bool Teleport(string steamId, array<float> pos)
    {
        if (!HasPos(pos))
            return false;
        PlayerBase player = FindPlayer(steamId);
        if (!player)
            return false;
        vector was = player.GetPosition();
        vector dest = Target(pos, 0);
        player.SetPosition(dest);
        ADD_Log.Info("teleport " + steamId + " " + was.ToString() + " -> " + dest.ToString() + ", now " + player.GetPosition().ToString());
        return true;
    }

    protected static bool TeleportEntity(string id, array<float> pos)
    {
        if (!HasPos(pos))
            return false;
        EntityAI entity = FindEntity(id);
        if (!entity)
            return false;
        vector was = entity.GetPosition();
        vector dest;
        Transport vehicle = Transport.Cast(entity);
        if (vehicle)
        {
            dest = Target(pos, 1.0);
            vector ori = vehicle.GetOrientation();
            vehicle.SetOrientation(Vector(ori[0], 0, 0));
            vector mat[4];
            vehicle.GetTransform(mat);
            mat[3] = dest;
            vehicle.SetTransform(mat);
            vehicle.Synchronize();
            dBodyActive(vehicle, ActiveState.ACTIVE);
        }
        else
        {
            dest = Target(pos, 0);
            AnimalBase animal = AnimalBase.Cast(entity);
            if (animal)
                LeaveHerd(animal);
            entity.SetPosition(dest);
            entity.Update();
        }
        ADD_Log.Info("teleport " + entity.GetType() + " " + id + " " + was.ToString() + " -> " + dest.ToString() + ", now " + entity.GetPosition().ToString());
        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(ADD_CommandExecutor.CheckTeleport, 10000, false, id, dest);
        return true;
    }

    // A herd walks back to its territory, so a moved animal gets a group without behaviour.
    protected static void LeaveHerd(DayZCreatureAI creature)
    {
        AIAgent agent = creature.GetAIAgent();
        if (!agent)
            return;
        AIWorld world = GetGame().GetWorld().GetAIWorld();
        if (!world)
            return;
        AIGroup lone = world.CreateDefaultGroup();
        if (!lone)
            return;
        AIGroup herd = agent.GetGroup();
        if (herd)
            herd.RemoveAgent(agent);
        lone.AddAgent(agent);
        ADD_Log.Debug("teleport: " + creature.GetType() + " left its herd");
    }

    static void CheckTeleport(string id, vector dest)
    {
        EntityAI entity = ADD_EntityRegistry.Get().Find(id);
        if (!entity)
        {
            ADD_Log.Debug("teleport check " + id + ": entity is gone");
            return;
        }
        vector now = entity.GetPosition();
        float drift = vector.Distance(Vector(now[0], 0, now[2]), Vector(dest[0], 0, dest[2]));
        string line = "teleport check " + entity.GetType() + " " + id + ": now " + now.ToString() + ", " + drift.ToString() + " m from target";
        if (drift > 50)
            ADD_Log.Warn(line);
        else
            ADD_Log.Info(line);
    }

    protected static bool SpawnItem(string steamId, string className)
    {
        if (className == "")
            return Reject("class is empty");
        PlayerBase player = FindPlayer(steamId);
        if (!player)
            return false;
        if (!player.GetInventory().CreateInInventory(className))
            return Reject("no room for " + className + " or unknown class");
        return true;
    }

    protected static string SpawnVehicle(string className, array<float> pos)
    {
        if (className == "" || !pos || pos.Count() < 3)
            return "";
        vector at = Vector(pos.Get(0), pos.Get(1), pos.Get(2));
        Object created = GetGame().CreateObjectEx(className, at, ECE_PLACE_ON_SURFACE | ECE_CREATEPHYSICS);
        EntityAI entity = EntityAI.Cast(created);
        if (!entity)
            return "";
        return ADD_EntityRegistry.NetworkID(entity);
    }

    protected static void RepairTree(EntityAI entity)
    {
        if (!entity)
            return;
        entity.SetHealth("", "", entity.GetMaxHealth("", ""));
        GameInventory inv = entity.GetInventory();
        if (!inv)
            return;
        int n = inv.AttachmentCount();
        for (int i = 0; i < n; i++)
        {
            EntityAI att = inv.GetAttachmentFromIndex(i);
            if (att && att != entity)
                RepairTree(att);
        }
    }

    protected static void RestoreZone(EntityAI entity, string zone)
    {
        float max = entity.GetMaxHealth(zone, "Health");
        if (max > 0)
            entity.SetHealth(zone, "Health", max);
    }

    // Fuel and radiator water. Does not spawn parts or touch the engine and chassis.
    protected static bool FillFuelCoolant(EntityAI entity)
    {
        CarScript car = CarScript.Cast(entity);
        if (car)
        {
            car.Fill(CarFluid.FUEL, car.GetFluidCapacity(CarFluid.FUEL));
            car.Fill(CarFluid.COOLANT, car.GetFluidCapacity(CarFluid.COOLANT));
            return true;
        }
        BoatScript boat = BoatScript.Cast(entity);
        if (boat)
        {
            boat.Fill(BoatFluid.FUEL, boat.GetFluidCapacity(BoatFluid.FUEL));
            return true;
        }
        return false;
    }

    protected static bool RepairVehicle(string id)
    {
        EntityAI entity = FindEntity(id);
        if (!entity)
            return false;
        RepairTree(entity);
        RestoreZone(entity, "Engine");
        RestoreZone(entity, "Chassis");
        FillFuelCoolant(entity);
        return true;
    }

    protected static bool Refuel(string id)
    {
        EntityAI entity = FindEntity(id);
        if (!entity)
            return false;
        if (!FillFuelCoolant(entity))
            return Reject(entity.GetType() + " has no fuel tank");
        return true;
    }

    // OnDebugSpawn creates the class's own attachments. Occupied slots stay.
    // Health and fluids are put back so only the missing parts are new.
    protected static bool SpawnMissingParts(string id)
    {
        EntityAI entity = FindEntity(id);
        if (!entity)
            return false;
        float hull = entity.GetHealth("", "");
        array<EntityAI> kept = new array<EntityAI>;
        array<float> keptHp = new array<float>;
        GameInventory inv = entity.GetInventory();
        if (inv)
        {
            int n = inv.AttachmentCount();
            for (int i = 0; i < n; i++)
            {
                EntityAI att = inv.GetAttachmentFromIndex(i);
                if (!att)
                    continue;
                kept.Insert(att);
                keptHp.Insert(att.GetHealth("", ""));
            }
        }
        float fuel = 0;
        float oil = 0;
        float brake = 0;
        float coolant = 0;
        float boatFuel = 0;
        bool isCar = false;
        bool isBoat = false;
        CarScript car = CarScript.Cast(entity);
        if (car)
        {
            isCar = true;
            fuel = car.GetFluidFraction(CarFluid.FUEL);
            oil = car.GetFluidFraction(CarFluid.OIL);
            brake = car.GetFluidFraction(CarFluid.BRAKE);
            coolant = car.GetFluidFraction(CarFluid.COOLANT);
        }
        BoatScript boat = BoatScript.Cast(entity);
        if (boat)
        {
            isBoat = true;
            boatFuel = boat.GetFluidFraction(BoatFluid.FUEL);
        }
        entity.OnDebugSpawn();
        entity.SetHealth("", "", hull);
        for (int k = 0; k < kept.Count(); k++)
        {
            if (kept.Get(k))
                kept.Get(k).SetHealth("", "", keptHp.Get(k));
        }
        if (isCar)
        {
            car.LeakAll(CarFluid.FUEL);
            car.Fill(CarFluid.FUEL, car.GetFluidCapacity(CarFluid.FUEL) * fuel);
            car.LeakAll(CarFluid.OIL);
            car.Fill(CarFluid.OIL, car.GetFluidCapacity(CarFluid.OIL) * oil);
            car.LeakAll(CarFluid.BRAKE);
            car.Fill(CarFluid.BRAKE, car.GetFluidCapacity(CarFluid.BRAKE) * brake);
            car.LeakAll(CarFluid.COOLANT);
            car.Fill(CarFluid.COOLANT, car.GetFluidCapacity(CarFluid.COOLANT) * coolant);
        }
        if (isBoat)
        {
            boat.LeakAll(BoatFluid.FUEL);
            boat.Fill(BoatFluid.FUEL, boat.GetFluidCapacity(BoatFluid.FUEL) * boatFuel);
        }
        return true;
    }

    protected static bool SetFluid(string id, string fluid, float value)
    {
        EntityAI entity = FindEntity(id);
        if (!entity)
            return false;
        CarScript car = CarScript.Cast(entity);
        if (!car)
            return Reject(entity.GetType() + " is not a car");
        CarFluid kind = CarFluid.FUEL;
        if (fluid == "oil")
            kind = CarFluid.OIL;
        else if (fluid == "brake")
            kind = CarFluid.BRAKE;
        else if (fluid == "coolant")
            kind = CarFluid.COOLANT;
        else if (fluid != "fuel" && fluid != "")
            return Reject("unknown fluid " + fluid);
        if (value < 0)
            value = 0;
        if (value > 1)
            value = 1;
        float capacity = car.GetFluidCapacity(kind);
        car.LeakAll(kind);
        car.Fill(kind, capacity * value);
        return true;
    }

    protected static bool DeleteEntity(string id)
    {
        EntityAI entity = FindEntity(id);
        if (!entity)
            return false;
        ADD_Log.Info("delete " + entity.GetType() + " " + id + " at " + entity.GetPosition().ToString());
        GetGame().ObjectDelete(entity);
        return true;
    }

    protected static bool SetDateTime(ADD_CmdParams params)
    {
        int year, month, day, curHour, curMinute;
        GetGame().GetWorld().GetDate(year, month, day, curHour, curMinute);
        int y = params.year;
        int mo = params.month;
        int d = params.day;
        int h = params.hour;
        int mi = params.minute;
        if (y < 1)
            y = year;
        if (mo < 1)
            mo = month;
        if (d < 1)
            d = day;
        if (mo < 1)
            mo = 1;
        if (mo > 12)
            mo = 12;
        if (d < 1)
            d = 1;
        if (d > 31)
            d = 31;
        if (h < 0)
            h = 0;
        if (h > 23)
            h = 23;
        if (mi < 0)
            mi = 0;
        if (mi > 59)
            mi = 59;
        GetGame().GetWorld().SetDate(y, mo, d, h, mi);
        return true;
    }

    // The whole change still takes about a minute. Rain and the sky do not move
    // together: a clear sky with rain still falling is the thing to avoid.
    // Rain stops while the clouds are still up, then the sky clears.
    // Rain only starts after the clouds have come in.
    protected static bool SetWeather(ADD_CmdParams params)
    {
        Weather weather = GetGame().GetWeather();
        if (!weather)
            return false;
        s_WeatherGen = s_WeatherGen + 1;
        int gen = s_WeatherGen;
        s_OverTarget = params.overcast;
        s_RainTarget = params.rain;
        float rainNow = weather.GetRain().GetActual();
        float overNow = weather.GetOvercast().GetActual();
        weather.GetFog().Set(params.fog, 60, 0);
        if (params.rain + 0.02 < rainNow)
        {
            weather.GetRain().Set(params.rain, 25, 60);
            weather.GetOvercast().Set(overNow, 0, 30);
            GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(ADD_CommandExecutor.FinishSky, 30000, false, gen);
        }
        else if (params.rain > rainNow + 0.02)
        {
            weather.GetOvercast().Set(params.overcast, 30, 60);
            weather.GetRain().Set(rainNow, 0, 30);
            GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(ADD_CommandExecutor.FinishRain, 30000, false, gen);
        }
        else
        {
            weather.GetOvercast().Set(params.overcast, 60, 0);
            weather.GetRain().Set(params.rain, 60, 0);
        }
        s_WindGen = s_WindGen + 1;
        s_WindFrom = weather.GetWindSpeed();
        s_WindTo = params.wind;
        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(ADD_CommandExecutor.StepWind, 1000, false, s_WindGen, 1);
        return true;
    }

    static void FinishSky(int gen)
    {
        if (gen != s_WeatherGen)
            return;
        Weather weather = GetGame().GetWeather();
        if (!weather)
            return;
        weather.GetOvercast().Set(s_OverTarget, 30, 0);
    }

    static void FinishRain(int gen)
    {
        if (gen != s_WeatherGen)
            return;
        Weather weather = GetGame().GetWeather();
        if (!weather)
            return;
        weather.GetRain().Set(s_RainTarget, 30, 0);
    }

    static void StepWind(int gen, int step)
    {
        if (gen != s_WindGen)
            return;
        Weather weather = GetGame().GetWeather();
        if (!weather)
            return;
        if (step >= 60)
        {
            weather.SetWindSpeed(s_WindTo);
            return;
        }
        float speed = s_WindFrom + (s_WindTo - s_WindFrom) * (step * 1.0 / 60);
        weather.SetWindSpeed(speed);
        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(ADD_CommandExecutor.StepWind, 1000, false, gen, step + 1);
    }

    protected static bool RestoreGear(string steamId, string raw)
    {
        PlayerBase player = FindPlayer(steamId);
        if (!player)
            return false;
        if (raw == "")
            raw = "[]";
        string body = "{\"items\":" + raw + "}";
        body.Replace("\"class\":", "\"kind\":");
        ADD_GearFile file = new ADD_GearFile;
        string error;
        JsonSerializer serializer = new JsonSerializer;
        if (!serializer.ReadFromString(file, body, error))
            return Reject("inventory: " + error);
        ClearGear(player);
        if (file.items)
        {
            for (int i = 0; i < file.items.Count(); i++)
                PlaceGear(player, player.GetInventory(), file.items.Get(i), true);
        }
        return true;
    }

    protected static void ClearGear(PlayerBase player)
    {
        if (!player || !player.GetInventory())
            return;
        EntityAI held = null;
        if (player.GetHumanInventory())
            held = player.GetHumanInventory().GetEntityInHands();
        array<EntityAI> worn = new array<EntityAI>;
        bool heldWorn = false;
        GameInventory inv = player.GetInventory();
        for (int i = 0; i < inv.AttachmentCount(); i++)
        {
            EntityAI att = inv.GetAttachmentFromIndex(i);
            if (!att)
                continue;
            if (att == held)
                heldWorn = true;
            worn.Insert(att);
        }
        if (held && !heldWorn)
            GetGame().ObjectDelete(held);
        for (int n = 0; n < worn.Count(); n++)
            GetGame().ObjectDelete(worn.Get(n));
    }

    protected static void PlaceGear(PlayerBase player, GameInventory parent, ADD_GearNode node, bool root)
    {
        if (!node || node.kind == "" || !parent)
            return;
        EntityAI created = null;
        if (root && node.slot == "Hands" && player.GetHumanInventory())
            created = player.GetHumanInventory().CreateInHands(node.kind);
        else if (node.slot != "")
        {
            int slotId = InventorySlots.GetSlotIdFromString(node.slot);
            if (slotId >= 0)
                created = parent.CreateAttachmentEx(node.kind, slotId);
        }
        if (!created)
            created = parent.CreateInInventory(node.kind);
        if (!created)
        {
            ADD_Log.Warn("restore: no room for " + node.kind);
            return;
        }
        created.SetHealth("", "Health", node.health);
        ItemBase ib = ItemBase.Cast(created);
        if (ib && node.qty > 0 && ib.GetQuantityMax() > 0)
            ib.SetQuantity(node.qty);
        if (!node.children || !created.GetInventory())
            return;
        for (int i = 0; i < node.children.Count(); i++)
            PlaceGear(player, created.GetInventory(), node.children.Get(i), false);
    }
}
