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
* Description: sync progress indication 
*
*/


#ifndef ASPPROGRESSDIALOG_H
#define ASPPROGRESSDIALOG_H


//  INCLUDES
#include    <e32base.h>
#include    <AknWaitDialog.h>
#include    <AknProgressDialog.h>

#include    "AspDefines.h"
#include    "AspResHandler.h"


// CLASS DECLARATIONS


/**
* MAspProgressDialogObserver
* 
* Observer interface function for observing progress dialog.
*/
NONSHARABLE_CLASS (MAspProgressDialogObserver)
    {
    public:
        /**
        * Callback method
        * Get's called when a dialog is dismissed.
        */
        virtual void HandleDialogEventL(TInt aButtonId) = 0;
    };



/**
* CAspProgressDialog2
*
* CAspProgressDialog2 is needed to modify CAknProgressDialog::OkToExitL. 
*/
NONSHARABLE_CLASS (CAspProgressDialog2) : public CAknProgressDialog
    {
	public:
        /**
        * C++ default constructor.
        */
		CAspProgressDialog2(CEikDialog** aSelfPtr, TInt aApplicationId);

        /**
        * Destructor.
        */
        virtual ~CAspProgressDialog2();
        
        /**
        * From the base class.
		* Called by framework before exiting the dialog.
        * @param Button id.
		* @return ETrue to exit\ EFalse to not to exit.
        */
        TBool OkToExitL(TInt aButtonId);
        
        /**
        * From the base class.
		* Called by framework for key event handling.
        * @param aKeyEvent.
		* @param TEventCode.
		* @return Return code.
        */
		TKeyResponse OfferKeyEventL(const TKeyEvent& aKeyEvent,TEventCode aType);
		
	private:
   		// has dialog received close event (eg. end key) 
		TBool iCloseEventReceived;

		// has cancel key been pressed 
		TBool iCancelEventReceived;

        // has sync been closed
        TBool iClosed;
        
        // has sync been canceled
        TBool iCanceled;
		
  		// id of the calling application
		TInt iApplicationId;
    };



/**
* CAspProgressDialog
*  
* CAspProgressDialog show sync progress dialog.
*/
NONSHARABLE_CLASS (CAspProgressDialog) : public CBase, public MProgressDialogCallback
    {

    public:// Constructors and destructor

        /**
        * Two-phased constructor.
        */
        static CAspProgressDialog* NewL(MAspProgressDialogObserver* aObserver);
        
        /**
        * Destructor.
        */
        virtual ~CAspProgressDialog();
    
    private:

        /**
        * C++ default constructor.
        */
        CAspProgressDialog(MAspProgressDialogObserver* aObserverPtr);
	
        /**
        * By default Symbian OS constructor is private.
        */
        void ConstructL();
        
		/**
		* From MProgressDialogCallback.
        * Called by the framework when dialog is dismissed.
        * @param aButtonId
        * @return None
        */
        void DialogDismissedL(TInt aButtonId);

    public:        
        
        /**
        * Launches progress dialog with custom bitmap.
        * @param aLabel.
        * @param aId.
        * @param aFileName.
        * @param aFileBitmapId.
        * @param aFileMaskId.
        * @return None.
        */
        void LaunchProgressDialogL(const TDesC& aLabel, TAknsItemID aId, const TDesC& aFileName, TInt aFileBitmapId, TInt aFileMaskId);

        
        /**
        * Closes progress dialog.
        * @param None
        * @return None
        */
        void CancelProgressDialogL();
        
        /**
        * Update dialog label text.
        * @param aText.
        * @return None.
        */
        void UpdateTextL(const TDesC& aText);

        /**
        * Set progress bar state.
        * @param aValue.
        * @return None
        */
        void SetProgress(TInt aValue);

        /**
        * Set progress bar state.
        * @param aFinalValue.
        * @return None
        */
        void SetFinalProgress(TInt aFinalValue);

        /**
        * Gets CAknNoteControl from CAknProgressDialog.
        * @param None
        * @return CAknNoteControl.
        */
		CAknNoteControl* NoteControl();

       /**
        * Cancels animation.
        * @param None.
        * @return None.
        */
		void CancelAnimation();
        
		/**
        * Starts animation.
        * @param None
        * @return None.
        */
		void StartAnimationL();
		
		/**
        * Hides cancel button.
        * @param None
        * @return None.
        */
		void HideButtonL();

		/**
        * Set application id.
        * @param aApplicationId
        * @return None.
        */
		void SetApplicationId(TInt aApplicationId);

		/**
        * Returns progress dialog
        * @param None
        * @return CAspProgressDialog2*
        */
		CAspProgressDialog2* ProgressDialog();

    private:
 		// id of the calling application
		TInt iApplicationId;
    
        // progress dialog
		CAspProgressDialog2*	iProgressDialog;
		
		// is dialog running
		TBool iProgressDialogRunning;

        // progress dialog info
        CEikProgressInfo* iProgressInfo;

		// last label text
		TBuf<KBufSize> iLastText;

		// animation state
		TBool iAnimation;

		// resource handler
		CAspResHandler* iResHandler;

		// dialog observer
        MAspProgressDialogObserver*	iObserver;
    };




#endif      // ASPPROGRESSDIALOG_H
            
// End of File
