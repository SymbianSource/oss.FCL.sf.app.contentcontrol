/*
* Copyright (c) 2002-2005 Nokia Corporation and/or its subsidiary(-ies). 
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



// INCLUDE FILES
#include "AspAutoSyncHandler.h"
#include "AspUtil.h"
#include "AspDialogUtil.h"
#include "AspDebug.h"

#include <schinfo.h> // TScheduleEntryInfo2
#include <s32mem.h>  // RDesWriteStream
#include <s32file.h> // RFileReadStream
//#include <centralrepository.h>  // CRepository
#include <rconnmon.h>




_LIT(KPeakTaskData, "aspschedulehandler peak task");
_LIT(KOffPeakTaskData, "aspschedulehandler off-peak task");


// ============================ MEMBER FUNCTIONS ===============================


	
/*******************************************************************************
 * class CAspAutoSyncHandler
 *******************************************************************************/

// -----------------------------------------------------------------------------
// CAspAutoSyncHandler::NewL
//
// -----------------------------------------------------------------------------
//
CAspAutoSyncHandler* CAspAutoSyncHandler::NewL()
    {
    CAspAutoSyncHandler* self = new (ELeave) CAspAutoSyncHandler();
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);

    return(self);
    }


// -----------------------------------------------------------------------------
// CAspAutoSyncHandler::CAspAutoSyncHandler
// 
// -----------------------------------------------------------------------------
//
CAspAutoSyncHandler::CAspAutoSyncHandler()
	{
	iSyncError = KErrNone;
	iSyncSessionOpen = EFalse;
	iSyncRunning = EFalse;
    }


// -----------------------------------------------------------------------------
// CAspAutoSyncHandler::ConstructL
//
// -----------------------------------------------------------------------------
//
void CAspAutoSyncHandler::ConstructL()
    {
    iWait = new (ELeave) CActiveSchedulerWait();
    
    iSchedule = CAspSchedule::NewL();
    } 


// ----------------------------------------------------------------------------
// Destructor
//
// ----------------------------------------------------------------------------
//
CAspAutoSyncHandler::~CAspAutoSyncHandler()
    {
    if (iWait && iWait->IsStarted())
        {
        iWait->AsyncStop();
        }
    delete iWait;
    
    delete iSchedule;
    
    CloseSyncSession();
    }


// -----------------------------------------------------------------------------
// CAspAutoSyncHandler::OnSyncMLSessionEvent (from MSyncMLEventObserver)
//
// -----------------------------------------------------------------------------
//
void CAspAutoSyncHandler::OnSyncMLSessionEvent(TEvent aEvent, TInt /*aIdentifier*/,
                                           TInt aError, TInt /*aAdditionalData*/)
	{
	FLOG( _L("CAspAutoSyncHandler::OnSyncMLSessionEvent START") );
	
	if (aEvent == EJobStart)
		{
        FLOG( _L("EJobStart") );
		}

	if (aEvent == EJobStop)
		{
    	if (aError != KErrNone)
			{
			iSyncError = aError;
    		}

        FLOG( _L("EJobStop") );
        
    	iWait->AsyncStop();
		}
    	
	if (aEvent == EJobStartFailed || aEvent == EJobRejected || aEvent == EServerTerminated)
		{
		FLOG( _L("EJobStartFailed or EJobRejected or EServerTerminated") );
		iWait->AsyncStop();
		}
		
	FLOG( _L("CAspAutoSyncHandler::OnSyncMLSessionEvent END") );
    }



// -----------------------------------------------------------------------------
// CAspAutoSyncHandler::OnSyncMLSyncError (from MSyncMLProgressObserver)
//
// -----------------------------------------------------------------------------
//
void CAspAutoSyncHandler::OnSyncMLSyncError(TErrorLevel aErrorLevel,
                                                TInt aError, TInt /*aTaskId*/,
                                                TInt /*aInfo1*/, TInt /*aInfo2*/)
	{
	FLOG( _L("CAspAutoSyncHandler::OnSyncMLSyncError START") );
	
	if (aErrorLevel == ESmlFatalError)
		{
		iSyncError = aError;
		iWait->AsyncStop();
		}

	FLOG( _L("CAspAutoSyncHandler::OnSyncMLSyncError END") );
	}


// -----------------------------------------------------------------------------
// CAspAutoSyncHandler::OnSyncMLSyncProgress (from MSyncMLProgressObserver)
//
// -----------------------------------------------------------------------------
//
void CAspAutoSyncHandler::OnSyncMLSyncProgress(TStatus /*aStatus*/,
                                                   TInt /*aInfo1*/, TInt /*aInfo2*/)
	{
	// not needed in scheduled sync
	}


// -----------------------------------------------------------------------------
// CAspAutoSyncHandler::OnSyncMLDataSyncModifications (from MSyncMLProgressObserver)
//
// -----------------------------------------------------------------------------
//
void CAspAutoSyncHandler::OnSyncMLDataSyncModifications(TInt /*aTaskId*/, 
	         const TSyncMLDataSyncModifications& /*aClientModifications*/,
	         const TSyncMLDataSyncModifications& /*aServerModifications*/)
	{
	// not needed in scheduled sync
	}
	

// -----------------------------------------------------------------------------
// CAspAutoSyncHandler::OpenSyncSessionL
// 
// -----------------------------------------------------------------------------
//
void CAspAutoSyncHandler::OpenSyncSessionL()
	{
	if (!iSyncSessionOpen)
		{
        TRAPD(err, iSyncSession.OpenL());
        
        if (err != KErrNone)
        	{
        	FLOG( _L("### CAspAutoSyncHandler: RSyncMLSession::OpenL failed (%d) ###"), err );
        	User::Leave(err);
        	}

		iSyncSessionOpen = ETrue;
		}
    }


// -----------------------------------------------------------------------------
// CloseSyncSession
// 
// -----------------------------------------------------------------------------
//
void CAspAutoSyncHandler::CloseSyncSession()
	{
	if (iSyncSessionOpen)
		{
		iSyncJob.Close(); // this can be done when job has not been opened
		iSyncSession.Close();
		iSyncSessionOpen = EFalse;
		}
    }


// -----------------------------------------------------------------------------
// CAspAutoSyncHandler::CheckMandatoryData
// 
// -----------------------------------------------------------------------------
//
TInt CAspAutoSyncHandler::CheckMandatoryDataL(TInt aProfileId)
	{
    TAspParam param(EApplicationIdSync, &iSyncSession);

    CAspProfile* profile = CAspProfile::NewLC(param);
    profile->OpenL(aProfileId, CAspProfile::EOpenRead, CAspProfile::EAllProperties);
    
	param.iProfile = profile;
	param.iMode = CAspContentList::EInitAll;
	CAspContentList* list = CAspContentList::NewLC(param);

   	//TInt contentCount = 0;
	TInt mandatoryConnectionData = CAspProfile::CheckMandatoryConnData(profile);
	//TInt mandatoryContentData    = list->CheckMandatoryDataL(contentCount);
	
	TInt mandatoryBearerType = EMandatoryOk;
	if (profile->BearerType() != EAspBearerInternet)
		{
		mandatoryBearerType = EMandatoryIncorrectBearerType;
		}
		
	TInt mandatoryAccessPoint = EMandatoryOk;
	/*if (profile->AccessPointL() == CAspAccessPointHandler::KAskAlways)
		{
		mandatoryAccessPoint = EMandatoryIncorrectAccessPoint;
		}*/
	
	CleanupStack::PopAndDestroy(list);
	CleanupStack::PopAndDestroy(profile);
	
	if (mandatoryBearerType != EMandatoryOk)
		{
		return mandatoryBearerType;
		}

	if (mandatoryAccessPoint != EMandatoryOk)
		{
		return mandatoryAccessPoint;
		}

	if (mandatoryConnectionData != EMandatoryOk)
		{
		return mandatoryConnectionData;
		}
		
	return EMandatoryOk;//mandatoryContentData;
    }


// -----------------------------------------------------------------------------
// CAspAutoSyncHandler::SynchronizeL
// 
// -----------------------------------------------------------------------------
//
void CAspAutoSyncHandler::SynchronizeL(RFile& aTaskFile)
	{
	
	FLOG( _L("CAspAutoSyncHandler::SynchronizeL START") );

	CFileStore*         store;
    RStoreReadStream    instream;
    // Get tasks from scheduler's store
    store = CDirectFileStore::FromLC(aTaskFile);
    instream.OpenLC(*store,store->Root());
    TInt count = instream.ReadInt32L();

	CScheduledTask* task = CScheduledTask::NewLC(instream);
    HBufC* taskData = const_cast<HBufC*>(&(task->Data()));
    TPtr ptr = taskData->Des();
    TBool doCleanUp = ETrue;

	if (ptr.Compare(KPeakTaskData) == 0)
		{
	 FLOG( _L("Peak Sync Scheduled") );
		TInt interval = iSchedule->SyncPeakSchedule();
		if (iSchedule->IntervalType(interval) == EHourly
					&& interval != CAspSchedule::EIntervalManual)
			{
			if (iSchedule->IsValidPeakScheduleL() != 0)
				{
				//postpone peak schedule until next peak start
				iSchedule->UpdatePeakScheduleL();
				CleanupStack::PopAndDestroy(task);
    			CleanupStack::PopAndDestroy( &instream ); 
				CleanupStack::PopAndDestroy( store ); 
				doCleanUp = EFalse;
				return;
				}
			else
				{
				//schedule next sync
				if(!iSchedule->IsValidNextPeakScheduleL())
					{
					iSchedule->UpdatePeakScheduleL();
					}
				else 
					{
					CAspSyncSchedule* syncSchedule = CAspSyncSchedule::NewLC();
					syncSchedule->EditPeakScheduleL(iSchedule);
					CleanupStack::PopAndDestroy(syncSchedule);
					}
				}
			}
		else
			{
			//schedule next sync
			CAspSyncSchedule* syncSchedule = CAspSyncSchedule::NewLC();
			syncSchedule->EditPeakScheduleL(iSchedule);
			CleanupStack::PopAndDestroy(syncSchedule);
			}
		}
			
	if (ptr.Compare(KOffPeakTaskData) == 0)
		{
		FLOG( _L("Off-Peak Sync Scheduled") );
		TInt interval = iSchedule->SyncOffPeakSchedule();
		if (iSchedule->IntervalType(interval) == EHourly)
			{
			if (iSchedule->IsValidOffPeakScheduleL() != 0)
				{
				//postpone off-schedule until next off-peak start
				iSchedule->UpdateOffPeakScheduleL();
				CleanupStack::PopAndDestroy(task);
				CleanupStack::PopAndDestroy( &instream ); 
				CleanupStack::PopAndDestroy( store ); 
				doCleanUp = EFalse;
				return;
				}
			else
				{
				//schedule next sync
				if(!iSchedule->IsValidNextOffPeakScheduleL())
					{
					iSchedule->UpdateOffPeakScheduleL();
					}
				else
					{
					CAspSyncSchedule* syncSchedule = CAspSyncSchedule::NewLC();
					syncSchedule->EditOffPeakScheduleL(iSchedule);
					CleanupStack::PopAndDestroy(syncSchedule);
					}
				}
		
			}
		}

	if(doCleanUp)	
		{
			CleanupStack::PopAndDestroy(task);
			CleanupStack::PopAndDestroy( &instream ); 
			CleanupStack::PopAndDestroy( store ); 
			doCleanUp = EFalse;
		}
	
	iSchedule->SetError(KErrNone);
	iSchedule->SaveL();
	
	TRAPD (err, DoSynchronizeL());
	User::LeaveIfError(err);

}

// -----------------------------------------------------------------------------
// CAspAutoSyncHandler::DoSynchronizeL
// 
// -----------------------------------------------------------------------------
//
void CAspAutoSyncHandler::DoSynchronizeL()
{
	
	OpenSyncSessionL();
	
	if (CAspProfile::OtherSyncRunning(&iSyncSession))
		{
		FLOG( _L("### other sync running ###") );
		TInt currentJobId = CAspProfile::CurrentJob(&iSyncSession);
		if (currentJobId != KErrNotFound)
			{
			iSyncJob.OpenL(iSyncSession, currentJobId);
			if (iSchedule->ProfileId() != iSyncJob.Profile())
				{
				FLOG( _L("CAspAutoSyncHandler::SynchronizeL Set Error") );
				iSchedule->SetError(EOtherSyncRunning);
				iSchedule->SaveL();
				}
			iSyncJob.Close();
			}
		FLOG( _L("CAspAutoSyncHandler::SynchronizeL END") );
		return;
		}
	
    TInt profileId = iSchedule->AutoSyncProfileId();
	iSchedule->UpdateProfileSettingsL();
	iSchedule->SaveL();

	
#ifdef _DEBUG
	iSchedule->LogScheduleL();
#endif
	
	if (!iSchedule->CanSynchronizeL())
		{
		FLOG( _L("### incorrect auto sync settings ###") );
		FLOG( _L("CAspAutoSyncHandler::SynchronizeL END") );
	    //iSchedule->SetError(EIncorrectSchedule);
		//iSchedule->SaveL();
		return;
		}

#if 0 //roaming check not supported		
	if (!iSchedule->RoamingAllowed())
		{
		if (IsRoaming())
			{
		    FLOG( _L("### cannot sync when roaming ###") );
		    FLOG( _L("CAspAutoSyncHandler::SynchronizeL END") );
		    return;
			}
		}
#endif

	
		
	if (CheckMandatoryDataL(profileId) != EMandatoryOk)
		{
		FLOG( _L("### incorrect sync profile settings ###") );
		FLOG( _L("CAspAutoSyncHandler::SynchronizeL END") );
		//iSchedule->SetError(EIncorrectProfile);
		//iSchedule->SaveL();
		return;
		}


    iSyncSession.RequestEventL(*this);     // for MSyncMLEventObserver events
    iSyncSession.RequestProgressL(*this);  // for MSyncMLProgressObserver events
    

    iSyncJob.CreateL(iSyncSession, profileId);  // start sync
    
    iSyncRunning = ETrue;
    
    iWait->Start();  // stop here until sync has completed
    
   	iSyncRunning = EFalse;

	iSchedule->UpdateServerIdL();
   	
   	if (iSyncError != KErrNone)
		{
		iSchedule->SetError(iSyncError);
		iSchedule->SaveL();
     	FLOG(_L("### auto sync failed (%d) ###"), iSyncError);
		}
	
    FLOG( _L("CAspAutoSyncHandler::SynchronizeL END") );

}


// -----------------------------------------------------------------------------
// CAspAutoSyncHandler::IsRoamingL
// 
// -----------------------------------------------------------------------------
//
TBool CAspAutoSyncHandler::IsRoamingL()
	{
	RConnectionMonitor conMon;
	
	conMon.ConnectL();

    TRequestStatus status;
    TInt registrationStatus(0);

    //check network status
    conMon.GetIntAttribute(EBearerIdGSM, 0, KNetworkRegistration, 
                            registrationStatus, status);

    User::WaitForRequest(status);
    conMon.Close();
    
    if (registrationStatus == ENetworkRegistrationRoaming)
    	{
    	return ETrue;	
    	}
    	
    return EFalse;
    }


// -----------------------------------------------------------------------------
// CAspAutoSyncHandler::IsRoaming
// 
// -----------------------------------------------------------------------------
//
TBool CAspAutoSyncHandler::IsRoaming()
	{
	TBool ret = EFalse;
	TRAPD(err, ret = IsRoamingL());
	
	if (err != KErrNone)
		{
		ret = EFalse; // default is not roaming	
		}
		
	return ret;
    }



/*******************************************************************************
 * class CAspSyncSchedule
 *******************************************************************************/


// -----------------------------------------------------------------------------
// CAspSyncSchedule::NewL
//
// -----------------------------------------------------------------------------
//
CAspSyncSchedule* CAspSyncSchedule::NewL()
    {
    CAspSyncSchedule* self = new (ELeave) CAspSyncSchedule();
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);

    return(self);
    }


// -----------------------------------------------------------------------------
// CAspSyncSchedule::NewLC
//
// -----------------------------------------------------------------------------
//
CAspSyncSchedule* CAspSyncSchedule::NewLC()
    {
    CAspSyncSchedule* self = new (ELeave) CAspSyncSchedule();
    CleanupStack::PushL(self);
    self->ConstructL();

    return(self);
    }


// -----------------------------------------------------------------------------
// CAspSyncSchedule::CAspSyncSchedule
// 
// -----------------------------------------------------------------------------
//
CAspSyncSchedule::CAspSyncSchedule()
	{
    }


// -----------------------------------------------------------------------------
// CAspSyncSchedule::ConstructL
//
// -----------------------------------------------------------------------------
//
void CAspSyncSchedule::ConstructL()
    {
        
    _LIT(KScheduleHandler, "z:\\sys\\bin\\aspschedulehandler.exe");
    const TInt KDefaultSchedulePriority = 1;
    
    TInt err = iScheduler.Connect();
	User::LeaveIfError(err);
	
    TFileName name(KScheduleHandler);
    err = iScheduler.Register(name, KDefaultSchedulePriority);
    User::LeaveIfError(err);
    
   	ipeakEntryList = new (ELeave) CArrayFixFlat<TScheduleEntryInfo2>(1);
	ioffPeakEntryList = new (ELeave) CArrayFixFlat<TScheduleEntryInfo2>(1);
   	ipeakTaskList  = new (ELeave) CArrayFixFlat<TTaskInfo>(1);
	ioffPeakTaskList  = new (ELeave) CArrayFixFlat<TTaskInfo>(1);
    } 


// ----------------------------------------------------------------------------
// Destructor
//
// ----------------------------------------------------------------------------
//
CAspSyncSchedule::~CAspSyncSchedule()
    {
    delete ipeakEntryList;
	delete ioffPeakEntryList;
    delete ipeakTaskList;
    delete ioffPeakTaskList;
	
    iScheduler.Close();
    }

// ----------------------------------------------------------------------------
// CAspSyncSchedule::DoDeleteScheduleL
// - Delete schedules if exist
// ----------------------------------------------------------------------------
//
void CAspSyncSchedule::DoDeleteScheduleL(CAspSchedule* aAspSchedule)
	{
	ipeakEntryList->Reset();
	ioffPeakEntryList->Reset();
	ipeakTaskList->Reset();
    ioffPeakTaskList->Reset();
	
    TScheduleState2 state;
    TTsTime time;
    
    TInt peakHandle = aAspSchedule->PeakScheduleHandle();
	TInt offPeakHandle = aAspSchedule->OffPeakScheduleHandle();
	
    if (peakHandle == KErrNotFound && offPeakHandle == KErrNotFound)
    	{
    	return; // nothing to delete
    	}
    if (peakHandle != KErrNotFound)
    	{
    	
    	TInt err = iScheduler.GetScheduleL(peakHandle, state,
                                  *ipeakEntryList, *ipeakTaskList, time);

    	TInt count = ipeakTaskList->Count();

		for (TInt i=0; i<count; i++)
    		{
    		TTaskInfo& task = (*ipeakTaskList)[i];
    		err = iScheduler.DeleteTask(task.iTaskId);
    		User::LeaveIfError(err);
    		}
    	
    	err = iScheduler.DeleteSchedule(peakHandle);
		aAspSchedule->SetPeakScheduleHandle(KErrNotFound);
		aAspSchedule->SetDailySyncEnabled(EFalse);
		aAspSchedule->SetPeakSyncEnabled(EFalse);
		
    	User::LeaveIfError(err);
    	}

	if (offPeakHandle != KErrNotFound)
    	{
    	
    	TInt err = iScheduler.GetScheduleL(offPeakHandle, state,
                                  *ioffPeakEntryList, *ioffPeakTaskList, time);

    	TInt count = ioffPeakTaskList->Count();

		for (TInt i=0; i<count; i++)
    		{
    		TTaskInfo& task = (*ioffPeakTaskList)[i];
    		err = iScheduler.DeleteTask(task.iTaskId);
    		User::LeaveIfError(err);
    		}
    	
    	err = iScheduler.DeleteSchedule(offPeakHandle);
		aAspSchedule->SetOffPeakScheduleHandle(KErrNotFound);
		aAspSchedule->SetOffPeakSyncEnabled(EFalse);
    	User::LeaveIfError(err);
    	}
	}


// ----------------------------------------------------------------------------
// CAspSyncSchedule::DeleteScheduleL
//
// ----------------------------------------------------------------------------
//
void CAspSyncSchedule::DeleteScheduleL(CAspSchedule* aAspSchedule)
	{

	TRAPD(err, DoDeleteScheduleL(aAspSchedule));
	
	if (err == KErrNone)
		{
		FLOG( _L("CAspSyncSchedule::DeleteScheduleL ok"));
		}
	else
		{
		FLOG( _L("CAspSyncSchedule::DeleteSchedule failed ,err: %d"), err );
		}
		
	if (err != KErrNone && err != KErrNotFound)
		{
		User::Leave(err);
		}
	}


// ----------------------------------------------------------------------------
// CAspSyncSchedule::CreatePeakScheduleL
//
// ----------------------------------------------------------------------------
//
void CAspSyncSchedule::CreatePeakScheduleL(CAspSchedule* aAspSchedule)
	{
	
	const TInt KRepeatForever = -1;
	const TInt KTaskPriority = 2;
		
	CAspSchedule* schedule = aAspSchedule;
	schedule->SetPeakScheduleHandle(KErrNotFound);
	
	TTsTime startTime;
	TIntervalType intervalType;
	TInt interval;
	TTime now;
	now.HomeTime();
	TInt peakschedule;
    TInt peakStatus = schedule->IsValidPeakScheduleL();

	TDateTime startday;
	TInt syncFrequency = schedule->SyncFrequency();

	//Daily schedule can be done using peak schedule
	if (IntervalType(syncFrequency) == EDaily)
		{
		   	startday = now.DateTime();
			TTime start = schedule->AutoSyncScheduleTime();
			TDateTime date = start.DateTime();
			TDateTime time(startday.Year(), startday.Month(), startday.Day(), date.Hour(), date.Minute(), 0, 0);
			TTime syncTime(time);
			startTime.SetLocalTime(syncTime);
		}
	else
		{
		TInt dayCount = 0;
	    TInt dayIndex = 0;
        schedule->SelectedDayInfo(dayCount, dayIndex); 
		if (dayCount == 0)
			{
			//no need to schedule if peak days are not selected
			schedule->SetPeakSyncEnabled(EFalse);
			return;
			}

		if(schedule->WeekdayEnabled(now.DayNoInWeek()))
			{
			startday = now.DateTime();
			//If the current time is peak, start now
			if (peakStatus == 0)
				{
				startTime.SetLocalTime(now);
				}
			//current time is before peak
			else if (peakStatus > 0)
				{
				TTime start = schedule->StartPeakTime();
				TDateTime date = start.DateTime();
				TDateTime time(startday.Year(), startday.Month(), startday.Day(), date.Hour(), date.Minute(), 0, 0);
				TTime syncTime(time);
				startTime.SetLocalTime(syncTime);
				}
			//current time is after peak.
			else if (peakStatus < 0)
				{
				TTime start = schedule->StartPeakTime();
				TDateTime date = start.DateTime();
				now = now + (TTimeIntervalDays)1;
				while(!schedule->WeekdayEnabled(now.DayNoInWeek()))
		 			{
					now = now + (TTimeIntervalDays)1;
		 			}
				startday = now.DateTime();
				TDateTime time(startday.Year(), startday.Month(), startday.Day(), date.Hour(), date.Minute(), 0, 0);
				TTime syncTime(time);
				startTime.SetLocalTime(syncTime);
				}
			}
			
		else
			{
			//find the next day selected for sync
		 	while(!schedule->WeekdayEnabled(now.DayNoInWeek()))
		 		{
				now = now + (TTimeIntervalDays)1;
		 		}
		 
		 	startday = now.DateTime();
		 	TTime start = schedule->StartPeakTime();
			TDateTime date = start.DateTime();
			TDateTime time(startday.Year(), startday.Month(), startday.Day(), date.Hour(), date.Minute(), 0, 0);
			TTime syncTime(time);
			startTime.SetLocalTime(syncTime);
		 	}

		}
	
	TInt validityPeriod = KMaxTInt;
	if (schedule->DailySyncEnabled())
		{
		peakschedule = schedule->SyncFrequency();
		}
	else
		{
	 	peakschedule = schedule->SyncPeakSchedule();
		}
    intervalType = IntervalType(peakschedule);
	interval = Interval(peakschedule);

	TScheduleEntryInfo2 peakEntry(startTime, intervalType, interval, (TTimeIntervalMinutes)validityPeriod);
	
	
	#ifdef _DEBUG
	TBuf<64> buf;
	LogTsTime(startTime, buf);
	FLOG(_L("Auto sync start time: %S"), &buf);
	#endif
    
	
	ipeakEntryList->Reset();
	ipeakEntryList->AppendL(peakEntry);
	
	TSchedulerItemRef ref;
	TInt err = iScheduler.CreatePersistentSchedule(ref, *ipeakEntryList);
	User::LeaveIfError(err);
	
	//keep the scheduler disabled until settings are saved
	iScheduler.DisableSchedule(ref.iHandle);
	
	schedule->SetPeakScheduleHandle(ref.iHandle);
	
	HBufC* hBuf = HBufC::NewMaxLC(KBufSize);
	hBuf->Des() = KPeakTaskData;
	
	TTaskInfo taskInfo;
	taskInfo.iName = KPeakTaskData;
	taskInfo.iRepeat = KRepeatForever;
	taskInfo.iPriority = KTaskPriority;

    err = iScheduler.ScheduleTask(taskInfo, *hBuf, ref.iHandle);
    User::LeaveIfError(err);
	
	CleanupStack::PopAndDestroy(hBuf);
	}


// ----------------------------------------------------------------------------
// CAspSyncSchedule::CreateOffPeakScheduleL
// 
// ----------------------------------------------------------------------------
//
void CAspSyncSchedule::CreateOffPeakScheduleL(CAspSchedule* aAspSchedule)
	{
	
	const TInt KRepeatForever = -1;
	const TInt KTaskPriority = 1;
		
	CAspSchedule* schedule = aAspSchedule;
	schedule->SetOffPeakScheduleHandle(KErrNotFound);
	
	TTime opStart= schedule->EndPeakTime()+ (TTimeIntervalMinutes)1;
	TDateTime opStartDate = opStart.DateTime();
	TTime pStart = schedule->StartPeakTime();
	TDateTime pStartDate = pStart.DateTime();
	
	//Dont create off-peak schedule if the duration is zero
	if (opStartDate.Hour() == pStartDate.Hour()
						&& opStartDate.Minute() == pStartDate.Minute())
		{
		schedule->SetOffPeakSyncEnabled(EFalse);
		return;
		}
	
	TTsTime startTime;
	TIntervalType intervalType;
	TInt interval;

	TTime now;
	now.HomeTime();
	
	TInt offpeakschedule = schedule->SyncOffPeakSchedule();
	intervalType = IntervalType(offpeakschedule);

	TInt offPeakStatus = schedule->IsValidOffPeakScheduleL();
	TDateTime startday;
	
	startday = now.DateTime();
	//If the current time is off peak, start now
	if (offPeakStatus == 0)
		{
		startTime.SetLocalTime(now);
		}
	//current time is peak
	else if (offPeakStatus > 0)
		{
		TTime start= schedule->EndPeakTime()+ (TTimeIntervalMinutes)1;
		TDateTime date = start.DateTime();
		TDateTime time(startday.Year(), startday.Month(), startday.Day(), date.Hour(), date.Minute(), 0, 0);
		TTime syncTime(time);
		startTime.SetLocalTime(syncTime);
		}
	
	TInt validityPeriod = KMaxTInt;
	
	interval = Interval(offpeakschedule);
	
	TScheduleEntryInfo2 offPeakEntry(startTime, intervalType, interval, (TTimeIntervalMinutes)validityPeriod);
	
	#ifdef _DEBUG
	TBuf<64> buf;
	LogTsTime(startTime, buf);
	FLOG(_L("Auto sync start time: %S"), &buf);
	#endif
    
	ioffPeakEntryList->Reset();
	ioffPeakEntryList->AppendL(offPeakEntry);
	
	TSchedulerItemRef ref;
	TInt err = iScheduler.CreatePersistentSchedule(ref, *ioffPeakEntryList);
	User::LeaveIfError(err);

	//keep the scheduler disabled until settings are saved
	iScheduler.DisableSchedule(ref.iHandle);
	
	schedule->SetOffPeakScheduleHandle(ref.iHandle);
	
	HBufC* hBuf = HBufC::NewMaxLC(KBufSize);
	hBuf->Des() = KOffPeakTaskData;
	
	TTaskInfo taskInfo;
	taskInfo.iName = KOffPeakTaskData;
	taskInfo.iRepeat = KRepeatForever;
	taskInfo.iPriority = KTaskPriority;

    err = iScheduler.ScheduleTask(taskInfo, *hBuf, ref.iHandle);
    User::LeaveIfError(err);
	
	CleanupStack::PopAndDestroy(hBuf);
	}

// -----------------------------------------------------------------------------
// CAspSyncSchedule::EnableSchedule()
// -Enable schedules after settings are saved.
// -----------------------------------------------------------------------------
//
void CAspSyncSchedule::EnableSchedule(CAspSchedule* aAspSchedule)
	{
	TInt peakHandle = aAspSchedule->PeakScheduleHandle();
	TInt offPeakHandle = aAspSchedule->OffPeakScheduleHandle();

	if (peakHandle != KErrNotFound)
		{
		iScheduler.EnableSchedule(peakHandle);
		}
	if (offPeakHandle != KErrNotFound)
		{
		iScheduler.EnableSchedule(offPeakHandle);
		}

	}

// -----------------------------------------------------------------------------
// CAspSyncSchedule::EditPeakScheduleL
// -Edit peak schedule entry
// -----------------------------------------------------------------------------
//

void CAspSyncSchedule::EditPeakScheduleL(CAspSchedule* aAspSchedule)
	{

	ipeakEntryList->Reset();
	ipeakTaskList->Reset();

	TTsTime tsTime;
	TScheduleState2 state;
	TInt peakHandle = aAspSchedule->PeakScheduleHandle();
		
	if (peakHandle == KErrNotFound)
		{
		User::Leave(KErrNotFound);	
		}
	
	TInt err = iScheduler.GetScheduleL(peakHandle, state, *ipeakEntryList, *ipeakTaskList, tsTime);
	
	User::LeaveIfError(err);
	
	TInt count = ipeakEntryList->Count();
	__ASSERT_DEBUG(count == 1, TUtil::Panic(KErrArgument));

	TInt peakInterval;
	if (aAspSchedule->DailySyncEnabled())
		{
		peakInterval = aAspSchedule->SyncFrequency();
		}
	else
		{
	 	peakInterval = aAspSchedule->SyncPeakSchedule();
		}
	TInt interval = Interval(peakInterval);
	TIntervalType intervalType = IntervalType(peakInterval);
	TTime now;
	now.HomeTime();
	TTsTime startTime ;
	if (intervalType == EHourly)
		{
		if (peakInterval == CAspSchedule::EInterval15Mins
				       		|| peakInterval == CAspSchedule::EInterval30Mins)
			{
			now = now + (TTimeIntervalMinutes)interval ;
			}
		else
			{
			now = now + (TTimeIntervalHours)interval ;
			}
		startTime.SetLocalTime(now);
		}
	else
		{
		now = now + (TTimeIntervalDays)interval;
		TDateTime startday = now.DateTime();
		TTime start = aAspSchedule->AutoSyncScheduleTime();
		TDateTime date = start.DateTime();
		TDateTime time(startday.Year(), startday.Month(), startday.Day(), date.Hour(), date.Minute(), 0, 0);
		TTime syncTime(time);
		startTime.SetLocalTime(syncTime);
		}
	
	TScheduleEntryInfo2& info = (*ipeakEntryList)[0];

	info.SetStartTime(startTime);
	info.SetInterval(interval);
	info.SetIntervalType(intervalType);
	
	err = iScheduler.EditSchedule(peakHandle, *ipeakEntryList);
	User::LeaveIfError(err);
	
	}


// -----------------------------------------------------------------------------
// CAspSyncSchedule::EditOffPeakScheduleL
// -Edit off-peak schedule entry
// -----------------------------------------------------------------------------
//
void CAspSyncSchedule::EditOffPeakScheduleL(CAspSchedule* aAspSchedule)
	{

	ioffPeakEntryList->Reset();
	ioffPeakTaskList->Reset();

	TTsTime tsTime;
	TScheduleState2 state;
	TInt offPeakHandle = aAspSchedule->OffPeakScheduleHandle();
		
	if (offPeakHandle == KErrNotFound)
		{
		User::Leave(KErrNotFound);	
		}
	
	TInt err = iScheduler.GetScheduleL(offPeakHandle, state, *ioffPeakEntryList, *ioffPeakTaskList, tsTime);
	
	User::LeaveIfError(err);
	
	TInt count = ioffPeakEntryList->Count();
	__ASSERT_DEBUG(count == 1, TUtil::Panic(KErrArgument));
	
	TInt offPeakInterval = aAspSchedule->SyncOffPeakSchedule();
	TInt interval = Interval(offPeakInterval);
	TIntervalType intervalType = IntervalType(offPeakInterval);
	TTime now;
	now.HomeTime();
	TTsTime startTime ;
	
	if (offPeakInterval == CAspSchedule::EInterval15Mins
				       		|| offPeakInterval == CAspSchedule::EInterval30Mins)
		{
		now = now + (TTimeIntervalMinutes)interval ;
		}
	else
		{
		now = now + (TTimeIntervalHours)interval ;
		}
	startTime.SetLocalTime(now);
	
	
	TScheduleEntryInfo2& info = (*ioffPeakEntryList)[0];

	info.SetStartTime(startTime);
	info.SetInterval(interval);
	info.SetIntervalType(intervalType);
	
	err = iScheduler.EditSchedule(offPeakHandle, *ioffPeakEntryList);
	User::LeaveIfError(err);
	
	}

// -----------------------------------------------------------------------------
// CAspSyncSchedule::UpdatePeakScheduleL
// -Postpone peak schedule till next peak start
// -----------------------------------------------------------------------------
//
void CAspSyncSchedule::UpdatePeakScheduleL(CAspSchedule* aAspSchedule)
	{

	ipeakEntryList->Reset();
	ipeakTaskList->Reset();

	TTime startPeak = aAspSchedule->StartPeakTime();
		
	TTime now;
	now.HomeTime();

		
	TDateTime startDate = startPeak.DateTime();
	
	TDateTime day = now.DateTime();
	if (day.Hour() > KStartPeakHour)
		{
		now = now + (TTimeIntervalDays)1;
		}
	TDateTime tomorrow = now.DateTime();
	
    TDateTime time(tomorrow.Year(), tomorrow.Month(), tomorrow.Day(), startDate.Hour(), startDate.Minute(), 0, 0);
	TTime nextPeakStart(time);
	
	while(!aAspSchedule->WeekdayEnabled(nextPeakStart.DayNoInWeek()))
		{
		nextPeakStart = nextPeakStart + (TTimeIntervalDays)1;
		}
	TTsTime tsTime;
	TScheduleState2 state;
	TInt peakHandle = aAspSchedule->PeakScheduleHandle();
		
	if (peakHandle == KErrNotFound)
		{
		User::Leave(KErrNotFound);	
		}
	
	TInt err = iScheduler.GetScheduleL(peakHandle, state, *ipeakEntryList, *ipeakTaskList, tsTime);
	
	User::LeaveIfError(err);
	
	TInt count = ipeakEntryList->Count();
	__ASSERT_DEBUG(count == 1, TUtil::Panic(KErrArgument));

	
	TTsTime startTime ;
	startTime.SetLocalTime(nextPeakStart);
	TIntervalType intervalType = IntervalType(aAspSchedule->SyncPeakSchedule());
	TInt interval = Interval(aAspSchedule->SyncPeakSchedule());	
	
	TScheduleEntryInfo2& info = (*ipeakEntryList)[0];

	info.SetStartTime(startTime);
	info.SetInterval(interval);
	info.SetIntervalType(intervalType);
	
	err = iScheduler.EditSchedule(peakHandle, *ipeakEntryList);
	User::LeaveIfError(err);
	
	
	}

// -----------------------------------------------------------------------------
// CAspSyncSchedule::UpdateOffPeakScheduleL
// -Postpone off-peak schedule till next off-peak start
// -----------------------------------------------------------------------------
//

void CAspSyncSchedule::UpdateOffPeakScheduleL(CAspSchedule* aAspSchedule)
	{

	ioffPeakEntryList->Reset();
	ioffPeakTaskList->Reset();

	TTime startoffPeak = aAspSchedule->EndPeakTime() + (TTimeIntervalMinutes)1;
		
	TTime now;
	now.HomeTime();

	TInt interval = aAspSchedule->SyncOffPeakSchedule();
	TIntervalType intervalType = IntervalType(interval);
	TInt intervalVal = Interval(interval);

	if (interval == CAspSchedule::EInterval15Mins
				|| interval == CAspSchedule::EInterval30Mins)

		{
		now = now + (TTimeIntervalMinutes)intervalVal;
		}
	else if (intervalType == EHourly)
		{
		now = now + (TTimeIntervalHours)intervalVal;
		}
	
	TDateTime startday = startoffPeak.DateTime();
	TDateTime today = now.DateTime();
	
    TDateTime time(today.Year(), today.Month(), today.Day(), startday.Hour(), startday.Minute(), 0, 0);
	
	TTime nextOffPeakStart(time);
	TTsTime tsTime;
	TScheduleState2 state;
	TInt offpeakHandle = aAspSchedule->OffPeakScheduleHandle();
		
	if (offpeakHandle == KErrNotFound)
		{
		User::Leave(KErrNotFound);	
		}
	
	TInt err = iScheduler.GetScheduleL(offpeakHandle, state, *ioffPeakEntryList, *ioffPeakTaskList, tsTime);
	
	User::LeaveIfError(err);
	
	TInt count = ioffPeakEntryList->Count();
	__ASSERT_DEBUG(count == 1, TUtil::Panic(KErrArgument));

	
	TTsTime startTime ;
	startTime.SetLocalTime(nextOffPeakStart);
	TScheduleEntryInfo2& info = (*ioffPeakEntryList)[0];

	info.SetStartTime(startTime);
	info.SetInterval(interval);
	info.SetIntervalType(intervalType);
	
	err = iScheduler.EditSchedule(offpeakHandle, *ioffPeakEntryList);
	User::LeaveIfError(err);
	
	
	}
// ----------------------------------------------------------------------------
// CAspSyncSchedule::IntervalType
// Return  interval type
// ----------------------------------------------------------------------------
//
TIntervalType CAspSyncSchedule::IntervalType(TInt aInterval)
	{
	if (aInterval == CAspSchedule::EInterval24hours
			  || aInterval == CAspSchedule::EInterval2days
			  || aInterval == CAspSchedule::EInterval4days
			  || aInterval == CAspSchedule::EInterval7days
			  || aInterval == CAspSchedule::EInterval14days
			  || aInterval == CAspSchedule::EInterval30days)
		{
		return EDaily;
		}
		
	return EHourly;
	}
	
	
// ----------------------------------------------------------------------------
// CAspSyncSchedule::Interval
// Return  interval value
// ----------------------------------------------------------------------------
//
TInt CAspSyncSchedule::Interval(TInt aInterval)
	{
	const TInt KIntervalManual  = 0;
	const TInt KInterval15Mins  = 15;
	const TInt KInterval30Mins  = 30;
	const TInt KInterval1hour   = 1;
	const TInt KInterval2hours  = 2;
	const TInt KInterval4hours  = 4;
	const TInt KInterval8hours  = 8;
	const TInt KInterval12hours = 12;		
	const TInt KInterval24hours = 1;
	const TInt KInterval2days   = 2;
	const TInt KInterval4days   = 4;
	const TInt KInterval7ays    = 7;
	const TInt KInterval14days  = 14;
	const TInt KInterval30days  = 30;
	
	    TInt ret = KIntervalManual;
    
    switch (aInterval)
    	{
    	case CAspSchedule::EIntervalManual:
    	    ret = KIntervalManual;
    	    break;
    	case CAspSchedule::EInterval15Mins:
    	    ret = KInterval15Mins;
    	    break;
    	case CAspSchedule::EInterval30Mins:
    	    ret = KInterval30Mins;
    	    break;
    	case CAspSchedule::EInterval1hour:
    	    ret = KInterval1hour;
    	    break;
    	case CAspSchedule::EInterval2hours:
    	    ret = KInterval2hours;
    	    break;
    	case CAspSchedule::EInterval4hours:
    	    ret = KInterval4hours;
    	    break;
		case CAspSchedule::EInterval8hours:
    	    ret = KInterval8hours;
    	    break;
		case CAspSchedule::EInterval12hours:
    	    ret = KInterval12hours;
    	    break;
		case CAspSchedule::EInterval24hours:
    	    ret = KInterval24hours;
    	    break;
		case CAspSchedule::EInterval2days:
    	    ret = KInterval2days;
    	    break;
		case CAspSchedule::EInterval4days:
    	    ret = KInterval4days;
    	    break;
		case CAspSchedule::EInterval7days:
    	    ret = KInterval7ays;
    	    break;
		case CAspSchedule::EInterval14days:
    	    ret = KInterval14days;
    	    break;
		case CAspSchedule::EInterval30days:	
    	    ret = KInterval30days;
    	    break;

			
        default:
            break;
    	}
    	
    return ret;	
	}


// ----------------------------------------------------------------------------
// CAspSyncSchedule::StartTime
//
// ----------------------------------------------------------------------------
//
TTsTime CAspSyncSchedule::StartTime(TInt aStartHour, TInt aInterval)
	{

	if (IntervalType(aInterval) == EDaily)
		{
	    TTime time = CAspSchedule::LocalStartTime(aStartHour);
	    TTsTime tsTime(time, EFalse);
        return tsTime;
		}
	
	TTime time = CAspSchedule::UniversalStartTime(aStartHour);
    TTsTime tsTime(time, ETrue);
    return tsTime;
    }


// -----------------------------------------------------------------------------
// CAspSyncSchedule::GetPeakStartTimeL
//
// -----------------------------------------------------------------------------
//
void CAspSyncSchedule::GetPeakStartTimeL(CAspSchedule* aAspSchedule, TTime& aTime, TBool aHomeTime)
	{

	ipeakEntryList->Reset();
	ipeakTaskList->Reset();
	
	TTsTime tsTime;
	TScheduleState2 state;
	TInt peakHandle = aAspSchedule->PeakScheduleHandle();
	
	TInt err = iScheduler.GetScheduleL(peakHandle, state, *ipeakEntryList, *ipeakTaskList, tsTime);
	User::LeaveIfError(err);
	if (aHomeTime)
		{
		aTime = tsTime.GetLocalTime();	
		}
	else
		{
	    aTime = tsTime.GetUtcTime();		
		}

	}


// -----------------------------------------------------------------------------
// CAspSyncSchedule::GetOffPeakStartTimeL
//
// -----------------------------------------------------------------------------
//
void CAspSyncSchedule::GetOffPeakStartTimeL(CAspSchedule* aAspSchedule, TTime& aTime, TBool aHomeTime)
	{

	ioffPeakEntryList->Reset();
	ioffPeakTaskList->Reset();
	
	TTsTime tsTime;
	TScheduleState2 state;
	TInt offPeakHandle = aAspSchedule->OffPeakScheduleHandle();
	
	TInt err = iScheduler.GetScheduleL(offPeakHandle, state, *ioffPeakEntryList, *ioffPeakTaskList, tsTime);
	User::LeaveIfError(err);
	if (aHomeTime)
		{
		aTime = tsTime.GetLocalTime();	
		}
	else
		{
	    aTime = tsTime.GetUtcTime();		
		}

	}


#ifdef _DEBUG

// ----------------------------------------------------------------------------
// CAspSyncSchedule::LogScheduleL
//
// ----------------------------------------------------------------------------
//
void CAspSyncSchedule::LogScheduleL(CAspSchedule* aAspSchedule)
	{

	FLOG( _L("---- scheduler settings ----") );

	TTsTime tsTime;
	TTime time;
	TScheduleState2 state;
	TInt peakhandle = aAspSchedule->PeakScheduleHandle();
	if (peakhandle != KErrNotFound )
		{
		FLOG( _L("---- Peak Time Sync Enabled -> Settings") );
		ipeakEntryList->Reset();
		ipeakTaskList->Reset();
		TInt err = iScheduler.GetScheduleL(peakhandle, state, *ipeakEntryList, *ipeakTaskList, tsTime);
		User::LeaveIfError(err);
	
		TScheduleEntryInfo2 info = (*ipeakEntryList)[0];
	
		TBuf<KBufSize> buf;
	
		FLOG(_L("peak schedule handle: %d"), peakhandle);
	
		LogInterval(info, buf);
		FLOG(_L("peak interval: %S"), &buf);

		LogIntervalType(info, buf);
		FLOG(_L("peak interval type: %S"), &buf);
	
		LogTsTime(tsTime, buf);
		FLOG(_L("peak sync start time: %S"), &buf);
			
		TTaskInfo taskInfo = (*ipeakTaskList)[0];
		LogTaskInfo(taskInfo, buf);
		FLOG(_L("peak task info: %S"), &buf);

		}
	TInt offPeakhandle = aAspSchedule->OffPeakScheduleHandle();
	if (offPeakhandle != KErrNotFound )
		{
		FLOG( _L("---- Off-Peak Time Sync Enabled -> Settings") );
		ioffPeakEntryList->Reset();
	    ioffPeakTaskList->Reset();
		TInt err = iScheduler.GetScheduleL(offPeakhandle, state, *ioffPeakEntryList, *ioffPeakTaskList, tsTime);
		User::LeaveIfError(err);
	
		TScheduleEntryInfo2 info = (*ioffPeakEntryList)[0];
	
		TBuf<KBufSize> buf;
	
		FLOG(_L("off-peak schedule handle: %d"), peakhandle);
	
		LogInterval(info, buf);
		FLOG(_L("off-peak interval: %S"), &buf);

		LogIntervalType(info, buf);
		FLOG(_L("off-peak interval type: %S"), &buf);
	
		LogTsTime(tsTime, buf);
		FLOG(_L("off-peak sync start time: %S"), &buf);
			
		TTaskInfo taskInfo = (*ioffPeakTaskList)[0];
		LogTaskInfo(taskInfo, buf);
		FLOG(_L("off-peak task info: %S"), &buf);

		}
		
	
		
	
	FLOG( _L("---- scheduler settings ----") );

    }


// ----------------------------------------------------------------------------
// CAspSyncSchedule::LogSchedule
//
// ----------------------------------------------------------------------------
//
TInt CAspSyncSchedule::LogSchedule(CAspSchedule* aAspSchedule)
	{
	TInt err = KErrNone;
	TRAP(err, LogScheduleL(aAspSchedule));
	
	return err;
    }


// ----------------------------------------------------------------------------
// CAspSyncSchedule::LogTaskInfo
//
// ----------------------------------------------------------------------------
//
void CAspSyncSchedule::LogTaskInfo(TTaskInfo& aInfo, TDes& aText)
	{
	aText.Format(_L("name: %S, repeat: %d, id: %d, priority: %d"), 
	             &aInfo.iName, aInfo.iRepeat, aInfo.iTaskId, aInfo.iPriority);
    }


// ----------------------------------------------------------------------------
// CAspSyncSchedule::LogIntervalType
//
// ----------------------------------------------------------------------------
//
void CAspSyncSchedule::LogIntervalType(TScheduleEntryInfo2& aInfo, TDes& aText)
	{
	aText = _L("unknown");
	
	TIntervalType type = aInfo.IntervalType();
	
	if (type == EDaily)
		{
		aText = _L("EDaily");
		}
	if (type == EHourly)
		{
		aText = _L("EHourly");
		}
    }


// ----------------------------------------------------------------------------
// CAspSyncSchedule::LogInterval
//
// ----------------------------------------------------------------------------
//
void CAspSyncSchedule::LogInterval(TScheduleEntryInfo2& aInfo, TDes& aText)
	{
	TInt num = aInfo.Interval();
	aText.Format(_L("interval: %d"), num);
    }


// ----------------------------------------------------------------------------
// CAspSyncSchedule::LogTime
//
// ----------------------------------------------------------------------------
//

void CAspSyncSchedule::LogTsTime(TTsTime aTsTime, TDes& aText)
	{
	TBuf<64> buf1; TBuf<64> buf2;

	TTime time = aTsTime.GetLocalTime();
	TRAP_IGNORE(TUtil::GetDateTimeTextL(buf1, time));
	time = aTsTime.GetUtcTime();
	TRAP_IGNORE(TUtil::GetDateTimeTextL(buf2, time));
	
	aText.Format(_L("local: %S  utc: %S"), &buf1, &buf2);
    }

#endif //  _DEBUG


