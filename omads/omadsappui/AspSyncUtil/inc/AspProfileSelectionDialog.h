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
* Description: Dialog to select profile to be used for scheduled sync 
*
*/



#ifndef ASPPROFILESELECTIONDIALOG_H
#define ASPPROFILESELECTIONDIALOG_H


//  INCLUDES

#include "AspDialogUtil.h"

#include <e32base.h>
#include <aknlists.h>
#include <AknDialog.h>
#include <aknselectionlist.h>

// FORWARD DECLARATIONS



// CLASS DECLARATION


/**
*  CAspProfileSelectionDialog
*
* CAspProfileSelectionDialog is used for selecting auto sync profile.
*/
NONSHARABLE_CLASS (CAspProfileSelectionDialog) : public CAknDialog, public MEikListBoxObserver
    {
    
	public:
	    /**
        * Launches dialog.
        * @param aParam Class that contains dialog parameters.
        * @return Completion code.
        */
		static TBool ShowDialogL(TAspParam& aParam);

	public:// Constructors and destructor

        /**
        * Two-phased constructor.
        */
		static CAspProfileSelectionDialog* NewL(TAspParam& aParam);
        
        /**
        * Destructor.
        */
        virtual ~CAspProfileSelectionDialog();
    
    private:

        /**
        * C++ default constructor.
        */
        CAspProfileSelectionDialog(TAspParam& aParam);
	
        /**
        * By default Symbian OS constructor is private.
        */
        void ConstructL();

	private:
        /**
        * From MEikListBoxObserver, called by framework.
        * @param aListBox.
        * @param aEventType.
		* @return None
        */
		void HandleListBoxEventL(CEikListBox* aListBox, TListBoxEvent aEventType);

        /**
        * From the base class.
		* Called by framework before the dialog is shown.
        * @param None
		* @return None
        */
        void PreLayoutDynInitL();

        /**
        * From the base class.
		* Called by framework before exiting the dialog.
        * @param aButtonId Button id.
		* @return ETrue to exit\ EFalse to not to exit.
        */
        TBool OkToExitL(TInt aButtonId);

        /**
        * From the base class.
		* Called by framework for key event handling.
        * @param aKeyEvent.
		* @param aType.
		* @return Return code.
        */
		TKeyResponse OfferKeyEventL(const TKeyEvent& aKeyEvent,TEventCode aType);

        /**
        * From the base class.
		* Called by framework when dialog is activated.
        * @param None.
		* @return None.
        */
		void ActivateL();
		
		/**
		* Method to get context sensitive help topic.
        * @param aContext Help topic to open.
		* @return None.
        */
		void GetHelpContext(TCoeHelpContext& aContext) const;

		/**
        * From the base class.
		* Called by framework before menu is shown.
        * @param aResourceId Menu resource id.
		* @param aMenuPane Pointer to the menu.
        * @return None.
        */
		void DynInitMenuPaneL(TInt aResourceID, CEikMenuPane* aMenuPane);

		/**
		* From the base class.
		* Handles menu events.
		* @param  aCommandId Command id.
		* @return None.
		*/
		void ProcessCommandL(TInt aCommandId);
		
		/**
		* From base class.
		* Called when UI layout changes. 
		* @param aType.
    	* @return None.
		*/
        void HandleResourceChange(TInt aType);

    private:
		/**
		* Handles setting item editing.
		* @param None.
		* @return None.
		*/
		void HandleOKL();
		
		/**
		* Set setting list icons.
		* @param None.
		* @return None.
		*/
		void SetIconsL();
		
		/**
		* Shown sync profile content list.
		* @param None.
		* @return None.
		*/
		void ShowContentListL();


    private:
		
		/**
        * Add settings titles into listbox.
        * @param None.
        * @return None.
        */
		void UpdateListBoxL();
		
		/**
		* Utility function, updates dialog cba.
		* @param None.
		* @return None.
		*/
		void UpdateCbaL();
		

    private:

		/**
        * Creates icon.
		* @param aId.
        * @param aFileName.
		* @param aFileIndex.
		* @param aFileMaskIndex.
        * @return Created icon.
        */
		CGulIcon* IconL(TAknsItemID aId, const TDesC& aFileName, TInt aFileIndex, TInt aFileMaskIndex);
		


	private:
		// dialog parameter for returning value to caller
		TAspParam* iDialogParam;

  		// id of the calling application
		TInt iApplicationId;
		
		// currently selected profile
		TInt iProfileId;

		// setting list
		CAknSingleGraphicStyleListBox* iListBox;

   		// sync session
		RSyncMLSession* iSyncSession;

		// for title and icon handling
		CStatusPaneHandler* iStatusPaneHandler;

		// long buffer for string handling
		TBuf<KBufSize255> iBuf;
		
		// list that contains all profiles
		CAspProfileList* iProfileList;
		
		// list that contains all contents (applications)
		CAspContentList* iContentList;
		
		// id of current cba resource
		TInt iCommandSetId;
    };


#endif      // ASPPROFILESELECTIONDIALOG_H
            
// End of File
