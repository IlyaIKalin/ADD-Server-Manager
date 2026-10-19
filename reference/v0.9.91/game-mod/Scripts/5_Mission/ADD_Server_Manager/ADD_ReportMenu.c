class ADD_ReportMenu extends UIScriptedMenu
{
    protected string m_Category;
    protected EditBoxWidget m_Message;

    void ADD_ReportMenu()
    {
        m_Category = "other";
    }

    override Widget Init()
    {
        layoutRoot = GetGame().GetWorkspace().CreateWidgets("ADD_Server_Manager/GUI/layouts/ADD_ReportMenu.layout");
        m_Message = EditBoxWidget.Cast(layoutRoot.FindAnyWidget("MessageBox"));
        return layoutRoot;
    }

    override bool OnClick(Widget w, int x, int y, int button)
    {
        if (!w)
            return false;
        string name = w.GetName();
        if (name == "BtnCheat")
            m_Category = "cheat";
        else if (name == "BtnOther")
            m_Category = "other";
        else if (name == "BtnSend")
        {
            string message = "";
            if (m_Message)
                message = m_Message.GetText();
            Send(m_Category, message, "");
            Close();
        }
        else if (name == "BtnClose")
            Close();
        else
            return false;
        return true;
    }

    static void Open()
    {
        GetGame().GetUIManager().ShowScriptedMenu(new ADD_ReportMenu, null);
    }

    static void Send(string category, string message, string targetSteam)
    {
        PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
        if (!player)
            return;
        if (!message || message == "")
            message = "report";
        GetGame().RPCSingleParam(player, ADD_ReportRpc.RPC_REPORT, new Param3<string, string, string>(category, message, targetSteam), true, player.GetIdentity());
    }
}
