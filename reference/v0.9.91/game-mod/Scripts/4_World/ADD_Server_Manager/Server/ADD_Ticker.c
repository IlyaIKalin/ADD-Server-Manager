class ADD_Ticker
{
    // timeslice is quantized to milliseconds, so FPS is counted as frames over elapsed game time.
    protected static int s_PeriodStart = -1;
    protected static int s_PeriodFrames;
    protected static int s_WindowStart = -1;
    protected static int s_WindowFrames;
    protected static float s_FpsMin = -1;
    protected static float s_FpsMax;
    protected static int s_Step;

    static void OnUpdate(float timeslice)
    {
        CountFrame();
        ADD_Publisher.Get().Tick();
        if (s_Step == 0)
            ADD_Http.Get().Tick();
        else if (s_Step == 1)
            ADD_Spawnables.Step();
        else if (s_Step == 2)
            ADD_EntitySnapshots.Step();
        else
            ADD_EntityChanges.Get().Step();
        s_Step++;
        if (s_Step > 3)
            s_Step = 0;
    }

    protected static void CountFrame()
    {
        int now = GetGame().GetTime();
        if (s_PeriodStart < 0)
        {
            s_PeriodStart = now;
            s_WindowStart = now;
            return;
        }
        s_PeriodFrames++;
        s_WindowFrames++;
        int span = now - s_WindowStart;
        if (span < 1000)
            return;
        float fps = s_WindowFrames * 1000.0 / span;
        if (s_FpsMin < 0 || fps < s_FpsMin)
            s_FpsMin = fps;
        if (fps > s_FpsMax)
            s_FpsMax = fps;
        s_WindowStart = now;
        s_WindowFrames = 0;
    }

    static void TakeFps(out float fpsMin, out float fpsAvg, out float fpsMax)
    {
        int now = GetGame().GetTime();
        fpsAvg = 0;
        int span = now - s_PeriodStart;
        if (s_PeriodStart >= 0 && span > 0)
            fpsAvg = s_PeriodFrames * 1000.0 / span;
        fpsMin = s_FpsMin;
        if (fpsMin < 0)
            fpsMin = fpsAvg;
        fpsMax = s_FpsMax;
        if (fpsMax <= 0)
            fpsMax = fpsAvg;
        s_PeriodStart = now;
        s_PeriodFrames = 0;
        s_FpsMin = -1;
        s_FpsMax = 0;
    }
}
