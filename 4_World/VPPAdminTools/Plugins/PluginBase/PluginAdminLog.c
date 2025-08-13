modded class PluginAdminLog
{
	/*override void LogPrint( string message )
	{
		super.LogPrint( message );
	}*/

	string VPPGetPlayerPrefix( vector pos, PlayerBase identity )
	{
		string PosArray[3] = { pos[0].ToString(), pos[2].ToString(), pos[1].ToString() };
		
		for ( int i = 0; i < 3; i++ )
		{
			m_DotIndex = PosArray[i].IndexOf(".");
			if ( m_DotIndex != -1 )
			{
				PosArray[i] = PosArray[i].Substring( 0, m_DotIndex + 2 );
			}
		}
		
		if ( !identity )
		{
			return string.Format("Player (Unknown/Dead Entity)\nID: (Unknown)\nPosition: <%1, %2, %3>\n", PosArray[0], PosArray[2], PosArray[1]);
		}
		
		return string.Format("Player: (%1)\nID: (%2)\nPosition: <%3, %4, %5>\n",  identity.VPlayerGetName(), identity.VPlayerGetSteamId(), PosArray[0], PosArray[2], PosArray[1]);
	}

	string VPPGetHitMessage( TotalDamageResult damageResult, int component, string zone, string ammo) 
	{
		if ( damageResult )	
		{
			float dmg = damageResult.GetHighestDamage("Health");
			return " into Zone: (" + zone + ") Component: (" + component.ToString() + ") Damage Dealt: (" + dmg.ToString() + ") Damage Type: (" + ammo + ") ";
		}
		else
		{
			return " into Block: (" + component.ToString() + ") 0 damage dealt ";
		}
	}

	override void PlayerHitBy( TotalDamageResult damageResult, int damageType, PlayerBase player, EntityAI source, int component, string dmgZone, string ammo ) // PlayerBase.c 
	{
		if ( player && source )		
		{
			string PlayerPrefix = VPPGetPlayerPrefix( player.GetPosition() , player ) + " HP: (" + player.GetHealth().ToString() + ") \n";
			string HitMessage   = VPPGetHitMessage( damageResult, component, dmgZone, ammo );
			HitDamageMessage rpt = new HitDamageMessage();
			int srcId = -1;
			int low, high;
			bool shouldSendLog = true;

			if (player.GetIdentity() == null) return;

			rpt.victimName = player.VPlayerGetName();
			rpt.victimId   = player.VPlayerGetSteamId();

			switch ( damageType )
			{
				case DT_CLOSE_COMBAT:	// Player melee, animals, infected 
				
					if ( ( source.IsZombie() || source.IsAnimal() ) )  // Infected & Animals
					{
						rpt.sourceName = source.GetType();
						rpt.sourceId   = "_obj";
						rpt.details    = PlayerPrefix + " hit by " + source.GetDisplayName() + HitMessage;
					}			
					else if ( source.IsPlayer() || (source.GetHierarchyRootPlayer() && source.GetHierarchyRootPlayer().IsPlayer()) )		// Fists, includes bayonets and weapon bitch slaps 
					{
						m_Source = PlayerBase.Cast(source.GetHierarchyRootPlayer());
						m_PlayerPrefix2 = VPPGetPlayerPrefix( m_Source.GetPosition() ,  m_Source );
						
						rpt.sourceName = m_Source.VPlayerGetName();
						rpt.sourceId   = m_Source.VPlayerGetSteamId();
						rpt.details    = PlayerPrefix + " hit by " + m_PlayerPrefix2 + HitMessage;
					}
					else if ( source.IsMeleeWeapon() )			// Melee weapons
					{
						m_ItemInHands = source.GetDisplayName();		
						m_Source = PlayerBase.Cast( source.GetHierarchyRootPlayer() );
						m_PlayerPrefix2 = VPPGetPlayerPrefix( m_Source.GetPosition() ,  m_Source );

						rpt.sourceName = m_Source.VPlayerGetName();
						rpt.sourceId   = m_Source.VPlayerGetSteamId();
						rpt.details = PlayerPrefix + " hit by " + m_PlayerPrefix2 + HitMessage + " with " + m_ItemInHands;			
					}
					else
					{
						//Includes Beartraps and tripwire
						rpt.sourceName = source.GetType();
						rpt.sourceId   = "_obj";

						EnScript.GetClassVar(source, "m_activatorSessionID", -1, srcId);
						GetGame().GetPlayerNetworkIDByIdentityID(srcId, low, high);
						m_Source = PlayerBase.Cast(GetGame().GetObjectByNetworkId(low, high));

						if (m_Source)
							rpt.details = PlayerPrefix + " hit by " + source.GetType() + HitMessage + "\nSetup by " + VPPGetPlayerPrefix(m_Source.GetPosition(), m_Source);
						else
							rpt.details = PlayerPrefix + " hit by " + source.GetType() + HitMessage;
					}
					break;
				
				case DT_FIRE_ARM:	// Player ranged
					if ( source.IsWeapon() )
					{
						m_ItemInHands = source.GetDisplayName();				
						m_Source = PlayerBase.Cast(source.GetHierarchyRootPlayer());
						m_PlayerPrefix2 = VPPGetPlayerPrefix( m_Source.GetPosition() ,  m_Source );
						m_Distance = vector.Distance( player.GetPosition(), m_Source.GetPosition() );
						
						rpt.sourceName = m_Source.VPlayerGetName();
						rpt.sourceId   = m_Source.VPlayerGetSteamId();

						rpt.details = PlayerPrefix + " hit by " + m_PlayerPrefix2 + HitMessage + " with ("+ m_ItemInHands +") from (" + m_Distance + ") meters ";
					}
					else 
					{
						rpt.sourceName = source.GetType();
						rpt.sourceId   = "_obj";
						rpt.details = PlayerPrefix + " hit by (" + source.GetType() + ") " + HitMessage;		
					}
					break;
				
				case DT_EXPLOSION:	// Explosion
					rpt.sourceName =  ammo;
					rpt.sourceId   = "Explosion";
					
					EnScript.GetClassVar(source, "m_activatorSessionID", -1, srcId);
					GetGame().GetPlayerNetworkIDByIdentityID(srcId, low, high);
					m_Source = PlayerBase.Cast(GetGame().GetObjectByNetworkId(low, high));

					if (m_Source)
						rpt.details = PlayerPrefix + " hit by explosion (" + ammo + ")\n " + "Caused by " + VPPGetPlayerPrefix(m_Source.GetPosition(), m_Source);
					else
						rpt.details = PlayerPrefix + " hit by explosion (" + ammo + ") ";
					break;
						
				case DT_STUN: 		// unused atm
					rpt.details = PlayerPrefix + " stunned by (" + ammo + ") ";
					break;
						
				case DT_CUSTOM:		// Others (Vehicle hit, fall, fireplace, barbed wire ...)
					if ( ammo == "FallDamage" || ammo == "FallDamageShock" || ammo == "FallDamageHealth")
					{
						//FallDamage ignored, it's too spammy
						shouldSendLog = false;
					}
					else if ( source.GetType() == "AreaDamageBase" )  
					{
						EntityAI parent = EntityAI.Cast( source );
						if ( parent )
						{
							rpt.sourceName = parent.GetType();
							rpt.sourceId   = "_obj";
							rpt.details = PlayerPrefix + " hit by (" + parent.GetType() + ") with (" + ammo + ") ";
						}
					}
					else
					{
						rpt.sourceName = source.GetType();
						rpt.sourceId   = "_obj";
						rpt.details = PlayerPrefix + " by (" + source.GetType() + ") with (" + ammo + ") ";
					}
					break;
											
				default:
					rpt.details ="WARNING: PlayerHitBy Unknown damageType: (" + ammo + ")";
					break;
			}
			//Wrap up and send
			if (shouldSendLog)
			{
				rpt.SetContent();
				rpt.AddEmbed();
				GetWebHooksManager().PostData(HitDamageMessage, rpt);
			}
		}
		else
		{
			LogPrint("DEBUG: player/source does not exist");
		}
		super.PlayerHitBy( damageResult, damageType, player, source, component, dmgZone, ammo );
	}

	override void PlayerKilled( PlayerBase player, Object source )  
	{
		if ( player && source )
		{
			string PlayerPrefix = VPPGetPlayerPrefix( player.GetPosition(), player );
			string PlayerPrefix2 = "";
			KillDeathMessage rpt = new KillDeathMessage();
			
			if (player.GetIdentity() == null) return;

			rpt.victimName = player.VPlayerGetName();
			rpt.victimGUID = player.VPlayerGetSteamId();

			if( player == source )	// deaths not caused by another object (starvation, dehydration)
			{
				m_StatWater  = player.GetStatWater();
				m_StatEnergy = player.GetStatEnergy();
				m_BleedMgr   = player.GetBleedingManagerServer();
				
				if ( m_StatWater && m_StatEnergy && m_BleedMgr )
				{
					rpt.details = PlayerPrefix + " died. Stats => Water: [" + m_StatWater.Get().ToString() + "] Energy: [" + m_StatEnergy.Get().ToString() + "] Bleed sources: [" + m_BleedMgr.GetBleedingSourcesCount().ToString() + "]";
				}
				else if ( m_StatWater && m_StatEnergy && !m_BleedMgr )
				{
					rpt.details = PlayerPrefix + " died. Stats => Water: " + m_StatWater.Get().ToString() + " Energy: " + m_StatEnergy.Get().ToString() + "]";
				}
				else
				{
					rpt.details = PlayerPrefix + " died. Stats => could not fetch";
				}
				rpt.killerName = rpt.victimName;
				rpt.killerGUID = player.VPlayerGetSteamId();
			}
			else if (source.IsInherited(Grenade_Base) || source.IsInherited(LandMineTrap))
			{
				//Grenades + Landmines
				int srcId = -1;
				EnScript.GetClassVar(source, "m_activatorSessionID", -1, srcId);

				int low, high;
				GetGame().GetPlayerNetworkIDByIdentityID(srcId, low, high);
				m_Source = PlayerBase.Cast(GetGame().GetObjectByNetworkId(low, high));
				if (m_Source)
				{
					PlayerPrefix2 = VPPGetPlayerPrefix(m_Source.GetPosition(), m_Source);
					rpt.details = PlayerPrefix + " killed by:\n" + PlayerPrefix2 + " with " + source.GetDisplayName();
				}else{
					//Generic no source message fallback
					rpt.details = PlayerPrefix + " killed by: " + source.GetType();
				}
			}
			else if ( source.IsWeapon() || source.IsMeleeWeapon() )  // player
			{
				m_Source = PlayerBase.Cast( EntityAI.Cast( source ).GetHierarchyRootPlayer() );
				string name;
				string guid;

				if(m_Source != null && m_Source.GetIdentity() != null)
				{
					name = m_Source.VPlayerGetName();
					guid = m_Source.VPlayerGetSteamId();
					
					PlayerPrefix2 = VPPGetPlayerPrefix( m_Source.GetPosition() , m_Source );
					
					if ( source.IsMeleeWeapon() )
					{
						rpt.details = PlayerPrefix + " killed by:\n" + PlayerPrefix2 + " with " + source.GetDisplayName();	
					}
					else
					{
						m_Distance = vector.Distance( player.GetPosition(), m_Source.GetPosition() );
						rpt.details = PlayerPrefix + " killed by:\n" + PlayerPrefix2 + " with [" + source.GetDisplayName() + "] from [" + m_Distance + "] meters ";
					}
					rpt.killerName = name;
					rpt.killerGUID = guid;
				}
				else
				{
					rpt.details = PlayerPrefix + " killed by: " + source.GetType();
				}
			}
			else
			{
				if (source.IsInherited(ZombieBase))
				{
					rpt.killerName = "Infected";
				}
				rpt.details = PlayerPrefix + " killed by: " + source.GetType();
			}
			rpt.SetContent();
			rpt.AddEmbed();
			GetWebHooksManager().PostData(KillDeathMessage, rpt);
		}
		super.PlayerKilled( player, source );
	}
};