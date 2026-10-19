modded class RespawnDialogue
{
    override Widget Init()
    {
        Widget root = super.Init();
        if (root)
            GetGame().GetWorkspace().CreateWidgets("ADD_Server_Manager/GUI/layouts/ADD_ReportRespawnButton.layout", root);
        return root;
    }

    override bool OnClick(Widget w, int x, int y, int button)
    {
        if (w && w.GetName() == "ADD_ReportFromDeath")
        {
            ADD_ReportMenu.Send("other", "death", "");
            return true;
        }
        return super.OnClick(w, x, y, button);
    }
}
