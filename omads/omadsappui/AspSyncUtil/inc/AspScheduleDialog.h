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
* Description: Scheduled sync UI 
*
*/



#ifndef ASPSCHEDULEDIALOG_H
#define ASPSCHEDULEDIALOG_H


//  INCLUDES

#include "AspDialogUtil.h"

#include <e32base.h>
#include <aknlists.h>
#include <AknDialog.h>


// FORWARD DECLARATIONS

class CAspSchedule;
class CAspProfileWizard;


// CLASS DECLARATION
/**
* MAuotSyncEditorPageObserver
*
* Observer interface function.
*/

NONSHARABLE_CLASS (MAutoSyncEditorPageObserver)
	{
  	
	public:
		/**
        * Editor page call this when editor is about to close.
        * @param aText Editor text.
        * @return ETrue if input is ok, EFalse otherwise. 
        */
		virtual TBool CheckValidityL() = 0;

		/**
	*Return the index of email item in provider list
	*/
		virtual TInt EmailIndex() = 0;

	};


/**
* TAutoSyncSettingPageParam
*
* This class is used as general parameter type.
*/
NONSHARABLE_CLASS (TAutoSyncSettingPageParam)
	{
	public:
	    TInt iCommandId;
	    TBuf<KBufSize> iPageText;
	    MAutoSyncEditorPageObserver* iObserver;
	    TInt iSettingType;
	};


/**
* CAspAutoCheckBoxSettingPage
*
* Setting page that returns command button id.
*/
NONSHARABLE_CLASS (CAutoSyncCheckBoxSettingPage) : public CAknCheckBoxSettingPage,public MEikListBoxObserver
	{
    public:
        CAutoSyncCheckBoxSettingPage(TInt aResourceID, CSelectionItemList* aItemArray, TAutoSyncSettingPageParam& aParam);

        ~CAutoSyncCheckBoxSettingPage();            
        
	public:

		void UpdateCbaL(TInt aResourceId);
		void DynamicInitL();

	protected: // From CCoeControl
	    void ProcessCommandL(TInt aCommandId);
	    
	    TKeyResponse OfferKeyEventL(const TKeyEvent& aKeyEvent, TEventCode aType);
	    //from MEikListBoxObserver for touch event handling
        void HandleListBoxEventL(CEikListBox* aListBox, TListBoxEvent aEventType);
		
	private:
	    TAutoSyncSettingPageParam& iParam;
   	    TInt iSelectionCount;
   	    CSelectionItemList* iSelectionItemList;
		
	};



/**
*  CAspScheduleDialog
*
* CAspScheduleDialog is used for editing sync connection data.
*/
NONSHARABLE_CLASS (CAspScheduleDialog) : public CAknDialog, public MEikListBoxObserver,
																	  public MAutoSyncEditorPageObserver
    {
    private:
		
	enum TRepositoryKey
		{
	    EKeySyncFrequency = 2,
	    EKeyPeakSyncInterval,
	    EKeyOffPeakSyncInterval
		};
    
    
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
		static CAspScheduleDialog* NewL(const TAspParam& aParam);
        
        /**
        * Destructor.
        */
        virtual ~CAspScheduleDialog();
    
    private:

        /**
        * C++ default constructor.
        */
        CAspScheduleDialog(const TAspParam& aParam);
	
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

		/**
		* From TAutoSyncSettingPageParam
		* Checks validity content selected
		* @param  None
		* @return ETrue/EFalse.
		*/

		TBool CheckValidityL();

		TInt EmailIndex();
		
		/**
		* Checks remote databse settings for the profile
		* @param  None
		* @return None.
		*/

		void CheckMandatoryDataL();

		/**
		* Checks remote databse settings for the profile
		* @param  None
		* @return ETrue/EFalse.
		*/
		TBool CheckBearerType();

		

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
		TBool EditSettingItemNumberL(CAspListItemData& aItem);
		
		/**
		* Shows editor for list type setting.
		* @param aItem Setting item data.
		* @return Return code.
		*/
		//TBool EditSettingItemListL(CAspListItemData& aItem);
 
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
		* Shows editor for profile setting.
		* @param aItem Setting item data.
		* @return Return code.
		*/
		TBool EditSettingItemProfileL(CAspListItemData& aItem);

		/**
		* Shows editor for content setting.
		* @param aItem Setting item data.
		* @return Return code.
		*/
		TBool EditSettingItemContentL(CAspListItemData& aItem);

		/**
		* Shows setting editor.
		* @param aItem Setting item data.
		* @return Return code.
		*/
		TBool EditSettingItemWeekdaysL(CAspListItemData& aItem);
		
		/**
		* Shows setting editor.
		* @param aItem Setting item data.
		* @return Return code.
		*/
		TBool EditSettingItemTimeL(CAspListItemData& aItem);
		
		/**
		* Shows editor for peak schedule selection
		* @param aItem Setting item data.
		* @return Return code.
		*/
		TBool EditSettingItemPeakScheduleL(CAspListItemData& aItem);

		/**
		* Shows editor for off-peak schedule selection
		* @param aItem Setting item data.
		* @return Return code.
		*/
		TBool EditSettingItemOffPeakScheduleL(CAspListItemData& aItem);

		/**
		* Shows editor for sync frequency selection
		* @param aItem Setting item data.
		* @return Return code.
		*/

		TBool EditSettingItemSyncFrequencyL(CAspListItemData& aItem);
	

    private:

		/**
		* Checks that all mandatory data is found.
		* @param None.
		* @return Return code.
		*/
		TInt CheckMandatoryFields();
		
		/**
		* Saves profile.
		* @param None.
		* @return None.
		*/
		void SaveSettingsL();

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
		* Get profile name.
		* @param aProfileId.
		* @param aText.
		* @return None.
		*/
		void GetProfileName(TInt& aProfileId, TDes& aText);

		/**
		* Get content name.
		* @param aProfileId.
		* @param aText.
		* @return None.
		*/
		//void GetContentName(TDes& aText);

		/**
		* Get day name.
		* @param aProfileId.
		* @param aText.
		* @return None.
		*/
		void GetWeekdayNameL(TDes& aText);

		/**
		* Update content list from schedule settings.
		* @param None.
		* @return None.
		*/
		//void GetContentSelectionL();
		
		/**
		* Get content list from sync profile.
		* @param aList.
		* @return None.
		*/
		//void GetContentSelection2L(CSelectionItemList* aList);
		
		/**
		* Write selected contents into schedule settings.
		* @param None.
		* @return None.
		*/
		//void SetContentSelectionL();

		/**
		* Construct CSelectionItemList.
		* @param aList.
		* @return None.
		*/
		void GetWeekdaySelectionL(CSelectionItemList* aList);

		/**
		* Construct CSelectionItemList.
		* @param aList.
		* @return None.
		*/
		void SetWeekdaySelectionL(CSelectionItemList* aList);
		
			/**
		* Set default day selection
		* @param None
		* @return None.
		*/
		void SetDefaultdaySelectionL();
		/**
		* Construct time text.
		* @param aText.
		* @param aTime.
		* @return None.
		*/
		void GetTimeTextL(TDes& aText, TTime aTime);
		
   
       /**
		*Get content name 
		* @param aText.
		* @return None.
		*/
        void GetContentsNameL(TDes& aText);

	   /**
		*Get content selection
		* @param aList
		* @return None.
		*/	   
        void GetContentSelectionL(CSelectionItemList* aList);

	   /**
		*Set content selection
		* @param aList.
		* @return None.
		*/
        void SetContentSelectionL(CSelectionItemList* aList);

		CDesCArray* GetSyncFrequencyListLC();
		void GetSyncFrequencyValueL(TInt schedule, TDes& aBuf);
		TInt SyncFrequencyL(const TDesC& aBuf);
		TInt GetSyncFrequencyPositionL(CDesCArray* arr, TInt aInterval);

		/**
		*Check if the profile has access point selected
		* @param aProfileId
		* @return None.
		*/
 		void CheckAccessPointSelectionL(TInt aProfileId);

		/**
		*Get list of peak schedule intervals
		* @return CDesCArray*
		*/
		CDesCArray* GetPeakScheduleListLC();

		/**
		*Get list of peak schedule intervals
		* @return CDesCArray*
		*/
		CDesCArray* GetOffPeakScheduleListLC();

			/**
		*Get time interval string corresponding to a schedule no
		* @return CDesCArray*
		*/
		void GetPeakScheduleValueL(TInt schedule, TDes& aBuf);
		void GetOffPeakScheduleValueL(TInt schedule, TDes& aBuf);

			/**
		*Get time interval value for the string
		* @return CDesCArray*
		*/
		TInt PeakScheduleL(const TDesC& aBuf);
		TInt OffPeakScheduleL(const TDesC& aBuf);

			/**
		*Position of selected interval in the list
		* @return CDesCArray*
		*/
		TInt GetPeakPositionL(CDesCArray* arr, TInt aInterval);
		TInt GetOffPeakPositionL(CDesCArray* arr, TInt aInterval);

			/**
		*Show auto sync info
		* @return CDesCArray*
		*/
		void ShowAutoSyncInfoL();

			/**
		*Set default content selection
		* 
		*/
		void SetDefaultContentL();
			
			/**
		*Read cenrep for operator configured sync interval values
		* 
		*/
		void ReadRepositoryL(TInt aKey, TInt& aValue);
			
			/**
		*Create content list
		* 
		*/
		void CreateContentListL();

			/**
		*Check if time definition is valid.
		* 
		*/
		TBool CheckPeakTime();

	private:
		// setting list
		CAknSettingStyleListBox* iSettingListBox;

		// list of setting items
		CAspSettingList *iSettingList;

		// list that contains all contents (applications)
		CAspContentList* iContentList;

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
		
		// list that contains all profiles
		CAspProfileList* iProfileList;
		
		// for storing auto sync settings
		CAspSchedule* iSchedule;

		// profile data
		CAspProfile* iProfile;

		// id of the calling application
		TInt iApplicationId;
		
		// weekday names
		CDesCArray* iWeekdayList;
		
		// have settings changed
		TBool iSettingChanged;
		
		// Content names
		CDesCArray* iContentsList;

		TBool iDoCleanUp;

		// is setting database in read only state
		TBool iSettingEnforcement;
		
		
    };


#endif      // ASPSCHEDULEDIALOG_H
            
// End of File
