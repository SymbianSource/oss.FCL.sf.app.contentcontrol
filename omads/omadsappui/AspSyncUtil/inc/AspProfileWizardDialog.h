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
* Description: Dialog to create to profile using wizard 
*
*/


#ifndef ASPPROFILEWIZARDDIALOG_H
#define ASPPROFILEWIZARDDIALOG_H

// BackGround Context
#include <AknsBasicBackgroundControlContext.h>

//  INCLUDES
#include <e32base.h>
#include <akntextsettingpage.h>         // CAknIntegerSettingPage
#include <aknpasswordsettingpage.h>     // CAknAlphaPasswordSettingPage
#include <aknradiobuttonsettingpage.h>  // CAknRadioButtonSettingPage
#include <AknInfoPopupNoteController.h> // CAknInfoPopupNoteController

#include "AspProfileWizard.h"


// CONSTANTS

//const TInt KMsToWaitBeforePopup = 1000;
const TInt KMsToWaitBeforePopup = 3000;
const TInt KMsToWaitBeforeRePopup = 6000;
const TInt KMsTimePopupInView = 60*1000;

/**
* MAspEditorPageObserver
*
* Observer interface function.
*/
NONSHARABLE_CLASS (MAspEditorPageObserver)
	{
    public:
	enum TEditorEventType
		{
	    EKeyEvent,
	    EContentCountEvent
		};
	
	public:
		/**
        * Editor page call this when editor is about to close.
        * @param aText Editor text.
        * @return ETrue if input is ok, EFalse otherwise. 
        */
		virtual TBool CheckValidity(const TDesC& aText, TInt aSettingType) = 0;

		/**
        * Editor page call this to inform observer about editor event.
        * @param aEvent.
        * @param aValue.
        * @return None. 
        */
		virtual void HandleEditorEvent(TInt aEvent, TInt aValue) = 0;

		/**
        * Editor page call this to inform observer about editor event.
        * @param aEvent.
        * @param aValue.
        * @return None. 
        */
		virtual void GetNavipaneText(TDes& aText, TInt aContentCount) = 0;
	};


/**
* TAspSettingPageParam
*
* This class is used as general parameter type.
*/
NONSHARABLE_CLASS (TAspSettingPageParam)
	{
	public:
	    TInt iCommandId;
	    TBuf<KBufSize> iPageText;
	    MAspEditorPageObserver* iObserver;
	    TInt iSettingType;
	};


/**
* CAspTextSettingPage
*
* Setting page that returns command button id.
*/
NONSHARABLE_CLASS (CAspTextSettingPage) : public CAknTextSettingPage                                          
	{
    public:
        CAspTextSettingPage(TInt aResourceID, TDes& aText, TInt aTextSettingPageFlags,
                            TAspSettingPageParam& aParam);
        ~CAspTextSettingPage();            
        
    protected: // From CCoeControl
	    void ProcessCommandL(TInt aCommandId);
	    TBool PostDisplayCheckL();
	    TBool OkToExitL(TBool aAccept);
	    TKeyResponse OfferKeyEventL(const TKeyEvent& aKeyEvent, TEventCode aType);

	private:
	    void UpdateNaviPaneL();
	    
	private:
	    TAspSettingPageParam& iParam;
	    CAspIndicatorHandler* iIndicatorHandler;
	};


/**
* CAspRadioButtonSettingPage
*
* Setting page that returns command button id.
*/
NONSHARABLE_CLASS (CAspRadioButtonSettingPage) : public CAknRadioButtonSettingPage ,public MEikListBoxObserver
	{
    public:
        CAspRadioButtonSettingPage(TInt aResourceID, TInt& aCurrentSelectionIndex, 
		                           const MDesCArray* aItemArray, TAspSettingPageParam& aParam);
        ~CAspRadioButtonSettingPage();            
        
    protected: // From CCoeControl
	    void ProcessCommandL(TInt aCommandId);
	    TBool PostDisplayCheckL();
	    TKeyResponse OfferKeyEventL(const TKeyEvent& aKeyEvent, TEventCode aType);
	    //from MEikListBoxObserver for Touch event Handling
	    void HandleListBoxEventL(CEikListBox* aListBox, TListBoxEvent aEventType);
	    
	private:
	    void UpdateNaviPaneL();
	    
	private:
	    TAspSettingPageParam& iParam;
	    CAspNaviPaneHandler* iNaviPaneHandler;
	    const MDesCArray* iItemArray;
	};
	

/**
* CAspCheckBoxSettingPage
*
* Setting page that returns command button id.
*/
NONSHARABLE_CLASS (CAspCheckBoxSettingPage) : public CAknCheckBoxSettingPage,public MEikListBoxObserver
	{
    public:
        CAspCheckBoxSettingPage(TInt aResourceID, CSelectionItemList* aItemArray, TAspSettingPageParam& aParam);

        ~CAspCheckBoxSettingPage();            
        
    protected: // From CCoeControl
	    void ProcessCommandL(TInt aCommandId);
   	    TBool PostDisplayCheckL();
   	    TKeyResponse OfferKeyEventL(const TKeyEvent& aKeyEvent, TEventCode aType);
   	    //from MEikListBoxObserver for touch event handling
        void HandleListBoxEventL(CEikListBox* aListBox, TListBoxEvent aEventType);
    
	private:
	    void UpdateNaviPaneL();

	private:
	    TAspSettingPageParam& iParam;
   	    CAspNaviPaneHandler* iNaviPaneHandler;
   	    TInt iSelectionCount;
   	    CSelectionItemList* iSelectionItemList;
	};


/**
* CAspAlphaPasswordSettingPage
*
* Setting page that returns command button id.
*/
NONSHARABLE_CLASS (CAspAlphaPasswordSettingPage) : public CAknAlphaPasswordSettingPage
	{
    public:
        CAspAlphaPasswordSettingPage(TInt aResourceID, TDes& aNewPassword, const TDesC& aOldPassword, TAspSettingPageParam& aParam);

        ~CAspAlphaPasswordSettingPage();            
        
    protected: // From CCoeControl
	    void ProcessCommandL(TInt aCommandId);
   	    TBool PostDisplayCheckL();
   	    TKeyResponse OfferKeyEventL(const TKeyEvent& aKeyEvent, TEventCode aType);
	    
	private:
	    void UpdateNaviPaneL();

	private:
	    TAspSettingPageParam& iParam;
	    //CAspNaviPaneHandler* iNaviPaneHandler;
	    CAspIndicatorHandler* iIndicatorHandler;
	};


// Forward Declaration 
class CAknsBasicBackgroundControlContext;

/**
*  CAspProfileWizardDialog
*
* Dialog for editing profile name, data and connection.
*/
NONSHARABLE_CLASS (CAspProfileWizardDialog) : public CAknDialog,
                                              public MAspActiveCallerObserver,
                                              public MAspEditorPageObserver
    {
	enum TAspProfileDialogListIndex
		{
	    EIndexProfileName = 0,
        EIndexProfileContent = 1,
	    EIndexProfileConnection = 2
		};

	public:
	    /**
        * Launches profile dialog for editing profile name, data and connection.
		* @param aParam Class that contains dialog parameters.
        * @return Completion code.
        */
		static TBool ShowDialogL(TAspParam& aParam);

    
	public:// Constructors and destructor

        /**
        * Two-phased constructor.
        */
        static CAspProfileWizardDialog* NewL(TAspParam& aParam);
        
        /**
        * Destructor.
        */
        virtual ~CAspProfileWizardDialog();
    
    private:

        /**
        * C++ default constructor.
        */
        CAspProfileWizardDialog(TAspParam& aParam);
	
        /**
        * By default Symbian OS constructor is private.
        */
        void ConstructL();


	private:
        /**
        * From the base class.
		* Called by framework before exiting the dialog.
        * @param Button id.
		* @return ETrue to exit\ EFalse to not to exit.
        */
		TBool OkToExitL(TInt aButtonId);
	   
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
		* Called by framework for key event handling.
        * @param aKeyEvent.
		* @param TEventCode.
		* @return Return code.
        */
		TKeyResponse OfferKeyEventL(const TKeyEvent& aKeyEvent,TEventCode aType);
	
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
		//void GetHelpContext(TCoeHelpContext& aContext) const;
		
    	/**
		* From base class.
		* Called when UI layout changes. 
		* @param aType.
    	* @return None.
		*/
        void HandleResourceChange(TInt aType);
        

	private:   // from MAspActiveCallerObserver
	
		/**
        * From MAspActiveCallerObserver
		* Called when CAspActiveCaller completes.
        * @param aCallId.
        * @return None.
        */
		void HandleActiveCallL(TInt aCallId);
		
	
	private:   // from MAspEditorPageObserver
	
		/**
        * From MAspEditorPageObserver
		* Check setting page input.
        * @param aText.
        * @param aSettingType.
        * @return ETrue is input is valid, EFalse othherwise.
        */
		TBool CheckValidity(const TDesC& aText, TInt aSettingType);
        
		/**
        * Check setting page input.
        * @param aText.
        * @param aSettingType.
        * @return ETrue is input is valid, EFalse othherwise.
        */
		TBool CheckValidityL(const TDesC& aText, TInt aSettingType);
		
		/**
        * Handle editor event.
        * @param aEvent.
        * @param aValue.
        * @return None.
        */
		void HandleEditorEvent(TInt aEvent, TInt aValue);
		
		/**
        * Get navipane text.
        * @param aText.
        * @param aContentCount.
        * @return None.
        */
		void GetNavipaneText(TDes& aText, TInt aContentCount);

        
    private:
   		/**
		* Utility function, updates dialog cba.
		* @param aResourceId.
		* @return None.
		*/
		void UpdateCbaL(TInt aResourceId);
		
   		/**
		* Utility function, sets text editor text.
		* @param aResourceId.
		* @return None.
		*/
		void SetTextEditorTextL(TInt aResource);
		
		/**
        * For setting the background Screen
		*/
    	void SetBackgroundContextL();
    	
    	/**
        * Update color and font
		*/
    	void UpdateTextColorL();
    	

    private:

   		/**
		* Edit setting item.
		* @param aItem Setting item data.
		* @return Return code.
		*/
        TInt EditSettingItemL(CAspWizardItem* aItem);

   		/**
		* Edit setting item.
		* @param aItem Setting item data.
		* @return Return code.
		*/
        TInt EditSettingItemTextL(CAspWizardItem* aItem);

   		/**
		* Edit setting item.
		* @param aItem Setting item data.
		* @return Return code.
		*/
        TInt EditSettingItemBearerTypeL(CAspWizardItem* aItem);
        
   		/**
		* Edit setting item.
		* @param aItem Setting item data.
		* @return Return code.
		*/
        TInt EditSettingItemProtocolVersionL(CAspWizardItem* aItem);

   		/**
		* Edit setting item.
		* @param aItem Setting item data.
		* @return Return code.
		*/
        TInt EditSettingItemLocalDatabaseL(CAspWizardItem* aItem);

   		/**
		* Edit setting item.
		* @param aItem Setting item data.
		* @return Return code.
		*/
        TInt EditSettingItemSecretL(CAspWizardItem* aItem);

   		/**
		* Edit setting item.
		* @param aItem Setting item data.
		* @return Return code.
		*/
        TInt EditSettingItemProfileNameL(CAspWizardItem* aItem);

   		/**
		* Edit setting item.
		* @param aItem Setting item data.
		* @return Return code.
		*/
        TInt EditSettingItemHostAddressL(CAspWizardItem* aItem);

   		/**
		* Edit setting item.
		* @param aItem Setting item data.
		* @return Return code.
		*/
        TInt EditSettingItemContentSelectionL(CAspWizardItem* aItem);
        
   		/**
		* Edit setting item.
		* @param aItem Setting item data.
		* @return Return code.
		*/
        TInt ShowTextEditorL(TInt aResourceId, TDes& aText, const TDesC& aTitle,
                             TBool aMandatory, TBool aLatinInput, TInt aMaxLength, TInt aSettingType);

    private:
        
   		/**
		* Get navi pane text.
		* @param aText.
		* @param aCurrent.
		* @param aFinal.
		* @return Return code.
		*/
        void GetNaviPaneText(TDes& aText, TInt aCurrent, TInt aFinal);

   		/**
		* Get navi pane text.
		* @param aText.
		* @param aItem.
		* @return Return code.
		*/
        void GetNaviPaneText(TDes& aText, CAspWizardItem* aItem);
        
		/**
        * Set default setting values.
        * @param None.
        * @return None.
        */
		void SetDefaultValuesL();
        
        
   		/**
		* Show one proifle wizard editor.
		* @param None.
		* @return Return code.
		*/
        void RunWizardL();

   		/**
		* Show one proifle wizard editor.
		* @param None.
		* @return Return code.
		*/
        TInt RunWizard();
        
        void ShowPopupNoteL(TInt aResource, TInt aDelay);
        void HidePopupNote();
        void ShowErrorNoteL(TInt aError);

       
    private:
        // text editor 
        CEikRichTextEditor* iTextEditor;
        
   		// text editor text
   		HBufC* iTextEditorText;
		
  		// handles profile setting data and profile creation
  		CAspProfileWizard* iWizard;
  		
  		// dialog parameter for returning value to caller
		TAspParam* iDialogParam;

		// id of the calling application
		TInt iApplicationId;

		// sync session
		RSyncMLSession* iSyncSession;

		// long buffer for string handling
		TBuf<KBufSize255> iBuf;

		// for title and icon handling
		CStatusPaneHandler* iStatusPaneHandler;
		
		// for making function call via active scheduler
		CAspActiveCaller* iActiveCaller;
		
		// for bearer type list construction
		CAspBearerHandler* iBearerHandler;
		
		// format string from resource file
		TBuf<KBufSize> iNaviPaneTextFormat;
		
		// popup note
		CAknInfoPopupNoteController* iPopupNote;
		
		// needed for unique server id check
		CAspProfileList* iProfileList;
		
		// Background context
	    CAknsBasicBackgroundControlContext* iBgContext;
	    
	    // Flag for checking whether moving in Backward direction or not 
	    TBool iMoveBack;
    };





#endif      // ASPPROFILEWIZARDDIALOG_h
            
// End of File
