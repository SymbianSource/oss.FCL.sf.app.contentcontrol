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
#include <txtfrmat.h>
#include <txtfmlyr.h>
#include <gdi.h>
// BackGround Context
#include <AknsBasicBackgroundControlContext.h>

#include "AspProfileWizardDialog.h"
#include "AspUtil.h"
#include "AspDialogUtil.h"
#include "AspDebug.h"
#include "AspSyncUtil.rh"

#include <textresolver.h>
#include <eikrted.h>  // CEikRichTextEditor


// ============================ MEMBER FUNCTIONS ===============================


/*******************************************************************************
 * class CAspTextSettingPage
 *******************************************************************************/


//------------------------------------------------------------------------------
// CAspTextSettingPage::CAspTextSettingPage
// 
//------------------------------------------------------------------------------
//
CAspTextSettingPage::CAspTextSettingPage(TInt aResourceID, TDes& aText,
                     TInt aTextSettingPageFlags, TAspSettingPageParam& aParam)
 : CAknTextSettingPage(aResourceID, aText, aTextSettingPageFlags), iParam(aParam)
	{
	}


//------------------------------------------------------------------------------
// Destructor
// 
//------------------------------------------------------------------------------
//
CAspTextSettingPage::~CAspTextSettingPage()
	{
	delete iIndicatorHandler;
	}


// -----------------------------------------------------------------------------
// CAspTextSettingPage::OKToExitL
// 
// -----------------------------------------------------------------------------
//
TBool CAspTextSettingPage::OkToExitL(TInt aButtonId)
	{
	 // command id set in ProcessCommandL
	if (iParam.iCommandId == EAknSoftkeyOk || iParam.iCommandId == EEikBidSelect)
		{
		if (iParam.iObserver)
			{
			CEikEdwin* edwin = TextControl();
			HBufC* hBuf = NULL;
			TInt len = edwin->TextLength();
			if (len == 0)
				{
				hBuf = HBufC::NewL(0);
				}
			else
				{
				hBuf = edwin->GetTextInHBufL();
				}
				
			TBool ret = iParam.iObserver->CheckValidity(hBuf->Des(),
			                                            iParam.iSettingType);
			
			delete hBuf;
			return ret;
			}
		}

	return  CAknTextSettingPage::OkToExitL(aButtonId);
	}


//------------------------------------------------------------------------------
// CAspTextSettingPage::ProcessCommandL
// 
//------------------------------------------------------------------------------
//
void CAspTextSettingPage::ProcessCommandL(TInt aCommandId)
	{
    iParam.iCommandId = aCommandId; // return command id to caller
    
    CAknTextSettingPage::ProcessCommandL(aCommandId);
	}


// ----------------------------------------------------------------------------
// CIMSSettingsWizardEditTextPage::PostDisplayCheckL
//
// ----------------------------------------------------------------------------
//
TBool CAspTextSettingPage::PostDisplayCheckL()
    {
    // Set the text to navipane
    UpdateNaviPaneL();

    return ETrue;
    }
    
// ----------------------------------------------------------------------------
// CIMSSettingsWizardEditTextPage::UpdateIndicatorL
//
// ----------------------------------------------------------------------------
//
void CAspTextSettingPage::UpdateNaviPaneL()
    {
    if (!iIndicatorHandler)
    	{
    	iIndicatorHandler = new (ELeave) CAspIndicatorHandler(
    	                                 iAvkonEnv->EditingStateIndicator());
    	}
    	
    iIndicatorHandler->SetIndicatorStateL(
              EAknNaviPaneEditorIndicatorMessageLength, iParam.iPageText);
    }
    

// ----------------------------------------------------------------------------
// CAspTextSettingPage::OfferKeyEventL
//
// ----------------------------------------------------------------------------
//
TKeyResponse CAspTextSettingPage::OfferKeyEventL(const TKeyEvent& aKeyEvent, 
                                                 TEventCode aType)
    {
    if(EKeyEnter == aKeyEvent.iCode)
    {
    	ProcessCommandL(EAknSoftkeyOk);
    	return EKeyWasConsumed;
    }
    if (aType == EEventKeyDown && iParam.iObserver)
        {
       	iParam.iObserver->HandleEditorEvent(
       	                  MAspEditorPageObserver::EKeyEvent, 0);
        }
        
    return CAknTextSettingPage::OfferKeyEventL(aKeyEvent, aType);
    }



/*******************************************************************************
 * class CAspRadioButtonSettingPage
 *******************************************************************************/


//------------------------------------------------------------------------------
// CAspRadioButtonSettingPage::CAspRadioButtonSettingPage
// 
//------------------------------------------------------------------------------
//
CAspRadioButtonSettingPage::CAspRadioButtonSettingPage(TInt aResourceID,
                            TInt& aCurrentSelectionIndex,
                            const MDesCArray* aItemArray,
                            TAspSettingPageParam& aParam)
                            
 : CAknRadioButtonSettingPage(aResourceID, aCurrentSelectionIndex, aItemArray),
   iParam(aParam)
	{
	iItemArray = aItemArray;
	}


//------------------------------------------------------------------------------
// Destructor
// 
//------------------------------------------------------------------------------
//
CAspRadioButtonSettingPage::~CAspRadioButtonSettingPage()
	{
	delete iNaviPaneHandler;
	}


//------------------------------------------------------------------------------
// CAspRadioButtonSettingPage::ProcessCommandL
//------------------------------------------------------------------------------
//
void CAspRadioButtonSettingPage::ProcessCommandL(TInt aCommandId)
	{
    if (aCommandId == EAknSoftkeyOk || aCommandId == EEikBidSelect)
    	{
    	TInt index = ListBoxControl()->CurrentItemIndex();
    	TBuf<KBufSize> buf;
    	buf.Num(index);
    	TBool ret = iParam.iObserver->CheckValidity(buf, iParam.iSettingType);
    	}
    
    iParam.iCommandId = aCommandId; // return command id to caller
    
    CAknRadioButtonSettingPage::ProcessCommandL(aCommandId);
	}


// ----------------------------------------------------------------------------
// CAspRadioButtonSettingPage::PostDisplayCheckL
// ----------------------------------------------------------------------------
//
TBool CAspRadioButtonSettingPage::PostDisplayCheckL()
    {
    UpdateNaviPaneL();

    return ETrue;
    }


// ----------------------------------------------------------------------------
// CAspRadioButtonSettingPage::UpdateNaviPaneL
// ----------------------------------------------------------------------------
//
void CAspRadioButtonSettingPage::UpdateNaviPaneL()
    {
    if (!iNaviPaneHandler)
    	{
    	iNaviPaneHandler = new (ELeave) CAspNaviPaneHandler(
    	                       iEikonEnv->AppUiFactory()->StatusPane());
    	}
    	
    iNaviPaneHandler->SetNaviPaneTitleL(iParam.iPageText);
    }

//----------------------------------------------------------------------------
//CAspRadioButtonSettingPage::HandleListBoxEventL
//----------------------------------------------------------------------------
//


void CAspRadioButtonSettingPage::HandleListBoxEventL(CEikListBox* aListBox, TListBoxEvent aEventType)
{
	if( AknLayoutUtils::PenEnabled() )
	{
	  	switch(aEventType)
		{  
			case EEventItemClicked:
       			 this->ProcessCommandL(EAknSoftkeySelect);	
        		 break;
		  	default:
				break;
    		
		}	
	}
}

// ----------------------------------------------------------------------------
// CAspRadioButtonSettingPage::OfferKeyEventL
//
// ----------------------------------------------------------------------------
//
TKeyResponse CAspRadioButtonSettingPage::OfferKeyEventL(const TKeyEvent& aKeyEvent, 
                                                 TEventCode aType)
    {
        if(EKeyEnter == aKeyEvent.iCode)
    {
    	ProcessCommandL(EAknSoftkeyOk);
    	return EKeyWasConsumed;
    }
    if (aType == EEventKeyDown && iParam.iObserver)
        {
       	iParam.iObserver->HandleEditorEvent(
                              MAspEditorPageObserver::EKeyEvent, 0);    
        }
        
    return CAknRadioButtonSettingPage::OfferKeyEventL(aKeyEvent, aType);
    }



/*******************************************************************************
 * class CAspAlphaPasswordSettingPage
 *******************************************************************************/


//------------------------------------------------------------------------------
// CAspAlphaPasswordSettingPage::CAspAlphaPasswordSettingPage
// 
//------------------------------------------------------------------------------
//
CAspAlphaPasswordSettingPage::CAspAlphaPasswordSettingPage(TInt aResourceID,
                              TDes& aNewPassword, const TDesC& aOldPassword,
                              TAspSettingPageParam& aParam)
                              
 : CAknAlphaPasswordSettingPage(aResourceID, aNewPassword, aOldPassword),
   iParam(aParam)
	{
	}


//------------------------------------------------------------------------------
// Destructor
// 
//------------------------------------------------------------------------------
//
CAspAlphaPasswordSettingPage::~CAspAlphaPasswordSettingPage()
	{
	delete iIndicatorHandler;
	}


//------------------------------------------------------------------------------
// CAspAlphaPasswordSettingPage::ProcessCommandL
// 
//------------------------------------------------------------------------------
//
void CAspAlphaPasswordSettingPage::ProcessCommandL(TInt aCommandId)
	{
    iParam.iCommandId = aCommandId; // return command id to caller
    
    CAknAlphaPasswordSettingPage::ProcessCommandL(aCommandId);
	}


// ----------------------------------------------------------------------------
// CAspAlphaPasswordSettingPage::PostDisplayCheckL

// ----------------------------------------------------------------------------
//
TBool CAspAlphaPasswordSettingPage::PostDisplayCheckL()
    {
    UpdateNaviPaneL();

    return ETrue;
    }

// ----------------------------------------------------------------------------
// CAspAlphaPasswordSettingPage::UpdateNaviPaneL
//
// ----------------------------------------------------------------------------
//
void CAspAlphaPasswordSettingPage::UpdateNaviPaneL()
    {
    if (!iIndicatorHandler)
    	{
    	iIndicatorHandler = new (ELeave) CAspIndicatorHandler(
    	                                 iAvkonEnv->EditingStateIndicator());
    	}
    	
    iIndicatorHandler->SetIndicatorStateL(
              EAknNaviPaneEditorIndicatorMessageLength, iParam.iPageText);
    }


// ----------------------------------------------------------------------------
// CAspAlphaPasswordSettingPage::OfferKeyEventL
//
// ----------------------------------------------------------------------------
//
TKeyResponse CAspAlphaPasswordSettingPage::OfferKeyEventL(const TKeyEvent& aKeyEvent, 
                                                 TEventCode aType)
    {
    if (aType == EEventKeyDown && iParam.iObserver)
        {
       	iParam.iObserver->HandleEditorEvent(
                                MAspEditorPageObserver::EKeyEvent, 0);    
        }
        
    return CAknAlphaPasswordSettingPage::OfferKeyEventL(aKeyEvent, aType);
    }


/*******************************************************************************
 * class CAspCheckBoxSettingPage
 *******************************************************************************/


//------------------------------------------------------------------------------
// CAspCheckBoxSettingPage::CAspCheckBoxSettingPage
// 
//------------------------------------------------------------------------------
//
CAspCheckBoxSettingPage::CAspCheckBoxSettingPage(TInt aResourceID,
                   CSelectionItemList* aItemArray, TAspSettingPageParam& aParam)
                   
 : CAknCheckBoxSettingPage(aResourceID, aItemArray), iParam(aParam)
	{
    iSelectionItemList = aItemArray;
	}


//------------------------------------------------------------------------------
// Destructor
// 
//------------------------------------------------------------------------------
//
CAspCheckBoxSettingPage::~CAspCheckBoxSettingPage()
	{
	delete iNaviPaneHandler;
	}


//------------------------------------------------------------------------------
// CAspCheckBoxSettingPage::ProcessCommandL
// 
//------------------------------------------------------------------------------
//
void CAspCheckBoxSettingPage::ProcessCommandL(TInt aCommandId)
	{
    if (aCommandId == EAknSoftkeyMark)
    	{
        CAknSetStyleListBox* listbox = ListBoxControl();
        TInt index = listbox->CurrentItemIndex();
       	CSelectableItem* selectableItem = (*iSelectionItemList)[index];
       	TBuf<KBufSize> buf;
       	buf = selectableItem->ItemText();
       	
       	TBool ret = iParam.iObserver->CheckValidity(buf, iParam.iSettingType);
			
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


// ----------------------------------------------------------------------------
// CAspCheckBoxSettingPage::PostDisplayCheckL
//
// ----------------------------------------------------------------------------
//
TBool CAspCheckBoxSettingPage::PostDisplayCheckL()
    {
    UpdateNaviPaneL();
    
    iSelectionCount = ListBoxControl()->SelectionIndexes()->Count();

    return ETrue;
    }


// ----------------------------------------------------------------------------
// CAspCheckBoxSettingPage::UpdateNaviPaneL
// ----------------------------------------------------------------------------
//
void CAspCheckBoxSettingPage::UpdateNaviPaneL()
    {
    if (!iNaviPaneHandler)
    	{
    	iNaviPaneHandler = new (ELeave) CAspNaviPaneHandler(
    	                       iEikonEnv->AppUiFactory()->StatusPane());
    	}
    	
    iNaviPaneHandler->SetNaviPaneTitleL(iParam.iPageText);
    }

// ----------------------------------------------------------------------------
// CAspCheckBoxSettingPage::OfferKeyEventL
//
// ----------------------------------------------------------------------------
//
TKeyResponse CAspCheckBoxSettingPage::OfferKeyEventL(const TKeyEvent& aKeyEvent, 
                                                 TEventCode aType)
    {
    if(EKeyEnter == aKeyEvent.iCode)
    {
    	ProcessCommandL(EAknSoftkeyMark);
    	return EKeyWasConsumed;
    }
   
    if (aType == EEventKeyDown && iParam.iObserver)
        {
       	iParam.iObserver->HandleEditorEvent(
                              MAspEditorPageObserver::EKeyEvent, 0);    
        }

    if (aType == EEventKeyUp && iParam.iObserver)
        {
	    TInt count = ListBoxControl()->SelectionIndexes()->Count();
       	if (count != iSelectionCount)
       		{
       		iParam.iObserver->GetNavipaneText(iParam.iPageText, count);
       	    UpdateNaviPaneL();
       	    iSelectionCount = count;
       		}
        }
        
    return CAknCheckBoxSettingPage::OfferKeyEventL(aKeyEvent, aType);
    }
// ----------------------------------------------------------------------------
// CAspCheckBoxSettingPage::HandleListBoxEventL
//
// ----------------------------------------------------------------------------
//

				 
void CAspCheckBoxSettingPage::HandleListBoxEventL(CEikListBox* aListBox, TListBoxEvent aEventType)
{
	TInt index;
	CSelectableItem* selectableItem;
	if( AknLayoutUtils::PenEnabled() )  
	{
	  	switch(aEventType)
	
		{   
			case EEventItemClicked:
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

/*******************************************************************************
 * class CAspProfileWizardDialog
 *******************************************************************************/



// -----------------------------------------------------------------------------
// CAspProfileWizardDialog::ShowDialogL
// 
// -----------------------------------------------------------------------------
//
TBool CAspProfileWizardDialog::ShowDialogL(TAspParam& aParam)
	{
    CAspProfileWizardDialog* dialog = CAspProfileWizardDialog::NewL(aParam);

	TBool ret = dialog->ExecuteLD(R_ASP_PROFILE_WIZARD_DIALOG);

	return ret;
	}


// -----------------------------------------------------------------------------
// CAspProfileWizardDialog::NewL
//
// -----------------------------------------------------------------------------
//
CAspProfileWizardDialog* CAspProfileWizardDialog::NewL(TAspParam& aParam)
    {
    FLOG( _L("CAspProfileWizardDialog::NewL START") );

    CAspProfileWizardDialog* self = new (ELeave) CAspProfileWizardDialog(aParam);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);

    FLOG( _L("CAspProfileWizardDialog::NewL END") );
    return self;
    }


// -----------------------------------------------------------------------------
// CAspProfileWizardDialog::CAspProfileWizardDialog
// 
// -----------------------------------------------------------------------------
//
CAspProfileWizardDialog::CAspProfileWizardDialog(TAspParam& aParam) 
    {
    iSyncSession = aParam.iSyncSession;
	iApplicationId = aParam.iApplicationId;
	iDialogParam = &aParam;
	
	iBgContext = NULL;
	iMoveBack = EFalse ;
	__ASSERT_ALWAYS(iSyncSession, TUtil::Panic(KErrGeneral));
    }


// -----------------------------------------------------------------------------
// CAspProfileWizardDialog::ConstructL
//
// -----------------------------------------------------------------------------
//
void CAspProfileWizardDialog::ConstructL()
    {
    FLOG( _L("CAspProfileWizardDialog::ConstructL START") );

    if (iApplicationId == KErrNotFound)
    	{
    	User::Leave(KErrNotFound); // unknown caller app
    	}
	
    TAspParam param2(KErrNotFound, iSyncSession);
	iBearerHandler = CAspBearerHandler::NewL(param2);

	iStatusPaneHandler = CStatusPaneHandler::NewL(iAvkonAppUi);
	iStatusPaneHandler->StoreOriginalTitleL();
	
	CAspResHandler::ReadL(iNaviPaneTextFormat, R_ASP_WIZARD_PAGE_COUNTER);
	
	iActiveCaller = CAspActiveCaller::NewL(this);

	TAspParam param(iApplicationId, iSyncSession);
	iWizard = CAspProfileWizard::NewL(param);
	SetDefaultValuesL();
	
	param.iApplicationId = EApplicationIdSync;
	iProfileList = CAspProfileList::NewL	(param);
	iProfileList->ReadAllProfilesL(CAspProfileList::ENoMandatoryCheck);
	
	iPopupNote = CAknInfoPopupNoteController::NewL();
	
    if (iApplicationId == EApplicationIdEmail)
    	{
   	    CDesCArray* arr = iWizard->LocalDatabaseList(KUidNSmlAdapterEMail.iUid);
	    if (!arr)
		    {
		    TDialogUtil::ShowErrorNoteL(R_ASP_NO_MAILBOXES);
		    User::Leave(KErrNotFound); // no syncml mailbox
		    }
		
	    if (arr->Count() == 0) 
		    {
		    TDialogUtil::ShowErrorNoteL(R_ASP_NO_MAILBOXES);
		    User::Leave(KErrNotFound); // no syncml mailbox
		    }
    	}

	FLOG( _L("CAspProfileWizardDialog::ConstructL END") );
    } 


// -----------------------------------------------------------------------------
// Destructor
//
// -----------------------------------------------------------------------------
//
CAspProfileWizardDialog::~CAspProfileWizardDialog()
    {
    FLOG( _L("CAspProfileWizardDialog::~CAspProfileWizardDialog START") );

    if (iAvkonAppUi)
    	{
    	iAvkonAppUi->RemoveFromStack(this);
    	}

	delete iWizard;
	if(iTextEditorText)
		{
		delete iTextEditorText;
		}
    delete iBearerHandler;
	delete iStatusPaneHandler;
	delete iActiveCaller;
	delete iProfileList;
	
	if (iPopupNote)
		{
    	iPopupNote->HideInfoPopupNote();
	    delete iPopupNote;
		}
	
	if (iBgContext)
		{
		delete iBgContext ;
		iBgContext = NULL ;
		}

	
	FLOG( _L("CAspProfileWizardDialog::~CAspProfileWizardDialog END") );
    }


//------------------------------------------------------------------------------
// CAspProfileWizardDialog::ActivateL (from CCoeControl)
//
// Called by system when dialog is activated.
//------------------------------------------------------------------------------
//
void CAspProfileWizardDialog::ActivateL()
	{
    FLOG( _L("CAspProfileWizardDialog::ActivateL START") );
    
    CAknDialog::ActivateL();

	// this cannot be in ConstructL which is executed before dialog is launched
	iAvkonAppUi->AddToStackL(this);
	
    FLOG( _L("CAspProfileWizardDialog::ActivateL END") );
    }


// -----------------------------------------------------------------------------
// CAspProfileWizardDialog::OKToExitL
// 
// -----------------------------------------------------------------------------
//
TBool CAspProfileWizardDialog::OkToExitL(TInt aButtonId)
	{
    FLOG( _L("CAspProfileWizardDialog::OkToExitL START") );
	
	iDialogParam->iReturnValue = KErrNotFound;
	iDialogParam->iProfileId = KErrNotFound;

	if (aButtonId == EEikBidOk)
		{
		if (iWizard->CurrentItemType() == CAspWizardItem::ETypeStart)
			{
			if (iTextEditorText)
   				{
   			 	delete iTextEditorText;
				iTextEditorText = NULL;
				}
    		TRect mainPane;
			AknLayoutUtils::LayoutMetricsRect( AknLayoutUtils::EApplicationWindow, mainPane );
   	 		iTextEditor->SetRect(mainPane);
    		iTextEditor->SetTextL(iTextEditorText);
    		UpdateCbaL(R_ASP_CBA_NEXT_BACK_NEXT);
		
			iWizard->MoveToNext();
			
			// start wizard when this completes (in HandleActiveCallL)
			iActiveCaller->Request();
			 
			FLOG( _L("CAspProfileWizardDialog::OkToExitL END") );
			return EFalse;
			}
		else if (iWizard->CurrentItemType() == CAspWizardItem::ETypeEnd)
			{
			iDialogParam->iReturnValue = KErrNone;
			iDialogParam->iProfileId = iWizard->CreatedProfileId();
	        
	        FLOG( _L("CAspProfileWizardDialog::OkToExitL END") );
			return ETrue;
			}
		//Case of quick succession clicks on touch screen profile creation wizard
		else
			{
			   return EFalse;	
			}
		}
	//Case of quick succession backs on touch screen profile creation wizard
	else if(aButtonId == EAknSoftkeyDone)
	{
		return EFalse;
	}
		
		
	
	FLOG( _L("CAspProfileWizardDialog::OkToExitL END") );
	return CAknDialog::OkToExitL(aButtonId);
	}


//------------------------------------------------------------------------------
// CAspProfileWizardDialog::ProcessCommandL
//
//------------------------------------------------------------------------------
//
void CAspProfileWizardDialog::ProcessCommandL(TInt /*aCommandId*/)
	{
	}


// ----------------------------------------------------------------------------
// CAspProfileWizardDialog::OfferKeyEventL
// 
// ----------------------------------------------------------------------------
//
TKeyResponse CAspProfileWizardDialog::OfferKeyEventL(const TKeyEvent& aKeyEvent,TEventCode aType)
	{
	if (aType == EEventKey)
		{
		switch (aKeyEvent.iCode)
			{
			case EKeyEnter:
			{
			     TryExitL(EEikBidOk);
			     return EKeyWasConsumed;
			}
			case EKeyOK:
				{
				break;
				}

			case EKeyEscape:  // framework calls this when dialog must shut down
				{
				return CAknDialog::OfferKeyEventL(aKeyEvent, aType);
        		}

            default:
				{
				//return iTextEditor->OfferKeyEventL(aKeyEvent, aType);
			    break;
				}
			}
		}

	return CAknDialog::OfferKeyEventL( aKeyEvent, aType);
	}


// -----------------------------------------------------------------------------
// CAspProfileWizardDialog::PreLayoutDynInitL
//
// Called by framework before dialog is shown.
// -----------------------------------------------------------------------------
//
void CAspProfileWizardDialog::PreLayoutDynInitL()
    {
    FLOG( _L("CAspProfileWizardDialog::PreLayoutDynInitL START") );
   
    iTextEditor = (CEikRichTextEditor*) ControlOrNull (EAspProfileWizardDialogList);
    
    User::LeaveIfNull(iTextEditor);
   
    SetTextEditorTextL(R_ASP_SET_WIZARD_START);
    
	TRect mainPane;
    AknLayoutUtils::LayoutMetricsRect(AknLayoutUtils::EMainPane, mainPane);        
    iTextEditor->SetRect(mainPane);
	
	SetBackgroundContextL ();
	
    iStatusPaneHandler->SetNaviPaneTitleL(KNullDesC);
    iStatusPaneHandler->SetTitleL(R_ASP_SETTING_DIALOG_TITLE);
    
    FLOG( _L("CAspProfileWizardDialog::PreLayoutDynInitL END") );
	}


// ----------------------------------------------------------------------------
// CAspProfileWizardDialog::HandleResourceChange
// 
// ----------------------------------------------------------------------------
//
void CAspProfileWizardDialog::HandleResourceChange(TInt aType)
    {   
    if (aType == KEikDynamicLayoutVariantSwitch) //Handle change in layout orientation
        {
        TRect mainPaneRect;
        AknLayoutUtils::LayoutMetricsRect(AknLayoutUtils::EMainPane, mainPaneRect);
        SetRect(mainPaneRect);
        iTextEditor->SetSize(mainPaneRect.Size());
        SetBackgroundContextL();
        CCoeControl::HandleResourceChange(aType);
		DrawDeferred();
	    return;
		}
	if (aType == KAknsMessageSkinChange)
		{
		// When skin changes, we need to update text color
		TRAP_IGNORE(UpdateTextColorL());
		}
		
    CCoeControl::HandleResourceChange(aType);
    }


// -----------------------------------------------------------------------------
// CAspProfileWizardDialog::HandleActiveCallL (from MAspActiveCallerObserver)
//
// -----------------------------------------------------------------------------
//
void CAspProfileWizardDialog::HandleActiveCallL(TInt /*aCallId*/)
	{
    FLOG( _L("CAspProfileWizardDialog::HandleActiveCallL START") );
	
	TInt err = RunWizard();
	
	if (err != KErrNone)
		{
		iWizard->DeleteProfile();
		
		ShowErrorNoteL(err);

        TryExitL(EEikBidCancel); // close profile wizard dialog
		}

    FLOG( _L("CAspProfileWizardDialog::HandleActiveCallL END") );
	}


// -----------------------------------------------------------------------------
// CAspProfileWizardDialog::RunWizard
//
// -----------------------------------------------------------------------------
//
TInt CAspProfileWizardDialog::RunWizard()
	{
    FLOG( _L("CAspProfileWizardDialog::RunWizard START") );
    
    TRAPD(err, RunWizardL());
	
	HidePopupNote();
	
	if (err != KErrNone)
		{
		iWizard->MoveToStart();
		FLOG( _L("### RunWizardL failed (%d) ###"), err );
		}
		
    FLOG( _L("CAspProfileWizardDialog::RunWizard END") );
	return err;
	}


// -----------------------------------------------------------------------------
// CAspProfileWizardDialog::RunWizardL
//
// -----------------------------------------------------------------------------
//
void CAspProfileWizardDialog::RunWizardL()
	{
	for (;;)
		{
		CAspWizardItem* item = iWizard->CurrentItem();
	
	    if (item->iSettingType == CAspWizardItem::ETypeEnd)
		    {
		    iMoveBack = ETrue ;
		    iWizard->CreateProfileL();
		    
		    if (iApplicationId == EApplicationIdSync)
		    	{
		    	SetTextEditorTextL(R_ASP_SET_WIZARD_END);
		        UpdateCbaL(R_ASP_CBA_ACTIVATE_LATER_ACTIVATE);
		    	}
		    else if (iApplicationId == EApplicationIdEmail)
		    	{
		    	SetTextEditorTextL(R_ASP_SET_WIZARD_END_EMAIL);
		    	UpdateCbaL(R_AVKON_SOFTKEYS_OK_EMPTY);
		    	}
		    else
		    	{
		    	SetTextEditorTextL(R_ASP_SET_WIZARD_END_APP);
		    	UpdateCbaL(R_AVKON_SOFTKEYS_OK_EMPTY);
		    	}

		    TBuf<KBufSize> buf;
		    GetNaviPaneText(buf, item);
		    iStatusPaneHandler->SetNaviPaneTitleL(buf);
		    
		    return; // return to wizard dialog view
		    }
		
	    if (item->iSettingType == CAspWizardItem::ETypeStart)
		    {
		    iMoveBack = ETrue ;
		    SetTextEditorTextL(R_ASP_SET_WIZARD_START);
		    UpdateCbaL(R_ASP_CBA_START_CANCEL_START);
		    return; // return to wizard dialog view
		    }

	    TInt ret = EditSettingItemL(item); // show setting editor
	
	    if (ret == EAknSoftkeyOk || ret == EEikBidSelect)
		    {
		    iMoveBack = EFalse ;
		    iWizard->MoveToNext();
        	}
	    else if (ret == EAknSoftkeyDone)
		    {
		    iMoveBack = ETrue ;
		    iWizard->MoveToPrevious();
			}
	    else
	        {
	        iMoveBack = EFalse ;
	        iWizard->MoveToStart();
		    return; // probably system cancel - wizard must close
		    }
		}
	}


//------------------------------------------------------------------------------
// CAspProfileWizardDialog::EditSettingItemL
//
// Calls setting editing functions. 
//------------------------------------------------------------------------------
//
TInt CAspProfileWizardDialog::EditSettingItemL(CAspWizardItem* aItem)
	{
	TInt ret = EAknSoftkeyOk;
	
	ShowPopupNoteL(aItem->iPopupNoteResourceId, KMsToWaitBeforePopup);
	
	switch (aItem->iSettingType)
		{
		case CAspWizardItem::ETypeStart:
		    break;
			
		case CAspWizardItem::ETypeProfileName:
			ret = EditSettingItemProfileNameL(aItem);
		    break;

		case CAspWizardItem::ETypeProtocolVersion:
			ret = EditSettingItemProtocolVersionL(aItem);
		    break;

		case CAspWizardItem::ETypeHostAddress:
			ret = EditSettingItemHostAddressL(aItem);
		    break;

		case CAspWizardItem::ETypeServerId:
			ret = EditSettingItemTextL(aItem);
		    break;

		case CAspWizardItem::ETypeBearerType:
			ret = EditSettingItemBearerTypeL(aItem);
		    break;

		case CAspWizardItem::ETypeUserName:
			ret = EditSettingItemTextL(aItem);
		    break;
		
		case CAspWizardItem::ETypePassword:
			ret = EditSettingItemSecretL(aItem);
		    break;
		    
		case CAspWizardItem::ETypeContentSelection:
			ret = EditSettingItemContentSelectionL(aItem);
		    break;

		case CAspWizardItem::ETypeSyncContent:
	    	ret = EditSettingItemTextL(aItem);
		    break;

		case CAspWizardItem::ETypeSyncContentLocalDatabase:
	    	ret = EditSettingItemLocalDatabaseL(aItem);
		    break;

		case CAspWizardItem::ETypeEnd:
			break;

		default:
		    break;
		}

	HidePopupNote();
	
	return ret;
	}


//------------------------------------------------------------------------------
// CAspProfileWizardDialog::EditSettingItemTextL
//
// Edit text setting item.
//------------------------------------------------------------------------------
//
TInt CAspProfileWizardDialog::EditSettingItemTextL(CAspWizardItem* aItem)
	{
	TUtil::StrCopy(iBuf, aItem->Value());
	
	TInt flags(CAknTextSettingPage::EZeroLengthAllowed);

    TAspSettingPageParam param;
    param.iCommandId = EAknSoftkeyOk;
    GetNaviPaneText(param.iPageText, aItem);
    param.iObserver = this;
    param.iSettingType = aItem->iSettingType;
    
	CAspTextSettingPage* dlg = new (ELeave) CAspTextSettingPage(
	                     R_ASP_SETTING_TEXT_NEXT_BACK, iBuf, flags, param);
	CleanupStack::PushL(dlg);

	dlg->SetSettingTextL(aItem->Header()); 
	dlg->ConstructL();
	dlg->TextControl()->SetTextLimit(aItem->iMaxLength);

	if (aItem->iLatinInput)
		{		
		dlg->TextControl()->SetAvkonWrap(ETrue);
		dlg->TextControl()->SetAknEditorCase(EAknEditorLowerCase);
		dlg->TextControl()->SetAknEditorFlags(EAknEditorFlagNoT9 |
		                                EAknEditorFlagLatinInputModesOnly);
		}
	
	CleanupStack::Pop(dlg);

    //
	// CAknTextSettingPage::ExecuteLD returns boolean. 
	// See CAknSettingPage::ProcessCommandL what softkey values map to
	// ETrue and EFalse.
	// Profile wizard editors return EAknSoftkeyDone for "Back" and 
	// EAknSoftkeyOk for "Next". Editors need "positive" ids for saving
	// setting values.
	//
	TInt ret = dlg->ExecuteLD(CAknSettingPage::EUpdateWhenChanged);
   

    aItem->SetValueL(iBuf);
    
    return param.iCommandId;
	}


//------------------------------------------------------------------------------
// CAspProfileWizardDialog::EditSettingItemProtocolVersionL
//
//------------------------------------------------------------------------------
//
TBool CAspProfileWizardDialog::EditSettingItemProtocolVersionL(CAspWizardItem* aItem)
	{
	TInt curSelection = aItem->iNumber;
	CDesCArray* arr2 = CAspResHandler::ReadDesArrayStaticLC(R_ASP_PROTOCOL_VERSION);
	CDesCArray* arr = TUtil::NumberConversionLC(arr2);
	
    TAspSettingPageParam param;
    param.iCommandId = EAknSoftkeyOk;
    GetNaviPaneText(param.iPageText, aItem);
    param.iObserver = this;
    param.iSettingType = aItem->iSettingType;
	
	CAspRadioButtonSettingPage* dlg = new (ELeave) CAspRadioButtonSettingPage(
	                                  R_ASP_SETTING_RADIO_NEXT_BACK, 
	                                  curSelection, arr, param);
	CleanupStack::PushL(dlg);
	dlg->SetSettingTextL(aItem->Header()); 
	CleanupStack::Pop(dlg);

	dlg->ExecuteLD();
	
	aItem->iNumber = curSelection;
   	aItem->SetValueL((*arr)[curSelection]);
   	
   	if (curSelection == EAspProtocol_1_1)
   		{
   		iWizard->SetContentEnabled(KUidNSmlAdapterEMail.iUid, EFalse);
        iWizard->SetEnabled(CAspWizardItem::ETypeServerId, EFalse);
   		}
   	else
   		{
   		iWizard->SetEnabled(CAspWizardItem::ETypeServerId, ETrue);
   		}

	CleanupStack::PopAndDestroy(arr);
	CleanupStack::PopAndDestroy(arr2);
   	return param.iCommandId;
	}


//------------------------------------------------------------------------------
// CAspProfileWizardDialog::EditSettingItemBearerTypeL
//
//------------------------------------------------------------------------------
//
TInt CAspProfileWizardDialog::EditSettingItemBearerTypeL(CAspWizardItem* aItem)
	{
	CDesCArray* arr = iBearerHandler->BuildBearerListLC();
	if (arr->Count() == 0) 
		{
		User::Leave(KErrNotFound);
		}

	TInt curSelection = iBearerHandler->ListIndexForBearer(aItem->iNumber);
	if (curSelection == KErrNotFound)
		{
		User::Leave(KErrNotFound);
		}


    TAspSettingPageParam param;
    param.iCommandId = EAknSoftkeyOk;
    GetNaviPaneText(param.iPageText, aItem);
    param.iObserver = this;
    param.iSettingType = aItem->iSettingType;
	
	CAspRadioButtonSettingPage* dlg = new (ELeave) CAspRadioButtonSettingPage(
	                                  R_ASP_SETTING_RADIO_NEXT_BACK, 
	                                  curSelection, arr, param);
	CleanupStack::PushL(dlg);
	dlg->SetSettingTextL(aItem->Header()); 
	CleanupStack::Pop(dlg);

	dlg->ExecuteLD();
		
	aItem->iNumber = iBearerHandler->BearerForListIndex(curSelection);
   	aItem->SetValueL((*arr)[curSelection]);
	
	CleanupStack::PopAndDestroy(arr);
		
   	return param.iCommandId;
	}


//------------------------------------------------------------------------------
// CAspProfileWizardDialog::EditSettingItemLocalDatabaseL
//
//------------------------------------------------------------------------------
//
TInt CAspProfileWizardDialog::EditSettingItemLocalDatabaseL(CAspWizardItem* aItem)
	{
	CDesCArray* arr = iWizard->LocalDatabaseList(aItem->iSettingId);
	if (!arr)
		{
		User::Leave(KErrNotFound);
		}
		
	if (arr->Count() == 0) 
		{
		User::Leave(KErrNotFound);
		}

	TInt curSelection = aItem->iNumber;
	if (curSelection < 0)
		{
		curSelection = 0;
		}


    TAspSettingPageParam param;
    param.iCommandId = EAknSoftkeyOk;
    GetNaviPaneText(param.iPageText, aItem);
    param.iObserver = this;
    param.iSettingType = aItem->iSettingType;
	
	CAspRadioButtonSettingPage* dlg = new (ELeave) CAspRadioButtonSettingPage(
	                                  R_ASP_SETTING_RADIO_NEXT_BACK, 
	                                  curSelection, arr, param);
	CleanupStack::PushL(dlg);
	dlg->SetSettingTextL(aItem->Header()); 
	CleanupStack::Pop(dlg);

	dlg->ExecuteLD();
		
	aItem->iNumber = curSelection;
   	aItem->SetValueL((*arr)[curSelection]);
	
   	return param.iCommandId;
	}


//------------------------------------------------------------------------------
// CAspProfileWizardDialog::EditSettingItemSecretL
//
//------------------------------------------------------------------------------
//
TInt CAspProfileWizardDialog::EditSettingItemSecretL(CAspWizardItem* aItem)
	{
	TBuf<KBufSize64> newPassword;
	TUtil::StrCopy(newPassword, aItem->Value());
	
	
    TAspSettingPageParam param;
    param.iCommandId = EAknSoftkeyOk;
    GetNaviPaneText(param.iPageText, aItem);
    param.iObserver = this;
    param.iSettingType = aItem->iSettingType;

	CAspAlphaPasswordSettingPage* pwd = new (ELeave) CAspAlphaPasswordSettingPage(
		     R_ASP_SETTING_PASSWORD_NEXT_BACK, newPassword, aItem->Value(), param);

	CleanupStack::PushL(pwd);
	pwd->SetSettingTextL(aItem->Header()); 
	pwd->ConstructL();
	pwd->AlphaPasswordEditor()->SetMaxLength(aItem->iMaxLength);
	//pwd->AlphaPasswordEditor()->RevealSecretText(ETrue);
	CleanupStack::Pop(pwd);

	TInt ret = pwd->ExecuteLD(CAknSettingPage::EUpdateWhenChanged);
		
	aItem->SetValueL(newPassword);
   	
   	return param.iCommandId;
	}


//------------------------------------------------------------------------------
// CAspProfileWizardDialog::EditSettingItemProfileNameL
//
//------------------------------------------------------------------------------
//
TInt CAspProfileWizardDialog::EditSettingItemProfileNameL(CAspWizardItem* aItem)
	{
	TInt commandId = EditSettingItemTextL(aItem);
      	
    return commandId;
	}


//------------------------------------------------------------------------------
// CAspProfileWizardDialog::EditSettingItemHostAddressL
//
//------------------------------------------------------------------------------
//
TInt CAspProfileWizardDialog::EditSettingItemHostAddressL(CAspWizardItem* aItem)
	{
	_LIT(KIPAddressRegX,"http://*");
	TInt bearer = iWizard->NumberValue(CAspWizardItem::ETypeBearerType);
	TPtrC ptr = aItem->Value();
	TPtrC httpHeader(KIPAddressRegX); 
	
	if (bearer == EAspBearerInternet)
		{
		   //If existing ptr is NOT an IP address
		   if(ptr.MatchC(httpHeader) == KErrNotFound)
		   {
				aItem->SetValueL(KHttpHeader);   	
		   }
		}
	else
		{
		    //If existing ptr is IP address
		    if(ptr.MatchC(httpHeader) != KErrNotFound)
		    {
		    	aItem->SetValueL(KNullDesC); 		
		    }
		}
	
	TInt commandId;
	while (ETrue)
		{
		commandId = EditSettingItemTextL(aItem);
		if (commandId == EAknSoftkeyOk && bearer == EAspBearerInternet)
			{
			if (!TURIParser::IsValidUri(aItem->Value()))
				{			
				TDialogUtil::ShowInformationNoteL(R_ASP_LOG_ERR_URIINVALID);
				}
			else
				{
				break;	
				}
			}
		else
			{
			break;	
			}
		}
    return commandId;
	}


//------------------------------------------------------------------------------
// CAspProfileWizardDialog::EditSettingItemContentSelectionL
//
//------------------------------------------------------------------------------
//
TInt CAspProfileWizardDialog::EditSettingItemContentSelectionL(CAspWizardItem* aItem)
	{
	CAspSelectionItemList* list = new (ELeave) CAspSelectionItemList(1);
	CleanupStack::PushL(TCleanupItem(CAspSelectionItemList::Cleanup, list));
	
	iWizard->GetContentSelectionL(list);
	
    TAspSettingPageParam param;
    param.iCommandId = EAknSoftkeyOk;
    GetNaviPaneText(param.iPageText, aItem);
    param.iObserver = this;
    param.iSettingType = aItem->iSettingType;

	CAspCheckBoxSettingPage* dlg = new (ELeave) CAspCheckBoxSettingPage(
	                                   R_ASP_SETTING_CHECK_NEXT_BACK, list, param);
	
   	CleanupStack::PushL(dlg);
    dlg->SetSettingTextL(aItem->Header());
	CleanupStack::Pop(dlg);

    TBool ret = dlg->ExecuteLD(CAknSettingPage::EUpdateWhenChanged);
    
    iWizard->SetContentSelectionL(list);
    
	CleanupStack::PopAndDestroy(list);

   	return param.iCommandId;
	}


//------------------------------------------------------------------------------
// CAspProfileWizardDialog::CheckValidity (from MAspEditorPageObserver)
//
//------------------------------------------------------------------------------
//
TBool CAspProfileWizardDialog::CheckValidity(const TDesC& aText, TInt aSettingType)
	{
	TInt ret = ETrue;
	
	TRAPD(err, ret = CheckValidityL(aText, aSettingType));
	
	if (err != KErrNone)
		{
		return ETrue;
		}
	
	return ret;
	}
	

//------------------------------------------------------------------------------
// CAspProfileWizardDialog::CheckValidityL (from MAspEditorPageObserver)
//
//------------------------------------------------------------------------------
//
TBool CAspProfileWizardDialog::CheckValidityL(const TDesC& aText, TInt aSettingType)
	{
	if (aSettingType == CAspWizardItem::ETypeProfileName)
		{
	    if (TUtil::IsEmpty(aText))
		    {
		    TDialogUtil::ShowInformationNoteL(R_ASP_NOTE_PROFILE_NAME_MISSING);
		    return EFalse;
		    }
		}
		
	if (aSettingType == CAspWizardItem::ETypeSyncContent)
		{
	    if (TUtil::IsEmpty(aText))
		    {
		    CAspWizardItem* item = iWizard->CurrentItem();
		    if (item->iErrorNoteResourceId == R_ASP_NOTE_CONTACTS_DB_MISSING)
		    	{
		    	TDialogUtil::ShowInformationNoteL(R_ASP_NOTE_CONTACTS_DB_MISSING);
		    	return EFalse;
		    	}
		    else if (item->iErrorNoteResourceId == R_ASP_NOTE_CALENDAR_DB_MISSING)
		    	{	
		    	TDialogUtil::ShowInformationNoteL(R_ASP_NOTE_CALENDAR_DB_MISSING);
		    	return EFalse;
		    	}
		    else if (item->iErrorNoteResourceId == R_ASP_NOTE_NOTES_DB_MISSING)
		    	{
		    	TDialogUtil::ShowInformationNoteL(R_ASP_NOTE_NOTES_DB_MISSING);
		    	return EFalse;
		    	}
		    else if (item->iErrorNoteResourceId == R_ASP_NOTE_EMAIL_DB_MISSING)
		    	{
		    	TDialogUtil::ShowInformationNoteL(R_ASP_NOTE_EMAIL_DB_MISSING);
		    	return EFalse;
		    	} 
		    else if (item->iErrorNoteResourceId == R_ASP_NOTE_MMS_DATABASE_MISSING)
		    	{
		    	TDialogUtil::ShowInformationNoteL(R_ASP_NOTE_MMS_DATABASE_MISSING);
		    	return EFalse;
		    	} 
		    else if (item->iErrorNoteResourceId == R_ASP_NOTE_SMS_DATABASE_MISSING)
		        {
		        TDialogUtil::ShowInformationNoteL(R_ASP_NOTE_SMS_DATABASE_MISSING);
		        return EFalse;
		        } 
		    else if (item->iErrorNoteResourceId == R_ASP_NOTE_BKM_DATABASE_MISSING)
		    	{
		    	TDialogUtil::ShowInformationNoteL(R_ASP_NOTE_BKM_DATABASE_MISSING);
		    	return EFalse;
		    	} 
		    else
		    	{
		    	TDialogUtil::ShowInformationNoteL(R_ASP_NOTE_REMOTE_DATABASE_MISSING);
		    	return EFalse;
    	    	}
    	        	    
    	    }
		}

	if (aSettingType == CAspWizardItem::ETypeHostAddress)
		{
	    if (TUtil::IsEmpty(aText))
		    {
		    TDialogUtil::ShowInformationNoteL(R_ASP_NOTE_HOST_ADDRESS_MISSING);
		    return EFalse;
		    }
		TInt bearer = iWizard->NumberValue(CAspWizardItem::ETypeBearerType);
		if	(bearer == EAspBearerInternet)		    
			{
				if (!TURIParser::IsValidUri(aText))
			    {
			    TDialogUtil::ShowInformationNoteL(R_ASP_NOTE_INCORRECT_HOST_ADDRESS);
			    return EFalse;
			    }
			}
			else if (bearer == EAspBearerBlueTooth)
			{
				if (!TURIParser::IsValidBluetoothHostAddress(aText))
			    {
			    TDialogUtil::ShowInformationNoteL(R_ASP_NOTE_INCORRECT_HOST_ADDRESS);
			    return EFalse;
			    }
			}
		}
    
	if (aSettingType == CAspWizardItem::ETypeServerId)
		{
	    if (!iProfileList->IsUniqueServerId(aText, KErrNotFound))
		    {
		    TDialogUtil::ShowInformationNoteL(R_ASP_NOTE_INCORRECT_SERVER_ID);
		    return EFalse;
		    }
		}

	if (aSettingType == CAspWizardItem::ETypeContentSelection)
		{
		TInt adapterId = iWizard->ContentId(aText);
		if (adapterId == KUidNSmlAdapterEMail.iUid)
			{
		    TInt protocol = iWizard->NumberValue(CAspWizardItem::ETypeProtocolVersion);
		   	
		   	
		   	CDesCArray* arr = iWizard->LocalDatabaseList(KUidNSmlAdapterEMail.iUid);
	        if (!arr)
		        {
		        TDialogUtil::ShowErrorNoteL(R_ASP_NO_MAILBOXES);
		        return EFalse;  // no syncml mailbox
		        }
		
	        if (arr->Count() == 0) 
		        {
		        TDialogUtil::ShowErrorNoteL(R_ASP_NO_MAILBOXES);
		        return EFalse;   // no syncml mailbox
		        }
			}
		}

	

	return ETrue;
	}


//------------------------------------------------------------------------------
// CAspProfileWizardDialog::GetNavipaneText (from MAspEditorPageObserver)
//
//------------------------------------------------------------------------------
//
void CAspProfileWizardDialog::GetNavipaneText(TDes& aText, TInt aContentCount)
	{
    CAspWizardItem* item = iWizard->CurrentItem();
    
    TInt total = iWizard->NumOfEnabledItems();
    total--; // start page not included in page count
    
    TInt current = iWizard->NumOfCurrentItem(item);
    current--; // start page not included in page count
    
    TInt enabledContentCount = iWizard->NumOfEnabledContentItems();
    
    total = total - enabledContentCount;
    total = total + aContentCount;

    GetNaviPaneText(aText, current, total);
	}


//------------------------------------------------------------------------------
// CAspProfileWizardDialog::HandleEditorEvent (from MAspEditorPageObserver)
//
//------------------------------------------------------------------------------
//
void CAspProfileWizardDialog::HandleEditorEvent(TInt aEvent, TInt /*aValue*/)
	{
	if (aEvent == MAspEditorPageObserver::EKeyEvent)
		{
		HidePopupNote();
		TRAP_IGNORE( ShowPopupNoteL(KErrNotFound, KMsToWaitBeforeRePopup) );
		}
	}
	

// -----------------------------------------------------------------------------
// CAspProfileWizardDialog::SetDefaultValuesL
// 
// -----------------------------------------------------------------------------
//
void CAspProfileWizardDialog::SetDefaultValuesL()
	{
	// use existing profile list for name creation
	if (iDialogParam->iProfileList)
		{
	    HBufC* hBuf = CAspProfile::GetNewProfileNameLC(
	                  iDialogParam->iProfileList, iApplicationId);
	    iWizard->SetStringValueL(hBuf->Des(), CAspWizardItem::ETypeProfileName);  
	    CleanupStack::PopAndDestroy(hBuf);
		}
	else
		{
		// create profile list for name creation
	    TAspParam param(EApplicationIdSync, iSyncSession);
        CAspProfileList* list =  CAspProfileList::NewLC(param);
	    list->ReadAllProfilesL(CAspProfileList::EBasePropertiesOnly);
	
	    HBufC* hBuf = CAspProfile::GetNewProfileNameLC(list, iApplicationId);
	    iWizard->SetStringValueL(hBuf->Des(), CAspWizardItem::ETypeProfileName);
	    CleanupStack::PopAndDestroy(hBuf);
	
	    CleanupStack::PopAndDestroy(list);
		}
		
	if (iBearerHandler)
		{
		TInt bearerType = iBearerHandler->DefaultBearer();
		iWizard->SetNumberValue(bearerType, CAspWizardItem::ETypeBearerType);
		}
		
	TInt num = EAspProtocol_1_2;
	iWizard->SetNumberValue(num, CAspWizardItem::ETypeProtocolVersion);
	}


// -----------------------------------------------------------------------------
// CAspProfileWizardDialog::GetNaviPaneText
//
// Function constructs navi pane text. Text format is:
// "Page %0N/%1N"  (eg "Page 5/6")
// -----------------------------------------------------------------------------
//
void CAspProfileWizardDialog::GetNaviPaneText(TDes& aText, TInt aCurrent, TInt aFinal)
	{
    TBuf <KBufSize> buf;
    
    // replace  %0N with aCurrent
    StringLoader::Format(buf, iNaviPaneTextFormat, 0, aCurrent);
    
    // replace %1N with aFinal
    StringLoader::Format(aText, buf, 1, aFinal);
    
    AknTextUtils::DisplayTextLanguageSpecificNumberConversion(aText);
	}


// -----------------------------------------------------------------------------
// CAspProfileWizardDialog::GetNaviPaneText
//
// -----------------------------------------------------------------------------
//
void CAspProfileWizardDialog::GetNaviPaneText(TDes& aText, CAspWizardItem* aItem)
	{
    TInt total = iWizard->NumOfEnabledItems();
    total--; // start page not included in page count
    
    TInt current = iWizard->NumOfCurrentItem(aItem);
    current--; // start page not included in page count

    GetNaviPaneText(aText, current, total);
	}


// -----------------------------------------------------------------------------
// CAspProfileWizardDialog::UpdateCbaL
// 
// -----------------------------------------------------------------------------
//
void CAspProfileWizardDialog::UpdateCbaL(TInt aResourceId)
    {
    CEikButtonGroupContainer& cba = ButtonGroupContainer();
    cba.SetCommandSetL(aResourceId);
    cba.DrawDeferred();
    }


// -----------------------------------------------------------------------------
// CAspProfileWizardDialog::SetTextEditorTextL
//
// -----------------------------------------------------------------------------
//
void CAspProfileWizardDialog::SetTextEditorTextL(TInt aResource)
	{
	if(iTextEditorText)
		{
		delete iTextEditorText;
		iTextEditorText = NULL;		
		}
	if (iMoveBack)
		{
		TRect mainPane;
    	AknLayoutUtils::LayoutMetricsRect(AknLayoutUtils::EMainPane, mainPane);        
    	iTextEditor->SetRect(mainPane);
		}
	iTextEditorText = CAspResHandler::ReadL(aResource);
	UpdateTextColorL() ;
	iTextEditor->SetTextL(iTextEditorText);
	}


// -----------------------------------------------------------------------------
// CAspProfileWizardDialog::ShowPopupNote
//
// -----------------------------------------------------------------------------
//
void CAspProfileWizardDialog::ShowPopupNoteL(TInt aResource, TInt aDelay) 
    {
    if (aResource != KErrNotFound)
    	{
    	HBufC* hBuf = CAspResHandler::ReadLC(aResource);    
        iPopupNote->SetTextL(hBuf->Des());
        CleanupStack::PopAndDestroy(hBuf);
    	}
    	
    iPopupNote->SetTimePopupInView(KMsTimePopupInView);    
    iPopupNote->SetTimeDelayBeforeShow(aDelay);
    iPopupNote->ShowInfoPopupNote();    
    }


// -----------------------------------------------------------------------------
// CAspProfileWizardDialog::HidePopupNote
//
// -----------------------------------------------------------------------------
//
void CAspProfileWizardDialog::HidePopupNote() 
    {
    iPopupNote->HideInfoPopupNote();
    }


// -----------------------------------------------------------------------------
// CAspProfileWizardDialog::ShowErrorNoteL
//
// -----------------------------------------------------------------------------
//
void CAspProfileWizardDialog::ShowErrorNoteL(TInt aError)
	{
	if (aError == KErrNoMemory || aError == KErrDiskFull)
		{
		CTextResolver* tr = CTextResolver::NewLC();
		iBuf = tr->ResolveErrorString(aError, CTextResolver::ECtxNoCtxNoSeparator);
     	CleanupStack::PopAndDestroy(tr);
		}
	else
		{
		CAspResHandler::ReadL(iBuf, R_ASP_LOG_ERR_PROFILE_WIZARD);
		}
		
    TDialogUtil::ShowErrorNoteL(iBuf);
	}

// -----------------------------------------------------------------------------
// CAspProfileWizardDialog::SetBackgroundContextL
//
// -----------------------------------------------------------------------------
//
void CAspProfileWizardDialog::SetBackgroundContextL( )
{
	TRect mainPane( 0,0,0,0 );
		
	AknLayoutUtils::LayoutMetricsRect( AknLayoutUtils::EScreen, mainPane );
	
	if (iBgContext)
		{
		delete iBgContext ;
		iBgContext = NULL ;
		}
	
	iBgContext = CAknsBasicBackgroundControlContext::NewL( 
                        KAknsIIDQsnBgAreaMain, mainPane, EFalse );
		
	iTextEditor->SetSkinBackgroundControlContextL( iBgContext );                            
}


// -----------------------------------------------------------------------------
// CAspProfileWizardDialog::UpdateTextColorL
//
// -----------------------------------------------------------------------------
//
void CAspProfileWizardDialog::UpdateTextColorL() 
	{
	TRgb textColor( KRgbBlack );
	AknsUtils::GetCachedColor( AknsUtils::SkinInstance(), 
	                           textColor,
	                           KAknsIIDQsnTextColors,
	                           EAknsCIQsnTextColorsCG6 );


	const CFont* sysfont = AknLayoutUtils::FontFromId(EAknLogicalFontSecondaryFont); 
	TFontSpec sysfontspec = sysfont->FontSpecInTwips();

	TCharFormat charFormat;
	TCharFormatMask charFormatMask;
	charFormat.iFontSpec = sysfontspec;

	charFormat.iFontPresentation.iTextColor = textColor; 
	charFormatMask.SetAll();
	charFormatMask.SetAttrib( EAttFontHeight );
	charFormatMask.SetAttrib( EAttColor );
	CCharFormatLayer* formatLayer = CCharFormatLayer::NewL(charFormat, charFormatMask);
	iTextEditor->SetCharFormatLayer(formatLayer);
	}

//  End of File  
