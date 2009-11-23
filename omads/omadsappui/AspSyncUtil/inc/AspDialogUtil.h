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


#ifndef ASPDIALOGUTIL_H
#define ASPDIALOGUTIL_H


//  INCLUDES
#include <e32base.h>
#include <aknlistquerycontrol.h>
#include <aknnavi.h>
#include <aknnavide.h>
#include <AknQueryDialog.h>

#include <SyncMLHistory.h>

#include <AknIndicatorContainer.h>      // CAknIndicatorContainer
#include <aknEditStateIndicator.h>      // MAknEditingStateIndicator
#include <akncheckboxsettingpage.h>     // CSelectionItemList
#include <aknradiobuttonsettingpage.h>  // CAknRadioButtonSettingPage
#include <akntextsettingpage.h>         // CaknTextSettingPage
#include <aknpasswordsettingpage.h>     // CAknAlphaPasswordSettingPage

#include "AspDefines.h"
#include "AspProfile.h"
#include "AspResHandler.h"



#ifdef RD_DSUI_TIMEDSYNC 

#include <akntabgrp.h>
#include <akntabobserver.h>

#endif

/**
* TDialogUtil
* 
* TDialogUtil contains general dialog utilities.
*/
NONSHARABLE_CLASS (TDialogUtil)
	{
    public:
		/**
		* Shows text query.
		* @param aTitle Title text.
		* @param aText Message text.
		* @param aMaxLength Max text length.
		* @return None.
		*/
        static void ShowTextQueryL(const TDesC& aTitle, TDes& aText, TInt aMaxLength);
		
		/**
		* Shows message query (title, text and Ok button).
		* @param aTitle Title text.
		* @param aText Message text.
		* @return None.
		*/
		static void ShowMessageQueryL(const TDesC& aTitle, const TDesC& aText);

		/**
		* Shows message query (title, text and Ok button).
		* @param aTitle Title text.
		* @param aResourcet Mssage text resource id.
		* @return None.
		*/
		static void ShowMessageQueryL(const TDesC& aTitle, TInt aResource);

		static void ShowAutoSyncMessageQueryL(const TDesC& aText);
		/**
		* Shows yes/no question.
		* @param aText Question text.
		* @return ETrue if user selected yes, EFalse otherwise..
		*/
		static TBool ShowConfirmationQueryL(const TDesC& aText);

		/**
		* Shows yes/no question.
		* @param aText Question text.
		* @return ETrue if user selected yes, EFalse otherwise..
		*/
		static TBool ShowRoamingConfirmationQueryL(const TDesC& aText);
		        
		/**
        * Shows ok/cancel question.
        * @param aResource Question text.
        * @return ETrue if user selected ok, EFalse otherwise..
        */
        static TBool ShowRoamingConfirmationQueryL(TInt aResource);

		/**
		* Shows yes/no question.
		* @param aResource Question text.
		* @return ETrue if user selected yes, EFalse otherwise..
		*/
		static TBool ShowConfirmationQueryL(TInt aResource);

		/**
		* Shows information text.
		* @param aText Information text.
		* @return None.
		*/
        static void ShowInformationQueryL(const TDesC& aText);
        
		/**
		* Shows information text.
		* @param aResource Information text.
		* @return None.
		*/
        static void ShowInformationQueryL(TInt aResource);
		
		/**
		* Shows information text.
		* @param aResource Information text.
		* @return None.
		*/
		static void ShowInformationNoteL(TInt aResource);

		/**
		* Shows information text.
		* @param aText Information text.
		* @return None.
		*/
		static void ShowInformationNoteL(const TDesC& aText);

		/**
		* Shows error text.
		* @param aResource Error text.
		* @return None.
		*/
		static void ShowErrorNoteL(TInt aResource);

		/**
		* Shows error text.
		* @param aText Error text.
		* @return None.
		*/
		static void ShowErrorNoteL(const TDesC& aText);
		
		/**
		* Shows error text.
		* @param aResource Error text.
		* @return None.
		*/
		static void ShowErrorQueryL(TInt aResource);

		/**
		* Shows error text.
		* @param aText Error text.
		* @return None.
		*/
		static void ShowErrorQueryL(const TDesC& aText);

		/**
		* Shows dialog with selection list.
		* @param aTitle Dialog title.
		* @param aList List of selection items.
		* @param aSelectedIndex Index of selected item.
		* @return EFalse if user canceled, ETrue otherwise.
		*/
		static TBool ShowListQueryL(const TDesC& aTitle, CDesCArray* aList, TInt* aSelectedIndex);
		/**
		* Shows dialog with selection list other than sync application
		* @param aTitle Dialog title.
		* @param aList List of selection items.
		* @param aSelectedIndex Index of selected item.
		* @return EFalse if user canceled, ETrue otherwise.
		*/
		static TBool ShowListQueryIconL(const TDesC& aTitle, CDesCArray* aList, TInt* aSelectedIndex);
		/**
		* Shows dialog with selection list.
		* @param aTitle Dialog title.
		* @param aList List of selection items.
		* @param aSelectedIndex Index of selected item.
		* @return EFalse if user canceled, ETrue otherwise.
		*/
		static TBool ShowListQueryL(TInt aTitle, CDesCArray* aList, TInt* aSelectedIndex);
		/**
		* Shows dialog with selection list opened from other than sync application
		* @param aTitle Dialog title.
		* @param aList List of selection items.
		* @param aSelectedIndex Index of selected item.
		* @return EFalse if user canceled, ETrue otherwise.
		*/
		static TBool ShowListQueryIconL(TInt aTitle, CDesCArray* aList, TInt* aSelectedIndex);

		/**
		* Shows dialog with profile list.
		* @param aList Profile list.
		* @param aId Id of selected profile.
		* @param aResource Dialog title resource id.
		* @return EFalse if user canceled, ETrue otherwise.
		*/
		static TBool ShowProfileQueryL(CAspProfileList* aList, TInt& aId, TInt aResource);
		/**
		* Shows dialog with profile list displayed other than sync application
		* @param aList Profile list.
		* @param aId Id of selected profile.
		* @param aResource Dialog title resource id.
		* @param aIndex to know the selected index
		* @return EFalse if user canceled, ETrue otherwise.
		*/
		static TBool ShowProfileQueryAppL(CAspProfileList* aList, TInt& aId, TInt aResource,TInt &aIndex);
		
		/**
		* Deletes menu item.
		* @param aMenuPane.
		* @param aCommandId Id of deleted menu.
		* @return None.
		*/
		static void DeleteMenuItem(CEikMenuPane* aMenuPane, TInt aCommandId);

		/**
		* Constructs an independent CGulIcon object.
		* @param aID Item ID of the masked bitmap to be created.
        * @param aFilename Filename to be used to construct the item, 
        * @param aFileBitmapId Id (for bitmap) in the file. 
        * @param aFileMaskId Id (for mask) in the file.
        * @return Pointer to the newly created CGulIcon object.
        *         Ownership of the object is transferred to the caller.
		*/
		static CGulIcon* CreateIconL(TAknsItemID aId, const TDesC& aFileName, TInt aFileBitmapId, TInt aFileMaskId);

		/**
		* Constructs an image.
		* @param aID Item ID of the masked bitmap to be created.
        * @param aFilename Filename to be used to construct the item, 
        * @param aFileBitmapId Id (for bitmap) in the file. 
        * @param aFileMaskId Id (for mask) in the file.
        * @return Pointer to the newly created 	CEikImage object.
		*/
		static CEikImage* CreateImageLC(TAknsItemID aId, const TDesC& aFileName, TInt aFileBitmapId, TInt aFileMaskId);
		
		/**
		* Constructs an list box item for CAknDoubleLargeStyleListBox.
		* @param aFirstLine.
        * @param aSecondLine 
        * @param aIndex1. 
        * @param aIndex2.
        * @return Pointerto the newly created list box item.
		*/
        static HBufC* ConstructListBoxItemLC(const TDesC& aFirstLine, const TDesC& aSecondLine, TInt aIndex);
        
		/**
		* Shows time editor page.
		* @param aTime.
        * @return ETrue if not canceled, EFalse otherwise.
		*/
		static TBool ShowTimeEditorL(TTime& aTime, const TDesC& aTitle);

		/**
		* Shows date editor page.
		* @param aTime.
        * @return ETrue if not canceled, EFalse otherwise.
		*/
		static TBool ShowDateEditorL(TTime& aTime, const TDesC& aTitle);
		
		/**
		* Shows list editor page.
		* @param aList.
		* @param aTitle.
		* @param aCurrent.
        * @return ETrue if not canceled, EFalse otherwise.
		*/
		static TBool ShowListEditorL(CDesCArray* aList, const TDesC& aTitle, TInt& aCurrent);
		
		/**
		* Shows list editor page.
		* @param aResourceId.
		* @param aList.
		* @param aTitle.
		* @param aCurrent.
        * @return ETrue if not canceled, EFalse otherwise.
		*/
		static TBool ShowListEditorL(TInt aResourceId, CDesCArray* aList, const TDesC& aTitle, TInt& aCurrent);
		
		
		/**
		* Shows text editor page.
		* @param aRessourceId.
		* @param aText.
		* @param aTitle.
    	* @param aMandatory.
		* @param aLatinInput.
		* @param aMaxLength.
        * @return ETrue if not canceled, EFalse otherwise.
		*/
		static TBool ShowTextEditorL(TInt aResourceId, TDes& aText, const TDesC& aTitle, TBool aMandatory, TBool aLatinInput, TInt aMaxLength);
		
		/**
		* Shows text editor page.
		* @param aText.
		* @param aTitle.
    	* @param aMandatory.
		* @param aLatinInput.
		* @param aMaxLength.
        * @return ETrue if not canceled, EFalse otherwise.
		*/
        static TBool ShowTextEditorL(TDes& aText, const TDesC& aTitle, TBool aMandatory, TBool aLatinInput, TInt aMaxLength);		

		/**
		* Shows integer editor page.
		* @param aNumber.
		* @param aTitle.
    	* @param aMin.
		* @param aMax.
		* @param aLatinInput.
        * @return ETrue if not canceled, EFalse otherwise.
		*/
        static TBool ShowIntegerEditorL(TInt& aNumber, const TDesC& aTitle, TInt aMin, TInt aMax, TBool aLatinInput);
        
		/**
		* Shows popup dialog.
		* @param aText Popup title.
		* @param aArray Popup lines.
        * @return None.
		*/
        static void ShowPopupDialogL(const TDesC& aText, CDesCArray* aArray);	

		/**
		* Shows popup dialog.
		* @param aText Popup title.
		* @param aArray Popup lines.
        * @return None.
		*/
        static void ShowPopupDialog2L(const TDesC& aText, CDesCArray* aArray);	
        
        /**
		*
		* Does Descriptor contain string "PC Suite"
		* Its NOT case sensitive
		*
		*/
		static TBool IsPCSuiteDesC(const TDesC& des);
		
		static TInt ShowAutoSyncLogL(TAny* /*aAny*/);

	};



/**
* CAspIndicatorHandler
* 
* This class is used for changing indicator state.
*/
NONSHARABLE_CLASS (CAspIndicatorHandler) : public CBase
	{
    public:
        /**
        * Destructor.
        */
		virtual ~CAspIndicatorHandler();

        /**
        * C++ default constructor.
        */
        CAspIndicatorHandler::CAspIndicatorHandler(MAknEditingStateIndicator* aIndicator);

	public:

		/**
		* Sets navi pane title.
		* @param aTitle.
		* @return None.
		*/
		void SetIndicatorStateL(TInt aIndicatorType, const TDesC& aTitle);

	private:
		// status pane
		//CEikStatusPane* iStatusPane;
	
	    // edit state indicator
	    MAknEditingStateIndicator* iIndicator;
        
        // indicator container
        CAknIndicatorContainer* iIndicatorContainer;
        
        // indicator type
        TInt iIndicatorType;
        
        // has indicator been modified
        TBool iIndicatorModified;
        
        // indicator type
        TInt iPreviousIndicatorType;
	};



/**
* CAspNaviPaneHandler
* 
* This class is used for changing navipane title.
*/
NONSHARABLE_CLASS (CAspNaviPaneHandler) : public CBase
	{
    public:
        /**
        * Destructor.
        */
		virtual ~CAspNaviPaneHandler();

        /**
        * C++ default constructor.
        */
		CAspNaviPaneHandler(CEikStatusPane* aStatusPane);

	public:

		/**
		* Sets navi pane title.
		* @param aTitle.
		* @return None.
		*/
		void SetNaviPaneTitleL(const TDesC& aTitle);
		
		/**
		* Store navi pane.
		* @param None.
		* @return None.
		*/
		void StoreNavipaneL();

	private:
		// status pane
		CEikStatusPane* iStatusPane;
	
	    // navi Pane
	    CAknNavigationControlContainer* iNaviPane;
        
        // navi decorator
        CAknNavigationDecorator* iNaviDecorator;
        
        // has Navidecorator been pushed into navi pane
        TBool iNavidecoratorPushed;
        
        // has navi pane been pushed into navigation pane's object stack
        TBool iNavipanePushed;
	};


#ifdef RD_DSUI_TIMEDSYNC 

class CAspSettingDialog;

/**
* CAspTabbedNaviPaneHandler
* 
* This class is used for changing tab group.
*/
NONSHARABLE_CLASS (CAspTabbedNaviPaneHandler) : public CBase, public MAknTabObserver
	{
    public:
        /**
        * Destructor.
        */
		virtual ~CAspTabbedNaviPaneHandler();

        /**
        * C++ default constructor.
        */
		CAspTabbedNaviPaneHandler(CEikStatusPane* aStatusPane ,CAspSettingDialog *aDialog);


		
	public:
   		void StoreNavipaneL();
	    void SetTabTextL(const TDesC& aText, TInt aTabId);
	    void AddTabL(const TDesC& aText, TInt aTabId);
	    void DeleteTabsL();
	    void SetActiveTabL(TInt aTabId);
		TInt TabIndex(TInt aTabId);
		void SetTabWidthL();
	    
	    
	private: // from  MAknTabObserver
	    void TabChangedL(TInt aIndex);

	private:
		// status pane
		CEikStatusPane* iStatusPane;
	
	    // navi Pane
	    CAknNavigationControlContainer* iNaviPane;
        
        // navi decorator
        CAknNavigationDecorator* iNaviDecorator;
        
       	// Tab group owned by the navigation decorator.
        CAknTabGroup* iTabGroup;

        
        // has Navidecorator been pushed into navi pane
        TBool iNavidecoratorPushed;
        
        // has navi pane been pushed into navigation pane's object stack
        TBool iNavipanePushed;
        
        CAspSettingDialog *iDialog;
	};

#endif 

/**
* CStatusPaneHandler
* 
* This class is used for changing dialog title and image.
*/
NONSHARABLE_CLASS (CStatusPaneHandler) : public CBase
	{
    public:
        /**
        * Two-phased constructor.
        */
		static CStatusPaneHandler* NewL(CAknAppUi* aAppUi);

        /**
        * Destructor.
        */
		virtual ~CStatusPaneHandler();

    private:
        /**
        * By default Symbian OS constructor is private.
        */
		void ConstructL();

        /**
        * C++ default constructor.
        */
		CStatusPaneHandler(CAknAppUi* aAppUi);

	public:
		/**
		* Stores original title so it can be restored when dialog closes.
		* @param  None.
		* @return None.
		*/
        void StoreOriginalTitleL();

		/**
		* Restores original title.
		* @param  None.
		* @return None.
		*/
		void RestoreOriginalTitleL();

		/**
		* Sets dialog title.
		* @param  aText.
		* @return None.
		*/
		void SetTitleL(const TDesC& aText);

		/**
		* Sets dialog title.
		* @param  aResourceId.
		* @return None.
		*/
		void SetTitleL(TInt aResourceId);

	public:
		/**
		* Sets dialog title.
		* @param aAppUi.
		* @param aText.
		* @return Return code.
		*/
		static TBool SetTitleL(CAknAppUi* aAppUi, const TDesC& aText);

		/**
		* Gets dialog title.
		* @param aAppUi.
		* @param aText.
		* @return Return code.
		*/
		static TBool GetTitleL(CAknAppUi* aAppUi, TDes& aText);
		
		/**
		* Sets dialog image.
		* @param aFilename.
		* @param aMainId.
		* @param aMaskId.
		* @return None.
		*/
		void SetNewImageL(const TDesC& aFilename,TInt aMainId,TInt aMaskId);

		/**
		* Restores original image.
		* @param  None.
		* @return None.
		*/
		void RestoreOriginalImageL();

		/**
		* Sets dialog image.
		* @param aAppUi.
		* @param aIcon.
		* @return Return code.
		*/
		static CEikImage* SetImageL(CAknAppUi* aAppUi, CEikImage* aIcon);

		/**
		* Sets navi pane title.
		* @param aTitle.
		* @return None.
		*/
		void SetNaviPaneTitleL(const TDesC& aTitle);
		
		/**
		* Sets navi pane title.
		* @param aTitle.
		* @return None.
		*/
		void SetNaviPaneTitle(const TDesC& aTitle);
		
		/**
		* Sets navi pane title.
		* @param aTitle.
		* @return None.
		*/
		void SetNaviPaneTitleL(TInt aResource);
		
		/**
		* Store navi pane.
		* @param None.
		* @return None.
		*/
		void StoreNavipaneL();

	private:
		// access to app ui
		CAknAppUi* iAppUi;

		// original status pane title
		TBuf<KBufSize> iOriginalTitle;
		
		// is original status pane title stored
		TBool iOriginalTitleStored;

		// original context pane image
		CEikImage* iOriginalImage;
		
   	    CAspNaviPaneHandler* iNaviPaneHandler;
	};




/**
* CAspSelectionItemList
*
* Needed for using cleanupstack with CSelectionItemList.
*/
NONSHARABLE_CLASS (CAspSelectionItemList) : public CSelectionItemList
	{
    public:
	    /**
        * C++ default constructor.
        */
	    CAspSelectionItemList(TInt aGranularity);

	    /**
         * Callback for operator TCleanupItem().
         */
	    static void Cleanup(TAny* aObj);
	};




/**
*  CAspListItemData
*  
*  CAspListItemData represents one setting item in UI setting list
*/
NONSHARABLE_CLASS (CAspListItemData) : public CBase
	{
	public:
		enum TAspListItemType
			{
			ETypeInternetAccessPoint,
			ETypeList,
			ETypeBearerList,
			ETypeProtocolList,
			ETypeLocalDatabase,
			ETypeListYesNo,
			ETypeSecret,
			ETypeHostAddress,
			ETypeFilter,
			ETypeUseFilters,
			ETypeSyncContent,
			ETypeAutoSyncProfile,
			ETypeAutoSyncContent,
			ETypeAutoSyncFrequency,
			ETypeAutoSyncDays,
			ETypeAutoSyncPeakSchedule,
			ETypeAutoSyncOffPeakSchedule,
			ETypeAutoSyncTime,
			ETypeNumber,
			ETypeBoolean,
			ETypeText,
			ETypeText8,
			ETypeDate,
			ETypeTime,
			ETypeSchedulingSetting,
			ETypeProfileSetting
			
			};
			
	public:
		static CAspListItemData* NewLC();
    	CAspListItemData();
		virtual ~CAspListItemData();
		void ConstructL();

	public:	
		void SetHeaderL(const TDesC& aText);
		void SetHeaderL(TInt aResourceId);
		void SetValueL(const TDesC& aText);
		void SetValueL(TInt aNumber);
		void SetDisplayValueL(const TDesC& aText);
		void SetDisplayValueL(TInt aResource);
		const TDesC& Header();
		const TDesC& Value();
		HBufC* ListItemTextL(TBool aConvert);
		void Init();
		TBool IsEmpty();

	public:
		// number type setting data
		TInt iNumberData;
		
		// date type setting
		TTime iDate;
		
		// time type setting
		TTime iTime;

		// setting item type
		TInt iItemType;
    	
    	// is setting shown in UI
		TInt iHidden;
		
		// max length for text type setting
		TInt iMaxLength;
		
		// setting editor resource id
		TInt iResource;

		// list index
		TInt iIndex;	

		// item id
		TInt iItemId;

        // is settimg mandatory
		TBool iMandatory;
		
		// is latin input needed ???
		TBool iLatinInput;
		
		// min number type setting value
		TInt iMinValue;

		// max number type setting value
		TInt iMaxValue;
		
		// filter item - used only in CAspFilterDialog
		//CAspFilter* iFilter;
		
		// did user press "Select" key or "Change" menu option
		TBool iSelectKeyPressed;

	private:
		// used in UI when value is empty
		HBufC*  iDisplayValue;
		
		// setting header (title)		
		HBufC* iHeader;

	    // text type setting value
	    HBufC* iValue;
	};




typedef CArrayPtr<CAspListItemData> CAspSettingList; //setting item list for profile page

/**
* CAspRadioButtonSettingPageEditor
*
* Setting page that returns command button id.
*/
NONSHARABLE_CLASS (CAspRadioButtonSettingPageEditor) : public CAknRadioButtonSettingPage
	{
    public:
        CAspRadioButtonSettingPageEditor(TInt aResourceID, TInt& aCurrentSelectionIndex, 
		                           const MDesCArray* aItemArray);
        ~CAspRadioButtonSettingPageEditor();            
        
	    TKeyResponse OfferKeyEventL(const TKeyEvent& aKeyEvent, TEventCode aType);
	};


/**
* CAspTextSettingPagePageEditor
*
* Setting page that returns command button id.
*/
NONSHARABLE_CLASS (CAspTextSettingPageEditor) : public CAknTextSettingPage
	{
    public:
        CAspTextSettingPageEditor(TInt aResourceID,TDes& aText,TInt aTextSettingPageFlags);                                                    
                                                     
        ~CAspTextSettingPageEditor();            
        
	    TKeyResponse OfferKeyEventL(const TKeyEvent& aKeyEvent, TEventCode aType);
	};


/**
* CAspAlphaPasswordSettingPageEditor
*
* Setting page that returns command button id.
*/
NONSHARABLE_CLASS (CAspAlphaPasswordSettingPageEditor) : public CAknAlphaPasswordSettingPage
	{
    public:
        CAspAlphaPasswordSettingPageEditor(TInt aResourceID, TDes& aNewPassword,const TDesC& aOldPassword);                                                    
                                                     
        ~CAspAlphaPasswordSettingPageEditor();            
        
	    TKeyResponse OfferKeyEventL(const TKeyEvent& aKeyEvent, TEventCode aType);
	};

/**
* CAspAlphaPasswordSettingPageEditor
*
* Setting page that returns command button id.
*/
NONSHARABLE_CLASS (CAspIntegerSettingPageEditor) : public CAknIntegerSettingPage
	{
    public:
        CAspIntegerSettingPageEditor(TInt aResourceID, TInt& aValue, TInt aTextSettingPageFlags);                                                    
                                                     
        ~CAspIntegerSettingPageEditor();            
        
	    TKeyResponse OfferKeyEventL(const TKeyEvent& aKeyEvent, TEventCode aType);
	};


#endif      // ASPDIALOGUTIL_H
            
// End of File
