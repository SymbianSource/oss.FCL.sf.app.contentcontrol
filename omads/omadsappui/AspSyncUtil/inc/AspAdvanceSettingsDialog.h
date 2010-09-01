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



#ifndef ASPADVANCESETTINGSDIALOG_H
#define ASPADVANCESETTINGSDIALOG_H


//  INCLUDES

#include "AspDialogUtil.h"

#include <e32base.h>
#include <aknlists.h>
#include <AknDialog.h>





// CLASS DECLARATION


/**
*  CAspAdvanceSettingsDialog
*
* CAspAdvanceSettingsDialog is used for editing sync connection data.
*/
NONSHARABLE_CLASS (CAspAdvanceSettingsDialog) : public CAknDialog, public MEikListBoxObserver
    {
    public:
    
    
	public:
	    /**
        * Launches dialog.
        * @param aParam Class that contains dialog parameters.
        * @return Completion code.
        */
		static TBool ShowDialogL();

	public:// Constructors and destructor

        /**
        * Two-phased constructor.
        */
		static CAspAdvanceSettingsDialog* NewL();
        
        /**
        * Destructor.
        */
        virtual ~CAspAdvanceSettingsDialog();
    
    private:

        /**
        * C++ default constructor.
        */
        CAspAdvanceSettingsDialog();
	
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
		* Set the MSK empty.
		* @param None.
		* @return None.
		*/
		void SetEmptyMiddleSoftKeyLabelL();

    private:
		 		        
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
		
    private:

        void  WriteRepositoryL(TInt aKey, const TInt& aValue);
        
        void  ReadRepositoryL(TInt aKey, TInt& aValue);
        
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
		TBool iDefConnSupported;
		TBool iMSKEmpty;
    };


#endif      // ASPPROFILEDIALOG_H
            
// End of File
