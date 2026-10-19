class ADD_Heartbeat
{
    protected static ref ADD_Heartbeat s_Instance;

    static ADD_Heartbeat Get()
    {
        if (!s_Instance)
            s_Instance = new ADD_Heartbeat;
        return s_Instance;
    }

    void Start()
    {
        int sec = ADD_Config.Get().Data.intervalsSec.heartbeat;
        if (sec < 5)
            sec = 30;
        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(this.Beat, sec * 1000, true);
    }

    void Beat()
    {
        float fpsMin, fpsAvg, fpsMax;
        ADD_Ticker.TakeFps(fpsMin, fpsAvg, fpsMax);
        Weather weather = GetGame().GetWeather();
        int year, month, day, hour, minute;
        GetGame().GetWorld().GetDate(year, month, day, hour, minute);
        string counts = ADD_EntityRegistry.Get().CountsJson();
        string data = "{" + ADD_Json.JKey("fpsMin") + ADD_Json.Num(fpsMin);
        data = data + "," + ADD_Json.JKey("fpsAvg") + ADD_Json.Num(fpsAvg);
        data = data + "," + ADD_Json.JKey("fpsMax") + ADD_Json.Num(fpsMax);
        string weatherJson = "{" + ADD_Json.JKey("overcast") + ADD_Json.Num(weather.GetOvercast().GetActual());
        weatherJson = weatherJson + "," + ADD_Json.JKey("fog") + ADD_Json.Num(weather.GetFog().GetActual());
        weatherJson = weatherJson + "," + ADD_Json.JKey("rain") + ADD_Json.Num(weather.GetRain().GetActual());
        weatherJson = weatherJson + "," + ADD_Json.JKey("wind") + "0," + ADD_Json.JKey("snowfall") + "0}";
        data = data + "," + ADD_Json.JKey("weather") + weatherJson;
        string timeJson = "{" + ADD_Json.JKey("year") + year.ToString();
        timeJson = timeJson + "," + ADD_Json.JKey("month") + month.ToString();
        timeJson = timeJson + "," + ADD_Json.JKey("day") + day.ToString();
        timeJson = timeJson + "," + ADD_Json.JKey("hour") + hour.ToString();
        timeJson = timeJson + "," + ADD_Json.JKey("minute") + minute.ToString() + "}";
        data = data + "," + ADD_Json.JKey("time") + timeJson;
        data = data + "," + ADD_Json.JKey("players") + PlayerCount().ToString() + "," + ADD_Json.JKey("counts") + counts + "," + ADD_Json.JKey("configVersion") + ADD_Config.Get().Data.version.ToString();
        data = data + "," + ADD_Json.JKey("uptimeSec") + (GetGame().GetTime() / 1000).ToString() + "," + ADD_Json.JKey("queueDepth") + ADD_Http.Get().QueueDepth().ToString() + "}";
        ADD_Publisher.Get().Publish("heartbeat", data, false);
    }

    protected int PlayerCount()
    {
        array<Man> players = new array<Man>;
        GetGame().GetPlayers(players);
        return players.Count();
    }
}

