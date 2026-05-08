enum PS_ETimerCommand
{
	STOP,
	START,
	ADD,
	MINUS
};

class PS_DefendFlagJsonData
{
	ref array<string> m_aFlaggedPlayerUIDs = {};
}

class PS_GameModeQuickTvTClass: PS_GameModeCoopClass
{
};

class PS_GameModeQuickTvT : PS_GameModeCoop
{
	static const string m_QuickTvTConfigFilePath = "$profile:PS_QuickTvT_Config.json";
	protected static ref PS_QuickTvTMissionsConfig m_QuickTvTMissionsConfig;

	static const string DEFEND_FLAG_FILE = "$profile:QTvT_DefendFlags.json";

	[Attribute("15000", UIWidgets.EditBox, "", "", category: "Reforger Lobby")]
	int m_iPreviewTime;

	[Attribute("30000", UIWidgets.EditBox, "", "", category: "Reforger Lobby")]
	int m_iSlotsTime;

	[Attribute("30000", UIWidgets.EditBox, "", "", category: "Reforger Lobby")]
	int m_iBriefingTime;

	[Attribute("480000", UIWidgets.EditBox, "", "", category: "Reforger Lobby")]
	int m_iGameTime;

	[Attribute("12000", UIWidgets.EditBox, "", "", category: "Reforger Lobby")]
	int m_iDebriefingTime;

	[RplProp()]
	protected int m_iStepTime;

	protected ref map<string, bool> m_mDefendFlagPlayers = new map<string, bool>();

	protected static int m_iMissionNum = 0;

	protected PlayerManager m_PlayerManager;

	protected bool m_bTimerEnabled = true;

	int GetStepTime()
	{
		return m_iStepTime;
	}

	override void OnGameStart()
	{
		super.OnGameStart();
		GetGame().GetCallqueue().CallLater(AddCommands, 0, false);

		if (!m_QuickTvTMissionsConfig)
		{
			m_QuickTvTMissionsConfig = new PS_QuickTvTMissionsConfig();
			SCR_JsonLoadContext configLoadContext = new SCR_JsonLoadContext();
			configLoadContext.LoadFromFile(m_QuickTvTConfigFilePath);
			configLoadContext.ReadValue("", m_QuickTvTMissionsConfig);

			m_QuickTvTMissionsConfig.SortRandom();
		}

		m_PlayerManager = GetGame().GetPlayerManager();

		if (m_iGameTime == 0)
		{
			m_iMissionNum = 0;
		}

		m_iStepTime = m_iPreviewTime;

		LoadDefendFlags();
	}

	override void EOnFrame(IEntity owner, float timeSlice)
	{
		if (!Replication.IsServer())
			return;

		if (m_bTimerEnabled && m_iStepTime > 0)
		{
			int playersCount = m_PlayerManager.GetPlayerCount();
			if (GetState() != SCR_EGameModeState.PREGAME || playersCount > 1)
				m_iStepTime -= timeSlice * 1000;

			if (m_iStepTime <= 0)
				AdvanceGameState(GetState());

			Replication.BumpMe();
		}
	}

	override void OnGameStateChanged()
	{
		super.OnGameStateChanged();

		SCR_EGameModeState state = GetState();
		switch (state)
		{
		case SCR_EGameModeState.PREGAME:
			m_iStepTime = m_iPreviewTime;
			break;
		case SCR_EGameModeState.SLOTSELECTION:
			m_iStepTime = m_iSlotsTime;
			break;
		case SCR_EGameModeState.CUTSCENE:
			break;
		case SCR_EGameModeState.BRIEFING:
			m_iStepTime = m_iBriefingTime;
			break;
		case SCR_EGameModeState.GAME:
			m_iStepTime = m_iGameTime + m_iFreezeTime;
			if (Replication.IsServer())
			{
				GetGame().GetCallqueue().CallLater(FlagDefendFactionPlayers, 3000, false);
			}
			break;
		case SCR_EGameModeState.DEBRIEFING:
			m_iStepTime = m_iDebriefingTime;
			break;
		case SCR_EGameModeState.POSTGAME:
			break;
		}
	}

	protected void LoadDefendFlags()
	{
		if (!Replication.IsServer())
			return;

		m_mDefendFlagPlayers.Clear();
		PS_DefendFlagJsonData data = new PS_DefendFlagJsonData();
		SCR_JsonLoadContext loadContext = new SCR_JsonLoadContext();
		if (loadContext.LoadFromFile(DEFEND_FLAG_FILE))
		{
			loadContext.ReadValue("", data);
			if (data && data.m_aFlaggedPlayerUIDs)
			{
				foreach (string uid : data.m_aFlaggedPlayerUIDs)
				{
					m_mDefendFlagPlayers[uid] = true;
				}
				Print(string.Format("[DefendFlag] Loaded %1 flagged UIDs from file", data.m_aFlaggedPlayerUIDs.Count()));
			}
		}
		else
		{
			Print("[DefendFlag] No saved flag file found, starting fresh");
		}
	}

	void FlagDefendFactionPlayers()
	{
		if (!Replication.IsServer())
			return;

		GUB_RandomizeSpawnManager spawnManager = GetRandomizeSpawnManager();
		Print(string.Format("[DefendFlag] FlagDefendFactionPlayers - spawnManager: %1, defendFaction: %2", spawnManager, spawnManager.GetDefendFaction()));
		if (!spawnManager || !spawnManager.GetDefendFaction())
			return;

		FactionKey defendFactionKey = spawnManager.GetDefendFaction().m_sFactionKey;
		array<int> playerIds = {};
		m_PlayerManager.GetPlayers(playerIds);
		Print(string.Format("[DefendFlag] FlagDefendFactionPlayers - defendFactionKey: %1, playerCount: %2", defendFactionKey, playerIds.Count()));
		foreach (int pId : playerIds)
		{
			FactionKey playerFaction = m_playableManager.GetPlayerFactionKey(pId);
			string uid = GetGame().GetBackendApi().GetPlayerUID(pId);
			Print(string.Format("[DefendFlag] Player %1 UID: %2 faction: %3 (defend: %4)", pId, uid, playerFaction, defendFactionKey));
			if (playerFaction == defendFactionKey && uid && !m_mDefendFlagPlayers.Contains(uid))
			{
				m_mDefendFlagPlayers[uid] = true;
				Print(string.Format("[DefendFlag] Flagged player UID: %1", uid));
			}
		}
		Print(string.Format("[DefendFlag] Total flagged players: %1", m_mDefendFlagPlayers.Count()));
		SaveDefendFlags();
	}

	protected void SaveDefendFlags()
	{
		if (!Replication.IsServer())
			return;

		PS_DefendFlagJsonData data = new PS_DefendFlagJsonData();
		foreach (string uid, bool _ : m_mDefendFlagPlayers)
		{
			data.m_aFlaggedPlayerUIDs.Insert(uid);
		}
		SCR_JsonSaveContext saveContext = new SCR_JsonSaveContext();
		saveContext.WriteValue("", data);
		saveContext.SaveToFile(DEFEND_FLAG_FILE);
	}

	bool IsPlayerDefendFlagged(string playerUID)
	{
		return m_mDefendFlagPlayers.Contains(playerUID);
	}

	bool IsDefendFactionRestrictedForLocal(FactionKey factionKeyPlayer)
	{
		if (GetState() != SCR_EGameModeState.SLOTSELECTION)
		{
			Print(string.Format("[DefendFlag] IsRestricted - state=%1 (not SLOTSELECTION), returning false", GetState()));
			return false;
		}
		PS_PlayableControllerComponent playableController = PS_PlayableControllerComponent.Cast(GetGame().GetPlayerController().FindComponent(PS_PlayableControllerComponent));
		if (!playableController)
		{
			Print("[DefendFlag] IsRestricted - no playableController, returning false");
			return false;
		}
		if (!playableController.m_bIsDefendFlagged)
		{
			Print("[DefendFlag] IsRestricted - m_bIsDefendFlagged=false, returning false");
			return false;
		}
		GUB_RandomizeSpawnManager spawnManager = GetRandomizeSpawnManager();
		if (!spawnManager || !spawnManager.GetDefendFaction())
		{
			Print(string.Format("[DefendFlag] IsRestricted - spawnManager=%1, returning false", spawnManager));
			return false;
		}
		FactionKey defendFactionKey = spawnManager.GetDefendFaction().m_sFactionKey;
		if (factionKeyPlayer != defendFactionKey)
		{
			Print(string.Format("[DefendFlag] IsRestricted - factionKeyPlayer=%1 != defendFactionKey=%2, returning false", factionKeyPlayer, defendFactionKey));
			return false;
		}
		int elapsed = m_iSlotsTime - m_iStepTime;
		Print(string.Format("[DefendFlag] IsRestricted - elapsed=%1ms / 60000ms, restricted=%2", elapsed, elapsed < 60000));
		if (elapsed >= 60000)
			return false;
		return true;
	}

	override void OnPlayerConnected(int playerId)
	{
		super.OnPlayerConnected(playerId);

		if (!Replication.IsServer())
			return;

		string uid = GetGame().GetBackendApi().GetPlayerUID(playerId);
		bool flagged = m_mDefendFlagPlayers.Contains(uid);
		Print(string.Format("[DefendFlag] OnPlayerConnected playerId=%1 uid=%2 flagged=%3", playerId, uid, flagged));

		Rpc(RPC_SetDefendFlagToPlayer, playerId, flagged);
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RPC_SetDefendFlagToPlayer(int playerId, bool flagged)
	{
		PlayerController localPc = GetGame().GetPlayerController();
		if (!localPc)
		{
			Print("[DefendFlag] RPC_SetDefendFlagToPlayer - no local PlayerController");
			return;
		}
		int localPlayerId = localPc.GetPlayerId();
		Print(string.Format("[DefendFlag] RPC_SetDefendFlagToPlayer - playerId=%1 flagged=%2 localPlayerId=%3 match=%4", playerId, flagged, localPlayerId, playerId == localPlayerId));
		if (playerId != localPlayerId)
			return;
		PS_PlayableControllerComponent playableController = PS_PlayableControllerComponent.Cast(localPc.FindComponent(PS_PlayableControllerComponent));
		if (playableController)
		{
			playableController.m_bIsDefendFlagged = flagged;
			Print(string.Format("[DefendFlag] Set m_bIsDefendFlagged=%1 for local player %2", flagged, localPlayerId));
		}
		else
		{
			Print("[DefendFlag] RPC_SetDefendFlagToPlayer - no PS_PlayableControllerComponent on local PlayerController");
		}
	}

	void AddCommands()
	{
		SCR_ChatPanelManager chatPanelManager = SCR_ChatPanelManager.GetInstance();
		ChatCommandInvoker invoker = chatPanelManager.GetCommandInvoker("timer");
		invoker.Insert(SendQTvT_Timer_CommandCallback);
	}

	void SendQTvT_Timer_CommandCallback(SCR_ChatPanel panel, string data)
	{
		PlayerController playerController = GetGame().GetPlayerController();
		if (!playerController)
			return;

		PS_PlayableControllerComponent playableController =
			PS_PlayableControllerComponent.Cast(playerController.FindComponent(PS_PlayableControllerComponent));
		if (!playableController)
			return;

		if (!PS_PlayersHelper.IsAdminOrServer())
			return;

		data = data.Trim();
		string dataLower = data;
		dataLower.ToLower();

		string subCommand;
		int timeValue = 0;

		int spaceIndex = dataLower.IndexOf(" ");
		if (spaceIndex > -1)
		{
			subCommand = dataLower.Substring(0, spaceIndex);
			string valueStr = dataLower.Substring(spaceIndex + 1, dataLower.Length() - spaceIndex - 1);
			valueStr = valueStr.Trim();
			timeValue = valueStr.ToInt();
		}
		else
		{
			subCommand = dataLower;
		}

		switch (subCommand)
		{
		case "stop":
			playableController.SendQTvTTimerCommand(PS_ETimerCommand.STOP, 0);
			break;
		case "start":
			playableController.SendQTvTTimerCommand(PS_ETimerCommand.START, 0);
			break;
		case "add":
			if (timeValue > 0)
				playableController.SendQTvTTimerCommand(PS_ETimerCommand.ADD, timeValue);
			break;
		case "minus":
			if (timeValue > 0)
				playableController.SendQTvTTimerCommand(PS_ETimerCommand.MINUS, timeValue);
			break;
		}
	}

	void ProcessTimerCommand(PS_ETimerCommand command, int value)
	{
		switch (command)
		{
		case PS_ETimerCommand.STOP:
			m_bTimerEnabled = false;
			break;
		case PS_ETimerCommand.START:
			m_bTimerEnabled = true;
			break;
		case PS_ETimerCommand.ADD:
			m_iStepTime += value * 1000;
			Replication.BumpMe();
			break;
		case PS_ETimerCommand.MINUS:
			m_iStepTime -= value * 1000;
			if (m_iStepTime < 0)
				m_iStepTime = 1;
			Replication.BumpMe();
			break;
		}
	}

	void CheckAlive()
	{
		PS_PlayableManager playableManager = PS_PlayableManager.GetInstance();

		FactionKey checkFaction = "";
		array<PS_PlayableContainer> playables = playableManager.GetPlayablesSorted();
		foreach (PS_PlayableContainer playable : playables)
		{
			EDamageState damageState = playable.GetDamageState();
			if (damageState == EDamageState.DESTROYED)
				continue;

			FactionKey factionKey = playable.GetFactionKey();
			if (checkFaction != "" && checkFaction != factionKey)
			{
				return;
			}
			checkFaction = factionKey;
		}

		m_OnOnlyOneFactionAlive.Invoke(checkFaction);
		AdvanceGameState(SCR_EGameModeState.GAME);
		GetGame().GetCallqueue().Remove(CheckAlive);
	}

	override bool CanJoinFaction(FactionKey factionKeyPlayer, FactionKey currentFaction)
	{
		if (m_iFactionsBalance == -1)
			return true;
		if (factionKeyPlayer == currentFaction)
			return true;

		map<FactionKey, int> players = new map<FactionKey, int>();
		map<FactionKey, int> playables = new map<FactionKey, int>();
		map<FactionKey, float> desiredratio = new map<FactionKey, float>();
		array<PS_PlayableContainer> playableComponents = m_playableManager.GetPlayablesSorted();

		//counting avaivable slots
		int playablesammount = 0;
		foreach (PS_PlayableContainer playable : playableComponents)
		{
			playablesammount = playablesammount + 1;
			FactionKey factionKey = playable.GetFactionKey();
			if (!players.Contains(factionKey))
				players[factionKey] = 0;
			if (!playables.Contains(factionKey))
				playables[factionKey] = 0;

			playables[factionKey] = playables[factionKey] + 1;
			int playerId = m_playableManager.GetPlayerByPlayable(playable.GetRplId());
			if (playerId > 0)
			{
				players[factionKey] = players[factionKey] + 1;
			}

		}
		if (currentFaction != "")
			players[currentFaction] = players[currentFaction] - 1;

		//counting how much there are factons units compared to every unit
		int playersCount = m_PlayerManager.GetPlayerCount();

		foreach (FactionKey factionKey, int count: playables)
		{
			desiredratio[factionKey] = count / playablesammount;
		}
		//clamping avaivable over the ratio slots in proportion to current player count to ensure balance for small scenarios
		//int adjfactionsbalance = Math.Clamp(m_iFactionsBalance,1,(playersCount / 10));

		if (players[factionKeyPlayer] < 1)
		{
			//but we still want to get one player even in tiniest scenario
			return true;
		}
		//check if that faction with a new player wouldnt get too many players
		float ratio = (players[factionKeyPlayer] + 1 - m_iFactionsBalance) / playersCount;

		return ratio <= desiredratio[factionKeyPlayer];
	}


	void BroadcastPolyZoneFactionChange(IEntity targetEntity, FactionKey factionKey, bool visible)
	{
		//Print("-2-BroadcastPolyZoneFactionChange called, entity=" + targetEntity + ", name=" + targetEntity.GetName() + ", factionKey=" + factionKey + ", visible=" + visible);
		if (!Replication.IsServer())
		{
			//Print("-2-ERROR: BroadcastPolyZoneFactionChange called on client!");
			return;
		}

		PS_PolyZone polyZone = PS_PolyZone.Cast(targetEntity.FindComponent(PS_PolyZone));
		if (polyZone)
			polyZone.ApplyFactionVisiblity(factionKey, visible);
		//else
		//Print("-2-ERROR: polyZone is null on entity=" + targetEntity);

		string entityName = targetEntity.GetName();
		//Print("-2-entityName=" + entityName);
		Rpc(RPC_BroadcastPolyZoneFactionChange, entityName, factionKey, visible);
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RPC_BroadcastPolyZoneFactionChange(string entityName, FactionKey factionKey, bool visible)
	{
		//Print("-3-RPC_BroadcastPolyZoneFactionChange received, entityName=" + entityName + ", factionKey=" + factionKey + ", visible=" + visible);
		IEntity targetEntity = GetGame().GetWorld().FindEntityByName(entityName);
		//Print("-3-Found entity by name: " + targetEntity);
		if (!targetEntity)
		{
			//Print("-3-ERROR: targetEntity is null for name=" + entityName);
			return;
		}

		PS_PolyZone polyZone = PS_PolyZone.Cast(targetEntity.FindComponent(PS_PolyZone));
		//Print("-3-polyZone component: " + polyZone);
		if (polyZone)
			polyZone.ApplyFactionVisiblity(factionKey, visible);
		else
			//Print("-3-ERROR: polyZone is null on found entity");
	}


};
