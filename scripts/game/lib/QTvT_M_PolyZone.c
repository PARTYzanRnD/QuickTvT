modded class GUB_RandomizeMissionLogic
{
	
	protected GUB_AttackPointEntity m_eSkirmishAttackLocation;
	
    void RandomizeSkirmishMission()
    {
		if (!m_fDefendFaction || !m_fAttackFaction)
			return;

		if (GetGame().InPlayMode())
		{
			if (!GenerateSkirmishLocations())
				return;
			Print("GUB_RandomizeMissionLogic: Generated Locations: Defend: " + m_eDefendLocation.GetName() + ", Attack: " + m_eAttackLocation.GetName());
			
	        GetGame().GetCallqueue().CallLater(SpawnSkirmishSquads, 100);
		}
		else
		{
			DebugLocations();
			DebugFactions();
		}
    }
	
	
	bool GenerateSkirmishLocations()
	{
        m_eDefendLocation = GetRandomizeSpawnManager().GetDefendPoints().GetRandomElement();
		
		array<GUB_DefendPointEntity> triedDefendPoints = {};

		while (true) // или пока не найдём подходящий вариант
		{
		    m_eDefendLocation = GetRandomizeSpawnManager().GetDefendPoints().GetRandomElement();
		
		    // если все возможные точки обороны уже были проверены, выходим с fallback'ом
		    if (triedDefendPoints.Find(m_eDefendLocation) != -1)
		        continue; // эту уже пробовали, берём другую
		
		    triedDefendPoints.Insert(m_eDefendLocation);
		
		    ref array<GUB_AttackPointEntity> validAttackPoints = GetValidAttackPoints(m_eDefendLocation);
		    if (validAttackPoints.IsEmpty())
		        continue; // нет атакующих точек – пробуем другую оборону
		
		    m_eAttackLocation = validAttackPoints.GetRandomElement();
		    validAttackPoints.Remove(validAttackPoints.Find(m_eAttackLocation));
		
		    if (validAttackPoints.IsEmpty())
		        continue; // не осталось точек для стычки – следующая оборона
		
		    // ищем подходящую точку для стычки (дальше 1000 метров)
		    m_eSkirmishAttackLocation = validAttackPoints.GetRandomElement();
		    vector attackOrigin = m_eAttackLocation.GetOrigin();
		    vector defOrigin = m_eDefendLocation.GetOrigin();
		    float distance = Math.Sqrt(vector.DistanceSq(m_eSkirmishAttackLocation.GetOrigin(), attackOrigin));
		    float distancedef = Math.Sqrt(vector.DistanceSq(attackOrigin, defOrigin));
		    float skirdistancedef = Math.Sqrt(vector.DistanceSq(m_eSkirmishAttackLocation.GetOrigin(), defOrigin));
		    int maxAttempts = validAttackPoints.Count();
		    int attempt = 0;
			Print(string.Format("GUB_RandomizeMissionLogic distance between %1 which is compared to %2 (%5) also dist from att is %3 and skir to def is %4 (%6)", distance, (m_fMinDistanceBetweenLocations * 1.5), distancedef, skirdistancedef, (distance <= (m_fMinDistanceBetweenLocations * 1.5)^2), (Math.AbsFloat(distancedef - skirdistancedef) >= 1000)));
		
		    while (distance <= (m_fMinDistanceBetweenLocations * 1.5) && attempt < maxAttempts && Math.AbsFloat(distancedef - skirdistancedef) >= 600)
		    {
		        m_eSkirmishAttackLocation = validAttackPoints.GetRandomElement();
		        distance = Math.Sqrt(vector.DistanceSq(m_eSkirmishAttackLocation.GetOrigin(), attackOrigin));
				distancedef = Math.Sqrt(vector.DistanceSq(attackOrigin, defOrigin));
		    		skirdistancedef = Math.Sqrt(vector.DistanceSq(m_eSkirmishAttackLocation.GetOrigin(), defOrigin));
		        attempt++;
		    }
		
		    if (distance > (m_fMinDistanceBetweenLocations * 1.5) && Math.AbsFloat(distancedef - skirdistancedef) < 600)
		    {
		        // нашли хорошую точку – выходим из цикла
		        return true;
		    }
			if (triedDefendPoints.Count() == GetRandomizeSpawnManager().GetDefendPoints().Count())
				break;
		    // иначе все варианты были слишком близко – пробуем другую точку обороны
		}
		
		Print(string.Format("GUB_RandomizeMissionLogic Error: Can't generate attack point to {%1}", m_eDefendLocation.GetName()), LogLevel.ERROR);
		return false;
		
		
		/*
		ref array<GUB_AttackPointEntity> validAttackPoints = GetValidAttackPoints(m_eDefendLocation);
		if (validAttackPoints.Count() == 0)
		{
			Print(string.Format("GUB_RandomizeMissionLogic Error: Can't generate attack point to {%1}", m_eDefendLocation.GetName()), LogLevel.ERROR);
			return false;
		}
		m_eAttackLocation = validAttackPoints.GetRandomElement();
		validAttackPoints.Remove(validAttackPoints.Find(m_eAttackLocation));
		m_eSkirmishAttackLocation = validAttackPoints.GetRandomElement();
		vector location = m_eAttackLocation.GetOrigin();
		distance = vector.DistanceSq(m_eSkirmishAttackLocation.GetOrigin(), location);
		if (distance <= 1000*1000)
			continue;
		Print("[s " + m_eSkirmishAttackLocation + " m_eAttackLocation " + m_eAttackLocation);
		return true;*/
	}
	
	
    void SpawnSkirmishSquads()
    {
		m_eSkirmishAttackLocation.SpawnSkirmishUnitsInChilds();
		m_eAttackLocation.SpawnUnitsInChilds();
    }
	
	void GenerateSkirmishMarkers()
	{
		Managed m = SCR_BaseContainerTools.CreateInstanceFromPrefab(m_MarkerMakerConfigPath);
		m_MarkerMakerConfig = PS_MarkerMakerConfig.Cast(m);
		if (!m_MarkerMakerConfig)
			return;
		
		for (IEntity child = m_eSkirmishAttackLocation.GetChildren(); child; child = child.GetSibling())
		{
			GUB_SpawnPointEntity spawnPoint = GUB_SpawnPointEntity.Cast(child);
			if (spawnPoint)
				spawnPoint.SpawnSkirmishMarkers(m_MarkerMakerConfig);
		}
		
		for (IEntity child = m_eAttackLocation.GetChildren(); child; child = child.GetSibling())
		{
			GUB_SpawnPointEntity spawnPoint = GUB_SpawnPointEntity.Cast(child);
			if (spawnPoint)
				spawnPoint.SpawnMarkers(m_MarkerMakerConfig);
		}
		
		m_eDefendLocation.SpawnObjectiveMarkers(m_MarkerMakerConfig);
	}
	
	void UpdateSkirmishDefendEntities()
	{
		if (!GetGame().InPlayMode())
			return;
		
		array<GUB_DefendPointEntity> defendPoints = GetRandomizeSpawnManager().GetDefendPoints();
		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (factionManager)
		{
			FactionKey flagFactionKey = "FIA";
			SCR_Faction flagFaction = SCR_Faction.Cast(factionManager.GetFactionByKey(flagFactionKey));
			foreach (GUB_DefendPointEntity defendPoint : defendPoints)
			{
				defendPoint.SetFlagFaction(flagFaction);
				
				bool isDefendLocation = defendPoint == m_eDefendLocation;
				defendPoint.SetFlagActivate(isDefendLocation);
				if (isDefendLocation)
				{
					defendPoint.AddObjective("ObjectiveDefendWin");
					defendPoint.AddObjective("ObjectiveAttackWin");
				}
			}
		}
	}
	
	
	void SpawnSkirmishObjectives()
	{
		string resourceName = "{A4F0B4846EF850C4}Prefabs/Objective/Objective.et";
		
		//IEntity defendObjectiveEntity = GetGame().SpawnEntityPrefab(Resource.Load(resourceName), GetGame().GetWorld(), EntitySpawnParams());
		IEntity defendObjectiveEntity = GetGame().GetWorld().FindEntityByName("ObjectiveDefendWin");
		PS_Objective defendObjective = PS_Objective.Cast(defendObjectiveEntity);
		if (defendObjective)
		{
			//defendObjective.SetName("ObjectiveDefendWin");
			defendObjective.m_sFactionKey = m_fDefendFaction.m_sFactionKey;
			defendObjective.SetCompleted(false);			
			defendObjective.SetTitle(defendObjective.GetTitle() + " " + defendObjective.m_sFactionKey);
			m_aGeneratedObjectives.Insert(defendObjective);
		}
		
		//IEntity attackObjectiveEntity = GetGame().SpawnEntityPrefab(Resource.Load(resourceName), GetGame().GetWorld(), EntitySpawnParams());
		IEntity attackObjectiveEntity = GetGame().GetWorld().FindEntityByName("ObjectiveAttackWin");
		PS_Objective attackObjective = PS_Objective.Cast(attackObjectiveEntity);
		if (attackObjective)
		{
			//attackObjective.SetName("ObjectiveAttackWin");
			attackObjective.m_sFactionKey = m_fAttackFaction.m_sFactionKey;
			attackObjective.SetCompleted(false);
			attackObjective.SetTitle(attackObjective.GetTitle() + " " + attackObjective.m_sFactionKey);
			m_aGeneratedObjectives.Insert(attackObjective);
		}
	}
	
	
	
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

modded class GUB_LocationPointEntity : GenericEntity
{
	
	void SpawnSkirmishUnitsInChilds()
	{
		for (IEntity child = GetChildren(); child; child = child.GetSibling())
        {
			GUB_SpawnPointEntity spawnPoint = GUB_SpawnPointEntity.Cast(child);
			if (spawnPoint)
				spawnPoint.SpawnSkirmishSquad();
        }
		Print(string.Format("GUB_LocationPointEntity: {%1} spawned %2 units", GetName(), GetAllUnitsCount()), LogLevel.NORMAL);
	}
}

modded class GUB_SpawnPointEntity : GenericEntity
{
	bool SpawnSkirmishSquad()
	{
		if (m_GeneratedGroup)
			return false;
		
		GUB_RandomizeSpawnManager spawnManager = GetRandomizeSpawnManager();
		GUB_FactionPrefab factionPrefab;
		factionPrefab = spawnManager.GetDefendFaction();
		
		GUB_GroupPrefab groupPrefab;
		foreach (GUB_GroupPrefab group : factionPrefab.m_aGroupPrefabs)
		{
			if (group.m_eGroupType == m_eGroupType)
			{
				groupPrefab = group;
				break;
			}
		}
		if (!groupPrefab)
			return false;
		
		string groupName;
		
		if (!SpawnSkirmishGroup(groupPrefab, groupName))
			return false;
		
		if (!SpawnVehicles(groupPrefab, groupName))
			return false;
		
		return true;
	}
	
	bool SpawnSkirmishMarkers(notnull PS_MarkerMakerConfig MarkerMakerConfig)
	{
		GUB_RandomizeSpawnManager spawnManager = GetRandomizeSpawnManager();
		PS_FactionManualMarkerConfig factionMarkerConfig;
		foreach (PS_FactionManualMarkerConfig markerConfig : MarkerMakerConfig.m_mFactionsManualMarkerConfig)
		{
			if (markerConfig.m_sFactionKey == spawnManager.GetDefendFaction().m_sFactionKey)
			{
				factionMarkerConfig = markerConfig;
				break;
			}
		}
		if (!factionMarkerConfig)
			return false;
		
		IEntity groupMarker = SpawnGroupMarker(factionMarkerConfig.m_mManualMarkerConfig, MarkerMakerConfig.m_sManualMarkerPrefab);
		if (groupMarker)
			m_aGeneratedMarkers.Insert(groupMarker);
		
		foreach (IEntity vehicleEntity : m_aGeneratedVehicles)
		{
			Vehicle vehicle = Vehicle.Cast(vehicleEntity);
			if (vehicle)
			{
				EVehicleType vehicleType = vehicle.m_eVehicleType;
				PS_ManualMarkerConfig markerConfig;
				
				foreach (PS_VehicleManualMarkerConfig vehicleManualMarkerConfig : MarkerMakerConfig.m_mVehiclesManualMarkerConfig)
				{
					if (vehicleManualMarkerConfig.m_iVehicleType == vehicleType)
					{
						markerConfig = vehicleManualMarkerConfig.m_mManualMarkerConfig;
						break;
					}
				}
				
				IEntity vehicleMarker = SpawnVehicleMarker(vehicle, markerConfig, MarkerMakerConfig.m_sManualMarkerPrefab);
				if (vehicleMarker)
					m_aGeneratedMarkers.Insert(vehicleMarker);
			}
		}
		return true;
	}
	
	/*protected bool SpawnSkirmishVehicles(GUB_GroupPrefab GroupPrefab, string GroupName)
	{
		if (m_bIsDefend && GroupPrefab.m_aDefendVehiclesPrefabs.Count() == 0)
			return true;
		if (!m_bIsDefend && GroupPrefab.m_aAttackVehiclesPrefabs.Count() == 0)
			return true;
		
		int NumVehicles;
		if (m_bIsDefend)
			NumVehicles = GroupPrefab.m_aDefendVehiclesPrefabs.Count();
		else
			NumVehicles = GroupPrefab.m_aAttackVehiclesPrefabs.Count();
		
		EntitySpawnParams spawnParams = new EntitySpawnParams();
        spawnParams.TransformMode = ETransformMode.WORLD;
        
        GetTransform(spawnParams.Transform);

        spawnParams.Transform[3] = spawnParams.Transform[3] + (spawnParams.Transform[2] * 3) + (spawnParams.Transform[0] * 4) + (spawnParams.Transform[1] * 5);
		
        vector angles = Math3D.MatrixToAngles(spawnParams.Transform);
		Math3D.AnglesToMatrix(Vector(-90, 0, 0) + angles, spawnParams.Transform);
		spawnParams.Transform[3] = spawnParams.Transform[3] - spawnParams.Transform[2] * (3 * NumVehicles - 3);
		
		for (int i = 0; i < NumVehicles; i++)
		{
			ResourceName resourceToSpawn;
			if (m_bIsDefend)
				resourceToSpawn = GroupPrefab.m_aDefendVehiclesPrefabs[i].m_aVehiclePrefabs.GetRandomElement();
			else
				resourceToSpawn = GroupPrefab.m_aAttackVehiclesPrefabs[i].m_aVehiclePrefabs.GetRandomElement();

            Resource resource = Resource.Load(resourceToSpawn);
            if (!resource.IsValid())
            {
                Print("GUB Error: Invalid resource definition: " + resourceToSpawn, LogLevel.ERROR);
				continue;
            }
			
			SCR_TerrainHelper.SnapAndOrientToTerrain(spawnParams.Transform, GetWorld());
			IEntity entity = GetGame().SpawnEntityPrefab(resource, GetGame().GetWorld(), spawnParams);
			Vehicle vehicle = Vehicle.Cast(entity);
			if (vehicle)
			{
				vehicle.m_sAttachmentGroupName = GroupName;
				//SCR_EntityHelper.SnapToGround(vehicle);
				m_aGeneratedVehicles.Insert(vehicle);
			}
			spawnParams.Transform[3] = spawnParams.Transform[3] + spawnParams.Transform[2] * 7;
		}
		return true;
	}*/
	
	protected bool SpawnSkirmishGroup(GUB_GroupPrefab GroupPrefab, out string GroupName)
	{
		GroupName = GetRandomizeSpawnManager().GetDefendFaction().m_sFactionKey;
		GroupName = GroupName + "_" + m_iCompanyCallsign.ToString() + m_iPlatoonCallsign.ToString() + m_iSquadCallsign.ToString();
		
		ResourceName resourceToSpawn;
		if (m_bIsDefend)
			resourceToSpawn = GroupPrefab.m_aDefendGroupPrefabs.GetRandomElement();
		else
			resourceToSpawn = GroupPrefab.m_aAttackGroupPrefabs.GetRandomElement();
		
        Resource resource = Resource.Load(resourceToSpawn);
        if (!resource.IsValid())
        {
            Print("GUB Error: Invalid resource definition: " + resourceToSpawn, LogLevel.ERROR);
            return false;
        }

        EntitySpawnParams spawnParams = new EntitySpawnParams();
        spawnParams.TransformMode = ETransformMode.WORLD;
        
        GetTransform(spawnParams.Transform);

        IEntity entity = GetGame().SpawnEntityPrefab(resource, GetGame().GetWorld(), spawnParams);
		SCR_AIGroup group = SCR_AIGroup.Cast(entity);
		if (group)
		{
			group.SetName(GroupName);
			PS_GroupCallsignAssigner callsignAssigner = PS_GroupCallsignAssigner.Cast(group.FindComponent(PS_GroupCallsignAssigner));
			if (callsignAssigner)
			{
				callsignAssigner.SetCompanyCallsign(m_iCompanyCallsign);
				callsignAssigner.SetPlatoonCallsign(m_iPlatoonCallsign);
				callsignAssigner.SetSquadCallsign(m_iSquadCallsign);
			}
			m_GeneratedGroup = group;
		}
		return true;
	}
	
	
	
}
class GUB_RandomizeSkirmishMissionComponentClass : GUB_RandomizeMissionComponentClass
{
}
class GUB_RandomizeSkirmishMissionComponent : GUB_RandomizeMissionComponent
{
	override protected void EOnInit(IEntity owner)
    {
		if (!Replication.IsServer())
			return;
		
		m_rLogic.RandomizeSkirmishMission();
		PS_GameModeCoop gameModeCoop = PS_GameModeCoop.Cast(GetGame().GetGameMode());
		if (gameModeCoop)
			gameModeCoop.GetOnGameStateChange().Insert(OnGameStateChanged);
		
		GetGame().GetCallqueue().CallLater(m_rLogic.SpawnSkirmishObjectives, 100);
		
		if (!m_bMarkersOnlyOnBriefing)
			GetGame().GetCallqueue().CallLater(m_rLogic.GenerateSkirmishMarkers, 100);
    }
	
	override void OnGameStateChanged(int NewState)
	{
		if (!Replication.IsServer())
			return;
		
		if (NewState == SCR_EGameModeState.BRIEFING)
		{
			if (m_bMarkersOnlyOnBriefing)
				m_rLogic.GenerateSkirmishMarkers();
		}
		if (NewState == SCR_EGameModeState.GAME)
		{
			m_rLogic.UpdateSkirmishDefendEntities();
			m_rLogic.DeleteUselessFreezeZones();
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