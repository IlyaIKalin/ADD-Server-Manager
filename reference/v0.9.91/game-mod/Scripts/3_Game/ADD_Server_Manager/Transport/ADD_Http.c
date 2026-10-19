// Batches are never dropped. The RAM queue holds MEMORY batches; the rest wait in spool files
// on disk, oldest first, and are loaded back as the queue drains. Order is kept: once the spool
// is not empty every new batch goes behind it.
class ADD_Http
{
    protected static ref ADD_Http s_Instance;

    static const int MEMORY = 64;
    static const int REFILL = 32;
    static const int CHUNK = 1000;
    static const int PAUSE_MS = 60000;

    protected ref array<string> m_Queue;
    protected bool m_Busy;
    protected int m_BackoffMs;
    protected int m_NextAt;
    protected int m_Sent;
    protected int m_Failed;
    protected int m_Rejected;
    protected string m_LastError;
    protected int m_SpoolHead;
    protected int m_SpoolTail;
    protected bool m_SpoolReady;

    static ADD_Http Get()
    {
        if (!s_Instance)
            s_Instance = new ADD_Http;
        return s_Instance;
    }

    void ADD_Http()
    {
        m_Queue = new array<string>;
        m_BackoffMs = 1000;
    }

    int QueueDepth()
    {
        return m_Queue.Count() + Spooled();
    }

    int Spooled()
    {
        return m_SpoolTail - m_SpoolHead;
    }

    string Stats()
    {
        string text = "batches sent " + m_Sent.ToString() + ", failed " + m_Failed.ToString() + ", queue " + m_Queue.Count().ToString() + ", spooled " + Spooled().ToString() + ", rejected " + m_Rejected.ToString();
        if (m_LastError != "")
            text = text + ", last error " + m_LastError;
        return text;
    }

    void Enqueue(string body, bool urgent)
    {
        OpenSpool();
        if (Spooled() > 0 || m_Queue.Count() >= MEMORY)
        {
            if (WriteFile(SpoolFile(m_SpoolTail), body))
            {
                m_SpoolTail++;
                return;
            }
            ADD_Log.Warn("http: spool write failed, batch kept in memory");
        }
        m_Queue.Insert(body);
    }

    // SpoolAll moves the memory queue to disk in front of the spool so a restart sends it first.
    void SpoolAll()
    {
        OpenSpool();
        for (int i = m_Queue.Count() - 1; i >= 0; i--)
        {
            if (!WriteFile(SpoolFile(m_SpoolHead - 1), m_Queue.Get(i)))
                return;
            m_SpoolHead--;
            m_Queue.Remove(i);
        }
    }

    void Tick()
    {
        OpenSpool();
        while (m_Queue.Count() < REFILL && Spooled() > 0)
        {
            string path = SpoolFile(m_SpoolHead);
            string spooled = ReadFile(path);
            DeleteFile(path);
            m_SpoolHead++;
            if (spooled != "")
                m_Queue.Insert(spooled);
        }
        if (m_Busy || m_Queue.Count() == 0)
            return;
        // Empty until LoadBootstrap runs, which also fills the default URL.
        if (ADD_Config.Get().Bootstrap.managerUrl == "")
            return;
        if (GetGame().GetTime() < m_NextAt)
            return;
        string body = m_Queue.Get(0);
        m_Queue.Remove(0);
        Send(body);
    }

    protected static string Dir()
    {
        return ADD_ModInfo2.PROFILE_DIR;
    }

    protected static string SpoolFile(int seq)
    {
        return Dir() + "/spool/b" + seq.ToString() + ".json";
    }

    protected void OpenSpool()
    {
        if (m_SpoolReady)
            return;
        m_SpoolReady = true;
        ADD_ModInfo2.EnsureProfileDir();
        MakeDirectory(Dir() + "/spool");
        MakeDirectory(Dir() + "/rejected");
        string name;
        FileAttr attr;
        bool found = false;
        FindFileHandle handle = FindFile(Dir() + "/spool/b*.json", name, attr, FindFileFlags.ALL);
        if (!handle)
            return;
        bool more = true;
        while (more)
        {
            if (name.Length() > 6)
            {
                int seq = name.Substring(1, name.Length() - 6).ToInt();
                if (!found || seq < m_SpoolHead)
                    m_SpoolHead = seq;
                if (!found || seq + 1 > m_SpoolTail)
                    m_SpoolTail = seq + 1;
                found = true;
            }
            more = FindNextFile(handle, name, attr);
        }
        CloseFindFile(handle);
        if (found)
            ADD_Log.Info("http: " + Spooled().ToString() + " spooled batches to resend");
    }

    protected static bool WriteFile(string path, string body)
    {
        FileHandle file = OpenFile(path, FileMode.WRITE);
        if (file == 0)
            return false;
        int length = body.Length();
        for (int i = 0; i < length; i += CHUNK)
        {
            int size = CHUNK;
            if (i + size > length)
                size = length - i;
            FPrintln(file, body.Substring(i, size));
        }
        CloseFile(file);
        return true;
    }

    protected static string ReadFile(string path)
    {
        if (!FileExist(path))
            return "";
        FileHandle file = OpenFile(path, FileMode.READ);
        if (file == 0)
            return "";
        string body = "";
        string line;
        while (FGets(file, line) >= 0)
            body = body + line;
        CloseFile(file);
        return body;
    }

    protected RestContext Open()
    {
        ADD_Bootstrap boot = ADD_Config.Get().Bootstrap;
        if (!boot || boot.managerUrl == "")
        {
            Fail("no managerUrl in bootstrap");
            return null;
        }
        RestApi api = GetRestApi();
        if (!api)
            api = CreateRestApi();
        if (!api)
        {
            Fail("RestApi is not available");
            return null;
        }
        RestContext ctx = api.GetRestContext(boot.managerUrl);
        if (!ctx)
        {
            Fail("no RestContext for " + boot.managerUrl);
            return null;
        }
        // SetHeader only fills the Content-Type value and the engine may drop it on GET,
        // so the token is also sent in the query string.
        ctx.SetHeader("application/json\r\nAuthorization: Bearer " + boot.token);
        return ctx;
    }

    // Path under /ingest/v2/<instance>/ with the token appended.
    protected string Url(string tail)
    {
        ADD_Bootstrap boot = ADD_Config.Get().Bootstrap;
        return "/ingest/v2/" + boot.instanceId + "/" + tail + "?token=" + boot.token;
    }

    protected void Fail(string error)
    {
        m_Failed++;
        if (error != m_LastError)
            ADD_Log.Warn("http: " + error);
        m_LastError = error;
    }

    protected void Recover(string what)
    {
        if (m_LastError != "")
            ADD_Log.Info("http: " + what + " works again after: " + m_LastError);
        m_LastError = "";
    }

    protected void Send(string body)
    {
        RestContext ctx = Open();
        if (!ctx)
        {
            Retry(body, 0);
            return;
        }
        m_Busy = true;
        string result = ctx.POST_now(Url("batch"), body);
        m_Busy = false;
        Handle(result, body);
    }

    protected void Handle(string result, string body)
    {
        if (!result || result == "")
        {
            Fail("batch: manager unreachable (empty response)");
            Retry(body, 0);
            return;
        }
        string code = ADD_Json.ErrorCode(result);
        if (code == "auth" || code == "protocol_version")
        {
            Fail("batch: " + code + " rejected, retrying every minute");
            Retry(body, PAUSE_MS);
            return;
        }
        if (code == "busy" || code == "rate_limited")
        {
            Fail("batch: " + code);
            Retry(body, 0);
            return;
        }
        if (code == "invalid_batch" || code == "too_large")
        {
            m_Rejected++;
            string path = Dir() + "/rejected/" + code + "_" + ADD_Clock.UnixMs().ToString() + ".json";
            WriteFile(path, body);
            Fail("batch " + code + ", saved to " + path);
            m_BackoffMs = 1000;
            return;
        }
        if (code != "")
        {
            Fail("batch: " + code);
            Retry(body, 0);
            return;
        }
        m_Sent++;
        Recover("batch");
        int version = ADD_Json.ConfigVersion(result);
        if (version > 0 && version != ADD_Config.Get().Data.version)
        {
            ADD_Log.Info("config version changed " + ADD_Config.Get().Data.version.ToString() + " -> " + version.ToString());
            FetchConfig();
        }
        m_BackoffMs = 1000;
    }

    // Retry puts the batch back at the head of the queue; delayMs 0 uses the growing backoff.
    protected void Retry(string body, int delayMs)
    {
        m_Queue.InsertAt(body, 0);
        if (delayMs > 0)
        {
            m_NextAt = GetGame().GetTime() + delayMs;
            return;
        }
        m_NextAt = GetGame().GetTime() + m_BackoffMs;
        if (m_BackoffMs == 1000)
            m_BackoffMs = 2000;
        else if (m_BackoffMs == 2000)
            m_BackoffMs = 5000;
        else if (m_BackoffMs == 5000)
            m_BackoffMs = 10000;
        else
            m_BackoffMs = 30000;
    }

    void FetchConfig()
    {
        RestContext ctx = Open();
        if (!ctx)
            return;
        string result = ctx.GET_now(Url("config"));
        if (!result || result == "")
        {
            Fail("config: manager unreachable (empty response)");
            return;
        }
        string code = ADD_Json.ErrorCode(result);
        if (code != "")
        {
            Fail("config: " + code);
            return;
        }
        ADD_Config.Get().ApplyConfigJson(result);
    }

    // GetNow returns the raw body; tail is the path after /ingest/v2/<instance>/.
    string GetNow(string tail)
    {
        RestContext ctx = Open();
        if (!ctx)
            return "";
        string result = ctx.GET_now(Url(tail));
        if (!result || result == "")
            Fail(tail + ": manager unreachable (empty response)");
        return result;
    }

    bool PostNow(string tail, string body)
    {
        RestContext ctx = Open();
        if (!ctx)
            return false;
        string result = ctx.POST_now(Url(tail), body);
        if (!result || result == "")
        {
            Fail(tail + ": manager unreachable (empty response)");
            return false;
        }
        string code = ADD_Json.ErrorCode(result);
        if (code != "")
        {
            Fail(tail + ": " + code);
            return false;
        }
        return true;
    }

    void PostAck(string commandId, bool ok, string error, string resultJson)
    {
        if (!resultJson || resultJson == "")
            resultJson = "{}";
        string flag = "false";
        if (ok)
            flag = "true";
        string body = "{" + ADD_Json.JKey("results") + "[{";
        body = body + ADD_Json.JKey("commandId") + ADD_Json.Quote(commandId);
        body = body + "," + ADD_Json.JKey("ok") + flag;
        body = body + "," + ADD_Json.JKey("error") + ADD_Json.Quote(error);
        body = body + "," + ADD_Json.JKey("result") + resultJson + "}]}";

        if (PostNow("commands/ack", body))
            ADD_Log.Debug("cmd " + commandId + " ack sent");
        else
            ADD_Log.Warn("cmd " + commandId + " ack NOT delivered");
    }
}
