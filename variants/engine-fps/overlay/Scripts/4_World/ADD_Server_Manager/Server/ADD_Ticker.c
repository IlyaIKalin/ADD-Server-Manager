class ADD_Ticker
{
    protected static int s_Step;

    static void OnUpdate(float timeslice)
    {
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

    static void TakeFps(out float fpsMin, out float fpsAvg, out float fpsMax)
    {
        fpsMin = 0;
        fpsAvg = 0;
        fpsMax = 0;
#ifdef SERVER
        ADD_EngineFps.TakeFps(fpsMin, fpsAvg, fpsMax);
#endif
    }
}
