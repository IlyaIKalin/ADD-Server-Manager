#ifdef SERVER
// Engine callbacks, measured outside Expansion's mission/script limiter.
class ADD_EngineFps
{
    protected static float s_PeriodStart = -1;
    protected static int s_PeriodFrames;
    protected static float s_WindowStart = -1;
    protected static int s_WindowFrames;
    protected static float s_FpsMin = -1;
    protected static float s_FpsMax;

    static void CountFrame()
    {
        float now = GetTickTime();
        if (s_PeriodStart < 0)
        {
            s_PeriodStart = now;
            s_WindowStart = now;
            return;
        }
        s_PeriodFrames++;
        s_WindowFrames++;
        float span = now - s_WindowStart;
        if (span < 1.0)
            return;
        float fps = s_WindowFrames / span;
        if (s_FpsMin < 0 || fps < s_FpsMin)
            s_FpsMin = fps;
        if (fps > s_FpsMax)
            s_FpsMax = fps;
        s_WindowStart = now;
        s_WindowFrames = 0;
    }

    static void TakeFps(out float fpsMin, out float fpsAvg, out float fpsMax)
    {
        float now = GetTickTime();
        fpsAvg = 0;
        float span = now - s_PeriodStart;
        if (s_PeriodStart >= 0 && span > 0)
            fpsAvg = s_PeriodFrames / span;
        fpsMin = s_FpsMin;
        if (fpsMin < 0)
            fpsMin = fpsAvg;
        fpsMax = s_FpsMax;
        if (fpsMax <= 0)
            fpsMax = fpsAvg;

        // Do not carry a partial one-second window into the next heartbeat.
        s_PeriodStart = now;
        s_PeriodFrames = 0;
        s_WindowStart = now;
        s_WindowFrames = 0;
        s_FpsMin = -1;
        s_FpsMax = 0;
    }
}

modded class DayZGame
{
    override void OnUpdate(bool doSim, float timeslice)
    {
        // Count before super: heartbeat may read the counters inside super.
        ADD_EngineFps.CountFrame();
        super.OnUpdate(doSim, timeslice);
    }
}
#endif
