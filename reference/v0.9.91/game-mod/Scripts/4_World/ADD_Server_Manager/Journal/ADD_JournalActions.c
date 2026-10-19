class ADD_JournalActions
{
    static void Done(ActionData data)
    {
        if (!data || !data.m_Action || !data.m_Player || !ADD_Config.Get().On("action"))
            return;
        string name = data.m_Action.ClassName();
        if (!ADD_Journal.Listed(ADD_Config.Get().Data.journal.actions, name))
            return;
        Object target;
        if (data.m_Target)
            target = data.m_Target.GetObject();
        EntityAI entity = EntityAI.Cast(target);
        string what;
        if (target)
            what = target.GetType();
        if (!entity)
            entity = data.m_MainItem;
        ADD_Journal.Write("action", name, data.m_Player, entity, data.m_Player.GetPosition(), what, "", 0);
    }

    static void Recipe(RecipeBase recipe, string first, string second, PlayerBase player)
    {
        if (!player || !ADD_Config.Get().On("recipe"))
            return;
        string name = recipe.ClassName();
        if (!ADD_Journal.Listed(ADD_Config.Get().Data.journal.recipes, name))
            return;
        ADD_Journal.Write("recipe", name, player, null, player.GetPosition(), first, second, 0);
    }

    static void Vehicle(PlayerBase player, Transport vehicle, string act)
    {
        if (!player || !vehicle)
            return;
        string cat = "carUse";
        if (vehicle.IsInherited(BoatScript))
            cat = "boatUse";
        ADD_Journal.Write(cat, act, player, vehicle, vehicle.GetPosition(), "", "", 0);
        ADD_EntityChanges.Get().Touch(vehicle);
    }
}

modded class ActionContinuousBase
{
    override void OnFinishProgress(ActionData action_data)
    {
        super.OnFinishProgress(action_data);
        if (GetGame().IsServer())
            ADD_JournalActions.Done(action_data);
    }
}

modded class RecipeBase
{
    override void PerformRecipe(ItemBase item1, ItemBase item2, PlayerBase player)
    {
        bool log = GetGame().IsServer() && item1 && item2 && player && CheckRecipe(item1, item2, player);
        string first;
        string second;
        if (log)
        {
            first = item1.GetType();
            second = item2.GetType();
        }
        super.PerformRecipe(item1, item2, player);
        if (log)
            ADD_JournalActions.Recipe(this, first, second, player);
    }
}

modded class ActionGetInTransport
{
    override void OnEndServer(ActionData action_data)
    {
        super.OnEndServer(action_data);
        if (!action_data || !action_data.m_Player)
            return;
        Transport vehicle;
        if (action_data.m_Target)
            vehicle = Transport.Cast(action_data.m_Target.GetObject());
        if (!vehicle && action_data.m_Player.GetCommand_Vehicle())
            vehicle = action_data.m_Player.GetCommand_Vehicle().GetTransport();
        ADD_JournalActions.Vehicle(action_data.m_Player, vehicle, "in");
    }
}

modded class ActionGetOutTransport
{
    override void OnEndServer(ActionData action_data)
    {
        GetOutTransportActionData data = GetOutTransportActionData.Cast(action_data);
        Transport vehicle;
        bool jumped;
        if (data)
        {
            vehicle = Transport.Cast(data.m_Vehicle);
            jumped = data.m_WasJumpingOut;
        }
        super.OnEndServer(action_data);
        if (!action_data)
            return;
        string act = "out";
        if (jumped)
            act = "jump";
        ADD_JournalActions.Vehicle(action_data.m_Player, vehicle, act);
    }
}
