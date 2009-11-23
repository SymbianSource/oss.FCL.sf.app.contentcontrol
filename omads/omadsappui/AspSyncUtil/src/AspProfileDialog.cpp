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

#include "AspProfileDialog.h"
#include "AspContentDialog.h"
#include "AspSyncUtil.rh"
#include "AspDebug.h"
#include <csxhelp/ds.hlp.hrh>

#include <aspsyncutil.mbg>  // for bitmap enumerations
#include <AknIconArray.h>   // for GulArray
#include <aknpasswordsettingpage.h>     // CAknAlphaPasswordSettingPage
#include <ConnectionUiUtilities.h>      // CConnectionUiUtilities
#include <featmgr.h>   // FeatureManager
#include <cmdefconnvalues.h>
#include <cmmanager.h>

#include "AspSchedule.h"

const TInt KMSKControlId( CEikButtonGroupContainer::EMiddleSoftkeyPosition );
// ============================ MEMBER FUNCTIONS ===============================



// -----------------------------------------------------------------------------
// CAspProfileDialog::ShowDialogL
// 
// -----------------------------------------------------------------------------
TBool CAspProfileDialog::ShowDialogL(const TAspParam& aParam)
	{
	CAspProfileDialog* dialog = CAspProfileDialog::NewL(aParam);

	TBool ret = dialog->ExecuteLD(R_ASP_PROFILE_DIALOG);

    return ret;
	}


// -----------------------------------------------------------------------------
// CAspProfileDialog::NewL
//
// -----------------------------------------------------------------------------
CAspProfileDialog* CAspProfileDialog::NewL(const TAspParam& aParam)
    {
    FLOG( _L("CAspProfileDialog::NewL START") );

    CAspProfileDialog* self = new ( ELeave ) CAspProfileDialog(aParam);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);

    FLOG( _L("CAspProfileDialog::NewL END") );
    return self;
    }


// -----------------------------------------------------------------------------
// CAspProfileDialog::CAspProfileDialog
// 
// -----------------------------------------------------------------------------
//
CAspProfileDialog::CAspProfileDialog(const TAspParam& aParam)
	{
    iSyncSession = aParam.iSyncSession;
    iApplicationId = aParam.iApplicationId;
    iProfile = aParam.iProfile;
    iContentList = aParam.iContentList;
    iProfileList = aParam.iProfileList;
    iEditMode = aParam.iMode;
    iMSKEmpty = EFalse;
       
	__ASSERT_ALWAYS(iProfile, TUtil::Panic(KErrGeneral));
	__ASSERT_ALWAYS(iContentList, TUtil::Panic(KErrGeneral));
	__ASSERT_ALWAYS(iSyncSession, TUtil::Panic(KErrGeneral));
    }


// -----------------------------------------------------------------------------
// CAspProfileDialog::ConstructL
//
// -----------------------------------------------------------------------------
//
void CAspProfileDialog::ConstructL()
    {
    FLOG( _L("CAspProfileDialog::ConstructL START") );
	
	// contruct menu for our dialog
	CAknDialog::ConstructL(R_ASP_PROFILE_DIALOG_MENU);

	iResHandler = CAspResHandler::NewL();

	iSettingList = new (ELeave) CArrayPtrFlat<CAspListItemData>(1);

    TAspParam param(KErrNotFound, iSyncSession);
	iBearerHandler = CAspBearerHandler::NewL(param);
	iApHandler = CAspAccessPointHandler::NewL(param);
	
	// get previous title so it can be restored
	iSettingEnforcement = TUtil::SettingEnforcementState();
	iStatusPaneHandler = CStatusPaneHandler::NewL(iAvkonAppUi);
	iStatusPaneHandler->StoreOriginalTitleL();
	
	FLOG( _L("CAspProfileDialog::ConstructL END") );
    } 


// ----------------------------------------------------------------------------
// Destructor
//
// ----------------------------------------------------------------------------
//
CAspProfileDialog::~CAspProfileDialog()
    {
    FLOG( _L("CAspProfileDialog::~CAspProfileDialog START") );

	delete iResHandler;
	
	if (iSettingList)
		{
		iSettingList->ResetAndDestroy();
	    delete iSettingList;
		}

	delete iStatusPaneHandler;
	delete iBearerHandler;
	delete iApHandler;
	
    if (iAvkonAppUi)
    	{
    	iAvkonAppUi->RemoveFromStack(this);
    	}

	FLOG( _L("CAspProfileDialog::~CAspProfileDialog END") );
    }


//------------------------------------------------------------------------------
// CAspProfileDialog::ActivateL
//
// Called by system when dialog is activated.
//------------------------------------------------------------------------------
//
void CAspProfileDialog::ActivateL()
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
void CAspProfileDialog::GetHelpContext(TCoeHelpContext& aContext) const
	{
	aContext.iMajor = KUidSmlSyncApp;
	aContext.iContext = KDS_HLP_SETTINGS;    
	}


// -----------------------------------------------------------------------------
// CAspProfileDialog::HandleListBoxEventL
// 
// -----------------------------------------------------------------------------
void CAspProfileDialog::HandleListBoxEventL(CEikListBox* /*aListBox*/,
                                            TListBoxEvent aEventType)
	{
	if( AknLayoutUtils::PenEnabled() )  
	  {
	   switch ( aEventType )
        {
         case EEventItemDoubleClicked:
              HandleOKL();
              break;
         case EEventEnterKeyPressed :
         case EEventItemClicked :
         case EEventItemDraggingActioned :
         case EEventPenDownOnItem :
        	  if(iEditMode != EDialogModeSettingEnforcement)
				    {
				    TRAPD(err ,CheckContentSettingL());//folder
		         	User::LeaveIfError(err);
				    }
              break;
         default:
              break;
        }
		
	  }
    
	}

// -----------------------------------------------------------------------------
// CAspProfileDialog::PreLayoutDynInitL
// 
// -----------------------------------------------------------------------------
//
void CAspProfileDialog::PreLayoutDynInitL()
    {
    iSettingListBox = (CAknSettingStyleListBox*) ControlOrNull (EAspProfileDialogList);
    
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
	
	if (iEditMode == EDialogModeEditMandatory)
		{
		TInt index = CheckMandatoryFields();
		if (index != KErrNotFound)
			{
			ListBox()->SetCurrentItemIndexAndDraw(index);
			}
		}
	
	TInt curIndex = ListBox()->CurrentItemIndex();
		
	CAspListItemData* curItem = GetItemForIndex(curIndex);
	TBool isPCSuite = CAspProfile::IsPCSuiteProfile(iProfile);
    	
   	//if((isPCSuite && curItem->iHidden == EVisibilityReadOnly)
   	if(curItem->iHidden == EVisibilityReadOnly
   		|| iEditMode == EDialogModeSettingEnforcement)
   	{
   	   	 SetEmptyMiddleSoftKeyLabelL();
   	}
   	
#ifdef RD_DSUI_TIMEDSYNC 
	iStatusPaneHandler->SetTitleL(R_ASP_TITLE_PROFILE_SETTINGS);
#else
    iProfile->GetName(iBuf);
    iStatusPaneHandler->SetTitleL(iBuf);
#endif
	iStatusPaneHandler->SetNaviPaneTitleL(KNullDesC);
    }


// ----------------------------------------------------------------------------
// CAspProfileDialog::SetMiddleSoftKeyLabelL
//
// ----------------------------------------------------------------------------
//
void CAspProfileDialog::SetEmptyMiddleSoftKeyLabelL()
    {
    ButtonGroupContainer().RemoveCommandFromStack(KMSKControlId,EAknSoftkeyOpen );
    HBufC* middleSKText = StringLoader::LoadLC( R_TEXT_SOFTKEY_EMPTY );

    ButtonGroupContainer().AddCommandToStackL(
        KMSKControlId,
        EAknSoftkeyEmpty,
        *middleSKText );
    iMSKEmpty= ETrue;
    CleanupStack::PopAndDestroy( middleSKText );
    }
// ----------------------------------------------------------------------------
// CAspProfileDialog::SetIconsL
//
// ----------------------------------------------------------------------------
//
void CAspProfileDialog::SetIconsL()
    {
    if (!iSettingListBox)
    	{
    	return;
    	}
 
 	TFileName bitmapName;
	CAspResHandler::GetBitmapFileName(bitmapName);
	CArrayPtr<CGulIcon>* icons = new (ELeave) CAknIconArray(KDefaultArraySize);
	CleanupStack::PushL(icons);
	
	// Create the lock icon as the last icon in the table
	CFbsBitmap* skinnedBitmap = NULL;
    CFbsBitmap* skinnedMask = NULL;
    
	// Make the icon and put it in the array
    CGulIcon* icon = CGulIcon::NewL();
    CleanupStack::PushL(icon);
    icon->SetBitmapsOwnedExternally(EFalse);
		
    MAknsSkinInstance* skin = AknsUtils::SkinInstance();
    // Create color icon
    AknsUtils::CreateColorIconL(skin,
							    KAknsIIDQgnIndiSettProtectedAdd, 
							    KAknsIIDQsnIconColors,
							    EAknsCIQsnIconColorsCG13,
							    skinnedBitmap, 
							    skinnedMask,
							    bitmapName,
							    EMbmAspsyncutilQgn_indi_sett_protected_add, 
							    EMbmAspsyncutilQgn_indi_sett_protected_add_mask,
							    KRgbBlack
							   );
	
    icon->SetMask(skinnedMask);     // ownership transferred
    icon->SetBitmap(skinnedBitmap); // ownership transferred
    
	icons->AppendL( icon );  

	CleanupStack::Pop(icon); // icon
    // Create the lock icon for highlight
    skinnedBitmap = NULL;
    skinnedMask = NULL;
    
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
// CAspProfileDialog::IconL
// 
// -----------------------------------------------------------------------------
//
CGulIcon* CAspProfileDialog::IconL(TAknsItemID aId, const TDesC& aFileName, TInt aFileIndex, TInt aFileMaskIndex)
	{
    return TDialogUtil::CreateIconL(aId, aFileName, aFileIndex, aFileMaskIndex);
	}


//------------------------------------------------------------------------------
// CAspProfileDialog::DynInitMenuPaneL
//
// Called by system before menu is shown.
//------------------------------------------------------------------------------
//
void CAspProfileDialog::DynInitMenuPaneL(TInt /*aResourceID*/, CEikMenuPane* aMenuPane)
	{
   
   	if (ListBox()->Model()->NumberOfItems() == 0) 
		{
		TDialogUtil::DeleteMenuItem(aMenuPane, EAspMenuCmdChange);
		return;
		}

	CAspListItemData* item = GetItemForIndex(ListBox()->CurrentItemIndex());
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
// CAspProfileDialog::ProcessCommandL
//
// Handle commands from menu.
//------------------------------------------------------------------------------
//
void CAspProfileDialog::ProcessCommandL(TInt aCommandId)
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
    		CAspListItemData* item = GetItemForIndex(ListBox()->CurrentItemIndex());

			if (item->iItemType == CAspListItemData::ETypeListYesNo)
				{
				// open editor for Yes/No setting
				if (EditSettingItemListL(*item))
					{
					SetVisibility();
					UpdateListBoxL(ListBox(), iSettingList);
					}
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
// CAspProfileDialog::OkToExitL
//
//------------------------------------------------------------------------------
//
TBool CAspProfileDialog::OkToExitL(TInt aButtonId)
	{
	if (aButtonId == EEikBidCancel)
		{
		// save silently and close dialog
		
		CheckSettingValues(aButtonId);

		TRAP_IGNORE(SaveSettingsL());
		return ETrue; // close dialog
		}

	if (aButtonId == EAknSoftkeyBack)
        {
        TBool ret = CheckSettingValues(aButtonId);
        if (!ret)
        	{
        	return EFalse; // leave dialog open
        	}
        	
		TRAP_IGNORE(SaveSettingsL());
		return ETrue; // close dialog
		}
		
	if (aButtonId == EAknSoftkeyOpen || aButtonId == EAknSoftkeyEmpty)  // MSK
        {
		CAspListItemData* item = GetItemForIndex(ListBox()->CurrentItemIndex());
		if (item->iHidden == EVisibilityReadOnly)
			{
    		if(iEditMode == EDialogModeSettingEnforcement)
    		{
    			TDialogUtil::ShowInformationNoteL(R_ASP_PROTECTED_SETTING);
    		}
     		else
     		{
			TDialogUtil::ShowInformationNoteL(R_ASP_NOTE_READ_ONLY);
     		}
			return EFalse;
			}

        HandleOKL();
		return EFalse;  // leave dialog open
		}
		
	if (aButtonId == EAknSoftkeyOptions)	
		{
		if (iCommandSetId == R_ASP_CBA_OPTIONS_BACK_OPEN)
			{
			UpdateMenuL(R_ASP_PROFILE_CONTEXT_MENU);
			}	
		else 
			{
			UpdateMenuL(R_ASP_PROFILE_DIALOG_MENU);	
			}
		return EFalse;
		}
	return CAknDialog::OkToExitL(aButtonId);
	}


//------------------------------------------------------------------------------
// CAspProfileDialog::CheckSettingValuesL
//
//------------------------------------------------------------------------------
//
TBool CAspProfileDialog::CheckSettingValuesL(TInt aButtonId)
	{
    if (iEditMode == EDialogModeReadOnly || 
        iEditMode == EDialogModeSettingEnforcement)
    	{
    	return ETrue;
    	}
	
	if (aButtonId == EEikBidCancel)
		{
		TInt index = CheckUniqueServerId();
		if (index != KErrNotFound)
			{
			Item(EAspServerId)->SetValueL(iOldServerId);
			}

		return ETrue;
		}

	TInt mandatoryIndex = CheckMandatoryFields();
	if (mandatoryIndex != KErrNotFound)
		{
		if (!TDialogUtil::ShowConfirmationQueryL(R_ASP_EXIT_ANYWAY))
			{
			ListBox()->SetCurrentItemIndexAndDraw(mandatoryIndex);
			return EFalse; // leave dialog open
			}
				
	    TInt serverIdIndex = CheckUniqueServerId();
	    if (serverIdIndex != KErrNotFound)
	    	{
	    	Item(EAspServerId)->SetValueL(iOldServerId);
	    	}
		}
	else
		{
		TInt serverIdIndex = CheckUniqueServerId();
	    if (serverIdIndex != KErrNotFound)
		    {
		    if (!TDialogUtil::ShowConfirmationQueryL(R_ASP_SERVER_ID_EXISTS))
			    {
			    ListBox()->SetCurrentItemIndexAndDraw(serverIdIndex);
			    return EFalse; // leave dialog open
			    }
		    else
			    {
			    Item(EAspServerId)->SetValueL(iOldServerId);
			    }
		    }
		}

    return ETrue;
	}


//------------------------------------------------------------------------------
// CAspProfileDialog::CheckSettingValues
//
//------------------------------------------------------------------------------
//
TBool CAspProfileDialog::CheckSettingValues(TInt aButtonId)
	{
	TBool ret = EFalse;
	
	TRAPD(err, ret = CheckSettingValuesL(aButtonId));
	if (err != KErrNone)
		{
		return ETrue;
		}

	return ret;
	}


// ----------------------------------------------------------------------------
// CAspProfileDialog::OfferKeyEventL
// 
// ----------------------------------------------------------------------------
//
TKeyResponse CAspProfileDialog::OfferKeyEventL(const TKeyEvent& aKeyEvent,TEventCode aType)
	{
	if (aType == EEventKey)
		{
		switch (aKeyEvent.iCode)
			{
			case EKeyEnter:
				{
					HandleOKL();
					return EKeyWasConsumed;
				}
			case EKeyEscape:  // framework calls this when dialog must shut down
				{
				return CAknDialog::OfferKeyEventL(aKeyEvent, aType);
        		}
			case EKeyUpArrow:
			case EKeyDownArrow:
			    {
			    	if(iEditMode != EDialogModeSettingEnforcement)
				    {
				    TRAPD(err ,CheckContentSettingL(aKeyEvent));//folder
		         	User::LeaveIfError(err);
				    }
			    break;
			    }
            default:
				{
			    break;
				}
			}
		}

	return CAknDialog::OfferKeyEventL( aKeyEvent, aType);
	}

// ---------------------------------------------------------------------------------------------
// CAspProfileDialog::CheckContentSetting
// modifies MSK and menu item depending on the item in the list box when touch events are handled
// ----------------------------------------------------------------------------------------------
//

void CAspProfileDialog::CheckContentSettingL()
{
	TInt curIndex = ListBox()->CurrentItemIndex();
	TInt count = iSettingList->Count();
	CAspListItemData* lastItem = (*iSettingList)[--count];
	

		
	CAspListItemData* curItem = GetItemForIndex(curIndex);
	TBool isPCSuite = CAspProfile::IsPCSuiteProfile(iProfile);
	if(isPCSuite && (curItem->iHidden == EVisibilityReadOnly))
	{
   		SetEmptyMiddleSoftKeyLabelL();
   		return;
	}
	if(curItem->iItemId != EAspSyncContent)
		{
		if(iCommandSetId != R_ASP_CBA_OPTIONS_BACK_CHANGE || iMSKEmpty)
			{
			UpdateCbaL(R_ASP_CBA_OPTIONS_BACK_CHANGE);
		
			}
		return;		
		}
	UpdateCbaL( R_ASP_CBA_OPTIONS_BACK_OPEN);

}

// ----------------------------------------------------------------------------
// CAspProfileDialog::CheckContentSetting
// modifies MSK depending on the item
// ----------------------------------------------------------------------------
//

void CAspProfileDialog::CheckContentSettingL(const TKeyEvent& aKeyEvent)
	{
	
	TInt curIndex = ListBox()->CurrentItemIndex();
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
   		SetEmptyMiddleSoftKeyLabelL();
   		return;
	}
	if(curItem->iItemId != EAspSyncContent)
		{
		if(iCommandSetId != R_ASP_CBA_OPTIONS_BACK_CHANGE || iMSKEmpty)
			{
			UpdateCbaL(R_ASP_CBA_OPTIONS_BACK_CHANGE);
		
			}
		return;		
		}
	UpdateCbaL( R_ASP_CBA_OPTIONS_BACK_OPEN);

	}

// ----------------------------------------------------------------------------
// CAspProfileDialog::HandleResourceChange
// 
// ----------------------------------------------------------------------------
//
void CAspProfileDialog::HandleResourceChange(TInt aType)
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
// CAspProfileDialog::CheckMandatoryFields
//
// Check that all mandatory fields are filled.
//------------------------------------------------------------------------------
//
TInt CAspProfileDialog::CheckMandatoryFields()
	{ 
	TInt count = iSettingList->Count();
	for (TInt i=0; i<count; i++)
		{
		CAspListItemData* item = (*iSettingList)[i];
		
		if (!CheckMandatoryContentFields(item))
			{
			return item->iIndex;
			}
			
		if ( item->iMandatory && item->IsEmpty() && (item->iHidden == EVisibilityNormal) )
			{
			return item->iIndex;
			}
		}

    return KErrNotFound;
	}


//------------------------------------------------------------------------------
// CAspProfileDialog::CheckUniqueServerId
//
//------------------------------------------------------------------------------
//
TInt CAspProfileDialog::CheckUniqueServerId()
	{
	TPtrC ptr = Item(EAspServerId)->Value();
	
	if (iOldServerId.Compare(ptr) == 0)
		{
		return KErrNotFound;  // server id has not changed
		}
	
	if (!iProfileList)
		{
		return KErrNotFound;
		}

	TInt profileId = iProfile->ProfileId();

	if (iProfileList->IsUniqueServerId(Item(EAspServerId)->Value(), profileId))
		{
		return KErrNotFound;
		}
		
	return Item(EAspServerId)->iIndex;
	}


//------------------------------------------------------------------------------
// CAspProfileDialog::CheckMandatoryContentFields
//
// Check that all mandatory fields are filled.
//------------------------------------------------------------------------------
//
TBool CAspProfileDialog::CheckMandatoryContentFields(CAspListItemData* aListItem)
	{
	if (aListItem->iItemType != CAspListItemData::ETypeSyncContent)
		{
		return ETrue;
		}
		
	TInt index = iContentList->FindTaskIndexForProvider(aListItem->iNumberData);
	if (index == KErrNotFound)
		{
		return ETrue;
		}
		
	TAspTaskItem& item = iContentList->TaskItem(index);
	if (item.iEnabled && !item.iRemoteDatabaseDefined)
		{
		return EFalse;
		}
		
    return ETrue;
	}


//------------------------------------------------------------------------------
// CAspProfileDialog::HandleOKL
//
//------------------------------------------------------------------------------
//
void CAspProfileDialog::HandleOKL()
	{
	CAspListItemData* item = GetItemForIndex(ListBox()->CurrentItemIndex());

	if (item->iHidden == EVisibilityReadOnly)
		{
			if(iEditMode == EDialogModeSettingEnforcement)
      		{
    			TDialogUtil::ShowInformationNoteL(R_ASP_PROTECTED_SETTING);
    		}
     		else
     		{
			TDialogUtil::ShowInformationNoteL(R_ASP_NOTE_READ_ONLY);
     		}
		return;
		}

	if (EditSettingItemL(*item))
		{
		SetVisibility();
		UpdateListBoxL(ListBox(), iSettingList);
		}
	}


// ----------------------------------------------------------------------------
// CAspProfileDialog::CreateSettingsListL
// 
// Function creates setting list array (iSettingsList).
// Note: enum TAspConnectionSettings must match with string array
// (R_ASP_CONNECTION_DIALOG_SETTING_ITEMS).
// ----------------------------------------------------------------------------
//
void CAspProfileDialog::CreateSettingsListL()
	{
	// read setting headers from resource
	CDesCArray* arr = iResHandler->ReadDesArrayLC(R_ASP_CONNECTION_DIALOG_SETTING_ITEMS);
	
	// add one CAspListItemData for each setting
	AddItemL(EAspProfileName, arr);
	AddItemL(EAspProtocolVersion, arr);
	AddItemL(EAspServerId, arr);

	AddItemL(EAspConnType, arr);
	AddItemL(EAspAccessPoint,R_ASP_NETWORK_DESTINATION);
	AddItemL(EAspHostAddress, arr);
	AddItemL(EAspPort, arr);
	AddItemL(EAspUsername, arr);
	AddItemL(EAspPassword, arr);
	
#ifdef __SYNCML_DS_ALERT
	AddItemL(EAspServerAlert, arr);
#endif

	AddItemL(EAspHttpsUsername, arr);
	AddItemL(EAspHttpsPassword, arr);
	
	AddItemL(EAspSyncDirection, arr);
	
#ifndef	RD_DSUI_TIMEDSYNC 
	AddContentItemsL();
#endif

	// write setting data into each CAspListItemData
	TInt count=iSettingList->Count();
	for (TInt i=0; i<count; i++)
		{
		InitSettingItemL((*iSettingList)[i]);
		}

	SetVisibility();  // find out what setting appear on UI

	CleanupStack::PopAndDestroy(arr);
	}


// ----------------------------------------------------------------------------
// CAspProfileDialog::AddItemL
// 
// ----------------------------------------------------------------------------
//
void CAspProfileDialog::AddItemL(TInt aItemId, CDesCArray* aHeaders)
	{
    CAspListItemData* item = CAspListItemData::NewLC();
	item->SetHeaderL((*aHeaders)[aItemId]);
	item->iItemId = aItemId;
	iSettingList->AppendL(item);
	CleanupStack::Pop(item);
	}


// ----------------------------------------------------------------------------
// CAspProfileDialog::AddItemL
// 
// ----------------------------------------------------------------------------
//
void CAspProfileDialog::AddItemL(TInt aItemId, TInt aResourceId)
	{
    CAspListItemData* item = CAspListItemData::NewLC();
	item->SetHeaderL(aResourceId);
	item->iItemId = aItemId;
	iSettingList->AppendL(item);
	CleanupStack::Pop(item);
	}


// ----------------------------------------------------------------------------
// CAspProfileDialog::AddContentItemsL
// 
// ----------------------------------------------------------------------------
//
void CAspProfileDialog::AddContentItemsL()
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
// CAspProfileDialog::UpdateListBoxL
// 
// Add settings headers into listbox.
// ----------------------------------------------------------------------------
//
void CAspProfileDialog::UpdateListBoxL(CEikTextListBox* aListBox,
                                       CAspSettingList* aItemList)
	{
	CDesCArray* arr = (CDesCArray*)aListBox->Model()->ItemTextArray();
	arr->Reset();

	TInt count = aItemList->Count();
	for (TInt i=0; i<count; i++ )
		{	
		CAspListItemData* item = (*aItemList)[i];
		TInt id = item->iItemId;
		
		TBool convert = ETrue;
	   TBool isPCSuite=CAspProfile::IsPCSuiteProfile(iProfile);
       TBool readOnly = iProfile->DeleteAllowed();
       if(isPCSuite && !readOnly||iSettingEnforcement)
        {
                if(id==EAspAccessPoint || id==EAspSyncDirection ||id==EAspConnType)
        	       convert = EFalse;//no number conversion
        }
       if(!isPCSuite &&!readOnly)
       {
                if(id==EAspAccessPoint ||id==EAspConnType)
        	       convert = EFalse;//no number conversion
       }	
		if (id == EAspServerId || id == EAspHostAddress ||
	       id == EAspPort || id == EAspUsername || id == EAspHttpsUsername||iSettingEnforcement)
			{
			convert = EFalse; // no number conversion
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

	ListBox()->HandleItemAdditionL();
	}


// -----------------------------------------------------------------------------
// CAspProfileDialog::SetVisibility
// 
// -----------------------------------------------------------------------------
//
void CAspProfileDialog::SetVisibility()
	{
    TBool isPCSuite = CAspProfile::IsPCSuiteProfile(iProfile);
    
    TBool readOnly = iProfile->DeleteAllowed();
	
	if (Item(EAspProtocolVersion)->iNumberData == EAspProtocol_1_1)
		{
		Item(EAspServerId)->iHidden = EVisibilityHidden;
		}
	else if (isPCSuite || !readOnly)
		{
		Item(EAspServerId)->iHidden = EVisibilityReadOnly;
		}
	else
		{
		Item(EAspServerId)->iHidden = EVisibilityNormal;
		}
	
	
	if (isPCSuite || !readOnly)
		{
		Item(EAspConnType)->iHidden = EVisibilityReadOnly;
		}
	else
		{
		Item(EAspConnType)->iHidden = EVisibilityNormal;
		}

	if (isPCSuite || !readOnly)
		{
		Item(EAspHostAddress)->iHidden = EVisibilityReadOnly;
		}
	else
		{
		Item(EAspHostAddress)->iHidden = EVisibilityNormal;
		}
		
	if(isPCSuite)
	{
		Item(EAspSyncDirection)->iHidden = EVisibilityReadOnly;
	}
	else
	{
		Item(EAspSyncDirection)->iHidden = EVisibilityNormal;
	}
    
		

	if (iBearerHandler->SupportedBearerCount() == 1)
		{
		Item(EAspConnType)->iHidden = EVisibilityReadOnly;
		}
	
    Item(EAspUsername)->iHidden = EVisibilityNormal;
	Item(EAspPassword)->iHidden = EVisibilityNormal;

	if (Item(EAspConnType)->iNumberData != EAspBearerInternet)
		{
		Item(EAspAccessPoint)->iHidden = EVisibilityHidden;
		Item(EAspPort)->iHidden = EVisibilityHidden;
		Item(EAspHttpsUsername)->iHidden = EVisibilityHidden;
		Item(EAspHttpsPassword)->iHidden = EVisibilityHidden;
    	}
	else
		{
		Item(EAspAccessPoint)->iHidden = EVisibilityNormal;
		Item(EAspPort)->iHidden = EVisibilityNormal;
		Item(EAspHttpsUsername)->iHidden = EVisibilityNormal;
		Item(EAspHttpsPassword)->iHidden = EVisibilityNormal;
		}
		
#ifdef __SYNCML_DS_ALERT
    Item(EAspServerAlert)->iHidden = EVisibilityNormal;
#endif
	
    if (iEditMode == EDialogModeReadOnly || 
        iEditMode == EDialogModeSettingEnforcement)
    	{
    	SetAllReadOnly();
    	}
	}


// -----------------------------------------------------------------------------
// CAspContentDialog::SetAllReadOnly
// 
// -----------------------------------------------------------------------------
//
void CAspProfileDialog::SetAllReadOnly()
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
// CAspProfileDialog::InitSettingItemL
// 
// Constructs CAspListItemData for one connection setting.
// -----------------------------------------------------------------------------
//
void CAspProfileDialog::InitSettingItemL(CAspListItemData* aItem)
	{
	__ASSERT_ALWAYS(aItem, TUtil::Panic(KErrGeneral));	
	
	iBuf = KNullDesC;  // reset common buffer
	
	switch (aItem->iItemId)
		{
		case EAspProfileName:
			{
			iProfile->GetName(iBuf);
			aItem->SetValueL(iBuf);
			aItem->SetDisplayValueL(R_ASP_MUST_BE_DEFINED);
			
			aItem->iMaxLength = KAspMaxProfileNameLength;
			aItem->iLatinInput = EFalse;
			aItem->iItemType = CAspListItemData::ETypeText;
			aItem->iMandatory = ETrue;
			break;
			}

		case EAspProtocolVersion:
			{
			CDesCArray* arr = iResHandler->ReadDesArrayLC(R_ASP_PROTOCOL_VERSION);
    
    		aItem->iNumberData = iProfile->ProtocolVersion();
			aItem->SetValueL((*arr)[aItem->iNumberData]);
				
			aItem->iResource = R_ASP_PROTOCOL_VERSION;
			aItem->iItemType = CAspListItemData::ETypeProtocolList;
			
			CleanupStack::PopAndDestroy(arr);
			break;
			}

		case EAspServerId:
			{
			iProfile->GetServerId(iBuf);
			aItem->SetValueL(iBuf);
			if(TDialogUtil::IsPCSuiteDesC(iBuf))
			{
				aItem->SetDisplayValueL(R_ASP_PC_SUITE_PROFILE);	
			}
			else
			{
			aItem->SetDisplayValueL(R_ASP_SETTING_VALUE_NONE);
			}

			aItem->iMaxLength = KAspMaxServerIdLength;
			aItem->iLatinInput = ETrue;
			aItem->iItemType = CAspListItemData::ETypeText;
			
			TUtil::StrCopy(iOldServerId, iBuf);
			break;
			}
		
		
		case EAspConnType:
			{
			aItem->iNumberData = iProfile->BearerType();

			if (!iBearerHandler->IsSupported(aItem->iNumberData))
				{
				aItem->iNumberData = iBearerHandler->DefaultBearer();
				}

 			iBearerHandler->GetBearerName(iBuf, aItem->iNumberData);
			aItem->SetValueL(iBuf);
			aItem->iItemType = CAspListItemData::ETypeBearerList;

			break;
			}
		
		case EAspAccessPoint:
			{
			TAspAccessPointItem item;
	        item.iUid = iProfile->AccessPointL();
	        TInt ret = iApHandler->GetInternetApInfo(item);
	        if (ret == KErrNone)
	        	{
	        	aItem->iNumberData = item.iUid;
	        	aItem->SetValueL(item.iName);
	        	aItem->iResource = R_ASP_NETWORK_CONNECTION;
	        	}
	        else
	        	{
	        	aItem->iNumberData = CAspAccessPointHandler::KDefaultConnection;	
	        	aItem->SetDisplayValueL(R_ASP_DEFAULT_CONNECTION);
	        	aItem->iResource = R_ASP_NETWORK_CONNECTION;
	        	}
	        aItem->iItemType = CAspListItemData::ETypeInternetAccessPoint;
			break;
			}
		
		case EAspHostAddress:
			{
			iProfile->GetHostAddress(iBuf, aItem->iNumberData);
			aItem->SetValueL(iBuf);
			if(TDialogUtil::IsPCSuiteDesC(iBuf))
			{
				aItem->SetDisplayValueL(R_ASP_PC_SUITE_PROFILE);	
	 		}
	        else
	        {
	        	aItem->SetDisplayValueL(R_ASP_MUST_BE_DEFINED);
	        }
			
			aItem->iMaxLength = KAspMaxURILength;
			aItem->iLatinInput = ETrue;
			aItem->iItemType = CAspListItemData::ETypeHostAddress;
			aItem->iMandatory = ETrue;
			
			break;
			}
		    
		case EAspPort:
			{
			iProfile->GetHostAddress(iBuf, aItem->iNumberData);
			if ( aItem->iNumberData > TURIParser::EMaxURIPortNumber || aItem->iNumberData < TURIParser::EMinURIPortNumber)
  					 	aItem->iNumberData = TURIParser::EDefaultHttpPort;	
			aItem->SetValueL(aItem->iNumberData);
			
			aItem->iMaxLength = KBufSize16;
			aItem->iMaxValue = TURIParser::EMaxURIPortNumber;
			aItem->iMinValue = TURIParser::EMinURIPortNumber;
			aItem->iItemType = CAspListItemData::ETypeNumber;
			aItem->iLatinInput = ETrue;
			break;
			}
		    
		case EAspUsername:
			{
			iProfile->GetUserName(iBuf);
			aItem->SetValueL(iBuf);
			aItem->SetDisplayValueL(R_ASP_SETTING_VALUE_NONE);

			aItem->iMaxLength = KAspMaxUsernameLength;
			aItem->iLatinInput = ETrue;
			aItem->iItemType = CAspListItemData::ETypeText;
			break;
			}
		    
		case EAspPassword:
			{
			iProfile->GetPassword(iBuf);
			aItem->SetValueL(iBuf);
			TUtil::Fill(iBuf, KSecretEditorMask, KSecretEditorMaskLength);
			aItem->SetDisplayValueL(iBuf);

			aItem->iMaxLength = KAspMaxPasswordLength;
			aItem->iItemType = CAspListItemData::ETypeSecret;
			break;
			}
		
		case EAspHttpsUsername:
			{
			iProfile->GetHttpUserNameL(iBuf);
			aItem->SetValueL(iBuf);
			aItem->SetDisplayValueL(R_ASP_SETTING_VALUE_NONE);

			aItem->iMaxLength = KAspMaxUsernameLength;
			aItem->iLatinInput = ETrue;
			aItem->iItemType = CAspListItemData::ETypeText;
			break;
			}

		case EAspHttpsPassword:
			{
			iProfile->GetHttpPasswordL(iBuf);
			aItem->SetValueL(iBuf);
			TUtil::Fill(iBuf, KSecretEditorMask, KSecretEditorMaskLength);
			aItem->SetDisplayValueL(iBuf);

			aItem->iMaxLength = KAspMaxPasswordLength;
			aItem->iItemType = CAspListItemData::ETypeSecret;
			break;
			}
		
		case EAspServerAlert:
			{
			CDesCArray* arr = iResHandler->ReadDesArrayLC(R_ASP_SERVER_ALERT_TYPE);

            aItem->iNumberData = iProfile->SASyncState();
			aItem->iResource = R_ASP_SERVER_ALERT_TYPE;
			aItem->SetValueL((*arr)[aItem->iNumberData]);
			aItem->iItemType = CAspListItemData::ETypeList;
			
			CleanupStack::PopAndDestroy(arr);
			break;
			}
			
		case EAspSyncDirection:
			{
    		CDesCArray* arr;
    		TBool isPCSuite = CAspProfile::IsPCSuiteProfile(iProfile);
    		if(isPCSuite)
			 {
			 	
			    arr = iResHandler->ReadDesArrayLC(R_ASP_SYNC_PCSUITE_TYPE);
    			aItem->iResource = R_ASP_SYNC_PCSUITE_TYPE;
    			   		
			 }
			 else
			 {
			 	arr = iResHandler->ReadDesArrayLC(R_ASP_SYNC_TYPE);
    			aItem->iResource =R_ASP_SYNC_TYPE ;
    			   		
			 }
    	
    		aItem->iNumberData = iContentList->SyncDirection();
			aItem->SetValueL((*arr)[aItem->iNumberData]);
			aItem->iItemType = CAspListItemData::ETypeList;
    		
    		CleanupStack::PopAndDestroy(arr);
			break;
			}
			
        
        case EAspSyncContent:
        	{
        	break;
        	}
        	
        default:
           	__ASSERT_DEBUG(EFalse, TUtil::Panic(KErrGeneral));
			break;
		
		}
	}


// ----------------------------------------------------------------------------
// CAspProfileDialog::SaveSettingsL
// 
// ----------------------------------------------------------------------------
//
void CAspProfileDialog::SaveSettingsL()
	{
    if (iEditMode == EDialogModeReadOnly || 
        iEditMode == EDialogModeSettingEnforcement)
    	{
    	return;
    	}

   	iBuf = KNullDesC;  // reset common buffer
   	
	TInt count = iSettingList->Count();
	
	for (TInt i=0; i<count; i++)
		{
		CAspListItemData* item = (*iSettingList)[i];
		
		switch (item->iItemId)
			{
			case EAspProfileName:
			    iProfile->SetNameL(item->Value());
			    break;

			case EAspProtocolVersion:
				iProfile->SetProtocolVersionL(item->iNumberData);
			    break;

			case EAspServerId:
			    iProfile->SetServerIdL(item->Value());
			    break;

			case EAspConnType:
				iProfile->SetBearerTypeL(item->iNumberData);
			    break;

			case EAspAccessPoint:
				iProfile->SetAccessPointL(item->iNumberData);
			    break;

			case EAspHostAddress:
				{
				iBuf = item->Value();
				TURIParser parser(iBuf);
				TInt portNumber = parser.Port();
	            if (!parser.IsValidPort(portNumber))
		            {
		            	if ( Item(EAspPort)->iNumberData > TURIParser::EMaxURIPortNumber || Item(EAspPort)->iNumberData < TURIParser::EMinURIPortNumber)
  							    Item(EAspPort)->iNumberData = TURIParser::EDefaultHttpPort;	
				 		portNumber = Item(EAspPort)->iNumberData;    	
		            }

				iProfile->SetHostAddressL(iBuf, portNumber);
				break;
				}

			case EAspPort:
				break;  // port is included in host address

			case EAspUsername:
				iProfile->SetUserNameL(item->Value());
			    break;
			
			case EAspPassword:
			    iProfile->SetPasswordL(item->Value());
			    break;

			case EAspServerAlert:
				{
				iProfile->SetSASyncStateL(item->iNumberData);
			    break;
				}

			case EAspHttpsUsername:
			    iProfile->SetHttpUserNameL(item->Value());
			    break;

			case EAspHttpsPassword:
			    iProfile->SetHttpPasswordL(item->Value());
			    break;
			    
			case EAspSyncDirection:
				iContentList->ModifyTaskDirectionsL(item->iNumberData);
				break;
			    
			}

		}

	iProfile->Save();
	iContentList->UpdateLocalDatabaseL();
#ifdef RD_DSUI_TIMEDSYNC
	CheckAutoSyncSetttigsL();
#endif
	}
	
	
//------------------------------------------------------------------------------
// CAspProfileDialog::EditSettingItemL
//
// Calls setting editing functions. 
//------------------------------------------------------------------------------
//
TBool CAspProfileDialog::EditSettingItemL(CAspListItemData& aItem)
	{
	TInt ret = EFalse;

#ifdef RD_DSUI_TIMEDSYNC 
	CAspSchedule* schedule = CAspSchedule::NewLC();
	if (schedule->IsAutoSyncEnabled())
		{
		TInt profileId = schedule->ProfileId();
		if (profileId == iProfile->ProfileId())
			{
			TDialogUtil::ShowInformationQueryL(R_ASP_NOTE_MODIFY_AUTO_SYNC_SETTINGS);
			}
		}
    CleanupStack::PopAndDestroy(schedule);
#endif

	switch (aItem.iItemType)
		{
		case CAspListItemData::ETypeInternetAccessPoint:
		    ret = EditSettingItemAccessPointL(aItem);
		    break;
			
		case CAspListItemData::ETypeText:
			ret = EditSettingItemTextL(aItem);
		    break;

		case CAspListItemData::ETypeSecret:
			ret = EditSettingItemSecretL(aItem);
		    break;

		case CAspListItemData::ETypeNumber:
			ret = EditSettingItemNumberL(aItem);
		    break;

		case CAspListItemData::ETypeList:
			ret = EditSettingItemListL(aItem);
		    break;
		
		case CAspListItemData::ETypeBearerList:
			ret = EditSettingItemBearerListL(aItem);
		    break;
		    
		case CAspListItemData::ETypeProtocolList:
			ret = EditSettingItemProtocolListL(aItem);
		    break;

		case CAspListItemData::ETypeListYesNo:
			ret = EditSettingItemYesNoL(aItem);
		    break;

		case CAspListItemData::ETypeHostAddress:
			ret = EditSettingItemHostAddressL(aItem);
			break;

		case CAspListItemData::ETypeSyncContent:
			ret = EditSettingItemSyncContentL(aItem);
			break;

		default:
		    break;
		}
	
	return ret;
	}


//------------------------------------------------------------------------------
// CAspProfileDialog::EditSettingItemYesNoL
//
// Change Yes/No value without showing radio button editor.
//------------------------------------------------------------------------------
//
TBool CAspProfileDialog::EditSettingItemYesNoL(CAspListItemData& aItem)
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
// CAspProfileDialog::EditSettingItemNumberL
//
//------------------------------------------------------------------------------
//
TBool CAspProfileDialog::EditSettingItemNumberL(CAspListItemData& aItem)
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
// CAspProfileDialog::EditSettingItemSecretL
//
//------------------------------------------------------------------------------
//
TBool CAspProfileDialog::EditSettingItemSecretL(CAspListItemData& aItem)
	{
	TBuf<KBufSize64> newPassword;
	newPassword = aItem.Value();
	
	
	CAspAlphaPasswordSettingPageEditor* pwd = new (ELeave) CAspAlphaPasswordSettingPageEditor(
		                                R_ASP_SETTING_PASSWORD, newPassword, aItem.Value());
	
	CleanupStack::PushL(pwd);
	pwd->SetSettingTextL(aItem.Header()); 
	pwd->ConstructL();
	pwd->AlphaPasswordEditor()->SetMaxLength(aItem.iMaxLength);
	//pwd->AlphaPasswordEditor()->RevealSecretText(ETrue);  // pasi
	CleanupStack::Pop(pwd);

	if (pwd->ExecuteLD(CAknSettingPage::EUpdateWhenChanged))
		{
		aItem.SetValueL(newPassword);
		return ETrue;
		}
	
	return EFalse;
	}


//------------------------------------------------------------------------------
// CAspProfileDialog::EditSettingItemTextL
//
// Edit text setting item.
//------------------------------------------------------------------------------
//
TBool CAspProfileDialog::EditSettingItemTextL(CAspListItemData& aItem)
	{
	TUtil::StrCopy(iBuf, aItem.Value());
	
	TInt len = iBuf.Length();
	if (len > aItem.iMaxLength)
		{
		iBuf.SetLength(aItem.iMaxLength);
		}
	
    TBool ret = TDialogUtil::ShowTextEditorL(iBuf, aItem.Header(), 
                             aItem.iMandatory, aItem.iLatinInput, aItem.iMaxLength);
   	if (ret)
		{
		aItem.SetValueL(iBuf);
		}
		
	return ret;
	}


//------------------------------------------------------------------------------
// CAspProfileDialog::EditSettingItemHostAddressL
//
//------------------------------------------------------------------------------
//
TBool CAspProfileDialog::EditSettingItemHostAddressL(CAspListItemData& aItem)
	{
	_LIT(KHttpHeader, "http://");
	
	TBool httpHeaderUsed = EFalse;
	TInt bearer = Item(EAspConnType)->iNumberData;
	HBufC* hBuf = HBufC::NewLC(KBufSize255);
	TPtr ptr = hBuf->Des();
	TUtil::StrCopy(ptr, aItem.Value());
	if (TUtil::IsEmpty(aItem.Value()) && bearer == EAspBearerInternet)
		{
		aItem.SetValueL(KHttpHeader);
		httpHeaderUsed = ETrue;
		}
		
	TBool ret = EFalse;
	for(;;)
	{
	ret = EditSettingItemTextL(aItem);
	
	if (httpHeaderUsed && (aItem.Value().Compare(KHttpHeader) == 0))
		{
		aItem.SetValueL(KNullDesC);
		}
	
	
	if (ret)
		{
		if(bearer == EAspBearerInternet)
		{
			if (!TURIParser::IsValidUri(aItem.Value()))
			{			
			TDialogUtil::ShowInformationNoteL(R_ASP_LOG_ERR_URIINVALID);
			}
		else
			{
			TPtrC ptr = aItem.Value();
			TURIParser parser(ptr);
	        TInt portNumber = parser.Port();
	        if (parser.IsValidPort(portNumber))
		        {
		        Item(EAspPort)->iNumberData = portNumber;
		        Item(EAspPort)->SetValueL(portNumber);
		        }
		    break;    
			}	
		}
		else if(bearer == EAspBearerBlueTooth)
		{
			if (!TURIParser::IsValidBluetoothHostAddress(aItem.Value()))
		    {
		    TDialogUtil::ShowInformationNoteL(R_ASP_NOTE_INCORRECT_HOST_ADDRESS);
		   
		    }
		    else
		    {
		    	break;
		    }
			}
		}
	else if(!ret) 
		{
		aItem.SetValueL(ptr);
		break;	
		}
	else
		{
		 break;	
		}
	}
	CleanupStack::PopAndDestroy(); //hBuf
	return ret;
	}


//------------------------------------------------------------------------------
// CAspProfileDialog::EditSettingItemListL
//
//------------------------------------------------------------------------------
//
TBool CAspProfileDialog::EditSettingItemListL(CAspListItemData& aItem)
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
// CAspProfileDialog::EditSettingItemProtocolListL
//
//------------------------------------------------------------------------------
//
TBool CAspProfileDialog::EditSettingItemProtocolListL(CAspListItemData& aItem)
	{
#ifdef __SYNCML_DS_EMAIL   // KFeatureIdSyncMlDsEmail
	TInt oldSelection = aItem.iNumberData;
#endif
	
	TInt curSelection = aItem.iNumberData;
	CDesCArray* arr2 = iResHandler->ReadDesArrayLC(aItem.iResource);
	CDesCArray* arr = TUtil::NumberConversionLC(arr2);
	
	
	if (TDialogUtil::ShowListEditorL(arr, aItem.Header(), curSelection))
		{
		aItem.iNumberData = curSelection; 
		aItem.SetValueL((*arr)[curSelection]);
		
	    
#ifdef __SYNCML_DS_EMAIL   // KFeatureIdSyncMlDsEmail

	    TBool emailSync = EFalse;
	    TInt id = iProfile->ApplicationId();
	    if (id == EApplicationIdSync || id == EApplicationIdEmail)
	    	{
	    	emailSync = ETrue;
	    	}
		

#ifdef RD_DSUI_TIMEDSYNC
		if (curSelection == EAspProtocol_1_1 )
				{
				CAspSchedule* schedule = CAspSchedule::NewLC();
				TInt profileId = schedule->ProfileId();
				if (profileId == iProfile->ProfileId())
					{
					TInt selectedContentCnt = 0;
					TInt selectedContentIndex = 0;
					TInt emailIndex = iContentList->FindProviderIndex(KUidNSmlAdapterEMail.iUid);
					schedule->ContentSelectionInfo(selectedContentCnt, selectedContentIndex);
					if (selectedContentCnt == 1 && selectedContentIndex == emailIndex)
						{
						schedule->SetProfileId(KErrNotFound);
						schedule->SetContentEnabled(emailIndex, EFalse);
						schedule->SetSyncPeakSchedule(CAspSchedule::EIntervalManual);
						schedule->SetSyncOffPeakSchedule(CAspSchedule::EIntervalManual);
						schedule->SetSyncFrequency(CAspSchedule::EIntervalManual);
						schedule->UpdateSyncScheduleL();
						schedule->SaveL();
						}
					}
				CleanupStack::PopAndDestroy(schedule);
				}
#endif
		
#endif
		
		CleanupStack::PopAndDestroy(arr);
		CleanupStack::PopAndDestroy(arr2);
		return ETrue;
		}

	CleanupStack::PopAndDestroy(arr);
	CleanupStack::PopAndDestroy(arr2);
	return EFalse;
	}
//------------------------------------------------------------------------------
// CAspProfileDialog::EditSettingItemAccessPointL
//
//------------------------------------------------------------------------------
//	
TBool CAspProfileDialog::EditSettingItemAccessPointL(CAspListItemData& aItem)
{

	TBool ret;
	TInt selection = 0;
	TInt curSelection = aItem.iNumberData;
	CDesCArray* arr = CAspResHandler::ReadDesArrayStaticLC(aItem.iResource);
	if(curSelection < 0)
	{
		curSelection = 0;
	}
	else
	{
		curSelection = 1;
	}
	ret = TDialogUtil::ShowListEditorL(arr, aItem.Header(), curSelection);
	CleanupStack::PopAndDestroy(arr);
	if(!ret)
	{
		return EFalse;
	}
	if (curSelection == 0 )
		{
		aItem.iNumberData = CAspAccessPointHandler::KDefaultConnection;
		aItem.SetValueL(KNullDesC);
		aItem.SetDisplayValueL(R_ASP_DEFAULT_CONNECTION);
		selection = CAspAccessPointHandler::KDefaultConnection;
		}	
	if(selection == CAspAccessPointHandler::KDefaultConnection)
	{
	#ifdef RD_DSUI_TIMEDSYNC
   	CAspSchedule* schedule = CAspSchedule::NewLC();
	TInt profileId = schedule->ProfileId();
	TInt enabled = schedule->IsAutoSyncEnabled();
	CleanupStack::PopAndDestroy(schedule);
	if(profileId == iProfile->ProfileId() && enabled)
		{
		TBool showQuery = ETrue;
			RCmManager cmmgr;
			cmmgr.OpenL();
			TCmDefConnValue defConnValue;
			cmmgr.ReadDefConnL(defConnValue);
			cmmgr.Close();
			if(defConnValue.iType == ECmDefConnDestination)
			{
				showQuery = EFalse;
			}
		if(showQuery)
		{
			HBufC* hBuf = CAspResHandler::ReadLC(R_ASP_QUERY_IAP_NO_ASK_ALWAYS);
    		if (!TDialogUtil::ShowConfirmationQueryL(hBuf->Des()))
	 		{
	  	  		CleanupStack::PopAndDestroy(hBuf);
				return ETrue;// user selected "Always ask" option 
    	}	
			CleanupStack::PopAndDestroy(hBuf);
		}
		else
		{
			return ETrue;
		}
			
		}
		else
		{
			return ETrue;
		}
    	   	
	#else
		return ETrue;
	#endif
	}
	TAspAccessPointItem item;
	item.iUid2 = aItem.iNumberData;
	
	ret = iApHandler->ShowApSelectDialogL(item);
	
	if (ret == CAspAccessPointHandler::EAspDialogSelect)
		{
		aItem.iNumberData = item.iUid;
		aItem.SetValueL(item.iName);
		return ETrue;
		}
	else if (ret == CAspAccessPointHandler::EAspDialogExit)
		{
		ProcessCommandL(EAknCmdExit); // user has selected "Exit" from options menu
		}
	else
		{
		// user canceled ap selection
		return EFalse;
		}
		
	return EFalse;
}

//------------------------------------------------------------------------------
// CAspProfileDialog::EditSettingItemBearerListL
//
//------------------------------------------------------------------------------
//
TBool CAspProfileDialog::EditSettingItemBearerListL(CAspListItemData& aItem)
	{
	CDesCArray* arr = iBearerHandler->BuildBearerListLC();
	if (arr->Count() == 0) 
		{
		User::Leave(KErrNotFound);
		}

	TInt curSelection = iBearerHandler->ListIndexForBearer(aItem.iNumberData);
	if (curSelection == KErrNotFound)
		{
		User::Leave(KErrNotFound);
		}

	if (TDialogUtil::ShowListEditorL(arr, aItem.Header(), curSelection))
		{
		aItem.iNumberData = iBearerHandler->BearerForListIndex(curSelection);
       	aItem.SetValueL((*arr)[curSelection]);
		CleanupStack::PopAndDestroy(arr);
		return ETrue;
		}
	
	CleanupStack::PopAndDestroy(arr);
	return EFalse;
	}



// -----------------------------------------------------------------------------
// CAspContentListDialog::EditSettingItemSyncContentL
//
// -----------------------------------------------------------------------------
//
TInt CAspProfileDialog::EditSettingItemSyncContentL(CAspListItemData& aItem)
	{
 	TAspParam param(iApplicationId, iSyncSession);
    param.iProfile = iProfile;
    param.iDataProviderId = aItem.iNumberData;
    param.iSyncTaskId = KErrNotFound;
    param.iContentList = iContentList;

    CAspContentDialog::ShowDialogL(param);
     
    return ETrue;
	}


//-----------------------------------------------------------------------------
// CAspProfileDialog::GetItemForIndex
// 
// Find item in list position aIndex.
//-----------------------------------------------------------------------------
//
CAspListItemData* CAspProfileDialog::GetItemForIndex(TInt aIndex)
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
// CAspProfileDialog::Item
// 
// Find item with aItemId (TAspConnectionSettingItem).
//-----------------------------------------------------------------------------
//
CAspListItemData* CAspProfileDialog::Item(TInt aItemId)
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
// CAspProfileDialog::ListBox
// 
// -----------------------------------------------------------------------------
//
CAknSettingStyleListBox* CAspProfileDialog::ListBox()
	{
	return iSettingListBox;
	}


// -----------------------------------------------------------------------------
// CAspProfileDialog::UpdateCbaL
// 
// -----------------------------------------------------------------------------
//
void CAspProfileDialog::UpdateCbaL(TInt aResourceId)
	{
	CEikButtonGroupContainer& cba = ButtonGroupContainer();
	cba.SetCommandSetL(aResourceId);
	iCommandSetId = aResourceId;
	iMSKEmpty = EFalse;
	cba.DrawDeferred();
	}

// -----------------------------------------------------------------------------
// CAspProfileDialog::UpdateMenuL
// 
// -----------------------------------------------------------------------------
//

void CAspProfileDialog::UpdateMenuL(TInt aResource)
	{
	CEikMenuBar* menuBar = iMenuBar; // from CAknDialog
	menuBar->SetMenuTitleResourceId(aResource);

	TRAPD(err, menuBar->TryDisplayMenuBarL());
    		
	User::LeaveIfError(err);
	}

// -----------------------------------------------------------------------------
// CAspProfileDialog::CheckAutoSyncSetttigsL
// 
// -----------------------------------------------------------------------------
//
void CAspProfileDialog::CheckAutoSyncSetttigsL()
	{
	CAspSchedule* schedule = CAspSchedule::NewLC();
	TInt profileId = schedule->ProfileId();
	if (profileId == iProfile->ProfileId())
			{
			if (iProfile->BearerType() != EAspBearerInternet)
				{
				schedule->SetProfileId(KErrNotFound);
				schedule->SetSyncPeakSchedule(CAspSchedule::EIntervalManual);
				schedule->SetSyncOffPeakSchedule(CAspSchedule::EIntervalManual);
				schedule->SetSyncFrequency(CAspSchedule::EIntervalManual);
				schedule->UpdateSyncScheduleL();
				schedule->SaveL();
				}
			else
				{
				schedule->UpdateProfileSettingsL();
				}
			}
	CleanupStack::PopAndDestroy(schedule);	
	}
	
//  End of File  
