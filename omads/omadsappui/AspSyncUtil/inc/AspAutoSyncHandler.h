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
* Description: Manages schedules using Symbian Task scheduler  
*
*/


#ifndef ASPAUTOSYNCHANDLER_H
#define ASPAUTOSYNCHANDLER_H

// INCLUDES
#include <e32base.h>
#include <SyncMLClient.h>    // for RSyncMLSession
#include <SyncMLObservers.h>
#include <schtask.h>

#include "AspDefines.h"
#include "AspUtil.h"
#include "AspSchedule.h"


// CLASS DECLARATION

/**
* CAspAutoSyncHandler
*  
*/
NONSHARABLE_CLASS (CAspAutoSyncHandler) : public CBase,
                                          public MSyncMLEventObserver,
						                  public MSyncMLProgressObserver
    {
    public:
    enum TAutoSyncError
        {
	    EOtherSyncRunning      = -100,
	    EIncorrectProfile      = -101,
	    EIncorrectSchedule     = -102
        };

	public:// Constructors and destructor

        /**
        * Two-phased constructor.
        */
        static CAspAutoSyncHandler* NewL();
        
        /**
        * Destructor.
        */
        virtual ~CAspAutoSyncHandler();
    
    private:

        /**
        * C++ default constructor.
        */	
        CAspAutoSyncHandler();
	
        /**
        * By default Symbian OS constructor is private.
        */
        void ConstructL();
		
    
    public:
        void SynchronizeL(RFile& aTaskFile);

		void DoSynchronizeL();
        
    private:
        void OpenSyncSessionL();
        void CloseSyncSession();
        TInt CheckMandatoryDataL(TInt aProfileId);
        TBool IsRoamingL();
        TBool IsRoaming();
                

    private: // from MSyncMLEventObserver
        
        void OnSyncMLSessionEvent(TEvent aEvent, TInt aIdentifier,
                                  TInt aError, TInt aAdditionalData);
	
    private: //from MSyncMLProgressObserver
	    
	    void OnSyncMLSyncProgress(TStatus aStatus, TInt aInfo1, TInt aInfo2);
	    void OnSyncMLDataSyncModifications(TInt aTaskId, 
        const TSyncMLDataSyncModifications& aClientModifications,
        const TSyncMLDataSyncModifications& aServerModifications);
	    void OnSyncMLSyncError(TErrorLevel aErrorLevel, TInt aError,
	                           TInt aTaskId, TInt aInfo1, TInt aInfo2);

   
		
    private:
		
		// sync session
		RSyncMLSession iSyncSession;

		// sync job
		RSyncMLDataSyncJob iSyncJob;

		// is sync currently running
		TBool iSyncRunning;
		
    	// sync error code
		TInt iSyncError;
		
		// is sync session open
		TBool iSyncSessionOpen;
		
		// for stopping code until sync is finished
		CActiveSchedulerWait* iWait;
		
		// for reading auto sync settings
		CAspSchedule* iSchedule;
	};


/**
* CAspSyncSchedule
*  
*/
NONSHARABLE_CLASS (CAspSyncSchedule) : public CBase
    {


	public:// Constructors and destructor

        /**
        * Two-phased constructor.
        */
        static CAspSyncSchedule* NewL();
        
        /**
        * Two-phased constructor.
        */
        static CAspSyncSchedule* NewLC();

        /**
        * Destructor.
        */
        virtual ~CAspSyncSchedule();
    
    private:

        /**
        * C++ default constructor.
        */	
        CAspSyncSchedule();
	
        /**
        * By default Symbian OS constructor is private.
        */
        void ConstructL();

	public:

		/**
        * Delete existing schedule.Called before creating a new schedule 
        */
        void DeleteScheduleL(CAspSchedule* aAspSchedule);
        void DoDeleteScheduleL(CAspSchedule* aAspSchedule);

		/**
        * Create a new schedule based on changed settings
        */
		void CreatePeakScheduleL(CAspSchedule* aAspSchedule);
		void CreateOffPeakScheduleL(CAspSchedule* aAspSchedule);

		/**
        * Edit the schedule to start after 15 min/30 mins.This is needed as task scheduler does not support 
        * schedulling interval less than a hour
        */
	
		void EditPeakScheduleL(CAspSchedule* aAspSchedule);
		void EditOffPeakScheduleL(CAspSchedule* aAspSchedule);	

		/**
        *  Post pone the scheduled sync in till next valid time
        */

		void UpdatePeakScheduleL(CAspSchedule* aAspSchedule);
		void UpdateOffPeakScheduleL(CAspSchedule* aAspSchedule);

		/**
        *  Enable schedules
        */
		void EnableSchedule(CAspSchedule* aAspSchedule);
		
#ifdef _DEBUG
    public:
        void LogScheduleL(CAspSchedule* aAspSchedule);
        TInt LogSchedule(CAspSchedule* aAspSchedule);
        static void LogIntervalType(TScheduleEntryInfo2& aInfo, TDes& aText);
        static void LogInterval(TScheduleEntryInfo2& aInfo, TDes& aText);
        static void LogTsTime(TTsTime aTsTime, TDes& aText);
        static void LogTaskInfo(TTaskInfo& aInfo, TDes& aText);
#endif        
        
    public:

		/**
        *  Get next sync time.Reads due time from task scheduler.
        */       
		void GetPeakStartTimeL(CAspSchedule* aAspSchedule, TTime& aTime, TBool aHomeTime);
        void GetOffPeakStartTimeL(CAspSchedule* aAspSchedule, TTime& aTime, TBool aHomeTime);
        
    public:
        TIntervalType IntervalType(TInt aInterval);
        TTsTime StartTime(TInt aStartHour, TInt aInterval);
        TInt Interval(TInt aInterval);
        
		
    private:
		//task scheduler server session
		RScheduler iScheduler;


		// schedule entry list
		CArrayFixFlat<TScheduleEntryInfo2>* ipeakEntryList;
		CArrayFixFlat<TScheduleEntryInfo2>* ioffPeakEntryList;

		// schedule entry list
		CArrayFixFlat<TTaskInfo>* ipeakTaskList;
		CArrayFixFlat<TTaskInfo>* ioffPeakTaskList;
		// long buffer for string handling
		TBuf<KBufSize255> iBuf;
	};



#endif  // ASPAUTOSYNCHANDLER_H

// End of file