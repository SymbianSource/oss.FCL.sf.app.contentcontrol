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

#include "AspAdvanceSettingsDialog.h"
#include "AspContentDialog.h"
#include "AspSyncUtil.rh"
#include "AspDebug.h"
#include <csxhelp/ds.hlp.hrh>

#include <aspsyncutil.mbg>  // for bitmap enumerations
#include <AknIconArray.h>   // for GulArray
#include <aknpasswordsettingpage.h>     // CAknAlphaPasswordSettingPage
#include <ConnectionUiUtilities.h>      // CConnectionUiUtilities
#include <featmgr.h>   // FeatureManager
#include <AspDefines.h> //For enum TRoamingSettings
#include <centralrepository.h> //For central Repository
#include "CPreSyncPlugin.h"

const TInt KMSKControlId( CEikButtonGroupContainer::EMiddleSoftkeyPosition );

// ============================ MEMBER FUNCTIONS ===============================



// -----------------------------------------------------------------------------
// CAspProfileDialog::ShowDialogL
// 
// -----------------------------------------------------------------------------
TBool CAspAdvanceSettingsDialog::ShowDialogL()
	{
	CAspAdvanceSettingsDialog* dialog = CAspAdvanceSettingsDialog::NewL();

	TBool ret = dialog->ExecuteLD(R_ASP_ADVANCE_SETTINGS_DIALOG);

    return ret;
	}


// -----------------------------------------------------------------------------
// CAspAdvanceSettingsDialog::NewL
//
// -----------------------------------------------------------------------------
CAspAdvanceSettingsDialog* CAspAdvanceSettingsDialog::NewL()
    {
    FLOG( _L("CAspAdvanceSettingsDialog::NewL START") );

    CAspAdvanceSettingsDialog* self = new ( ELeave ) CAspAdvanceSettingsDialog();
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);

    FLOG( _L("CAspAdvanceSettingsDialog::NewL END") );
    return self;
    }


// -----------------------------------------------------------------------------
// CAspAdvanceSettingsDialog::CAspAdvanceSettingsDialog
// 
// -----------------------------------------------------------------------------
//
CAspAdvanceSettingsDialog::CAspAdvanceSettingsDialog()
	{
    }


// -----------------------------------------------------------------------------
// CAspAdvanceSettingsDialog::ConstructL
//
// -----------------------------------------------------------------------------
//
void CAspAdvanceSettingsDialog::ConstructL()
    {
    FLOG( _L("CAspAdvanceSettingsDialog::ConstructL START") );
	
	// contruct menu for our dialog
	CAknDialog::ConstructL(R_ADVANCE_SETTINGS_DIALOG_MENU);

	iResHandler = CAspResHandler::NewL();

	iSettingList = new (ELeave) CArrayPtrFlat<CAspListItemData>(1);
	
	// get previous title so it can be restored
	iStatusPaneHandler = CStatusPaneHandler::NewL(iAvkonAppUi);
	iStatusPaneHandler->StoreOriginalTitleL();
	
	// store current navi pane
	iStatusPaneHandler->StoreNavipaneL();

	FLOG( _L("CAspAdvanceSettingsDialog::ConstructL END") );
    } 


// ----------------------------------------------------------------------------
// Destructor
//
// ----------------------------------------------------------------------------
//
CAspAdvanceSettingsDialog::~CAspAdvanceSettingsDialog()
    {
    FLOG( _L("CAspAdvanceSettingsDialog::~CAspAdvanceSettingsDialog START") );

	delete iResHandler;
	
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

	FLOG( _L("CAspAdvanceSettingsDialog::~CAspAdvanceSettingsDialog END") );
    }


//------------------------------------------------------------------------------
// CAspAdvanceSettingsDialog::ActivateL
//
// Called by system when dialog is activated.
//------------------------------------------------------------------------------
//
void CAspAdvanceSettingsDialog::ActivateL()
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
void CAspAdvanceSettingsDialog::GetHelpContext(TCoeHelpContext& aContext) const
	{
	aContext.iMajor = KUidSmlSyncApp;
	aContext.iContext = KDS_HLP_MAIN_VIEW;    
	}


// -----------------------------------------------------------------------------
// CAspAdvanceSettingsDialog::HandleListBoxEventL
// 
// -----------------------------------------------------------------------------
#ifdef RD_SCALABLE_UI_V2
void CAspAdvanceSettingsDialog::HandleListBoxEventL(CEikListBox* /*aListBox*/,
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
#else
void CAspAdvanceSettingsDialog::HandleListBoxEventL(CEikListBox* /*aListBox*/,
                                            TListBoxEvent /*aEventType*/)
{
	
}
#endif


// -----------------------------------------------------------------------------
// CAspAdvanceSettingsDialog::PreLayoutDynInitL
// 
// -----------------------------------------------------------------------------
//
void CAspAdvanceSettingsDialog::PreLayoutDynInitL()
    {
    iSettingListBox = (CAknSettingStyleListBox*) ControlOrNull (EAdvanceSettingsDialogList);
    
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
		
	iStatusPaneHandler->SetNaviPaneTitleL(R_ASP_TITLE_ADVANCED_SETTINGS);
    }


// ----------------------------------------------------------------------------
// CAspAdvanceSettingsDialog::SetMiddleSoftKeyLabelL
//
// ----------------------------------------------------------------------------
//
void CAspAdvanceSettingsDialog::SetEmptyMiddleSoftKeyLabelL()
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
// CAspAdvanceSettingsDialog::SetIconsL
//
// ----------------------------------------------------------------------------
//
void CAspAdvanceSettingsDialog::SetIconsL()
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
// CAspAdvanceSettingsDialog::IconL
// 
// -----------------------------------------------------------------------------
//
CGulIcon* CAspAdvanceSettingsDialog::IconL(TAknsItemID aId, const TDesC& aFileName, TInt aFileIndex, TInt aFileMaskIndex)
	{
    return TDialogUtil::CreateIconL(aId, aFileName, aFileIndex, aFileMaskIndex);
	}


//------------------------------------------------------------------------------
// CAspAdvanceSettingsDialog::DynInitMenuPaneL
//
// Called by system before menu is shown.
//------------------------------------------------------------------------------
//
void CAspAdvanceSettingsDialog::DynInitMenuPaneL(TInt /*aResourceID*/, CEikMenuPane* aMenuPane)
	{   
	CAspListItemData* item = GetItemForIndex(ListBox()->CurrentItemIndex());
	if (item->iHidden == EVisibilityReadOnly)
		{
		TDialogUtil::DeleteMenuItem(aMenuPane, EAspMenuCmdChange);
		}
	
	if (!FeatureManager::FeatureSupported(KFeatureIdHelp))
		{
		TDialogUtil::DeleteMenuItem(aMenuPane, EAspMenuCmdHelp);
		}
    }


//------------------------------------------------------------------------------
// CAspAdvanceSettingsDialog::ProcessCommandL
//
// Handle commands from menu.
//------------------------------------------------------------------------------
//
void CAspAdvanceSettingsDialog::ProcessCommandL(TInt aCommandId)
	{
	HideMenu();

	switch (aCommandId)
		{
		case EAspMenuCmdHelp:
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
// CAspAdvanceSettingsDialog::OkToExitL
//
//------------------------------------------------------------------------------
//
TBool CAspAdvanceSettingsDialog::OkToExitL(TInt aButtonId)
	{
	if (aButtonId == EEikBidCancel)
		{
		// save silently and close dialog
		
		CheckSettingValues(aButtonId);

		return ETrue; // close dialog
		}

	if (aButtonId == EAknSoftkeyBack)
        {
        TBool ret = CheckSettingValues(aButtonId);
        if (!ret)
        	{
        	return EFalse; // leave dialog open
        	}
        	
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
			UpdateMenuL(R_ADVANCE_SETTINGS_DIALOG_MENU);
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
// CAspAdvanceSettingsDialog::CheckSettingValuesL
//
//------------------------------------------------------------------------------
//
TBool CAspAdvanceSettingsDialog::CheckSettingValuesL(TInt /* aButtonId */)
	{
	
    if (iEditMode == EDialogModeReadOnly || 
        iEditMode == EDialogModeSettingEnforcement)
    	{
    	return ETrue;
    	}
	
    return ETrue;
	}


//------------------------------------------------------------------------------
// CAspAdvanceSettingsDialog::CheckSettingValues
//
//------------------------------------------------------------------------------
//
TBool CAspAdvanceSettingsDialog::CheckSettingValues(TInt aButtonId)
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
// CAspAdvanceSettingsDialog::OfferKeyEventL
// 
// ----------------------------------------------------------------------------
//
TKeyResponse CAspAdvanceSettingsDialog::OfferKeyEventL(const TKeyEvent& aKeyEvent,TEventCode aType)
	{
	if (aType == EEventKey)
		{
		switch (aKeyEvent.iCode)
			{
			case EKeyEnter:
			case EKeyOK:
				{
					OkToExitL(EAknSoftkeyOpen);
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
// CAspAdvanceSettingsDialog::CheckContentSetting
// modifies MSK and menu item depending on the item in the list box when touch events are handled
// ----------------------------------------------------------------------------------------------
//

void CAspAdvanceSettingsDialog::CheckContentSettingL()
{
	TInt curIndex = ListBox()->CurrentItemIndex();
	TInt count = iSettingList->Count();
	CAspListItemData* lastItem = (*iSettingList)[--count];
	

		
	CAspListItemData* curItem = GetItemForIndex(curIndex);
	//TBool isPCSuite = CAspProfile::IsPCSuiteProfile(iProfile);
	//if(isPCSuite && (curItem->iHidden == EVisibilityReadOnly))
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
// CAspAdvanceSettingsDialog::CheckContentSetting
// modifies MSK based on the item
// ----------------------------------------------------------------------------
//

void CAspAdvanceSettingsDialog::CheckContentSettingL(const TKeyEvent& aKeyEvent)
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
	//TBool isPCSuite = CAspProfile::IsPCSuiteProfile(iProfile);
	//if(isPCSuite && (curItem->iHidden == EVisibilityReadOnly))
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
// CAspAdvanceSettingsDialog::HandleResourceChange
// 
// ----------------------------------------------------------------------------
//
void CAspAdvanceSettingsDialog::HandleResourceChange(TInt aType)
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
// CAspAdvanceSettingsDialog::HandleOKL
//
//------------------------------------------------------------------------------
//
void CAspAdvanceSettingsDialog::HandleOKL()
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
// CAspAdvanceSettingsDialog::CreateSettingsListL
// 
// Function creates setting list array (iSettingsList).
// Note: enum TAspConnectionSettings must match with string array
// (R_ASP_CONNECTION_DIALOG_SETTING_ITEMS).
// ----------------------------------------------------------------------------
//
void CAspAdvanceSettingsDialog::CreateSettingsListL()
	{
	// read setting headers from resource
	CDesCArray* arr = iResHandler->ReadDesArrayLC(R_ADVANCE_SETTINGS_DIALOG_ITEMS);
	
	// add one CAspListItemData for each setting
	AddItemL(ERoamingSettings, arr);

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
// CAspAdvanceSettingsDialog::AddItemL
// 
// ----------------------------------------------------------------------------
//
void CAspAdvanceSettingsDialog::AddItemL(TInt aItemId, CDesCArray* aHeaders)
	{
    CAspListItemData* item = CAspListItemData::NewLC();
	item->SetHeaderL((*aHeaders)[aItemId]);
	item->iItemId = aItemId;
	iSettingList->AppendL(item);
	CleanupStack::Pop(item);
	}
// ----------------------------------------------------------------------------
// CAspAdvanceSettingsDialog::AddItemL
// 
// ----------------------------------------------------------------------------
//
void CAspAdvanceSettingsDialog::AddItemL(TInt aItemId, TInt aResourceId)
	{
    CAspListItemData* item = CAspListItemData::NewLC();
	item->SetHeaderL(aResourceId);
	item->iItemId = aItemId;
	iSettingList->AppendL(item);
	CleanupStack::Pop(item);
	}

// ----------------------------------------------------------------------------
// CAspAdvanceSettingsDialog::UpdateListBoxL
// 
// Add settings headers into listbox.
// ----------------------------------------------------------------------------
//
void CAspAdvanceSettingsDialog::UpdateListBoxL(CEikTextListBox* aListBox,
                                       CAspSettingList* aItemList)
	{
	CDesCArray* arr = (CDesCArray*)aListBox->Model()->ItemTextArray();
	arr->Reset();

	TInt count = aItemList->Count();
	for (TInt i=0; i<count; i++ )
		{	
		CAspListItemData* item = (*aItemList)[i];		
		TInt aValue = 0;
		ReadRepositoryL(KNSmlRoamingSettingKey, aValue);
		if(aValue != ERoamingSettingBlock)
		{
		WriteRepositoryL(KNSmlRoamingSettingKey, item->iNumberData);
		}
		else
		{
		SetEmptyMiddleSoftKeyLabelL();
		}
		    		
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


// -----------------------------------------------------------------------------
// CAspAdvanceSettingsDialog::SetVisibility
// 
// -----------------------------------------------------------------------------
//
void CAspAdvanceSettingsDialog::SetVisibility()
	{
    TInt aValue(0);
    TRAPD(err ,ReadRepositoryL(KNSmlRoamingSettingKey, aValue));
    if(aValue == ERoamingSettingBlock)
    {
	Item(ERoamingSettings)->iHidden = EVisibilityReadOnly;
	}
	else
	{
	Item(ERoamingSettings)->iHidden = EVisibilityNormal;
	}
	}

void CAspAdvanceSettingsDialog::ReadRepositoryL(TInt aKey, TInt& aValue)
    {
    CRepository* rep = CRepository::NewLC(KCRUidNSmlDSApp);
    TInt err = rep->Get(aKey, aValue);
    User::LeaveIfError(err);
    
    CleanupStack::PopAndDestroy(rep);
    }

// -----------------------------------------------------------------------------
// CAspAdvanceSettingsDialog::InitSettingItemL
// 
// Constructs CAspListItemData for one connection setting.
// -----------------------------------------------------------------------------
//
void CAspAdvanceSettingsDialog::InitSettingItemL(CAspListItemData* aItem)
	{
	__ASSERT_ALWAYS(aItem, TUtil::Panic(KErrGeneral));	
	
	iBuf = KNullDesC;  // reset common buffer
	
	switch (aItem->iItemId)
		{		
        case ERoamingSettings:
            {
            CDesCArray* arr = iResHandler->ReadDesArrayLC(R_ASP_ROAMING_SETTINGS);
            TInt aValue(0);
            ReadRepositoryL(KNSmlRoamingSettingKey, aValue);

            if(aValue == ERoamingSettingBlock)
                {
                aItem->iNumberData = ERoamingSettingAlwaysAsk;
                }
            else
                {
                aItem->iNumberData = aValue;
                }
            //aItem->iNumberData = iProfile->SASyncState();
            aItem->iResource = R_ASP_ROAMING_SETTINGS;
            aItem->SetValueL((*arr)[aItem->iNumberData]);
            aItem->iItemType = CAspListItemData::ETypeList;
            
            CleanupStack::PopAndDestroy(arr);
            break;
            }
			
        default:
           	__ASSERT_DEBUG(EFalse, TUtil::Panic(KErrGeneral));
			break;
		
		}
	}
	
//------------------------------------------------------------------------------
// CAspAdvanceSettingsDialog::EditSettingItemL
//
// Calls setting editing functions. 
//------------------------------------------------------------------------------
//
TBool CAspAdvanceSettingsDialog::EditSettingItemL(CAspListItemData& aItem)
	{
	TInt ret = EFalse;
	
	switch (aItem.iItemType)
		{

		case CAspListItemData::ETypeList:
			ret = EditSettingItemListL(aItem);
		    break;
		

		default:
		    break;
		}
	
	return ret;
	}

//------------------------------------------------------------------------------
// CAspAdvanceSettingsDialog::EditSettingItemListL
//
//------------------------------------------------------------------------------
//
TBool CAspAdvanceSettingsDialog::EditSettingItemListL(CAspListItemData& aItem)
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

//-----------------------------------------------------------------------------
// CAspAdvanceSettingsDialog::GetItemForIndex
// 
// Find item in list position aIndex.
//-----------------------------------------------------------------------------
//
CAspListItemData* CAspAdvanceSettingsDialog::GetItemForIndex(TInt aIndex)
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
// CAspAdvanceSettingsDialog::Item
// 
// Find item with aItemId (TAspConnectionSettingItem).
//-----------------------------------------------------------------------------
//
CAspListItemData* CAspAdvanceSettingsDialog::Item(TInt aItemId)
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
// CAspAdvanceSettingsDialog::ListBox
// 
// -----------------------------------------------------------------------------
//
CAknSettingStyleListBox* CAspAdvanceSettingsDialog::ListBox()
	{
	return iSettingListBox;
	}


// -----------------------------------------------------------------------------
// CAspAdvanceSettingsDialog::UpdateCbaL
// 
// -----------------------------------------------------------------------------
//
void CAspAdvanceSettingsDialog::UpdateCbaL(TInt aResourceId)
	{
	CEikButtonGroupContainer& cba = ButtonGroupContainer();
	cba.SetCommandSetL(aResourceId);
	iCommandSetId = aResourceId;
	iMSKEmpty = EFalse;
	cba.DrawDeferred();
	}

// -----------------------------------------------------------------------------
// CAspAdvanceSettingsDialog::UpdateMenuL
// 
// -----------------------------------------------------------------------------
//

void CAspAdvanceSettingsDialog::UpdateMenuL(TInt aResource)
	{
	CEikMenuBar* menuBar = iMenuBar; // from CAknDialog
	menuBar->SetMenuTitleResourceId(aResource);

	TRAPD(err, menuBar->TryDisplayMenuBarL());
    		
	User::LeaveIfError(err);
	
			
	}
	
// -----------------------------------------------------------------------------
// CAspAdvanceSettingsDialog::UpdateMenuL
// 
// -----------------------------------------------------------------------------
//
void CAspAdvanceSettingsDialog::WriteRepositoryL(TInt aKey, const TInt& aValue)
    {
    const TUid KCRRoamingSettingUID = {0x101F9A1D};
    const TUid KRepositoryId = KCRRoamingSettingUID;

    CRepository* rep = CRepository::NewLC(KRepositoryId);
    TInt err = rep->Set(aKey, aValue);
    User::LeaveIfError(err);
    
    CleanupStack::PopAndDestroy(rep);
    }
//  End of File  
