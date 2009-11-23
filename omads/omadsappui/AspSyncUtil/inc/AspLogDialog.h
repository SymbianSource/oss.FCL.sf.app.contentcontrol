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


#ifndef ASPLOGDIALOG_H
#define ASPLOGDIALOG_H


//  INCLUDES
#include <e32base.h>
#include <SyncMLHistory.h>

#include "AspDefines.h"
#include "AspProfile.h"
#include "AspResHandler.h"




/**
* CAspLogDialog
*  
* CAspLogDialog shows sync log information.
*/
NONSHARABLE_CLASS (CAspLogDialog) : public CBase
    {
	
	enum TLogItemType
		{
	    ELogAddedPhone,
	    ELogUpdatedPhone,
	    ELogDeletedPhone,
	    ELogDiscardedPhone,
	    ELogMovedPhone,
	    ELogAddedServer,
	    ELogUpdatedServer,
	    ELogDeletedServer,
	    ELogDiscardedServer,
	    ELogMovedServer
		};

	public:
	    /**
        * Launches log dialog.
		* @param None.
        * @return None.
        */
		void ShowDialogL();

    
	public:// Constructors and destructor

        /**
        * Two-phased constructor.
        */
        static CAspLogDialog* NewL(const TAspParam& aParam);
        
        /**
        * Destructor.
        */
        virtual ~CAspLogDialog();
    
    private:

        /**
        * C++ default constructor.
        */
        CAspLogDialog(const TAspParam& aParam);
	
        /**
        * By default Symbian OS constructor is private.
        */
        void ConstructL();

	private:

		/**
        * Creates text lines that are shown in log dialog.
		* @param aList.
		* @param aProfile.
		* @param aTaskList.
        * @return None.
        */
		void CreateListL(CDesCArray* aList, CAspProfile* aProfile, CAspContentList* aTaskList);
       
		/**
        * Adds one line into listbox array.
    	* @param aList.
		* @param aHistoryJob.
		* @param aTaskList.
        * @param aType.
        * @return None.
        */
        void AddLogItemsL(CDesCArray* aList, const CSyncMLHistoryJob* aHistoryJob, 
                                CAspContentList* aTaskList, TInt aType);
                                
		/**
        * Adds one line into listbox array.
    	* @param aList.
		* @param aHistoryJob.
		* @param aTaskList.
        * @return None.
        */
        void AddTaskErrorsL(CDesCArray* aList, const CSyncMLHistoryJob* aHistoryJob, 
                                CAspContentList* aTaskList);
                                
		/**
        * Gets number of synced items.
		* @param aTaskInfo.
		* @param aType.
        * @return Item count.
        */
        TInt SyncedItemCount(const CSyncMLHistoryJob::TTaskInfo& aTaskInfo, TInt aType);
                                 
		/**
        * Get log item text.
    	* @param aType.
		* @param aDataProviderId
		* @param aDisplayName
        * @return None.
        */
        HBufC* LogItemTextLC(TInt aType, TInt aDataProviderId, const TDesC& aDisplayName);
        
		/**
        * Add one line into log item list.
    	* @param aList.
		* @param aText
        * @return None.
        */
        void AddLineL(CDesCArray* aList, TDes& aText);
        
		/**
        * Gets latest history (sync) job.
		* @param aHistoryLog.
        * @return History job.
        */
        CSyncMLHistoryJob* LatestHistoryJob(RSyncMLHistoryLog& aHistoryLog);
 
		/**
        * Has server been modified during sync.
    	* @param aHistoryJob.
        * @return TBool.
        */
        TBool ServerModified(const CSyncMLHistoryJob* aHistoryJob);

		/**
        * Has phone been modified during sync.
    	* @param aHistoryJob.
        * @return TBool.
        */
        TBool PhoneModified(const CSyncMLHistoryJob* aHistoryJob);
        
		/**
        * Has there been task errors during sync.
    	* @param aHistoryJob.
        * @return TBool.
        */
        TBool HasTaskErrors(const CSyncMLHistoryJob* aHistoryJob);


	private:
        // profile id
		TInt iProfileId;
		
    	// session with sync server - only used in ConstructL
		RSyncMLSession* iSyncSession;
	
		// resource handler
		CAspResHandler* iResHandler;
		
        // for string handling
		TBuf<KBufSize255> iBuf;
		
        // for string handling
		TBuf<KBufSize255> iBuf2;

		// list of text lines that are shown in log dialog
		CDesCArrayFlat* iList;
	};





#endif      // ASPLOGDIALOG_H
            
// End of File
