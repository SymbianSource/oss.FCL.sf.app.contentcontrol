/*
* Copyright (c) 2002-2005 Nokia Corporation and/or its subsidiary(-ies). 
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
#include "AspContentDialog.h"
#include "AspUtil.h"
#include "AspDefines.h"
#include "AspDialogUtil.h"
#include "AspResHandler.h"
#include "AspDebug.h"
#include "AspSyncUtil.rh"
#include <csxhelp/ds.hlp.hrh>

#include <aspsyncutil.mbg>  // for bitmap enumerations
#include <AknIconArray.h>   // for GulArray
#include <featmgr.h>   // FeatureManager
#include "AspSchedule.h"

const TInt KMSKControlId( CEikButtonGroupContainer::EMiddleSoftkeyPosition );

// -----------------------------------------------------------------------------
// CAspContentDialog::ShowDialogL
// 
// -----------------------------------------------------------------------------
TBool CAspContentDialog::ShowDialogL(TAspParam& aParam)
	{
	CAspContentDialog* dialog = CAspContentDialog::NewL(aParam);

	return dialog->ExecuteLD(R_ASP_CONTENT_DIALOG);
	}


// -----------------------------------------------------------------------------
// CAspContentDialog::NewL
//
// -----------------------------------------------------------------------------
CAspContentDialog* CAspContentDialog::NewL(TAspParam& aParam)
    {
    FLOG( _L("CAspContentDialog::NewL START") );
    
    CAspContentDialog* self = new (ELeave) CAspContentDialog(aParam);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);

    FLOG( _L("CAspContentDialog::NewL END") );
    return(self);
    }


//------------------------------------------------------------------------------
// CAspContentDialog::CAspContentDialog
//
//------------------------------------------------------------------------------
//
CAspContentDialog::CAspContentDialog(TAspParam& aParam)
	{
	iApplicationId = aParam.iApplicationId;
	iProfile = aParam.iProfile;
	iSyncSession = aParam.iSyncSession;
	iContentList = aParam.iContentList;
	iDialogParam = &aParam;
	iDataProviderId = aParam.iDataProviderId;
	
	iDataStoreChanged = EFalse;
	iMskEmpty = EFalse;
	
    __ASSERT_ALWAYS(iDataProviderId != KErrNotFound, TUtil::Panic(KErrGeneral));	
	__ASSERT_ALWAYS(iProfile, TUtil::Panic(KErrGeneral));
	__ASSERT_ALWAYS(iContentList, TUtil::Panic(KErrGeneral));
	__ASSERT_ALWAYS(iSyncSession, TUtil::Panic(KErrGeneral));
	}


//------------------------------------------------------------------------------
// Destructor
//
//------------------------------------------------------------------------------
//
CAspContentDialog::~CAspContentDialog()
	{
    FLOG( _L("CAspContentDialog::~CAspContentDialog START") );
	    
    if (iSettingList)
    	{
    	iSettingList->ResetAndDestroy();
	    delete iSettingList;
    	}

	delete iStatusPaneHandler;
	delete iLocalDataStores;
	
	if (iAvkonAppUi)
		{
		iAvkonAppUi->RemoveFromStack(this);
		}
		
    FLOG( _L("CAspContentDialog::~CAspContentDialog END") );
	}


// -----------------------------------------------------------------------------
// CAspContentDialog::ConstructL
//
// -----------------------------------------------------------------------------
//
void CAspContentDialog::ConstructL()
    {
    FLOG( _L("CAspContentDialog::ConstructL START") );
    
	CAknDialog::ConstructL(R_ASP_CONTENT_DIALOG_MENU);
	
	iSettingEnforcement = TUtil::SettingEnforcementState();
	
    iSettingList = new (ELeave) CArrayPtrFlat<CAspListItemData>(KDefaultArraySize);
	
	iLocalDataStores = new (ELeave) CDesCArrayFlat(KDefaultArraySize);
	iContentList->GetLocalDatabaseList(iDataProviderId, iLocalDataStores);
	
	TInt count = iLocalDataStores->Count();
	if (count == 0)
		{
		if (iDataProviderId == KUidNSmlAdapterEMail.iUid)
			{
			TDialogUtil::ShowErrorNoteL(R_ASP_NO_MAILBOXES);
			}
		else
			{
			TDialogUtil::ShowErrorNoteL(R_ASP_LOG_ERR_LOCALDATABASE);
			User::Leave(KErrNotFound);
			}
		}
		
	InitTaskDataL();

	// get previous title so it can be restored
	iStatusPaneHandler = CStatusPaneHandler::NewL(iAvkonAppUi);
	iStatusPaneHandler->StoreOriginalTitleL();
	
	// store current navi pane
    iStatusPaneHandler->StoreNavipaneL();

	FLOG( _L("CAspContentDialog::ConstructL END") );
    } 


//------------------------------------------------------------------------------
// CAspContentDialog::ActivateL (from CCoeControl)
//
// Called by system when dialog is activated
//------------------------------------------------------------------------------
//
void CAspContentDialog::ActivateL()
	{
    CAknDialog::ActivateL();

	// this cannot be in ConstructL which is executed before dialog is launched
	iAvkonAppUi->AddToStackL(this);
    }

//------------------------------------------------------------------------------
// CAspContentDialog::CheckContentSettingL
//
// Check if the MSK label is correct for the context
//------------------------------------------------------------------------------

void CAspContentDialog::CheckContentSettingL(const TKeyEvent& aKeyEvent)
	{
	
	TInt curIndex = iListBox->CurrentItemIndex();
	TInt count = iSettingList->Count();
	CAspListItemData* lastItem = (*iSettingList)[--count];
	
	switch (aKeyEvent.iCode)
		{
		case EKeyUpArrow:
			{
				if (curIndex)
				{
					curIndex--;
				}
				else
				{
				    curIndex = lastItem->iIndex;
				}
			break;
			}
		case EKeyDownArrow:
			{
				if (curIndex == lastItem->iIndex)
				{
				curIndex = 0;
				}
				else
				{
				curIndex++;
				}
			break;
			}
		}	
	CAspListItemData* curItem = GetItemForIndex(curIndex);
	if(curItem->iHidden == EVisibilityReadOnly)
	{
		UpdateMiddleSoftKeyLabelL(EAknSoftkeyOpen,R_TEXT_SOFTKEY_EMPTY);
	   	iMskEmpty = ETrue;
	   	return;	
	}
	if(iMskEmpty)
	{
		UpdateMiddleSoftKeyLabelL(EAknSoftkeyEmpty,R_QTN_MSK_CHANGE);
	   	iMskEmpty = EFalse;
	}

	}
	
//------------------------------------------------------------------------------
// CAspContentDialog::UpdateMiddleSoftKeyLabelL 
//
// Update the MSK 
//------------------------------------------------------------------------------
	void CAspContentDialog::UpdateMiddleSoftKeyLabelL(TInt aCommandId,TInt aResourceId)
	{
		ButtonGroupContainer().RemoveCommandFromStack(KMSKControlId,aCommandId );
		HBufC* middleSKText = StringLoader::LoadLC( aResourceId );

		ButtonGroupContainer().AddCommandToStackL(
		KMSKControlId,
		aCommandId,
		*middleSKText );
		CleanupStack::PopAndDestroy( middleSKText );
    }


//------------------------------------------------------------------------------
// CAspContentDialog::GetHelpContext
//
//------------------------------------------------------------------------------
//
void CAspContentDialog::GetHelpContext(TCoeHelpContext& aContext) const
	{
	aContext.iMajor = KUidSmlSyncApp;
    
	TInt dataProvider = DataProviderType();
	if (dataProvider == EApplicationIdContact)
		{
		aContext.iContext = KDS_HLP_SETTINGS_CNT;
		}
	else if (dataProvider == EApplicationIdCalendar)
		{
		aContext.iContext = KDS_HLP_SETTINGS_CAL;
		}
	else if (dataProvider == EApplicationIdNote)
		{
		aContext.iContext = KDS_HLP_SETTINGS_NOTES;
		}
	else if (dataProvider == EApplicationIdSms)
		{
		aContext.iContext = KDS_HLP_SETTINGS_SMS;
		}
	else
		{
		aContext.iContext = KDS_HLP_MAIN_VIEW;  // unknown application
		}
	}


//------------------------------------------------------------------------------
// CAspContentDialog::ProcessCommandL
//
// Handle menu commands.
//------------------------------------------------------------------------------
//
void CAspContentDialog::ProcessCommandL( TInt aCommandId )
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
// CAspContentDialog::OfferKeyEventL
//
// Handle key events.
//------------------------------------------------------------------------------
//
TKeyResponse CAspContentDialog::OfferKeyEventL(const TKeyEvent& aKeyEvent,TEventCode aType)
	{
	if (aType == EEventKey)
		{
		switch (aKeyEvent.iCode)
			{
			case EKeyEscape:  // framework calls this when dialog must shut down
				{
				return CAknDialog::OfferKeyEventL(aKeyEvent, aType);
        		}

			case EKeyEnter:
			case EKeyOK: // OK button
				{
				HandleOKL();
				return EKeyWasConsumed;
				}
			case EKeyUpArrow:
			case EKeyDownArrow:
			    {
			    	if(!iSettingEnforcement)
				    {
				    TRAPD(err ,CheckContentSettingL(aKeyEvent));
		         	User::LeaveIfError(err);
				    }
			    break;
			    }
			}
		}

	return CAknDialog::OfferKeyEventL(aKeyEvent, aType);
	}


//------------------------------------------------------------------------------
// CAspContentDialog::OkToExitL
//
//------------------------------------------------------------------------------
//
TBool CAspContentDialog::OkToExitL(TInt aButtonId)
	{
	if (aButtonId == EEikBidCancel)
		{
		// save silently and quit application
		TRAP_IGNORE(SaveSettingsL());
		return ETrue;
		}

	if (aButtonId == EAknSoftkeyBack)
        {
		TInt index = CheckMandatoryFields();
		if (index != KErrNotFound)
			{
			if (!TDialogUtil::ShowConfirmationQueryL(R_ASP_EXIT_ANYWAY))
				{
				iListBox->SetCurrentItemIndexAndDraw(index);
				return EFalse; // leave dialog open
				}
			}
		
		TRAP_IGNORE(SaveSettingsL());
		
		return ETrue;
		}
		
	if (aButtonId == EAknSoftkeyOpen||aButtonId == EAknSoftkeyEmpty )  // MSK
        {
        HandleOKL();
		return EFalse;  // leave dialog open
		}
		

	return CAknDialog::OkToExitL(aButtonId);
	}


//------------------------------------------------------------------------------
// CAspContentDialog::PreLayoutDynInitL
//
// Called by system before the dialog is shown.
//------------------------------------------------------------------------------
//
void CAspContentDialog::PreLayoutDynInitL()
	{
	iListBox = (CAknSettingStyleListBox*)Control(EAspContentDialog);
	
   	__ASSERT_ALWAYS(iListBox, TUtil::Panic(KErrGeneral));
	
	iListBox->SetListBoxObserver(this);
	iListBox->CreateScrollBarFrameL(ETrue);
	iListBox->ScrollBarFrame()->SetScrollBarVisibilityL(
	                   CEikScrollBarFrame::EOn, CEikScrollBarFrame::EAuto);
	                   
   	SetIconsL();	                   
	
    CreateSettingsListL();
    UpdateListBoxL(iListBox, iSettingList);
	TInt curIndex = iListBox->CurrentItemIndex();
		
	CAspListItemData* curItem = GetItemForIndex(curIndex);
	TBool isPCSuite = CAspProfile::IsPCSuiteProfile(iProfile);
	
	if(iSettingEnforcement || curItem->iHidden == EVisibilityReadOnly)
	{
	UpdateMiddleSoftKeyLabelL(EAknSoftkeyOpen,R_TEXT_SOFTKEY_EMPTY);	
   		
	}
	
	//Set title pane text
	TInt index = iContentList->FindProviderIndex(iDataProviderId);
	if (index != KErrNotFound)
		{
		TAspProviderItem& item = iContentList->ProviderItem(index);
    	HBufC* hBuf = CAspResHandler::GetContentTitleLC(iDataProviderId, 
    	                                                item.iDisplayName);
	    iStatusPaneHandler->SetTitleL(hBuf->Des());
	    CleanupStack::PopAndDestroy(hBuf);
		}
	}


// ----------------------------------------------------------------------------
// CAspContentDialog::SetIconsL
//
// ----------------------------------------------------------------------------
//
void CAspContentDialog::SetIconsL()
    {
    if (!iListBox)
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
    CArrayPtr<CGulIcon>* arr = iListBox->ItemDrawer()->FormattedCellData()->IconArray();
    if (arr)
    	{
    	arr->ResetAndDestroy();
	    delete arr;
	    arr = NULL;
    	}

	iListBox->ItemDrawer()->FormattedCellData()->SetIconArrayL(icons);
	CleanupStack::Pop(icons);
    }


// -----------------------------------------------------------------------------
// CAspContentDialog::IconL
// 
// -----------------------------------------------------------------------------
//
CGulIcon* CAspContentDialog::IconL(TAknsItemID aId, const TDesC& aFileName,
                                   TInt aFileIndex, TInt aFileMaskIndex)
	{
    return TDialogUtil::CreateIconL(aId, aFileName, aFileIndex, aFileMaskIndex);
	}


//------------------------------------------------------------------------------
// CAspContentDialog::DynInitMenuPaneL
//
// Called by system before menu is shown.
//------------------------------------------------------------------------------
//
void CAspContentDialog::DynInitMenuPaneL( TInt aResourceID, CEikMenuPane* aMenuPane)
	{
    if (aResourceID != R_ASP_CONTENT_DIALOG_MENU_PANE)
		{
		return;
		}

	if (iListBox->Model()->NumberOfItems() == 0)
		{
		TDialogUtil::DeleteMenuItem(aMenuPane, EAspMenuCmdChange);
		}
		
	CAspListItemData* item = GetItemForIndex(iListBox->CurrentItemIndex());

	if (item->iHidden == EVisibilityReadOnly)
	    {
	    TDialogUtil::DeleteMenuItem(aMenuPane, EAspMenuCmdChange);
	    }
	    
	TInt provider = DataProviderType();

	if (provider == KErrNotFound)
		{
		TDialogUtil::DeleteMenuItem(aMenuPane, EAknCmdHelp);
		}
	
    if (!FeatureManager::FeatureSupported(KFeatureIdHelp))
		{
		TDialogUtil::DeleteMenuItem(aMenuPane, EAknCmdHelp);
		}

	}


//------------------------------------------------------------------------------
// CAspContentDialog::HandleListBoxEventL
//
// Handle listbox events.
//------------------------------------------------------------------------------
//
void CAspContentDialog::HandleListBoxEventL(CEikListBox* /*aListBox*/, 
                                            TListBoxEvent aEventType)
	{
    switch ( aEventType )
        {
        case EEventItemSingleClicked:
            HandleOKL();
            break;
        default:
           break;
        }
	}


//-----------------------------------------------------------------------------
// CAspContentDialog::HandleOKL
//
// Handle content parameter editing.
//-----------------------------------------------------------------------------
//
void CAspContentDialog::HandleOKL()
	{
	CAspListItemData* item = GetItemForIndex(iListBox->CurrentItemIndex());
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
		UpdateListBoxL(iListBox, iSettingList);
		}
	}


// ----------------------------------------------------------------------------
// CAspContentDialog::HandleResourceChange
// 
// ----------------------------------------------------------------------------
//
void CAspContentDialog::HandleResourceChange(TInt aType)
    {   
    if (aType == KEikDynamicLayoutVariantSwitch) //Handle change in layout orientation
        {
        TRect mainPaneRect;
        AknLayoutUtils::LayoutMetricsRect(AknLayoutUtils::EMainPane, mainPaneRect);
        SetRect(mainPaneRect);
        iListBox->SetSize(mainPaneRect.Size());
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
// CAspContentDialog::InitTaskDataL
//
//------------------------------------------------------------------------------
//
void CAspContentDialog::InitTaskDataL()
	{
	iContentList->ReadTaskL(iDataProviderId, iLocalDatabase, iRemoteDatabase,
		                    iTaskEnabled, iSyncDirection);
		                        
   	if (iSettingEnforcement)
		{
		return;
		}

    // check that local database exists
    if (iLocalDatabase.Length() > 0)
    	{
    	TInt pos = 0;
    	if (iLocalDataStores->Find(iLocalDatabase, pos) != 0)
    		{
    		iLocalDatabase = KNullDesC; // local database not found
    		iDataStoreChanged = ETrue;
    		}
    	}

    if (iLocalDatabase.Length() == 0)
    	{
    	if (iDataProviderId == KUidNSmlAdapterEMail.iUid)
    		{
    		iContentList->UpdateDataProviderL(iDataProviderId);
    		}
    		
	    TInt index = iContentList->FindProviderIndex(iDataProviderId);
	    if (index != KErrNotFound)
		    {
		    TAspProviderItem& item = iContentList->ProviderItem(index);
     		if (item.iDefaultDataStore.Length() > 0)
			    {
			    iLocalDatabase = item.iDefaultDataStore;
			    iDataStoreChanged = ETrue;
			    }
		    }
    	}
    }


// ----------------------------------------------------------------------------
// CAspContentDialog::UpdateListBoxL
// 
// Add settings headers into listbox.
// ----------------------------------------------------------------------------
//
void CAspContentDialog::UpdateListBoxL(CEikTextListBox* aListBox,
                                       CAspSettingList* aItemList)
	{
	CDesCArray* arr = (CDesCArray*)aListBox->Model()->ItemTextArray();
	arr->Reset();

	TInt count = aItemList->Count();
	for (TInt index=0; index<count; index++ )
		{
	   	TBool convert = ETrue;
	   	TBool isPCSuite=CAspProfile::IsPCSuiteProfile(iProfile);
	   	TBool readOnly = iProfile->DeleteAllowed();

		CAspListItemData* item = (*aItemList)[index];
		TInt id=item->iItemId;
		if(iSettingEnforcement)
		{
			if(id==EAspLocalDatabase ||id==EAspRemoteDatabase )
			convert=EFalse;
		}
		else
		{
 			   if(isPCSuite|| !readOnly)
				{
					if(id==EAspRemoteDatabase)
					convert=EFalse;
				}
	
		}
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

	aListBox->HandleItemAdditionL();
	}


// ----------------------------------------------------------------------------
// CAspContentDialog::CreateSettingsListL
// 
// ----------------------------------------------------------------------------
//
void CAspContentDialog::CreateSettingsListL()
	{
	AddItemL(EAspLocalDatabase, R_ASP_SETT_LOCAL_DATABASE);
	AddRemoteDatabaseL(iDataProviderId);

	// write setting data into each CAspListItemData
	TInt count = iSettingList->Count();
	for (TInt i=0; i<count; i++)
		{
		InitSettingItemL((*iSettingList)[i]);
		}

    SetVisibility();
	}


// ----------------------------------------------------------------------------
// CAspContentDialog::AddItemL
// 
// ----------------------------------------------------------------------------
//
void CAspContentDialog::AddItemL(TInt aItemId, TInt aResourceId)
	{
    CAspListItemData* item = CAspListItemData::NewLC();
	item->SetHeaderL(aResourceId);
	item->iItemId = aItemId;
	iSettingList->AppendL(item);
	CleanupStack::Pop(item);
	}


// ----------------------------------------------------------------------------
// CAspContentDialog::AddRemoteDatabaseL
// 
// ----------------------------------------------------------------------------
//
void CAspContentDialog::AddRemoteDatabaseL(TInt aDataProviderId)
	{
    CAspListItemData* item = CAspListItemData::NewLC();
    TInt resId = CAspResHandler::RemoteDatabaseHeaderId(aDataProviderId);
	item->SetHeaderL(resId);
	item->iItemId = EAspRemoteDatabase;
	iSettingList->AppendL(item);
	CleanupStack::Pop(item);
	}


// -----------------------------------------------------------------------------
// CAspContentDialog::SetVisibility
// 
// -----------------------------------------------------------------------------
//
void CAspContentDialog::SetVisibility()
	{
    TBool isPCSuite = CAspProfile::IsPCSuiteProfile(iProfile);
    TBool readOnly = iProfile->DeleteAllowed();
    	
	Item(EAspLocalDatabase)->iHidden = EVisibilityNormal;
	if (iLocalDataStores->Count() < 2)
		{
		if (iDialogParam->iDataProviderId != KUidNSmlAdapterEMail.iUid)
			{
			Item(EAspLocalDatabase)->iHidden = EVisibilityHidden;
			}
		}
		
	if (isPCSuite)
		{
		if (Item(EAspLocalDatabase)->iHidden == EVisibilityNormal)
			{
			if (iDialogParam->iDataProviderId != KUidNSmlAdapterEMail.iUid)
				{
				Item(EAspLocalDatabase)->iHidden = EVisibilityReadOnly;
				}
			}
		
		TInt provider = DataProviderType();
	    if (provider != KErrNotFound)
		    {
		    Item(EAspRemoteDatabase)->iHidden = EVisibilityReadOnly;    
		    }
		}

	if(!readOnly)
		{
		TInt provider = DataProviderType();
	    if (provider != KErrNotFound)
		    {
		    Item(EAspRemoteDatabase)->iHidden = EVisibilityReadOnly;    
		    }
		}
		
    if (iSettingEnforcement)
    	{
    	SetAllReadOnly(); // profile database is locked
    	}
	}


// -----------------------------------------------------------------------------
// CAspContentDialog::SetAllReadOnly
// 
// -----------------------------------------------------------------------------
//
void CAspContentDialog::SetAllReadOnly()
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
// CAspContentDialog::InitSettingItemL
// 
// Constructs CAspListItemData for one setting.
// -----------------------------------------------------------------------------
//
void CAspContentDialog::InitSettingItemL(CAspListItemData* aItem)
	{
	iBuf = KNullDesC;  // reset common buffer
	
	switch (aItem->iItemId)
		{
		case EAspLocalDatabase:
			{
    		aItem->SetValueL(iLocalDatabase);
    		aItem->SetDisplayValueL(R_ASP_SETTING_VALUE_NONE);
    		
    		aItem->iMaxLength = KAspMaxLocalNameLength;
    		aItem->iLatinInput = ETrue;
    		aItem->iItemType = CAspListItemData::ETypeLocalDatabase;
			break;
			}

		case EAspRemoteDatabase:
			{
			aItem->SetValueL(iRemoteDatabase);
			aItem->SetDisplayValueL(R_ASP_MUST_BE_DEFINED);
			
			aItem->iMaxLength = KAspMaxRemoteNameLength;
			aItem->iLatinInput = ETrue;
			aItem->iMandatory = ETrue;
			aItem->iItemType = CAspListItemData::ETypeText;
			break;
			}

        default:
			break;
		}
	}


//------------------------------------------------------------------------------
// CAspContentDialog::SaveSettingsL
//
// Save content settings.
//------------------------------------------------------------------------------
//
void CAspContentDialog::SaveSettingsL()
	{
	if (iSettingEnforcement)
		{
		return;
		}
    	
	TBool localDatabaseChanged = EFalse;
	TBool remoteDatabaseChanged = EFalse;

	TPtrC ptr1 = Item(EAspLocalDatabase)->Value();
	if (ptr1.Compare(iLocalDatabase) != 0)
		{
		localDatabaseChanged = ETrue;
		}
		
	TPtrC ptr2 = Item(EAspRemoteDatabase)->Value();
	if (ptr2.Compare(iRemoteDatabase) != 0)
		{
		remoteDatabaseChanged = ETrue;
		}
	
	if (remoteDatabaseChanged || localDatabaseChanged || iDataStoreChanged)
		{
		iLocalDatabase = Item(EAspLocalDatabase)->Value();
	    iRemoteDatabase = Item(EAspRemoteDatabase)->Value();
        
   	    iContentList->CreateTaskL(iDataProviderId, iLocalDatabase, iRemoteDatabase,
	                              iTaskEnabled, iSyncDirection);
		                          
        iContentList->InitAllTasksL(); // iContentList comes from main view		                          
		}
		
#ifdef RD_DSUI_TIMEDSYNC
	CAspSchedule* schedule = CAspSchedule::NewLC();
	TInt profileId = schedule->ProfileId();
	if (profileId == iProfile->ProfileId())
		{
		schedule->UpdateProfileSettingsL();
		}
	CleanupStack::PopAndDestroy(schedule);
#endif
	}


//------------------------------------------------------------------------------
// CAspContentDialog::CheckMandatoryFields
//
// Check that mandatory fields are filled correctly.
//------------------------------------------------------------------------------
//
TInt CAspContentDialog::CheckMandatoryFields()
	{
	if (iSettingEnforcement)
		{
		return KErrNotFound;
		}

    if (!iContentList->TaskEnabled(iDataProviderId))
    	{
   		return KErrNotFound; // not part of sync - no need to check mandatory
    	}
	
    TInt count = iSettingList->Count();
	for (TInt i=0; i<count; i++)
		{
		CAspListItemData* item = (*iSettingList)[i];
		if (item->iMandatory && item->IsEmpty() && (item->iHidden == EVisibilityNormal))
			{
			return item->iIndex;
			}
		}
		
	return KErrNotFound;
	}


//------------------------------------------------------------------------------
// CAspContentDialog::EditSettingItemL
//
// Calls setting editing functions. 
//------------------------------------------------------------------------------
//
TBool CAspContentDialog::EditSettingItemL(CAspListItemData& aItem)
	{
	TBool ret = EFalse;
	
	switch (aItem.iItemType)
		{
		case CAspListItemData::ETypeText:
			ret = EditSettingItemTextL(aItem);
		    break;

		case CAspListItemData::ETypeList:
			ret = EditSettingItemListL(aItem);
		    break;
		
		case CAspListItemData::ETypeLocalDatabase:
			ret = EditSettingItemLocalDatabaseL(aItem);
			break;

		default:
		    break;

		}

    return ret;
	}


//------------------------------------------------------------------------------
// CAspContentDialog::EditSettingItemListL
//
//------------------------------------------------------------------------------
//
TBool CAspContentDialog::EditSettingItemListL(CAspListItemData& aItem)
	{
	TInt curSelection = aItem.iNumberData;
	CDesCArray* arr = CAspResHandler::ReadDesArrayStaticLC(aItem.iResource);
	
	TBool ret = TDialogUtil::ShowListEditorL(arr, aItem.Header(), curSelection);
	if (ret)
		{
		aItem.iNumberData = curSelection; 
		aItem.SetValueL((*arr)[curSelection]);
		}

	CleanupStack::PopAndDestroy(arr);
	return ret;
	}


//------------------------------------------------------------------------------
// CAspContentDialog::EditSettingItemTextL
//
// Edit text setting item.
//------------------------------------------------------------------------------
//
TBool CAspContentDialog::EditSettingItemTextL(CAspListItemData& aItem)
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
// CAspContentDialog::EditSettingItemLocalDatabaseL
//
//------------------------------------------------------------------------------
//
TBool CAspContentDialog::EditSettingItemLocalDatabaseL(CAspListItemData& aItem)
	{
	if (iLocalDataStores->Count() < 1)
		{
		if (iDataProviderId == KUidNSmlAdapterEMail.iUid)
			{
			TDialogUtil::ShowErrorNoteL(R_ASP_NO_MAILBOXES);
			}
			
		return EFalse;
		}
	
	TInt curSelection = 0;
	TInt countDatabase = iLocalDataStores->Count();
	
	while (iLocalDataStores->Find(iLocalDatabase, curSelection) && (curSelection < countDatabase))
		{
		curSelection++;
		}
	if (TDialogUtil::ShowListEditorL(iLocalDataStores, aItem.Header(), curSelection))
		{
		aItem.SetValueL((*iLocalDataStores)[curSelection]);
		return ETrue;
		}

	return EFalse;
	}


//------------------------------------------------------------------------------
// CAspContentDialog::EditSettingItemYesNoL
//
// Change Yes/No value straight if ok key is pressed.
//------------------------------------------------------------------------------
//
TBool CAspContentDialog::EditSettingItemYesNoL(CAspListItemData& aItem)
	{
	CDesCArray* arr = CAspResHandler::ReadDesArrayStaticLC(aItem.iResource);
	
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


//-----------------------------------------------------------------------------
// CAspContentDialog::GetItemForIndex
// 
// Find item in list position aIndex.
//-----------------------------------------------------------------------------
//
CAspListItemData* CAspContentDialog::GetItemForIndex(TInt aIndex)
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
// CAspContentDialog::Item
// 
// Find item with aItemId (TAspConnectionSettingItem).
//-----------------------------------------------------------------------------
//
CAspListItemData* CAspContentDialog::Item(TInt aItemId)
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
// CAspContentDialog::DataProviderType
//
// -----------------------------------------------------------------------------
//
TInt CAspContentDialog::DataProviderType() const
    {
    if (iDialogParam->iDataProviderId == KUidNSmlAdapterContact.iUid)
    	{
    	return EApplicationIdContact;
    	}
    if (iDialogParam->iDataProviderId == KUidNSmlAdapterCalendar.iUid)
    	{
    	return EApplicationIdCalendar;
    	}
    if (iDialogParam->iDataProviderId == KUidNSmlAdapterEMail.iUid)
    	{
    	return EApplicationIdEmail;
    	}
    if (iDialogParam->iDataProviderId == KUidNSmlAdapterNote.iUid)
    	{
    	return EApplicationIdNote;
    	}
    if (iDialogParam->iDataProviderId == KUidNSmlAdapterSms.iUid)
    	{
    	return EApplicationIdSms;
    	}
	if (iDialogParam->iDataProviderId == KUidNSmlAdapterMMS.iUid)
    	{
    	return EApplicationIdMms;
    	}
    if (iDialogParam->iDataProviderId == KUidNSmlAdapterBookmarks.iUid)
    	{
    	return EApplicationIdBookmarks;
    	}

    return KErrNotFound;
    } 




// End of file

