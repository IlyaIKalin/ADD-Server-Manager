class ADD_Json
{
    static string DQuote()
    {
        int code = 34;
        return code.AsciiToString();

    }

    static string Backslash()
    {
        int code = 92;
        return code.AsciiToString();
    }

    static string JKey(string key)
    {
        string q = DQuote();
        return q + key + q + ":";
    }

    static string Quote(string value)
    {
        if (!value)
            value = "";
        string bs = Backslash();
        string q = DQuote();
        value.Replace(bs, bs + bs);
        value.Replace(q, bs + q);
        value.Replace("\n", bs + "n");
        value.Replace("\r", bs + "r");
        return q + value + q;
    }

    static string Num(float value)
    {
        return value.ToString();
    }

    static string Vec(vector pos)
    {
        return "[" + pos[0].ToString() + "," + pos[1].ToString() + "," + pos[2].ToString() + "]";
    }

    static string ErrorCode(string body)
    {
        if (!body || body == "")
            return "";
        ADD_ApiBody parsed = new ADD_ApiBody;
        string error;
        JsonSerializer serializer = new JsonSerializer;
        if (!serializer.ReadFromString(parsed, body, error) || !parsed.error)
            return "";
        return parsed.error.code;
    }

    static int ConfigVersion(string body)
    {
        if (!body || body == "")
            return 0;
        ADD_ApiBody parsed = new ADD_ApiBody;
        string error;
        JsonSerializer serializer = new JsonSerializer;
        if (!serializer.ReadFromString(parsed, body, error))
            return 0;
        return parsed.configVersion;
    }
}

class ADD_ErrorBody
{
    string code;
    string message;
}

class ADD_ApiBody
{
    bool ok;
    int configVersion;
    ref ADD_ErrorBody error;

    void ADD_ApiBody()
    {
        error = new ADD_ErrorBody;
    }
}

class ADD_CmdParams
{
    string steamId;
    string className;
    string id;
    ref array<float> pos;
    int year;
    int month;
    int day;
    float hour;
    float minute;
    string fluid;
    float value;
    string key;
    string text;
    float overcast;
    float fog;
    float rain;
    float wind;

    void ADD_CmdParams()
    {
        pos = new array<float>;
    }
}

class ADD_GearNode
{
    string slot;
    string kind;
    float qty;
    float health;
    ref array<ref ADD_GearNode> children;

    void ADD_GearNode()
    {
        children = new array<ref ADD_GearNode>;
    }
}

class ADD_GearFile
{
    ref array<ref ADD_GearNode> items;

    void ADD_GearFile()
    {
        items = new array<ref ADD_GearNode>;
    }
}

class ADD_ModCommand
{
    string id;
    string type;
    ref ADD_CmdParams params;

    void ADD_ModCommand()
    {
        params = new ADD_CmdParams;
    }
}

class ADD_CommandsBody
{
    ref array<ref ADD_ModCommand> commands;

    void ADD_CommandsBody()
    {
        commands = new array<ref ADD_ModCommand>;
    }
}

class ADD_ConfigEnvelope
{
    ref ADD_ModConfigData config;

    void ADD_ConfigEnvelope()
    {
        config = new ADD_ModConfigData;
    }
}

class ADD_Bootstrap
{
    string managerUrl;
    string instanceId;
    string token;
}

class ADD_Intervals
{
    int heartbeat;
    int online;
    int positions;
}

class ADD_SnapshotsCfg
{
    int intervalSec;
    int changeScanSec;
}

// Every journal stream has its own switch. A stream that is off is not collected and not sent.
class ADD_Streams
{
    bool playerHit = true;
    bool disease = true;
    bool brokenLegs = true;
    bool unconscious = true;
    bool cuts = true;
    bool death = true;
    bool deathItems = true;
    bool session = true;
    bool chat = true;
    bool itemMove = true;
    bool itemDamage = true;
    bool action = true;
    bool recipe = true;
    bool carDamage = true;
    bool carUse = true;
    bool carSpawn = true;
    bool carDelete = true;
    bool carMove = true;
    bool boatDamage = true;
    bool boatUse = true;
    bool boatSpawn = true;
    bool boatDelete = true;
    bool boatMove = true;
    bool build = true;
    bool dismantle = true;
    bool flagBuild = true;
    bool baseGone = true;
    bool baseChange = true;
    bool trackedItems = true;
    bool zombie = true;
    bool animal = true;
    bool gas = true;
    bool crash = true;
    bool corpse = true;
    bool heartbeat = true;
    bool online = true;
    bool positions = true;

    bool On(string key)
    {
        switch (key)
        {
            case "playerHit": return playerHit;
            case "disease": return disease;
            case "brokenLegs": return brokenLegs;
            case "unconscious": return unconscious;
            case "cuts": return cuts;
            case "death": return death;
            case "deathItems": return deathItems;
            case "session": return session;
            case "chat": return chat;
            case "itemMove": return itemMove;
            case "itemDamage": return itemDamage;
            case "action": return action;
            case "recipe": return recipe;
            case "carDamage": return carDamage;
            case "carUse": return carUse;
            case "carSpawn": return carSpawn;
            case "carDelete": return carDelete;
            case "carMove": return carMove;
            case "boatDamage": return boatDamage;
            case "boatUse": return boatUse;
            case "boatSpawn": return boatSpawn;
            case "boatDelete": return boatDelete;
            case "boatMove": return boatMove;
            case "build": return build;
            case "dismantle": return dismantle;
            case "flagBuild": return flagBuild;
            case "baseGone": return baseGone;
            case "baseChange": return baseChange;
            case "trackedItems": return trackedItems;
            case "zombie": return zombie;
            case "animal": return animal;
            case "gas": return gas;
            case "crash": return crash;
            case "corpse": return corpse;
            case "heartbeat": return heartbeat;
            case "online": return online;
            case "positions": return positions;
        }
        return false;
    }
}

class ADD_JournalCfg
{
    int flushMs;
    float moveMeters;
    float vehicleMinDamage;
    ref ADD_Streams streams;
    // Empty list means every action or recipe is written.
    ref array<string> actions;
    ref array<string> recipes;

    void ADD_JournalCfg()
    {
        flushMs = 1500;
        moveMeters = 0.5;
        vehicleMinDamage = 2;
        streams = new ADD_Streams;
        actions = new array<string>;
        recipes = new array<string>;
        actions.Insert("ActionBuryBody");
        actions.Insert("ActionStartEngine");
        actions.Insert("ActionRepairCarEngineWithBlowtorch");
        actions.Insert("ActionPlaceObject");
        actions.Insert("ActionDeployObject");
        actions.Insert("ActionPlaceOnGround");
    }
}

class ADD_ReportsCfg
{
    bool enabled;
    int cooldownSec;
    int maxMessageLen;
    float targetRadiusM;
}

class ADD_PositionsCfg
{
    float minMoveMeters;
    int staleSec;
}

class ADD_CombatCfg
{
    float hitMinDamage;
    bool magicBullet;
}

class ADD_ModConfigData
{
    int version;
    bool presenceDetail;
    ref ADD_Intervals intervalsSec;
    ref ADD_PositionsCfg positions;
    ref ADD_SnapshotsCfg snapshots;
    ref ADD_JournalCfg journal;
    ref ADD_CombatCfg combat;
    ref ADD_ReportsCfg reports;
    ref array<string> trackedItemClasses;

    void ADD_ModConfigData()
    {
        intervalsSec = new ADD_Intervals;
        positions = new ADD_PositionsCfg;
        snapshots = new ADD_SnapshotsCfg;
        journal = new ADD_JournalCfg;
        combat = new ADD_CombatCfg;
        reports = new ADD_ReportsCfg;
        trackedItemClasses = new array<string>;
        version = 1;
        presenceDetail = true;
        intervalsSec.heartbeat = 30;
        intervalsSec.online = 30;
        intervalsSec.positions = 5;
        positions.minMoveMeters = 2;
        positions.staleSec = 60;
        snapshots.intervalSec = 60;
        snapshots.changeScanSec = 5;
        combat.hitMinDamage = 0.5;
        combat.magicBullet = true;
        reports.enabled = true;
        reports.cooldownSec = 30;
        reports.maxMessageLen = 500;
        reports.targetRadiusM = 50;
        trackedItemClasses.Insert("TentBase");
        trackedItemClasses.Insert("Barrel_ColorBase");
        trackedItemClasses.Insert("UndergroundStash");
    }

    void Validate()
    {
        if (!intervalsSec)
            intervalsSec = new ADD_Intervals;
        if (!positions)
            positions = new ADD_PositionsCfg;
        if (!snapshots)
            snapshots = new ADD_SnapshotsCfg;
        if (!journal)
            journal = new ADD_JournalCfg;
        if (!journal.streams)
            journal.streams = new ADD_Streams;
        if (!journal.actions)
            journal.actions = new array<string>;
        if (!journal.recipes)
            journal.recipes = new array<string>;
        if (!combat)
            combat = new ADD_CombatCfg;
        if (!reports)
            reports = new ADD_ReportsCfg;
        if (!trackedItemClasses)
            trackedItemClasses = new array<string>;
        if (intervalsSec.heartbeat < 5)
            intervalsSec.heartbeat = 5;
        if (intervalsSec.online < 5)
            intervalsSec.online = 30;
        if (intervalsSec.positions < 1)
            intervalsSec.positions = 1;
        if (snapshots.intervalSec < 10)
            snapshots.intervalSec = 60;
        if (snapshots.changeScanSec < 1)
            snapshots.changeScanSec = 5;
        if (journal.flushMs < 500)
            journal.flushMs = 1500;
        if (journal.moveMeters <= 0)
            journal.moveMeters = 0.5;
        if (journal.vehicleMinDamage < 0)
            journal.vehicleMinDamage = 2;
        if (reports.maxMessageLen < 10)
            reports.maxMessageLen = 500;
        if (reports.targetRadiusM < 1)
            reports.targetRadiusM = 50;
        if (positions.minMoveMeters < 0)
            positions.minMoveMeters = 2;
    }
}
