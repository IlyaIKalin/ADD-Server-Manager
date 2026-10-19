modded class MissionGameplay
{
    override void OnUpdate(float timeslice)
    {
        super.OnUpdate(timeslice);
        if (!GetGame() || !GetGame().IsClient() || !GetUApi())
            return;
        UAInput input = GetUApi().GetInputByName("UAADDReport");
        if (input && input.LocalPress())
            ADD_ReportMenu.Open();
    }
}
