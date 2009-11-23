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
* Description: Edit view setting dialog 
*
*/



#ifndef ASPSETTINGVIEWDIALOG_H
#define ASPSETTINGVIEWDIALOG_H


//  INCLUDES

#include "AspDialogUtil.h"

#include <e32base.h>
#include <aknlists.h>
#include <AknDialog.h>

#include <AknInfoPopupNoteController.h> // CAknInfoPopupNoteController



// CLASS DECLARATION


/**
*  CAspSettingViewDialog
*
* CAspSettingViewDialog is used for editing various settings.
*/
NONSHARABLE_CLASS (CAspSettingViewDialog) : public CAknDialog, public MEikListBoxObserver
    {
    public:
    
    
	public:
	    /**
        * Launches dialog.
        * @param aParam Class that contains dialog parameters.
        * @return Completion code.
        */
		static TBool ShowDialogL(const TAspParam& aParam);

	public:// Constructors and destructor

        /**
        * Two-phased constructor.
        */
		static CAspSettingViewDialog* NewL(const TAspParam& aParam);
        
        /**
        * Destructor.
        */
        virtual ~CAspSettingViewDialog();
    
    private:

        /**
        * C++ default constructor.
        */
        CAspSettingViewDialog(const TAspParam& aParam);
	
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

	
	public:  // common editors
	
    
    private:
    
		/**
		* Handles setting item editing.
		* @param aItem Setting item data.
		* @return Return code.
		*/
	   void EditSettingItemL(CAspListItemData& aItem);
		
		/**
		* Opens profile setting dialog.
		* @param None.
		* @return Return code.
		*/
	  TInt EditSettingItemProfileSettingL();
	  
	  	/**
		* Opens schedule dialog.
		* @param None.
		* @return Return code.
		*/
	  TInt EditSettingItemSchedulingSettingL();
    	/**
		* Opens content dialog.
		* @param None.
		* @return Return code.
		*/
	  TInt EditSettingItemSyncContentL(CAspListItemData& aItem);


    private:

		

	private:
		/**
        * Create one CNSmlDSProfileData for each setting.
        * @param None.
        * @return None.
        */
		void CreateSettingsListL();
		
		/**
        * Initalizes setting item needed for listbox handling.
        * @param aItem.
        * @return None.
        */
		void InitSettingItemL(CAspListItemData* aItem);
		
		/**
        * Add settings titles into listbox.
        * @param aListBox.
        * @param aItemList List of setting items.
        * @return None.
        */
		void UpdateListBoxL(CEikTextListBox* aListBox, CAspSettingList* aItemList);

		/**
        * Adds profile setting into item list.
        * @param None.
		* @return None.
        */
		void AddProfileSettingItemL();
		/**
        * Adds scheduling setting into item list.
        * @param None.
		* @return None.
        */
		void AddSchedulingSettingItemL();
		/**
        * Adds sync contents into item list.
        * @param None.
		* @return None.
        */
		void AddContentItemsL();
		
		

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
		* Returns setting item with listbox position aIndex.
		* @param aIndex Listbox index.
		* @return Pointer to setting item.
        */
		CAspListItemData* GetItemForIndex(TInt aIndex);

		/**
        * Returns setting item with id aItemId.
        * @param aItemId Item id.
        * @return Pointer to setting item.
        */
		CAspListItemData* Item(TInt aItemId);

		/**
		* Returns listbox.
		* @param None.
		* @return Pointer to listbox.
		*/
		CAknSettingStyleListBox* ListBox();

		/**
		* Returns profile.
		* @param None.
		* @return Pointer to sync profile.
		*/
		//CAspProfile* Profile();

		TInt GetIndexFromProvider(TInt aDataProvider);

		void SetVisibility();

	private:
		// setting list
		CAknSettingStyleListBox* iSettingListBox;

		// list of setting items
		CAspSettingList *iSettingList;
		
     	// profile data
		CAspProfile* iProfile;

   		// sync session
		RSyncMLSession* iSyncSession;

		// for title and icon handling
		CStatusPaneHandler* iStatusPaneHandler;

		// resource handler
		CAspResHandler* iResHandler;

		// dialog edit mode
		TInt iEditMode;

		// long buffer for string handling
		TBuf<KBufSize255> iBuf;

		TInt iDataProviderId;
		
		// list that contains all contents (applications)
		CAspContentList* iContentList;
		
		// list that contains all profiles
		CAspProfileList* iProfileList;

		// id of the calling application
		TInt iApplicationId;

		CAspSchedule* iSchedule;

				
		};


#endif      // ASPSETTINGVIEWIALOG_H
            
// End of File
