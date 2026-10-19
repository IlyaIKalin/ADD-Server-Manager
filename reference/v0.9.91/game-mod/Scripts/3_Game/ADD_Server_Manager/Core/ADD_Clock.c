class ADD_Clock
{
    static int UnixMs()
    {
        if (GetGame())
            return GetGame().GetTime();
        return 1;
    }
}
