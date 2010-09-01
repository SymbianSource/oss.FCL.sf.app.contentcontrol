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
#include "AspFilterDialog.h"
#include "AspFilter.h"
#include "AspUtil.h"
#include "AspDefines.h"
#include "AspDialogUtil.h"
#include "AspResHandler.h"
#include "AspSyncUtil.rh"
#include "AspDebug.h"

#include <AknRadioButtonSettingPage.h>  // CAknRadioButtonSettingPage
#include <AknCheckBoxSettingPage.h>  // CAknRadioButtonSettingPage
#include <AknTextSettingPage.h>         // for CAknIntegerSettingPage

//#include "ds.hlp.hrh"  // help text ids




// ============================ MEMBER FUNCTIONS ===============================


/*******************************************************************************
 * class CAspMultiChoiceListSettingPage
 *******************************************************************************/



// -----------------------------------------------------------------------------
// CAspFilterDialog::ShowDialogL
// 
// -----------------------------------------------------------------------------
TBool CAspFilterDialog::ShowDialogL(TAspParam& aParam)
	{
	CAspFilterDialog* dialog = CAspFilterDialog::NewL(aParam);

	TBool ret = dialog->ExecuteLD(R_ASP_FILTER_DIALOG);

    return ret;
	}


// -----------------------------------------------------------------------------
// CAspFilterDialog::NewL
//
// -----------------------------------------------------------------------------
CAspFilterDialog* CAspFilterDialog::NewL(TAspParam& aParam)
    {
    CAspFilterDialog* self = new (ELeave) CAspFilterDialog(aParam);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);

    return( self );
    }


//------------------------------------------------------------------------------
// CAspFilterDialog::CAspFilterDialog
//
//------------------------------------------------------------------------------
//
CAspFilterDialog::CAspFilterDialog(TAspParam& aParam)
	{
	iApplicationId = aParam.iApplicationId;
	iDialogParam = &aParam;
	iFilter = aParam.iSyncFilter;
	
    __ASSERT_ALWAYS(iFilter, TUtil::Panic(KErrGeneral));	
	}


//------------------------------------------------------------------------------
// Destructor
//
//------------------------------------------------------------------------------
//
CAspFilterDialog::~CAspFilterDialog()
	{
	if (iSettingList)
		{
		iSettingList->ResetAndDestroy();
	    delete iSettingList;
		}

	delete iStatusPaneHandler;
	delete iResHandler;
	
	if (iAvkonAppUi)
		{
		iAvkonAppUi->RemoveFromStack(this);
		}
	}


// -----------------------------------------------------------------------------
// CAspFilterDialog::ConstructL
//
// -----------------------------------------------------------------------------
//
void CAspFilterDialog::ConstructL()
    {
	CAknDialog::ConstructL(R_ASP_FILTER_DIALOG_MENU);
	
	iResHandler = CAspResHandler::NewL();
    iSettingList = new (ELeave) CArrayPtrFlat<CAspListItemData>(10);
	
	// get previous title so it can be restored
	iStatusPaneHandler = CStatusPaneHandler::NewL(iAvkonAppUi);
	iStatusPaneHandler->StoreOriginalTitleL();
    } 


//------------------------------------------------------------------------------
// CAspFilterDialog::ActivateL (from CCoeControl)
//
// Called by system when dialog is activated
//------------------------------------------------------------------------------
//
void CAspFilterDialog::ActivateL()
	{
    CAknDialog::ActivateL();

	// this cannot be in ConstructL which is executed before dialog is launched
	iAvkonAppUi->AddToStackL(this);
    }


//-----------------------------------------------------------------------------
// CAspFilterDialog::HandleOKL
//
// Handle content parameter editing.
//-----------------------------------------------------------------------------
//
void CAspFilterDialog::HandleOKL()
	{
	CAspListItemData* item = ItemForIndex(ListBox()->CurrentItemIndex());
	item->iFilter = iFilter;

	if (EditSettingItemL(*item))
		{
		SetVisibility();
		UpdateListBoxL(ListBox(), iSettingList);
		}
	}


//------------------------------------------------------------------------------
// CAspFilterDialog::ProcessCommandL
//
// Handle menu commands.
//------------------------------------------------------------------------------
//
void CAspFilterDialog::ProcessCommandL( TInt aCommandId )
	{
	HideMenu();

	switch (aCommandId)
		{
		case EAspMenuCmdHelp:
			{
            break;
			}

		case EAspMenuCmdChange:
			{				
			CAspListItemData* item = ItemForIndex(ListBox()->CurrentItemIndex());
			item->iSelectKeyPressed = EFalse;
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
// CAspFilterDialog::OkToExitL
//
//------------------------------------------------------------------------------
//
TBool CAspFilterDialog::OkToExitL(TInt aButtonId)
	{
	if (aButtonId == EEikBidCancel)
		{
		// save silently and quit application (= return ETrue)
		TRAP_IGNORE(SaveSettingsL());
	
		return ETrue;
		}

	if ( aButtonId == EAknSoftkeyBack )
        {
		TInt index = CheckMandatoryFieldsL();
		if (index != KErrNotFound)
			{
			if (!TDialogUtil::ShowConfirmationQueryL(R_ASP_EXIT_ANYWAY))
				{
				ListBox()->SetCurrentItemIndexAndDraw(index);
				return EFalse; // leave dialog open
				}
			}
		
		SaveSettingsL();
		return ETrue;
		}

	return CAknDialog::OkToExitL(aButtonId);
	}


//------------------------------------------------------------------------------
// CAspFilterDialog::OfferKeyEventL
//
// Handle key events.
//------------------------------------------------------------------------------
//
TKeyResponse CAspFilterDialog::OfferKeyEventL(const TKeyEvent& aKeyEvent,TEventCode aType)
	{

	if (aType == EEventKey)
		{
		switch (aKeyEvent.iCode)
			{
			case EKeyEscape:  // framework calls this when dialog must shut down
				{
				return CAknDialog::OfferKeyEventL(aKeyEvent, aType);
        		}

			case EKeyOK: // OK button
				{
				CAspListItemData* item = ItemForIndex(ListBox()->CurrentItemIndex());
			    item->iSelectKeyPressed = ETrue;

				HandleOKL();
				return EKeyWasConsumed;
				}
			}
		}

	return CAknDialog::OfferKeyEventL(aKeyEvent, aType);
	}


//------------------------------------------------------------------------------
// CAspFilterDialog::PreLayoutDynInitL
//
// Called by system before the dialog is shown.
//------------------------------------------------------------------------------
//
void CAspFilterDialog::PreLayoutDynInitL()
	{
	iListBox = (CAknSettingStyleListBox*)Control(EAspFilterDialog);
	
	ListBox()->SetListBoxObserver(this);
	ListBox()->CreateScrollBarFrameL( ETrue );
	ListBox()->ScrollBarFrame()->SetScrollBarVisibilityL( CEikScrollBarFrame::EOn, CEikScrollBarFrame::EAuto ); 
	
    CreateSettingsListL();
    UpdateListBoxL(ListBox(), iSettingList);
	
	//Set title pane text
	iStatusPaneHandler->SetTitleL(iFilter->DisplayName());
	}



//------------------------------------------------------------------------------
// CAspFilterDialog::DynInitMenuPaneL
//
// Called by system before menu is shown.
//------------------------------------------------------------------------------
//
void CAspFilterDialog::DynInitMenuPaneL( TInt aResourceID, CEikMenuPane* aMenuPane)
	{
    if (aResourceID != R_ASP_FILTER_DIALOG_MENU_PANE)
		{
		return;
		}

	if (ListBox()->Model()->NumberOfItems() == 0)
		{
		TDialogUtil::DeleteMenuItem(aMenuPane, EAspMenuCmdChange);
		}
	}


//------------------------------------------------------------------------------
// CAspFilterDialog::HandleListBoxEventL
//
// Handle listbox events.
//------------------------------------------------------------------------------
//
void CAspFilterDialog::HandleListBoxEventL(CEikListBox* /*aListBox*/, TListBoxEvent aEventType)
	{
	if (aEventType == EEventEnterKeyPressed || aEventType == EEventItemSingleClicked)
		{
		HandleOKL();
		}
	}


// ----------------------------------------------------------------------------
// CAspFilterDialog::UpdateListBoxL
// 
// Add settings headers into listbox.
// ----------------------------------------------------------------------------
//
void CAspFilterDialog::UpdateListBoxL(CEikTextListBox* aListBox, CAspSettingList* aItemList)
	{
	CDesCArray* arr = (CDesCArray*)aListBox->Model()->ItemTextArray();
	arr->Reset();

	TInt count = aItemList->Count();
	for (TInt index=0; index<count; index++ )
		{
	   	TBool convert = ETrue;

		CAspListItemData* item = (*aItemList)[index];
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


// ----------------------------------------------------------------------------
// CAspFilterDialog::CreateSettingsListL
// 
// Function creates setting list array (iSettingList).
// ----------------------------------------------------------------------------
//
void CAspFilterDialog::CreateSettingsListL()
	{
    TInt count = iFilter->ItemCount();	
	for (TInt i=0; i<count; i++)
		{
		CAspFilterItem* item = iFilter->Item(i);
		AddItemL(item);
		}
		
	// write setting data into each CAspListItemData
	count=iSettingList->Count();
	for (TInt i=0; i<count; i++)
		{
		CAspListItemData* item = (*iSettingList)[i];
		item->iFilter = iFilter;
		
		InitSettingItemL(item);
		}

	SetVisibility();  // find out what setting appear on UI
	}


// ----------------------------------------------------------------------------
// CAspFilterDialog::AddItemL
// 
// ----------------------------------------------------------------------------
//
void CAspFilterDialog::AddItemL(CAspFilterItem* aFilterItem)
	{
    CAspListItemData* item = CAspListItemData::NewLC();
	item->SetHeaderL(aFilterItem->DisplayName());
	item->iItemId =aFilterItem->Id();
	item->iItemType = aFilterItem->DataType();
	item->iMandatory = aFilterItem->IsMandatory();
	
	if (item->iMandatory)
		{
		item->SetDisplayValueL(R_ASP_MUST_BE_DEFINED);
		}
	else
		{
		item->SetDisplayValueL(R_ASP_SETTING_VALUE_NONE);
		}
		
	iSettingList->AppendL(item);
	CleanupStack::Pop(item);
	}


// -----------------------------------------------------------------------------
// CAspFilterDialog::InitSettingItemL
// 
// -----------------------------------------------------------------------------
//
void CAspFilterDialog::InitSettingItemL(CAspListItemData* aItem)
	{
	__ASSERT_ALWAYS(aItem->iFilter, TUtil::Panic(KErrGeneral));
	
	CAspFilterItem* filterItem = aItem->iFilter->ItemForId(aItem->iItemId);
	
	if (filterItem->ListType() != CAspFilterItem::ETypeQuery)
		{
		aItem->SetValueL(filterItem->DisplayValueL());
		return; // this filter item is list, not a single value
		}
	
	
	HBufC* hBuf = HBufC::NewLC(KBufSize255);
	TPtr ptr = hBuf->Des();
	
    	
	switch (aItem->iItemType)
		{
		case CAspListItemData::ETypeNumber:
			{
			aItem->SetValueL(filterItem->IntL());
		    aItem->iNumberData = filterItem->IntL();
            break;
			}

		case CAspListItemData::ETypeText:
			{
			filterItem->GetText(ptr);
   			aItem->SetValueL(ptr);
    		break;
			}

		case CAspListItemData::ETypeBoolean:
			{
			CDesCArray* stringData = CAspResHandler::ReadDesArrayStaticLC(R_ASP_FILTER_ITEM_YESNO);
			
			aItem->iNumberData = EAspSettingEnabled;      // 1;
			if (!filterItem->BoolL())
				{
				aItem->iNumberData = EAspSettingDisabled; // 0;
				}
			
         	aItem->SetValueL((*stringData)[aItem->iNumberData]);
         	aItem->iResource = R_ASP_FILTER_ITEM_YESNO;
         	
         	CleanupStack::PopAndDestroy(stringData);
			break;
     		}

		case CAspListItemData::ETypeTime:
			{
    		aItem->iTime = filterItem->TimeL();
    		if (aItem->iTime > 0)
    			{
    			TUtil::GetTimeTextL(ptr, aItem->iTime);
    		    aItem->SetValueL(ptr);
    			}
	
			break;
			}

		case CAspListItemData::ETypeDate:
			{
    		aItem->iDate = filterItem->DateL();
    		if (aItem->iDate > 0)
    			{
    			TUtil::GetDateTextL(ptr, aItem->iDate);
    		    aItem->SetValueL(ptr);
    			}
	
			break;
			}

        default:
			TUtil::Panic(KErrArgument);
			break;
		
		}
		
	CleanupStack::PopAndDestroy(hBuf);
	}


// -----------------------------------------------------------------------------
// CAspFilterDialog::SetVisibility
// 
// -----------------------------------------------------------------------------
//
void CAspFilterDialog::SetVisibility()
	{
	}


//------------------------------------------------------------------------------
// CAspFilterDialog::SaveSettingsL
//
//------------------------------------------------------------------------------
//
void CAspFilterDialog::SaveSettingsL()
	{
    iFilter->SaveL();
	}


//------------------------------------------------------------------------------
// CAspFilterDialog::CheckMandatoryFieldsL
//
// Check that mandatory fields are filled correctly.
//------------------------------------------------------------------------------
//
TInt CAspFilterDialog::CheckMandatoryFieldsL()
	{ 
    TInt count = iSettingList->Count();
	for (TInt i=0; i<count; i++)
		{
		CAspListItemData* item = (*iSettingList)[i];
		if (item->iMandatory && item->IsEmpty())
			{
			return item->iIndex;
			}
		}
		
	return KErrNotFound;
	}


//------------------------------------------------------------------------------
// CAspFilterDialog::EditFilterL
//
// NOTE: Function is not used by CAspFilterDialog. It can be called from
// other dialogs in case filter only contains one filter item.
//------------------------------------------------------------------------------
//
TBool CAspFilterDialog::EditFilterL(CAspListItemData& aItem)
	{
    __ASSERT_ALWAYS(aItem.iFilter, TUtil::Panic(KErrGeneral));
	
	if (aItem.iFilter->ItemCount() != 1)
		{
		return EFalse; // this functions edits filter with one filter item
		}
		
	CAspFilterItem* filterItem = aItem.iFilter->Item(0);
		
	__ASSERT_ALWAYS(filterItem, TUtil::Panic(KErrGeneral));
	
	
	CAspListItemData* item = CAspListItemData::NewLC();
	
	item->SetHeaderL(filterItem->DisplayName());
	item->iItemId = filterItem->Id();
	item->iItemType = filterItem->DataType();
	item->iMandatory = filterItem->IsMandatory();
	
	item->iSelectKeyPressed = aItem.iSelectKeyPressed;
	item->iFilter = aItem.iFilter;

	if (item->iMandatory)
		{
		item->SetDisplayValueL(R_ASP_MUST_BE_DEFINED);
		}
	else
		{
		item->SetDisplayValueL(R_ASP_SETTING_VALUE_NONE);
		}
	
	InitSettingItemL(item);
	
	TBool ret = EditSettingItemL(*item);
	
    CleanupStack::PopAndDestroy(item);
	return ret;
	}


//------------------------------------------------------------------------------
// CAspFilterDialog::EditSettingItemL
//
// Calls setting editing functions. 
//------------------------------------------------------------------------------
//
TBool CAspFilterDialog::EditSettingItemL(CAspListItemData& aItem)
	{
	__ASSERT_ALWAYS(aItem.iFilter, TUtil::Panic(KErrGeneral));
	
	TBool ret = EFalse;

	CAspFilterItem* filterItem = aItem.iFilter->ItemForId(aItem.iItemId);
    TInt type = filterItem->ListType();
    
	switch (type)
		{
		case CAspFilterItem::ETypeQuery:
			ret = EditSettingItemQueryL(aItem);
		    break;

		case CAspFilterItem::ETypeSingleSelection:
			ret = EditSingleChoiceListL(aItem);
		    break;
		
		case CAspFilterItem::ETypeSingleSelectionUserDefined:
			ret = EditSingleChoiceListL(aItem);
		    break;

		case CAspFilterItem::ETypeMultipleSelection:
		    ret = EditMultiChoiceListL(aItem);
		   	break;

		case CAspFilterItem::ETypeMultipleSelectionUserDefined:
		    ret = EditMultiChoiceListL(aItem);
		   	break;

		default:
		    break;

		}

	return ret;
	}


//------------------------------------------------------------------------------
// CAspFilterDialog::EditSettingItemQueryL
//
//------------------------------------------------------------------------------
//
TBool CAspFilterDialog::EditSettingItemQueryL(CAspListItemData& aItem)
	{
	TBool ret = EFalse;
	
	switch (aItem.iItemType)
		{
		case CAspListItemData::ETypeText:
			ret = EditSettingItemTextL(aItem);
		    break;

		case CAspListItemData::ETypeNumber:
			ret = EditSettingItemNumberL(aItem);
		    break;
		
		case CAspListItemData::ETypeBoolean:
		    if (aItem.iSelectKeyPressed)
		    	{
		    	ret = EditSettingItemBooleanL(aItem);
		    	}
		    else
		    	{
		    	ret = EditSettingItemListL(aItem);
		    	}
		    break;

		case CAspListItemData::ETypeTime:
		    ret = EditSettingItemTimeL(aItem);
		   	break;

		case CAspListItemData::ETypeDate:
		    ret = EditSettingItemDateL(aItem);
		   	break;

		default:
		    break;

		}

	return ret;
	}


//------------------------------------------------------------------------------
// CAspFilterDialog::EditSingleChoiceListL
//
//------------------------------------------------------------------------------
//
TBool CAspFilterDialog::EditSingleChoiceListL(CAspListItemData& aItem)
	{
	CAspSelectionItemList*	list = new (ELeave) CAspSelectionItemList(1);
	CleanupStack::PushL(TCleanupItem(CAspSelectionItemList::Cleanup, list));

	CAspFilterItem* filterItem = (aItem.iFilter)->ItemForId(aItem.iItemId);
	TInt listType = filterItem->ListType();
	TInt dataType = filterItem->DataType();
	
	TInt type = CAspChoiceList::ETypeNormal;
	if (listType == CAspFilterItem::ETypeSingleSelectionUserDefined &&
	    dataType == CAspListItemData::ETypeNumber)
		{
		type = CAspChoiceList::ETypeIntegerUserDefined;
		}
	
	if (listType == CAspFilterItem::ETypeSingleSelectionUserDefined &&
	    dataType == CAspListItemData::ETypeText)
		{
		type = CAspChoiceList::ETypeTextUserDefined;
		}
	
	filterItem->GetSelectionL(list);
	
	CAspChoiceList* editor = CAspChoiceList::NewLC(filterItem->DisplayName(), list, type, filterItem->MaxLength());
	
	TBool ret = editor->ShowListL();
	if (ret)
		{
		filterItem->SetSelectionL(list);
		aItem.SetValueL(filterItem->DisplayValueL());
		}
		
	
	CleanupStack::PopAndDestroy(editor);
	CleanupStack::PopAndDestroy(list);
	
	return ret;
	}


//------------------------------------------------------------------------------
// CAspFilterDialog::EditMultiChoiceListL
//
//------------------------------------------------------------------------------
//
TBool CAspFilterDialog::EditMultiChoiceListL(CAspListItemData& aItem)
	{
	CAspSelectionItemList*	list = new (ELeave) CAspSelectionItemList(1);
	CleanupStack::PushL(TCleanupItem(CAspSelectionItemList::Cleanup, list));
	
	CAspFilterItem* filterItem = aItem.iFilter->ItemForId(aItem.iItemId);
	TInt listType = filterItem->ListType();
	TInt dataType = filterItem->DataType();
	
	TInt type = CAspChoiceList::ETypeNormal;
	if (listType == CAspFilterItem::ETypeMultipleSelectionUserDefined &&
	    dataType == CAspListItemData::ETypeNumber)
		{
		type = CAspChoiceList::ETypeIntegerUserDefined;
		}
	if (listType == CAspFilterItem::ETypeMultipleSelectionUserDefined &&
	    dataType == CAspListItemData::ETypeText)
		{
		type = CAspChoiceList::ETypeTextUserDefined;
		}
	
	filterItem->GetSelectionL(list);
	
	CAspMultiChoiceList* editor = CAspMultiChoiceList::NewLC(filterItem->DisplayName(), list, type, filterItem->MaxLength());
	
	TBool ret = editor->ShowListL();
	if (ret)
		{
		filterItem->SetSelectionL(list);
		aItem.SetValueL(filterItem->DisplayValueL());
		}
	
	
	CleanupStack::PopAndDestroy(editor);
	CleanupStack::PopAndDestroy(list);
	
	return ret;
	}


//------------------------------------------------------------------------------
// CAspFilterDialog::EditSettingItemTextL
//
//------------------------------------------------------------------------------
//
TBool CAspFilterDialog::EditSettingItemTextL(CAspListItemData& aItem)
	{
	CAspFilterItem* filterItem = aItem.iFilter->ItemForId(aItem.iItemId);
	
	HBufC* hBuf = HBufC::NewLC(KBufSize255);
	TPtr ptr = hBuf->Des();
	
	TUtil::StrCopy(ptr, aItem.Value());
	
	TBool ret = TDialogUtil::ShowTextEditorL(ptr, aItem.Header(), aItem.iMandatory, aItem.iLatinInput, filterItem->MaxLength());
	if (ret)
		{
		aItem.SetValueL(ptr);
		filterItem->SetTextL(ptr);
		}
		
	CleanupStack::PopAndDestroy(hBuf);
	return ret;
	}


//------------------------------------------------------------------------------
// CAspFilterDialog::EditSettingItemListL
//
//------------------------------------------------------------------------------
//
TBool CAspFilterDialog::EditSettingItemListL(CAspListItemData& aItem)
	{
	CAspFilterItem* filterItem = aItem.iFilter->ItemForId(aItem.iItemId);
	
	TInt curSelection = aItem.iNumberData;
	CDesCArray* stringData = CAspResHandler::ReadDesArrayStaticLC(aItem.iResource);
	
	TBool ret = TDialogUtil::ShowListEditorL(stringData, aItem.Header(), curSelection);
	if (ret)
		{
		aItem.iNumberData = curSelection; 
		aItem.SetValueL((*stringData)[curSelection]);
		filterItem->SetIntL(aItem.iNumberData);
		}

	CleanupStack::PopAndDestroy(stringData);
	return ret;
	}


//------------------------------------------------------------------------------
// CAspFilterDialog::EditSettingItemBooleanL
//
// Change Yes/No value straight if ok key is pressed
//------------------------------------------------------------------------------
//
TBool CAspFilterDialog::EditSettingItemBooleanL(CAspListItemData& aItem)
	{
	CAspFilterItem* filterItem = aItem.iFilter->ItemForId(aItem.iItemId);
		
	CDesCArray* stringData = CAspResHandler::ReadDesArrayStaticLC(R_ASP_FILTER_ITEM_YESNO);
	
	if (aItem.iNumberData == EAspSettingDisabled)
		{
		aItem.iNumberData = EAspSettingEnabled;
		}
	else
		{
		aItem.iNumberData = EAspSettingDisabled;
		}
	
	aItem.SetValueL( (*stringData)[aItem.iNumberData] );
	filterItem->SetIntL(aItem.iNumberData);

	CleanupStack::PopAndDestroy(stringData);
	return ETrue;
	}


//------------------------------------------------------------------------------
// CAspFilterDialog::EditSettingItemNumberL
//
//------------------------------------------------------------------------------
//
TBool CAspFilterDialog::EditSettingItemNumberL(CAspListItemData& aItem)
	{
	CAspFilterItem* filterItem = aItem.iFilter->ItemForId(aItem.iItemId);
		
	TBool ret = TDialogUtil::ShowIntegerEditorL(aItem.iNumberData, aItem.Header(), aItem.iMinValue, aItem.iMaxValue);

	if (ret)
		{
		aItem.SetValueL(aItem.iNumberData);
		filterItem->SetIntL(aItem.iNumberData);
		}

	return ret;
	}


//------------------------------------------------------------------------------
// CAspFilterDialog::EditSettingItemDateL
//
//------------------------------------------------------------------------------
//
TBool CAspFilterDialog::EditSettingItemDateL(CAspListItemData& aItem)
	{
	CAspFilterItem* filterItem = aItem.iFilter->ItemForId(aItem.iItemId);
	
	TBool ret = TDialogUtil::ShowDateEditorL(aItem.iDate, aItem.Header());

	if (ret)
		{
		TBuf<KBufSize> buf;
		TUtil::GetDateTextL(buf, aItem.iDate);
	    aItem.SetValueL(buf);
        filterItem->SetDateL(aItem.iDate);
		}

	return ret;
	}


//------------------------------------------------------------------------------
// CAspFilterDialog::EditSettingItemTimeL
//
//------------------------------------------------------------------------------
//
TBool CAspFilterDialog::EditSettingItemTimeL(CAspListItemData& aItem)
	{
	CAspFilterItem* filterItem = aItem.iFilter->ItemForId(aItem.iItemId);
	
	TBool ret = TDialogUtil::ShowTimeEditorL(aItem.iTime, aItem.Header());

	if (ret)
		{
		TBuf<KBufSize> buf;
		TUtil::GetTimeTextL(buf, aItem.iTime);
	    aItem.SetValueL(buf);
        filterItem->SetTimeL(aItem.iTime);
		}

	return ret;
	}


//-----------------------------------------------------------------------------
// CAspFilterDialog::ItemForIndex
// 
// Find item in list position aIndex.
//-----------------------------------------------------------------------------
//
CAspListItemData* CAspFilterDialog::ItemForIndex(TInt aIndex)
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
// CAspFilterDialog::Item
// 
// Find item with aItemId (TAspConnectionSettingItem).
//-----------------------------------------------------------------------------
//
CAspListItemData* CAspFilterDialog::Item(TInt aItemId)
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
// CAspFilterDialog::ResHandler
// 
// -----------------------------------------------------------------------------
//
CAspResHandler* CAspFilterDialog::ResHandler()
	{
	__ASSERT_DEBUG(iResHandler, TUtil::Panic(KErrGeneral));

	return iResHandler;
	}


// -----------------------------------------------------------------------------
// CAspFilterDialog::ListBox
// 
// -----------------------------------------------------------------------------
//
CAknSettingStyleListBox* CAspFilterDialog::ListBox()
	{
    __ASSERT_DEBUG(iListBox, TUtil::Panic(KErrGeneral));	
	
	return iListBox;
	}


//------------------------------------------------------------------------------
// CAspFilterDialog::GetHelpContext
//
//------------------------------------------------------------------------------
//
void CAspFilterDialog::GetHelpContext(TCoeHelpContext& aContext) const
	{
	aContext.iMajor=KUidSmlSyncApp;
	//aContext.iContext=KDS_HLP_APPLICATION;
	}






/*******************************************************************************
 * class CAspMultiChoiceListSettingPage
 *******************************************************************************/


//------------------------------------------------------------------------------
// CAspMultiChoiceListSettingPage::ProcessCommandL
// 
//------------------------------------------------------------------------------
//
void CAspMultiChoiceListSettingPage::ProcessCommandL(TInt aCommandId)
	{
		
	if (iType == ETypeUserDefined && aCommandId == EAknSoftkeySelect)
		{
		CAknSetStyleListBox* listBox = ListBoxControl();
		TInt bottomIndex = listBox->BottomItemIndex();
		TInt currentIndex = listBox->CurrentItemIndex();
		
		TBool itemChecked = (*iSelectionList)[currentIndex]->SelectionStatus();
		
		if (bottomIndex == currentIndex && !itemChecked) //  "User defined" has focus and is checked
			{
			
			if (!iObserver->HandleChoiceListEventL(0, 0))
				{
				return;  // leave editor open if user defined editor is canceled
				}
			}
		}
		
	CAknCheckBoxSettingPage::ProcessCommandL(aCommandId);
	}


//------------------------------------------------------------------------------
// CAspMultiChoiceListSettingPage::CAspMultiChoiceListSettingPage
// 
//------------------------------------------------------------------------------
//
CAspMultiChoiceListSettingPage::CAspMultiChoiceListSettingPage(TInt aResourceID, CSelectionItemList* aItemArray,
                                                               MAspChoiceListObserver* aObserver, TInt aType)
 : CAknCheckBoxSettingPage(aResourceID, aItemArray)
	{
	iObserver = aObserver;
	iType = aType;
	iSelectionList = aItemArray;
	}


//------------------------------------------------------------------------------
// Destructor
// 
//------------------------------------------------------------------------------
//
CAspMultiChoiceListSettingPage::~CAspMultiChoiceListSettingPage()
	{
	}




/*******************************************************************************
 * class CAspChoiceListSettingPage
 *******************************************************************************/


//------------------------------------------------------------------------------
// CAspChoiceListSettingPage::ProcessCommandL
// 
//------------------------------------------------------------------------------
//
void CAspChoiceListSettingPage::ProcessCommandL(TInt aCommandId)
	{
		
	if (iType == ETypeUserDefined && aCommandId == EAknSoftkeySelect)
		{
		CAknSetStyleListBox* listBox = ListBoxControl();
		TInt num1 = listBox->BottomItemIndex();
		TInt num2 = listBox->CurrentItemIndex();
		
		if (num1 == num2)
			{
			if (!iObserver->HandleChoiceListEventL(0, 0))
				{
				return;  // leave editor open if user defined editor is canceled
				}
			}
		}
		
	CAknRadioButtonSettingPage::ProcessCommandL(aCommandId);
	}


//------------------------------------------------------------------------------
// CAspChoiceListSettingPage::CAspChoiceListSettingPage
// 
//------------------------------------------------------------------------------
//
CAspChoiceListSettingPage::CAspChoiceListSettingPage(TInt aResourceID, TInt& aCurrentSelectionIndex, 
                                                     const MDesCArray* aItemArray,
                                                     MAspChoiceListObserver* aObserver, TInt aType)
 : CAknRadioButtonSettingPage(aResourceID, aCurrentSelectionIndex, aItemArray)
	{
	iObserver = aObserver;
	iType = aType;
	}


//------------------------------------------------------------------------------
// Destructor
// 
//------------------------------------------------------------------------------
//
CAspChoiceListSettingPage::~CAspChoiceListSettingPage()
	{
	}




	
/*******************************************************************************
 * class CAspChoiceList
 *******************************************************************************/
	
	
	
// -----------------------------------------------------------------------------
// CAspChoiceList::NewLC
//
// -----------------------------------------------------------------------------
CAspChoiceList* CAspChoiceList::NewLC(const TDesC& aTitle, CSelectionItemList* aList, TInt aType, TInt aMaxLength)
    {
    CAspChoiceList* self = new (ELeave) CAspChoiceList(aTitle, aList, aType, aMaxLength);
    CleanupStack::PushL(self);
    self->ConstructL();

    return(self);
    }


//------------------------------------------------------------------------------
// CAspChoiceList::CAspChoiceList
// 
//------------------------------------------------------------------------------
//
CAspChoiceList::CAspChoiceList(const TDesC& aTitle, CSelectionItemList* aList, TInt aType, TInt aMaxLength)
	{
	__ASSERT_DEBUG(aList, TUtil::Panic(KErrGeneral));
	
	TUtil::StrCopy(iTitle, aTitle);
	iSelectionList = aList;
	iListType = aType;
	iMaxTextLength = aMaxLength;
	}


//------------------------------------------------------------------------------
// Destructor
// 
//------------------------------------------------------------------------------
//
CAspChoiceList::~CAspChoiceList()
	{
	}

	
//------------------------------------------------------------------------------
// CAspChoiceList::ConstructL
// 
//------------------------------------------------------------------------------
//
void CAspChoiceList::ConstructL()
	{
	}


//------------------------------------------------------------------------------
// CAspChoiceList::ShowListL
// 
//------------------------------------------------------------------------------
//
TBool CAspChoiceList::ShowListL()
	{
	TInt current = SelectedIndex(iSelectionList);
	TInt listType = CAspChoiceListSettingPage::ETypeNormal;
	if (iListType != CAspChoiceList::ETypeNormal)
		{
		listType = CAspChoiceListSettingPage::ETypeUserDefined;
		AddUserDefinedL(); // replace last item text as "User defined"
		}
		
	CDesCArray* arr = DesArrayLC(iSelectionList);

	CAspChoiceListSettingPage* dlg = new (ELeave) CAspChoiceListSettingPage(
	                                       R_ASP_SETTING_RADIO, current, arr, this, listType);
	CleanupStack::PushL(dlg);
	dlg->SetSettingTextL(iTitle); 
	CleanupStack::Pop(dlg);

	TBool ret = dlg->ExecuteLD();
	
	SetSelectedIndex(iSelectionList, current);
	if (iListType != CAspChoiceList::ETypeNormal)
		{
		RemoveUserDefinedL(); // restore original last item text
		}
	
	CleanupStack::PopAndDestroy(arr);
	return ret;
	}


//------------------------------------------------------------------------------
// CAspChoiceList::DesArrayLC
// 
//------------------------------------------------------------------------------
//
CDesCArray* CAspChoiceList::DesArrayLC(CSelectionItemList* aList)
	{
	CDesCArray* arr = new (ELeave) CDesCArrayFlat(5);
	CleanupStack::PushL(arr);
	
	TInt count = aList->Count();
	for (TInt i=0; i<count; i++)
		{
		CSelectableItem* item = (*aList)[i];
		TBuf<128> buf(item->ItemText());
		arr->AppendL(item->ItemText());
		}
		
	return arr;
	}


//------------------------------------------------------------------------------
// CAspChoiceList::SelectedIndex
// 
//------------------------------------------------------------------------------
//
TInt CAspChoiceList::SelectedIndex(CSelectionItemList* aList)
	{
    TInt count = aList->Count();
	for (TInt i=0; i<count; i++)
		{
		CSelectableItem* item = (*aList)[count-1];
		if (item->SelectionStatus())
			{
			return i;
			}
		}
		
	return KErrNotFound;
	}


//------------------------------------------------------------------------------
// CAspChoiceList::SetSelectedIndex
// 
//------------------------------------------------------------------------------
//
void CAspChoiceList::SetSelectedIndex(CSelectionItemList* aList, TInt aIndex)
	{
    TInt count = aList->Count();
    
    __ASSERT_DEBUG(aIndex<count, TUtil::Panic(KErrGeneral));
    
	for (TInt i=0; i<count; i++)
		{
		CSelectableItem* item = (*aList)[i];
		if (i == aIndex)
			{
			item->SetSelectionStatus(ETrue);
			}
		else
			{
			item->SetSelectionStatus(EFalse);
			}
		}
	}


//------------------------------------------------------------------------------
// CAspChoiceList::AddItemL
// 
//------------------------------------------------------------------------------
//
void CAspChoiceList::AddItemL(CSelectionItemList* aList, TDes& aText, TBool aEnable)
	{
	CSelectableItem* item = new (ELeave) CSelectableItem(aText, aEnable);
   	CleanupStack::PushL(item);
	item->ConstructL();
	aList->AppendL(item);
	CleanupStack::Pop(item);
	}
	

//------------------------------------------------------------------------------
// CAspChoiceList::AddUserDefined
// 
//------------------------------------------------------------------------------
//
void CAspChoiceList::AddUserDefinedL()
	{
	TInt count = iSelectionList->Count();
	CSelectableItem* item = (*iSelectionList)[count-1];
	
	TBool selected = item->SelectionStatus();
	iUserDefinedText = item->ItemText();
	if (iListType == CAspChoiceList::ETypeIntegerUserDefined)
		{
		User::LeaveIfError(TUtil::StrToInt(iUserDefinedText, iUserDefinedInt));
		}

	
	delete item;
	iSelectionList->Delete(count-1);
	iSelectionList->Compress();

	TBuf<KBufSize> buf;
	CAspResHandler::ReadL(buf, R_ASP_LIST_USER_DEFINED);
	
	
	AddItemL(iSelectionList, buf, selected);
	}


//------------------------------------------------------------------------------
// CAspChoiceList::RemoveUserDefined
// 
//------------------------------------------------------------------------------
//
void CAspChoiceList::RemoveUserDefinedL()
	{
	TInt count = iSelectionList->Count();
	CSelectableItem* item = (*iSelectionList)[count-1];
    TBool selected = item->SelectionStatus();

	delete item;
	iSelectionList->Delete(count-1);
	iSelectionList->Compress();

	if (iListType == CAspChoiceList::ETypeIntegerUserDefined)
		{
		iUserDefinedText.Num(iUserDefinedInt);
		}
	AddItemL(iSelectionList, iUserDefinedText, selected);
	}


//------------------------------------------------------------------------------
// CAspChoiceList::EditTextL
//
//------------------------------------------------------------------------------
//
TBool CAspChoiceList::EditTextL(TDes& aText, const TDesC& aTitle)
	{
	TBool mandatory = EFalse;
	TBool latinInput = ETrue;
	TInt maxLength = iMaxTextLength;
	
	return TDialogUtil::ShowTextEditorL(aText, aTitle, mandatory, latinInput, maxLength);
	}


//------------------------------------------------------------------------------
// CAspChoiceList::HandleChoiceListEventL
// 
//------------------------------------------------------------------------------
//
TBool CAspChoiceList::HandleChoiceListEventL(TInt /*aEvent*/, TInt /*aListType*/)
	{
	if (iListType == CAspChoiceList::ETypeIntegerUserDefined)
		{
		return TDialogUtil::ShowIntegerEditorL(iUserDefinedInt, iTitle, KErrNotFound, KErrNotFound);
		}
	else if (iListType == CAspChoiceList::ETypeTextUserDefined)
		{
		return EditTextL(iUserDefinedText, iTitle);
		}
		
	return EFalse;
	}

	
	
	
/*******************************************************************************
 * class CAspMultiChoiceList
 *******************************************************************************/
	
	
// -----------------------------------------------------------------------------
// CAspMultiChoiceList::NewLC
//
// -----------------------------------------------------------------------------
CAspMultiChoiceList* CAspMultiChoiceList::NewLC(const TDesC& aTitle, CSelectionItemList* aList, TInt aType, TInt aMaxLength)
    {
    CAspMultiChoiceList* self = new (ELeave) CAspMultiChoiceList(aTitle, aList, aType, aMaxLength);
    CleanupStack::PushL(self);
    self->ConstructL();

    return(self);
    }


//------------------------------------------------------------------------------
// CAspMultiChoiceList::CAspMultiChoiceList
// 
//------------------------------------------------------------------------------
//
CAspMultiChoiceList::CAspMultiChoiceList(const TDesC& aTitle, CSelectionItemList* aList, TInt aType, TInt aMaxLength)
	{
	__ASSERT_DEBUG(aList, TUtil::Panic(KErrGeneral));
	
	TUtil::StrCopy(iTitle, aTitle);
	iSelectionList = aList;
	iListType = aType;
	iMaxTextLength = aMaxLength;
	}


//------------------------------------------------------------------------------
// Destructor
// 
//------------------------------------------------------------------------------
//
CAspMultiChoiceList::~CAspMultiChoiceList()
	{
	}
	
	
//------------------------------------------------------------------------------
// CAspMultiChoiceList::ConstructL
// 
//------------------------------------------------------------------------------
//
void CAspMultiChoiceList::ConstructL()
	{
	}


//------------------------------------------------------------------------------
// CAspMultiChoiceList::ShowListL
// 
//------------------------------------------------------------------------------
//
TBool CAspMultiChoiceList::ShowListL()
	{
	TInt listType = CAspChoiceListSettingPage::ETypeNormal;
	if (iListType != CAspChoiceList::ETypeNormal)
		{
		listType = CAspChoiceListSettingPage::ETypeUserDefined;
		AddUserDefinedL(); // replace last item text as "User defined"
		}
	

	CAspMultiChoiceListSettingPage* dlg = new (ELeave) CAspMultiChoiceListSettingPage(
	                                       R_ASP_SETTING_RADIO, iSelectionList, this, listType);
	CleanupStack::PushL(dlg);
	dlg->SetSettingTextL(iTitle); 
	CleanupStack::Pop(dlg);

	TBool ret = dlg->ExecuteLD();
	
	if (iListType != CAspChoiceList::ETypeNormal)
		{
		RemoveUserDefinedL(); // set original last item text
		}
	
	return ret;
	}


//------------------------------------------------------------------------------
// CAspMultiChoiceList::AddItemL
// 
//------------------------------------------------------------------------------
//
void CAspMultiChoiceList::AddItemL(CSelectionItemList* aList, TDes& aText, TBool aEnable)
	{
	CSelectableItem* item = new (ELeave) CSelectableItem(aText, aEnable);
   	CleanupStack::PushL(item);
	item->ConstructL();
	aList->AppendL(item);
	CleanupStack::Pop(item);
	}


//------------------------------------------------------------------------------
// CAspMultiChoiceList::AddItemL
// 
//------------------------------------------------------------------------------
//
void CAspMultiChoiceList::AddItemL(TDes& aText, TBool aEnable)
	{
	AddItemL(iSelectionList, aText, aEnable);
	}
	
	
//------------------------------------------------------------------------------
// CAspMultiChoiceList::AddItemL
// 
//------------------------------------------------------------------------------
//
void CAspMultiChoiceList::AddItemL(TInt aNumber, TBool aEnable)
	{
	TBuf<KBufSize> buf;
	
	buf.Num(aNumber);
	AddItemL(iSelectionList, buf, aEnable);
	}


//------------------------------------------------------------------------------
// CAspMultiChoiceList::AddUserDefined
// 
//------------------------------------------------------------------------------
//
void CAspMultiChoiceList::AddUserDefinedL()
	{
	TInt count = iSelectionList->Count();
	CSelectableItem* item = (*iSelectionList)[count-1];
	
	TBool selected = item->SelectionStatus();
	iUserDefinedText = item->ItemText();
	if (iListType == CAspChoiceList::ETypeIntegerUserDefined)
		{
		User::LeaveIfError(TUtil::StrToInt(iUserDefinedText, iUserDefinedInt));
		}

	
	delete item;
	iSelectionList->Delete(count-1);
	iSelectionList->Compress();

	TBuf<KBufSize> buf;
	CAspResHandler::ReadL(buf, R_ASP_LIST_USER_DEFINED);
	
	
	AddItemL(iSelectionList, buf, selected);
	}


//------------------------------------------------------------------------------
// CAspMultiChoiceList::RemoveUserDefined
// 
//------------------------------------------------------------------------------
//
void CAspMultiChoiceList::RemoveUserDefinedL()
	{
	TInt count = iSelectionList->Count();
	CSelectableItem* item = (*iSelectionList)[count-1];
    TBool selected = item->SelectionStatus();

	delete item;
	iSelectionList->Delete(count-1);
	iSelectionList->Compress();

	if (iListType == CAspChoiceList::ETypeIntegerUserDefined)
		{
		iUserDefinedText.Num(iUserDefinedInt);
		}
	AddItemL(iSelectionList, iUserDefinedText, selected);
	}


//------------------------------------------------------------------------------
// CAspMultiChoiceList::EditTextL
//
//------------------------------------------------------------------------------
//
TBool CAspMultiChoiceList::EditTextL(TDes& aText, const TDesC& aTitle)
	{
	TBool mandatory = ETrue;
	TBool latinInput = ETrue;
	TInt maxLength = iMaxTextLength;
	
	return TDialogUtil::ShowTextEditorL(aText, aTitle, mandatory, latinInput, maxLength);
	}


//------------------------------------------------------------------------------
// CAspMultiChoiceList::HandleChoiceListEventL
// 
//------------------------------------------------------------------------------
//
TBool CAspMultiChoiceList::HandleChoiceListEventL(TInt /*aEvent*/, TInt /*aListType*/)
	{
	if (iListType == CAspChoiceList::ETypeIntegerUserDefined)
		{
		return TDialogUtil::ShowIntegerEditorL(iUserDefinedInt, iTitle, KErrNotFound, KErrNotFound);
		}
	else if (iListType == CAspChoiceList::ETypeTextUserDefined)
		{
		return EditTextL(iUserDefinedText, iTitle);
		}
		
	return EFalse;
	}



// End of file
