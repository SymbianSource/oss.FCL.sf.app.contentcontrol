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
* Description:  Contains general data definitions for AspSyncUtil.
*
*/


#ifndef ASPFILTERDIALOG_H
#define ASPFILTERDIALOG_H


//  INCLUDES
#include <e32base.h>
#include <eiklbo.h>
#include <aknlists.h>
#include <aknnavide.h>
#include <AknDialog.h>

#include <CAknCommonDialogsBase.h>  // TCommonDialogType
#include <AknRadioButtonSettingPage.h>  // CAknRadioButtonSettingPage
#include <AknCheckBoxSettingPage.h>  // CAknRadioButtonSettingPage


#include "AspDialogUtil.h"




// CLASS DECLARATION


/**
* CAspFilterDialog
* 
* CAspFilterDialog shows list of sync filter items. 
*/
NONSHARABLE_CLASS (CAspFilterDialog) : public CAknDialog, public MEikListBoxObserver
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
		CAspFilterDialog(TAspParam& aParam);

						
        /**
        * Two-phased constructor.
		* @param aParam Class that contains dialog parameters.
        */
		static CAspFilterDialog* NewL(TAspParam& aParam);

        /**
        * By default Symbian 2nd phase constructor is private.
        */
		void ConstructL();

	    /**
        * Destructor.
        */
		~CAspFilterDialog();

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

	private:

		/**
		* Check that mandatory fields are set
		* @return Index of the invalid mandatory item.
		*/
		TInt CheckMandatoryFieldsL();
	
		/**
		* Save content parameters into profile.
    	* @return None.
		*/
		void SaveSettingsL();

		/**


		/**
		* Set filter type setting visibility.
		* @param aVisible Visibility.
		* @return None.
		*/
		void SetFilterSettingsVisibility(TBool aVisible);
    
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
        * @param aItem.
		* @return None.
        */
		void AddItemL(CAspFilterItem* aFilterItem);
		
		/**
        * Initalizes setting item needed for listbox handling.
        * @param aItem.
        * @return None.
        */
		static void InitSettingItemL(CAspListItemData* aItem);


    public:
        static TBool EditFilterL(CAspListItemData& aItem);
        
    private:
		static TBool EditSettingItemL(CAspListItemData& aItem);
        static TBool EditSettingItemQueryL(CAspListItemData& aItem);

        static TBool EditSettingItemTextL(CAspListItemData& aItem);
        static TBool EditSettingItemListL(CAspListItemData& aItem);
        static TBool EditSettingItemBooleanL(CAspListItemData& aItem);
        static TBool EditSettingItemNumberL(CAspListItemData& aItem);
        static TBool EditSettingItemDateL(CAspListItemData& aItem);
        static TBool EditSettingItemTimeL(CAspListItemData& aItem);

        
        static TBool EditMultiChoiceListL(CAspListItemData& aItem);
        static TBool EditSingleChoiceListL(CAspListItemData& aItem);

	 
    private:
		/**
		* Utility function.
		* @param aIndex Listbox index.
		* @return setting item with listbox position aIndex.
        */
		CAspListItemData* ItemForIndex(TInt aIndex);

		/**
        * Utility function.
        * @param aItemId Item id.
        * @return Pointer to setting item.
        */
		CAspListItemData* Item(TInt aItemId);

  
		/**
		* Utility function.
		* @param None.
		* @return Sync profile.
        */
		RSyncMLDataSyncProfile& Profile();

		/**
		* Utility function.
		* @param None.
		* @return Sync task.
        */
		//RSyncMLTask& Task();

		/**
		* Utility function.
		* @param None.
		* @return Listbox.
        */
		CAknSettingStyleListBox* ListBox();
		
		/**
		* Utility function.
		* @param None.
		* @return Pointer to resource handler.
		*/
		CAspResHandler* ResHandler();
		
		
		/**
        * Set setting item visibility (normal/readonly/hidden).
        * @param None.
		* @return None.
        */
		void SetVisibility();

		
		

	private:
		// id of the calling application
		TInt iApplicationId;

		// list box for setting items 
		CAknSettingStyleListBox* iListBox;
		
		// setting items
		CAspSettingList *iSettingList;
		
		// CEikStatusPane contains dialog title and image
		CEikStatusPane* iStatusPane;

		// for title and icon handling
		CStatusPaneHandler* iStatusPaneHandler;

		// session with sync server
		RSyncMLSession* iSyncSession;
		
		// long buffer for string handling
		TBuf<KBufSize255> iBuf;
		
		// sync task
		CAspFilter* iFilter;
		
		// dialog parameters
		TAspParam* iDialogParam;
		
		// resource handler
		CAspResHandler* iResHandler;
	};


/**
* MAspChoiceListObserver
*
* MAspChoiceListObserver is for handling choice list events.
*/
class MAspChoiceListObserver
    {
    public:
	    virtual TBool HandleChoiceListEventL(TInt aEvent, TInt aListType) = 0;
    };


/**
* CAspChoiceListSettingPage
*
* Radio button setting page that contains "User defined" option.
*/
NONSHARABLE_CLASS (CAspChoiceListSettingPage) : public CAknRadioButtonSettingPage
	{
    public:
	enum TEditorType
		{
		ETypeNormal,
	    ETypeUserDefined
		};
	
    public: 
	    CAspChoiceListSettingPage(TInt aResourceID, TInt& aCurrentSelectionIndex,
                                  const MDesCArray* aItemArray, 
                                  MAspChoiceListObserver* aObserver, TInt aType);
        ~CAspChoiceListSettingPage();            
	
    protected: // From CCoeControl
	    void ProcessCommandL(TInt aCommandId);
	    
	private:
	    MAspChoiceListObserver* iObserver;
	    TInt iType;
	};


/**
* CAspMultiChoiceListSettingPage
*
* Check box button setting page that contains "User defined" option.
*/
NONSHARABLE_CLASS (CAspMultiChoiceListSettingPage) : public CAknCheckBoxSettingPage
	{
    public:
	enum TEditorType
		{
		ETypeNormal,
	    ETypeUserDefined
		};
	
    public: 
	    CAspMultiChoiceListSettingPage(TInt aResourceID, CSelectionItemList* aItemArray, 
                                  MAspChoiceListObserver* aObserver, TInt aType);
        ~CAspMultiChoiceListSettingPage();            
	
    protected: // From CCoeControl
	    void ProcessCommandL(TInt aCommandId);
	    
	private:
	    MAspChoiceListObserver* iObserver;
	    TInt iType;
	    CSelectionItemList* iSelectionList;
	};



/**
* CAspChoiceList
*
* CAspChoiceList implemets choicelist with "User defined" item.
*/
NONSHARABLE_CLASS (CAspChoiceList) : public CBase, public MAspChoiceListObserver
    {
    public:
	enum TListType
		{
		ETypeNormal,
		ETypeTextUserDefined,
	    ETypeIntegerUserDefined,
	    ETypeDateUserDefined,
	    ETypeTimeUserDefined
		};

    public:
		static CAspChoiceList* NewLC(const TDesC& aTitle, CSelectionItemList* aList, TInt aType, TInt aMaxLength);
		~CAspChoiceList();
	private:
		CAspChoiceList(const TDesC& aTitle, CSelectionItemList* iList, TInt aType, TInt aMaxLength);
		void ConstructL();
    
    public:
        TBool ShowListL();
      
    public:
        void AddItemL(CSelectionItemList* aList, TDes& aText, TBool aEnable);
        void AddItemL(TDes& aText, TBool aEnable);
        void AddItemL(TInt aNumber, TBool aEnable);
        void AddUserDefinedL();
        void RemoveUserDefinedL();
        CDesCArray* DesArrayLC(CSelectionItemList* aList);
        void SetSelectedIndex(CSelectionItemList* aList, TInt aIndex);
        TInt SelectedIndex(CSelectionItemList* aList);
        TBool EditTextL(TDes& aText, const TDesC& aTitle);


    private: // from MAspChoiceListObserver
	    TBool HandleChoiceListEventL(TInt aEvent, TInt aListType);


	private:
	    TInt  iUserDefinedInt;
	    TBuf<KBufSize>  iUserDefinedText;
	    TInt iMaxTextLength;
	    
	    TInt iListType;
        TBuf<KBufSize> iTitle;
        CSelectionItemList* iSelectionList;
    };



/**
* CAspMultiChoiceList
*
* CAspChoiceList implemets multiple selection choicelist with "User defined" item.
*/
NONSHARABLE_CLASS (CAspMultiChoiceList) : public CBase, public MAspChoiceListObserver
    {
    public:
		static CAspMultiChoiceList* NewLC(const TDesC& aTitle, CSelectionItemList* aList, TInt aType, TInt aMaxLength);
		~CAspMultiChoiceList();
	private:
		CAspMultiChoiceList(const TDesC& aTitle, CSelectionItemList* iList, TInt aType, TInt aMaxLength);
		void ConstructL();
    
    public:
        TBool ShowListL();

    
    public:
        void AddItemL(CSelectionItemList* aList, TDes& aText, TBool aEnable);
        void AddItemL(TDes& aText, TBool aEnable);
        void AddItemL(TInt aNumber, TBool aEnable);
        void AddUserDefinedL();
        void RemoveUserDefinedL();
        TBool EditTextL(TDes& aText, const TDesC& aTitle);
    
    private: // from MAspChoiceListObserver
	    TBool HandleChoiceListEventL(TInt aEvent, TInt aListType);

	private:
	    TInt  iUserDefinedInt;
	    TBuf<KBufSize>  iUserDefinedText;
	    TInt iMaxTextLength;
	    
	    TInt iListType;
        TBuf<KBufSize> iTitle;
        CSelectionItemList* iSelectionList;
    };



#endif      // ASPFILTERDIALOG_H
            
// End of File
