class VPPESPTracker: ScriptedWidgetEventHandler 
{
    protected Widget			m_RootWidget;
    protected GridSpacerWidget m_SpacerGrid;
    protected CheckBoxWidget   m_CheckBox;
    protected TextWidget       m_ItemNameWidget;
    protected TextWidget       m_ItemDistanceWidget;
	protected SliderWidget     m_HealthInput;
	protected SliderWidget     m_BloodInput;
	
	protected string 			m_BBtypeName; //for basebuilding typenames
	protected string 			m_BaseItemCode;
	
	protected string       		m_GUIDInput;
	protected string       		m_GUID64Input;
	protected ButtonWidget 		m_GUIDCopy;
	protected ButtonWidget 		m_SteamIDCopy;
	protected ButtonWidget 		m_CopyAll;
	protected ButtonWidget 		m_PosCopy;
	protected CheckBoxWidget 	m_ChkSelectPlayer;
	protected ButtonWidget 		m_BtnTpToMe;
	protected ButtonWidget 		m_BtnTpMeTo;
	protected ButtonWidget 		m_BtnHeal;
	protected ButtonWidget 		m_BtnKill;
	protected ButtonWidget 		m_BtnTpReturn;
	
	protected TextWidget 		m_Passcode;
	protected ButtonWidget 		m_PassCodeCopy;
	protected ButtonWidget 		m_PassCodeReset;
	
    protected string			m_ItemName;
    protected vector			m_MarkerPositon;
    protected bool				m_IsMarkerVisible;
	protected bool				m_DetialedWidget;
	protected SurvivorBase		player;
	protected int				m_bgColor;
	EntityAI					m_TrackerEntity;
	
	ref VPPHoldLeftClickEvent   m_MouseEventHold;

	protected ref array<ref VPPEspCatagoryHeader> m_catagoryHeaders;

    void VPPESPTracker(string itemName, EntityAI trackedEntity, int color = -1, bool visible = true ) 
	{
        m_RootWidget = GetGame().GetWorkspace().CreateWidgets(VPPATUIConstants.EspTracker, null);
        m_SpacerGrid       = GridSpacerWidget.Cast(m_RootWidget.FindAnyWidget("SpacerGrid"));

        m_CheckBox         = CheckBoxWidget.Cast(m_RootWidget.FindAnyWidget("CheckBox"));
        m_ItemNameWidget   = TextWidget.Cast(m_RootWidget.FindAnyWidget("ItemName"));
        m_ItemDistanceWidget = TextWidget.Cast(m_RootWidget.FindAnyWidget("ItemDistance"));
        m_bgColor = color;

        m_TrackerEntity    = trackedEntity;
        if (SurvivorBase.Cast(trackedEntity))
        {
        	player = SurvivorBase.Cast(m_TrackerEntity);
        }
		
        m_ItemName  	   = itemName;
        m_IsMarkerVisible  = visible;

        m_ItemNameWidget.SetText(m_ItemName);
		m_ItemNameWidget.SetColor(color);
        m_SpacerGrid.Update();
        m_ItemNameWidget.Update();
        m_ItemDistanceWidget.Update();
		
		m_RootWidget.SetSort(1022,true);
		m_ReferenceAlpha = m_SpacerGrid.GetAlpha(); //max
		GetGame().GetUpdateQueue(CALL_CATEGORY_GUI).Insert(this.DoUpdate);
		m_RootWidget.SetHandler(this);
		m_RootWidget.SetUserData(this); //for callbacks
		VPPUIManager.WIDGET_PTRs.Insert(m_RootWidget); //For drag-selection
    }

    void ~VPPESPTracker() 
	{
		if (VPPUIManager.WIDGET_PTRs != NULL && m_RootWidget != NULL)
			VPPUIManager.WIDGET_PTRs.RemoveItem(m_RootWidget); //For drag-selection

		GetGame().GetUpdateQueue(CALL_CATEGORY_GUI).Remove(this.DoUpdate);
        if (m_RootWidget != null)
			m_RootWidget.Unlink();
		
		if (m_catagoryHeaders){
			foreach(VPPEspCatagoryHeader header : m_catagoryHeaders){
				header.Destroy();
			}
		}
    }
	
	void InitPlayerEspWidget()
	{
		if (m_RootWidget != null)
			m_RootWidget.Unlink();

		m_catagoryHeaders = {};
		m_RootWidget = GetGame().GetWorkspace().CreateWidgets(VPPATUIConstants.EspTrackerDetailed, null);
		m_RootWidget.SetHandler(this);
		m_SpacerGrid  = GridSpacerWidget.Cast(m_RootWidget);
		//m_SpacerGrid.SetColor(ARGB(50, ((m_bgColor >> 16) & 0xFF), ((m_bgColor >> 8) & 0xFF), ((m_bgColor >> 0) & 0xFF)));
		
		Widget catagory;
		VPPEspCatagoryHeader mgr;
		if (player)
		{
			//Player info
			catagory = GetGame().GetWorkspace().CreateWidgets(VPPATUIConstants.EspTrackerPanelHeader, m_SpacerGrid);
			catagory.GetScript(mgr);
			mgr.BuildCatagory(VPPATUIConstants.EspTrackerInfos, "Info", false);
			mgr.SetRootCatagory(true, this);
			m_catagoryHeaders.Insert(mgr);
			//Player health
			catagory = GetGame().GetWorkspace().CreateWidgets(VPPATUIConstants.EspTrackerPanelHeader, m_SpacerGrid);
			catagory.GetScript(mgr);
			mgr.BuildCatagory(VPPATUIConstants.EspTrackerPlayerHealth, "Health");
			m_catagoryHeaders.Insert(mgr);
			//Player actions
			catagory = GetGame().GetWorkspace().CreateWidgets(VPPATUIConstants.EspTrackerPanelHeader, m_SpacerGrid);
			catagory.GetScript(mgr);
			mgr.BuildCatagory(VPPATUIConstants.EspTrackerActions, "Actions");
			m_catagoryHeaders.Insert(mgr);
			
			m_ItemNameWidget   	 = TextWidget.Cast(m_RootWidget.FindAnyWidget("NameInput"));
			m_ItemDistanceWidget = TextWidget.Cast(m_RootWidget.FindAnyWidget("DistanceInput"));
			m_HealthInput		 = SliderWidget.Cast(m_RootWidget.FindAnyWidget("HealthInput"));
			m_BloodInput		 = SliderWidget.Cast(m_RootWidget.FindAnyWidget("BloodInput"));
			m_GUIDCopy		 	 = ButtonWidget.Cast(m_RootWidget.FindAnyWidget("GUIDCopy"));
			m_SteamIDCopy        = ButtonWidget.Cast(m_RootWidget.FindAnyWidget("SteamIDCopy"));
			m_CopyAll        	 = ButtonWidget.Cast(m_RootWidget.FindAnyWidget("CopyAll"));
			
			m_PosCopy = ButtonWidget.Cast(m_RootWidget.FindAnyWidget("PosCopy"));
			m_ChkSelectPlayer = CheckBoxWidget.Cast(m_RootWidget.FindAnyWidget("ChkSelectPlayer"));
			m_BtnTpToMe = ButtonWidget.Cast(m_RootWidget.FindAnyWidget("BtnTpToMe"));
			m_BtnTpMeTo = ButtonWidget.Cast(m_RootWidget.FindAnyWidget("BtnTpMeTo"));
			m_BtnHeal = ButtonWidget.Cast(m_RootWidget.FindAnyWidget("BtnHeal"));
			m_BtnKill = ButtonWidget.Cast(m_RootWidget.FindAnyWidget("BtnKill"));
			m_BtnTpReturn = ButtonWidget.Cast(m_RootWidget.FindAnyWidget("BtnTpReturn"));
			
			EspToolsMenu espManager = EspToolsMenu.Cast(VPPAdminHud.Cast(GetVPPUIManager().GetMenuByType(VPPAdminHud)).GetSubMenuByType(EspToolsMenu));
			if (!player.IsAlive() && !espManager.ShowDeadPlayers())
			{
				espManager.RemoveTracker(this);
				return;
			}

			if (player.GetIdentity())
			{
				string pName = player.GetIdentity().GetName();
				if (!player.IsAlive())
					pName += " (Dead)";

				m_ItemNameWidget.SetText(pName);
				m_GUIDInput = player.GetIdentity().GetId();
				array<ref SyncPlayer> data = ClientData.m_PlayerList.m_PlayerList;
				if (data)
				{
					foreach(SyncPlayer pdata : data)
					{
						if (pdata.m_PlayerName == player.GetIdentity().GetName())
						{
							m_GUID64Input = pdata.m_UID;
							break;
						}
					}
				}
			}
		}
		
		//Base building actions
		if (BasebuildingHelperFuncs.IsRelatedToBaseBuilding(m_TrackerEntity, m_BBtypeName))
		{
			//Register lister RPC for aids expansion codelock system
			GetRPCManager().AddRPC("RPC_VPPESPTools", "HandleCodeFromObj", this, SingeplayerExecutionType.Client);
			
			catagory = GetGame().GetWorkspace().CreateWidgets(VPPATUIConstants.EspTrackerPanelHeader, m_SpacerGrid);
			catagory.GetScript(mgr);
			mgr.BuildCatagory(VPPATUIConstants.EspTrackerBaseBuilding, "Base Building", false);
			mgr.SetRootCatagory(true, this);
			m_catagoryHeaders.Insert(mgr);
			
			m_Passcode = TextWidget.Cast(m_RootWidget.FindAnyWidget("Passcode"));
			m_PassCodeCopy = ButtonWidget.Cast(m_RootWidget.FindAnyWidget("PassCodeCopy"));
			m_PassCodeReset = ButtonWidget.Cast(m_RootWidget.FindAnyWidget("PassCodeReset"));
			
			int combo;
			Print(m_BBtypeName);
			if (BasebuildingHelperFuncs.HasCombinationLock(GetTrackingObject(), m_BBtypeName, combo))
			{
				m_Passcode.SetText(combo.ToString());
			}else{
				m_Passcode.SetText("No passcode.");
			}
		}
		m_RootWidget.Update();
		m_RootWidget.SetSort(1023,true);
		m_RootWidget.GetSize(m_ReferenceX, m_ReferenceY);
	}
	
	void ExpandHeaders(bool healthOnly = false)
	{
		if (!m_catagoryHeaders || m_catagoryHeaders.Count() <= 0)
		{
			InitPlayerEspWidget();
			m_DetialedWidget = true;
		}

		if (healthOnly)
		{
			if (m_catagoryHeaders[0]) //player info
				m_catagoryHeaders[0].ChangeState();

			if (m_catagoryHeaders[1]) //health info
				m_catagoryHeaders[1].ChangeState();

			return;
		}

		foreach(VPPEspCatagoryHeader header : m_catagoryHeaders)
		{
			if (!header)
				continue;

			header.ChangeState();
		}
	}
	
	override bool OnClick(Widget w, int x, int y, int button)
	{
		switch(w)
		{
			case m_PassCodeCopy:
				if (m_BaseItemCode != string.Empty)
				{
					GetVPPUIManager().DisplayNotification("Copied passcode to clipboard!");
					GetGame().CopyToClipboard(string.Format("%1 Position: %2 Passcode: %3", m_TrackerEntity.GetType(), m_TrackerEntity.GetPosition().ToString(), m_BaseItemCode));
				}
				else
				{
					int combo;
					if (BasebuildingHelperFuncs.HasCombinationLock(m_TrackerEntity, m_BBtypeName, combo))
					{
						GetVPPUIManager().DisplayNotification("Copied passcode to clipboard!");
						GetGame().CopyToClipboard(string.Format("%1 Position: %2 Passcode: %3", m_TrackerEntity.GetType(), m_TrackerEntity.GetPosition().ToString(), combo));
					}
				}
				return true;
			break;
			
			case m_PassCodeReset:
				GetRPCManager().VSendRPC("RPC_VPPESPTools", "RestPasscodeFence", new Param1<EntityAI>(m_TrackerEntity), true);
				m_Passcode.SetText("Unlocked.");
				return true;
			break;
			
			case m_GUIDCopy:
				GetVPPUIManager().DisplayNotification("Bohemia Interactive ID of player copied to clipboard!");
				GetGame().CopyToClipboard(m_GUIDInput);
				return true;
			break;
			
			case m_SteamIDCopy:
				GetVPPUIManager().DisplayNotification("Steam64 ID of player copied to clipboard!");
				GetGame().CopyToClipboard(m_GUID64Input);
				return true;
			break;
			
			case m_CopyAll:
				GetVPPUIManager().DisplayNotification("Copied player detials to Clipboard!");
				string info = string.Format("Name: %1\nBohemia Interactive ID: %2\nSteam64 ID: %3\n\nSupport the mod and buy us a coffee: https://dayzvpp.com/donate", m_ItemName, m_GUIDInput, m_GUID64Input); 
				GetGame().CopyToClipboard(info);
				return true;
			break;
			
			case m_PosCopy:
				GetVPPUIManager().DisplayNotification("Copied player position & orientation to Clipboard!");
				string pPos = string.Format("Position: %1\nOrientation:%2", player.GetPosition().ToString(), player.GetOrientation().ToString());
				GetGame().CopyToClipboard(pPos);
				return true;
			break;
			
			case m_ChkSelectPlayer:
				MenuPlayerManager pManager;
				VPPAdminHud rootMenu = VPPAdminHud.Cast(GetVPPUIManager().GetMenuByType(VPPAdminHud));
				if (rootMenu.GetSubMenuByType(MenuPlayerManager) == null)
				{
					rootMenu.CreateSubMenu(MenuPlayerManager);
				}
				pManager = MenuPlayerManager.Cast(VPPAdminHud.Cast(GetVPPUIManager().GetMenuByType(VPPAdminHud)).GetSubMenuByType(MenuPlayerManager));
				pManager.GetPlayerEntry(m_GUID64Input).SetSelected(m_ChkSelectPlayer.IsChecked());
				pManager.SendForPlayerStats();
				return true;
			break;
			
			case m_BtnTpToMe:
				GetRPCManager().VSendRPC("RPC_MissionServer", "HandleChatCommand", new Param1<string>("/bring " + m_ItemName), true);
				return true;
			break;
			
			case m_BtnTpMeTo:
				GetRPCManager().VSendRPC("RPC_MissionServer", "HandleChatCommand", new Param1<string>("/goto " + m_ItemName), true);
				return true;
			break;
			
			case m_BtnTpReturn:
				GetRPCManager().VSendRPC("RPC_MissionServer", "HandleChatCommand", new Param1<string>("/return " + m_ItemName), true);
				return true;
			break;
			
			case m_BtnHeal:
				GetRPCManager().VSendRPC("RPC_MissionServer", "HandleChatCommand", new Param1<string>("/heal " + m_ItemName), true);
				return true;
			break;
			
			case m_BtnKill:
				GetRPCManager().VSendRPC("RPC_MissionServer", "HandleChatCommand", new Param1<string>("/kill " + m_ItemName), true);
				return true;
			break;
		}		
		
		if (w != null && w == m_CheckBox)
		{
			HandleCheckBox();
			return true;
		}
		return false;
	}

	void HandleCheckBox()
	{
		if (!m_CheckBox)
			return;

		EspToolsMenu espManager = EspToolsMenu.Cast(VPPAdminHud.Cast(GetVPPUIManager().GetMenuByType(VPPAdminHud)).GetSubMenuByType(EspToolsMenu));
		
		if (m_CheckBox.IsChecked())
		{
			if (player)
			{
				InitPlayerEspWidget();
				m_DetialedWidget = true;
				return;
			}
			
			if (BaseBuildingBase.Cast(m_TrackerEntity) || BasebuildingHelperFuncs.IsItemStorageSafe(m_TrackerEntity))
			{
				InitPlayerEspWidget();
				m_DetialedWidget = true;
				if (espManager)
				{
					espManager.AddEntry(m_ItemName, m_TrackerEntity);
				}
				return;
			}
			//Color update
			m_CheckBox.SetColor(ARGB(150,0,255,21));
			if (espManager) //Add
				espManager.AddEntry(m_ItemName, m_TrackerEntity);
		}
		else
		{
			m_CheckBox.SetColor(ARGB(150,255,0,0)); //Color update
			//Remove
			if (espManager)
				espManager.RemoveEntryByObject(m_TrackerEntity);
		}
	}

	override bool OnMouseButtonDown(Widget w, int x, int y, int button)
	{
		super.OnMouseButtonDown(w, x, y, button);
		if (w == m_SpacerGrid)
		{
			if (!m_MouseEventHold)
				m_MouseEventHold = new VPPHoldLeftClickEvent(this, "OnDragTracker", "OnDragTrackerStop");

			m_MouseEventHold.SetActive(true);
			return true;
		}
		return false;
	}

	override bool OnDoubleClick(Widget w, int x, int y, int button)
	{
		super.OnDoubleClick(w, x, y, button);
		if (w == m_SpacerGrid)
		{
			return true;
		}
		return false;
	}
	
    float CalcDistance() 
	{
		vector endPos = GetGame().GetPlayer().GetPosition();
		if (IsFreeCamActive())
			endPos = VPPGetCurrentCameraPosition();

        return vector.Distance(endPos, m_TrackerEntity.GetPosition());
    }

    protected	float	m_ReferenceX;
	protected	float	m_ReferenceY;
	protected	float	m_ReferenceAlpha;

	protected	bool	m_ScaleWithDistance = true;
	
	protected	float	m_PivotX = 0.5;
	protected	float	m_PivotY = 0.5;

    void DoUpdate(float tDelta)
	{
		if (m_TrackerEntity == null)
		{
			delete this;
			return;
		}
		
        if (IsMarkerVisible() && m_RootWidget && GetGame().GetPlayer()) 
		{
			EspToolsMenu espManager;
			vector startPos = m_TrackerEntity.GetPosition();

			if (player)
			{
				int headBoneIdx = player.GetBoneIndexByName("Head");
				vector ls = player.GetBonePositionMS(headBoneIdx);
				ls[1] = ls[1] + 0.3;
				startPos  = player.CoordToParent(ls);
				
				espManager = EspToolsMenu.Cast(VPPAdminHud.Cast(GetVPPUIManager().GetMenuByType(VPPAdminHud)).GetSubMenuByType(EspToolsMenu));
				if (!player.IsAlive() && (espManager && !espManager.ShowDeadPlayers()))
				{
					espManager.RemoveTracker(this);
					return;
				}
			}

			vector ScreenPosRelative = GetGame().GetScreenPosRelative(startPos);
            float alpha = m_SpacerGrid.GetAlpha();
			float z = ScreenPosRelative[2];

			//Alpha fading
			if(m_ScaleWithDistance && z >= 75)
			{
				alpha = Math.Clamp(m_ReferenceAlpha * (75 / z), 0.005, m_ReferenceAlpha);
			}
			m_SpacerGrid.SetAlpha(alpha);

            if( ScreenPosRelative[0] >= 1 || ScreenPosRelative[0] == 0 || ScreenPosRelative[1] >= 1 || ScreenPosRelative[1] == 0 ) {
                m_RootWidget.Show(false);
                return;
            } else if( ScreenPosRelative[2] < 0 ) {
                m_RootWidget.Show(false);
                return;
            } else {
                m_RootWidget.Show(true);
            }

            float pos_x, pos_y;
            vector ScreenPos = GetGame().GetScreenPos(startPos);
            pos_x = ScreenPos[0];
            pos_y = ScreenPos[1];

            pos_x = Math.Ceil(pos_x);
            pos_y = Math.Ceil(pos_y);

            m_RootWidget.SetPos(pos_x, pos_y);

			if (m_ItemDistanceWidget)
			{
				m_ItemDistanceWidget.SetText( "[" + Math.Floor(CalcDistance()).ToString() + "m]" );
			}

			BaseBuildingBase baseBuilding;
			if (player || BaseBuildingBase.CastTo(baseBuilding, m_TrackerEntity) || BasebuildingHelperFuncs.IsItemStorageSafe(m_TrackerEntity))
			{
				if (player)
				{
					//Check death
					if (!player.IsAlive() && espManager.ShowDeadPlayers() && player.GetIdentity())
					{
						m_ItemNameWidget.SetText(player.GetIdentity().GetName() + " (Dead)");
					}
				}

				float current_FOV = Camera.GetCurrentFOV();
				float config_FOV = GetDayZGame().GetUserFOVFromConfig();
				float FOV_scale = current_FOV / config_FOV;

				float xs, ys;
				m_RootWidget.GetSize(xs, ys);
				
				if(m_ScaleWithDistance && z >= 50)
				{
					xs = m_ReferenceX * ((50 / z) * FOV_scale);
					ys = m_ReferenceY * ((50 / z) * FOV_scale);
				}
				//Print(xs.ToString() + " :: " + ys.ToString());
				m_RootWidget.SetSize(xs, ys, true);
				m_RootWidget.Update();
			}
		
            m_SpacerGrid.Update();
			if (m_ItemNameWidget)
			{
			    m_ItemNameWidget.Update();
            	m_ItemNameWidget.SetColor(m_bgColor);
			}
			
			if (m_ItemDistanceWidget)
            	m_ItemDistanceWidget.Update();

			if (player && m_DetialedWidget)
			{
				m_RootWidget.FindAnyWidget("IconDead").Show(!player.IsAlive());
				m_RootWidget.FindAnyWidget("StrDead").Show(!player.IsAlive());

				m_HealthInput.SetCurrent(player.GetTransferValues().m_HealthClient * 100);
				m_BloodInput.SetCurrent(player.GetTransferValues().m_BloodClient * 5000);
			}
        }
		else if (m_RootWidget != NULL) 
		{
            m_RootWidget.Show(false);
        }
    }
	
	//For teleport feature
	void OnDragTracker()
	{
		m_SpacerGrid.SetColor(ARGB(m_SpacerGrid.GetAlpha() * 255, 27, 27, 255));
	}

	void OnDragTrackerStop()
	{
		if (m_MouseEventHold)
			m_MouseEventHold.SetActive(false);

		if (m_TrackerEntity)
		{
			vector posMarker = m_MouseEventHold.GetMarker().GetPosition();
			GetRPCManager().VSendRPC("RPC_TeleportManager", "TeleportEntity", new Param1<vector>(posMarker), true, NULL, m_TrackerEntity);
		}

		m_SpacerGrid.SetColor(ARGB(m_SpacerGrid.GetAlpha() * 255, 0, 0, 0));
	}

	//Callback for when give widget (WIDGET_PTRs) is selected
	void OnWidgetDragSelect(bool state)
	{
		if (!IsMarkerVisible())
			return;
		if (IsChecked() && state)
			return;

		SetChecked(state);
		HandleCheckBox();
	}

	//Handle callback from server for shitty ideas
	void HandleCodeFromObj(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		if(type == CallType.Client)
		{
			Param2<string,EntityAI> data;
			if(!ctx.Read(data))
				return;
			
			if (m_TrackerEntity == data.param2)
			{
				if (m_Passcode)
				{
					m_BaseItemCode = data.param1;
					m_Passcode.SetText(data.param1);
				}
			}
		}
	}

    bool IsMarkerVisible() 
	{
        return m_IsMarkerVisible;
    }
	
	void SetChecked(bool state)
	{
		if (m_CheckBox != null)
			m_CheckBox.SetChecked(state);
	}
	
	bool IsChecked()
	{
		if (m_CheckBox != null)
			return m_CheckBox.IsChecked();
		
		return false;
	}

    void SetMarkerVisible(bool visible) 
	{
        m_IsMarkerVisible = visible;
   	}
	
	bool IsTrackedObject(Object obj)
	{
		return obj.GetNetworkIDString() == m_TrackerEntity.GetNetworkIDString();
	}
	
	EntityAI GetTrackingObject()
	{
		return m_TrackerEntity;
	}

	Widget GetRootWidget()
	{
		return m_RootWidget;
	}
};