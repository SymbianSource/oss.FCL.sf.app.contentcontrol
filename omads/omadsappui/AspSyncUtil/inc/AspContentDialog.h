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
* Description: Editing content settings 
*
*/


#ifndef ASPCONTENTDIALOG_H
#define ASPCONTENTDIALOG_H


//  INCLUDES
#include <e32base.h>
#include <eiklbo.h>
#include <aknlists.h>
#include <aknnavide.h>
#include <AknDialog.h>

#include <CAknCommonDialogsBase.h>  // TCommonDialogType

#include "AspDialogUtil.h"



// CLASS DECLARATION


/**
* CAspContentDialog
* 
* Content dialog is used to edit sync content (sync task) data.
*/
NONSHARABLE_CLASS (CAspContentDialog) : public CAknDialog, public MEikListBoxObserver
    {
	public:
	    /**
        * Launches content dialog.
        * @param aParam Class that contains dialog parameters.
        * @return Completion code.
        */
		static TBool ShowDialogL(TAspParam& aParam);

	private:

		/**
		* C++ default constructor.
		* @param aParam Class that contains dialog parameters.
		* @return None.
		*/
		CAspContentDialog(TAspParam& aParam);

						
        /**
        * Two-phased constructor.
		* @param aParam Class that contains dialog parameters.
        */
		static CAspContentDialog* NewL(TAspParam& aParam);

        /**
        * By default Symbian 2nd phase constructor is private.
        */
		void ConstructL();

	    /**
        * Destructor.
        */
		virtual ~CAspContentDialog();
		
		/**
		* Check if the MSK label is correct for the context
		* @param aKeyEvent Key code.
	    */	
		void CheckContentSettingL(const TKeyEvent& aKeyEvent);
		
		/**
		* Update the MSK Label
		* @param aCommandId command id.
		* @param aResourceId resource id .
	    */
		void UpdateMiddleSoftKeyLabelL(TInt aCommandId,TInt aResourceId);
		

	private:	

	   /**
		* From the base class.
        * Called when dialog is activated.
        * @return None.
        */
		void ActivateL();

		/**
		* From the base class.
        * Handles key presses.
        * @param aKeyEvent Key code.
		* @param aType Key type.
        * @return Result code.
        */
		TKeyResponse OfferKeyEventL(const TKeyEvent& aKeyEvent,TEventCode aType);
	
		
		/**
		* From the base class.
		* Handles menu events.
		* @param  aCommandId Command id.
		* @return None.
		*/
		virtual void ProcessCommandL(TInt aCommandId);

		/**
        * From the base class
		* Called by framework before menu is shown.
        * @param aResourceId Menu resource id.
		* @param aMenuPane Pointer to the menu.
        * @return None.
        */
		void DynInitMenuPaneL(TInt aResourceID, CEikMenuPane* aMenuPane);

		/**
		* Handle content parameter editing.
		* @return None.
		*/
		void HandleOKL();

		/**
		* Method to get context sensitive help topic.
        * @param aContext Help topic to open.
		* @return None.
        */
		void GetHelpContext(TCoeHelpContext& aContext) const;
	
		/**
        * From the base class.
		* Called by framework before exiting the dialog.
        * @param aButtonId Id of the pressed button
		* @return ETrue to exit\ EFalse to not to exit.
        */
		TBool OkToExitL(TInt aButtonId);
	
		/**
        * From the base class.
		* Called by framework before the dialog is shown.
		* @return None.
        */
		void PreLayoutDynInitL();

		/**
        * From MEikListBoxObserver.
		* Handle listbox events.
        * @param aListBox Pointer to used listbox.
		* @param aEventType Event type.
        * @return None.
        */
		void HandleListBoxEventL(CEikListBox* aListBox, TListBoxEvent aEventType);
		
		/**
		* From base class.
		* Called when UI layout changes. 
		* @param aType.
    	* @return None.
		*/
        void HandleResourceChange(TInt aType);
        
		/**
		* Set setting list icons.
		* @param None.
		* @return None.
		*/
		void SetIconsL();
        

	private:

		/**
		* Check that mandatory fields are set
		* @return Index of the invalid mandatory item.
		*/
		TInt CheckMandatoryFields();
	
		/**
		* Save content parameters into profile.
    	* @return None.
		*/
		void SaveSettingsL();

		/**
		* Set filter type setting visibility.
		* @param None.
		* @return None.
		*/
		void SetFilterVisibility(TInt aVisibility);
    
    private:
    		
		/**
        * Add settings titles into listbox.
        * @param aListBox.
        * @param aItemList List of setting items.
        * @return None.
        */
		void UpdateListBoxL(CEikTextListBox* aListBox, CAspSettingList* aItemList);
		
		/**
        * Create one CAspListItemData for each setting.
        * @param None.
        * @return None.
        */
		void CreateSettingsListL();
		
		/**
        * Adds one setting item into item list.
        * @param aItemId Item id.
        * @param aResourceId.
		* @return None.
        */
		void AddItemL(TInt aItemId, TInt aResourceId);
		
		/**
        * Adds one setting item into item list.
        * @param aDataProviderId.
		* @return None.
        */
		void AddRemoteDatabaseL(TInt aDataProviderId);
		
		/**
        * Initalizes setting item needed for listbox handling.
        * @param aItem.
        * @return None.
        */
		void InitSettingItemL(CAspListItemData* aItem);


    private:

		/**
		* Handles setting item editing.
		* @param aItem Setting item data.
		* @return Return code.
		*/
		TBool EditSettingItemL(CAspListItemData& aItem);

		/**
		* Change Yes/No value without showing edit dialog.
		* @param aItem Setting item data.
		* @return Return code.
		*/
		TBool EditSettingItemYesNoL(CAspListItemData& aItem);
		
		/**
		* Shows editor for text type setting.
		* @param aItem Setting item data.
		* @return Return code.
		*/
		TBool EditSettingItemTextL(CAspListItemData& aItem);

		/**
		* Shows editor for list type setting.
		* @param aItem Setting item data.
		* @return Return code.
		*/
        TBool EditSettingItemLocalDatabaseL(CAspListItemData& aItem);
    
		/**
		* Shows editor for list type setting.
		* @param aItem Setting item data.
		* @return Return code.
		*/
		static TBool EditSettingItemListL(CAspListItemData& aItem);
  
  
    private:
		/**
		* Utility function.
		* @param aIndex Listbox index.
		* @return setting item with listbox position aIndex.
        */
		CAspListItemData* GetItemForIndex(TInt aIndex);

		/**
        * Utility function.
        * @param aItemId Item id.
        * @return Pointer to setting item.
        */
		CAspListItemData* Item(TInt aItemId);
		
		/**
		* Finds out sync data provider type.
		* @param None.
		* @return Data provider type.
		*/
		TInt DataProviderType() const;
		
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
        * Read sync profile task data.
        * @param None.
		* @return None.
        */
		void InitTaskDataL();
		
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
		// id of the calling application
		TInt iApplicationId;

		// list box for setting items 
		CAknSettingStyleListBox* iListBox;
		
		// setting items
		CAspSettingList *iSettingList;
		
		// list that contains all contents (applications)
		CAspContentList* iContentList;

		// for title and icon handling
		CStatusPaneHandler* iStatusPaneHandler;

		// session with sync server
		RSyncMLSession* iSyncSession;
		
		// sync profile
		CAspProfile* iProfile;

		// long buffer for string handling
		TBuf<KBufSize255> iBuf;
		
		// is setting database in read only state
		TBool iSettingEnforcement;
		
		// has user changed sync content local or remote datastore
		TBool iDataStoreChanged;
		
		
    private:
		
		// dialog parameters
		TAspParam* iDialogParam;
	
		// sync data provider
		TInt iDataProviderId;

		// is content included in sync
		TBool iTaskEnabled;
		
		// sync type
		TInt iSyncDirection;

		// needed for finding out whether user changed db name
		TBuf<KBufSize> iLocalDatabase;
		
		// needed for finding out whether user changed db name
		TBuf<KBufSize> iRemoteDatabase;
		
		// names of all available local data stores
		CDesCArray* iLocalDataStores;
		//is MSK Empty	
		TBool iMskEmpty;
	};




#endif      // ASPCONTENTDIALOG_H
            
// End of File
