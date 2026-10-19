// Info and Warn go to the RPT and to profiles\ADDEV\ADD_Server_Manager\add.log; Debug only to the file.
// The RPT is buffered by the engine, add.log is closed after every line so it can be read live.
class ADD_Log
{
    protected static string FilePath()
    {
        return ADD_ModInfo2.PROFILE_DIR + "/add.log";
    }

    protected static string PrevPath()
    {
        return ADD_ModInfo2.PROFILE_DIR + "/add.prev.log";
    }

    protected static bool s_Ready;

    static void Info(string message)
    {
        Print("[ADD] " + message);
        Write("INFO", message);
    }

    static void Warn(string message)
    {
        Print("[ADD][warn] " + message);
        Write("WARN", message);
    }

    static void Debug(string message)
    {
        Write("DEBUG", message);
    }

    protected static void Write(string level, string message)
    {
        if (!s_Ready)
            Open();
        FileHandle handle = OpenFile(FilePath(), FileMode.APPEND);
        if (!handle)
            return;
        FPrintln(handle, Stamp() + " " + level + " " + message);
        CloseFile(handle);
    }

    protected static void Open()
    {
        s_Ready = true;
        ADD_ModInfo2.EnsureProfileDir();
        if (FileExist(FilePath()))
        {
            if (FileExist(PrevPath()))
                DeleteFile(PrevPath());
            CopyFile(FilePath(), PrevPath());
        }
        FileHandle handle = OpenFile(FilePath(), FileMode.WRITE);
        if (handle)
            CloseFile(handle);
    }

    protected static string Stamp()
    {
        int year, month, day, hour, minute, second;
        GetYearMonthDay(year, month, day);
        GetHourMinuteSecond(hour, minute, second);
        return year.ToString() + "-" + month.ToStringLen(2) + "-" + day.ToStringLen(2) + " " + hour.ToStringLen(2) + ":" + minute.ToStringLen(2) + ":" + second.ToStringLen(2);
    }
}
