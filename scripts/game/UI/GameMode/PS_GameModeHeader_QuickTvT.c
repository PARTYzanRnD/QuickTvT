modded class PS_GameModeHeader
{
	protected TextWidget m_StepTimerText;
	protected PS_GameModeQuickTvT m_GameMode;
	
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		
		m_GameMode = PS_GameModeQuickTvT.Cast(GetGame().GetGameMode());
		if (!m_GameMode)
			return;
		
		m_StepTimerText = TextWidget.Cast(w.FindAnyWidget("StepTimerText"));
		if (!m_StepTimerText)
			return;
		
		GetGame().GetCallqueue().CallLater(UpdateTimer, 0, true);
		UpdateTimer();
	}
	
	override void HandlerDeattached(Widget w)
	{
		super.HandlerDeattached(w);
		GetGame().GetCallqueue().Remove(UpdateTimer);
	}
	
	void UpdateTimer()
	{
		if (!m_GameMode || !m_StepTimerText)
		{
			GetGame().GetCallqueue().Remove(UpdateTimer);
			return;
		}
		
		int time = m_GameMode.GetStepTime() + 999;
		
		int seconds = time / 1000;
		int minutes = seconds / 60;
		seconds = Math.Mod(seconds, 60);
		
		m_StepTimerText.SetTextFormat("%1:%2", minutes.ToString(2), seconds.ToString(2));
	}
}
