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
#include "AspContentListDialog.h"
#include "AspContentDialog.h"
#include "AspDialogUtil.h"
#include "AspSyncUtil.rh"  // for menu command ids
#include "AspUtil.h"
#include "AspDebug.h"
#include "AspDefines.h"
#include "AspResHandler.h"
#include <csxhelp/ds.hlp.hrh>


#include <AspSyncUtil.mbg>  // for bitmap enumerations
#include <akniconarray.h>  // GulArray
#include <FeatMgr.h>   // FeatureManager



// ============================ MEMBER FUNCTIONS ===============================



// -----------------------------------------------------------------------------
// CAspContentListDialog::ShowDialogL
// 
// -----------------------------------------------------------------------------
//
TBool CAspContentListDialog::ShowDialogL(const TAspParam& aParam)
	{
    CAspContentListDialog* dialog = CAspContentListDialog::NewL(aParam);

	TBool ret = dialog->ExecuteLD(R_ASP_CONTENT_LIST_DIALOG);

	return ret;
	}


// -----------------------------------------------------------------------------
// CAspContentListDialog::NewL
//
// -----------------------------------------------------------------------------
//
CAspContentListDialog* CAspContentListDialog::NewL(const TAspParam& aParam)
    {
    FLOG( _L("CAspContentListDialog::NewL START") );

    CAspContentListDialog* self = new (ELeave) CAspContentListDialog(aParam);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);

    FLOG( _L("CAspContentListDialog::NewL END") );
    return(self);
    }


// -----------------------------------------------------------------------------
// CAspContentListDialog::CAspContentListDialog
// 
// -----------------------------------------------------------------------------
CAspContentListDialog::CAspContentListDialog(const TAspParam& aParam) 
    {
	iApplicationId = aParam.iApplicationId;
	iProfile = aParam.iProfile;
	iSyncSession = aParam.iSyncSession;
	
	__ASSERT_ALWAYS(iProfile, TUtil::Panic(KErrGeneral));
	__ASSERT_ALWAYS(iSyncSession, TUtil::Panic(KErrGeneral));		
    }


// -----------------------------------------------------------------------------
// CAspContentListDialog::ConstructL
//
// -----------------------------------------------------------------------------
void CAspContentListDialog::ConstructL()
    {
    FLOG( _L("CAspContentListDialog::ConstructL START" ) );

	// contruct menu for our dialog
	CAknDialog::ConstructL(R_ASP_CONTENT_LIST_DIALOG_MENU);

	// get previous title so it can be restored
	iStatusPaneHandler = CStatusPaneHandler::NewL(iAvkonAppUi);
	iStatusPaneHandler->StoreOriginalTitleL();


    TAspParam param(iApplicationId, iSyncSession);
	param.iProfile = iProfile;
	param.iMode = CAspContentList::EInitAll;

	iContentList = CAspContentList::NewL(param);

    iStatusPaneHandler->SetTitleL(R_ASP_CONTENT_LIST_DIALOG_TITLE);

	FLOG( _L("CAspContentListDialog::ConstructL END") );
    } 


// -----------------------------------------------------------------------------
// Destructor
//
// -----------------------------------------------------------------------------
CAspContentListDialog::~CAspContentListDialog()
    {
    FLOG( _L("CAspContentListDialog::~CAspContentListDialog START") );
    
    if (iAvkonAppUi)
    	{
    	iAvkonAppUi->RemoveFromStack(this);
    	}

	delete iStatusPaneHandler;
	delete iContentList;
	
	FLOG( _L("CAspContentListDialog::~CAspContentListDialog END") );
    }


//------------------------------------------------------------------------------
// CAspContentListDialog::ActivateL (from CCoeControl)
//
// Called by system when dialog is activated.
//------------------------------------------------------------------------------
//
void CAspContentListDialog::ActivateL()
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
void CAspContentListDialog::GetHelpContext(TCoeHelpContext& aContext) const
	{
	aContext.iMajor = KUidSmlSyncApp;
	//aContext.iContext = KDS_HLP_APPS_VIEW;    
	}


//------------------------------------------------------------------------------
// CAspContentListDialog::ProcessCommandL
//
// Handle commands from menu.
//------------------------------------------------------------------------------
//
void CAspContentListDialog::ProcessCommandL(TInt aCommandId)
	{
	HideMenu();

	switch (aCommandId)
		{
        case EAspMenuCmdEdit:
			{
			ShowContentL();
			break;
			}
        
        case EAspMenuCmdHelp:
			{
			TUtil::LaunchHelpAppL(iEikonEnv);
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
			{
			break;
			}
		}
	}


//------------------------------------------------------------------------------
// CAspContentListDialog::OkToExitL
//
//------------------------------------------------------------------------------
//
TBool CAspContentListDialog::OkToExitL(TInt aButtonId)
	{
	if (aButtonId == EEikBidCancel)
		{
		// exit going on - must return true.
		return ETrue;
		}

	if (aButtonId == EAknSoftkeyBack)
        {
		return ETrue;
		}

	return CAknDialog::OkToExitL(aButtonId);
	}


// ----------------------------------------------------------------------------
// CAspContentListDialog::OfferKeyEventL
// 
// ----------------------------------------------------------------------------
//
TKeyResponse CAspContentListDialog::OfferKeyEventL(const TKeyEvent& aKeyEvent,TEventCode aType)
	{
	if (aType == EEventKey)
		{
		switch (aKeyEvent.iCode)
			{
			case EKeyOK:
				{
				ShowContentL();
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


// -----------------------------------------------------------------------------
// CAspContentListDialog::PreLayoutDynInitL
//
// Called by framework before dialog is shown.
// -----------------------------------------------------------------------------
//
void CAspContentListDialog::PreLayoutDynInitL()
    {
    iSettingListBox = (CAknDoubleLargeStyleListBox*) ControlOrNull (EAspContentListDialogList);
    
   	__ASSERT_ALWAYS(iSettingListBox, TUtil::Panic(KErrGeneral));
	
	iSettingListBox->SetListBoxObserver(this);
	iSettingListBox->CreateScrollBarFrameL(ETrue);
	iSettingListBox->ScrollBarFrame()->SetScrollBarVisibilityL( CEikScrollBarFrame::EOff, CEikScrollBarFrame::EAuto );

	TFileName bitmapName;
	CAspResHandler::GetBitmapFileName(bitmapName);

	CArrayPtr<CGulIcon>* icons = new (ELeave) CAknIconArray(KDefaultArraySize);
	CleanupStack::PushL(icons);

 	// NOTE: icons must be appended in correct order (TAspContentListDialogIconIndex)
 	icons->AppendL(IconL(KAknsIIDQgnPropSmlSyncServer, bitmapName, EMbmAspsyncutilQgn_prop_sml_sync_server, EMbmAspsyncutilQgn_prop_sml_sync_server_mask));
	icons->AppendL(IconL(KAknsIIDQgnPropSmlSyncToServer, bitmapName, EMbmAspsyncutilQgn_prop_sml_sync_to_server, EMbmAspsyncutilQgn_prop_sml_sync_to_server_mask));
	icons->AppendL(IconL(KAknsIIDQgnPropSmlSyncFromServer, bitmapName, EMbmAspsyncutilQgn_prop_sml_sync_from_server, EMbmAspsyncutilQgn_prop_sml_sync_from_server_mask));
	icons->AppendL(IconL(KAknsIIDQgnPropSmlSyncOff, bitmapName, EMbmAspsyncutilQgn_prop_sml_sync_off, EMbmAspsyncutilQgn_prop_sml_sync_off_mask));

	iSettingListBox->ItemDrawer()->FormattedCellData()->SetIconArrayL(icons);
	CleanupStack::Pop(icons);

	UpdateListBoxL();
	
	iSettingListBox->UpdateScrollBarsL();
	iSettingListBox->ScrollBarFrame()->MoveVertThumbTo(0);
	}


// -----------------------------------------------------------------------------
// CAspContentListDialog::IconL
// 
// -----------------------------------------------------------------------------
//
CGulIcon* CAspContentListDialog::IconL(TAknsItemID aId, const TDesC& aFileName, TInt aFileIndex, TInt aFileMaskIndex)
	{
    return TDialogUtil::CreateIconL(aId, aFileName, aFileIndex, aFileMaskIndex);
	}


// -----------------------------------------------------------------------------
// CAspContentListDialog::DynInitMenuPaneL
//
// Called by framework before menu is shown.
// -----------------------------------------------------------------------------
//
void CAspContentListDialog::DynInitMenuPaneL(TInt aResourceID, 	CEikMenuPane* /*aMenuPane*/)
	{
    if (aResourceID != R_ASP_CONTENT_LIST_DIALOG_MENU_PANE)
		{
		return;
		}

	if (!FeatureManager::FeatureSupported(KFeatureIdHelp))
		{
		TDialogUtil::DeleteMenuItem(aMenuPane, EAspMenuCmdHelp);
		}

    }


// -----------------------------------------------------------------------------
// CAspContentListDialog::HandleListBoxEventL (from MEikListBoxObserver)
// 
// -----------------------------------------------------------------------------
//
void CAspContentListDialog::HandleListBoxEventL(CEikListBox* /*aListBox*/, TListBoxEvent /*aEventType*/)
	{
	}


// ----------------------------------------------------------------------------
// CAspContentListDialog::HandleResourceChange
// 
// ----------------------------------------------------------------------------
//
void CAspContentListDialog::HandleResourceChange(TInt aType)
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
		
    CCoeControl::HandleResourceChange(aType);
    }


// -----------------------------------------------------------------------------
// CAspContentListDialog::UpdateListBoxL
//	
// -----------------------------------------------------------------------------
//
void CAspContentListDialog::UpdateListBoxL()
	{
	TInt iconIndex = EIconNotDefined;
	
	CAknDoubleLargeStyleListBox* listBox = ListBox();
	CDesCArray* items = (CDesCArray*) listBox->Model()->ItemTextArray();
	items->Reset();
    listBox->ItemDrawer()->ClearAllPropertiesL();
	
	TInt count = iContentList->ProviderCount();

	for (TInt i=0; i<count; i++)
		{
		HBufC* firstLine = NULL;
		HBufC* secondLine = NULL;
				
		iconIndex = EIconNotDefined;
		TAspProviderItem& providerItem = iContentList->ProviderItem(i);
		
		firstLine = CAspResHandler::GetContentNameLC(providerItem.iDataProviderId, providerItem.iDisplayName);
		
		TInt index = iContentList->FindTaskIndexForProvider(providerItem.iDataProviderId);
		
		if (index != KErrNotFound)
			{
			TAspTaskItem& task = iContentList->TaskItem(index);
			TInt status = iContentList->CheckMandatoryTaskDataL(task.iTaskId);
	        if (status == EMandatoryOk)
			    {
			    secondLine = CAspResHandler::ReadLC(R_ASP_DEFINED);
                if (task.iSyncDirection == ESyncDirectionTwoWay)
				    {
				    iconIndex = EIconBothDirections;
				    }
			    if (task.iSyncDirection == ESyncDirectionOneWayFromDevice)
				    {
				    iconIndex = EIconDeviceToServer;
				    }
			    if (task.iSyncDirection == ESyncDirectionOneWayIntoDevice)
				    {
				    iconIndex = EIconServerToDevice;
				    }
			    }
			else
				{
				secondLine = CAspResHandler::ReadLC(R_ASP_NOT_DEFINED);
				}
			}
		else
			{
			secondLine = CAspResHandler::ReadLC(R_ASP_NOT_DEFINED);
			}
		
		TPtr ptr1 = firstLine->Des();
		AknTextUtils::DisplayTextLanguageSpecificNumberConversion(ptr1);
		TPtr ptr2 = secondLine->Des();
        AknTextUtils::DisplayTextLanguageSpecificNumberConversion(ptr2);

		HBufC* hBuf = NULL;
		hBuf = TDialogUtil::ConstructListBoxItemLC(firstLine->Des(), secondLine->Des(), iconIndex);

		items->AppendL(hBuf->Des());
		
		CleanupStack::PopAndDestroy(hBuf);
		CleanupStack::PopAndDestroy(secondLine);
		CleanupStack::PopAndDestroy(firstLine);
		}

	listBox->HandleItemAdditionL();

	if (listBox->Model()->NumberOfItems() == 0)
		{
		User::Leave(KErrNotFound);
		}
	}


// -----------------------------------------------------------------------------
// CAspContentListDialog::ShowContentL
//
// -----------------------------------------------------------------------------
//
void CAspContentListDialog::ShowContentL()
	{
 	TAspParam param(iApplicationId, iSyncSession);
    param.iProfile = iProfile;
    param.iDataProviderId = SelectedProvider().iDataProviderId;
    param.iSyncTaskId = KErrNotFound;

    CAspContentDialog::ShowDialogL(param);
     
    iContentList->InitAllTasksL();
    
	UpdateListBoxL();
	}


// -----------------------------------------------------------------------------
// CAspContentListDialog::ListBox
//
// -----------------------------------------------------------------------------
//
CAknDoubleLargeStyleListBox* CAspContentListDialog::ListBox()
	{
	__ASSERT_DEBUG(iSettingListBox, TUtil::Panic(KErrGeneral));

	return iSettingListBox;
	}


// -----------------------------------------------------------------------------
// CAspContentListDialog::SelectedProvider
//
// -----------------------------------------------------------------------------
//
TAspProviderItem& CAspContentListDialog::SelectedProvider()
	{
	TInt index = ListBox()->CurrentItemIndex();

	return iContentList->ProviderItem(index);
	}



//  End of File  
