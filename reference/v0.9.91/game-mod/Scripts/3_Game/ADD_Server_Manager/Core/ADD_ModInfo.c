class ADD_ModInfo2
{
    static const string NAME = "ADD_Server_Manager";
    static const string VERSION = "0.9.91";
    static const int PROTOCOL = 2;
    // Settings live under profiles\ADDEV\ADD_Server_Manager. MakeDirectory creates one level.
    static const string PROFILE_ROOT = "$profile:ADDEV";
    static const string PROFILE_DIR = "$profile:ADDEV/ADD_Server_Manager";
    static const string SETTINGS_FILE = "ADD_Server_ManagerSetting.json";

    static void EnsureProfileDir()
    {
        if (!FileExist(PROFILE_ROOT))
            MakeDirectory(PROFILE_ROOT);
        if (!FileExist(PROFILE_DIR))
            MakeDirectory(PROFILE_DIR);
    }
}
