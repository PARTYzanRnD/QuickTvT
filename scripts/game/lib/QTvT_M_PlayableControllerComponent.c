modded class PS_PlayableControllerComponent
{
	void SendQTvTTimerCommand(PS_ETimerCommand command, int value)
	{
		Rpc(RPC_SendQTvTTimerCommand, command, value);
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RPC_SendQTvTTimerCommand(PS_ETimerCommand command, int value)
	{
		PS_GameModeQuickTvT gameMode = PS_GameModeQuickTvT.Cast(GetGame().GetGameMode());
		if (gameMode)
			gameMode.ProcessTimerCommand(command, value);
	}

	override void RPC_ChangeFactionKey(int playerId, FactionKey factionKey)
	{
		PS_GameModeQuickTvT gameModeQuickTvT = PS_GameModeQuickTvT.Cast(GetGame().GetGameMode());
		if (gameModeQuickTvT)
			gameModeQuickTvT.m_iLastCanJoinFactionPlayerId = playerId;
		super.RPC_ChangeFactionKey(playerId, factionKey);
	}

	override void RPC_SetPlayerPlayable(int playerId, RplId playableId)
	{
		PS_GameModeQuickTvT gameModeQuickTvT = PS_GameModeQuickTvT.Cast(GetGame().GetGameMode());
		if (gameModeQuickTvT)
			gameModeQuickTvT.m_iLastCanJoinFactionPlayerId = playerId;
		super.RPC_SetPlayerPlayable(playerId, playableId);
	}

	override void RPC_SetPlayablePlayer(RplId playableId, int playerId)
	{
		PS_GameModeQuickTvT gameModeQuickTvT = PS_GameModeQuickTvT.Cast(GetGame().GetGameMode());
		if (gameModeQuickTvT)
			gameModeQuickTvT.m_iLastCanJoinFactionPlayerId = playerId;
		super.RPC_SetPlayablePlayer(playableId, playerId);
	}
}