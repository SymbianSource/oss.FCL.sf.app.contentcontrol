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

#include "AspScheduleDialog.h"
#include "AspProfileSelectionDialog.h"
#include "AspSchedule.h"
#include "AspProfileWizard.h"
#include "AspSyncUtil.rh"
#include "AspDebug.h"
#include <csxhelp/ds.hlp.hrh>

#include <aspsyncutil.mbg>  // for bitmap enumerations
#include <AknIconArray.h>   // for GulArray
#include <featmgr.h>   // FeatureManager

#include "AspProfile.h"
#include "AspContentDialog.h"
#include "AspSettingViewDialog.h"
#include <ConnectionUiUtilities.h>    
#include <cmdefconnvalues.h>
#include <cmmanager.h>
const TInt KMSKControlId( CEikButtonGroupContainer::EMiddleSoftkeyPosition );
// ============================ MEMBER FUNCTIONS ===============================



// -----------------------------------------------------------------------------
// AspScheduleDialog::ShowDialogL
// 
// -----------------------------------------------------------------------------
TBool CAspScheduleDialog::ShowDialogL(const TAspParam& aParam)
	{
	CAspScheduleDialog* dialog = CAspScheduleDialog::NewL(aParam);

	TBool ret = dialog->ExecuteLD(R_ASP_AUTO_SYNC_DIALOG);

    return ret;
	}


// -----------------------------------------------------------------------------
// CAspScheduleDialog::NewL
//
// -----------------------------------------------------------------------------
CAspScheduleDialog* CAspScheduleDialog::NewL(const TAspParam& aParam)
    {
    FLOG( _L("CAspScheduleDialog::NewL START") );

    CAspScheduleDialog* self = new (ELeave) CAspScheduleDialog(aParam);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);

    FLOG( _L("CAspScheduleDialog::NewL END") );
    return self;
    }


// -----------------------------------------------------------------------------
// CAspScheduleDialog::CAspScheduleDialog
// 
// -----------------------------------------------------------------------------
//
CAspScheduleDialog::CAspScheduleDialog(const TAspParam& aParam)
	{
    iSyncSession = aParam.iSyncSession;
    iApplicationId = aParam.iApplicationId;
    iProfileList = aParam.iProfileList;
    iEditMode = aParam.iMode;
	iProfile = aParam.iProfile;
	iContentList = aParam.iContentList;
	iSchedule = aParam.iSchedule;

	iDoCleanUp = EFalse;
	           
	__ASSERT_ALWAYS(iProfileList, TUtil::Panic(KErrGeneral));
	__ASSERT_ALWAYS(iSyncSession, TUtil::Panic(KErrGeneral));
    }


// -----------------------------------------------------------------------------
// CAspScheduleDialog::ConstructL
//
// -----------------------------------------------------------------------------
//
void CAspScheduleDialog::ConstructL()
    {
    FLOG( _L("CAspScheduleDialog::ConstructL START") );
	
	CAknDialog::ConstructL(R_ASP_AUTO_SYNC_DIALOG_MENU);

	iResHandler = CAspResHandler::NewL();

	iSettingEnforcement = TUtil::SettingEnforcementState();
	
	iSettingList = new (ELeave) CArrayPtrFlat<CAspListItemData>(KDefaultArraySize);

	if (!iSchedule)
		{
		iDoCleanUp = ETrue;
		iSchedule = CAspSchedule::NewL();
		}
    
    iWeekdayList = iResHandler->ReadDesArrayL(R_ASP_AUTO_SYNC_DAYS_LIST);
    iContentsList = new (ELeave) CDesCArrayFlat(KDefaultArraySize);
	// get previous title so it can be restored
	iStatusPaneHandler = CStatusPaneHandler::NewL(iAvkonAppUi);
	iStatusPaneHandler->StoreOriginalTitleL();
	
    iSettingChanged = EFalse;

	
	FLOG( _L("CAspScheduleDialog::ConstructL END") );
    }


// ----------------------------------------------------------------------------
// Destructor
//
// ----------------------------------------------------------------------------
//
CAspScheduleDialog::~CAspScheduleDialog()
    {
    FLOG( _L("CAspScheduleDialog::~CAspScheduleDialog START") );

	delete iResHandler;
	
	if (iSettingList)
		{
		iSettingList->ResetAndDestroy();
	    delete iSettingList;
		}
	if (iDoCleanUp)
		{
		delete iSchedule;
		}
    delete iWeekdayList;
    delete iContentsList;
	delete iStatusPaneHandler;

    if (iAvkonAppUi)
    	{
    	iAvkonAppUi->RemoveFromStack(this);
    	}

	FLOG( _L("CAspScheduleDialog::~CAspScheduleDialog END") );
    }


//------------------------------------------------------------------------------
// CAspScheduleDialog::ActivateL
//
// Called by system when dialog is activated.
//------------------------------------------------------------------------------
//
void CAspScheduleDialog::ActivateL()
	{
    CAknDialog::ActivateL();

	// this cannot be in ConstructL which is executed before dialog is launched
	iAvkonAppUi->AddToStackL(this);
    }


//------------------------------------------------------------------------------
// CAspContentListDialog::GetHelpContext
//
//------------------------------------------------------------------------------
//
void CAspScheduleDialog::GetHelpContext(TCoeHelpContext& aContext) const
	{
	aContext.iMajor = KUidSmlSyncApp;
	aContext.iContext = KDS_HLP_SETTINGS;
	}


// -----------------------------------------------------------------------------
// CAspScheduleDialog::HandleListBoxEventL
// 
// -----------------------------------------------------------------------------

void CAspScheduleDialog::HandleListBoxEventL(CEikListBox* /*aListBox*/,
                                            TListBoxEvent aEventType)
	{
	if( AknLayoutUtils::PenEnabled() )  
	  {
	   switch ( aEventType )
        {
         case EEventItemSingleClicked:
              HandleOKL();
              break;
         case EEventEnterKeyPressed :
         case EEventItemDraggingActioned :
         case EEventPenDownOnItem :
              break;
         default:
              break;
        }
		
	  }
    
	}


// -----------------------------------------------------------------------------
// CAspScheduleDialog::PreLayoutDynInitL
// 
// -----------------------------------------------------------------------------
//
void CAspScheduleDialog::PreLayoutDynInitL()
    {
    iSettingListBox = (CAknSettingStyleListBox*) ControlOrNull (EAspAutoSyncDialogList);
    
   	__ASSERT_ALWAYS(iSettingListBox, TUtil::Panic(KErrGeneral));
    
	if(iSettingEnforcement)
	{
	ButtonGroupContainer().RemoveCommandFromStack(KMSKControlId,EAknSoftkeyOpen );	
	HBufC* middleSKText = StringLoader::LoadLC( R_TEXT_SOFTKEY_EMPTY );

    ButtonGroupContainer().AddCommandToStackL(
        KMSKControlId,
        EAknSoftkeyEmpty,
        *middleSKText );
    CleanupStack::PopAndDestroy( middleSKText );
	}
	iSettingListBox->SetListBoxObserver(this);
	iSettingListBox->CreateScrollBarFrameL(ETrue);
	iSettingListBox->ScrollBarFrame()->SetScrollBarVisibilityL(
	                 CEikScrollBarFrame::EOn, CEikScrollBarFrame::EAuto);
	
	SetIconsL();
	
	// create array of setting items (iSettingList)
	CreateSettingsListL();

	// add setting headers into listbox
	UpdateListBoxL(iSettingListBox, iSettingList);
	
	if (iEditMode == EDialogModeEditMandatory)
		{
		TInt index = CheckMandatoryFields();
		if (index != KErrNotFound)
			{
			iSettingListBox->SetCurrentItemIndexAndDraw(index);
			}
		}
	

	iStatusPaneHandler->SetTitleL(R_ASP_TITLE_AUTOMATIC_SYNC_SETTINGS);
	iStatusPaneHandler->SetNaviPaneTitleL(KNullDesC);
    }


// ----------------------------------------------------------------------------
// CAspScheduleDialog::SetIconsL
//
// ----------------------------------------------------------------------------
//
void CAspScheduleDialog::SetIconsL()
    {
    if (!iSettingListBox)
    	{
    	return;
    	}
 
 	TFileName bitmapName;
	CAspResHandler::GetBitmapFileName(bitmapName);
	CArrayPtr<CGulIcon>* icons = new (ELeave) CAknIconArray(KDefaultArraySize);
	CleanupStack::PushL(icons);
	
	icons->AppendL(IconL(KAknsIIDQgnIndiSettProtectedAdd, bitmapName, 
	                     EMbmAspsyncutilQgn_indi_sett_protected_add,
	                     EMbmAspsyncutilQgn_indi_sett_protected_add_mask));
	
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


// -----------------------------------------------------------------------------
// CAspScheduleDialog::IconL
// 
// -----------------------------------------------------------------------------
//
CGulIcon* CAspScheduleDialog::IconL(TAknsItemID aId, const TDesC& aFileName, TInt aFileIndex, TInt aFileMaskIndex)
	{
    return TDialogUtil::CreateIconL(aId, aFileName, aFileIndex, aFileMaskIndex);
	}


//------------------------------------------------------------------------------
// CAspScheduleDialog::DynInitMenuPaneL
//
// Called by system before menu is shown.
//------------------------------------------------------------------------------
//
void CAspScheduleDialog::DynInitMenuPaneL(TInt aResourceID, CEikMenuPane* aMenuPane)
	{
    if (aResourceID != R_ASP_AUTO_SYNC_DIALOG_MENU_PANE)
		{
		return;
		}

	if (iSettingListBox->Model()->NumberOfItems() == 0) 
		{
		TDialogUtil::DeleteMenuItem(aMenuPane, EAspMenuCmdChange);
		return;
		}

	CAspListItemData* item = GetItemForIndex(iSettingListBox->CurrentItemIndex());
	if (item->iHidden == EVisibilityReadOnly)
		{
		TDialogUtil::DeleteMenuItem(aMenuPane, EAspMenuCmdChange);
		}
	
	if (!FeatureManager::FeatureSupported(KFeatureIdHelp))
		{
		TDialogUtil::DeleteMenuItem(aMenuPane, EAknCmdHelp);
		}
    }


//------------------------------------------------------------------------------
// CAspScheduleDialog::ProcessCommandL
//
// Handle commands from menu.
//------------------------------------------------------------------------------
//
void CAspScheduleDialog::ProcessCommandL(TInt aCommandId)
	{
	HideMenu();

	switch (aCommandId)
		{
		case EAknCmdHelp:
			{
			
			TUtil::LaunchHelpAppL(iEikonEnv);
            break;

			}

		case EAspMenuCmdChange:
			{				
    		CAspListItemData* item = GetItemForIndex(iSettingListBox->CurrentItemIndex());

			if (item->iItemType == CAspListItemData::ETypeListYesNo)
				{
				// open editor for Yes/No setting
			/*	if (EditSettingItemListL(*item))
					{
					SetVisibility();
					UpdateListBoxL(iSettingListBox, iSettingList);
					
					iSettingChanged = ETrue;
					}*/
				}				
			else
				{
				HandleOKL();
				}

			break;
			}
		    
        case EAspMenuCmdExit:
        case EAknCmdExit:
        case EEikCmdExit:
			{
			// close dialog and exit calling application
			iAvkonAppUi->ProcessCommandL(EAknCmdExit);
			break;
			}

		default:			
		    break;
		}
	}


//------------------------------------------------------------------------------
// CAspScheduleDialog::OkToExitL
//
//------------------------------------------------------------------------------
//
TBool CAspScheduleDialog::OkToExitL(TInt aButtonId)
	{
	if (aButtonId == EEikBidCancel)
		{
    //	TRAP_IGNORE(SaveSettingsL());
		return ETrue;
		}

	if (aButtonId == EAknSoftkeyBack)
        {
   #if 0
		TInt mandatoryIndex = CheckMandatoryFields();
		if (mandatoryIndex != KErrNotFound)
			{
			if (!TDialogUtil::ShowConfirmationQueryL(R_ASP_EXIT_ANYWAY))
				{
				iSettingListBox->SetCurrentItemIndexAndDraw(mandatoryIndex);
				return EFalse; // leave dialog open
				}
			}
   #endif
		if (iSettingChanged)
			{
			if(!CheckPeakTime())
				{
				HBufC* hBuf = CAspResHandler::ReadLC(R_ASP_INCORRECT_PEAK_TIME);
				TDialogUtil::ShowInformationNoteL(hBuf->Des());
				CleanupStack::PopAndDestroy(hBuf);
				return EFalse;
				}
		    TRAPD(err, SaveSettingsL());
		    if (err == KErrNone)
			    {
			    CheckMandatoryDataL();
		        iSchedule->CopyAutoSyncContentsL();
				iSchedule->UpdateProfileSettingsL();
				iSchedule->UpdateSyncSchedule();
			    iSchedule->SaveL(); // for saving sync schedule id
			    iSchedule->EnableScheduleL(); 
			    }
			}
		ShowAutoSyncInfoL();
		return ETrue;
		}
		
	if (aButtonId == EAknSoftkeyOpen || aButtonId == EAknSoftkeyEmpty)  // MSK
        {
        HandleOKL();
		return EFalse;  // leave dialog open
		}
	
	return CAknDialog::OkToExitL(aButtonId);
	}


// ----------------------------------------------------------------------------
// CAspScheduleDialog::OfferKeyEventL
// 
// ----------------------------------------------------------------------------
//
TKeyResponse CAspScheduleDialog::OfferKeyEventL(const TKeyEvent& aKeyEvent,
                                                      TEventCode aType)
	{
	if (aType == EEventKey)
		{
		switch (aKeyEvent.iCode)
			{
			case EKeyEnter:
			case EKeyOK:
				{
				CAspListItemData* item = 
				GetItemForIndex(iSettingListBox->CurrentItemIndex());
     			if (item->iHidden == EVisibilityReadOnly)
     				{
     				TDialogUtil::ShowInformationNoteL(R_ASP_NOTE_READ_ONLY);
  					return EKeyWasConsumed;
     				}
		
				
				HandleOKL();
				return EKeyWasConsumed;
				}

			case EKeyEscape:  // framework calls this when dialog must shut down
				{
				return CAknDialog::OfferKeyEventL(aKeyEvent, aType);
        		}

            default:
				{
			    break;
				}
			}
		}

	return CAknDialog::OfferKeyEventL( aKeyEvent, aType);
	}


// ----------------------------------------------------------------------------
// CAspScheduleDialog::HandleResourceChange
// 
// ----------------------------------------------------------------------------
//
void CAspScheduleDialog::HandleResourceChange(TInt aType)
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


//------------------------------------------------------------------------------
// CAspScheduleDialog::CheckMandatoryFields
//
// Check that all mandatory fields are filled.
//------------------------------------------------------------------------------
//
TInt CAspScheduleDialog::CheckMandatoryFields()
	{ 
	TInt count = iSettingList->Count();
	for (TInt i=0; i<count; i++)
		{
		CAspListItemData* item = (*iSettingList)[i];
			
		if ( item->iMandatory && item->IsEmpty() && (item->iHidden == EVisibilityNormal) )
			{
			return item->iIndex;
			}
		}

    return KErrNotFound;
	}


//------------------------------------------------------------------------------
// CAspScheduleDialog::HandleOKL
//
//------------------------------------------------------------------------------
//
void CAspScheduleDialog::HandleOKL()
	{
	CAspListItemData* item = GetItemForIndex(iSettingListBox->CurrentItemIndex());

	if (item->iHidden == EVisibilityReadOnly)
		{
		if(!iSettingEnforcement)
    	{
     	TDialogUtil::ShowInformationNoteL(R_ASP_NOTE_READ_ONLY);
    	}
     	else
     	{
     		TDialogUtil::ShowInformationNoteL(R_ASP_PROTECTED_SETTING);	
     	}
		return;
		}

	if (EditSettingItemL(*item))
		{
		SetVisibility();
		UpdateListBoxL(iSettingListBox, iSettingList);
		
		iSettingChanged = ETrue;
		}
	}


// ----------------------------------------------------------------------------
// CAspScheduleDialog::CreateSettingsListL
// 
// ----------------------------------------------------------------------------
//
void CAspScheduleDialog::CreateSettingsListL()
	{

    AddItemL(EAspAutoSyncProfile,R_ASP_SETT_AUTO_SYNC_PROFILE);
    AddItemL(EAspAutoSyncContents, R_ASP_SETT_AUTO_SYNC_CONTENTS);
	AddItemL(EAspAutoSyncFrequency, R_ASP_SETT_AUTO_SYNC_FREQ);	
    AddItemL(EAspAutoSyncPeakSchedule, R_ASP_SETT_AUTO_PEAK_SCHEDULE);
    AddItemL(EAspAutoSyncOffPeakSchedule, R_ASP_SETT_AUTO_OFF_PEAK_SCHEDULE);
    AddItemL(EAspAutoSyncScheduleTime, R_ASP_SETT_AUTO_SYNC_TIME);
	AddItemL(EAspAutoSyncPeakStartTime, R_ASP_SETT_AUTO_PEAK_START);
    AddItemL(EAspAutoSyncPeakEndTime, R_ASP_SETT_AUTO_PEAK_END);
	AddItemL(EAspAutoSyncPeakDays, R_ASP_SETT_AUTO_SYNC_DAYS);

//roaming to be implemented later
#if 0 
	AddItemL(EAspAutoSyncRoamingStatus, R_ASP_SETTING_AUTO_SYNC_ROAMING_STATUS);
#endif

	// write setting data into each CAspListItemData
	TInt count=iSettingList->Count();
	for (TInt i=0; i<count; i++)
		{
		InitSettingItemL((*iSettingList)[i]);
		}

	SetVisibility();  // find out what setting appear on UI
    }


// ----------------------------------------------------------------------------
// CAspScheduleDialog::AddItemL
// 
// ----------------------------------------------------------------------------
//
void CAspScheduleDialog::AddItemL(TInt aItemId, TInt aResourceId)
	{
    CAspListItemData* item = CAspListItemData::NewLC();
	item->SetHeaderL(aResourceId);
	item->iItemId = aItemId;
	iSettingList->AppendL(item);
	CleanupStack::Pop(item);
	}


// ----------------------------------------------------------------------------
// CAspScheduleDialog::UpdateListBoxL
// 
// Add settings headers into listbox.
// ----------------------------------------------------------------------------
//
void CAspScheduleDialog::UpdateListBoxL(CEikTextListBox* aListBox,
                                        CAspSettingList* aItemList)
	{
	CDesCArray* arr = (CDesCArray*)aListBox->Model()->ItemTextArray();
	arr->Reset();

	TInt count = aItemList->Count();
	for (TInt i=0; i<count; i++ )
		{	
		CAspListItemData* item = (*aItemList)[i];
		
		TBool convert = ETrue;

		if (item->iHidden != EVisibilityHidden)
			{
			HBufC* hBuf = item->ListItemTextL(convert);
			CleanupStack::PushL(hBuf);
			
			arr->AppendL(hBuf->Des());
			
			// store listbox index (from zero up) into setting item 
			item->iIndex = arr->Count() - 1;

			CleanupStack::PopAndDestroy(hBuf);
			}
		else
			{
			item->iIndex = KErrNotFound;
			}
		}

	iSettingListBox->HandleItemAdditionL();
	}


// -----------------------------------------------------------------------------
// CAspScheduleDialog::SetVisibility
// 
// -----------------------------------------------------------------------------
//
void CAspScheduleDialog::SetVisibility()
	{

	Item(EAspAutoSyncScheduleTime)->iHidden = EVisibilityHidden;
	Item(EAspAutoSyncPeakStartTime)->iHidden = EVisibilityHidden;
	Item(EAspAutoSyncPeakEndTime)->iHidden = EVisibilityHidden;
	Item(EAspAutoSyncPeakSchedule)->iHidden = EVisibilityHidden;
	Item(EAspAutoSyncOffPeakSchedule)->iHidden = EVisibilityHidden;
	Item(EAspAutoSyncPeakDays)->iHidden = EVisibilityHidden;

	if (Item(EAspAutoSyncFrequency)->iNumberData == CAspSchedule::EManyTimesPerDay)
		{
		Item(EAspAutoSyncScheduleTime)->iHidden = EVisibilityHidden;
		Item(EAspAutoSyncPeakSchedule)->iHidden = EVisibilityNormal;
		Item(EAspAutoSyncOffPeakSchedule)->iHidden = EVisibilityNormal;
		if (Item(EAspAutoSyncPeakSchedule)->iNumberData || 
								Item(EAspAutoSyncOffPeakSchedule)->iNumberData)
			{
			Item(EAspAutoSyncPeakStartTime)->iHidden = EVisibilityNormal;
     		Item(EAspAutoSyncPeakEndTime)->iHidden = EVisibilityNormal;
			Item(EAspAutoSyncPeakDays)->iHidden = EVisibilityNormal;
			}
		}
	if (iSchedule->IntervalType(Item(EAspAutoSyncFrequency)->iNumberData) == EDaily  )
		{
		Item(EAspAutoSyncScheduleTime)->iHidden = EVisibilityNormal;
		}
	
	if (iEditMode == EDialogModeReadOnly || iSettingEnforcement)
    	{
    	SetAllReadOnly();
    	}
 
  	}


// -----------------------------------------------------------------------------
// CAspContentDialog::SetAllReadOnly
// 
// -----------------------------------------------------------------------------
//
void CAspScheduleDialog::SetAllReadOnly()
	{
	TInt count = iSettingList->Count();

	for (TInt i=0; i<count; i++ )
		{
		CAspListItemData* item = (*iSettingList)[i];
		if (item->iHidden != EVisibilityHidden)
			{
			item->iHidden = EVisibilityReadOnly;
			}
		}
	}


// -----------------------------------------------------------------------------
// CAspScheduleDialog::InitSettingItemL
// 
// Constructs CAspListItemData for one connection setting.
// -----------------------------------------------------------------------------
//
void CAspScheduleDialog::InitSettingItemL(CAspListItemData* aItem)
	{
	__ASSERT_ALWAYS(aItem, TUtil::Panic(KErrGeneral));	
	
	iBuf = KNullDesC;  // reset common buffer
	
	switch (aItem->iItemId)
		{

		case EAspAutoSyncProfile:
			{
			if (!iSchedule->IsAutoSyncEnabled())
				{
				aItem->iNumberData = KErrNotFound;
				}
			else
				{
				aItem->iNumberData = iSchedule->ProfileId();
				}
			
			GetProfileName(aItem->iNumberData, iBuf);
			aItem->SetValueL(iBuf);
			aItem->SetDisplayValueL(R_ASP_SETT_VALUE_NOT_DEFINED);
			aItem->iMandatory = ETrue;
			aItem->iItemType = CAspListItemData::ETypeAutoSyncProfile;
			break;
			}
		case EAspAutoSyncContents:
		    {
			if (!iSchedule->IsAutoSyncEnabled())
				{
				SetDefaultContentL();
				}
			GetContentsNameL(iBuf);
			aItem->SetValueL(iBuf);
			aItem->SetDisplayValueL(R_ASP_SETTING_VALUE_NONE);
			aItem->iItemType = CAspListItemData::ETypeAutoSyncContent;
			break;	
		    }
		case EAspAutoSyncFrequency:
			{
			CDesCArray* arr = GetSyncFrequencyListLC();
			aItem->iNumberData = iSchedule->SyncFrequency();
			if (iSchedule->ProfileId() == KErrNotFound || !iSchedule->IsContentSelected()
															|| !iSchedule->IsAutoSyncEnabled())
				{
				aItem->iNumberData = CAspSchedule::EIntervalManual;
				}
			TInt currentSelection = GetSyncFrequencyPositionL(arr, aItem->iNumberData); 
			aItem->SetValueL((*arr)[currentSelection]);
			aItem->iItemType = CAspListItemData::ETypeAutoSyncFrequency;
			CleanupStack::PopAndDestroy(arr);
			break;

			}
		case EAspAutoSyncPeakSchedule:
			{
			CDesCArray* arr = GetPeakScheduleListLC();
			aItem->iNumberData = iSchedule->SyncPeakSchedule();
			if (iSchedule->ProfileId() == KErrNotFound || !iSchedule->IsContentSelected()
														|| !iSchedule->IsAutoSyncEnabled())
				{
				aItem->iNumberData = CAspSchedule::EIntervalManual;
				}
			TInt currentSelection = GetPeakPositionL(arr, aItem->iNumberData); 
			aItem->SetValueL((*arr)[currentSelection]);
			aItem->iItemType = CAspListItemData::ETypeAutoSyncPeakSchedule;
			CleanupStack::PopAndDestroy(arr);
			break;
			}
		
		case EAspAutoSyncOffPeakSchedule:
			{
			CDesCArray* arr = GetOffPeakScheduleListLC();
			aItem->iNumberData = iSchedule->SyncOffPeakSchedule();
			if (iSchedule->ProfileId() == KErrNotFound || !iSchedule->IsContentSelected()
															|| !iSchedule->IsAutoSyncEnabled())
				{
				aItem->iNumberData = CAspSchedule::EIntervalManual;
				}
			TInt currentSelection = GetOffPeakPositionL(arr, aItem->iNumberData); 
			aItem->SetValueL((*arr)[currentSelection]);
			aItem->iItemType = CAspListItemData::ETypeAutoSyncOffPeakSchedule;
			CleanupStack::PopAndDestroy(arr);
			break;
			}
		case EAspAutoSyncScheduleTime:
			{
			if (iSchedule->DailySyncEnabled())
				{
				aItem->iTime = iSchedule->AutoSyncScheduleTime();
				}
			else
				{
				TTime currentTime;
				currentTime.HomeTime();
				aItem->iTime = currentTime;
				}
			
			GetTimeTextL(iBuf, aItem->iTime);  
			aItem->SetValueL(iBuf);
			aItem->SetDisplayValueL(R_ASP_SETTING_VALUE_NONE);
			aItem->iItemType = CAspListItemData::ETypeAutoSyncTime;
			Item(EAspAutoSyncScheduleTime)->iHidden = EVisibilityHidden;
			break;
			}
		case EAspAutoSyncPeakStartTime:
			{
			if (iSchedule->ProfileId() == KErrNotFound || !iSchedule->IsContentSelected()
													    || !iSchedule->IsAutoSyncEnabled())
				{
				TDateTime time(0, EJanuary, 0, KStartPeakHour, KStartPeakMin, 0, 0);
				TTime startTime(time);
				aItem->iTime = time;
				}
			else
				{
				aItem->iTime = iSchedule->StartPeakTime();	
				}

			GetTimeTextL(iBuf, aItem->iTime);  
			aItem->SetValueL(iBuf);
			aItem->SetDisplayValueL(R_ASP_SETTING_VALUE_NONE);
			aItem->iItemType = CAspListItemData::ETypeAutoSyncTime;
			break;
			}
		case EAspAutoSyncPeakEndTime:
			{
			if (iSchedule->ProfileId() == KErrNotFound || !iSchedule->IsContentSelected()
													    || !iSchedule->IsAutoSyncEnabled())
				{
				TDateTime time(0, EJanuary, 0, KEndPeakHour, KEndPeakMin, 0, 0);
				TTime startTime(time);
				aItem->iTime = time;
				}
			else
				{
				aItem->iTime = iSchedule->EndPeakTime();	
				}
			GetTimeTextL(iBuf, aItem->iTime);			
			aItem->SetValueL(iBuf);
			aItem->SetDisplayValueL(R_ASP_SETTING_VALUE_NONE);
			aItem->iItemType = CAspListItemData::ETypeAutoSyncTime;
			break;
			}
		case EAspAutoSyncPeakDays:
			{
			if (iSchedule->ProfileId() == KErrNotFound || !iSchedule->IsContentSelected()
													    || !iSchedule->IsAutoSyncEnabled())
				{
				SetDefaultdaySelectionL();
				}
			GetWeekdayNameL(iBuf);
			aItem->SetValueL(iBuf);
			aItem->SetDisplayValueL(R_ASP_SETTING_VALUE_NONE);				
			aItem->iResource = R_ASP_AUTO_SYNC_DAYS_LIST;
			aItem->iItemType = CAspListItemData::ETypeAutoSyncDays;
	   		break;
			}		 
		case EAspAutoSyncRoamingStatus:
			{
			CDesCArray* arr = iResHandler->ReadDesArrayLC(R_ASP_AUTO_SYNC_ROAMING_YESNO);
    		
    		if (iSchedule->RoamingAllowed())
				{
				aItem->iNumberData = EAspSettingEnabled;  // 1
				}
			else
				{
				aItem->iNumberData = EAspSettingDisabled; // 0
				}

			aItem->iResource = R_ASP_AUTO_SYNC_ROAMING_YESNO;
			aItem->SetValueL((*arr)[aItem->iNumberData]);
			aItem->iItemType = CAspListItemData::ETypeListYesNo;
			
			CleanupStack::PopAndDestroy(arr);
            break;
			}
        	
        default:
           	TUtil::Panic(KErrGeneral);
			break;
	
	  	
		}
	}


// ----------------------------------------------------------------------------
// CAspScheduleDialog::SaveSettingsL
// 
// ----------------------------------------------------------------------------
//
void CAspScheduleDialog::SaveSettingsL()
	{
   	iBuf = KNullDesC;  // reset common buffer
   	
	TInt count = iSettingList->Count();

	if (iSettingEnforcement)
		{
		return;
		}
	
	for (TInt i=0; i<count; i++)
		{
		CAspListItemData* item = (*iSettingList)[i];
		
		switch (item->iItemId)
			{
			case EAspAutoSyncProfile:
				{
				TInt oldProfile = iSchedule->ProfileId();
				iSchedule->SetProfileId(item->iNumberData);
				if(oldProfile != iSchedule->ProfileId())
					{
                    iSchedule->SaveL(); //Saved values to be used in Update
					iSchedule->UpdateProfileSettingsL();
					}
				break;
				}
				
			case EAspAutoSyncContents:
			    break;//handled in EditSettingItemContentL  
			case EAspAutoSyncFrequency:
				{
				iSchedule->SetSyncFrequency(item->iNumberData);
				if (iSchedule->IntervalType(item->iNumberData) == EDaily)
					{
					iSchedule->SetDailySyncEnabled(ETrue);
					}
				else
					{
					iSchedule->SetDailySyncEnabled(EFalse);	
					}

				}
			case EAspAutoSyncPeakSchedule:
				{
				iSchedule->SetSyncPeakSchedule(item->iNumberData);
				if(item->iNumberData)
					{
					iSchedule->SetPeakSyncEnabled(ETrue);
					}
				else
					{
					iSchedule->SetPeakSyncEnabled(EFalse);	
					}
				break;
				}
			case EAspAutoSyncOffPeakSchedule:
				{
				iSchedule->SetSyncOffPeakSchedule(item->iNumberData);
			    if(item->iNumberData)
					{
					iSchedule->SetOffPeakSyncEnabled(ETrue);
					}
				else
					{
					iSchedule->SetOffPeakSyncEnabled(EFalse);	
					}
				break;
				}
			case EAspAutoSyncScheduleTime:
			    iSchedule->SetAutoSyncScheduleTime(item->iTime);
			case EAspAutoSyncPeakStartTime:
				iSchedule->SetStartPeakTime(item->iTime);
				break;
			case EAspAutoSyncPeakEndTime:
				iSchedule->SetEndPeakTime(item->iTime);
				break;
			case EAspAutoSyncPeakDays:
				 break;//handled in EditSettingItemWeekdaysL
            case EAspAutoSyncRoamingStatus:
			    iSchedule->SetRoamingAllowed(item->iNumberData);
			    break;
			    
            default:
                break;
			}
		}

	iSchedule->SaveL();
	}
	
	
//------------------------------------------------------------------------------
// CAspScheduleDialog::EditSettingItemL
//
// Calls setting editing functions. 
//------------------------------------------------------------------------------
//
TBool CAspScheduleDialog::EditSettingItemL(CAspListItemData& aItem)
	{
	TInt ret = EFalse;
	
	switch (aItem.iItemType)
		{
		case CAspListItemData::ETypeText:
			ret = EditSettingItemTextL(aItem);
		    break;

		case CAspListItemData::ETypeAutoSyncTime:
			ret = EditSettingItemTimeL(aItem);
		    break;

		case CAspListItemData::ETypeAutoSyncFrequency:
			ret = EditSettingItemSyncFrequencyL(aItem);
		    break;

		case CAspListItemData::ETypeAutoSyncPeakSchedule:
			ret = EditSettingItemPeakScheduleL(aItem);
		    break;

		case CAspListItemData::ETypeAutoSyncOffPeakSchedule:
			ret = EditSettingItemOffPeakScheduleL(aItem);
		    break;
		
		case CAspListItemData::ETypeAutoSyncProfile:
			ret = EditSettingItemProfileL(aItem);
		    break;

		case CAspListItemData::ETypeAutoSyncContent:
		    ret = EditSettingItemContentL(aItem);
			break;
		    
		case CAspListItemData::ETypeAutoSyncDays:
			ret = EditSettingItemWeekdaysL(aItem);
		    break;

		case CAspListItemData::ETypeListYesNo:
			ret = EditSettingItemYesNoL(aItem);
		    break;

		default:
		    break;
		}
	
	return ret;
	}


//------------------------------------------------------------------------------
// CAspScheduleDialog::EditSettingItemYesNoL
//
// Change Yes/No value without showing radio button editor.
//------------------------------------------------------------------------------
//
TBool CAspScheduleDialog::EditSettingItemYesNoL(CAspListItemData& aItem)
	{
	CDesCArray* arr = iResHandler->ReadDesArrayLC(aItem.iResource);
	
	if (aItem.iNumberData == EAspSettingDisabled)
		{
		aItem.iNumberData = EAspSettingEnabled;
		}
	else
		{
		aItem.iNumberData = EAspSettingDisabled;
		}
	
	
	// store localized setting text (Yes/No)
	aItem.SetValueL( (*arr)[aItem.iNumberData] );

	CleanupStack::PopAndDestroy(arr);
	return ETrue;
	}


//------------------------------------------------------------------------------
// CAspScheduleDialog::EditSettingItemNumberL
//
//------------------------------------------------------------------------------
//
TBool CAspScheduleDialog::EditSettingItemNumberL(CAspListItemData& aItem)
	{
	TBool ret = TDialogUtil::ShowIntegerEditorL(aItem.iNumberData, aItem.Header(),
	                         aItem.iMinValue, aItem.iMaxValue, aItem.iLatinInput);

	if (ret)
		{
		aItem.SetValueL(aItem.iNumberData);
		}

	return ret;
	}


//------------------------------------------------------------------------------
// CAspScheduleDialog::EditSettingItemTextL
//
// Edit text setting item.
//------------------------------------------------------------------------------
//
TBool CAspScheduleDialog::EditSettingItemTextL(CAspListItemData& aItem)
	{
	TUtil::StrCopy(iBuf, aItem.Value());
	
    TBool ret = TDialogUtil::ShowTextEditorL(iBuf, aItem.Header(),
                             aItem.iMandatory, aItem.iLatinInput, aItem.iMaxLength);
   	if (ret)
		{
		aItem.SetValueL(iBuf);
		}
		
	return ret;
	}

//------------------------------------------------------------------------------
// CAspScheduleDialog::EditSettingItemSyncFrequencyL
//
//------------------------------------------------------------------------------
//
TBool CAspScheduleDialog::EditSettingItemSyncFrequencyL(CAspListItemData& aItem)
	{
	CDesCArray* arr = GetSyncFrequencyListLC();
	TInt curSelection = GetSyncFrequencyPositionL(arr, aItem.iNumberData); 
	TBool ret = TDialogUtil::ShowListEditorL(arr, aItem.Header(), curSelection);
	if (ret)
		{
		aItem.iNumberData = SyncFrequencyL((*arr)[curSelection]); 
		aItem.SetValueL((*arr)[curSelection]);
		}

	if (iSchedule->IntervalType(aItem.iNumberData) == EDaily
					|| aItem.iNumberData == CAspSchedule::EIntervalManual)
		{
		Item(EAspAutoSyncPeakSchedule)->iNumberData = CAspSchedule::EIntervalManual;
		GetPeakScheduleValueL(CAspSchedule::EIntervalManual, iBuf);
		Item(EAspAutoSyncPeakSchedule)->SetValueL(iBuf);
		Item(EAspAutoSyncOffPeakSchedule)->iNumberData = CAspSchedule::EIntervalManual;
		GetPeakScheduleValueL(CAspSchedule::EIntervalManual, iBuf);
		Item(EAspAutoSyncOffPeakSchedule)->SetValueL(iBuf);
		}
		
	CleanupStack::PopAndDestroy(arr);
	return ret;
	}



//------------------------------------------------------------------------------
// CAspScheduleDialog::EditSettingItemPeakScheduleL
//
//------------------------------------------------------------------------------
//
TBool CAspScheduleDialog::EditSettingItemPeakScheduleL(CAspListItemData& aItem)
	{

	CDesCArray* arr = GetPeakScheduleListLC();

	TInt curSelection = GetPeakPositionL(arr, aItem.iNumberData); 
	TBool ret = TDialogUtil::ShowListEditorL(arr, aItem.Header(), curSelection);
	if (ret)
		{
		aItem.iNumberData = PeakScheduleL((*arr)[curSelection]); 
		aItem.SetValueL((*arr)[curSelection]);
		}

	CleanupStack::PopAndDestroy(arr);
	return ret;
	}

//------------------------------------------------------------------------------
// CAspScheduleDialog::EditSettingItemOffPeakScheduleL
//
//------------------------------------------------------------------------------
//
TBool CAspScheduleDialog::EditSettingItemOffPeakScheduleL(CAspListItemData& aItem)
	{
	
	CDesCArray* arr = GetOffPeakScheduleListLC();

	TInt curSelection = GetOffPeakPositionL(arr, aItem.iNumberData); 
	TBool ret = TDialogUtil::ShowListEditorL(arr, aItem.Header(), curSelection);
	if (ret)
		{
		aItem.iNumberData = OffPeakScheduleL((*arr)[curSelection]); ; 
		aItem.SetValueL((*arr)[curSelection]);
		}
	
	CleanupStack::PopAndDestroy(arr);
	return ret;
	}


//------------------------------------------------------------------------------
// CAspScheduleDialog::EditSettingItemProfileL
//
//------------------------------------------------------------------------------
//
TBool CAspScheduleDialog::EditSettingItemProfileL(CAspListItemData& aItem)
	{

	TAspFilterInfo info;
    info.iFilterType = TAspFilterInfo::EIncludeRemoteProfile;
    CAspProfileList* remoteProfList = iProfileList->FilteredListL(info);
    CleanupStack::PushL(remoteProfList);
	
	TInt asProfileId = iSchedule->AutoSyncProfileId();
	if(asProfileId != KErrNotFound)
		{
		remoteProfList->Remove(asProfileId);
		}
    TInt curSelection = remoteProfList->ListIndex(aItem.iNumberData);
    TInt count = remoteProfList->Count();
   
    CDesCArray* profileNames = new (ELeave) CDesCArrayFlat(KMaxProfileCount);
	CleanupStack::PushL(profileNames);
    for(TInt index = 0; index < count; ++index)
		{
		TAspProfileItem& item = remoteProfList->Item(index);
		GetProfileName(item.iProfileId ,iBuf);
		profileNames->AppendL(iBuf);
		}
	
	TBool ret = TDialogUtil::ShowListEditorL(profileNames, aItem.Header(), curSelection);

	if (ret)
		{
		TAspProfileItem& item = remoteProfList->Item(curSelection);
		aItem.iNumberData = item.iProfileId;
		GetProfileName(item.iProfileId, iBuf);
		aItem.SetValueL(iBuf);

		if (aItem.iNumberData != KErrNotFound)
		{
	    CheckAccessPointSelectionL(aItem.iNumberData);
		}
		}
	CleanupStack::PopAndDestroy(profileNames);
	CleanupStack::PopAndDestroy(remoteProfList);
	return ret;  // ETrue or EFalse
	}


//------------------------------------------------------------------------------
// CAspScheduleDialog::EditSettingItemWeekdaysL
//
//------------------------------------------------------------------------------
//
TBool CAspScheduleDialog::EditSettingItemWeekdaysL(CAspListItemData& aItem)
	{
	CAspSelectionItemList* list = new (ELeave) CAspSelectionItemList(1);
	CleanupStack::PushL(TCleanupItem(CAspSelectionItemList::Cleanup, list));
	
	GetWeekdaySelectionL(list);

	CAknCheckBoxSettingPage* dlg = new (ELeave) CAknCheckBoxSettingPage(
	                                   R_ASP_MULTI_SELECTION_LIST, list);
	
   	CleanupStack::PushL(dlg);
    dlg->SetSettingTextL(aItem.Header());
	CleanupStack::Pop(dlg);

    TBool ret = dlg->ExecuteLD(CAknSettingPage::EUpdateWhenChanged);
    
    if (ret)
    	{
        SetWeekdaySelectionL(list);
        GetWeekdayNameL(iBuf);
        aItem.SetValueL(iBuf);
    	}
    
	CleanupStack::PopAndDestroy(list);

   	return ret;
	}


//------------------------------------------------------------------------------
// CAspScheduleDialog::EditSettingItemTimeL
//
//------------------------------------------------------------------------------
//
//
TBool CAspScheduleDialog::EditSettingItemTimeL(CAspListItemData& aItem)
	{

	TBool ret = TDialogUtil::ShowTimeEditorL(aItem.iTime, aItem.Header());
	
	if (ret)
		{
 		GetTimeTextL(iBuf, aItem.iTime);
		aItem.SetValueL(iBuf);
		}

   	return ret;
	}

//------------------------------------------------------------------------------
// CAspScheduleDialog::EditSettingItemContentL
//
//------------------------------------------------------------------------------
//
//
TBool CAspScheduleDialog::EditSettingItemContentL(CAspListItemData& aItem)
	{
	
	CAspSelectionItemList* list = new (ELeave) CAspSelectionItemList(1);
	CleanupStack::PushL(TCleanupItem(CAspSelectionItemList::Cleanup, list));

	TInt emailIndex = iContentList->FindProviderIndex(KUidNSmlAdapterEMail.iUid);
	if (!iSchedule->MailboxExistL())
		{
		iSchedule->SetContentEnabled(emailIndex, EFalse);
		}
	
	TInt profileId = Item(EAspAutoSyncProfile)->iNumberData;
	if (profileId != KErrNotFound)
		{
		TAspParam param(iApplicationId, iSyncSession);

		CAspProfile* profile = CAspProfile::NewLC(param);
		profile->OpenL(profileId, CAspProfile::EOpenRead,
                                         CAspProfile::EBaseProperties);
		TInt protocol = profile->ProtocolVersion();
		CleanupStack::PopAndDestroy(profile);
		if (protocol == EAspProtocol_1_1)
			{
	  		iSchedule->SetContentEnabled(emailIndex, EFalse);
			}
		}
	GetContentSelectionL(list);
	
	//CAknCheckBoxSettingPage* dlg = new (ELeave) CAknCheckBoxSettingPage(
	//                                   R_ASP_MULTI_SELECTION_LIST, list);

	TAutoSyncSettingPageParam param;
    param.iCommandId = EAknSoftkeyUnmark;
   
    param.iObserver = this;
    

	CAutoSyncCheckBoxSettingPage* dlg = new (ELeave) CAutoSyncCheckBoxSettingPage(
	                                   R_ASP_MULTI_SELECTION_LIST, list, param);

	
   	CleanupStack::PushL(dlg);
    dlg->SetSettingTextL(aItem.Header());

	CleanupStack::Pop(dlg);

    TBool ret = dlg->ExecuteLD(CAknSettingPage::EUpdateWhenChanged);
	
	if (ret)
    	{
        SetContentSelectionL(list);
        GetContentsNameL(iBuf);
        aItem.SetValueL(iBuf);
    	}

	CleanupStack::PopAndDestroy(list);

   	return ret;
	
	}


//------------------------------------------------------------------------------
// CAspScheduleDialog::CheckAccessPointSelection
// Check if the selected profile has valid access point
//------------------------------------------------------------------------------
//
void CAspScheduleDialog::CheckAccessPointSelectionL(TInt aProfileId)
	{
	
	CAspProfile* profile = NULL;
	TAspParam param(iApplicationId, iSyncSession);
	TBool doCleanUp = EFalse;

	if (iProfile && aProfileId == iProfile->ProfileId())//setting view opened & selected same profile
		{
		profile = iProfile;
		}
	else
		{
		profile = CAspProfile::NewLC(param);//Automatic sync menu opened
		TRAPD(err, profile->OpenL(aProfileId, CAspProfile::EOpenReadWrite,
                                         CAspProfile::EAllProperties));
    	if (err == KErrLocked)
    		{
    		CleanupStack::PopAndDestroy(profile);
    		TDialogUtil::ShowErrorNoteL(R_ASP_LOCKED_PROFILE);
			return;
			}
		doCleanUp = ETrue;
		}

	TInt accessPoint = profile->AccessPointL();
	TAspAccessPointItem item;
    item.iUid = accessPoint;
	CAspAccessPointHandler* apHandler = CAspAccessPointHandler::NewL(param);
	CleanupStack::PushL(apHandler);

	TInt ret = apHandler->GetInternetApInfo(item);
	if (ret == KErrNone)
	   	{
			CleanupStack::PopAndDestroy(apHandler);
			if (doCleanUp)
				{
		 		CleanupStack::PopAndDestroy(profile);
		 		}
		return;
	   		}
		RCmManager cmmgr;
		cmmgr.OpenL();
		TCmDefConnValue defConnValue;
		cmmgr.ReadDefConnL(defConnValue);
		cmmgr.Close();
		if(defConnValue.iType == ECmDefConnDestination)
		{
			CleanupStack::PopAndDestroy(apHandler);
			if (doCleanUp)
		     {
	 		 CleanupStack::PopAndDestroy(profile);
	 		 }
		return;
		}
		
	HBufC* hBuf = CAspResHandler::ReadLC(R_ASP_QUERY_IAP_NO_ASK_ALWAYS);
	if (!TDialogUtil::ShowConfirmationQueryL(hBuf->Des()))
	    {
	  	 CleanupStack::PopAndDestroy(hBuf);
		 CleanupStack::PopAndDestroy(apHandler);
	  	 if (doCleanUp)
		     {
	 		 CleanupStack::PopAndDestroy(profile);
	 		 }
		 return ;// user selected "Always ask" option 
		}
	else
		{
		item.iUid2 = accessPoint;
		TInt ret = apHandler->ShowApSelectDialogL(item);
	
		if (ret == CAspAccessPointHandler::EAspDialogSelect)
			{
			profile->SetAccessPointL(item.iUid);
			profile->Save();
			}
		}	
		
	CleanupStack::PopAndDestroy(hBuf);
	CleanupStack::PopAndDestroy(apHandler);
		
	if (doCleanUp)
	     {
	 	 CleanupStack::PopAndDestroy(profile);
		 }
	return;
	}


//------------------------------------------------------------------------------
// CAspScheduleDialog::GetProfileName
//
//------------------------------------------------------------------------------
//
void CAspScheduleDialog::GetProfileName(TInt& aProfileId, TDes& aText)
	{
	aText = KNullDesC;
	
	TInt index = iProfileList->ListIndex(aProfileId);
    
    if (index == KErrNotFound)
    	{
    	aProfileId = KErrNotFound;
        return; // profile does not exist	
    	}
    	
    TAspProfileItem& item = iProfileList->Item(index);
    aText = item.iProfileName;
	}


//------------------------------------------------------------------------------
// CAspScheduleDialog::GetWeekdayNameL
// 
//------------------------------------------------------------------------------
//
void CAspScheduleDialog::GetWeekdayNameL(TDes& aText)
	{
	const TInt KWeekdayCount = ESunday + 1;
	
	aText = KNullDesC;
	
	TInt selectedDayCount = 0;
	TInt selectedDayIndex = 0;
	
	iSchedule->SelectedDayInfo(selectedDayCount, selectedDayIndex);
		
	if (selectedDayCount == 1)
		{
		aText = (*iWeekdayList)[selectedDayIndex];
		}
	else if (selectedDayCount == KWeekdayCount)
		{
		CAspResHandler::ReadL(aText, R_ASP_SETT_AUTO_SYNC_ALL_DAYS);
		}
	else if (selectedDayCount > 1)
		{
		CAspResHandler::ReadL(aText, R_ASP_SETT_AUTO_SYNC_SEL_DAYS);
		}
    }


//------------------------------------------------------------------------------
// CAspScheduleDialog::GetWeekdaySelectionL
// 
//------------------------------------------------------------------------------
//
void CAspScheduleDialog::GetWeekdaySelectionL(CSelectionItemList* aList)
	{
	for (TInt i=EMonday; i<=ESunday; i++)
		{
		TBuf<KBufSize> buf((*iWeekdayList)[i]);
		TBool enabled = iSchedule->WeekdayEnabled(i);

        CSelectableItem* selectableItem =
                         new (ELeave) CSelectableItem(buf, enabled);
                         
        CleanupStack::PushL(selectableItem);
        selectableItem->ConstructL();
        aList->AppendL(selectableItem);
        CleanupStack::Pop(selectableItem);
		}
	}


//------------------------------------------------------------------------------
// CAspScheduleDialog::SetWeekdaySelectionL
// 
//------------------------------------------------------------------------------
//
void CAspScheduleDialog::SetWeekdaySelectionL(CSelectionItemList* aList)
	{
	TInt count = aList->Count();
	
	__ASSERT_DEBUG(count == ESunday + 1, TUtil::Panic(KErrGeneral));
	
	for (TInt i=0; i<count; i++)
		{
		CSelectableItem* selectableItem = (*aList)[i];
		TBool selected = selectableItem->SelectionStatus();
		iSchedule->SetWeekdayEnabled(i, selected);
		}
	}

//------------------------------------------------------------------------------
// CAspScheduleDialog::SetDefaultdaySelectionL
// 
//------------------------------------------------------------------------------
//
void CAspScheduleDialog::SetDefaultdaySelectionL()
	{
	iSchedule->SetWeekdayEnabled(EMonday,    ETrue);
   	iSchedule->SetWeekdayEnabled(ETuesday,   ETrue);
   	iSchedule->SetWeekdayEnabled(EWednesday, ETrue);
   	iSchedule->SetWeekdayEnabled(EThursday,  ETrue);
   	iSchedule->SetWeekdayEnabled(EFriday,    ETrue);
   	iSchedule->SetWeekdayEnabled(ESaturday,  EFalse);
   	iSchedule->SetWeekdayEnabled(ESunday,    EFalse);
	}

//-----------------------------------------------------------------------------
// CAspScheduleDialog::GetItemForIndex
// 
// Find item in list position aIndex.
//-----------------------------------------------------------------------------
//
CAspListItemData* CAspScheduleDialog::GetItemForIndex(TInt aIndex)
	{
	CAspListItemData* item = NULL;
	
	TInt count = iSettingList->Count();
	for (TInt i=0; i<count; i++)
		{
		CAspListItemData* temp = (*iSettingList)[i];
		if (temp->iIndex == aIndex)
			{
			item = temp;
			break;
			}
		}

	__ASSERT_ALWAYS(item, TUtil::Panic(KErrGeneral));

    return item;
	}


//-----------------------------------------------------------------------------
// CAspScheduleDialog::Item
// 
// Find item with aItemId (TAspConnectionSettingItem).
//-----------------------------------------------------------------------------
//
CAspListItemData* CAspScheduleDialog::Item(TInt aItemId)
	{
	CAspListItemData* item = NULL;

	TInt count = iSettingList->Count();
	for (TInt i=0; i<count; i++)
		{
		CAspListItemData* temp = (*iSettingList)[i];
		if (temp->iItemId == aItemId)
			{
			item = temp;
			break;
			}
		}
	
	__ASSERT_ALWAYS(item, TUtil::Panic(KErrGeneral));

    return item;
	}


// -----------------------------------------------------------------------------
// CAspScheduleDialog::GetTimeTextL
//
// -----------------------------------------------------------------------------
//
void CAspScheduleDialog::GetTimeTextL(TDes& aText, TTime aTime)
	{
	HBufC* hBuf = CAspResHandler::ReadLC(R_QTN_TIME_USUAL_WITH_ZERO);
    aTime.FormatL(aText, *hBuf);
    CleanupStack::PopAndDestroy(hBuf);
	}

//------------------------------------------------------------------------------
// CAspScheduleDialog::GetContentsNameL
// 
//------------------------------------------------------------------------------
//
void CAspScheduleDialog::GetContentsNameL(TDes& aText)
	{
	
	TInt contentCount = iContentList->ProviderCount();
	
	aText = KNullDesC;
	
	TInt selectedContentCnt = 0;
	TInt selectedContentIndex = 0;
	
	iSchedule->ContentSelectionInfo(selectedContentCnt, selectedContentIndex);
		
	if (selectedContentCnt == contentCount)
		{
		CAspResHandler::ReadL(aText, R_ASP_SETT_VALUE_ALL_CONTENTS);
		}
	else if (selectedContentCnt > 0)
		{
		CAspResHandler::ReadL(aText, R_ASP_SETT_VALUE_SELECTED_CONTENTS);
		}
    }



//------------------------------------------------------------------------------
// CAspScheduleDialog::GetContentSelectionL
// Get selected contents 
//------------------------------------------------------------------------------
//

void CAspScheduleDialog::GetContentSelectionL(CSelectionItemList* aList)
	{
	
	TInt providerCount = iContentList->ProviderCount();
	
	CreateContentListL();
	for (TInt index = 0; index < providerCount; index++)
		{
		TBuf<KBufSize> buf((*iContentsList)[index]);
		TBool enabled = iSchedule->ContentEnabled(index);

        CSelectableItem* selectableItem =
                         new (ELeave) CSelectableItem(buf, enabled);
                         
        CleanupStack::PushL(selectableItem);
        selectableItem->ConstructL();
        aList->AppendL(selectableItem);
        CleanupStack::Pop(selectableItem);
		}
	}

//------------------------------------------------------------------------------
// CAspScheduleDialog::CreateContentList
// 
//------------------------------------------------------------------------------
//
void CAspScheduleDialog::CreateContentListL()
	{

	TInt providerCount = iContentList->ProviderCount();

	for (TInt index = 0; index < providerCount; index++)
		{
		HBufC* name = NULL;
		TAspProviderItem& provider = iContentList->ProviderItem(index);
		name = CAspResHandler::GetContentNameLC(
		            provider.iDataProviderId, provider.iDisplayName);	
		//iContentsList->AppendL(provider.iDisplayName);
		iContentsList->AppendL(*name);
		CleanupStack::PopAndDestroy();//name
		}
	}
	

//------------------------------------------------------------------------------
// CAspScheduleDialog::SetDefaultContentL
// Set default content selection
//------------------------------------------------------------------------------
//
void CAspScheduleDialog::SetDefaultContentL()
	{
	
	TInt providerCount = iContentList->ProviderCount();
	
	for (TInt index = 0; index < providerCount; index++)
		{
		TAspProviderItem& provider = iContentList->ProviderItem(index);
			
		if (provider.iDataProviderId == KUidNSmlAdapterEMail.iUid)
			{
			if (iSchedule->MailboxExistL())
				{
				iSchedule->SetContentEnabled(index, ETrue);
				}
			else
				{
				iSchedule->SetContentEnabled(index, EFalse);
				}

			if (iSchedule->ProfileId() != KErrNotFound
		 					&& !iSchedule->ProtocolL())
				{
				iSchedule->SetContentEnabled(index, EFalse);
				}
			}
		else
			{
			iSchedule->SetContentEnabled(index, ETrue);
			}
		}
	
	}

//------------------------------------------------------------------------------
// CAspScheduleDialog::SetContentSelectionL
// Set selected contents 
//------------------------------------------------------------------------------
//
	
void CAspScheduleDialog::SetContentSelectionL(CSelectionItemList* aList)
	{
    
    TInt count = aList->Count();

	for (TInt i=0; i<count; i++)
		{
		CSelectableItem* selectableItem = (*aList)[i];
		TBool selected = selectableItem->SelectionStatus();
		iSchedule->SetContentEnabled(i, selected);
		}
    
    }

//------------------------------------------------------------------------------
// CAspScheduleDialog::GetSyncFrequencyListLC
// 
//------------------------------------------------------------------------------
//

CDesCArray* CAspScheduleDialog::GetSyncFrequencyListLC()
	{
	const TInt KScheduleCount = 5;
	CDesCArray* scheduleList = new (ELeave) CDesCArrayFlat(KScheduleCount);
	CleanupStack::PushL(scheduleList);

	GetSyncFrequencyValueL(CAspSchedule::EIntervalManual, iBuf);
	scheduleList->AppendL(iBuf);
	
	GetSyncFrequencyValueL(CAspSchedule::EManyTimesPerDay, iBuf);
	scheduleList->AppendL(iBuf);

	GetSyncFrequencyValueL(CAspSchedule::EInterval24hours, iBuf);
	scheduleList->AppendL(iBuf);
	
	TInt keyVal;
	TRAPD (err ,ReadRepositoryL(EKeySyncFrequency, keyVal));
	if (err == KErrNone)
		{
		TUint mask = 1;
		for(TInt schedule = 0; schedule < KScheduleCount; ++schedule)
			{
			TInt offsetSyncFreq = 10;
			if (mask & keyVal)
				{
				offsetSyncFreq +=schedule;
				GetSyncFrequencyValueL(offsetSyncFreq ,iBuf);
				scheduleList->AppendL(iBuf);		
				}
			mask = mask << 1;
			}
		}
	else
		{
		//defualt items in the list
		
		GetSyncFrequencyValueL(CAspSchedule::EInterval2days, iBuf);
		scheduleList->AppendL(iBuf);

		GetSyncFrequencyValueL(CAspSchedule::EInterval7days, iBuf);
		scheduleList->AppendL(iBuf);

		GetSyncFrequencyValueL(CAspSchedule::EInterval30days, iBuf);
		scheduleList->AppendL(iBuf);
		
		}
	return scheduleList;

	}

//------------------------------------------------------------------------------
// CAspScheduleDialog::GetSyncFrequencyValueL
// 
//------------------------------------------------------------------------------
//

void CAspScheduleDialog::GetSyncFrequencyValueL(TInt schedule, TDes& aBuf)
	{
	aBuf = KNullDesC;
	
	switch(schedule)
		{
		case CAspSchedule::EIntervalManual:
			iResHandler->ReadL(aBuf, R_ASP_SETT_AUTO_SYNC_FREQ_VALUE_MANUAL);
			break;
		case CAspSchedule::EInterval24hours:
			iResHandler->ReadL(aBuf, R_ASP_SETT_AUTO_VALUE_24_HOURS);
			break;
		case CAspSchedule::EInterval2days:
			iResHandler->ReadL(aBuf, R_ASP_SETT_AUTO_VALUE_2_DAYS);
			break;
		case CAspSchedule::EInterval4days:
			iResHandler->ReadL(aBuf, R_ASP_SETT_AUTO_VALUE_4_DAYS);
			break;
		case CAspSchedule::EInterval7days:
			iResHandler->ReadL(aBuf, R_ASP_SETT_AUTO_VALUE_7_DAYS);
			break;
		case CAspSchedule::EInterval14days:
			iResHandler->ReadL(aBuf, R_ASP_SETT_AUTO_VALUE_14_DAYS);
			break;
		case CAspSchedule::EInterval30days:
			iResHandler->ReadL(aBuf, R_ASP_SETT_AUTO_VALUE_30_DAYS);
			break;
		case CAspSchedule::EManyTimesPerDay:
			iResHandler->ReadL(aBuf, R_ASP_SETT_AUTO_SYNC_FREQ_MANY_A_DAY);
			break;
			
		default:
			break;
		}
		
	}

// ----------------------------------------------------------------------------
// CAspScheduleDialog::PeakSchedule
// Get sync frequency number from string
// ----------------------------------------------------------------------------
//
TInt CAspScheduleDialog::SyncFrequencyL(const TDesC& aBuf)
	{
	       		
		iResHandler->ReadL(iBuf, R_ASP_SETT_AUTO_SYNC_FREQ_VALUE_MANUAL);
		if(iBuf.Compare(aBuf) == 0)
			{
			return CAspSchedule::EIntervalManual;
			}
		iResHandler->ReadL(iBuf, R_ASP_SETT_AUTO_SYNC_FREQ_MANY_A_DAY);
		if(iBuf.Compare(aBuf) == 0)
			{
			return CAspSchedule::EManyTimesPerDay;
			}
		iResHandler->ReadL(iBuf, R_ASP_SETT_AUTO_VALUE_24_HOURS);
		if(iBuf.Compare(aBuf) == 0)
			{
			return CAspSchedule::EInterval24hours;
			}
		iResHandler->ReadL(iBuf, R_ASP_SETT_AUTO_VALUE_2_DAYS);
		if(iBuf.Compare(aBuf) == 0)
			{
			return CAspSchedule::EInterval2days;
			}
		iResHandler->ReadL(iBuf, R_ASP_SETT_AUTO_VALUE_4_DAYS);
		if(iBuf.Compare(aBuf) == 0)
			{
			return CAspSchedule::EInterval4days;
			}
		iResHandler->ReadL(iBuf, R_ASP_SETT_AUTO_VALUE_7_DAYS);
		if(iBuf.Compare(aBuf) == 0)
			{
			return CAspSchedule::EInterval7days;
			}
		iResHandler->ReadL(iBuf, R_ASP_SETT_AUTO_VALUE_14_DAYS);
		if(iBuf.Compare(aBuf) == 0)
			{
			return CAspSchedule::EInterval14days;
			}
		iResHandler->ReadL(iBuf, R_ASP_SETT_AUTO_VALUE_30_DAYS);
		if(iBuf.Compare(aBuf) == 0)
			{
			return CAspSchedule::EInterval30days;
			}
		
	   	
    	return KErrNotFound;
	}

// ----------------------------------------------------------------------------
// CAspScheduleDialog::GetSyncFrequencyPositionL
// Get position of selected interval from the interval list
// ----------------------------------------------------------------------------
//
TInt CAspScheduleDialog::GetSyncFrequencyPositionL(CDesCArray* arr, TInt aInterval)
	{
	const TInt KScheduleCount = 8;
	for (TInt i=0; i < KScheduleCount; i++)
		{
		if(aInterval == SyncFrequencyL((*arr)[i]))
		return i;
		}
	return KErrNotFound;

	}

//------------------------------------------------------------------------------
// CAspScheduleDialog::GetPeakScheduleListLC
// 
//------------------------------------------------------------------------------
//

CDesCArray* CAspScheduleDialog::GetPeakScheduleListLC()
	{
	const TInt KScheduleCount = 7;
	CDesCArray* scheduleList = new (ELeave) CDesCArrayFlat(KScheduleCount);
	CleanupStack::PushL(scheduleList);

	GetPeakScheduleValueL(CAspSchedule::EIntervalManual, iBuf);
	scheduleList->AppendL(iBuf);

	TInt keyVal;
	TRAPD (err,	ReadRepositoryL(EKeyPeakSyncInterval, keyVal));
	if (err == KErrNone)
		{
		TUint mask = 1;
		for(TInt schedule = 1; schedule <= KScheduleCount; ++schedule)
			{
			if (mask & keyVal)
				{
				GetPeakScheduleValueL(schedule ,iBuf);
				scheduleList->AppendL(iBuf);		
				}
			mask = mask << 1;
			}
		}
	else
		{
		GetPeakScheduleValueL(CAspSchedule::EInterval15Mins, iBuf);
		scheduleList->AppendL(iBuf);

		GetPeakScheduleValueL(CAspSchedule::EInterval1hour, iBuf);
		scheduleList->AppendL(iBuf);

		GetPeakScheduleValueL(CAspSchedule::EInterval2hours, iBuf);
		scheduleList->AppendL(iBuf);

		GetPeakScheduleValueL(CAspSchedule::EInterval4hours, iBuf);
		scheduleList->AppendL(iBuf);

		GetPeakScheduleValueL(CAspSchedule::EInterval12hours, iBuf);
		scheduleList->AppendL(iBuf);
		}
	return scheduleList;

	}
//------------------------------------------------------------------------------
// CAspScheduleDialog::GetPeakScheduleValueL
// 
//------------------------------------------------------------------------------
//

void CAspScheduleDialog::GetPeakScheduleValueL(TInt schedule, TDes& aBuf)
	{
	aBuf = KNullDesC;
	
	switch(schedule)
		{
		case CAspSchedule::EIntervalManual:
			iResHandler->ReadL(aBuf, R_ASP_SETT_AUTO_VALUE_MANUAL);
			break;
		case CAspSchedule::EInterval15Mins:
			iResHandler->ReadL(aBuf, R_ASP_SETT_AUTO_VALUE_15_MINS);
			break;
		case CAspSchedule::EInterval30Mins:
			iResHandler->ReadL(aBuf, R_ASP_SETT_AUTO_VALUE_30_MINS);
			break;
		case CAspSchedule::EInterval1hour:
			iResHandler->ReadL(aBuf, R_ASP_SETT_AUTO_VALUE_1_HOUR);
			break;
		case CAspSchedule::EInterval2hours:
			iResHandler->ReadL(aBuf, R_ASP_SETT_AUTO_VALUE_2_HOURS);
			break;
		case CAspSchedule::EInterval4hours:
			iResHandler->ReadL(aBuf, R_ASP_SETT_AUTO_VALUE_4_HOURS);
			break;
		case CAspSchedule::EInterval8hours:
			iResHandler->ReadL(aBuf, R_ASP_SETT_AUTO_VALUE_8_HOURS);
			break;
		case CAspSchedule::EInterval12hours:
			iResHandler->ReadL(aBuf, R_ASP_SETT_AUTO_VALUE_12_HOURS);
			break;
				
		default:
			break;
		}
		
	}

//------------------------------------------------------------------------------
// CAspScheduleDialog::GetOffPeakScheduleListLC
// 
//------------------------------------------------------------------------------
//

CDesCArray* CAspScheduleDialog::GetOffPeakScheduleListLC()
	{
	const TInt KScheduleCount = 7;
	CDesCArray* scheduleList = new (ELeave) CDesCArrayFlat(KScheduleCount);
	CleanupStack::PushL(scheduleList);

	GetOffPeakScheduleValueL(CAspSchedule::EIntervalManual, iBuf);
	scheduleList->AppendL(iBuf);

	TInt keyVal;
	TRAPD (err,	ReadRepositoryL(EKeyOffPeakSyncInterval, keyVal));
	if (err == KErrNone)
		{
		TUint mask = 1;
		for(TInt schedule = 1; schedule <= KScheduleCount; ++schedule)
			{
			if (mask & keyVal)
				{
				GetOffPeakScheduleValueL(schedule ,iBuf);
				scheduleList->AppendL(iBuf);		
				}
			mask = mask << 1;
			}
		}
	else
		{
		GetOffPeakScheduleValueL(CAspSchedule::EInterval15Mins, iBuf);
		scheduleList->AppendL(iBuf);

		GetOffPeakScheduleValueL(CAspSchedule::EInterval1hour, iBuf);
		scheduleList->AppendL(iBuf);

		GetOffPeakScheduleValueL(CAspSchedule::EInterval2hours, iBuf);
		scheduleList->AppendL(iBuf);

		GetOffPeakScheduleValueL(CAspSchedule::EInterval4hours, iBuf);
		scheduleList->AppendL(iBuf);

		GetOffPeakScheduleValueL(CAspSchedule::EInterval12hours, iBuf);
		scheduleList->AppendL(iBuf);
		}
	
	return scheduleList;
	}

//------------------------------------------------------------------------------
// CAspScheduleDialog::GetOffPeakScheduleValueL
// 
//------------------------------------------------------------------------------
//
void CAspScheduleDialog::GetOffPeakScheduleValueL(TInt schedule, TDes& aBuf)
	{
	aBuf = KNullDesC;
	
	switch(schedule)
		{
		case CAspSchedule::EIntervalManual:
			iResHandler->ReadL(aBuf, R_ASP_SETT_AUTO_VALUE_OFF_MANUAL);
			break;
		case CAspSchedule::EInterval15Mins:
			iResHandler->ReadL(aBuf, R_ASP_SETT_AUTO_VALUE_OFF_15_MINS);
			break;
		case CAspSchedule::EInterval30Mins:
			iResHandler->ReadL(aBuf, R_ASP_SETT_AUTO_VALUE_OFF_30_MINS);
			break;
		case CAspSchedule::EInterval1hour:
			iResHandler->ReadL(aBuf, R_ASP_SETT_AUTO_VALUE_OFF_1_HOUR);
			break;
		case CAspSchedule::EInterval2hours:
			iResHandler->ReadL(aBuf, R_ASP_SETT_AUTO_VALUE_OFF_2_HOURS);
			break;
		case CAspSchedule::EInterval4hours:
			iResHandler->ReadL(aBuf, R_ASP_SETT_AUTO_VALUE_OFF_4_HOURS);
			break;
		case CAspSchedule::EInterval8hours:
			iResHandler->ReadL(aBuf, R_ASP_SETT_AUTO_VALUE_OFF_8_HOURS);
			break;
		case CAspSchedule::EInterval12hours:
			iResHandler->ReadL(aBuf, R_ASP_SETT_AUTO_VALUE_OFF_12_HOURS);
			break;
		
		default:
			break;
		}
	
	}

// ----------------------------------------------------------------------------
// CAspScheduleDialog::PeakScheduleL
// Get peak schedule number from string
// ----------------------------------------------------------------------------
//
TInt CAspScheduleDialog::PeakScheduleL(const TDesC& aBuf)
	{
	       		
		iResHandler->ReadL(iBuf, R_ASP_SETT_AUTO_VALUE_MANUAL);
		if(iBuf.Compare(aBuf) == 0)
			{
			return CAspSchedule::EIntervalManual;
			}
		iResHandler->ReadL(iBuf, R_ASP_SETT_AUTO_VALUE_15_MINS);
		if(iBuf.Compare(aBuf) == 0)
			{
			return CAspSchedule::EInterval15Mins;
			}
		iResHandler->ReadL(iBuf, R_ASP_SETT_AUTO_VALUE_30_MINS);
		if(iBuf.Compare(aBuf) == 0)
			{
			return CAspSchedule::EInterval30Mins;
			}
		iResHandler->ReadL(iBuf, R_ASP_SETT_AUTO_VALUE_1_HOUR);
		if(iBuf.Compare(aBuf) == 0)
			{
			return CAspSchedule::EInterval1hour;
			}
        iResHandler->ReadL(iBuf, R_ASP_SETT_AUTO_VALUE_2_HOURS);
		if(iBuf.Compare(aBuf) == 0)
			{
			return CAspSchedule::EInterval2hours;
			}
		iResHandler->ReadL(iBuf, R_ASP_SETT_AUTO_VALUE_4_HOURS);
		if(iBuf.Compare(aBuf) == 0)
			{
			return CAspSchedule::EInterval4hours;
			}
	    iResHandler->ReadL(iBuf, R_ASP_SETT_AUTO_VALUE_8_HOURS);
		if(iBuf.Compare(aBuf) == 0)
			{
			return CAspSchedule::EInterval8hours;
			}
		iResHandler->ReadL(iBuf, R_ASP_SETT_AUTO_VALUE_12_HOURS);
		if(iBuf.Compare(aBuf) == 0)
			{
			return CAspSchedule::EInterval12hours;
			}
		
	   	
    	return KErrNotFound;
	}

// ----------------------------------------------------------------------------
// CAspScheduleDialog::OffPeakScheduleL
// Get off-peak schedule number from string
// ----------------------------------------------------------------------------
//
TInt CAspScheduleDialog::OffPeakScheduleL(const TDesC& aBuf)
	{
	       		
		iResHandler->ReadL(iBuf, R_ASP_SETT_AUTO_VALUE_OFF_MANUAL);
		if(iBuf.Compare(aBuf) == 0)
			{
			return CAspSchedule::EIntervalManual;
			}
		iResHandler->ReadL(iBuf, R_ASP_SETT_AUTO_VALUE_OFF_15_MINS);
		if(iBuf.Compare(aBuf) == 0)
			{
			return CAspSchedule::EInterval15Mins;
			}
		iResHandler->ReadL(iBuf, R_ASP_SETT_AUTO_VALUE_OFF_30_MINS);
		if(iBuf.Compare(aBuf) == 0)
			{
			return CAspSchedule::EInterval30Mins;
			}
		iResHandler->ReadL(iBuf, R_ASP_SETT_AUTO_VALUE_OFF_1_HOUR);
		if(iBuf.Compare(aBuf) == 0)
			{
			return CAspSchedule::EInterval1hour;
			}
        iResHandler->ReadL(iBuf, R_ASP_SETT_AUTO_VALUE_OFF_2_HOURS);
		if(iBuf.Compare(aBuf) == 0)
			{
			return CAspSchedule::EInterval2hours;
			}
		iResHandler->ReadL(iBuf, R_ASP_SETT_AUTO_VALUE_OFF_4_HOURS);
		if(iBuf.Compare(aBuf) == 0)
			{
			return CAspSchedule::EInterval4hours;
			}
	    iResHandler->ReadL(iBuf, R_ASP_SETT_AUTO_VALUE_OFF_8_HOURS);
		if(iBuf.Compare(aBuf) == 0)
			{
			return CAspSchedule::EInterval8hours;
			}
		iResHandler->ReadL(iBuf, R_ASP_SETT_AUTO_VALUE_OFF_12_HOURS);
		if(iBuf.Compare(aBuf) == 0)
			{
			return CAspSchedule::EInterval12hours;
			}
		
		
    	return KErrNotFound;
	}

// ----------------------------------------------------------------------------
// CAspScheduleDialog::GetPeakPositionL
// Get position of selected interval from the interval list
// ----------------------------------------------------------------------------
//
TInt CAspScheduleDialog::GetPeakPositionL(CDesCArray* arr, TInt aInterval)
	{
	const TInt KScheduleCount = 6;
	for (TInt i=0; i < KScheduleCount; i++)
		{
		if(aInterval == PeakScheduleL((*arr)[i]))
		return i;
		}
	return KErrNotFound;

	}

// ----------------------------------------------------------------------------
// CAspScheduleDialog::GetOffPeakPositionL
// Get position of selected interval from the interval list
// ----------------------------------------------------------------------------
//
TInt CAspScheduleDialog::GetOffPeakPositionL(CDesCArray* arr, TInt aInterval)
	{
	const TInt KScheduleCount = 6;
	for (TInt i=0; i < KScheduleCount; i++)
		{
		if(aInterval == OffPeakScheduleL((*arr)[i]))
		return i;
		}
	return KErrNotFound;

	}

// ----------------------------------------------------------------------------
// CAspScheduleDialog::CheckMandatoryDataL
// 
// ----------------------------------------------------------------------------
//

void CAspScheduleDialog::CheckMandatoryDataL()
	{
	FLOG( _L("CAspScheduleDialog::CheckMandatoryDataL START") );
	
	if (iSchedule->ProfileId() == KErrNotFound)
		{
		return;
		}
	if (!iSchedule->IsAutoSyncEnabled())
		{
		return;
		}
	
	TInt incompleteCount;
	TInt firstItem;

	TBool ret = iSchedule->CheckMandatoryDataL(incompleteCount, firstItem);
	if (!ret)
		{
		if (!TDialogUtil::ShowConfirmationQueryL(R_ASP_NOTE_AUTO_CONTENTS_MISSING_SETTINGS))
			{
			return ; 
			}

		TAspParam param(iApplicationId, iSyncSession);
			
		TBool doCleanUp = EFalse;

		CAspProfile* profile = NULL;
		TInt aProfileId = iSchedule->ProfileId();

		if (iProfile && (aProfileId == iProfile->ProfileId()))//setting view opened & selected same profile
			{
			profile = iProfile;
			}
		else
			{
			profile = CAspProfile::NewLC(param);//Automatic sync menu opened
			TRAPD(err, profile->OpenL(aProfileId, CAspProfile::EOpenReadWrite,
                                         CAspProfile::EAllProperties));
		 
    		if (err == KErrLocked)
    			{
    			CleanupStack::PopAndDestroy(profile);
    			TDialogUtil::ShowErrorNoteL(R_ASP_LOCKED_PROFILE);
				return ;
				}
			doCleanUp = ETrue;
			iContentList->SetProfile(profile);
			}

		TAspProviderItem& provider = iContentList->ProviderItem(firstItem);	
		param.iProfile = profile;
		param.iContentList = iContentList;
		param.iDataProviderId = provider.iDataProviderId;
		param.iProfileList = iProfileList;
		param.iMode = iEditMode;
		param.iSchedule = iSchedule;
		if (incompleteCount == 1)
			{
			CAspContentDialog::ShowDialogL(param);
			}
		else if (incompleteCount >1)
			{
			CAspSettingViewDialog::ShowDialogL(param);
			}
			
		if (doCleanUp)
			{
			iContentList->SetProfile(NULL);
			CleanupStack::PopAndDestroy(profile);
			}

		}

	if (!CheckBearerType())
			{
			iSchedule->SetProfileId(KErrNotFound);
			return;
			}
	if (!iSchedule->MailboxExistL() || !iSchedule->ProtocolL())
			{
			TInt emailIndex = iContentList->FindProviderIndex(KUidNSmlAdapterEMail.iUid);
			iSchedule->SetContentEnabled(emailIndex, EFalse);
			}
		
	FLOG( _L("CAspScheduleDialog::CheckMandatoryDataL END") );
	
	}
// ----------------------------------------------------------------------------
// CAspScheduleDialog::ShowAutoSyncInfoL
// Show next auto sync time 
// ----------------------------------------------------------------------------
//
void CAspScheduleDialog::ShowAutoSyncInfoL()
	{
	FLOG( _L("CAspScheduleDialog::ShowAutoSyncInfoL START") );
	if (iSchedule->ProfileId() == KErrNotFound)
		{
		iSchedule->SetDailySyncEnabled(EFalse);
		iSchedule->SetPeakSyncEnabled(EFalse);
		iSchedule->SetOffPeakSyncEnabled(EFalse);
        FLOG( _L("Profile Not Found") );
		return ;
		}
		
	if (!iSchedule->IsAutoSyncEnabled())
		{
		return;
		}

	if (iSchedule->PeakSyncEnabled() && !iSchedule->OffPeakSyncEnabled())
		{
		TInt dayCount = 0;
	    TInt dayIndex = 0;
        iSchedule->SelectedDayInfo(dayCount, dayIndex); 
		if (dayCount == 0)
			{
			return;
			}
		}
	if (iSchedule->PeakSyncEnabled() && (iSchedule->IsValidPeakScheduleL() == 0)
						|| iSchedule->OffPeakSyncEnabled() && (iSchedule->IsValidOffPeakScheduleL() == 0 ))
		{
		return;
		}
	
	TTime currentTime;
	currentTime.HomeTime();
	TDateTime today = currentTime.DateTime();
	
	TDateTime startDay = iSchedule->AutoSyncScheduleTime().DateTime();
	
	if (iSchedule->SyncFrequency() >= CAspSchedule::EInterval24hours)
		{
		if (startDay.Hour() == today.Hour() && startDay.Minute() <= today.Minute())
			{
			return;
			}
		else if (startDay.Hour() < today.Hour())
			{
			return;
			}
		}

	TTime nextSync;
    iSchedule->GetStartTimeL(nextSync ,ETrue);
   	 
	TDateTime syncDay = nextSync.DateTime();

	TBuf<KBufSize> aTimeString;
	if(today.Day() == syncDay.Day() && today.Month() == syncDay.Month())
		{
		//show next sync time
		TBuf<KBufSize> timeFormat;
		CAspResHandler::ReadL(timeFormat, R_QTN_TIME_USUAL_WITH_ZERO);
		nextSync.FormatL(aTimeString, timeFormat);
		}
	else 
		{
		//show sync day
		TBuf<KBufSize> dateFormat;
		CAspResHandler::ReadL(dateFormat, R_QTN_DATE_USUAL_WITH_ZERO);
		nextSync.FormatL(aTimeString, dateFormat);
		}
	
	HBufC* hBuf = CAspResHandler::ReadLC(R_ASP_SETT_AUTO_SYNC_DONE, aTimeString);
	TDialogUtil::ShowInformationNoteL(hBuf->Des());

	CleanupStack::PopAndDestroy(hBuf);

	FLOG( _L("CAspScheduleDialog::ShowAutoSyncInfoL END") );

   	
	}


// ----------------------------------------------------------------------------
// CAspScheduleDialog::CheckBearerType
// Return ETrue if bearer type of the selected profile is internet & EFalse otherwise
// ----------------------------------------------------------------------------
//
TBool CAspScheduleDialog::CheckBearerType()
	{
	TAspParam param(iApplicationId, iSyncSession);
	
	CAspProfile* profile = CAspProfile::NewLC(param);

	TInt aProfileId = iSchedule->ProfileId();
	TRAPD(err, profile->OpenL(aProfileId, CAspProfile::EOpenRead,
                                         CAspProfile::EAllProperties));
	User::LeaveIfError(err);
	if (profile->BearerType() != EAspBearerInternet)
		{
		CleanupStack::PopAndDestroy(profile);
		return EFalse;
		}
	CleanupStack::PopAndDestroy(profile);
	return ETrue;
	}
// ----------------------------------------------------------------------------
// CAspScheduleDialog::CheckPeakTime
// Check peak time definition
// ----------------------------------------------------------------------------
//
TBool CAspScheduleDialog::CheckPeakTime()
	{
	if (Item(EAspAutoSyncPeakSchedule)->iNumberData == CAspSchedule::EIntervalManual
				&& Item(EAspAutoSyncOffPeakSchedule)->iNumberData == CAspSchedule::EIntervalManual)
		{
		return ETrue;
		}
	if (Item(EAspAutoSyncPeakStartTime)->iTime >= Item(EAspAutoSyncPeakEndTime)->iTime)
		{
		return EFalse;
		}
	return ETrue;
	}

// ----------------------------------------------------------------------------
// CAspScheduleDialog::CheckValidityL
// Check validity of content selected
// ----------------------------------------------------------------------------
//
TBool CAspScheduleDialog::CheckValidityL()
	{
	TBool ret = ETrue;
	TRAPD(err, ret = iSchedule->MailboxExistL())
	User::LeaveIfError(err);
	if (!ret)
		{
		TDialogUtil::ShowErrorNoteL(R_ASP_NO_MAILBOXES);
		TInt emailIndex = iContentList->FindProviderIndex(KUidNSmlAdapterEMail.iUid);
		iSchedule->SetContentEnabled(emailIndex, EFalse);
		return EFalse;  // no syncml mailbox
		}
	TInt profileId = Item(EAspAutoSyncProfile)->iNumberData;
	if (profileId != KErrNotFound)
		{
		TAspParam param(iApplicationId, iSyncSession);

		CAspProfile* profile = CAspProfile::NewLC(param);
		profile->OpenL(profileId, CAspProfile::EOpenRead,
                                         CAspProfile::EBaseProperties);
		TInt protocol = profile->ProtocolVersion();
		CleanupStack::PopAndDestroy(profile);

		}
	return ETrue;
	}
// ----------------------------------------------------------------------------
// CAspScheduleDialog::EmailIndex
//  ----------------------------------------------------------------------------
//
TInt CAspScheduleDialog::EmailIndex()
	{
	TInt index = iContentList->FindProviderIndex(KUidNSmlAdapterEMail.iUid);
	return index;

	}
	
// -----------------------------------------------------------------------------
// CAspScheduleDialog::ReadRepositoryL
//
// -----------------------------------------------------------------------------
//
void CAspScheduleDialog::ReadRepositoryL(TInt aKey, TInt& aValue)
	{
	const TUid KRepositoryId = KCRUidNSmlDSApp;
	const TInt KSyncFrequencyCount = 3;
	const TInt KSyncIntervalCount = 5;

    CRepository* rep = CRepository::NewLC(KRepositoryId);
    TInt err = rep->Get(aKey, aValue);
	User::LeaveIfError(err);
	CleanupStack::PopAndDestroy(rep);
    

	TInt keyVal = aValue;
	if (aKey == EKeySyncFrequency)
		{
		TInt intervalCount = 0;
		TUint mask = 1;
		TInt maxSchedules = 5;
		for (TInt i = 0 ;i < maxSchedules ;i++)
			{
			if (mask & keyVal)
				{
				intervalCount++;
				}
			mask = mask << 1;
			}
		if (keyVal >= 0x32 || intervalCount != KSyncFrequencyCount)
			{
			User::Leave(KErrGeneral);
			}
		}
	else if (aKey == EKeyPeakSyncInterval ||
									aKey == EKeyOffPeakSyncInterval)
		{
		TInt intervalCount = 0;
		TUint mask = 1;
		TInt maxSchedules = 7;
		for (TInt i = 0 ;i < maxSchedules ;i++)
			{
			if (mask & keyVal)
				{
				intervalCount++;
				}
			mask = mask << 1;
			}
		if (keyVal >= 0x128 || intervalCount != KSyncIntervalCount)
			{
			User::Leave(KErrGeneral);
			}
		}
	}

//------------------------------------------------------------------------------
// CAutoSyncCheckBoxSettingPage::CAspCheckBoxSettingPage
// 
//------------------------------------------------------------------------------
//
CAutoSyncCheckBoxSettingPage::CAutoSyncCheckBoxSettingPage(TInt aResourceID,
                   CSelectionItemList* aItemArray, TAutoSyncSettingPageParam& aParam)
                   
 : CAknCheckBoxSettingPage(aResourceID, aItemArray), iParam(aParam)
	{
    iSelectionItemList = aItemArray;
	}


//------------------------------------------------------------------------------
// Destructor
// 
//------------------------------------------------------------------------------
//
CAutoSyncCheckBoxSettingPage::~CAutoSyncCheckBoxSettingPage()
	{
	
	}


//------------------------------------------------------------------------------
// CAutoSyncCheckBoxSettingPage::ProcessCommandL
// 
//------------------------------------------------------------------------------
//
void CAutoSyncCheckBoxSettingPage::ProcessCommandL(TInt aCommandId)
	{
    if (aCommandId == EAknSoftkeyUnmark || aCommandId == EAknSoftkeyMark)
    	{
        
        CAknSetStyleListBox* listbox = ListBoxControl();
    	TInt index = listbox->CurrentItemIndex();
       	CSelectableItem* selectableItem = (*iSelectionItemList)[index];
       	TBool ret = ETrue;

		UpdateCbaL(R_ASP_CBA_OK_CANCEL_MARK);
		TInt emailIndex = iParam.iObserver->EmailIndex();
		if (index == emailIndex)
			{
       		ret = iParam.iObserver->CheckValidityL();
			}       	
		
		TInt count = ListBoxControl()->SelectionIndexes()->Count();
		TInt selected = count - 1;
		if (selectableItem->SelectionStatus())
			{
        	if (!selected)//all unselected
				{
				UpdateCbaL(R_ASP_CBA_EMPTY_CANCEL_MARK);
				}
			}
		else
			{
			if (index == emailIndex && !ret && !count)
				{
				UpdateCbaL(R_ASP_CBA_EMPTY_CANCEL_MARK);
				}
			}
			
       	if (ret)
       		{
            CAknCheckBoxSettingPage::ProcessCommandL(aCommandId);
            return;
       		}
       	else
       		{
       		return;	
       		}
    	}
    
        	
    iParam.iCommandId = aCommandId; // return command id to caller

    CAknCheckBoxSettingPage::ProcessCommandL(aCommandId);
	}
	

//------------------------------------------------------------------------------
// CAutoSyncCheckBoxSettingPage::OfferKeyEventL
// 
//------------------------------------------------------------------------------
//
TKeyResponse CAutoSyncCheckBoxSettingPage::OfferKeyEventL(const TKeyEvent& aKeyEvent, 
                                                 TEventCode aType)
    {
		    if(EKeyEnter == aKeyEvent.iCode)
		    {
		    	ProcessCommandL(EAknSoftkeyMark);
		    	return EKeyWasConsumed;
		    }
		    return CAknCheckBoxSettingPage::OfferKeyEventL(aKeyEvent, aType);
    }


// -----------------------------------------------------------------------------
// CAutoSyncCheckBoxSettingPage::UpdateCbaL
// 
// -----------------------------------------------------------------------------
//
void CAutoSyncCheckBoxSettingPage::UpdateCbaL(TInt aResourceId)
    {
    CEikButtonGroupContainer* cba = Cba();
    cba->SetCommandSetL(aResourceId);
    cba->DrawDeferred();
    }
// -----------------------------------------------------------------------------
// CAutoSyncCheckBoxSettingPage::DynamicInitL
// 
// -----------------------------------------------------------------------------
//
void CAutoSyncCheckBoxSettingPage::DynamicInitL()
	{
	TInt count = iSelectionItemList->Count();
	for (TInt i=0; i<count; i++)
		{
		CSelectableItem* selectableItem = (*iSelectionItemList)[i];
		TBool selected = selectableItem->SelectionStatus();
		if (selected)
			{
			return;
			}
		}
	UpdateCbaL(R_ASP_CBA_EMPTY_CANCEL_MARK);
	}
	
// ----------------------------------------------------------------------------
// CAutoSyncCheckBoxSettingPage::HandleListBoxEventL
//
// ----------------------------------------------------------------------------
//

				 
void CAutoSyncCheckBoxSettingPage::HandleListBoxEventL(CEikListBox* aListBox, TListBoxEvent aEventType)
{
	TInt index = 0;
	CSelectableItem* selectableItem;
	if( AknLayoutUtils::PenEnabled() )  
	{
	  	switch(aEventType)
	
		{   
			 case EEventItemSingleClicked:
       		 	 index=aListBox->CurrentItemIndex();
       		 	 selectableItem = (*iSelectionItemList)[index];
       		 	 if(selectableItem->SelectionStatus())
       		 	 {
       		   		this->ProcessCommandL(EAknSoftkeyUnmark);	
       		 	 }
       		     else
       		     {
       			    this->ProcessCommandL(EAknSoftkeyMark);
       		     }
		        break;
	 	default:
			    break;
		}	
	}

}

//  End of File  
