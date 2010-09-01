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
#include "AspProgressDialog.h"
#include "AspSyncUtil.rh"
#include "AspUtil.h"
#include "AspDebug.h"
#include "AspDefines.h"
#include "AspResHandler.h"
#include "AspDialogUtil.h"


#include <eikprogi.h>
#include <notecontrol.h>



// ============================ MEMBER FUNCTIONS ===============================



/*******************************************************************************
 * class CAspProgressDialog
 *******************************************************************************/


// -----------------------------------------------------------------------------
// CAspProgressDialog::CAspProgressDialog
//
// -----------------------------------------------------------------------------
//
CAspProgressDialog::CAspProgressDialog(MAspProgressDialogObserver* aObserver) : iObserver(aObserver)
    {
    FLOG( _L("CAspProgressDialog: CAspProgressDialog") );
    }


// -----------------------------------------------------------------------------
// CAspProgressDialog::NewL
//
// -----------------------------------------------------------------------------
//
CAspProgressDialog* CAspProgressDialog::NewL(MAspProgressDialogObserver* aObserver)
    {
    FLOG( _L("CAspProgressDialog::NewL START") );

    CAspProgressDialog* self = new (ELeave) CAspProgressDialog(aObserver);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);

    FLOG( _L("CAspProgressDialog::NewL END") );
    return(self);
    }


// -----------------------------------------------------------------------------
// CAspProgressDialog::ConstructL
//
// -----------------------------------------------------------------------------
//
void CAspProgressDialog::ConstructL()
    {
    FLOG( _L("CAspProgressDialog::ConstructL START") );

	iLastText = KNullDesC;
	iAnimation = EFalse;
	iResHandler = CAspResHandler::NewL();
	iProgressDialogRunning = EFalse;
    
	FLOG( _L("CAspProgressDialog::ConstructL END") );
    } 


// -----------------------------------------------------------------------------
// Destructor
//
// -----------------------------------------------------------------------------
//
CAspProgressDialog::~CAspProgressDialog()
    {
    FLOG( _L("CAspProgressDialog::~CAspProgressDialog START") );

	delete iResHandler;
    delete iProgressDialog;

	FLOG( _L("CAspProgressDialog::~CAspProgressDialog END") );
    }


// -----------------------------------------------------------------------------
// CAspProgressDialog::LaunchProgressDialogL
//
// -----------------------------------------------------------------------------
//
void CAspProgressDialog::LaunchProgressDialogL(const TDesC& aLabel, TAknsItemID aId, const TDesC& aFileName, TInt aFileBitmapId, TInt aFileMaskId)
	{
    FLOG( _L("CAspProgressDialog::LaunchProgressDialogL START") );

	iProgressDialog = new(ELeave) CAspProgressDialog2(reinterpret_cast<CEikDialog**>(&iProgressDialog), iApplicationId);
    
	iProgressDialog->PrepareLC(R_ASP_PROGRESS_NOTE);
    iProgressDialog->SetTextL(aLabel);
	
	CEikImage* image = TDialogUtil::CreateImageLC(aId, aFileName, aFileBitmapId, aFileMaskId);
    iProgressDialog->SetImageL(image);
	CleanupStack::Pop(image);

    // get reference to progress info bar
    iProgressInfo = iProgressDialog->GetProgressInfoL();
    iProgressDialog->SetCallback(this);
    
    iProgressDialog->RunLD();
    iProgressDialogRunning = ETrue;

	FLOG( _L("CAspProgressDialog::LaunchProgressDialogL END") );
	}


// -----------------------------------------------------------------------------
// CAspProgressDialog::CancelProgressDialogL
//
// -----------------------------------------------------------------------------
//
void CAspProgressDialog::CancelProgressDialogL()
    {
    FLOG( _L("CAspProgressDialog::CancelProgressDialogL START") );

	if (!iProgressDialogRunning)
		{
		return; // dialog is already closed
		}
		
	if(iProgressDialog)
        {        
        iProgressDialog->ProcessFinishedL();
        iProgressDialogRunning = EFalse;
        }    

	FLOG( _L("CAspProgressDialog::CancelProgressDialogL END") );
    }

// -----------------------------------------------------------------------------
// CAspProgressDialog::ProgressDialog
//
// -----------------------------------------------------------------------------
//
CAspProgressDialog2* CAspProgressDialog::ProgressDialog()
	{
	return iProgressDialog;
	}

// -----------------------------------------------------------------------------
// CAspProgressDialog::UpdateTextL
//
// -----------------------------------------------------------------------------
//
void CAspProgressDialog::UpdateTextL(const TDesC& aText)
    {
    FLOG( _L("CAspProgressDialog::UpdateTextL START") );

	__ASSERT_DEBUG(iProgressDialog, TUtil::Panic(KErrGeneral));

	if (iLastText.Compare(aText) == 0)
		{
		return;  // text has not changed
		}
    
    iProgressDialog->SetTextL(aText);
	TUtil::StrCopy(iLastText, aText);
	
	FLOG( _L("aText is '%S'"), &aText );

	FLOG( _L("CAspProgressDialog::UpdateTextL END") );
    }


// -----------------------------------------------------------------------------
// CAspProgressDialog::SetProgress
//
// -----------------------------------------------------------------------------
//
void CAspProgressDialog::SetProgress(TInt aValue)
    {
    FLOG( _L("CAspProgressDialog::SetProgress START") );
    
	__ASSERT_DEBUG(iProgressDialog  && iProgressInfo, TUtil::Panic(KErrGeneral));

	CEikProgressInfo::SInfo info = iProgressInfo->Info();

	TInt val = aValue;
	if (val > info.iFinalValue)
		{
		val = info.iFinalValue;
		}
	
	iProgressInfo->SetAndDraw(val);
	
	FLOG( _L("aValue is %d"), aValue );
    
	FLOG( _L("CAspProgressDialog::SetProgress END") );
    }


// -----------------------------------------------------------------------------
// CAspProgressDialog::SetFinalProgress
//
// -----------------------------------------------------------------------------
//
void CAspProgressDialog::SetFinalProgress(TInt aFinalValue)
	{
    FLOG( _L("CAspProgressDialog::SetFinalProgress START") );
    
    __ASSERT_DEBUG(iProgressInfo, TUtil::Panic(KErrGeneral));

    if (aFinalValue < 0)
    	{
    	iProgressInfo->SetFinalValue(0);
    	}
    else
    	{
    	iProgressInfo->SetFinalValue(aFinalValue);
    	}
    	
	iProgressInfo->SetAndDraw(0);

	FLOG( _L("aFinalValue is %d"), aFinalValue );
	
	FLOG( _L("CAspProgressDialog::SetFinalProgress END") );
	}


// -----------------------------------------------------------------------------
// CAspProgressDialog::NoteControl
//
// -----------------------------------------------------------------------------
//
CAknNoteControl* CAspProgressDialog::NoteControl()
	{
    CAknNoteControl* note = STATIC_CAST(CAknNoteControl*, iProgressDialog->ControlOrNull(EAspSyncProgressNote));

	__ASSERT_DEBUG(note, TUtil::Panic(KErrGeneral));

	return note;
	}


// -----------------------------------------------------------------------------
// CAspProgressDialog::CancelAnimation
//
// -----------------------------------------------------------------------------
//
void CAspProgressDialog::CancelAnimation()
	{
	FLOG( _L("CAspProgressDialog::CancelAnimation START") );
	
	NoteControl()->CancelAnimation();
	iAnimation = EFalse;

	FLOG( _L("CAspProgressDialog::CancelAnimation END") );
	}


// ----------------------------------------------------------------------------
// CAspProgressDialog::StartAnimationL
//
// ----------------------------------------------------------------------------
//
void CAspProgressDialog::StartAnimationL()
	{
	FLOG( _L("CAspProgressDialog::StartAnimationL START") );
	
	if (!iAnimation)
		{
		NoteControl()->StartAnimationL();
		iAnimation = ETrue;
		
		FLOG( _L("animation started") );
		}
		
    FLOG( _L("CAspProgressDialog::StartAnimationL END") );
	}


// -----------------------------------------------------------------------------
// CAspProgressDialog::DialogDismissedL (From MProgressDialogCallback)
//
// -----------------------------------------------------------------------------
//
void CAspProgressDialog::DialogDismissedL(TInt aButtonId)
    {
    FLOG( _L("CAspProgressDialog::DialogDismissedL START" ) );
  
    if (!iObserver)
		{
		FLOG( _L("### no observer ###") );
		FLOG( _L("CAspProgressDialog::DialogDismissedL END") );
		return; // no observer
		}
	
	// end key handling
	if (aButtonId == EKeyPhoneEnd)
	    {
        TRAP_IGNORE(iObserver->HandleDialogEventL(EKeyPhoneEnd));
        }

	// this dialog only sends one cancel event - other events are ignored 
	if (aButtonId == EEikBidCancel)
		{
        TRAP_IGNORE(iObserver->HandleDialogEventL(EEikBidCancel));
        }

    FLOG( _L("CAspProgressDialog::DialogDismissedL END") );
    }


// -----------------------------------------------------------------------------
// CAspProgressDialog::HideButtonL
//
// -----------------------------------------------------------------------------
//
void CAspProgressDialog::HideButtonL()
    {
    const TInt KLastButtonIndex = 2;
    
   	CEikButtonGroupContainer& container = iProgressDialog->ButtonGroupContainer();
	
	container.RemoveCommandFromStack(KLastButtonIndex, EAknSoftkeyCancel);
	container.AddCommandToStackL(KLastButtonIndex, EAknSoftkeyEmpty, KNullDesC);
	container.DrawDeferred();
    }


// ----------------------------------------------------------------------------
// CAspProgressDialog::SetApplicationId
// 
// ----------------------------------------------------------------------------
//
void CAspProgressDialog::SetApplicationId(TInt aApplicationId)
	{
	iApplicationId = aApplicationId;
	}





/*******************************************************************************
 * class CAspProgressDialog2
 *******************************************************************************/


// -----------------------------------------------------------------------------
// CAspProgressDialog2::CAspProgressDialog2
//
// -----------------------------------------------------------------------------
//
CAspProgressDialog2::CAspProgressDialog2(CEikDialog** aSelfPtr, TInt aApplicationId)
 : CAknProgressDialog(aSelfPtr)
	{
	FLOG( _L("CAspProgressDialog2::CAspProgressDialog2 START") );
	
    iCloseEventReceived = EFalse;
    iCancelEventReceived = EFalse;
    iClosed = EFalse;
    iCanceled = EFalse;
	iApplicationId = aApplicationId;
	
	if (iApplicationId != EApplicationIdSync)
		{
		// to make sure that wserv does not change our priority even we lose foreground.
	    // this is needed for receiving event EKeyPhoneEnd without delay.
	    
	    RWsSession& wsSession = iEikonEnv->WsSession();
	    wsSession.ComputeMode(RWsSession::EPriorityControlDisabled);
	    
	    FLOG( _L("RWsSession::ComputeMode(RWsSession::EPriorityControlDisabled)") );
	    
		}
		
	FLOG( _L("CAspProgressDialog2::CAspProgressDialog2 END") );
	}


// -----------------------------------------------------------------------------
// Destructor
//
// -----------------------------------------------------------------------------
//
CAspProgressDialog2::~CAspProgressDialog2()
    {
    FLOG( _L("CAspProgressDialog2::~CAspProgressDialog2 START") );

	if (iApplicationId != EApplicationIdSync)
		{
		RWsSession& wsSession = iEikonEnv->WsSession();
	    wsSession.ComputeMode(RWsSession::EPriorityControlComputeOff);
	    
	    FLOG( _L("RWsSession::ComputeMode(RWsSession::EPriorityControlComputeOff)") );
		}

    if (iAvkonAppUi)
        {
        iAvkonAppUi->RemoveFromStack(this);
        }

	FLOG( _L("CAspProgressDialog2::~CAspProgressDialog2 END") );
    }


// -----------------------------------------------------------------------------
// CAspProgressDialog2::OkToExitL
//
// -----------------------------------------------------------------------------
//
TBool CAspProgressDialog2::OkToExitL(TInt aButtonId)
    {
    FLOG( _L("CAspProgressDialog2::OkToExitL START") );
    
    // close key handling
    if (iCloseEventReceived && !iClosed)
        {
   	    if (iCallback)
   	    	{
   	    	TRAP_IGNORE(iCallback->DialogDismissedL(EKeyPhoneEnd));
   	    	
   	    	FLOG( _L("CAspProgressDialog2::OkToExitL: close handling done") );
  	    	}
   		    
        if (iAvkonAppUi)
            {
            // prevent further close events
            iAvkonAppUi->RemoveFromStack(this);
            }
            
        FLOG( _L("CAspProgressDialog2::OkToExitL END") );

        iClosed = ETrue;
        return ETrue;
        }

    // cancel key handling
    if (aButtonId == EEikBidCancel && iCancelEventReceived && !iCanceled && !iClosed)
        {
        if (iCallback)
        	{
        	TRAP_IGNORE(iCallback->DialogDismissedL(EEikBidCancel));
        	
        	FLOG( _L("CAspProgressDialog2::OkToExitL: cancel key handling done") );
        	}
        
		FLOG( _L("CAspProgressDialog2::OkToExitL END") );
		
		iCanceled = ETrue;
		return EFalse; // leave dialog open - let observer close it
		}


    FLOG( _L("CAspProgressDialog2::OkToExitL END") );
    
    return CAknProgressDialog::OkToExitL(aButtonId);
    }


// ----------------------------------------------------------------------------
// CAspProgressDialog2::OfferKeyEventL
// 
// ----------------------------------------------------------------------------
//
TKeyResponse CAspProgressDialog2::OfferKeyEventL(const TKeyEvent& aKeyEvent,TEventCode aType)
	{
	FLOG( _L("CAspProgressDialog2::OfferKeyEventL START") );
	
	if (aKeyEvent.iScanCode == EStdKeyDevice1)
		{
		iCancelEventReceived = ETrue;
		FLOG( _L("CAspProgressDialog2::OfferKeyEventL: EStdKeyDevice1 received") );
     	}
		

	if (aType == EEventKey)
		{
		switch (aKeyEvent.iCode)
			{

            case EKeyEscape:  // iCode 27 (0x1b)
            	{
            	iCloseEventReceived = ETrue;
            	FLOG( _L("CAspProgressDialog2::OfferKeyEventL: EKeyEscape received") );
            	break;
            	}
            	
            case EKeyPhoneEnd:
             	{
             	iCloseEventReceived = ETrue;
                FLOG( _L("CAspProgressDialog2::OfferKeyEventL: EKeyPhoneEnd received") );
                break;
               	}

            default:
                FLOG( _L("event key code is %d"),  aKeyEvent.iCode);        
			    break;
			}
		}


    FLOG( _L("CAspProgressDialog2::OfferKeyEventL END") );
    
	return CAknProgressDialog::OfferKeyEventL(aKeyEvent, aType);
	}




//  End of File  
