modded class GUB_RandomizeMissionLogic
{
	
	
	void Updatestupidzones()
	{
		array<GUB_DefendPointEntity> defendPoints = GetRandomizeSpawnManager().GetDefendPoints();
		foreach (GUB_DefendPointEntity defendPoint : defendPoints)
		{
			if (defendPoint == m_eDefendLocation)
			{	
				//Print("---m_eDefendLocation");
				SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
				if (factionManager)
				{
					FactionKey flagFactionKey = GetRandomizeSpawnManager().GetDefendFaction().m_sFactionKey;
					FactionKey flagFactionKey2 = GetRandomizeSpawnManager().GetAttackFaction().m_sFactionKey;
					//SCR_Faction flagFaction = SCR_Faction.Cast(factionManager.GetFactionByKey(flagFactionKey));
					IEntity polyzone = GetGame().GetWorld().FindEntityByName(defendPoint.GetPolyzoneEntityName());
					//polyzone.Assignpolyzoneaction(flagFaction);
					if (polyzone)
					{
						//Print("-1-polyzone" + polyzone);
						
						PS_PolyZone polyzone2zone = PS_PolyZone.Cast(polyzone.FindComponent(PS_PolyZone));
						polyzone2zone.SetVisibleForFaction(flagFactionKey, true);
						polyzone2zone.SetVisibleForFaction(flagFactionKey2, true);
						//Print("---polyzone2zone" + polyzone2zone.m_aVisibleForFactions);
						//polyzone.m_LinePolygon.m_iColor = Color.FromSRGBA(0,0,0,255);
						for (IEntity child = polyzone.GetChildren(); child; child = child.GetSibling())
						{
							
							PS_PolyZoneTrigger polytrigger = PS_PolyZoneTrigger.Cast(child);
							if (polytrigger)
							{	
								//Print("---polytrigger" + polytrigger);
								polytrigger.m_sFactionKey = flagFactionKey;
							}
						}
					}
					
				}
			} else {
				IEntity polyzone2 = GetGame().GetWorld().FindEntityByName(defendPoint.GetPolyzoneEntityName());
				PS_PolyZone polyzone2zone = PS_PolyZone.Cast(polyzone2.FindComponent(PS_PolyZone));
				//Print("---polyzone2" + polyzone2zone);
				
				if (polyzone2zone)
				//if (polyzone2)
				{
					////Print("---polyzone2" + polyzone2zone);
					//SCR_MapEntity m_MapEntity = SCR_MapEntity.GetMapInstance();
					//polyzone2zone.DeleteMapWidget(m_MapEntity.GetMapConfig());
					polyzone2.SetOrigin("0 0 0");
					//Print("---getpolyzone2" + polyzone2.GetOrigin());
					//polyzone2zone.UpdatePolygon();
					delete polyzone2;
					//Print("---depolyzone2" + polyzone2);
					//polyzone2zone.UpdatePolygon();
				}
			}
		}
	}
	
	
}

modded class GUB_RandomizeMissionComponent : ScriptComponent
{
	
	
	override void OnGameStateChanged(int NewState)
	{
		if (!Replication.IsServer())
			return;
		
		if (NewState == SCR_EGameModeState.BRIEFING)
		{
			m_rLogic.Updatestupidzones();
			if (m_bMarkersOnlyOnBriefing)
				m_rLogic.GenerateMarkers();
		}
		if (NewState == SCR_EGameModeState.GAME)
		{
			m_rLogic.UpdateDefendEntities();
			m_rLogic.DeleteUselessFreezeZones();
		}
	}
}

modded class GUB_DefendPointEntity : GUB_LocationPointEntity
{
	[Attribute("", UIWidgets.EditBox, "Related polyzone")]
	private string m_sPolyzoneEntityName;
	string GetPolyzoneEntityName()
	{
		return m_sPolyzoneEntityName;
	}
}


modded class PS_PolyZone
{
	void SetVisibleForFaction(FactionKey factionKey, bool visible)
	{
		//Print("-1-SetVisibleForFaction called, factionKey=" + factionKey + ", visible=" + visible);
		//Print("-1-SetVisibleForFaction m_ePolylineShapeEntity=" + m_ePolylineShapeEntity + ", name=" + m_ePolylineShapeEntity);
		if (Replication.IsServer())
		{
			PS_GameModeQuickTvT gameMode = PS_GameModeQuickTvT.Cast(GetGame().GetGameMode());
			if (gameMode)
				gameMode.BroadcastPolyZoneFactionChange(m_ePolylineShapeEntity, factionKey, visible);
			else
				//Print("-1-ERROR: gameMode is null!");
		}
		else
		{
			//Print("-1-SetVisibleForFaction called on CLIENT, skipping");
		}
	}

	void ApplyFactionVisiblity(FactionKey factionKey, bool visible)
	{
		//Print("-1-ApplyFactionVisiblity called, factionKey=" + factionKey + ", visible=" + visible);
		//Print("-1-ApplyFactionVisiblity m_aVisibleForFactions before: " + m_aVisibleForFactions);

		if (visible)
		{
			if (!m_aVisibleForFactions.Contains(factionKey))
				m_aVisibleForFactions.Insert(factionKey);
		}
		else
		{
			m_aVisibleForFactions.RemoveItem(factionKey);
		}

		//Print("-1-ApplyFactionVisiblity m_aVisibleForFactions after: " + m_aVisibleForFactions);

		if (m_MapEntity && m_MapEntity.IsOpen())
		{
			if (IsCurrentVisibility())
			{
				if (!m_wCanvasWidget)
				{
					//Print("-1-ApplyFactionVisiblity creating map widget");
					CreateMapWidget(m_MapEntity.GetMapConfig());
				}
			}
			else
			{
				if (m_wCanvasWidget)
				{
					//Print("-1-ApplyFactionVisiblity deleting map widget");
					DeleteMapWidget(m_MapEntity.GetMapConfig());
				}
			}
		}
		else
		{
			//Print("-1-ApplyFactionVisiblity map not open or m_MapEntity null, skipping widget refresh");
		}
	}
}