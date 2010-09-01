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
* Description: Editing Content list (not used anymore) 
*
*/


#ifndef ASPCONTENTLISTDIALOG_H
#define ASPCONTENTLISTDIALOG_H


//  INCLUDES
#include <e32base.h>
#include <eikdialg.h>
#include <eiklbo.h>
#include <aknlists.h>

#include "aspdefines.h"
#include "asputil.h"
#include "aspdialogutil.h"
#include "aspreshandler.h"

#include <aknlistquerydialog.h>



// CLASS DECLARATION

/**
* CAspContentListDialog
*  
* Content list dialog shows all available sync contents (sync data providers).
*/
NONSHARABLE_CLASS (CAspContentListDialog) : public CAknDialog, public MEikListBoxObserver
    {
    enum TAspContentListDialogIconIndex
		{
	    EIconBothDirections = 0,
	    EIconDeviceToServer = 1,
	    EIconServerToDevice = 2,
	    EIconNotDefined     = 3
		};

	public:

	    /**
        * Launches setting dialog.
        * @param aParam Class that contains dialog parameters.
        * @return Completion code.
        */
		static TBool ShowDialogL(const TAspParam& aParam);

    public:// Constructors and destructor

        /**
        * Two-phased constructor.
        */
        static CAspContentListDialog* NewL(const TAspParam& aParam);
        
        /**
        * Destructor.
        */
        virtual ~CAspContentListDialog();
    
    private:

        /**
        * C++ default constructor.
        */
        CAspContentListDialog(const TAspParam& aParam);
	
        /**
        * By default Symbian OS constructor is private.
        */
        void ConstructL();


	private:
        /**
        * From the base class.
		* Called by framework before the dialog is shown.
        * @param None
		* @return None
        */
		void PreLayoutDynInitL();

		/**
		* From the base class.
		* Handles menu events.
		* @param  aCommandId Command id.
		* @return None.
		*/
		void ProcessCommandL(TInt aCommandId);

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

		/**
        * From the base class.
		* Called by framework before menu is shown.
        * @param aResourceId Menu resource id.
		* @param aMenuPane Pointer to the menu.
        * @return None.
        */
		void DynInitMenuPaneL(TInt aResourceID,	CEikMenuPane* aMenuPane);

	   /**
		* From the base class.
        * Called when dialog is activated.
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
		* From base class.
		* Called when UI layout changes. 
		* @param aType.
    	* @return None.
		*/
        void HandleResourceChange(TInt aType);

    private: // from MEikListBoxObserver

	    /**
        * From MEikListBoxObserver
        * @param aListBox Pointer to listbox.
		* @param aEventType Event type.
        * @return None.
        */
		void HandleListBoxEventL(CEikListBox* aListBox, TListBoxEvent aEventType);

	
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


		/**
        * Updates listbox items.
        * @param None.
        * @return None.
        */
		void UpdateListBoxL();

		/**
        * Shows content dialog.
        * @param None.
        * @return None.
        */
		void ShowContentL();

    private:
		/**
		* Utility function.
		* @return Sync session.
        */
		RSyncMLSession& Session();

		/**
        * Utility function.
        * @return Dialog list box.
        */
		CAknDoubleLargeStyleListBox* ListBox();

		/**
        * Utility function.
        * @return Currently selected list box item.
        */
		TAspProviderItem& SelectedProvider();


    private:
		// id of the calling application
		TInt iApplicationId;

		// sync profile
		CAspProfile* iProfile;
		
		// sync session
		RSyncMLSession* iSyncSession;

		// list box that contains all contents (applications)
		CAknDoubleLargeStyleListBox* iSettingListBox;

		// for title and icon handling
		CStatusPaneHandler* iStatusPaneHandler;

    	// resource handler
		CAspResHandler* iResHandler;

		// list that contains all contents (applications)
		CAspContentList* iContentList;
    };


#endif    // ASPCONTENTLISTDIALOG_H
            
// End of File
