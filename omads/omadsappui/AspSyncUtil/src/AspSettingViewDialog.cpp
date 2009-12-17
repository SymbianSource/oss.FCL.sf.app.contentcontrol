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

#include "AspSettingViewDialog.h"
#include "AspProfileDialog.h"
#include "AspContentDialog.h"
#include "AspScheduleDialog.h"

#include "AspSyncUtil.rh"
#include "AspDebug.h"
#include <csxhelp/ds.hlp.hrh>

#include <aspsyncutil.mbg>  // for bitmap enumerations
#include <AknIconArray.h>   // for GulArray

#include "AspSchedule.h"


// ============================ MEMBER FUNCTIONS ===============================



// -----------------------------------------------------------------------------
// CAspSettingViewDialog::ShowDialogL
// 
// -----------------------------------------------------------------------------
TBool CAspSettingViewDialog::ShowDialogL(const TAspParam& aParam)
	{
    CAspSettingViewDialog* dialog = CAspSettingViewDialog::NewL(aParam);

	TBool ret = dialog->ExecuteLD(R_ASP_SETTING_VIEW_DIALOG);

    return ret;
	}


// -----------------------------------------------------------------------------
// CAspSettingViewDialog::NewL
//
// -----------------------------------------------------------------------------
CAspSettingViewDialog* CAspSettingViewDialog::NewL(const TAspParam& aParam)
    {
    FLOG( _L("CAspSettingViewDialog::NewL START") );

    CAspSettingViewDialog* self = new ( ELeave )CAspSettingViewDialog (aParam);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);

    FLOG( _L("CAspSettingViewDialog::NewL END") );
    return self;
    }


// -----------------------------------------------------------------------------
// CAspSettingViewDialog::CAspSettingViewDialog
// 
// -----------------------------------------------------------------------------
//
CAspSettingViewDialog::CAspSettingViewDialog(const TAspParam& aParam)
	{
    iSyncSession = aParam.iSyncSession;
    iApplicationId = aParam.iApplicationId;
    iProfile = aParam.iProfile;
    iContentList = aParam.iContentList;
    iProfileList = aParam.iProfileList;
    iEditMode = aParam.iMode;
	iDataProviderId =aParam.iDataProviderId;
	iSchedule = aParam.iSchedule;
    
       
	__ASSERT_ALWAYS(iProfile, TUtil::Panic(KErrGeneral));
	__ASSERT_ALWAYS(iContentList, TUtil::Panic(KErrGeneral));
	__ASSERT_ALWAYS(iSyncSession, TUtil::Panic(KErrGeneral));
    }


// -----------------------------------------------------------------------------
// CAspSettingViewDialog::ConstructL
//
// -----------------------------------------------------------------------------
//
void CAspSettingViewDialog::ConstructL()
    {
    FLOG( _L("CAspSettingViewDialog::ConstructL START") );
	
	// contruct menu for our dialog
	CAknDialog::ConstructL(R_ASP_SETTING_VIEW_DIALOG_MENU);

	iSettingList = new (ELeave) CArrayPtrFlat<CAspListItemData>(1);

	TAspParam param(KErrNotFound, iSyncSession);
		
	// get previous title so it can be restored
	iStatusPaneHandler = CStatusPaneHandler::NewL(iAvkonAppUi);
	iStatusPaneHandler->StoreOriginalTitleL();
	
	FLOG( _L("CAspSettingViewDialog::ConstructL END") );
    } 


// ----------------------------------------------------------------------------
// Destructor
//
// ----------------------------------------------------------------------------
//
CAspSettingViewDialog::~CAspSettingViewDialog()
    {
    FLOG( _L("CAspSettingViewDialog::~CAspSettingViewDialog START") );

	if (iSettingList)
		{
		iSettingList->ResetAndDestroy();
	    delete iSettingList;
		}

	delete iStatusPaneHandler;
	
	
    if (iAvkonAppUi)
    	{
    	iAvkonAppUi->RemoveFromStack(this);
    	}

	FLOG( _L("~CAspSettingViewDialog::~~CAspSettingViewDialog END") );
    }


//------------------------------------------------------------------------------
// CAspSettingViewDialog::ActivateL
//
// Called by system when dialog is activated.
//------------------------------------------------------------------------------
//
void CAspSettingViewDialog::ActivateL()
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
void CAspSettingViewDialog::GetHelpContext(TCoeHelpContext& aContext) const
	{
	aContext.iMajor = KUidSmlSyncApp;
	aContext.iContext = KDS_HLP_SETTINGS;    
	}


// -----------------------------------------------------------------------------
// CAspSettingViewDialog::HandleListBoxEventL
// 
// -----------------------------------------------------------------------------
void CAspSettingViewDialog::HandleListBoxEventL(CEikListBox* /*aListBox*/,
                                            TListBoxEvent aEventType)
	{
    switch ( aEventType )
        {
        case EEventEnterKeyPressed:
        case EEventItemSingleClicked:
            HandleOKL();
            break;
        default:
           break;
        }
	}


// -----------------------------------------------------------------------------
// CAspSettingViewDialog::PreLayoutDynInitL
// 
// -----------------------------------------------------------------------------
//
void CAspSettingViewDialog::PreLayoutDynInitL()
    {
    iSettingListBox = (CAknSettingStyleListBox*) ControlOrNull (EAspSettingViewDialogList);
    
   	__ASSERT_ALWAYS(iSettingListBox, TUtil::Panic(KErrGeneral));
    
	iSettingListBox->SetListBoxObserver(this);
	iSettingListBox->CreateScrollBarFrameL(ETrue);
	iSettingListBox->ScrollBarFrame()->SetScrollBarVisibilityL(
	                 CEikScrollBarFrame::EOn, CEikScrollBarFrame::EAuto);
	
	SetIconsL();
	
	// create array of setting items (iSettingList)
	CreateSettingsListL();

	// add setting headers into listbox
	UpdateListBoxL(iSettingListBox, iSettingList);

	if (iDataProviderId)
		{
		iSettingListBox->SetCurrentItemIndex(GetIndexFromProvider(iDataProviderId));
		}
    iProfile->GetName(iBuf);
	iStatusPaneHandler->SetTitleL(iBuf);
	iStatusPaneHandler->SetNaviPaneTitleL(KNullDesC);
    }


// ----------------------------------------------------------------------------
// CAspSettingViewDialog::SetIconsL
//
// ----------------------------------------------------------------------------
//
void CAspSettingViewDialog::SetIconsL()
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
    CArrayPtr<CGulIcon>* arr =
    iSettingListBox->ItemDrawer()->FormattedCellData()->IconArray();
    
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
// CAspSettingViewDialog::IconL
// 
// -----------------------------------------------------------------------------
//
CGulIcon* CAspSettingViewDialog::IconL(TAknsItemID aId, const TDesC& aFileName, TInt aFileIndex, TInt aFileMaskIndex)
	{
    return TDialogUtil::CreateIconL(aId, aFileName, aFileIndex, aFileMaskIndex);
	}


//------------------------------------------------------------------------------
// CAspSettingViewDialog::ProcessCommandL
//
// Handle commands from menu.
//------------------------------------------------------------------------------
//
void CAspSettingViewDialog::ProcessCommandL(TInt aCommandId)
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
		case EAspMenuCmdOpen:
			{				
			HandleOKL();
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
// CAspSettingViewDialog::OkToExitL
//
//------------------------------------------------------------------------------
//
TBool CAspSettingViewDialog::OkToExitL(TInt aButtonId)
	{
	if (aButtonId == EEikBidCancel || aButtonId == EAknSoftkeyBack)
		{
		return ETrue; // close dialog
		}

	if (aButtonId == EAknSoftkeyOpen)  // MSK
        {
	    HandleOKL();
		return EFalse;  // leave dialog open
		}

	return CAknDialog::OkToExitL(aButtonId);
	}




// ----------------------------------------------------------------------------
// CAspSettingViewDialog::OfferKeyEventL
// 
// ----------------------------------------------------------------------------
//
TKeyResponse CAspSettingViewDialog::OfferKeyEventL(const TKeyEvent& aKeyEvent,TEventCode aType)
	{
	if (aType == EEventKey)
		{
		switch (aKeyEvent.iCode)
			{
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
// CAspSettingViewDialog::HandleResourceChange
// 
// ----------------------------------------------------------------------------
//
void CAspSettingViewDialog::HandleResourceChange(TInt aType)
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
// CAspSettingViewDialog::HandleOKL
//
//------------------------------------------------------------------------------
//
void CAspSettingViewDialog::HandleOKL()
	{
	CAspListItemData* item = GetItemForIndex(ListBox()->CurrentItemIndex());

	EditSettingItemL(*item);
	SetVisibility();
	UpdateListBoxL(iSettingListBox, iSettingList);
	
	}


// ----------------------------------------------------------------------------
// CAspSettingViewDialog::CreateSettingsListL
// 
// Function creates setting list array (iSettingsList).
// ----------------------------------------------------------------------------
//
void CAspSettingViewDialog::CreateSettingsListL()
	{
	
	AddProfileSettingItemL();
#ifdef RD_DSUI_TIMEDSYNC

	AddSchedulingSettingItemL();
	
#endif
	AddContentItemsL();
	SetVisibility();
	UpdateListBoxL(iSettingListBox, iSettingList);
	
	}

// ----------------------------------------------------------------------------
// CAspSettingViewDialog::AddProfileSettingItemL
// 
// 
// ----------------------------------------------------------------------------
//
void CAspSettingViewDialog::AddProfileSettingItemL()
	{
	
	CAspListItemData* item = CAspListItemData::NewLC();
	item->SetHeaderL(R_ASP_SETTING_FOLDER_PROFILE);
	item->iItemId = EAspProfileSetting;
	item->iItemType = CAspListItemData::ETypeProfileSetting;
	item->iNumberData = KErrNotFound;
	iSettingList->AppendL(item);
	CleanupStack::Pop(item);
		
	}

// ----------------------------------------------------------------------------
// CAspSettingViewDialog::AddSchedulingSettingItemL
// 
// ----------------------------------------------------------------------------
//
void CAspSettingViewDialog::AddSchedulingSettingItemL()
	{
    CAspListItemData* item = CAspListItemData::NewLC();
	item->SetHeaderL(R_ASP_SETTING_FOLDER_SCHEDULE);
	item->iItemId = EAspSchedulingSetting;
	item->iItemType = CAspListItemData::ETypeSchedulingSetting;
	item->iNumberData = KErrNotFound;
	iSettingList->AppendL(item);
	CleanupStack::Pop(item);
	}

	
// ----------------------------------------------------------------------------
// CAspSettingViewDialog::AddContentItemsL
// 
// ----------------------------------------------------------------------------
//
void CAspSettingViewDialog::AddContentItemsL()
	{
   	TInt count = iContentList->ProviderCount();

	for (TInt i=0; i<count; i++)
		{
		TAspProviderItem& providerItem = iContentList->ProviderItem(i);
		TInt appId = TUtil::AppIdFromProviderId(providerItem.iDataProviderId);
		if (iApplicationId != EApplicationIdSync && iApplicationId != appId)
			{
			continue;
			}
				
		HBufC* firstLine = CAspResHandler::GetContentSettingLC(
		                                   providerItem.iDataProviderId,
		                                   providerItem.iDisplayName);
		
	    CAspListItemData* item = CAspListItemData::NewLC();
    
    	item->SetHeaderL(firstLine->Des());
	    item->iItemId = EAspSyncContent;
	    item->iItemType = CAspListItemData::ETypeSyncContent;
	    item->iNumberData = providerItem.iDataProviderId;
	    iSettingList->AppendL(item);
	    CleanupStack::Pop(item);
	    
	    CleanupStack::PopAndDestroy(firstLine);
    	}
	}


// ----------------------------------------------------------------------------
// CAspSettingViewDialog::UpdateListBoxL
// 
// Add settings headers into listbox.
// ----------------------------------------------------------------------------
//
void CAspSettingViewDialog::UpdateListBoxL(CEikTextListBox* aListBox,
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

	ListBox()->HandleItemAdditionL();
	}


//------------------------------------------------------------------------------
// CAspSettingViewDialog::EditSettingItemL
//
// Calls setting editing functions. 
//------------------------------------------------------------------------------
//
void CAspSettingViewDialog::EditSettingItemL(CAspListItemData& aItem)
	{
	TBool ret = KErrNone;
	switch (aItem.iItemType)
		{
    	case CAspListItemData::ETypeProfileSetting:
    	ret = EditSettingItemProfileSettingL();
			break;

		case CAspListItemData::ETypeSchedulingSetting:
    	ret = EditSettingItemSchedulingSettingL();
			break;

		case CAspListItemData::ETypeSyncContent:
			ret = EditSettingItemSyncContentL(aItem);
			break;

		default:
		    break;
		}
	
		User::LeaveIfError(ret);
	
	
	}

// -----------------------------------------------------------------------------
// CAspContentListDialog::EditSettingItemProfileSettingL
//
// -----------------------------------------------------------------------------
//
TInt CAspSettingViewDialog::EditSettingItemProfileSettingL()
	{
 	TAspParam param(iApplicationId, iSyncSession);
    param.iProfile = iProfile;
    param.iProfileList = iProfileList;
    param.iMode = iEditMode;
    param.iContentList = iContentList;
    
    CAspProfileDialog::ShowDialogL(param);
    
    //Update profile name in navi pane ,to reflect any name change during profile editing
    iProfile->GetName(iBuf);
	iStatusPaneHandler->SetTitleL(iBuf);
	iStatusPaneHandler->SetNaviPaneTitleL(KNullDesC);
     
    return KErrNone;
	}
	
// -----------------------------------------------------------------------------
// CAspContentListDialog::EditSettingItemSchedulingSettingL
//
// -----------------------------------------------------------------------------
//
TInt CAspSettingViewDialog::EditSettingItemSchedulingSettingL()
	{
 	TAspParam param(iApplicationId, iSyncSession);
    param.iProfileList = iProfileList;
    param.iProfile = iProfile;
	param.iContentList = iContentList;
	param.iSchedule = iSchedule;

	TBool settingEnforcement = TUtil::SettingEnforcementState();
	
	CAspSchedule* schedule = CAspSchedule::NewLC();
	if (schedule->IsAutoSyncEnabled() && !settingEnforcement)
		{
		TInt profileId = schedule->ProfileId();
		if(profileId != iProfile->ProfileId() && profileId != KErrNotFound)
			{
			if(!TDialogUtil::ShowConfirmationQueryL(R_ASP_QUERY_AUTO_SYNC_ON))
				{
				CleanupStack::PopAndDestroy(schedule);
				return KErrNone;
				}
			}
		}
    CleanupStack::PopAndDestroy(schedule);

    CAspScheduleDialog::ShowDialogL(param);

    return KErrNone;
	}

// -----------------------------------------------------------------------------
// CAspContentListDialog::EditSettingItemSyncContentL
//
// -----------------------------------------------------------------------------
//
TInt CAspSettingViewDialog::EditSettingItemSyncContentL(CAspListItemData& aItem)
	{

	TAspParam param(iApplicationId, iSyncSession);
    param.iProfile = iProfile;
    param.iDataProviderId = aItem.iNumberData;
    param.iSyncTaskId = KErrNotFound;
    param.iContentList = iContentList;

    CAspContentDialog::ShowDialogL(param);

	    
    return KErrNone;
	}

   
//-----------------------------------------------------------------------------
// CAspSettingViewDialog::GetItemForIndex
// 
// Find item in list position aIndex.
//-----------------------------------------------------------------------------
//
CAspListItemData* CAspSettingViewDialog::GetItemForIndex(TInt aIndex)
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
// CAspSettingViewDialog::SetVisibility
// 
// -----------------------------------------------------------------------------
//
void CAspSettingViewDialog::SetVisibility()
	{
	
	if (iProfile->BearerType() == EAspBearerInternet)
		{
		Item(EAspSchedulingSetting)->iHidden = EVisibilityNormal;
		}
	else
		{
		Item(EAspSchedulingSetting)->iHidden = EVisibilityHidden;
		}
	}
//-----------------------------------------------------------------------------
// CAspSettingViewDialog::GetIndexFromProvider
// 
// Find index for a provider
//-----------------------------------------------------------------------------
//
TInt CAspSettingViewDialog::GetIndexFromProvider(TInt aDataProvider)
	{
	
	TInt count = iSettingList->Count();
	TInt index;
	for (index=0; index<count; index++)
		{
		CAspListItemData* temp = (*iSettingList)[index];
		if (temp->iNumberData == aDataProvider)
			{
			break;
			}
		}
	return index;
	}
	

//-----------------------------------------------------------------------------
// CAspSettingViewDialog::Item
// 
// Find item with aItemId.
//-----------------------------------------------------------------------------
//
CAspListItemData* CAspSettingViewDialog::Item(TInt aItemId)
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
// CAspSettingViewDialog::ListBox
// 
// -----------------------------------------------------------------------------
//
CAknSettingStyleListBox* CAspSettingViewDialog::ListBox()
	{
	return iSettingListBox;
	}


//  End of File  
