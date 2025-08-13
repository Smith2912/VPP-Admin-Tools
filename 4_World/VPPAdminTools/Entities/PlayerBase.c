enum InvisToggleType
{
	PLAYER = 1,
	SCRIPT
};

modded class PlayerBase
{
	private bool    hasGodmode;
	private bool    hasUnlimitedAmmo;
	private bool 	hasFlyinCar;
	bool			m_isInvisible;
	float           m_VPlayerScale;
	bool    		m_VScalePlayer;
	vector 			m_VPlayerPosCache;
	private bool    m_VfreezePlayer;
	private string  m_VPlayerName;
	private string  m_VPlayerSteamId;
	private string  m_VPlayerHashedId;
	private int     m_VPlayerSessionId;

	private InvisToggleType m_invisType;

	override void Init()
	{
		super.Init();
		RegisterNetSyncVariableBool("m_isInvisible");
		RegisterNetSyncVariableBool("m_VfreezePlayer");
		RegisterNetSyncVariableBool("m_VScalePlayer");
		RegisterNetSyncVariableFloat("m_VPlayerScale", 0.01, 100.0, 3);
		GetRPCManager().AddRPC( "RPC_PlayerBase", "InvokeReload", this, SingleplayerExecutionType.Client );
	}

	void ~PlayerBase()
	{
		ResetEntityName();
	}

	override void EEKilled( Object killer )
	{
		super.EEKilled(killer);
		ResetEntityName();
	}

	override void OnConnect()
	{
		super.OnConnect();
		if (GetIdentity())
		{
			m_VPlayerName = GetIdentity().GetName();
			m_VPlayerSteamId = GetIdentity().GetPlainId();
			m_VPlayerHashedId = GetIdentity().GetId();
			m_VPlayerSessionId = GetIdentity().GetPlayerId();
			SetName(m_VPlayerSteamId);
		}
	}

	override bool CanBeTargetedByAI(EntityAI ai)
	{
		if (InvisibilityStatus())
		{
			return false;
		}
		return super.CanBeTargetedByAI(ai);
	}

	override void SetActions(out TInputActionMap InputActionMap)
	{
		super.SetActions(InputActionMap);
		AddAction(ActionAdminBaseBuilder, InputActionMap);
	}

	private void ResetEntityName()
	{
		if (GetGame() && !GetGame().IsServer())
			return;

		SetName("");
	}

	void VPPHealPlayer(bool set_max = true)
	{
		if (GetGame().IsServer() || !GetGame().IsMultiplayer())
		{
			GetStomach().ClearContents();

			DamageSystem.ResetAllZones(this);
			GetModifiersManager().ResetAll();
			
			// bleeding sources
			if (m_BleedingManagerServer)
				m_BleedingManagerServer.RemoveAllSources();
			
			// Stats
			if (GetPlayerStats())
			{
				int bloodType = GetStatBloodType().Get();
				GetPlayerStats().ResetAllStats();
				GetStatBloodType().Set(bloodType);
			}

			// Agents
			if (m_AgentPool)
				m_AgentPool.RemoveAllAgents();
			
			if (m_StaminaHandler)
				m_StaminaHandler.SetStamina(GameConstants.STAMINA_MAX);
			
			// uncon
			if (IsUnconscious())
				DayZPlayerSyncJunctures.SendPlayerUnconsciousness(this, false);
			
			// set max
			if (set_max)
			{
				GetStatWater().Set(GetStatWater().GetMax());
				GetStatEnergy().Set(GetStatEnergy().GetMax());
			}
			
			// fix up inventory
			FixAllInventoryItems();
			
			//remove bloody hands
			PluginLifespan moduleLifespan = PluginLifespan.Cast(GetPlugin(PluginLifespan));
			moduleLifespan.UpdateBloodyHandsVisibilityEx(this, eBloodyHandsTypes.CLEAN);
			ClearBloodyHandsPenaltyChancePerAgent(eAgents.SALMONELLA);
			
			if (GetArrowManager())
				GetArrowManager().ClearArrows();
		}
	}

	void HealBrokenLegs()
	{
		float add_health_coef = 1.0;
		eBrokenLegs state = GetBrokenLegs();
		if (state == eBrokenLegs.BROKEN_LEGS)
		{
			AddHealth("LeftLeg","Health",(GetMaxHealth("LeftLeg", "Health")  - GetHealth("LeftLeg", "Health")) * add_health_coef);
			AddHealth("RightLeg","Health",(GetMaxHealth("RightLeg", "Health") - GetHealth("RightLeg", "Health")) * add_health_coef);
			AddHealth("RightFoot","Health",(GetMaxHealth("RightFoot", "Health") - GetHealth("RightFoot", "Health")) * add_health_coef);
			AddHealth("LeftFoot","Health",(GetMaxHealth("LeftFoot", "Health") - GetHealth("LeftFoot", "Health")) * add_health_coef);
		}
	}

	void InvokeReload(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		if (type == CallType.Client)
        {
			if ( !GetGame().IsInventoryOpen() && this )
			{
				EntityAI entity_hands = this.GetHumanInventory().GetEntityInHands();
				
				if ( entity_hands && entity_hands.IsWeapon() )
				{
					Magazine foundMag = Magazine.Cast(entity_hands.GetAttachmentByConfigTypeName( "DefaultMagazine" ));
					if( foundMag && foundMag.IsMagazine())
					{
						foundMag.LocalSetAmmoMax();
					}else{
						this.QuickReloadWeapon( entity_hands );
					}
				}
			}
		}
	}

	override void OnVariablesSynchronized()
	{
		super.OnVariablesSynchronized();
		//Client single instance only (this wont trigger on other local entity for another client. See SurvivorBase.c)
		VPPFreezePlayer(m_VfreezePlayer);
	}

	/*
	override void OnRPC(PlayerIdentity sender, int rpc_type, ParamsReadContext ctx)
	{
		super.OnRPC(sender, rpc_type, ctx);
		if (GetGame().IsDedicatedServer() && sender && rpc_type == VPPATRPCs.RPC_SYNC_FREECAM_POS)
		{
			if (!GetPermissionManager().VerifyPermission(sender.GetPlainId(), "FreeCamera"))
				return;

			vector camPos;
			if (ctx.Read(camPos))
			{
				SetPosition(camPos);
			}
		}
	}
	*/

	void VPPSetScaleValue(float scale)
	{
		if (GetGame().IsServer())
		{
			m_VPlayerScale = scale;
			SetSynchDirty();
			SetScale(scale);
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write(scale);
			rpc.Send(this, VPPATRPCs.RPC_SYNC_SCALEVAL, true, NULL);
		}
	}
	
	void VPPSetInvisibility(bool state, InvisToggleType toggleType = InvisToggleType.PLAYER)
	{
		if (GetGame().IsServer())
		{
			m_invisType = toggleType;
			/*
			if (!state)
			{
				//trigger update network bubble for clients
				SetAllowDamage(false);
				dBodyEnableGravity(this, false);
				vector realPos = GetPosition();
				GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(this.PosRelocation, 1000, false, realPos);
				SetPosition("0 1200 0");
			}
			*/

			m_isInvisible = state;
			SetSynchDirty();
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write(state);
			rpc.Send(this, VPPATRPCs.RPC_SYNC_INVISIBILITY, true, NULL);
		}
	}

	void PosRelocation(vector pos)
	{
		if (!GetGame().IsDedicatedServer() || !GetGame().IsServer())
			return;

		SetPosition(pos);
		dBodyEnableGravity(this, true);
		if (!GodModeStatus())
			SetAllowDamage(true);
	}
	
	void setGodMode(bool trigger)
	{
		hasGodmode = trigger;
		SetAllowDamage(!trigger);
	}

	string VPlayerGetHashedId() return m_VPlayerHashedId;
	string VPlayerGetSteamId() return m_VPlayerSteamId;
	string VPlayerGetName() return m_VPlayerName;
	int VPlayerGetSessionId() return m_VPlayerSessionId;

	void VPPFreezePlayer(bool state)
	{
		if (GetGame().IsServer())
		{
			m_VfreezePlayer = state;
			SetSynchDirty();
			return;
		}
		
		HumanInputController hic = this.GetInputController();
		if (hic)
			hic.SetDisabled( state );
	}

	bool VPPIsFreezeControls()
	{
		return m_VfreezePlayer;
	}
	
	bool GodModeStatus()
	{
		return hasGodmode;
	}
	
	InvisToggleType InvisibilityToggleType()
	{
		return m_invisType;
	}

	bool InvisibilityStatus()
	{
		return m_isInvisible;
	}

	float VPPGetPlayerScale()
	{
		return m_VPlayerScale;
	}
	
	void VPPSetUnlimitedAmmo(bool state)
	{
		hasUnlimitedAmmo = state;
	}
	
	bool VPPIsUnlimitedAmmo()
	{
		return hasUnlimitedAmmo;
	}
	
	void SetCanFlyCars(bool state)
	{
		hasFlyinCar = state;
	}
	
	bool CanFlyCar()
	{
		return hasFlyinCar;
	}

	void UnlimitedAmmoCheck(Weapon_Base weapon)
	{
		if (VPPIsUnlimitedAmmo())
		{
			Magazine magazine;
			if (GetGame().IsServer())
			{
				magazine = weapon.GetMagazine(weapon.GetCurrentMuzzle());

				if (GetGame().IsMultiplayer() && magazine)
	            	GetGame().RemoteObjectDelete(magazine);

				if(magazine)
					magazine.ServerSetAmmoMax();

				if (GetGame().IsMultiplayer() && magazine)
            		GetGame().RemoteObjectCreate(magazine);
			}
			else
			{
				magazine = weapon.GetMagazine(weapon.GetCurrentMuzzle());
				if(magazine)
				{
					magazine.LocalSetAmmoMax();
				}
			}
		}
	}

	/*Misc functions*/
	vector GetHeadingPosition(int distance)
	{
		float heading = MiscGameplayFunctions.GetHeadingAngle(this);
		vector posModifier = Vector(-(distance * Math.Sin(heading)), 0, distance * Math.Cos(heading));
		return posModifier + GetPosition();
	}

	float GetAimingAngleLR()
	{
		HumanCommandWeapons	hcw = GetCommandModifier_Weapons();
		if (hcw != null)
			return hcw.GetBaseAimingAngleLR();
		return 0.0;
	}
	
	float GetAimingAngleUD()
	{
		HumanCommandWeapons	hcw = GetCommandModifier_Weapons();
		if (hcw != null)
			return hcw.GetBaseAimingAngleUD();
		return 0.0;
	}
};