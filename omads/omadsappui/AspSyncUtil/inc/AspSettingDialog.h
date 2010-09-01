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
* Description: Main view setting dialog 
*
*/


#ifndef ASPSETTINGDIALOG_H
#define ASPSETTINGDIALOG_H


//  INCLUDES
#include <e32base.h>
#include <eikdialg.h>
#include <eiklbo.h>
#include <aknlists.h>
#include <AknDialog.h>
#include <aknnavi.h>
#include <AknInfoPopupNoteController.h> // CAknInfoPopupNoteController

#include "AspDefines.h"
#include "AspDialogUtil.h"
#include "AspResHandler.h"
#include "AspDbNotifier.h"
#include "AspSyncHandler.h"


// FORWARD DECLARATIONS

// CONSTANTS

const TInt KSettingDialogPopupDisplayTime = 3*1000;



// CLASS DECLARATION

/**
*  CAspSettingDialog
*
* This is sync app  main dialog.
*/
NONSHARABLE_CLASS (CAspSettingDialog) : public CAknDialog, public MEikListBoxObserver,
                                        public MAspDbEventHandler, public MAspSyncObserver
	{
	
	enum TAspSettingDialogIconIndex
		{
	    EIconIndexHTTP = 0,
        EIconIndexBT   = 1,
	    EIconIndexUSB  = 2,
	    EIconIndexIrda = 3,
	    EIconIndexSyncOff = 4,
		EIconIndexMultipleContent = 5
		};

	public:

	    /**
        * Launches setting dialog.
		* @param aParam Class that contains dialog parameters.
        * @return Return code.
        */
		static TBool ShowDialogL(TAspParam& aParam);
		
    public:// Constructors and destructor

        /**
        * Two-phased constructor.
        */
        static CAspSettingDialog* NewL(TAspParam& aParam);
        
        /**
        * Destructor.
        */
        virtual ~CAspSettingDialog();
    
    private:

        /**
        * C++ default constructor.
        */
        CAspSettingDialog(TAspParam& aParam);
	
        /**
        * By default Symbian OS constructor is private.
        */
        void ConstructL();

	private:
       
        /**
        * Called by framework before the dialog is shown.
        * @param None
		* @return None
        */
		void PostLayoutDynInitL();

        /**
        * Called by framework after the dialog is shown.
        * @param None
		* @return None
        */
		void PreLayoutDynInitL();

		/**
		* Handles menu events.
		* @param  aCommandId Command id.
		* @return None.
		*/
		void ProcessCommandL(TInt aCommandId);
	
		/**
		* Handles menu events, ProcessCommandL calls this.
		* @param  aCommandId Command id.
		* @return None.
		*/
		void DoProcessCommandL(TInt aCommandId);

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
		* Called by framework before exiting the dialog.
        * @param Button id.
		* @return ETrue to exit\ EFalse to not to exit.
        */
		TBool OkToExitL(TInt aButtonId);

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
		* Called when display resolution changes.
        * @param aType.
		* @return None.
        */
		void HandleResourceChange(TInt aType);
		
		/**
		* Sets settings dialog icons.
        * @param None.
		* @return None.
        */
		void SetIconsL();
		

    private: // from MEikListBoxObserver
        /**
        * From MEikListBoxObserver, called by framework.
        * @param aListBox.
        * @param aEventType.
		* @return None
        */
		void HandleListBoxEventL(CEikListBox* aListBox, TListBoxEvent aEventType);
	
    private:  // from MAspDbEventHandler
		/**
        * From MAspDbEventHandler
        * @param aStatus EClose, EUnlock, ECommit, ERollback, or ERecover (d32dbms.h).
        * @return None. 
        */
		void HandleDbEventL(TAspDbEvent aEvent);

	private: // from MAspActiveCallerObserver
		/**
        * From MAspActiveCallerObserver
		* Called when CAspActiveCaller completes.
        * @param aCallId.
        * @return None.
        */
		void HandleActiveCallL(TInt aCallId);

	private: // from MAspSyncObserver
		/**
        * From MAspSyncObserver
		* Called when synchronization completes.
        * @param aError.
        * @param aInfo1.
        * @return None.
        */
		void HandleSyncEventL(TInt aError, TInt aInfo1);


    private:
    
    
        	/**
		* Handles setting item editing.
		* @param None.
		* @return None.
		*/
		void HandleOKL();
		
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
        * Creates set type profile.
		* @param None.
        * @return None.
        */
		void CreateProfileL();

		/**
        * Creates application type profile.
		* @param None.
        * @return None.
        */
		void CreateAppProfileL();

		/**
        * Creates application type profile for sync app.
		* @param None.
        * @return None.
        */
		TBool CreateAppProfileFromSyncAppL();
        		
		/**
        * Changes active profile.
		* @param None.
        * @return None.
        */
        void ChangeProfileL();
        
		/**
        * Shows profile dialog.
		* @param aEditMode.
		* @param aFocus 
        * @return None.
        */
		void EditProfileL(TInt aEditMode ,TInt aFocus);

		/**
        * Shows profile dialog.
		* @param TInt aEditMode.
		* @param TInt aFocus.
        * @return None.
        */
		void DoEditProfileL(TInt aEditMode ,TInt aFocus);

		/**
        * Syncs selected profile.
		* @param None.
        * @return None.
        */
		void SyncProfileL();

		/**
        * Syncs selected profile.
		* @param None.
        * @return None.
        */
		void CopyFromServerL();

		/**
        * Syncs selected profile.
		* @param aSyncType.
        * @return None.
        */
		void DoSyncProfileL(TInt aSyncType);

		/**
        * Shows context menu.
		* @param aResource.
        * @return None.
        */
        void ShowContextMenuL(TInt aResource);
        
		/**
		* Initialilzes context menu.
		* @param aMenuPane.
		* @return None.
		*/
		void InitContextMenuL(CEikMenuPane* aMenuPane);

		/**
		* Initialilzes options menu.
		* @param aMenuPane.
		* @return None.
		*/
		void InitOptionsMenuL(CEikMenuPane* aMenuPane);
		
		/**
		* Deletes sync profile.
		* @param None.
		* @return None.
		*/
		void DeleteProfileL();
        
		/**
		* Shows log dialog.
		* @param None.
		* @return None.
		*/
		void ShowLogDialogL();

		/**
		* Shows auto sync dialog.
		* @param None.
		* @return None.
		*/
		void ShowAutoSyncDialogL();
		
		/**
		* Shows popup info note about current profile.
		* @param None.
		* @return None.
		*/
		void ShowCurrentProfileInfoL();

		/**
		* Shows popup info note about current profile.
		* @param None.
		* @return None.
		*/
		void ShowCurrentProfileInfo();

		/**
		* Shows popup info note about current profile.
		* @param aProfileId.
		* @return Boolean.
		*/
		TBool ShowAutoSyncProfileInfoL();

		/**
		* Shows popup info note about current profile.
		* @param aProfileId.
		* @return Boolean.
		*/
		TBool ShowAutoSyncProfileInfo();
		/**
		* Update the MSK Label
		* @param aCommandId command id.
		* @param aResourceId resource id .
	    */
		void UpdateMiddleSoftKeyLabelL(TInt aCommandId,TInt aResourceId);

		void ShowAutoSyncLog(TAny* /*aAny*/);

    private:

		/**
		* Utility function.
		* @param None.
		* @return Dialog listbox.
		*/
		CAknDoubleLargeStyleListBox* ListBox();
		
	   /**
		* Utility function.
		* @param None.
		* @return None
		* To toggle Syncmlmail checkbox in profile settings
		*/
		void CancelMailBoxForEnterL();

		/**
		* Utility function.
		* @param None.
		* @return Sync handler.
		*/
        TBool SyncRunning();

		/**
		* Utility function.
		* @param aProfileId.
		* @return Profile's listbox index.
		*/
		TInt ProfileIndex(TInt aProfileId);

		/**
		* Utility function, sets listbox current item.
		* @param None.
		* @return None.
		*/
		void SetCurrentIndex();

		/**
		* Utility function, stores listbox current item.
		* @param aIndex.
		* @return Selected item index.
		*/
		void SetCurrentIndex(TInt aIndex);

		/**
		* Utility function, gets listbox current item.
		* @param None.
		* @return Current index.
		*/
		TInt CurrentIndex();
		
		/**
		* Utility function, updates dialog cba.
		* @param aResourceId.
		* @return None.
		*/
		void UpdateCbaL(TInt aResourceId);

		/**
		* Sets currrent profile.
		* @param aProfileId.
		* @return None.
		*/
		void DoSetCurrentProfileL(TInt aProfileId);

		/**
		* Sets currrent profile.
		* @param None.
		* @return None.
		*/
		void SetCurrentProfileL();

		/**
		* Utility function.
		* @param None.
		* @return Selected profile.
		*/
		TAspProfileItem& CurrentProfileL();
		
		/**
		* Utility function.
		* @param None.
		* @return ETrue if current profile exists, EFalse otherwise.
		*/
		TBool HasCurrentProfile();
		
		/**
		* Utility function.
		* @param aProfileId.
		* @return ETrue if current profile has id aProfileId, EFalse otherwise.
		*/
		TBool IsCurrentProfile(TInt aProfileId);

		/**
		* Save current profile.
		* @param None.
		* @return None.
		*/
		void DoSaveCurrentProfileL();

		/**
		* Save current profile.
		* @param None.
		* @return None.
		*/
		TInt SaveCurrentProfile();
		
		/**
		* Update content selection.
		* @param None.
		* @return None.
		*/
		void UpdateContentSelectionL();
		
		/**
		* Update profile name.
		* @param None.
		* @return None.
		*/
		void UpdateProfileNameL();
		
		/**
		* Check profile mandatory data
		* @param None.
		* @return Return code.
		*/
		TInt CheckMandatoryDataL();

		/**
		* Check profile mandatory data
		* @param None.
		* @return Return code.
		*/
		TInt DoCheckMandatoryDataL();

		/**
		* Update navi pane text.
		* @param aText.
		* @return None.
		*/
        void UpdateNavipaneL(const TDesC& aText);
        
		/**
		* Show popup note.
		* @param aText.
		* @return None.
		*/
        void ShowPopupNoteL(const TDesC& aText);
        
		/**
		* Cancels checkbox select/unselect.
		* @param None.
		* @return None.
		*/
        void CancelCheckboxEventL();
        
		/**
		* Cancels checkbox select/unselect.
		* @param None.
		* @return None.
		*/
        void CancelCheckboxEvent();

		/**
		* Checks whether email content can be selected.
		* @param None.
		* @return EFalse if selection is not possible.
		*/
        TBool CheckEmailSelectionL();

		/**
		* Checks whether email content can be selected.
		* @param None.
		* @return EFalse if selection is not possible.
		*/
        TBool CheckEmailSelection();
        
      public:
      
        /**
        * Updates listbox items.
        * @param aUpdateProfileList.
        * @return None.
        */
		void UpdateListBoxL();
		
        
        /**
		* Sets currrent profile.
		* @param aProfileId.
		* @return None.
		*/
		void SetCurrentProfile(TInt aProfileId);
		
		/**
		* Delete Auto sync history
		* @param None.
		* @return None.
		*/
		void DeleteAutoSyncHistory();   
		
#ifdef RD_DSUI_TIMEDSYNC 

		void SetNextProfileL();
        void SetPreviousProfileL();
        void UpdateTabsL();
        
#endif
		void UpdateMarkMenuItem(CEikMenuPane* aMenuPane);
		void SaveSelectionL();


		
    /**
		* Checks whether the profile is already opened in another application.
		* @param aProfileId.
		* @return ETrue if profile locked ,else EFalse.
		*/        
        TBool IsProfileLockedL(TInt aProfileId);
     
     
     /**
		* Checks whether sync is ongoing for a profile
		* @param aProfileId.
		* @return ETrue if sync ongoing ,else EFalse.
		*/        
        TBool IsSyncOnGoingL(TInt aProfileId);

#ifdef _DEBUG
    private: 
        void TestL();
        void LogDatabaseEvent(TAspDbEvent aEvent);
#endif

		

    private:
   		// list that contains all contents (applications)
		CAspContentList* iContentList;

		// id of the calling application
		TInt iApplicationId;

		// id of the dialog mode
		TInt iDialogMode;

        // profile list (from CNSmlDSSettings)
		CAspProfileList* iProfileList;

		// list of profiles
		CAknDoubleLargeStyleListBox* iSettingListBox;

		// for title and icon handling
		CStatusPaneHandler* iStatusPaneHandler;
		
		// for hiding tabs
		CAknNavigationControlContainer* iNaviPane;

    	// resource handler
		CAspResHandler* iResHandler;

		// for getting settings database notifications 
		CAspDbNotifier* iDbNotifier;

		// currently selected listbox item
		TInt iCurrentListBoxIndex;

		// performs sync operation, uses CNSmlAppEngine
		CAspSyncHandler* iSyncHandler;
		
		// session with sync server
		RSyncMLSession* iSyncSession;
		
		// is setting database in read only state
		TBool iSettingEnforcement;
		
		// long buffer for string handling
		TBuf<KBufSize255> iBuf;

		// currently open profile id
		TInt iCurrentProfileId;

		// current profile name
		TBuf<KAspMaxProfileNameLength> iCurrentProfileName;
		
		// popup note
        CAknInfoPopupNoteController* iPopupNote;
        
		// for bearer type list construction
		CAspBearerHandler* iBearerHandler;
		
		// to know whether autosyncdialog is active or not
		TBool iAutoSyncDialog;

#ifdef RD_DSUI_TIMEDSYNC	
		// Tab group owned by the navigation decorator.
        CAspTabbedNaviPaneHandler* iTabHandler;

		//Set if any profile is edited or profile count changed
		TBool iUpdateTabGroup;
#endif

		
    };


#endif    // ASPSETTINGDIALOG_H
            
// End of File
