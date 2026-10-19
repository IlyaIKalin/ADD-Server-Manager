class ADD_Uuid
{
    static string New()
    {
        int a = Math.RandomInt(0, 2000000000);
        int b = Math.RandomInt(0, 2000000000);
        return a.ToString() + "-" + b.ToString() + "-" + ADD_Clock.UnixMs().ToString();
    }
}
