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

#include "AspProfileSelectionDialog.h"
#include "AspSchedule.h"
#include "AspProfileWizard.h"
#include "AspSyncUtil.rh"
#include "AspDebug.h"
#include <csxhelp/ds.hlp.hrh>

#include <AspSyncUtil.mbg>  // for bitmap enumerations
#include <Avkon.mbg>
#include <akniconarray.h>   // for GulArray



// ============================ MEMBER FUNCTIONS ===============================



// -----------------------------------------------------------------------------
// CAspProfileSelectionDialog::ShowDialogL
// 
// -----------------------------------------------------------------------------
TBool CAspProfileSelectionDialog::ShowDialogL(TAspParam& aParam)
	{
	CAspProfileSelectionDialog* dialog = CAspProfileSelectionDialog::NewL(aParam);

	TBool ret = dialog->ExecuteLD(R_ASP_AUTO_SYNC_PROFILE_DIALOG);

    return ret;
	}


// -----------------------------------------------------------------------------
// CAspProfileSelectionDialog::NewL
//
// -----------------------------------------------------------------------------
CAspProfileSelectionDialog* CAspProfileSelectionDialog::NewL(TAspParam& aParam)
    {
    FLOG( _L("CAspProfileSelectionDialog::NewL START") );

    CAspProfileSelectionDialog* self = new (ELeave) CAspProfileSelectionDialog(aParam);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);

    FLOG( _L("CAspProfileSelectionDialog::NewL END") );
    return self;
    }


// -----------------------------------------------------------------------------
// CAspProfileSelectionDialog::CAspProfileSelectionDialog
// 
// -----------------------------------------------------------------------------
//
CAspProfileSelectionDialog::CAspProfileSelectionDialog(TAspParam& aParam)
	{
    iSyncSession = aParam.iSyncSession;
    iApplicationId = aParam.iApplicationId;
    iProfileId = aParam.iProfileId;
    iDialogParam = &aParam;
           
	__ASSERT_ALWAYS(iSyncSession, TUtil::Panic(KErrGeneral));
    }


// -----------------------------------------------------------------------------
// CAspProfileSelectionDialog::ConstructL
//
// -----------------------------------------------------------------------------
//
void CAspProfileSelectionDialog::ConstructL()
    {
    FLOG( _L("CAspProfileSelectionDialog::ConstructL START") );
	
	CAknDialog::ConstructL(R_ASP_AUTO_SYNC_PROFILE_DIALOG_MENU);

	TAspParam param(iApplicationId, iSyncSession);
	CAspProfileList* list = CAspProfileList::NewLC(param);
	list->ReadAllProfilesL(CAspProfileList::ENoMandatoryCheck);
		
    TAspFilterInfo info;
    info.iFilterType = TAspFilterInfo::EIncludeRemoteProfile;
    iProfileList = list->FilteredListL(info);
    iProfileList->Sort();
    CleanupStack::PopAndDestroy(list);
	
    param.iMode = CAspContentList::EInitDataProviders;
	iContentList = CAspContentList::NewL(param);


	// get previous title so it can be restored
	iStatusPaneHandler = CStatusPaneHandler::NewL(iAvkonAppUi);
	iStatusPaneHandler->StoreOriginalTitleL();
	
	iCommandSetId = R_ASP_CBA_OPTIONS_SELECT_BACK;
	
	FLOG( _L("CAspProfileSelectionDialog::ConstructL END") );
    }


// ----------------------------------------------------------------------------
// Destructor
//
// ----------------------------------------------------------------------------
//
CAspProfileSelectionDialog::~CAspProfileSelectionDialog()
    {
    FLOG( _L("CAspProfileSelectionDialog::~CAspProfileSelectionDialog START") );

    delete iProfileList;
    delete iContentList;
	delete iStatusPaneHandler;
	
    if (iAvkonAppUi)
    	{
    	iAvkonAppUi->RemoveFromStack(this);
    	}

	FLOG( _L("CAspProfileSelectionDialog::~CAspProfileSelectionDialog END") );
    }


//------------------------------------------------------------------------------
// CAspProfileSelectionDialog::ActivateL
//
// Called by system when dialog is activated.
//------------------------------------------------------------------------------
//
void CAspProfileSelectionDialog::ActivateL()
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
void CAspProfileSelectionDialog::GetHelpContext(TCoeHelpContext& aContext) const
	{
	aContext.iMajor = KUidSmlSyncApp;
	//aContext.iContext = KDS_HLP_SET_CONN_SETTINGS;
	}


// -----------------------------------------------------------------------------
// CAspProfileSelectionDialog::HandleListBoxEventL
// 
// -----------------------------------------------------------------------------
void CAspProfileSelectionDialog::HandleListBoxEventL(CEikListBox* /*aListBox*/,
                                             TListBoxEvent /*aEventType*/)
	{
	}


// -----------------------------------------------------------------------------
// CAspProfileSelectionDialog::PreLayoutDynInitL
// 
// -----------------------------------------------------------------------------
//
void CAspProfileSelectionDialog::PreLayoutDynInitL()
    {
    iListBox = (CAknSingleGraphicStyleListBox*) (ControlOrNull(EAspAutoSyncProfileDialogList));
    
   	__ASSERT_ALWAYS(iListBox, TUtil::Panic(KErrGeneral));
    
	iListBox->SetListBoxObserver(this);
	iListBox->CreateScrollBarFrameL(ETrue);
	iListBox->ScrollBarFrame()->SetScrollBarVisibilityL(
	                 CEikScrollBarFrame::EOn, CEikScrollBarFrame::EAuto);
	
	SetIconsL();
	
    
    TInt index = iProfileList->ListIndex(iProfileId);
    if (index == KErrNotFound)
    	{
    	// select first profile
    	if (iProfileList->Count() > 0)
    		{
   	    	TAspProfileItem& item = iProfileList->Item(0);
   	    	iProfileId = item.iProfileId;	
    		}
    	}

    UpdateListBoxL();
    

	iStatusPaneHandler->SetTitleL(R_ASP_TITLE_AUTOMATIC_SYNC_PROFILE_SELECTION);
	iStatusPaneHandler->SetNaviPaneTitleL(KNullDesC);
    }


// ----------------------------------------------------------------------------
// CAspProfileSelectionDialog::SetIconsL
//
// ----------------------------------------------------------------------------
//
void CAspProfileSelectionDialog::SetIconsL()
    {
    if (!iListBox)
    	{
    	return;
    	}
 
 	TFileName bitmapName;
	CAspResHandler::GetBitmapFileName(bitmapName);
	CArrayPtr<CGulIcon>* icons = new (ELeave) CAknIconArray(KDefaultArraySize);
	CleanupStack::PushL(icons);
		
	
	icons->AppendL(IconL(KAknsIIDQgnPropCheckboxOff, bitmapName,
	                     EMbmAvkonQgn_prop_checkbox_off,
	                     EMbmAvkonQgn_prop_checkbox_off_mask));

	/*
	icons->AppendL(IconL(KAknsIIDQgnIndiMarkedAdd, bitmapName, 
	                     EMbmAvkonQgn_indi_marked_add,
	                     EMbmAvkonQgn_indi_marked_add_mask));
	*/

	icons->AppendL(IconL(KAknsIIDQgnIndiSettProtectedAdd, bitmapName, 
	                     EMbmAspsyncutilQgn_indi_sett_protected_add,
	                     EMbmAspsyncutilQgn_indi_sett_protected_add_mask));
	
    // delete old icons
    CArrayPtr<CGulIcon>* arr = iListBox->ItemDrawer()->ColumnData()->IconArray();
    if (arr)
    	{
    	arr->ResetAndDestroy();
	    delete arr;
	    arr = NULL;
    	}

	iListBox->ItemDrawer()->ColumnData()->SetIconArray(icons);
	CleanupStack::Pop(icons);
    }


// -----------------------------------------------------------------------------
// CAspProfileSelectionDialog::IconL
// 
// -----------------------------------------------------------------------------
//
CGulIcon* CAspProfileSelectionDialog::IconL(TAknsItemID aId, const TDesC& aFileName, TInt aFileIndex, TInt aFileMaskIndex)
	{
    return TDialogUtil::CreateIconL(aId, aFileName, aFileIndex, aFileMaskIndex);
	}


//------------------------------------------------------------------------------
// CAspProfileSelectionDialog::DynInitMenuPaneL
//
// Called by system before menu is shown.
//------------------------------------------------------------------------------
//
void CAspProfileSelectionDialog::DynInitMenuPaneL(TInt aResourceID, CEikMenuPane* aMenuPane)
	{
    if (aResourceID != R_ASP_AUTO_SYNC_PROFILE_DIALOG_MENU_PANE)
		{
		return;
		}
		
	TInt index = iListBox->CurrentItemIndex();
	if (index != KErrNotFound)
		{
	    TAspProfileItem& item = iProfileList->Item(index);
	
	    if (iProfileId == item.iProfileId)
		    {
		    TDialogUtil::DeleteMenuItem(aMenuPane, EAspMenuCmdSelect);
		    }
		}
	else
		{
		TDialogUtil::DeleteMenuItem(aMenuPane, EAspMenuCmdSelect);
		TDialogUtil::DeleteMenuItem(aMenuPane, EAspMenuCmdView);
		}
    }


//------------------------------------------------------------------------------
// CAspProfileSelectionDialog::ProcessCommandL
//
// Handle commands from menu.
//------------------------------------------------------------------------------
//
void CAspProfileSelectionDialog::ProcessCommandL(TInt aCommandId)
	{
	HideMenu();

	switch (aCommandId)
		{
		case EAspMenuCmdHelp:
			{
            break;
			}

		case EAspMenuCmdSelect:
			{
			HandleOKL();
			break;
			}

		case EAspMenuCmdView:
			{
			ShowContentListL();			
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
// CAspProfileSelectionDialog::OkToExitL
//
//------------------------------------------------------------------------------
//
TBool CAspProfileSelectionDialog::OkToExitL(TInt aButtonId)
	{
	(*iDialogParam).iReturnValue = EFalse;
	(*iDialogParam).iProfileId = iProfileId;
	
	if (aButtonId == EEikBidCancel)
		{
		return ETrue;
		}

	if (aButtonId == EAknSoftkeyBack)
        {
       	(*iDialogParam).iReturnValue = ETrue;
		return ETrue;
		}

	if (aButtonId == EAknSoftkeyOpen)  // MSK
        {
        HandleOKL();
		return EFalse;  // leave dialog open
		}

	return CAknDialog::OkToExitL(aButtonId);
	}


// ----------------------------------------------------------------------------
// CAspProfileSelectionDialog::OfferKeyEventL
// 
// ----------------------------------------------------------------------------
//
TKeyResponse CAspProfileSelectionDialog::OfferKeyEventL(const TKeyEvent& aKeyEvent,
                                                      TEventCode aType)
	{
	
	// catch listbox item "gain focus" here
	if (aType == EEventKeyUp)
		{
		UpdateCbaL();
		return CAknDialog::OfferKeyEventL( aKeyEvent, aType);
		}
		
	if (aType == EEventKey)
		{
		switch (aKeyEvent.iCode)
			{
			case EKeyEnter:
			case EKeyOK:
				{
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
// CAspProfileSelectionDialog::HandleResourceChange
// 
// ----------------------------------------------------------------------------
//
void CAspProfileSelectionDialog::HandleResourceChange(TInt aType)
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
// CAspProfileSelectionDialog::HandleOKL
//
//------------------------------------------------------------------------------
//
void CAspProfileSelectionDialog::HandleOKL()
	{
	if (iCommandSetId == R_ASP_CBA_OPTIONS_VIEW_CONTENT_BACK)
		{
		ShowContentListL();
		return;
		}
		
	TInt index = iListBox->CurrentItemIndex();
	if (index != KErrNotFound)
		{
	    TAspProfileItem& item = iProfileList->Item(index);
	
	    if (item.iProfileId != iProfileId)
		    {
		    iProfileId = item.iProfileId; // uselect
		    UpdateListBoxL();
		    }
		}
	
	}


// ----------------------------------------------------------------------------
// CAspProfileSelectionDialog::UpdateListBoxL
// 
// Add settings headers into listbox.
// ----------------------------------------------------------------------------
//
void CAspProfileSelectionDialog::UpdateListBoxL()
	{
	_LIT(KFormat, "%d\t%S");
		
	CDesCArray* arr = (CDesCArray*) iListBox->Model()->ItemTextArray();
	arr->Reset();

	TInt count = iProfileList->Count();
	for (TInt i=0; i<count; i++ )
		{
		TAspProfileItem& item = iProfileList->Item(i);

		TBuf<KBufSize> buf;
		
		if (item.iProfileId == iProfileId)
			{
			buf.Format(KFormat, 1, &item.iProfileName);
			}
		else
			{
			buf.Format(KFormat, 0, &item.iProfileName);
			}

		arr->AppendL(buf);
		}

	iListBox->HandleItemAdditionL();
	}


// -----------------------------------------------------------------------------
// CAspProfileSelectionDialog::UpdateCbaL
// 
// -----------------------------------------------------------------------------
//
void CAspProfileSelectionDialog::UpdateCbaL()
    {
   	TInt index = iListBox->CurrentItemIndex();
   	if (index == KErrNotFound)
   		{
   		return;	
   		}
   		
	TAspProfileItem& item = iProfileList->Item(index);
	CEikButtonGroupContainer& cba = ButtonGroupContainer();
	
	if (item.iProfileId == iProfileId)
		{
		if (iCommandSetId != R_ASP_CBA_OPTIONS_VIEW_CONTENT_BACK)
			{
	        cba.SetCommandSetL(R_ASP_CBA_OPTIONS_VIEW_CONTENT_BACK);
	        cba.DrawDeferred();
	        iCommandSetId = R_ASP_CBA_OPTIONS_VIEW_CONTENT_BACK;
			}
		}
    else 
    	{
    	if (iCommandSetId != R_ASP_CBA_OPTIONS_SELECT_BACK)
    		{
    	    cba.SetCommandSetL(R_ASP_CBA_OPTIONS_SELECT_BACK);
    	    cba.DrawDeferred();
    	    iCommandSetId = R_ASP_CBA_OPTIONS_SELECT_BACK;		
    		}
    	}
    }


// ----------------------------------------------------------------------------
// CAspProfileSelectionDialog::ShowContentListL
// 
// ----------------------------------------------------------------------------
//
void CAspProfileSelectionDialog::ShowContentListL()
	{
	TInt index = iListBox->CurrentItemIndex();
	if (index == KErrNotFound)
		{
		return;	
		}
		
	TAspProfileItem& item = iProfileList->Item(index);
	
    TAspParam param(iApplicationId, iSyncSession);
	CAspProfile* profile = CAspProfile::NewLC(param);
    
   	profile->OpenL(item.iProfileId, CAspProfile::EOpenRead,
                                          CAspProfileList::EBasePropertiesOnly);
    iContentList->SetProfile(profile);
    TRAPD(err, iContentList->InitAllTasksL());
    iContentList->SetProfile(NULL);
    User::LeaveIfError(err);
    CleanupStack::PopAndDestroy(profile);
                                          

	CDesCArray* arr = new (ELeave) CDesCArrayFlat(KDefaultArraySize);
	CleanupStack::PushL(arr);
	
    TInt count = iContentList->ProviderCount();
    for (TInt i=0; i<count; i++)
    	{
    	TAspProviderItem& item = iContentList->ProviderItem(i);
    	
    	TInt index = iContentList->FindTaskIndexForProvider(item.iDataProviderId);
    	if (index != KErrNotFound)
    		{
    		HBufC* hBuf = CAspResHandler::GetContentNameLC(item.iDataProviderId,
		                                               item.iDisplayName);
		    arr->AppendL(hBuf->Des());
		    CleanupStack::PopAndDestroy(hBuf);
    		}
    	}

	HBufC* hBuf = CAspResHandler::ReadLC(R_ASP_TITLE_AUTOMATIC_SYNC_CONTENT_LIST);
	
	TDialogUtil::ShowPopupDialogL(hBuf->Des(), arr);
	
	CleanupStack::PopAndDestroy(hBuf);
	CleanupStack::PopAndDestroy(arr);
	}
	
	

//  End of File  
