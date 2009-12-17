/*
* Copyright (c) 2005 Nokia Corporation and/or its subsidiary(-ies). 
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of "Eclipse Public License v1.0"
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* Nokia Corporation - initial contribution.
*
* Contributors:
*
* Description:  
*
*/


// INCLUDE FILES
#include <AknIconArray.h>   // for GulArray
#include <aknconsts.h>      // for KAvkonBitmapFile

#include "AspSettingDialog.h"
#include "AspProfileDialog.h"
#include "AspScheduleDialog.h"
#include "AspSchedule.h"
#include "AspDialogUtil.h"
#include "AspLogDialog.h"
#include "AspUtil.h"
#include "AspProfileWizardDialog.h"
#include "AspDebug.h"
#include "AspSyncUtil.rh"
#include "AspAdvanceSettingsDialog.h"

#include <aknsettingitemlist.h>

#include <csxhelp/ds.hlp.hrh>  // for help text ids

#ifndef RD_DSUI_TIMEDSYNC 
#include <avkon.mbg>
#else
#include "AspSettingViewDialog.h"
#endif
#include <DataSyncInternalPSKeys.h>
#include "AspAutoSyncHandler.h"
#include <featmgr.h>   // FeatureManager

const TInt KMSKControlId( CEikButtonGroupContainer::EMiddleSoftkeyPosition );




// ============================ MEMBER FUNCTIONS ===============================




/******************************************************************************
 * class CAspSettingDialog
 ******************************************************************************/

// -----------------------------------------------------------------------------
// CAspSettingDialog::ShowDialogL
// 
// -----------------------------------------------------------------------------
//
TBool CAspSettingDialog::ShowDialogL(TAspParam& aParam)
	{
    CAspSettingDialog* dialog = CAspSettingDialog::NewL(aParam);

	TBool ret = dialog->ExecuteLD(R_ASP_SETTING_DIALOG);

	return ret;
	}


// -----------------------------------------------------------------------------
// CAspSettingDialog::NewL
//
// -----------------------------------------------------------------------------
//
CAspSettingDialog* CAspSettingDialog::NewL(TAspParam& aParam)
    {
    FLOG( _L("CAspSettingDialog::NewL START") );

    CAspSettingDialog* self = new (ELeave) CAspSettingDialog(aParam);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);

    FLOG( _L("CAspSettingDialog::NewL END") );
    return self;
    }


// -----------------------------------------------------------------------------
// CAspSettingDialog::CAspSettingDialog
// 
// -----------------------------------------------------------------------------
//
CAspSettingDialog::CAspSettingDialog(TAspParam& aParam) 
    {
	iApplicationId = aParam.iApplicationId;
	iSyncSession = aParam.iSyncSession;
	iDialogMode = aParam.iMode;
	
	iCurrentProfileId = KErrNotFound;
	iCurrentProfileName = KNullDesC;
	
	__ASSERT_ALWAYS(iSyncSession, TUtil::Panic(KErrGeneral));
    }


// -----------------------------------------------------------------------------
// CAspSettingDialog::ConstructL
//
// -----------------------------------------------------------------------------
//
void CAspSettingDialog::ConstructL()
    {
    FLOG( _L("CAspSettingDialog::ConstructL START") );

    CAknDialog::ConstructL(R_ASP_SETTING_DIALOG_MENU);
    
    iSettingEnforcement = TUtil::SettingEnforcementState();

	TAspParam param(iApplicationId, iSyncSession);
	iProfileList = CAspProfileList::NewL(param);
	iProfileList->ReadAllProfilesL(CAspProfileList::ENoMandatoryCheck);
	iProfileList->Sort();
	
    param.iMode = CAspContentList::EInitDataProviders;
	iContentList = CAspContentList::NewL(param);
	
	// get previous title so it can be restored
	iStatusPaneHandler = CStatusPaneHandler::NewL(iAvkonAppUi);
	iStatusPaneHandler->StoreOriginalTitleL();
	
#ifdef RD_DSUI_TIMEDSYNC 

    iTabHandler = new (ELeave) CAspTabbedNaviPaneHandler(iAvkonAppUi->StatusPane() ,this);
	iUpdateTabGroup = ETrue;

#else
	// store current navi pane
	iStatusPaneHandler->StoreNavipaneL();
	
#endif
	
	

	iResHandler = CAspResHandler::NewL();

	// start listening sync database events
	iDbNotifier = CAspDbNotifier::NewL(param, this);
	iDbNotifier->RequestL();
	
	iPopupNote = CAknInfoPopupNoteController::NewL();
    iBearerHandler = CAspBearerHandler::NewL(param);

	SetCurrentProfileL();
	
	FLOG( _L("CAspSettingDialog::ConstructL END") );
    } 


// -----------------------------------------------------------------------------
// Destructor
//
// -----------------------------------------------------------------------------
//
CAspSettingDialog::~CAspSettingDialog()
    {
    FLOG( _L("CAspSettingDialog::~CAspSettingDialog START") );

	delete iProfileList;
	delete iContentList;
	
	delete iStatusPaneHandler;
	delete iResHandler;
	
	delete iDbNotifier;
	iDbNotifier = NULL;
	
	delete iSyncHandler;   // created when sync is started
	
	if (iAvkonAppUi)
		{
		iAvkonAppUi->RemoveFromStack(this);
		}

	if (iPopupNote)
		{
    	iPopupNote->HideInfoPopupNote();
	    delete iPopupNote;
		}

	delete iBearerHandler;

#ifdef RD_DSUI_TIMEDSYNC

	delete iTabHandler;
#endif

	FLOG( _L("CAspSettingDialog::~CAspSettingDialog END") );
    }


//------------------------------------------------------------------------------
// CAspSettingDialog::ActivateL (from CCoeControl)
//
// Called by system when dialog is activated
//------------------------------------------------------------------------------
//
void CAspSettingDialog::ActivateL()
	{
    CAknDialog::ActivateL();

	// this cannot be in ConstructL which is executed before dialog is launched
	iAvkonAppUi->AddToStackL(this);
    }


//------------------------------------------------------------------------------
// CAspSettingDialog::GetHelpContext
//
//
//------------------------------------------------------------------------------
//
void CAspSettingDialog::GetHelpContext(TCoeHelpContext& aContext) const
	{
	aContext.iMajor = KUidSmlSyncApp;
	aContext.iContext = KDS_HLP_MAIN_VIEW;    
	}


//------------------------------------------------------------------------------
// CAspSettingDialog::ProcessCommandL
//
// Handle commands from menu.
//------------------------------------------------------------------------------
//
void CAspSettingDialog::ProcessCommandL(TInt aCommandId)
	{
	TInt err = KErrNone;

    // prevent db notifications while processing menu command
	//Notifier()->SetDisabled(ETrue);
	TRAP(err, DoProcessCommandL(aCommandId));
	//Notifier()->SetDisabled(EFalse);
	
	if (err != KErrNone)
		{
		iDbNotifier->CheckUpdateEventL();
		FLOG( _L("### CAspSettingDialog::ProcessCommandL failed (%d) ###"), err );
		User::Leave(err);
		}
	}


//------------------------------------------------------------------------------
// CAspSettingDialog::DoProcessCommandL
//
// Handle commands from menu.
//------------------------------------------------------------------------------
//
void CAspSettingDialog::DoProcessCommandL(TInt aCommandId)
	{
	HideMenu();

	switch (aCommandId)
		{
        case EAspMenuCmdSync:
			{
			SyncProfileL();
			break;
			}
		case EAspMenuCmdCopyFromServer:
			{
			CopyFromServerL();
			break;
			}
        case EAspMenuCmdChangeProfile:
			{
			ChangeProfileL();
			break;
			}
        case EAspMenuCmdEdit:
			{
			EditProfileL(EDialogModeEdit ,KErrNotFound);
			break;
			}
        case EAspMenuCmdNewSet:
			{
			CreateProfileL();
			break;
			}
		case EAspMenuCmdDelete:
			{
			DeleteProfileL();
	   		break;
			}
		case EAspMenuCmdViewLog:
			{
			ShowLogDialogL();
			break;
			}
		case EAspMenuCmdAutoSync:
			{
			ShowAutoSyncDialogL();
			break;
			}
		case EAspMenuCmdAdvanceSettings:
		    {
		    //CAspAdvanceSettingsDialog* dialog = CAspAdvanceSettingsDialog::NewL();
		    //CleanupStack::PushL(dialog);
		        
		    CAspAdvanceSettingsDialog::ShowDialogL();
		        
		    //CleanupStack::PopAndDestroy(dialog);

		    //ShowContextMenuL(R_ASP_ROAMING_SETTINGS_CONTEXT_MENU);
		    break;
		    }
        case EAspMenuCmdRoamingSettings:
            {
            ShowContextMenuL(R_ASP_ROAMING_SETTINGS_CONTEXT_MENU);
            break;
            }		

		case EAspMenuCmdMark:
		case EAspMenuCmdUnmark:
			{
			SaveSelectionL();
			break;	
			}
    case EAknCmdHelp:
			{
            TUtil::LaunchHelpAppL(iEikonEnv);
			break;
		  }

        case EAspMenuCmdExit:
        case EAknCmdExit:
        case EEikCmdExit:
			{
   		
            //
			// Exit dialog
			//
			// CEikAppUi::ProcessCommandL starts CAknAppShutter that 
			// closes all dialogs and finally calling application. Before 
			// dialog is closed (deleted) it's OkToExitL(EEikBidCancel)
			// is called. EEikBidCancel means OkToExitL must silently
			// save and return ETrue.
			//
			iAvkonAppUi->ProcessCommandL(EAknCmdExit);

			break;
			}

		default:
			{
			break;
			}
		}
	}


// -----------------------------------------------------------------------------
// CAspSettingDialog::OkToExitL
//
// Called by framework before dialog is closed.
// -----------------------------------------------------------------------------
//
TBool CAspSettingDialog::OkToExitL(TInt aButtonId)
    {
    
	// Options/Exit, End key, Close key ('c')
	if (aButtonId == EEikBidCancel)
		{
		// framework calls this when dialog must shut down
    	return ETrue;
		}

	// Back key (cba)
	if (aButtonId == EAknSoftkeyBack)
        {
		return ETrue;
		}

    // Exit key (cba)
	if (aButtonId == EAknSoftkeyExit)
        {
        if (iDialogMode == EModeDialogNoWait && iApplicationId == EApplicationIdSync)
        	{
            //  close sync app
        	iAvkonAppUi->ProcessCommandL(EAknCmdExit);
        	}

        return ETrue;
		}
		
	if (aButtonId == EAknSoftkeyMark || aButtonId == EAknSoftkeyUnmark 
		|| aButtonId == EAknSoftkeyOpen)  // MSK
        {

		if (iProfileList->Count() == 0)
			{
			return EFalse;
			}
        
        if (IsProfileLockedL(iCurrentProfileId))
        	{
        	TDialogUtil::ShowErrorNoteL(R_ASP_LOCKED_PROFILE);
			CancelCheckboxEvent();
			return EFalse;
			}
        
        if (iSettingEnforcement)
			{
			TDialogUtil::ShowInformationNoteL(R_ASP_PROTECTED_SETTING);
			return EFalse;  // leave dialog open;
			}
        
        if (!CheckEmailSelection())
        	{
        	return EFalse;  // leave dialog open;
        	}
				
		TInt ret =  SaveCurrentProfile();
		if (ret != KErrNone)
			{
			CancelCheckboxEvent();
			}
			
		return EFalse;  // leave dialog open
		}
		

	return CAknDialog::OkToExitL(aButtonId);
    }


// ----------------------------------------------------------------------------
// CAspSettingDialog::CancelMailBoxForEnterL
// 
// ----------------------------------------------------------------------------
//

void CAspSettingDialog::CancelMailBoxForEnterL()
{
	CListBoxView* view = iSettingListBox->View();
	
    TInt index = view->CurrentItemIndex();
    TAspProviderItem& item = iContentList->ProviderItem(index);
    TBool selected = view->ItemIsSelected(index);
    
    if(selected)
    {
        view->DeselectItem(index);
    }
    else
    {
    	view->SelectItemL(index);
    }   
    
}


// ----------------------------------------------------------------------------
// CAspSettingDialog::OfferKeyEventL
// 
// ----------------------------------------------------------------------------
//
TKeyResponse CAspSettingDialog::OfferKeyEventL(const TKeyEvent& aKeyEvent,TEventCode aType)
	{
	if (aType == EEventKey)
		{
		switch (aKeyEvent.iCode)
			{
			//case EKeyOK:
			//	{
			//	ShowContextMenuL(R_ASP_SETTING_DIALOG_CONTEXT_MENU);
			//	return EKeyWasConsumed;
			//	}
			case EKeyEnter:
			{
                   CancelMailBoxForEnterL();
                   OkToExitL(EAknSoftkeyOpen);
                   return EKeyWasConsumed;
			}

			case EKeyEscape:  // framework calls this when dialog must shut
				{
				return CAknDialog::OfferKeyEventL(aKeyEvent, aType);
        		}
        		
			//case EStdKeyHome:
			//	{
			//	DeleteSelectedProfileL();
			//	return EKeyWasConsumed;
			//	}

#ifdef RD_DSUI_TIMEDSYNC 

			case EKeyLeftArrow:
				{
				SetPreviousProfileL();
				return EKeyWasConsumed;
				}

			case EKeyRightArrow:
				{
				SetNextProfileL();
				return EKeyWasConsumed;
				}

#endif
            default:
			    break;
			}
		}

	return CAknDialog::OfferKeyEventL( aKeyEvent, aType);
	}


// ----------------------------------------------------------------------------
// CAspSettingDialog::HandleResourceChange
// 
// ----------------------------------------------------------------------------
//

void CAspSettingDialog::HandleResourceChange(TInt aType)
    {   
    if (aType == KEikDynamicLayoutVariantSwitch) //Handle change in layout orientation
        {
        TRect mainPaneRect;
        AknLayoutUtils::LayoutMetricsRect(AknLayoutUtils::EMainPane, mainPaneRect);
        SetRect(mainPaneRect);
        iSettingListBox->SetSize(mainPaneRect.Size());
        CCoeControl::HandleResourceChange(aType);
		DrawDeferred();
	    return;
		}
		
    if (aType == KAknsMessageSkinChange)
        {
        TRAP_IGNORE(SetIconsL());
        }
        
    CCoeControl::HandleResourceChange(aType);
    }

/*
void CAspSettingDialog::HandleResourceChange(TInt aType)
    {   
    if (aType == KEikDynamicLayoutVariantSwitch) //Handle change in layout orientation
        {
  
        TRect mainPaneRect;
        AknLayoutUtils::LayoutMetricsRect(AknLayoutUtils::EMainPane, mainPaneRect);
        TBuf<299> buf;
        buf.Format(_L("(%d,%d) (%d,%d)"), mainPaneRect.iTl.iX, mainPaneRect.iTl.iY, mainPaneRect.iBr.iX, mainPaneRect.iBr.iY);

        SetRect(mainPaneRect);
        iSettingListBox->HandleResourceChange(aType);
		DrawNow();
  
        
        CAknDialog::HandleResourceChange(aType);
        
        TRect mainPaneRect;
        AknLayoutUtils::LayoutMetricsRect(AknLayoutUtils::EMainPane, mainPaneRect);
        SetRect(mainPaneRect);
        
        iSettingListBox->SetSize(mainPaneRect.Size());
        //iSettingListBox->HandleResourceChange(aType);
        CCoeControl::HandleResourceChange(aType);
		DrawDeferred();
		
		
	    return;
		}
		
    if (aType == KAknsMessageSkinChange)
        {
        TRAP_IGNORE(SetIconsL());
        }
        
    CCoeControl::HandleResourceChange(aType);
    }
*/

// ----------------------------------------------------------------------------
// CAspSettingDialog::SetIconsL
//
// ----------------------------------------------------------------------------
//
void CAspSettingDialog::SetIconsL()
    {
    if (!iSettingListBox)
    	{
    	return;
    	}
 
 	TFileName bitmapName;
	CAspResHandler::GetBitmapFileName(bitmapName);
	CArrayPtr<CGulIcon>* icons = new (ELeave) CAknIconArray(KDefaultArraySize);
	CleanupStack::PushL(icons);
	
	// NOTE: icons must be appended in correct order (TAspSettingDialogIconIndex)
	icons->AppendL(IconL(KAknsIIDQgnPropCheckboxOn, KAvkonBitmapFile, 
	                     EMbmAvkonQgn_prop_checkbox_on,
	                     EMbmAvkonQgn_prop_checkbox_on_mask));
	                     
	icons->AppendL(IconL(KAknsIIDQgnPropCheckboxOff, KAvkonBitmapFile,
	                     EMbmAvkonQgn_prop_checkbox_off,
	                     EMbmAvkonQgn_prop_checkbox_off_mask));
	
    // delete old icons
    CArrayPtr<CGulIcon>* arr = iSettingListBox->ItemDrawer()->FormattedCellData()->IconArray();
    if (arr)
    	{
    	arr->ResetAndDestroy();
	    delete arr;
	    arr = NULL;
    	}

	iSettingListBox->ItemDrawer()->FormattedCellData()->SetIconArrayL(icons);
	CleanupStack::Pop(icons);
    }


// ----------------------------------------------------------------------------
// CAspSettingDialog::ShowContextMenuL
// 
// ----------------------------------------------------------------------------
//
void CAspSettingDialog::ShowContextMenuL(TInt aResource)
	{
	if (SyncRunning())
		{
		return;
		}
	
	//
	// Switch to Context specific options menu,
	// Show it and switch back to main options menu.
	// 
	CEikMenuBar* menuBar = iMenuBar; // from CAknDialog
	menuBar->SetMenuTitleResourceId(aResource);


	// TRAP displaying of menu bar.
	// If it fails, the correct resource is set back before leave.
	TRAPD(err, menuBar->TryDisplayMenuBarL());
	
	menuBar->SetMenuTitleResourceId(R_ASP_SETTING_DIALOG_MENU);
		
	User::LeaveIfError(err);
	}


// -----------------------------------------------------------------------------
// CAspSettingDialog::PostLayoutDynInitL
//
// Called by framework after dialog is shown.
// -----------------------------------------------------------------------------
//
void CAspSettingDialog::PostLayoutDynInitL()
	{
	#ifdef RD_DSUI_TIMEDSYNC
	ShowAutoSyncProfileInfo();
	#else
  ShowCurrentProfileInfo();
  #endif
	}
	

// -----------------------------------------------------------------------------
// CAspSettingDialog::PreLayoutDynInitL
//
// Called by framework before dialog is shown.
// -----------------------------------------------------------------------------
//
void CAspSettingDialog::PreLayoutDynInitL()
    {
    iSettingListBox = (CAknDoubleLargeStyleListBox*) ControlOrNull (EAspSettingDialogList);
    
   	__ASSERT_ALWAYS(iSettingListBox, TUtil::Panic(KErrGeneral));
	
	iSettingListBox->SetListBoxObserver(this);
	iSettingListBox->CreateScrollBarFrameL(ETrue);
	iSettingListBox->ScrollBarFrame()->SetScrollBarVisibilityL(
	                 CEikScrollBarFrame::EOff, CEikScrollBarFrame::EAuto);
	
	SetIconsL();
	UpdateListBoxL();

#ifdef RD_DSUI_TIMEDSYNC 

	UpdateTabsL();
#else

	UpdateNavipaneL(iCurrentProfileName);
#endif

	
	iSettingListBox->UpdateScrollBarsL();
	iSettingListBox->ScrollBarFrame()->MoveVertThumbTo(0);

	iStatusPaneHandler->SetTitleL(R_ASP_SETTING_DIALOG_TITLE);

	SetCurrentIndex();
//#ifdef RD_DSUI_CP_INTEGRATION
	TBool flag = CEikonEnv::Static()->StartedAsServerApp() ;
	if (flag || iApplicationId != EApplicationIdSync)
/*#else
	if (iApplicationId != EApplicationIdSync)
#endif */
		{
		UpdateCbaL(R_ASP_CBA_OPTIONS_BACK_MARK);
		}
		
	if(iSettingEnforcement)
	{
		UpdateMiddleSoftKeyLabelL(EAknSoftkeyOpen,R_TEXT_SOFTKEY_EMPTY);	
   	}
	}


// -----------------------------------------------------------------------------
// CAspSettingDialog::DynInitMenuPaneL
//
// Called by framework before menu is shown.
// -----------------------------------------------------------------------------
//
void CAspSettingDialog::DynInitMenuPaneL(TInt aResourceID, 	CEikMenuPane* aMenuPane)
	{
	TInt menu1 = R_ASP_SETTING_DIALOG_MENU_PANE;
	TInt menu2 = R_ASP_SETTING_DIALOG_CONTEXT_MENU_PANE;
	
    if (aResourceID != menu1 && aResourceID != menu2)
		{
		return; // not one of our menus
		}

	if (aResourceID == R_ASP_SETTING_DIALOG_CONTEXT_MENU_PANE)
		{
		InitContextMenuL(aMenuPane); // separate handling for context menu
		}
	else
		{
		InitOptionsMenuL(aMenuPane);
		}
	}


// -----------------------------------------------------------------------------
// CAspSettingDialog::InitContextMenuL
//
// -----------------------------------------------------------------------------
//
void CAspSettingDialog::InitContextMenuL(CEikMenuPane* /*aMenuPane*/)
	{
	}


// -----------------------------------------------------------------------------
// CAspSettingDialog::InitOptionsMenuL
//
// -----------------------------------------------------------------------------
//
void CAspSettingDialog::InitOptionsMenuL(CEikMenuPane* aMenuPane)
	{
	TInt profileCount = iProfileList->Count();
	TInt providerCount = iContentList->ProviderCount();
	TInt deletableProfileCount = iProfileList->DeletableProfileCount();
	
	if (providerCount == 0)  // no data providers
		{
		TDialogUtil::DeleteMenuItem(aMenuPane, EAspMenuCmdSync);
		TDialogUtil::DeleteMenuItem(aMenuPane, EAspMenuCmdNewSet);
		TDialogUtil::DeleteMenuItem(aMenuPane, EAspMenuCmdEdit);
		TDialogUtil::DeleteMenuItem(aMenuPane, EAspMenuCmdChangeProfile);
		TDialogUtil::DeleteMenuItem(aMenuPane, EAspMenuCmdDelete);
		TDialogUtil::DeleteMenuItem(aMenuPane, EAspMenuCmdAutoSync);
		TDialogUtil::DeleteMenuItem(aMenuPane, EAspMenuCmdViewLog);
		TDialogUtil::DeleteMenuItem(aMenuPane, EAspMenuCmdCopyFromServer);
		return;
    	}
	
	
	// Options/Sync
	// Options/Copy all from server
	if (iCurrentProfileId == KErrNotFound && profileCount > 0)
		{
		TDialogUtil::DeleteMenuItem(aMenuPane, EAspMenuCmdSync);
		TDialogUtil::DeleteMenuItem(aMenuPane, EAspMenuCmdCopyFromServer);
		}
	if (iSettingEnforcement && profileCount == 0)
		{
		TDialogUtil::DeleteMenuItem(aMenuPane, EAspMenuCmdSync);
		TDialogUtil::DeleteMenuItem(aMenuPane, EAspMenuCmdCopyFromServer);
		}

	if (iCurrentProfileId == KErrNotFound)
		{
		TDialogUtil::DeleteMenuItem(aMenuPane, EAspMenuCmdCopyFromServer);
		}

	// Options/Change
	if (profileCount == 0)
		{
		TDialogUtil::DeleteMenuItem(aMenuPane, EAspMenuCmdChangeProfile);
		TDialogUtil::DeleteMenuItem(aMenuPane, EAspMenuCmdMark);
	    TDialogUtil::DeleteMenuItem(aMenuPane, EAspMenuCmdUnmark);	 
		}
	if (profileCount == 1 && iCurrentProfileId != KErrNotFound)
		{
		TDialogUtil::DeleteMenuItem(aMenuPane, EAspMenuCmdChangeProfile);
		}
	
	// Options/Edit
	if (iCurrentProfileId == KErrNotFound)
		{
		TDialogUtil::DeleteMenuItem(aMenuPane, EAspMenuCmdEdit);
		}
	
	// Options/Auto sync
	if (iCurrentProfileId == KErrNotFound)
		{
		TDialogUtil::DeleteMenuItem(aMenuPane, EAspMenuCmdAutoSync);
		}

	// Options/New
	if (iSettingEnforcement)
		{
		TDialogUtil::DeleteMenuItem(aMenuPane, EAspMenuCmdNewSet);
		TDialogUtil::DeleteMenuItem(aMenuPane, EAspMenuCmdMark);
	    TDialogUtil::DeleteMenuItem(aMenuPane, EAspMenuCmdUnmark);	 
		}

	// Options/Delete   
   if (IsSyncOnGoingL(iCurrentProfileId) || iSettingEnforcement 
   										|| deletableProfileCount == 0)
    	{
    	TDialogUtil::DeleteMenuItem(aMenuPane, EAspMenuCmdDelete);
     	}
	
	// Options/View log
	if (iCurrentProfileId == KErrNotFound)
		{
		TDialogUtil::DeleteMenuItem(aMenuPane, EAspMenuCmdViewLog);
		}
	
	if (!FeatureManager::FeatureSupported(KFeatureIdHelp))
		{
		TDialogUtil::DeleteMenuItem(aMenuPane, EAknCmdHelp);
		}
		
#ifdef RD_DSUI_TIMEDSYNC 
	if(profileCount)
		{
	 	//Delete active profile
	 	TAspParam param(iApplicationId, iSyncSession);
	 	TAspProfileItem& currentProfile = CurrentProfileL();
	 	if(!currentProfile.iDeleteAllowed)
	 		{
	    	TDialogUtil::DeleteMenuItem(aMenuPane, EAspMenuCmdDelete);
			}
	 	//Change profile option is removed
	 	TDialogUtil::DeleteMenuItem(aMenuPane, EAspMenuCmdChangeProfile);
	 	

	 	CAspSchedule* schedule = CAspSchedule::NewLC();
	 	TInt asProfileId = schedule->AutoSyncProfileId();
	 	if (iCurrentProfileId == schedule->ProfileId()
	 							&& asProfileId != KErrNotFound)
			{
			CAspProfile* asProfile = CAspProfile::NewLC(param);
			asProfile->OpenL(asProfileId, CAspProfile::EOpenRead, CAspProfile::EAllProperties);
			if (!asProfile->IsSynced() && !currentProfile.iSynced)
				{
				TDialogUtil::DeleteMenuItem(aMenuPane, EAspMenuCmdViewLog);
				}
			CleanupStack::PopAndDestroy(asProfile);
			}
	 	else
			{
			if (!currentProfile.iSynced)
		    	{
	        	TDialogUtil::DeleteMenuItem(aMenuPane, EAspMenuCmdViewLog);
		    	}

			}
		 CleanupStack::PopAndDestroy(schedule);
		}
#else
	 
	 
	 TAspProfileItem& profile = CurrentProfileL();
     if (!profile.iSynced)
		 {
	         iProfileList->ReadProfileL(iCurrentProfileId);
	       }
	  
	   if (!profile.iSynced)
	           {
	     TDialogUtil::DeleteMenuItem(aMenuPane, EAspMenuCmdViewLog);
		 }
#endif
if(!iSettingEnforcement)
{
	UpdateMarkMenuItem(aMenuPane);
}
#ifndef RD_DSUI_TIMEDSYNC
     TDialogUtil::DeleteMenuItem(aMenuPane, EAspMenuCmdAutoSync);
#endif

	}

// -----------------------------------------------------------------------------
// CAspSettingDialog::IsSyncOnGoing
//
// -----------------------------------------------------------------------------
//
TBool CAspSettingDialog::IsSyncOnGoingL(TInt aProfileId)
	{
	TInt currentJobId = CAspProfile::CurrentJob(iSyncSession);
	if (currentJobId != KErrNotFound)
			{
			RSyncMLDataSyncJob syncJob;
			syncJob.OpenL(*iSyncSession, currentJobId);
			if (syncJob.Profile() == aProfileId)
				{
				syncJob.Close();
				return ETrue;
				}
			syncJob.Close();
			}
	return EFalse;
	}

	
#ifdef RD_DSUI_TIMEDSYNC 
// -----------------------------------------------------------------------------
// CAspSettingDialog::SetNextProfileL
//
// -----------------------------------------------------------------------------
//
void CAspSettingDialog::SetNextProfileL()
	{
    FLOG( _L("CAspSettingDialog::SetNextProfileL START") );

	if (iProfileList->Count() == 0)
		{
		return;
		}
    
    TInt currentIndex = iProfileList->ListIndex(iCurrentProfileId);
    if (currentIndex == KErrNotFound)
    	{
    	FLOG( _L("CAspSettingDialog::SetNextProfileL END") );
    	
    	User::Leave(KErrNotFound);
    	}
    
    TInt newIndex = 0;
    TInt count = iProfileList->Count();
    if (currentIndex >= count-1)
        {
        newIndex = 0;
        }
    else
        {
        newIndex = currentIndex + 1;
        }
    TAspProfileItem& olditem = iProfileList->Item(currentIndex);        
    TAspProfileItem& item = iProfileList->Item(newIndex);

	if (item.iProfileName.Compare(KAutoSyncProfileName) == 0)
			{
			if(newIndex >= count-1)
				{
				newIndex = 0;
				}
			else
				{
				newIndex++;
				}
			TAspProfileItem& newItem = iProfileList->Item(newIndex);
			SetCurrentProfile(newItem.iProfileId);
			}
	else 
		{
		SetCurrentProfile(item.iProfileId);
		}
	
	//ShowCurrentProfileInfo();
    UpdateListBoxL();
	//UpdateNavipaneL(iCurrentProfileName);
    UpdateTabsL();
        
    FLOG( _L("CAspSettingDialog::SetNextProfileL END") );
	}


// -----------------------------------------------------------------------------
// CAspSettingDialog::SetPreviousProfileL
//
// -----------------------------------------------------------------------------
//
void CAspSettingDialog::SetPreviousProfileL()
	{
    FLOG( _L("CAspSettingDialog::SetPreviousProfileL START") );

	if (iProfileList->Count() == 0)
		{
		return;
		}
    
    TInt currentIndex = iProfileList->ListIndex(iCurrentProfileId);
    if (currentIndex == KErrNotFound)
    	{
    	FLOG( _L("CAspSettingDialog::SetNextProfileL END") );
    	
    	User::Leave(KErrNotFound);
    	}
    
    TInt newIndex = 0;
    TInt count = iProfileList->Count();
    if (currentIndex == 0)
        {
        newIndex = count - 1;
        }
    else
        {
        newIndex = currentIndex - 1;
        }
   
    TAspProfileItem& item = iProfileList->Item(newIndex);

	if (item.iProfileName.Compare(KAutoSyncProfileName) == 0)
		{
		if(newIndex == 0)
			{
				newIndex = count - 1;
			}
		else
			{
				newIndex--;
			}
			TAspProfileItem& newItem = iProfileList->Item(newIndex);
			SetCurrentProfile(newItem.iProfileId);
		}
	else
		{
		 SetCurrentProfile(item.iProfileId);	
		}

   
	//ShowCurrentProfileInfo();
    UpdateListBoxL();
	//UpdateNavipaneL(iCurrentProfileName);
	UpdateTabsL();

    FLOG( _L("CAspSettingDialog::SetPreviousProfileL END") );
	}


// -----------------------------------------------------------------------------
// CAspSettingDialog::UpdateTabsL
//
// -----------------------------------------------------------------------------
//
void CAspSettingDialog::UpdateTabsL()
	{
    FLOG( _L("CAspSettingDialog::UpdateTabsL START") );

	TInt count = iProfileList->Count();
	if (count == 0)
		{
		//handle no profile condition
		iTabHandler->DeleteTabsL();
		_LIT(KEmpty, " ");
		TBuf<64> tabText(KEmpty);
		TInt profileId = KMaxProfileCount;
		iTabHandler->AddTabL(tabText, profileId);
		iTabHandler->SetActiveTabL(profileId);
		return;
		}

	if (iTabHandler->TabIndex(iCurrentProfileId) == KErrNotFound)
		{
		iUpdateTabGroup = ETrue;
		}
	if (iUpdateTabGroup)
		{
		//update changed settings
	    iProfileList->ReadAllProfilesL(CAspProfileList::ENoMandatoryCheck);
		iProfileList->Sort();
    	SetCurrentProfile(iCurrentProfileId);
		count = iProfileList->Count();
		iTabHandler->DeleteTabsL();
        for (TInt i=0; i<count; i++)
        	{
        	TAspProfileItem& item = iProfileList->Item(i);
			iTabHandler->AddTabL(item.iProfileName, item.iProfileId);
    		}
    	iTabHandler->SetTabWidthL();
		iUpdateTabGroup = EFalse;		
		}
	if (iCurrentProfileId != KErrNotFound)
		{
		iTabHandler->SetTabTextL(iCurrentProfileName, iCurrentProfileId);	
		}
	CAspSchedule* schedule = CAspSchedule::NewLC();
	if (iCurrentProfileId == schedule->ProfileId()
							 && schedule->IsAutoSyncEnabled())
		{
		//show indicator icon for profile that has auto sync ON.
		_LIT(KSpace,"  ");
		TBuf<64> tabText;
		tabText.Append(KClockCharacter);//Unicode character U+F815 (clock)
		tabText.Append(KSpace);
		tabText.Append(iCurrentProfileName);
		iTabHandler->SetTabTextL(tabText, iCurrentProfileId);
		}
		
	if (iCurrentProfileId != KErrNotFound)
        {
        iTabHandler->SetActiveTabL(iCurrentProfileId);    
        }

	CleanupStack::PopAndDestroy(schedule);
	
	
    FLOG( _L("CAspSettingDialog::UpdateTabsL END") );
	}

#endif
// -----------------------------------------------------------------------------
// CAspSettingDialog::UpdateMarkMenuItem
//
// -----------------------------------------------------------------------------
//

void CAspSettingDialog::UpdateMarkMenuItem(CEikMenuPane* aMenuPane)
	{
	
	CListBoxView* view = iSettingListBox->View();
    
    TInt index = view->CurrentItemIndex();
    TBool mark = view->ItemIsSelected(index);
	
	aMenuPane->SetItemDimmed(EAspMenuCmdMark,mark);
	aMenuPane->SetItemDimmed(EAspMenuCmdUnmark,!mark);
		
	}

// -----------------------------------------------------------------------------
// CAspSettingDialog::SaveSelection
// Update Mark/Unmark selected from menu 
// -----------------------------------------------------------------------------
//
void CAspSettingDialog::SaveSelectionL()
{
    if (IsProfileLockedL(iCurrentProfileId))
       	{
       	TDialogUtil::ShowErrorNoteL(R_ASP_LOCKED_PROFILE);
		return;
		}
       
       if (iSettingEnforcement)
		{
		TDialogUtil::ShowInformationNoteL(R_ASP_PROTECTED_SETTING);
		return; 
		}
		
	//update check box & profile
    CListBoxView* view = iSettingListBox->View();
    
    TInt index = view->CurrentItemIndex();
    TBool selected = view->ItemIsSelected(index);
    if (selected)
    	{
        view->DeselectItem(index);
       	}
    else
    	{
    	view->SelectItemL(index);
    	}
    iSettingListBox->SetCurrentItemIndex(index);
    if (!CheckEmailSelection())
       	{
       	return; 
       	}
	TInt ret =  SaveCurrentProfile();
	if (ret != KErrNone)
		{
		CancelCheckboxEvent();
		}
	
}



// -----------------------------------------------------------------------------
// CAspSettingDialog::DoSetCurrentProfileL
//
// -----------------------------------------------------------------------------
//
void CAspSettingDialog::DoSetCurrentProfileL(TInt aProfileId)
	{
    FLOG( _L("CAspSettingDialog::DoSetCurrentProfileL START") );
    
    iCurrentProfileId = KErrNotFound;
    iCurrentProfileName = KNullDesC;
    iContentList->RemoveAllTasks();
    iProfileList->ReadAllProfilesL(CAspProfileList::ENoMandatoryCheck);
	iProfileList->Sort();
	
	if (iProfileList->Count() == 0)
		{
		return;
		}
        
    if (aProfileId == KErrNotFound)
    	{
    	FLOG( _L("CAspSettingDialog::DoSetCurrentProfileL END") );
    	User::Leave(KErrNotFound);
    	}
    	
    TInt index = iProfileList->ListIndex(aProfileId);
    if (index == KErrNotFound)
    	{
    	FLOG( _L("### unknown profile (id=%d) ###"), aProfileId );
    	FLOG( _L("CAspSettingDialog::DoSetCurrentProfileL END") );
    	
    	User::Leave(KErrNotFound);
    	}
    
    TAspParam param(iApplicationId, iSyncSession);
	CAspProfile* profile = CAspProfile::NewLC(param);
	
	//if profile already locked ,open as read only
	if (IsProfileLockedL(aProfileId))
	{
	   profile->OpenL(aProfileId, CAspProfile::EOpenRead, CAspProfile::EAllProperties);
    
	}
	else
	{
	   profile->OpenL(aProfileId, CAspProfile::EOpenReadWrite, CAspProfile::EAllProperties);
	}
	
    iContentList->SetProfile(profile);
    
    // read sync tasks from database into content list
    iContentList->InitAllTasksL();
    if (IsProfileLockedL(aProfileId))
    {
    	iContentList->UpdateLocalDatabaseL();
    }
    
    if (!iContentList->CheckTaskDirectionsL())
       	{
        if (!iSettingEnforcement)
           	{
           	iContentList->ModifyTaskDirectionsL(ESyncDirectionTwoWay);
    	   	}
    	   	
    	FLOG( _L("### sync direction conflict ###") );
    	}
    
    //check if mailbox exists
    CDesCArray* localDataStores = new (ELeave) CDesCArrayFlat(KDefaultArraySize);
	CleanupStack::PushL(localDataStores);
	iContentList->GetLocalDatabaseList(KUidNSmlAdapterEMail.iUid, localDataStores);

    TInt num = profile->ProtocolVersion();
    if (num == EAspProtocol_1_1 || localDataStores->Count() == 0)
    	{
    	TInt index = iContentList->FindTaskIdForProvider( KUidNSmlAdapterEMail.iUid);
		if (index != KErrNotFound)
			{
			TAspTaskItem emailItem = iContentList->ReadTaskItemL(profile->Profile(), index);
			if (emailItem.iEnabled)
				{
		    	iContentList->ModifyTaskIncludedL(KUidNSmlAdapterEMail.iUid, EFalse, KNullDesC);
				}
			}
    	}
	CleanupStack::PopAndDestroy(localDataStores);
	
    iContentList->SetProfile(NULL);
    
    iCurrentProfileId = aProfileId;
    
    profile->GetName(iCurrentProfileName);
  	if (TUtil::IsEmpty(iCurrentProfileName))
		{
		CAspResHandler::ReadL(iCurrentProfileName, R_ASP_UNNAMED_SET);
		}

  
    CleanupStack::PopAndDestroy(profile);
    
    FLOG( _L("CAspSettingDialog::DoSetCurrentProfileL END") );
	}
	

// -----------------------------------------------------------------------------
// CAspSettingDialog::SetCurrentProfile
//
// -----------------------------------------------------------------------------
//
void CAspSettingDialog::SetCurrentProfile(TInt aProfileId)
	{
	if (iContentList->IsLocked())
		{
		return; // iContentList used by CAspProfileDialog
		}
		
	TInt err = KErrNone;
	TInt oldCurrentProfileId = iCurrentProfileId;
	
	TRAP(err, DoSetCurrentProfileL(aProfileId));
	
	iContentList->SetProfile(NULL);
	
    if (err != KErrNone)
    	{
        iCurrentProfileId = KErrNotFound;
        iCurrentProfileName = KNullDesC;
        iContentList->RemoveAllTasks();
       	}
       	
    if (err != KErrNone && oldCurrentProfileId != KErrNotFound)
    	{
   		// some error - try restoring old current profile
   		TRAP(err, DoSetCurrentProfileL(oldCurrentProfileId));
   		
   		iContentList->SetProfile(NULL);
	
	    if (err != KErrNone)
    	    {
            iCurrentProfileId = KErrNotFound;
    	    iCurrentProfileName = KNullDesC;
            iContentList->RemoveAllTasks();
    	    }
    	}
	}


// -----------------------------------------------------------------------------
// CAspSettingDialog::SetCurrentProfile
//
// -----------------------------------------------------------------------------
//
void CAspSettingDialog::SetCurrentProfileL()
	{

#ifdef RD_DSUI_TIMEDSYNC //display profiles in alphabetic order.
	TInt index = 0;
	if (!iProfileList->Count())
		{
		index = KErrNotFound;
		}

#else
	TInt index = iProfileList->FindLastSyncedProfileIndexL();

#endif
	if (index != KErrNotFound)
		{
		TAspProfileItem& item = iProfileList->Item(index);
		SetCurrentProfile(item.iProfileId);
		}
	else
		{
		SetCurrentProfile(KErrNotFound);
		}
	}


// -----------------------------------------------------------------------------
// CAspSettingDialog::CurrentProfileL
//
// -----------------------------------------------------------------------------
//
TAspProfileItem& CAspSettingDialog::CurrentProfileL()
	{
	TInt index = iProfileList->ListIndex(iCurrentProfileId);
	if (index == KErrNotFound)
		{
		User::Leave(KErrNotFound);
		}
		
	return iProfileList->Item(index);
	}


// -----------------------------------------------------------------------------
// CAspSettingDialog::HasCurrentProfile
//
// -----------------------------------------------------------------------------
//
TBool CAspSettingDialog::HasCurrentProfile()
	{
	TInt ret = ETrue;
	
	if (iCurrentProfileId == KErrNotFound)
		{
		ret = EFalse;
		}
		
	if (iProfileList->Count() == 0)
		{
		ret = EFalse;
		}
		
	if (ret)
		{
	    TInt index = iProfileList->ListIndex(iCurrentProfileId);	
	    
	    __ASSERT_DEBUG(index != KErrNotFound, TUtil::Panic(KErrGeneral));
		}
	
	return ret;
	}


// -----------------------------------------------------------------------------
// CAspSettingDialog::IsCurrentProfile
//
// -----------------------------------------------------------------------------
//
TBool CAspSettingDialog::IsCurrentProfile(TInt aProfileId)
	{
	if (aProfileId != KErrNotFound && aProfileId == iCurrentProfileId)
		{
		return ETrue;
		}
		
	return EFalse;
	}


// -----------------------------------------------------------------------------
// CAspSettingDialog::DoSaveCurrentProfileL
//
// -----------------------------------------------------------------------------
//
void CAspSettingDialog::DoSaveCurrentProfileL()
	{
	if (iSettingEnforcement)
		{
		return;  // read-only profile
		}

	if (!HasCurrentProfile())
		{
		User::Leave(KErrNotFound);
		}

    TAspParam param(iApplicationId, iSyncSession);
	CAspProfile* profile = CAspProfile::NewLC(param);
    profile->OpenL(iCurrentProfileId, CAspProfile::EOpenReadWrite, 
                                      CAspProfile::EAllProperties);

	iContentList->SetProfile(profile);
	// store selected sync tasks into database
	iContentList->IncludeTasks(iSettingListBox->SelectionIndexes());
	iContentList->SetProfile(NULL);
	
	CleanupStack::PopAndDestroy(profile);
	}


// -----------------------------------------------------------------------------
// CAspSettingDialog::SaveCurrentProfile
//
// -----------------------------------------------------------------------------
//
TInt CAspSettingDialog::SaveCurrentProfile()
	{
	TRAPD(err, DoSaveCurrentProfileL());
	
	iContentList->SetProfile(NULL);
	
    if (err == KErrLocked)
   		{
        TRAP_IGNORE( TDialogUtil::ShowErrorNoteL(R_ASP_LOCKED_PROFILE) );
        }

	if (err != KErrNone)
		{
		FLOG( _L("### CAspSettingDialog::SaveCurrentProfile: err=%d ###"), err );
    	}
    	
    return err;
	}


// -----------------------------------------------------------------------------
// CAspSettingDialog::EditProfileL
//
// -----------------------------------------------------------------------------
//
void CAspSettingDialog::EditProfileL(TInt aEditMode ,TInt aFocus)
	{
	if (iSettingEnforcement)
		{
		aEditMode = EDialogModeSettingEnforcement;
		}
    
    if (IsProfileLockedL(iCurrentProfileId))
		{
		TDialogUtil::ShowErrorNoteL(R_ASP_LOCKED_PROFILE);
		return;
		}
	TRAPD(err, DoEditProfileL(aEditMode ,aFocus));
    iContentList->SetProfile(NULL);
	User::LeaveIfError(err);

#ifdef RD_DSUI_TIMEDSYNC 
	iUpdateTabGroup = ETrue;
	UpdateTabsL();
#endif

	}


// -----------------------------------------------------------------------------
// CAspSettingDialog::DoEditProfileL
//
// -----------------------------------------------------------------------------
//
void CAspSettingDialog::DoEditProfileL(TInt aEditMode, TInt aFocus)
	{
	if (!HasCurrentProfile())
		{
		return;
		}
		
    TAspParam param(iApplicationId, iSyncSession);
	CAspProfile* profile = CAspProfile::NewLC(param);
    
    iContentList->SetProfile(profile);
    
    if (aEditMode == EDialogModeEdit || aEditMode == EDialogModeEditMandatory)
    	{
    	profile->OpenL(iCurrentProfileId, CAspProfile::EOpenReadWrite,
                                          CAspProfile::EAllProperties);
                                          
    	}
    else
    	{
    	profile->OpenL(iCurrentProfileId, CAspProfile::EOpenRead,
                                          CAspProfile::EAllProperties);
    	}

		
	TAspParam param2(iApplicationId, iSyncSession);
	param2.iProfile = profile;
	param2.iMode = aEditMode;
    param2.iContentList = iContentList;
    param2.iProfileList = iProfileList;  // needed for unique server id check
    param2.iDataProviderId = aFocus;
 
#ifdef RD_DSUI_TIMEDSYNC 
	CAspSettingViewDialog::ShowDialogL(param2);
#else
	CAspProfileDialog::ShowDialogL(param2);
#endif

	iContentList->SetProfile(NULL);
	CleanupStack::PopAndDestroy(profile);
	}


// -----------------------------------------------------------------------------
// CAspSettingDialog::CheckMandatoryDataL
//
// -----------------------------------------------------------------------------
//
TInt CAspSettingDialog::CheckMandatoryDataL()
	{
	TInt ret = EMandatoryOk;
	
	TRAPD(err, ret = DoCheckMandatoryDataL());
	
	User::LeaveIfError(err);
		
	return ret;
	}


// -----------------------------------------------------------------------------
// CAspSettingDialog::DoCheckMandatoryDataL
//
// -----------------------------------------------------------------------------
//
TInt CAspSettingDialog::DoCheckMandatoryDataL()
	{
	if (!HasCurrentProfile())
		{
		User::Leave(KErrNotFound);
		}
		
    TAspParam param(iApplicationId, iSyncSession);
	CAspProfile* profile = CAspProfile::NewLC(param);
    profile->OpenL(iCurrentProfileId, CAspProfile::EOpenReadWrite,
                                      CAspProfile::EAllProperties);
    
    
    TInt mandatoryConnectionData = CAspProfile::CheckMandatoryConnData(profile);
    iContentList->SetProfile(profile);	
	TInt contentCount = 0;
	TInt mandatoryContentData = iContentList->CheckMandatoryDataL(contentCount);
	iContentList->SetProfile(NULL);
	CleanupStack::PopAndDestroy(profile);

	if (mandatoryConnectionData != EMandatoryOk)
		{
		return mandatoryConnectionData;
		}
		
	return mandatoryContentData;
	}


// -----------------------------------------------------------------------------
// CAspSettingDialog::DeleteProfileL
//
// -----------------------------------------------------------------------------
//
void CAspSettingDialog::DeleteProfileL()
    {

#ifdef RD_DSUI_TIMEDSYNC 
	 TAspParam param(iApplicationId, iSyncSession);
	 CAspProfile* profile = CAspProfile::NewLC(param);
     TRAPD(err,profile->OpenL(iCurrentProfileId, CAspProfile::EOpenReadWrite,
                                      CAspProfile::EAllProperties));

	 if (err == KErrLocked)
   		{
        TDialogUtil::ShowErrorNoteL(R_ASP_LOCKED_PROFILE);
		CleanupStack::PopAndDestroy(profile);
		return;
	 	}
	 CAspSchedule* schedule = CAspSchedule::NewLC();
	
	 TInt profileId = schedule->ProfileId();
	 if (profileId == iCurrentProfileId && schedule->IsAutoSyncEnabled())
	 	{
		HBufC* hBuf = CAspResHandler::ReadLC(R_ASP_DS_CONF_QUERY_DELETE_AUTO_SYNC_PROFILE);
		if (!TDialogUtil::ShowConfirmationQueryL(hBuf->Des()))
			{
			CleanupStack::PopAndDestroy(hBuf);
			CleanupStack::PopAndDestroy(schedule);
	        CleanupStack::PopAndDestroy(profile);
			return;
			}
		CleanupStack::PopAndDestroy(hBuf);
		schedule->SetProfileId(KErrNotFound);
		schedule->UpdateSyncScheduleL();
		schedule->SaveL();
		}
	 else
	 	{
		TBuf<KBufSize> profileName;
	 	profile->GetName(profileName);
	 	HBufC* hBuf1 = CAspResHandler::ReadLC(R_ASP_QUERY_COMMON_CONF_DELETE, profileName);

	 	if (!TDialogUtil::ShowConfirmationQueryL(hBuf1->Des()))
		    {
			CleanupStack::PopAndDestroy(hBuf1);
			CleanupStack::PopAndDestroy(schedule);
			CleanupStack::PopAndDestroy(profile);
			return;
	 		}
		CleanupStack::PopAndDestroy(hBuf1);
	 	}
	CleanupStack::PopAndDestroy(schedule);
	CleanupStack::PopAndDestroy(profile);
	if (!IsSyncOnGoingL(iCurrentProfileId))
		{
		TRAPD (err1,iSyncSession->DeleteProfileL(iCurrentProfileId));
		
		if (err1 == KErrLocked)
   			{
       		TDialogUtil::ShowErrorNoteL(R_ASP_LOCKED_PROFILE);
       		}
    	User::LeaveIfError(err);
		}
	iUpdateTabGroup = ETrue;	
	SetPreviousProfileL();
	
	
#else

    TAspFilterInfo info;
    info.iFilterType = TAspFilterInfo::EIncludeDeletableProfile;
    CAspProfileList* filteredList = iProfileList->FilteredListL(info);
    CleanupStack::PushL(filteredList);
    
	TInt count = filteredList->Count();
	TInt err = KErrNone;

	if (count == 1)
		{
		TAspProfileItem& profile = filteredList->Item(0);

		HBufC* hBuf = CAspResHandler::ReadLC(R_ASP_CONFIRM_DELETE, profile.iProfileName);
	    if (TDialogUtil::ShowConfirmationQueryL(hBuf->Des()))
		    {
		    if (!IsSyncOnGoingL(profile.iProfileId))
		    	{
		    	TRAP(err, iSyncSession->DeleteProfileL(profile.iProfileId));
		    	}
		    }
   		
   		CleanupStack::PopAndDestroy(hBuf);
    	}
	
	else if (count > 0)
		{
		TInt profileId = KErrNotFound;
	    if (TDialogUtil::ShowProfileQueryL(filteredList, profileId,
	                                       R_ASP_QUERY_TITLE_DELETE_PROFILE))
		    {
		    if (!IsSyncOnGoingL(profileId))
		    	{
		    	TRAP(err, iSyncSession->DeleteProfileL(profileId));
		    	}
		    }
		}

	if (err == KErrLocked)
   		{
        TDialogUtil::ShowErrorNoteL(R_ASP_LOCKED_PROFILE);
        }
    User::LeaveIfError(err);
    
	CleanupStack::PopAndDestroy(filteredList);

#endif
    }


// -----------------------------------------------------------------------------
// CAspSettingDialog::ShowLogDialogL
//
// -----------------------------------------------------------------------------
//
void CAspSettingDialog::ShowLogDialogL()
    {
    
	TAspParam param(iApplicationId, iSyncSession);
	TAspProfileItem& profile = CurrentProfileL();
	param.iProfileId = profile.iProfileId;

#ifdef RD_DSUI_TIMEDSYNC 
   	
	CAspSchedule* schedule = CAspSchedule::NewLC();
	if (iCurrentProfileId == schedule->ProfileId())
		{
		TInt asProfileId = schedule->AutoSyncProfileId();
		CAspProfile* asProfile = CAspProfile::NewLC(param);
		asProfile->OpenL(asProfileId, CAspProfile::EOpenRead, CAspProfile::EAllProperties);

		/*if (asProfile->LastSync() > profile.iLastSync)
			{
			param.iProfileId = asProfileId;
			}*/	
		TTime now;
		now.HomeTime();
		if (profile.iLastSync == 0)
			{
			param.iProfileId = asProfileId;
			}
		else if ((asProfile->LastSync()< now && profile.iLastSync < now)
								|| (asProfile->LastSync() > now && profile.iLastSync > now))
			{
			if (asProfile->LastSync() > profile.iLastSync)
				{
				param.iProfileId = asProfileId;
				}
			}
		else if	((asProfile->LastSync()< now && profile.iLastSync > now))
			{
			param.iProfileId = asProfileId;
			}
		CleanupStack::PopAndDestroy(asProfile);
		}
	
	CleanupStack::PopAndDestroy(schedule);
		
#endif   
	

    CAspLogDialog* dialog = CAspLogDialog::NewL(param);
    CleanupStack::PushL(dialog);
        
    dialog->ShowDialogL();
        
    CleanupStack::PopAndDestroy(dialog);
    }


// -----------------------------------------------------------------------------
// CAspSettingDialog::ShowAutoSyncDialogL
//
// -----------------------------------------------------------------------------
//
void CAspSettingDialog::ShowAutoSyncDialogL()
    {
    
    CAspSchedule* schedule = CAspSchedule::NewLC();
	if (schedule->IsAutoSyncEnabled())
		{
		TInt profileId = schedule->ProfileId();
		if(profileId != iCurrentProfileId && profileId != KErrNotFound)
			{
			if(!TDialogUtil::ShowConfirmationQueryL(R_ASP_QUERY_AUTO_SYNC_ON))
				{
				CleanupStack::PopAndDestroy(schedule);
				return ;
				}
			}
		}
    CleanupStack::PopAndDestroy(schedule);
	
	TAspParam param(iApplicationId, iSyncSession);
    param.iProfileList = iProfileList;
	param.iProfile = NULL;
	param.iContentList = iContentList;
 	CAspScheduleDialog::ShowDialogL(param);

#ifdef RD_DSUI_TIMEDSYNC 
	UpdateTabsL();
#endif
	
    }


// -----------------------------------------------------------------------------
// CAspSettingDialog::ChangeProfileL
//
// -----------------------------------------------------------------------------
//
void CAspSettingDialog::ChangeProfileL()
    {
    if (iProfileList->Count() == 0)
    	{
    	return;
    	}
    	
	TInt profileId = KErrNotFound;
	if (TDialogUtil::ShowProfileQueryL(iProfileList, profileId, 
	                                   R_ASP_QUERY_TITLE_CHANGE_PROFILE))
		{
		if (profileId == iCurrentProfileId)
			{
			return;
			}
			
		SetCurrentProfile(profileId);
		//ShowCurrentProfileInfo();
		UpdateListBoxL();
		
#ifdef RD_DSUI_TIMEDSYNC //RD_DSUI_NO_TIMEDSYNC

		UpdateTabsL();
#else
		UpdateNavipaneL(iCurrentProfileName);
#endif
		}
    }


// -----------------------------------------------------------------------------
// CAspSettingDialog::CopyFromServerL
//
// -----------------------------------------------------------------------------
//
void CAspSettingDialog::CopyFromServerL()
	{
	if (!TDialogUtil::ShowConfirmationQueryL(R_ASP_COPY_FROM_SERVER))
		{
		return;
		}

	TRAPD(err, DoSyncProfileL(ESyncDirectionRefreshFromServer));
	
	if (err == KErrLocked)
		{
		TDialogUtil::ShowErrorNoteL(R_ASP_LOCKED_PROFILE);
		}
	User::LeaveIfError(err);
	}


// -----------------------------------------------------------------------------
// CAspSettingDialog::SyncProfileL
//
// -----------------------------------------------------------------------------
//
void CAspSettingDialog::SyncProfileL()
	{
		
	if (IsProfileLockedL(iCurrentProfileId))
		{
		TDialogUtil::ShowErrorNoteL(R_ASP_LOCKED_PROFILE);
		return;
		}
	
	TRAPD(err, DoSyncProfileL(KErrNotFound));
	User::LeaveIfError(err);
	}


// -----------------------------------------------------------------------------
// CAspSettingDialog::DoSyncProfileL
//
// -----------------------------------------------------------------------------
//
void CAspSettingDialog::DoSyncProfileL(TInt aSyncType)
	{
    __ASSERT_ALWAYS(iSyncSession, TUtil::Panic(KErrGeneral));
    	
	if (iProfileList->Count() == 0)
		{
	    if (TDialogUtil::ShowConfirmationQueryL(R_ASP_QUERY_NO_PROFILES))
		    {
		    CreateProfileL();
		    }
		    
		return;
    	}
    
    //check PS key
    TInt keyVal = 0;//EDataSyncNotRunning
    TInt err = RProperty::Get( KPSUidDataSynchronizationInternalKeys,
                                 KDataSyncStatus,
                                 keyVal );
    if(err == KErrNone && keyVal)
    	{
    	TDialogUtil::ShowInformationNoteL(R_ASP_SYNC_ALREADY_RUNNING);
		return;
    	}
    
	
	if (CAspProfile::OtherSyncRunning(iSyncSession))
		{
		TDialogUtil::ShowInformationNoteL(R_ASP_SYNC_ALREADY_RUNNING);
		return;
		}

     // check that valid current profile exists
    TInt mandatoryCheck = CheckMandatoryDataL();
    
 	if (mandatoryCheck == EMandatoryNoContent)
		{
		TDialogUtil::ShowErrorNoteL(R_ASP_ERR_NOCONTENT);
		return;
        }
	else if (mandatoryCheck != EMandatoryOk)
		{
        if (iSettingEnforcement)
    	    {
    	    TDialogUtil::ShowInformationNoteL(R_ASP_NOTE_TARM_MANDATORY_MISSING);
    	    }    	
		else if (TDialogUtil::ShowConfirmationQueryL(R_ASP_QUERY_MANDATORY_MISSING))
			{
			TInt contentCount = 0;
			TInt incompleteContent = KErrNotFound;
			TInt taskIndex = KErrNotFound;
			if (iContentList->CheckMandatoryDataL(contentCount))
				{
				TInt count = iContentList->ProviderCount();
				for (TInt i=0; i<count; i++)
					{
					//data provider ids are sorted before task list creation ,so provider id has to be used
					//to get the correct position of item
					TAspProviderItem& providerItem = iContentList->ProviderItem(i);
					taskIndex = iContentList->FindTaskIndexForProvider(providerItem.iDataProviderId);
					if (taskIndex == KErrNotFound)
						{
						//task is not created
						continue;	
						}
					TAspTaskItem& task = iContentList->TaskItem(taskIndex);
					if (!task.iEnabled)
						{
						continue; // task not part of sync - no need to check remote data base
						}
					if (!task.iRemoteDatabaseDefined)
						{
						incompleteContent = task.iDataProviderId;
						break;
						}
					}
				}
			EditProfileL(EDialogModeEditMandatory ,incompleteContent);
			}
			
		return;
        }


	if (!iSyncHandler)
		{
    	TAspParam param(iApplicationId, NULL);
		iSyncHandler = CAspSyncHandler::NewL(param);
		}
	else
		{
		if (SyncRunning())
			{
			TDialogUtil::ShowInformationNoteL(R_ASP_SYNC_ALREADY_RUNNING);
    		return;
			}
		}
    	
    iSyncHandler->SetObserver(this);

   	TAspSyncParam param(iApplicationId);
	param.iProfileId = iCurrentProfileId;
	if (aSyncType == ESyncDirectionRefreshFromServer)
		{
		param.iSyncDirection = aSyncType;
		}

#ifdef RD_DSUI_TIMEDSYNC 
	DeleteAutoSyncHistory();
#endif

	iSyncHandler->SynchronizeL(param);
	}

// -----------------------------------------------------------------------------
// CAspSettingDialog::DeleteAutoSyncHistory
//
// -----------------------------------------------------------------------------
//
void CAspSettingDialog::DeleteAutoSyncHistory()
	{
	TAspParam param(iApplicationId, iSyncSession);
	CAspSchedule* schedule = CAspSchedule::NewLC();
	if (schedule->IsAutoSyncEnabled() && iCurrentProfileId == schedule->ProfileId())
		{
		TInt asProfileId = schedule->AutoSyncProfileId();
		CAspProfile* asProfile = CAspProfile::NewLC(param);
		asProfile->OpenL(asProfileId, CAspProfile::EOpenReadWrite, CAspProfile::EAllProperties);
		asProfile->DeleteHistory();
		CleanupStack::PopAndDestroy(asProfile);
		}
	CleanupStack::PopAndDestroy(schedule);
	}

// -----------------------------------------------------------------------------
// CAspSettingDialog::CreateProfileL
//
// -----------------------------------------------------------------------------
//
void CAspSettingDialog::CreateProfileL()
	{
	TAspParam param(iApplicationId, iSyncSession);
	param.iProfileList = iProfileList;
	
	CAspProfileWizardDialog::ShowDialogL(param);

#ifdef RD_DSUI_TIMEDSYNC 
	iUpdateTabGroup = ETrue;
#endif

	if (param.iReturnValue == KErrNone && param.iProfileId != KErrNotFound)
		{
		SetCurrentProfile(param.iProfileId);
		UpdateListBoxL();
#ifdef RD_DSUI_TIMEDSYNC 
	
		UpdateTabsL();
#else
		UpdateNavipaneL(iCurrentProfileName);
#endif
		}
	}


// -----------------------------------------------------------------------------
// CAspSettingDialog::IconL
// 
// -----------------------------------------------------------------------------
//
CGulIcon* CAspSettingDialog::IconL(TAknsItemID aId, const TDesC& aFileName, TInt aFileIndex, TInt aFileMaskIndex)
	{
    return TDialogUtil::CreateIconL(aId, aFileName, aFileIndex, aFileMaskIndex);
	}

	
// -----------------------------------------------------------------------------
// CAspSettingDialog::UpdateListBoxL
//	
// -----------------------------------------------------------------------------
//
void CAspSettingDialog::UpdateListBoxL()
	{
	SetCurrentIndex();
	
	CAknDoubleLargeStyleListBox* listBox = ListBox();
	CDesCArray* items = (CDesCArray*) listBox->Model()->ItemTextArray();
	items->Reset();
    listBox->ItemDrawer()->ClearAllPropertiesL();
   
	TInt count = iContentList->ProviderCount();

	for (TInt i=0; i<count; i++)
		{
		HBufC* firstLine = NULL;
		HBufC* secondLine = NULL;
				
		TAspProviderItem& providerItem = iContentList->ProviderItem(i);
		
		firstLine = CAspResHandler::GetContentNameLC(
		            providerItem.iDataProviderId, providerItem.iDisplayName);
		
		TInt index = iContentList->FindTaskIndexForProvider(providerItem.iDataProviderId);
		if (providerItem.iDataProviderId == KUidNSmlAdapterEMail.iUid )
		{
		
			if(!providerItem.iHasDefaultDataStore)
			{
			TInt providerListIndex = iContentList->FindProviderIndex(providerItem.iDataProviderId);
			
			CListBoxView* view = iSettingListBox->View();

			view->DeselectItem(providerListIndex);
			}
		}
		if (index != KErrNotFound)
			{
			TAspTaskItem& task = iContentList->TaskItem(index);
	
		    if (task.iLastSync != 0) // task has been synchronised
			    {
			    secondLine = TUtil::SyncTimeLC(task.iLastSync);
			    }
		    else
			    {
			    secondLine = CAspResHandler::ReadLC(R_ASP_NOT_SYNCED);
			    }
			}
		else
			{
			secondLine = CAspResHandler::ReadLC(R_ASP_NOT_SYNCED);
			}
		
		
		TPtr ptr1 = firstLine->Des();
		AknTextUtils::DisplayTextLanguageSpecificNumberConversion(ptr1);
		TPtr ptr2 = secondLine->Des();
        AknTextUtils::DisplayTextLanguageSpecificNumberConversion(ptr2);

		HBufC* hBuf = NULL;
		hBuf = TDialogUtil::ConstructListBoxItemLC(firstLine->Des(), secondLine->Des(), 0);

		items->AppendL(hBuf->Des());
		
		CleanupStack::PopAndDestroy(hBuf);
		CleanupStack::PopAndDestroy(secondLine);
		CleanupStack::PopAndDestroy(firstLine);
		}

	listBox->HandleItemAdditionL();
	
	if (listBox->Model()->NumberOfItems() == 0)
		{
	    iStatusPaneHandler->SetNaviPaneTitle(KNullDesC);
		}
	else
		{
		UpdateContentSelectionL();
		
		TInt current = CurrentIndex();
		if (current != KErrNotFound)
			{
			ListBox()->SetCurrentItemIndexAndDraw(current);
			}
		}
	}


// -----------------------------------------------------------------------------
// CAspSettingDialog::UpdateContentSelectionL
//
// -----------------------------------------------------------------------------
//
void CAspSettingDialog::UpdateContentSelectionL()
    {
    // update selected task check boxes from current profile content list
    CListBoxView::CSelectionIndexArray* arrayOfSelectionIndexes = NULL;
    arrayOfSelectionIndexes = new (ELeave) CArrayFixFlat<TInt>(KDefaultArraySize);
    CleanupStack::PushL(arrayOfSelectionIndexes);

	if (HasCurrentProfile())
		{
		iContentList->GetIncludedProviders(arrayOfSelectionIndexes);
		}
        
    iSettingListBox->SetSelectionIndexesL(arrayOfSelectionIndexes);

    CleanupStack::PopAndDestroy(arrayOfSelectionIndexes);
    }
    

// -----------------------------------------------------------------------------
// CAspSettingDialog::UpdateNavipaneL
//
// -----------------------------------------------------------------------------
//
void CAspSettingDialog::UpdateNavipaneL(const TDesC& aText)
	{
	if (iCurrentProfileName.Compare(aText) != 0)
		{
		TUtil::StrCopy(iCurrentProfileName, aText);
		}
		
	iStatusPaneHandler->SetNaviPaneTitleL(iCurrentProfileName);
	}


// -----------------------------------------------------------------------------
// CAspSettingDialog::HandleSyncEventL (from MAspSyncObserver)
//
// -----------------------------------------------------------------------------
//
void CAspSettingDialog::HandleSyncEventL(TInt /*aError*/, TInt aInfo1)
	{
    iDbNotifier->CreateUpdateEventL(aInfo1, KErrNone);
	}


// -----------------------------------------------------------------------------
// CAspSettingDialog::HandleDbEventL (from MAspDbEventHandler)
//
// This function updates UI when it receives sync profile database
// events (see AspDbNotifier).
// -----------------------------------------------------------------------------
//
void CAspSettingDialog::HandleDbEventL(TAspDbEvent aEvent)
	{
	FLOG( _L("CAspSettingDialog::HandleDbEventL START") );

    switch (aEvent.iType)
		{
		case CAspDbNotifier::EClose:
			{
			
#ifdef _DEBUG			
			LogDatabaseEvent(aEvent);
#endif		
	
			TDialogUtil::ShowErrorNoteL(R_ASP_LOG_ERR_SERVERERROR);
            iAvkonAppUi->ProcessCommandL(EAknCmdExit); //  close sync app
			break;
			}
		
		case CAspDbNotifier::EUpdate:
			{
			iProfileList->ReadProfileL(aEvent.iProfileId);
			iProfileList->Sort();
      	    if (IsCurrentProfile(aEvent.iProfileId))
            	{
            	if (!iContentList->IsLocked())
            		{
            		SetCurrentProfile(aEvent.iProfileId);
                    UpdateListBoxL();
#ifdef RD_DSUI_TIMEDSYNC 
					UpdateTabsL();
#else
                    UpdateNavipaneL(iCurrentProfileName);
#endif               
            		}
            	}
			
#ifdef _DEBUG
            LogDatabaseEvent(aEvent);
#endif
			
			break;
			}

		case CAspDbNotifier::EDelete:
			{

#ifdef _DEBUG
            LogDatabaseEvent(aEvent);
#endif

            TInt index = iProfileList->ListIndex(aEvent.iProfileId);
            if (index != KErrNotFound)
            	{
            	iProfileList->Remove(aEvent.iProfileId);
            	if (IsCurrentProfile(aEvent.iProfileId))
            		{
            		if (!iContentList->IsLocked())
            		    {
            		    SetCurrentProfileL(); // set new current profile
                        UpdateListBoxL();
#ifdef RD_DSUI_TIMEDSYNC 
					UpdateTabsL();
#else
                    UpdateNavipaneL(iCurrentProfileName);
#endif            		 
						}
            		}
            	}
			break;
			}

		case CAspDbNotifier::EUpdateAll:
			{

#ifdef _DEBUG			
			LogDatabaseEvent(aEvent);
#endif

			iProfileList->ReadAllProfilesL(CAspProfileList::ENoMandatoryCheck);
			iProfileList->Sort();
           	if (!iContentList->IsLocked())
            	{
            	SetCurrentProfile(iCurrentProfileId); // set new current profile
                UpdateListBoxL();
#ifdef RD_DSUI_TIMEDSYNC 
					UpdateTabsL();
#else
                    UpdateNavipaneL(iCurrentProfileName);
#endif         	
				}

			break;
			}

		default:
			break;
		}
		
	FLOG( _L("CAspSettingDialog::HandleDbEventL END") );
	}
// -----------------------------------------------------------------------------
// CAspSettingDialog::HandleOKL()
// 
// -----------------------------------------------------------------------------
//

void CAspSettingDialog::HandleOKL()
{
	
	if (iProfileList->Count() == 0)
			{
			return;
			}
        
        if (IsProfileLockedL(iCurrentProfileId))
        	{
        	TDialogUtil::ShowErrorNoteL(R_ASP_LOCKED_PROFILE);
			CancelCheckboxEvent();
			return;
			}
        
        if (iSettingEnforcement)
			{
			CancelCheckboxEvent();
			TDialogUtil::ShowInformationNoteL(R_ASP_NOTE_MAIN_LOCKED);
			return ;  // leave dialog open;
			}
        
        if (!CheckEmailSelection())
        	{
        	return ;  // leave dialog open;
        	}
				
		TInt ret =  SaveCurrentProfile();
		if (ret != KErrNone)
			{
			CancelCheckboxEvent();
			}
			
		return ;
}
// -----------------------------------------------------------------------------
// CAspSettingDialog::HandleListBoxEventL (from MEikListBoxObserver)
// 
// -----------------------------------------------------------------------------
//
void CAspSettingDialog::HandleListBoxEventL(CEikListBox* /*aListBox*/, TListBoxEvent aEventType)
	{
	   if( AknLayoutUtils::PenEnabled() )  
	   {
	   	 switch(aEventType)
		{
			case EEventItemSingleClicked :
			     HandleOKL();
			     break;
			default:
			     break;
		}
	   }
	  
    }

// -----------------------------------------------------------------------------
// CAspSettingDialog::HandleActiveCallL (from MAspActiveCallerObserver)
//
// -----------------------------------------------------------------------------
//
void CAspSettingDialog::HandleActiveCallL(TInt /*aCallId*/)
	{
	}


// -----------------------------------------------------------------------------
// CAspSettingDialog::SetCurrentIndex
//
// -----------------------------------------------------------------------------
//
void CAspSettingDialog::SetCurrentIndex()
	{
	TInt count = ListBox()->Model()->NumberOfItems();

	if (count == 0)
		{
		iCurrentListBoxIndex = KErrNotFound;  // empty list
		}
	else
		{
		iCurrentListBoxIndex = ListBox()->CurrentItemIndex();
		}
	}


// -----------------------------------------------------------------------------
// CAspSettingDialog::CurrentIndex
//
// -----------------------------------------------------------------------------
//
TInt CAspSettingDialog::CurrentIndex()
	{
	TInt count = ListBox()->Model()->NumberOfItems();
	
	if (count == 0)
		{
		iCurrentListBoxIndex = KErrNotFound;
		}
	else if (iCurrentListBoxIndex >= count)
		{
		iCurrentListBoxIndex = count-1; // last listbox item
		}
	else if (iCurrentListBoxIndex < 0)
		{
		iCurrentListBoxIndex = 0; // first listbox item
		}
		
	return iCurrentListBoxIndex;
	}


// -----------------------------------------------------------------------------
// CAspSettingDialog::ShowCurrentProfileInfoL
//
// -----------------------------------------------------------------------------
//
void CAspSettingDialog::ShowCurrentProfileInfoL()
	{
	TAspProfileItem& item = CurrentProfileL();
	TBuf<KBufSize> buf;
    iBearerHandler->GetBearerName(buf, item.iBearer);
    HBufC* hBuf = CAspResHandler::ReadProfileInfoTextLC(item.iProfileName, buf);
    ShowPopupNoteL(hBuf->Des());
    CleanupStack::PopAndDestroy(hBuf);
	}


// -----------------------------------------------------------------------------
// CAspSettingDialog::ShowCurrentProfileInfo
//
// -----------------------------------------------------------------------------
//
void CAspSettingDialog::ShowCurrentProfileInfo()
	{
	TRAP_IGNORE(ShowCurrentProfileInfoL());
	}


// -----------------------------------------------------------------------------
// CAspSettingDialog::ShowAutoSyncProfileInfoL
//
// -----------------------------------------------------------------------------
//
TBool CAspSettingDialog::ShowAutoSyncProfileInfo()
	{
	TBool ret = EFalse;
	
	TRAPD(err, ret = ShowAutoSyncProfileInfoL());

#ifdef RD_DSUI_TIMEDSYNC
	UpdateTabsL();	
#endif
	
	if (err != KErrNone)
		{
		return EFalse;
		}
	return ret;
	}

// -----------------------------------------------------------------------------
// CAspSettingDialog::ShowAutoSyncProfileInfoL
//
// -----------------------------------------------------------------------------
//
TBool CAspSettingDialog::ShowAutoSyncProfileInfoL()
	{
	CAspSchedule* schedule = CAspSchedule::NewLC();
	
	if (!schedule->CanSynchronizeL())
    	{
   	    CleanupStack::PopAndDestroy(schedule);
	    return EFalse;
    	}
	
	TInt autoSyncError = schedule->Error();
	if (autoSyncError != KErrNone)
		{

		TInt profileId = schedule->ProfileId();
		TInt asProfileId = schedule->AutoSyncProfileId();
		TAspParam param(iApplicationId, iSyncSession);
		CAspProfile* asProfile = CAspProfile::NewLC(param);
		asProfile->OpenL(asProfileId, CAspProfile::EOpenRead, CAspProfile::EAllProperties);
		CAspProfile* profile = CAspProfile::NewLC(param);
		profile->OpenL(profileId, CAspProfile::EOpenRead, CAspProfile::EAllProperties);

		//display auto sync error query only if profile hasn't been manually synced after last auto sync
		if (asProfile->LastSync() >= profile->LastSync())
			{
			if (autoSyncError == CAspAutoSyncHandler::EOtherSyncRunning)
				{
				HBufC* hBuf = CAspResHandler::ReadLC(R_ASP_MANUAL_SYNC_OVERRIDE);
				_LIT(KEmpty, " ");
				TDialogUtil::ShowMessageQueryL(KEmpty, hBuf->Des());
   	   			CleanupStack::PopAndDestroy(hBuf);
				}
			else
				{
				HBufC* hBuf = CAspResHandler::ReadLC(R_ASP_POPUP_AUTO_SYNC_ERROR);
				_LIT(KNewLine ,"\n");
				TBuf<KBufSize> buf(hBuf->Des());
				buf.Append(KNewLine);
				CAspResHandler::ReadL(iBuf, R_ASP_MAIN_AUTO_ERROR_LOG_LINK);
				buf.Append(iBuf);
				CleanupStack::PopAndDestroy(hBuf);
				hBuf = HBufC::NewLC(buf.Size());
				TPtr ptr(hBuf->Des());
				ptr = buf;

			   	TDialogUtil::ShowAutoSyncMessageQueryL(hBuf->Des());
	   	   		CleanupStack::PopAndDestroy(hBuf);
				}
	  	    
   	    	schedule->SetError(KErrNone);
   	    	schedule->SaveL();

			CleanupStack::PopAndDestroy(profile);
			CleanupStack::PopAndDestroy(asProfile);
			CleanupStack::PopAndDestroy(schedule);
   	    	return ETrue;
			}
		
		CleanupStack::PopAndDestroy(profile);
		CleanupStack::PopAndDestroy(asProfile);
		
		schedule->SetError(KErrNone);
   	    schedule->SaveL();
		}

    
    TTime nextSync;
    schedule->GetStartTimeL(nextSync ,ETrue);

    TInt profileIndex = iProfileList->ListIndex(schedule->ProfileId());
    TAspProfileItem& item = iProfileList->Item(profileIndex); 
    
	HBufC* hBuf1 = CAspResHandler::ReadAutoSyncInfoTextLC( item.iProfileName, nextSync);
	
	TDialogUtil::ShowInformationNoteL(hBuf1->Des());
   	CleanupStack::PopAndDestroy(hBuf1);
    CleanupStack::PopAndDestroy(schedule);
	return ETrue;
	}


// -----------------------------------------------------------------------------
// CAspSettingDialog::ShowPopupNote
//
// -----------------------------------------------------------------------------
//
void CAspSettingDialog::ShowPopupNoteL(const TDesC& aText) 
    {
    iPopupNote->SetTextL(aText);
    iPopupNote->SetTimePopupInView(KSettingDialogPopupDisplayTime);    
    iPopupNote->ShowInfoPopupNote();    
    }


// -----------------------------------------------------------------------------
// CAspSettingDialog::Listbox
//
// -----------------------------------------------------------------------------
//
CAknDoubleLargeStyleListBox* CAspSettingDialog::ListBox()
	{
	__ASSERT_DEBUG(iSettingListBox, TUtil::Panic(KErrGeneral));

	return iSettingListBox;
	}


// -----------------------------------------------------------------------------
// CAspSettingDialog::SyncRunning
//
// -----------------------------------------------------------------------------
//
TBool CAspSettingDialog::SyncRunning()
	{
    if (!iSyncHandler)
		{
		return EFalse;
		}

	if (iSyncHandler->SyncRunning())
		{
		return ETrue;
		}

	return EFalse;
	}


// -----------------------------------------------------------------------------
// CAspSettingDialog::UpdateCbaL
// 
// -----------------------------------------------------------------------------
//
void CAspSettingDialog::UpdateCbaL(TInt aResourceId)
    {
    CEikButtonGroupContainer& cba = ButtonGroupContainer();
    cba.SetCommandSetL(aResourceId);
    cba.DrawDeferred();
    }


// ----------------------------------------------------------------------------
// CAspSettingDialog::CancelCheckboxEvent
// 
// ----------------------------------------------------------------------------
//
void CAspSettingDialog::CancelCheckboxEventL()
	{
	CListBoxView* view = iSettingListBox->View();
    
    TInt index = view->CurrentItemIndex();
    TBool selected = view->ItemIsSelected(index);
    if (selected)
       	{
        view->DeselectItem(index);
       	}
    else
    	{
    	view->SelectItemL(index);
    	}
    	
    iSettingListBox->SetCurrentItemIndex(index);
    //view->SetCurrentItemIndex(index);
	}


// ----------------------------------------------------------------------------
// CAspSettingDialog::CancelCheckboxEvent
// 
// ----------------------------------------------------------------------------
//
void CAspSettingDialog::CancelCheckboxEvent()
	{
	TRAP_IGNORE(CancelCheckboxEventL());
	}


// ----------------------------------------------------------------------------
// CAspSettingDialog::CheckEmailSelection
// 
// ----------------------------------------------------------------------------
//
TBool CAspSettingDialog::CheckEmailSelection()
	{
	TBool ret = ETrue;
	
	TRAPD(err, ret = CheckEmailSelectionL());
	
	if (err != KErrNone)
		{
		return ETrue;
		}
	
	return ret;
	}


// ----------------------------------------------------------------------------
// CAspSettingDialog::CheckEmailSelectionL
// 
// ----------------------------------------------------------------------------
//
TBool CAspSettingDialog::CheckEmailSelectionL()
	{
	CListBoxView* view = iSettingListBox->View();
	
    TInt index = view->CurrentItemIndex();
    TAspProviderItem& item = iContentList->ProviderItem(index);
    TBool selected = view->ItemIsSelected(index);
    
    if (item.iDataProviderId != KUidNSmlAdapterEMail.iUid)
    	{
    	return ETrue;
    	}
        
    if (!selected)
    	{
    	return ETrue;
    	}
    
    iContentList->UpdateDataProviderL(item.iDataProviderId);
    if (!item.iHasDefaultDataStore)
    	{
    	CancelCheckboxEvent();
    	TDialogUtil::ShowErrorNoteL(R_ASP_NO_MAILBOXES);
    	return EFalse;
    	}
    	
    if (HasCurrentProfile())
    	{
    	TAspParam param(iApplicationId, iSyncSession);
	    CAspProfile* profile = CAspProfile::NewLC(param);
        profile->OpenL(iCurrentProfileId, CAspProfile::EOpenRead, CAspProfile::EAllProperties);
      
	    TInt num = profile->ProtocolVersion();
	    CleanupStack::PopAndDestroy(profile);

    	}
    
    	
    
    return ETrue;
	}

// ----------------------------------------------------------------------------
// CAspSettingDialog::IsProfileLockedL
// Checks if a particular profile is already opened
// ----------------------------------------------------------------------------
//
TBool CAspSettingDialog::IsProfileLockedL(TInt aProfileId)
{
	
	TAspParam param(iApplicationId, iSyncSession);
	CAspProfile* profile = CAspProfile::NewLC(param);
	
	
	TRAPD(err, profile->OpenL(aProfileId, CAspProfile::EOpenReadWrite,
                                         CAspProfile::EAllProperties));
                                         
    
	if (err == KErrLocked)
		{
		CleanupStack::PopAndDestroy(profile);
		return ETrue;
		}
	else
		{
		CleanupStack::PopAndDestroy(profile);
		return EFalse;
		}
	
}
// ----------------------------------------------------------------------------
// CAspSettingDialog::UpdateMiddleSoftKeyLabelL
// Update the MSK Label
// ----------------------------------------------------------------------------
//
void CAspSettingDialog::UpdateMiddleSoftKeyLabelL(TInt aCommandId,TInt aResourceId)
{
	ButtonGroupContainer().RemoveCommandFromStack(KMSKControlId,aCommandId );
	HBufC* middleSKText = StringLoader::LoadLC( aResourceId );

	ButtonGroupContainer().AddCommandToStackL(
	KMSKControlId,
	aCommandId,
	*middleSKText );
	CleanupStack::PopAndDestroy( middleSKText );
}

#ifdef _DEBUG


// -----------------------------------------------------------------------------
// LogDatabaseEvent
//
// -----------------------------------------------------------------------------
//
void CAspSettingDialog::LogDatabaseEvent(TAspDbEvent aEvent)
	{
	TBuf<KBufSize> buf;
	
    if (aEvent.iType == CAspDbNotifier::EClose)
    	{
    	FLOG( _L("### EClose: close setting dialog ###") );
    	}
    else if (aEvent.iType == CAspDbNotifier::EUpdate)
    	{
        TInt index = iProfileList->ListIndex(aEvent.iProfileId);
        if (index != KErrNotFound)
           	{
           	TAspProfileItem& item = iProfileList->Item(index);
            FTRACE( RDebug::Print(_L("EUpdate: %S (%d)"), &item.iProfileName, aEvent.iProfileId) );
               
            buf.Format(_L("EUpdate: %S"), &item.iProfileName);
		    TUtil::Print(buf);
           	}
        else
          	{
           	FTRACE( RDebug::Print(_L("### EUpdate: profile not found (%d) ###"), aEvent.iProfileId) );
           	TUtil::Print(_L("EUpdate: profile not found"));
           	}
    	}
    else if (aEvent.iType == CAspDbNotifier::EDelete)
    	{
        TInt index = iProfileList->ListIndex(aEvent.iProfileId);
        if (index != KErrNotFound)
           	{
           	TAspProfileItem& item = iProfileList->Item(index);
		    FTRACE( RDebug::Print(_L("EDelete: %S (%d)"), &item.iProfileName, aEvent.iProfileId) );

            buf.Format(_L("EDelete: %S"), &item.iProfileName);
		    TUtil::Print(buf);
           	}
        else
           	{
           	FTRACE( RDebug::Print(_L("EDelete: profile not found (%d)"), aEvent.iProfileId) );
           	TUtil::Print(_L("EDelete: profile not found"));
           	}
    	
    	}
    else if (aEvent.iType == CAspDbNotifier::EUpdateAll)
    	{
     	FLOG( _L("EUpdateAll: read all profiles") );
     	TUtil::Print(_L("EUpdateAll: read all profiles"));
    	}
	}


// -----------------------------------------------------------------------------
// TestL
//
// -----------------------------------------------------------------------------
//
void CAspSettingDialog::TestL()
    {
    TAspParam param(iApplicationId, iSyncSession);
	CAspProfile* profile = CAspProfile::NewLC(param);
    profile->OpenL(iCurrentProfileId, CAspProfile::EOpenRead, CAspProfile::EAllProperties);
      
	TBuf<128> buf; TBuf<128> buf2;
	profile->GetPassword(buf);
	TInt len = buf.Length();
	buf2.Format(_L("len = %d  text = %S"), len, &buf);
	TDialogUtil::ShowMessageQueryL(_L("huu"), buf2);
    CleanupStack::PopAndDestroy(profile);
    
    /*
    CListBoxView* view = iSettingListBox->View();
    const CListBoxView::CSelectionIndexArray* arr = view->SelectionIndexes();
    TBuf<128> buf; TBuf<128> buf2;
    TInt count = arr->Count();
    for (TInt i=0; i<count; i++)
     	{
       	buf2.Format(_L("%d "), (*arr)[i]);
       	buf.Append(buf2);
       	}
    TDialogUtil::ShowMessageQueryL(_L("huu"), buf);
    */
    
    /*
    HBufC* hBuf = iResHandler->ReadProgressTextLC(_L("qwerty ggggggggggggggggggggggg"), CAspState::EPhaseSending);
    TDialogUtil::ShowMessageQueryL(_L("huu"), hBuf->Des());
    CleanupStack::PopAndDestroy(hBuf);
    
    hBuf = iResHandler->ReadProgressTextLC(_L("qwerty ggggggggggggggggggggggg"), CAspState::EPhaseReceiving);
    TDialogUtil::ShowMessageQueryL(_L("huu"), hBuf->Des());
    CleanupStack::PopAndDestroy(hBuf);

    hBuf = iResHandler->ReadProgressTextLC(_L("qwerty ggggggggggggggggggggggg"), 34);
    TDialogUtil::ShowMessageQueryL(_L("huu"), hBuf->Des());
    CleanupStack::PopAndDestroy(hBuf);
    */
    }


#endif





//  End of File  
