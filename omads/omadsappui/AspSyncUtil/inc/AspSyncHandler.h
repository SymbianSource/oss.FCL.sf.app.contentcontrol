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
* Description: Creates sync job and receives progress events 
*
*/


#ifndef ASPSYNCHANDLER_H
#define ASPSYNCHANDLER_H

// INCLUDES
#include <e32base.h>
#include <SyncMLObservers.h>
#include <textresolver.h>


#include "AspProgressDialog.h"
#include "AspState.h"
#include "AspProfile.h"



// CLASS DECLARATION


/**
* MAspProgressDialogObserver
* 
* Observer interface function for observing sync events.
*/
NONSHARABLE_CLASS (MAspSyncObserver)
    {
    public:
        /**
        * Callback method
        * Get's called when a synchronization has completed.
        */
        virtual void HandleSyncEventL(TInt aError, TInt aInfo1) = 0;
    };


/**
* TAspSyncParam
*
* This class is used as sync parameter type.
*/
NONSHARABLE_CLASS (TAspSyncParam)
	{
    public:
    	TAspSyncParam(TInt aApplicationId);
        
    public:
		// calling application id
		TInt iApplicationId;

		// profile id
		TInt iProfileId;

		// job id
		TInt iJobId;

		// taskId
		TInt iTaskId;

        // sync direction
		TInt iSyncDirection;
		
		// progress dialog modality
		TInt iDialogMode;
	};


/**
* CAspSyncHandler
* 
* CAspSyncHandler handles data synchronizing.
*/
NONSHARABLE_CLASS (CAspSyncHandler) : public CBase, 
                        public MAspProgressDialogObserver,
						public MAspActiveCallerObserver,
						public MSyncMLEventObserver,
						public MSyncMLProgressObserver
						
	{

    public:
        /**
        * Two-phased constructor.
        */
	    static CAspSyncHandler* NewL(const TAspParam& aParam);

	    /**
        * Destructor.
        */
	    virtual ~CAspSyncHandler();

    private:
	    /**
        * By default Symbian 2nd phase constructor is private.
        */
	    void ConstructL();

	    /**
        * C++ default constructor.
        */
	    CAspSyncHandler(const TAspParam& aParam);

		
	private:	
        /**
        * From MAspProgressDialogObserver
    	* Gets called when sync progress dialog closes.
        * @param aButtonId Button id.
        * @return None
        */
		void HandleDialogEventL(TInt aButtonId);

    
	private: // from MAspActiveCallerObserver
		/**
        * From MAspActiveCallerObserver
		* Called when CAspActiveCaller completes.
        * @param aCallId.
        * @return None.
        */
		void HandleActiveCallL(TInt aCallId);


    private:    // from MSyncMLEventObserver
        
		/**
        * From MSyncMLEventObserver
		* Called when SyncML session events oocur.
        * @param aEvent.
        * @param aIdentifier.
        * @param aError.
        * @param aAdditionalData.
        * @return None.
        */
        void OnSyncMLSessionEvent(TEvent aEvent, TInt aIdentifier, TInt aError, TInt aAdditionalData);
	
	
    private:  //from MSyncMLProgressObserver
	    
	    /**
	    * Receives notification of a synchronisation error.
        * @param aErrorLevel  The error level.
	    * @param aError		  The type of error. This is one of the SyncMLError error values.	
	    * @param aTaskId      The ID of the task for which the error occurred.
	    * @param aInfo1       An integer that can contain additional information about the error. Normally 0.
	    * @param aInfo2       An integer that can contain additional information about the error. Normally 0.
	    * @return             None.
	    */
	    void OnSyncMLSyncError(TErrorLevel aErrorLevel, TInt aError, TInt aTaskId, TInt aInfo1, TInt aInfo2);
       	
       	/**
	    * Receives notification of synchronisation progress.
	    * @param aStatus	The current status, e.g. 'Connecting'.
	    * @param aInfo1	An integer that can contain additional information about the progress.
	    * @param aInfo2	An integer that can contain additional information about the progress.
	    */
	    void OnSyncMLSyncProgress(TStatus aStatus, TInt aInfo1, TInt aInfo2);
	    
	    /**
	    * Receives notification of modifications to synchronisation tasks.
        * @param aTaskId               The ID of the task.
	    * @param aClientModifications  Modifications made on the clients Data Store.
	    * @param aServerModifications  Modifications made on the server Data Store.
	    */
	    void OnSyncMLDataSyncModifications(TInt aTaskId, const TSyncMLDataSyncModifications& aClientModifications, const TSyncMLDataSyncModifications& aServerModifications);

	private:
	    void OnSyncMLDataSyncModificationsL(TInt aTaskId, const TSyncMLDataSyncModifications& aClientModifications, const TSyncMLDataSyncModifications& aServerModifications);
	    
	    
	private:
        /**
        * Shows progress dialog.
		* @param None.
        * @return Error code.
        */
        void ShowProgressDialogL();

        /**
        * Shows error dialog.
		* @param None.
        * @return Error code.
        */
        void ShowErrorNote();
        
        /**
        * Completes sync operation.
		* @param aError.
        * @return None.
        */
        void SynchronizeCompleted(TInt aError);
	
	public:
	
        /**
        * Performs synchronization.
		* @param aSyncParam.
        * @return None
        */
        void SynchronizeL(TAspSyncParam& aSyncParam);
        
        /**
        * Sets sync observer.
		* @param aObserevr.
        * @return None
        */
        void SetObserver(MAspSyncObserver* aObserver);

    
    private:
        /**
        * Performs synchronization.
		* @param aSyncParam.
        * @return None
        */
        void DoSynchronizeL(TAspSyncParam& aSyncParam);

	private:
		/**
        * Utility function.
        * @return CAspResHandler.
        */
		CAspResHandler* ResHandlerL();
		
		/**
        * Utility function.
        * @return Progress dialog.
        */
		CAspProgressDialog* Dialog();

		/**
        * Utility function.
        * @return Sync state.
        */
		CAspState* State();

    public:

		/**
        * Utility function.
        * @return ETrue if sync is currently running, EFalse otherwise.
        */
		TBool SyncRunning();

    private:
		/**
        * Updates progress dialog.
        * @param None.
    	* @return None.
        */
		void UpdateDialogL();

		/**
        * Updates progress dialog.
        * @param None.
    	* @return None.
        */
		void UpdateDialog();

		/**
        * Get list profiles that can be used for syncing.
        * @param aApplicationId Application (sync content) id.
    	* @return None.
        */
		CAspProfileList* GetProfileListLC(TInt aApplicationId);
		
		/**
        * Utility function.
        * @param None.
        * @return None.
        */
        void CreateContentListL();

		/**
        * Utility function.
        * @param None.
        * @return ETrue if server alert sync, EFalse otherwise.
        */
        TBool IsServerAlertSync();

		/**
        * Opens sync session.
        * @param None.
        * @return None.
        */
        void OpenSyncSessionL();

		/**
        * Closes sync session.
        * @param None.
        * @return None.
        */
        void CloseSyncSession();
        
		/**
        * Calculates progress count.
        * @param aC Phone side sync events.
        * @param aS Server side sync events.
        * @return Progress count.
        */
        TInt ProgressCount(const TSyncMLDataSyncModifications& aC,
                           const TSyncMLDataSyncModifications& aS);
                           
		/**
        * Checks that sync profile has valid local database.
        * @param aProfileId.
        * @return None.
        */
        void CheckLocalDatabaseL(TInt aProfileId);
        
		/**
        * Checks whether progress dialog needs to be updated.
        * @param None.
        * @return Boolean.
        */
        TBool UpdateRequired();
        
		/**
        * Completes sync with short delay.
        * @param aError.
        * @return None.
        */
        void CompleteWithDelay(TInt aError);
        
        TBool IsRoamingL();
        
        void ReadRepositoryL(TInt aKey, TInt& aValue);
        
        TInt BearerTypeL(TInt aProfileId);
        
#ifdef _DEBUG
    private:  // debug code
        void GetSyncStatusText(TDes& aText, MSyncMLProgressObserver::TStatus aStatus);
        void GetSyncEventText(TDes& aText, MSyncMLEventObserver::TEvent aEvent);
        void LogSessionEvent(TEvent& aEvent, TInt aIdentifier, TInt aError);
        void LogProgressEvent(TStatus& aStatus, TInt aInfo1);
        void LogErrorEvent(TErrorLevel aErrorLevel, TInt aError);
        void LogModificationEvent(TInt aTaskId, const TSyncMLDataSyncModifications& aC, const TSyncMLDataSyncModifications& aS);
        void LogModificationEvent(TInt aTaskId, const TDesC& aText);
#endif

    private:
		// id of the calling application
		TInt iApplicationId;

		// profile id
		TInt iProfileId;
		
		// sync session
		RSyncMLSession iSyncSession;

		// sync job
		RSyncMLDataSyncJob iSyncJob;
		
		// needed for getting data provider names
		CAspContentList* iContentList;
		
		// shows sync progress dialog
		CAspProgressDialog* iProgressDialog;

		// sync handler state
		CAspState* iState;

		// reads strings
		CAspResHandler* iResHandler;

		// for making function call via active scheduler
		CAspActiveCaller* iActiveCaller;

		// is sync currently running
		TBool iSyncRunning;
		
		// is current sync servert alert sync
		TBool iServerAlertSync;

		// is sync session open
		TBool iSyncSessionOpen;
		
		// sync error code
		TInt iSyncError;
		
		// current sync job id
		TInt iCurrentSyncJobId;
		
		// sync observer, used for sending sync completion event
		MAspSyncObserver* iSyncObserver;

        // for converting error code to error string
        CTextResolver* iTextResolver;
        
		// has "sync completed" event arrived 
		TBool iCompleteEventReceived;
		
		// has "sync stopped" event arrived 
		TBool iStopEventReceived;
		
		// sync modality 
		TBool iIsSynchronousOperation;
		
	    // for stopping code until sync is finished
    	CActiveSchedulerWait* iWait;
		
	};
	
	


	
#endif  // ASPSYNCHANDLER_H


// End of file