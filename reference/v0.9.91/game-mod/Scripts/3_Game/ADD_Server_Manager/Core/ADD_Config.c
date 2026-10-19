class ADD_Config
{
    protected static ref ADD_Config s_Instance;
    ref ADD_Bootstrap Bootstrap;
    ref ADD_ModConfigData Data;

    static ADD_Config Get()
    {
        if (!s_Instance)
            s_Instance = new ADD_Config;
        return s_Instance;
    }

    void ADD_Config()
    {
        Bootstrap = new ADD_Bootstrap;
        Data = new ADD_ModConfigData;
    }

    void LoadBootstrap()
    {
        ADD_ModInfo2.EnsureProfileDir();
        string path = ADD_ModInfo2.PROFILE_DIR + "/" + ADD_ModInfo2.SETTINGS_FILE;
        if (!FileExist(path))
            WriteDefaultSettings(path);
        FileHandle handle = OpenFile(path, FileMode.READ);
        if (!handle)
            return;
        string line;
        string text;
        while (FGets(handle, line) >= 0)
            text = text + line;

        CloseFile(handle);
        string error;
        JsonSerializer serializer = new JsonSerializer;
        if (!serializer.ReadFromString(Bootstrap, text, error))
            ADD_Log.Warn(ADD_ModInfo2.SETTINGS_FILE + ": " + error);
        if (Bootstrap.managerUrl == "")
            Bootstrap.managerUrl = "http://127.0.0.1:8082";
        ADD_Log.Info("bootstrap: manager " + Bootstrap.managerUrl + ", instance " + Bootstrap.instanceId + ", token " + Bootstrap.token.Length().ToString() + " chars");
    }

    protected void WriteDefaultSettings(string path)
    {
        Bootstrap.managerUrl = "http://127.0.0.1:8082";
        Bootstrap.instanceId = "";
        Bootstrap.token = "";
        FileHandle handle = OpenFile(path, FileMode.WRITE);
        if (!handle)
        {
            ADD_Log.Warn(ADD_ModInfo2.SETTINGS_FILE + " write failed");
            return;
        }
        string quote = "\"";
        string empty = quote + quote;
        FPrintln(handle, "{");
        FPrintln(handle, "  " + quote + "managerUrl" + quote + ": " + quote + Bootstrap.managerUrl + quote + ",");
        FPrintln(handle, "  " + quote + "instanceId" + quote + ": " + empty + ",");
        FPrintln(handle, "  " + quote + "token" + quote + ": " + empty);
        FPrintln(handle, "}");
        CloseFile(handle);
        ADD_Log.Info(ADD_ModInfo2.SETTINGS_FILE + " created");
    }

    bool ApplyConfigJson(string body)
    {
        if (!body || body == "")
            return false;
        string error;
        JsonSerializer serializer = new JsonSerializer;
        ADD_ConfigEnvelope envelope = new ADD_ConfigEnvelope;
        if (!serializer.ReadFromString(envelope, body, error) || !envelope.config)
        {
            ADD_Log.Warn("config: " + error);
            return false;
        }
        if (envelope.config.version <= 0)
            return false;
        envelope.config.Validate();
        Data = envelope.config;
        ADD_Log.Info("mod config version " + Data.version.ToString() + ", flush " + Data.journal.flushMs.ToString() + " ms, snapshots every " + Data.snapshots.intervalSec.ToString() + " s");
        return true;
    }

    bool On(string stream)
    {
        if (!Data || !Data.journal || !Data.journal.streams)
            return true;
        return Data.journal.streams.On(stream);
    }
}
