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
* Description: Dialog to edit profile setting 
*
*/



#ifndef ASPPROFILEDIALOG_H
#define ASPPROFILEDIALOG_H


//  INCLUDES

#include "AspDialogUtil.h"

#include <e32base.h>
#include <aknlists.h>
#include <AknDialog.h>





// CLASS DECLARATION


/**
*  CAspProfileDialog
*
* CAspProfileDialog is used for editing sync connection data.
*/
NONSHARABLE_CLASS (CAspProfileDialog) : public CAknDialog, public MEikListBoxObserver
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
		static CAspProfileDialog* NewL(const TAspParam& aParam);
        
        /**
        * Destructor.
        */
        virtual ~CAspProfileDialog();
    
    private:

        /**
        * C++ default constructor.
        */
        CAspProfileDialog(const TAspParam& aParam);
	
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

	
	public:  // common editors
	
		/**
		* Shows editor for text type setting.
		* @param aItem Setting item data.
		* @return Return code.
		*/
		TBool EditSettingItemTextL(CAspListItemData& aItem);
		
		/**
		* Shows editor for number type setting.
		* @param aItem Setting item data.
		* @return Return code.
		*/
		static TBool EditSettingItemNumberL(CAspListItemData& aItem);
		
		/**
		* Shows editor for secret type setting.
		* @param aItem Setting item data.
		* @return Return code.
		*/
		static TBool EditSettingItemSecretL(CAspListItemData& aItem);
		
		/**
		* Shows editor for list type setting.
		* @param aItem Setting item data.
		* @return Return code.
		*/
		static TBool EditSettingItemListL(CAspListItemData& aItem);
 		/**
		* Checks whether MSK label and menu command are correct for the context
		* @param aKeyEvent
		* @return None.
		*/
 		void CheckContentSettingL(const TKeyEvent& aKeyEvent);
 		/**
		* Checks whether MSK label and menu command are correct for the context 
		* @param None
		* @return None.
		*/
 		void CheckContentSettingL();
        /**
		* Utility function, updates dialog cba.
		* @param aResourceId.
		* @return None.
		*/
		void UpdateCbaL(TInt aResourceId);
		
		/**
		* Utility function, updates menu bar.
		* @param aResource.
		* @return None.
		*/
		
		void UpdateMenuL(TInt aResource);
    
    private:
    
		/**
		* Handles setting item editing.
		* @param aItem Setting item data.
		* @return Return code.
		*/
		TBool EditSettingItemL(CAspListItemData& aItem);

		/**
		* Change Yes/No setting in place without showing editor.
		* @param aItem Setting item data.
		* @return Return code.
		*/
		TBool EditSettingItemYesNoL(CAspListItemData& aItem);
		
		/**
		* Shows editor for bearer type setting.
		* @param aItem Setting item data.
		* @return Return code.
		*/
		TBool EditSettingItemBearerListL(CAspListItemData& aItem);
		
		/**
		* Shows editor for protocol version setting.
		* @param aItem Setting item data.
		* @return Return code.
		*/
		TBool EditSettingItemProtocolListL(CAspListItemData& aItem);

		/**
		* Shows editor for text type setting.
		* @param aItem Setting item data.
		* @return Return code.
		*/
		TBool EditSettingItemHostAddressL(CAspListItemData& aItem);

		/**
		* Shows dialog for access point selection.
		* @param aItem Setting item data.
		* @return Return code.
		*/
		TBool EditSettingItemAccessPointL(CAspListItemData& aItem);
		
    	/**
		* Opens content dialog.
		* @param None.
		* @return Return code.
		*/
		TInt EditSettingItemSyncContentL(CAspListItemData& aItem);
		
		/**
		* Set the MSK empty.
		* @param None.
		* @return None.
		*/
		void SetEmptyMiddleSoftKeyLabelL();

    private:

		/**
		* Checks that all mandatory data is found.
		* @param None.
		* @return Return code.
		*/
		TInt CheckMandatoryFields();
		
		/**
		* Checks that all mandatory data is found.
		* @param aListItem.
		* @return Return code.
		*/
		TBool CheckMandatoryContentFields(CAspListItemData* aListItem);
		
		/**
		* Checks that server id is unique.
		* @param None.
		* @return Return code.
		*/
		TInt CheckUniqueServerId();

		/**
		* Saves profile.
		* @param None.
		* @return Return code.
		*/
		void SaveSettingsL();
        
        /**
		* Check setting values before dialog close.
        * @param aButtonId Button id.
		* @return ETrue to exit\ EFalse to not to exit.
        */
        TBool CheckSettingValuesL(TInt aButtonId);

        /**
		* Check setting values before dialog close.
        * @param aButtonId Button id.
		* @return ETrue to exit\ EFalse to not to exit.
        */
        TBool CheckSettingValues(TInt aButtonId);

		/**
		* Check if the profile is selected for auto sync ,and if selected check for mandatory settings
      		* @return None
        */
		void CheckAutoSyncSetttigsL();

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
        * Adds one setting item into item list.
        * @param aItemId Item id.
        * @param aHeaders Array of item headers.
		* @return None.
        */
		void AddItemL(TInt aItemId, CDesCArray* aHeaders);

		/**
        * Adds one setting item into item list.
        * @param aItemId Item id.
        * @param aResourceId.
		* @return None.
        */
		void AddItemL(TInt aItemId, TInt aResourceId);

		/**
        * Set setting item visibility (normal/readonly/hidden).
        * @param None.
		* @return None.
        */
		void SetVisibility();
		
        /**
        * Set all settings to read-only state.
        * @param None.
		* @return None.
        */
		void SetAllReadOnly();
		
		
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
		

	private:
		// setting list
		CAknSettingStyleListBox* iSettingListBox;

		// list of setting items
		CAspSettingList *iSettingList;
		
   		// list that contains all contents (applications)
		//CAspContentList* iContentList;

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

		// for bearer type list construction
		CAspBearerHandler* iBearerHandler;
		
		// for access point selection
		CAspAccessPointHandler* iApHandler;
		
		// long buffer for string handling
		TBuf<KBufSize255> iBuf;
		
		// list that contains all contents (applications)
		CAspContentList* iContentList;
		
		// list that contains all profiles
		CAspProfileList* iProfileList;

		// id of the calling application
		TInt iApplicationId;
		
		// is setting database in read only state
		TBool iSettingEnforcement;
		// profile's server id
		TBuf<KAspMaxServerIdLength> iOldServerId;
    	// id of current cba resource
		TInt iCommandSetId;
		// is  MSK empty
		TBool iMSKEmpty;
    };


#endif      // ASPPROFILEDIALOG_H
            
// End of File
