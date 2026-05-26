modded class PS_CharacterSelector
{
	override void OnClicked(SCR_ButtonBaseComponent button)
	{
		if (m_bStateClickSkip)
		{
			m_bStateClickSkip = false;
			return;
		}

		int playerId = m_CoopLobby.GetSelectedPlayer();
		if (m_iPlayerId == -2)
		{
			m_CoopLobby.SetPreviewPlayable(m_iPlayableId, true);
			AudioSystem.PlaySound("{C97850E4341F0CF9}Sounds/UI/Samples/Menu/UI_Button_Fail.wav");
			return;
		}
		if (m_iPlayerId > 0 && playerId != m_iPlayerId)
		{
			m_CoopLobby.SetPreviewPlayable(m_iPlayableId, true);
			AudioSystem.PlaySound("{C97850E4341F0CF9}Sounds/UI/Samples/Menu/UI_Button_Fail.wav");
			return;
		}
		PS_PlayableContainer playableContainer = m_PlayableManager.GetPlayableById(m_iPlayableId);
		if (playableContainer.GetDamageState() == EDamageState.DESTROYED)
		{
			m_CoopLobby.SetPreviewPlayable(m_iPlayableId, true);
			AudioSystem.PlaySound("{C97850E4341F0CF9}Sounds/UI/Samples/Menu/UI_Button_Fail.wav");
			return;
		}

		SCR_EGameModeState gameState = m_GameModeCoop.GetState();
		if (!PS_PlayersHelper.IsAdminOrServer())
		{
			RplId playableId = m_PlayableManager.GetPlayableByPlayer(m_iCurrentPlayerId);
			if (gameState == SCR_EGameModeState.BRIEFING && playableId != RplId.Invalid())
			{
				m_CoopLobby.SetPreviewPlayable(m_iPlayableId, true);
				return;
			}
		}

		if (playerId != m_iPlayerId)
		{
			PS_GameModeQuickTvT gameModeQuickTvT = PS_GameModeQuickTvT.Cast(GetGame().GetGameMode());
			Print(string.Format("[DefendFlag] OnClicked - m_sFactionKey=%1, m_iCurrentPlayerId=%2, gameModeQuickTvT=%3", m_sFactionKey, m_iCurrentPlayerId, gameModeQuickTvT));
			if (gameModeQuickTvT)
			{
				PS_PlayableControllerComponent localPc = PS_PlayableControllerComponent.Cast(GetGame().GetPlayerController().FindComponent(PS_PlayableControllerComponent));
				Print(string.Format("[DefendFlag] OnClicked - m_bIsDefendFlagged=%1", localPc.m_bIsDefendFlagged));
			}
			if (gameModeQuickTvT && gameModeQuickTvT.IsDefendFactionRestrictedForLocal(m_sFactionKey) && !gameModeQuickTvT.IsSkirmish())
			{
				SCR_ChatPanelManager chatPanelManager = SCR_ChatPanelManager.GetInstance();
				ChatCommandInvoker invoker = chatPanelManager.GetCommandInvoker("lmsg");
				invoker.Invoke(null, "Уже играли за оборону - подождите минуту");
				m_CoopLobby.SetPreviewPlayable(m_iPlayableId, true);
				AudioSystem.PlaySound("{C97850E4341F0CF9}Sounds/UI/Samples/Menu/UI_Button_Fail.wav");
				return;
			}
			//RplId playableId = m_PlayableManager.GetPlayableByPlayer(m_iPlayerId);
			/*
			SCR_GroupsManagerComponent groupsManagerComponent = SCR_GroupsManagerComponent.GetInstance();
			SCR_AIGroup playerGroup =  groupsManagerComponent.FindGroup(m_playablePlayerGroupId[m_iPlayerId]);
			SCR_ChimeraCharacter leaderCharacter = null;
			if (playerGroup)
				leaderCharacter = SCR_ChimeraCharacter.Cast(playerGroup.GetLeaderEntity());
			Print("[l leaderCharacter " + leaderCharacter);*/
			if (!CanJoinFaction())
			{

				SCR_ChatPanelManager chatPanelManager = SCR_ChatPanelManager.GetInstance();
				ChatCommandInvoker invoker = chatPanelManager.GetCommandInvoker("lmsg");
				invoker.Invoke(null, "Где баланс?");
				SCR_ChatPanelManager.GetInstance().ShowHelpMessage("Соблюдайте баланс сторон");
				m_CoopLobby.SetPreviewPlayable(m_iPlayableId, true);
				return;
			}

			AudioSystem.PlaySound("{9500A96BBA3B0581}Sounds/UI/Samples/Menu/UI_Gadget_Select.wav");
			m_PlayableControllerComponent.MoveToVoNRoom(playerId, m_sFactionKey, m_sPlayableCallsign);
			m_PlayableControllerComponent.ChangeFactionKey(playerId, m_sFactionKey);
			m_PlayableControllerComponent.SetPlayerState(playerId, PS_EPlayableControllerState.NotReady);
			m_PlayableControllerComponent.SetPlayerPlayable(playerId, m_iPlayableId);
		} else {
			AudioSystem.PlaySound("{9500A96BBA3B0581}Sounds/UI/Samples/Menu/UI_Gadget_Select.wav");
			m_PlayableControllerComponent.MoveToVoNRoom(playerId, m_sFactionKey, "#PS-VoNRoom_Faction");
			m_PlayableControllerComponent.ChangeFactionKey(playerId, "");
			m_PlayableControllerComponent.SetPlayerState(playerId, PS_EPlayableControllerState.NotReady);
			m_PlayableControllerComponent.SetPlayerPlayable(playerId, RplId.Invalid());
			if (PS_PlayersHelper.IsAdminOrServer())
				m_PlayableControllerComponent.UnpinPlayer(playerId);
		}

		if (PS_PlayersHelper.IsAdminOrServer() && playerId != m_iCurrentPlayerId && gameState == SCR_EGameModeState.GAME)
			m_PlayableControllerComponent.ForceSwitch(playerId);
		if (!PS_PlayersHelper.IsAdminOrServer() && playerId == m_iCurrentPlayerId && gameState == SCR_EGameModeState.BRIEFING)
			m_PlayableControllerComponent.SwitchToMenuServer(SCR_EGameModeState.BRIEFING);
	}

};


/*modded class PS_ContextMenu : SCR_ScriptedWidgetComponent
{
	override void ActionKick(int playerId)
	{
		if (!PS_PlayersHelper.IsAdminOrServer())
			return;
		if (GetGame().GetPlayerController().GetPlayerId() == playerId)
			return;
		
		PS_PlayableControllerComponent localPc = PS_PlayableControllerComponent.Cast(GetGame().GetPlayerController().FindComponent(PS_PlayableControllerComponent));
		Print(string.Format("[DefendFlag] Onkick - m_bIsDefendFlagged=%1", localPc.m_bIsDefendFlagged));
		if (localPc.m_bIsDefendFlagged)
			return;
		string name = "#PS-ContextAction_Kick";
		return AddAction(IMAGESET, "kickCommandAlt", name, "",
			new PS_ContextActionDataPlayer(playerId)
		).GetOnOnContextAction().Insert(OnActionKick);
	}
	override void OnActionKick(PS_ContextAction contextAction, PS_ContextActionDataPlayer contextActionDataPlayer)
	{
		PS_PlayableControllerComponent localPc = PS_PlayableControllerComponent.Cast(GetGame().GetPlayerController().FindComponent(PS_PlayableControllerComponent));
		Print(string.Format("[DefendFlag] Onkick - m_bIsDefendFlagged=%1", localPc.m_bIsDefendFlagged));
		if (!localPc.m_bIsDefendFlagged)
		{
			SCR_UISoundEntity.SoundEvent("SOUND_LOBBY_KICK");
			PS_PlayableManager.GetPlayableController().KickPlayer(contextActionDataPlayer.GetPlayerId());
		}
	}
	
	PS_ScriptInvokerOnContextAction ActionFreeSlot(RplId playableId)
	{
		PS_PlayableControllerComponent localPc = PS_PlayableControllerComponent.Cast(GetGame().GetPlayerController().FindComponent(PS_PlayableControllerComponent));
		Print(string.Format("[DefendFlag] Onkick - m_bIsDefendFlagged=%1", localPc.m_bIsDefendFlagged));
		if (localPc.m_bIsDefendFlagged)
		{
			return AddAction(IMAGESET, "kickCommandAlt", "#PS-ContextAction_FreeSlot", "",
				new PS_ContextActionDataPlayable(playableId)
			).GetOnOnContextAction();
		}
	}*/
/*modded class PS_CharacterSelector : SCR_ButtonComponent
{
	override void OpenContext()
	{
		string playerName = PS_PlayableManager.GetInstance().GetPlayerName(m_iPlayerId);
		PS_ContextMenu contextMenu = PS_ContextMenu.CreateContextMenuOnMousePosition(m_CoopLobby.GetRootWidget(), playerName);
		contextMenu.ActionOpenInventory(m_iPlayableId).Insert(OnActionOpenInventory);
		
		if (m_iPlayerId > 0)
		{
			if (PS_PlayersHelper.IsAdminOrServer())
			{
				contextMenu.ActionGetArmaId(m_iPlayerId);
			}
			if (m_iPlayerId != m_iCurrentPlayerId)
			{
				PermissionState mute = PermissionState.DISALLOWED;
				SocialComponent socialComp = SocialComponent.Cast(GetGame().GetPlayerController().FindComponent(SocialComponent));
				if (socialComp.IsMuted(m_iPlayerId))
					contextMenu.ActionUnmute(m_iPlayerId);
				else
					contextMenu.ActionMute(m_iPlayerId);
				
				PS_PlayableControllerComponent localPc = PS_PlayableControllerComponent.Cast(GetGame().GetPlayerController().FindComponent(PS_PlayableControllerComponent));
				Print(string.Format("[DefendFlag] Onkick - m_sFlaggedDefendFactionKey=%1 m_sFactionKey=%2", localPc.m_sFlaggedDefendFactionKey, m_sFactionKey));
				if (m_bCanKick && m_iPlayerId >= 0 && m_iPlayerId != m_iCurrentPlayerId && m_sFactionKey != localPc.m_sFlaggedDefendFactionKey)
					contextMenu.ActionFreeSlot(m_iPlayableId).Insert(OnActionFreeSlot);
				
				if (PS_PlayersHelper.IsAdminOrServer())
				{
					contextMenu.ActionDirectMessage(m_iPlayerId);
					contextMenu.ActionKick(m_iPlayerId);
					
					if (m_PlayableManager.GetPlayerPin(m_iPlayerId))
						contextMenu.ActionUnpin(m_iPlayerId);
					else
						contextMenu.ActionPin(m_iPlayerId);
				}
			}
			if (m_CoopLobby.GetSelectedPlayer() != m_iPlayerId && PS_PlayersHelper.IsAdminOrServer())
			{
				contextMenu.ActionPlayerSelect(m_iPlayerId);
			}
		}
		
		if (PS_PlayersHelper.IsAdminOrServer())
			if (m_iPlayerId != -2)
				contextMenu.ActionLock(m_iPlayableId).Insert(OnActionLock);
			else
				contextMenu.ActionUnlock(m_iPlayableId).Insert(OnActionUnlock);
	}
}*/