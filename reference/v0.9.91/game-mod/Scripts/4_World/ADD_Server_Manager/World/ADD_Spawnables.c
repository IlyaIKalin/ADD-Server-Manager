class ADD_Spawnables
{
    protected static int s_Index;
    protected static bool s_Done;

    static void Step()
    {
        if (s_Done)
            return;
        string cfg = "CfgVehicles";
        if (s_Index == 1)
            cfg = "CfgWeapons";
        else if (s_Index == 2)
            cfg = "CfgMagazines";
        else if (s_Index > 2)
        {
            s_Done = true;
            return;
        }
        string category = "item";
        if (s_Index == 1)
            category = "weapon";
        else if (s_Index == 2)
            category = "magazine";
        int count = GetGame().ConfigGetChildrenCount(cfg);
        int limit = count;
        if (limit > 80)
            limit = 80;
        string items = "";
        for (int i = 0; i < limit; i++)
        {
            string name;
            GetGame().ConfigGetChildName(cfg, i, name);
            if (items != "")
                items = items + ",";
            items = items + "{" + ADD_Json.JKey("class") + ADD_Json.Quote(name) + "," + ADD_Json.JKey("category") + ADD_Json.Quote(category) + "}";
        }
        s_Index++;
        if (items != "")
            ADD_Publisher.Get().Publish("spawnables", "{" + ADD_Json.JKey("part") + (s_Index - 1).ToString() + "," + ADD_Json.JKey("parts") + "3," + ADD_Json.JKey("items") + "[" + items + "]}", false);
    }
}
