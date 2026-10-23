class CfgPatches
{
    class ADD_Server_Manager_FPS
    {
        units[] = {};
        weapons[] = {};
        requiredVersion = 0.1;
        // Place our DayZGame override outside Expansion's preload limiter.
        requiredAddons[] = {"ADD_Server_Manager", "DayZExpansion_Core_Scripts"};
    };
};

class CfgMods
{
    class ADD_Server_Manager_FPS
    {
        type = "mod";
        dependencies[] = {"Game"};
        class defs
        {
            class gameScriptModule
            {
                value = "";
                files[] = {"ADD_Server_Manager_FPS/Scripts/3_Game"};
            };
        };
    };
};
