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

#include "AspSchedule.h"
#include "AspUtil.h"
#include "AspDialogUtil.h"
#include "AspDebug.h"
#include "AspProfile.h"
#include "AspLogDialog.h"
#include "AspAutoSyncHandler.h"

#include "schinfo.h" // TScheduleEntryInfo2
#include <s32mem.h>  // RDesWriteStream
#include <s32file.h> // RFileReadStream
#include <bautils.h> // BaflUtils
#include <centralrepository.h>  // CRepository
#include <rconnmon.h>
#include <SyncMLClientDS.h>



/*******************************************************************************
 * class CAspSchedule
 *******************************************************************************/


// -----------------------------------------------------------------------------
// CAspSchedule::NewL
//
// -----------------------------------------------------------------------------
//
CAspSchedule* CAspSchedule::NewL()
    {
    CAspSchedule* self = new (ELeave) CAspSchedule();
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);

    return(self);
    }


// -----------------------------------------------------------------------------
// CAspSchedule::NewLC
//
// -----------------------------------------------------------------------------
//
CAspSchedule* CAspSchedule::NewLC()
    {
    CAspSchedule* self = new (ELeave) CAspSchedule();
    CleanupStack::PushL(self);
    self->ConstructL();

    return(self);
    }

// -----------------------------------------------------------------------------
// CAspSchedule::CAspSchedule
// 
// -----------------------------------------------------------------------------
//
CAspSchedule::CAspSchedule()
	{
	iSettingChanged = EFalse;
	iSyncSessionOpen = EFalse;
    }


// -----------------------------------------------------------------------------
// CAspSchedule::ConstructL
//
// -----------------------------------------------------------------------------
//
void CAspSchedule::ConstructL()
    {
    InternalizeL();
	
	if(iAutoSyncProfileId == KErrNotFound)
		{
		CreateAutoSyncProfileL();
		}
	
		
    } 


// ----------------------------------------------------------------------------
// Destructor
//
// ----------------------------------------------------------------------------
//
CAspSchedule::~CAspSchedule()
    {
    iList.Close();
	CloseSyncSession();
    }


// ----------------------------------------------------------------------------
// InitializeL
//
// ----------------------------------------------------------------------------
//
void CAspSchedule::InitializeL()
    {
    
    FLOG( _L("CAspSchedule::InitializeL START") );
    iError          = KErrNone;
    iProfileId      = KErrNotFound;
    iRoamingAllowed = EFalse;
	iDailySyncEnabled = EFalse;
	iPeakSyncEnabled = EFalse;
	iOffPeakSyncEnabled = EFalse;
	

	iContentFlags = 0xFF;
	iSyncFrequency          = EIntervalManual;
	iSyncPeakSchedule       = EIntervalManual;
    iSyncOffPeakSchedule    = EIntervalManual;

    iPeakScheduleHandle = KErrNotFound;
	iOffPeakScheduleHandle = KErrNotFound;

	iStartPeakTimeHr		 = KStartPeakHour;
	iStartPeakTimeMin		 = KStartPeakMin;
	iEndPeakTimeHr         = KEndPeakHour;
	iEndPeakTimeMin         = KEndPeakMin;

	iAutoSyncProfileId = KErrNotFound;
	iAutoSyncScheduleTimeHr = KDefaultStartHour;
	iAutoSyncScheduleTimeMin = 0;

	SetWeekdayEnabled(EMonday,    ETrue);
   	SetWeekdayEnabled(ETuesday,   ETrue);
   	SetWeekdayEnabled(EWednesday, ETrue);
   	SetWeekdayEnabled(EThursday,  ETrue);
   	SetWeekdayEnabled(EFriday,    ETrue);
   	SetWeekdayEnabled(ESaturday,  EFalse);
   	SetWeekdayEnabled(ESunday,    EFalse);

	InitializeContentsL();
	FLOG( _L("CAspSchedule::InitializeL END") );

    }

// ----------------------------------------------------------------------------
// InitializeContentsL
//
// ----------------------------------------------------------------------------
//
void CAspSchedule::InitializeContentsL()
	{
	OpenSyncSessionL();
	TAspParam param(EApplicationIdSync, &iSyncSession);
	param.iMode = CAspContentList::EInitDataProviders;
	CAspContentList* asContentList = CAspContentList::NewLC(param);		
	
	TInt providerCount = asContentList->ProviderCount();
	
	for (TInt index = 0; index < providerCount; index++)
		{
		TAspProviderItem& provider = asContentList->ProviderItem(index);
			
		if (provider.iDataProviderId == KUidNSmlAdapterEMail.iUid)
			{
			if (MailboxExistL())
				{
				SetContentEnabled(index, ETrue);
				}
			else
				{
				SetContentEnabled(index, EFalse);
				}
			}
		else
			{
			SetContentEnabled(index, ETrue);
			}
		
		}
	CleanupStack::PopAndDestroy(asContentList);


	}

// -----------------------------------------------------------------------------
// CAspSchedule::DoInternalizeL
//
// -----------------------------------------------------------------------------
//
void CAspSchedule::DoInternalizeL()
	{
	const TInt KMaxLength = 1024;
	
    HBufC8*  hBuf = HBufC8::NewLC(KMaxLength);
    TPtr8 ptr = hBuf->Des();
    
    ReadRepositoryL(ERepositoryKeyBin, ptr);
    
    RDesReadStream stream(ptr);
    stream.PushL();
    
    InternalizeL(stream);
    
    stream.Pop();
    stream.Close();
    
    CleanupStack::PopAndDestroy(hBuf);
	}


// -----------------------------------------------------------------------------
// CAspSchedule::InternalizeL
//
// -----------------------------------------------------------------------------
//
void CAspSchedule::InternalizeL()
	{
	FLOG( _L("CAspSchedule::InternalizeL START") );
	TRAPD(err, DoInternalizeL());
	
	if (err == KErrNoMemory || err == KErrLocked || err == KErrAlreadyExists)
		{
		FLOG( _L("CAspSchedule::InternalizeL Error 1:  %d"),err );
		User::Leave(err);
		}
		
	else if (err != KErrNone)
		{
		FLOG( _L("CAspSchedule::InternalizeL Error 2:  %d"),err );
		ResetRepositoryL();  // settings corrupted
	    InitializeL();
		}
	}


// -----------------------------------------------------------------------------
// CAspSchedule::InternalizeL
//
// -----------------------------------------------------------------------------
//
void CAspSchedule::InternalizeL(RReadStream& aStream)
	{
	const TInt KAutoSyncSettingCount = 20;
	
    iList.Reset();
    
    // total setting count
	TInt32 count = 0;
	
	TRAPD(err, count = aStream.ReadInt32L());
	
	if (err != KErrNone && err != KErrEof)
		{
		User::Leave(err);
		}
	
	if (count != KAutoSyncSettingCount)
		{
		InitializeL(); // settings not found - use defaults
		SaveL();
		return;
		}
    
    
    iError                 = aStream.ReadInt32L();
    iProfileId             = aStream.ReadInt32L();
    iWeekdayFlags          = aStream.ReadInt32L();
    TInt roamingAllowed    = aStream.ReadInt32L();


	
	iPeakScheduleHandle = aStream.ReadInt32L();
	iOffPeakScheduleHandle = aStream.ReadInt32L();
	iContentFlags           = aStream.ReadInt32L();
	iSyncPeakSchedule       = aStream.ReadInt32L();
    iSyncOffPeakSchedule    = aStream.ReadInt32L();
	iStartPeakTimeHr		= aStream.ReadInt32L();
	iStartPeakTimeMin		= aStream.ReadInt32L();
	iEndPeakTimeHr          = aStream.ReadInt32L();
	iEndPeakTimeMin         = aStream.ReadInt32L();
	iSyncFrequency          = aStream.ReadInt32L();
	iDailySyncEnabled 			= aStream.ReadInt32L();
	iPeakSyncEnabled        = aStream.ReadInt32L();
	iOffPeakSyncEnabled		= aStream.ReadInt32L();

	iAutoSyncProfileId		= aStream.ReadInt32L();
	
	iAutoSyncScheduleTimeHr = aStream.ReadInt32L();
	iAutoSyncScheduleTimeMin = aStream.ReadInt32L();
	
#ifdef _DEBUG
    LogSettings();
#endif

    iRoamingAllowed = EFalse;
    if (roamingAllowed)
    	{
    	iRoamingAllowed = ETrue;
    	}

	CheckEmailSelectionL();
    }   

// -----------------------------------------------------------------------------
// CAspSchedule::CheckEmailSelectionL()
//
// -----------------------------------------------------------------------------
//
void CAspSchedule::CheckEmailSelectionL()
	{
	TRAPD(err, OpenSyncSessionL());
	if (err != KErrNone)
		{
		
		/*OpenSynSessionL() may leave when system time is changed so that peak and off-peak syncs are
		scheduled simulataneously.In that case also next sync time has to be updated. So a leave here should be
		trapped and return to the time update function.	
		*/
		
		FLOG( _L("Session cannot be opened now : return") );
		return;
		}
	TAspParam param(EApplicationIdSync, &iSyncSession);
	param.iMode = CAspContentList::EInitDataProviders;
	CAspContentList* asContentList = CAspContentList::NewLC(param);		
	
	TInt providerCount = asContentList->ProviderCount();

	TInt emailIndex = asContentList->FindProviderIndex(KUidNSmlAdapterEMail.iUid);
					
	if (!MailboxExistL())
		{
		SetContentEnabled(emailIndex, EFalse);
		}
	else if (!IsAutoSyncEnabled())
		{
		SetContentEnabled(emailIndex, ETrue);
		}

	if (iProfileId != KErrNotFound
		 					&& !ProtocolL())
		{
		SetContentEnabled(emailIndex, EFalse);
		}
	
	CleanupStack::PopAndDestroy(asContentList);

	}
	


// -----------------------------------------------------------------------------
// CAspSchedule::ExternalizeL
//
// -----------------------------------------------------------------------------
//
void CAspSchedule::ExternalizeL()
	{
	const TInt KMaxLength = 1024;
	
    HBufC8*  hBuf = HBufC8::NewLC(KMaxLength);
    TPtr8 ptr = hBuf->Des();
    RDesWriteStream stream(ptr);
    stream.PushL();
    
    ExternalizeL(stream);
    
    stream.CommitL();
    stream.Pop();
    stream.Close();
    
    TRAPD (err,WriteRepositoryL(ERepositoryKeyBin, ptr));
	User::LeaveIfError(err);
    
    CleanupStack::PopAndDestroy(hBuf);
	}


// -----------------------------------------------------------------------------
// CAspSchedule::ExternalizeL
//
// -----------------------------------------------------------------------------
//
void CAspSchedule::ExternalizeL(RWriteStream& aStream)
	{
	const TInt KAutoSyncSettingCount = 20;
	
        
    TInt roamingAllowed = 0;
    if (iRoamingAllowed)
    	{
    	roamingAllowed = 1;
    	}
    
   
    aStream.WriteInt32L(KAutoSyncSettingCount);
	aStream.WriteInt32L(iError);
    aStream.WriteInt32L(iProfileId);
    aStream.WriteInt32L(iWeekdayFlags);
    aStream.WriteInt32L(roamingAllowed);

	aStream.WriteInt32L(iPeakScheduleHandle);
	aStream.WriteInt32L(iOffPeakScheduleHandle);
    aStream.WriteInt32L(iContentFlags);
	aStream.WriteInt32L(iSyncPeakSchedule);
	aStream.WriteInt32L(iSyncOffPeakSchedule);
	aStream.WriteInt32L(iStartPeakTimeHr);
	aStream.WriteInt32L(iStartPeakTimeMin);
	aStream.WriteInt32L(iEndPeakTimeHr);
   	aStream.WriteInt32L(iEndPeakTimeMin);
	aStream.WriteInt32L(iSyncFrequency);
	aStream.WriteInt32L(iDailySyncEnabled);
	aStream.WriteInt32L(iPeakSyncEnabled);
	aStream.WriteInt32L(iOffPeakSyncEnabled);
	aStream.WriteInt32L(iAutoSyncProfileId);

    aStream.WriteInt32L(iAutoSyncScheduleTimeHr);
	aStream.WriteInt32L(iAutoSyncScheduleTimeMin);

#ifdef _DEBUG
	LogSettings();
#endif

	
	}


// -----------------------------------------------------------------------------
// CAspSchedule::WriteRepositoryL
//
// -----------------------------------------------------------------------------
//
void CAspSchedule::WriteRepositoryL(TInt aKey, const TDesC8& aValue)
	{
	const TUid KRepositoryId = KCRUidNSmlDSApp; // 0x101F9A1D

    CRepository* rep = CRepository::NewLC(KRepositoryId);
    TInt err = rep->Set(aKey, aValue);
    User::LeaveIfError(err);
    
    CleanupStack::PopAndDestroy(rep);
	}


// -----------------------------------------------------------------------------
// CAspSchedule::ReadRepositoryL
//
// -----------------------------------------------------------------------------
//
void CAspSchedule::ReadRepositoryL(TInt aKey, TDes8& aValue)
	{
	const TUid KRepositoryId = KCRUidNSmlDSApp;

    CRepository* rep = CRepository::NewLC(KRepositoryId);
    TInt err = rep->Get(aKey, aValue);
    User::LeaveIfError(err);
    
    CleanupStack::PopAndDestroy(rep);
	}


// -----------------------------------------------------------------------------
// CAspSchedule::ResetRepositoryL
//
// -----------------------------------------------------------------------------
//
void CAspSchedule::ResetRepositoryL()
	{
	const TUid KRepositoryId = KCRUidNSmlDSApp;

    CRepository* rep = CRepository::NewLC(KRepositoryId);
    TInt err = rep->Reset();
    User::LeaveIfError(err);
    
    CleanupStack::PopAndDestroy(rep);
	}


// -----------------------------------------------------------------------------
// CAspSchedule::SaveL
//
// -----------------------------------------------------------------------------
//
void CAspSchedule::SaveL()
	{
	ExternalizeL();
	}

// -----------------------------------------------------------------------------
// CAspSchedule::PeakScheduleHandle
//
// -----------------------------------------------------------------------------
//
TInt CAspSchedule::PeakScheduleHandle()
	{
	return iPeakScheduleHandle;
	}

// -----------------------------------------------------------------------------
// CAspSchedule::SetPeakScheduleHandle
//
// -----------------------------------------------------------------------------
//
void CAspSchedule::SetPeakScheduleHandle(TInt aScheduleHandle)
	{
	iPeakScheduleHandle = aScheduleHandle;
	}


// -----------------------------------------------------------------------------
// CAspSchedule::SetPeakScheduleHandle
//
// -----------------------------------------------------------------------------
//
TInt CAspSchedule::OffPeakScheduleHandle()
	{
	return iOffPeakScheduleHandle;
	}


// -----------------------------------------------------------------------------
// CAspSchedule::SetPeakScheduleHandle
//
// -----------------------------------------------------------------------------
//
void CAspSchedule::SetOffPeakScheduleHandle(TInt aScheduleHandle)
	{
	iOffPeakScheduleHandle = aScheduleHandle;
	}


// -----------------------------------------------------------------------------
// CAspSchedule::ProfileId
//
// -----------------------------------------------------------------------------
//
TInt CAspSchedule::ProfileId()
	{
	return iProfileId;
	}


// -----------------------------------------------------------------------------
// CAspSchedule::SetProfileId
//
// -----------------------------------------------------------------------------
//
void CAspSchedule::SetProfileId(TInt aProfileId)
	{
	iProfileId = aProfileId;
	}

// -----------------------------------------------------------------------------
// CAspSchedule::DailySyncEnabled
//
// -----------------------------------------------------------------------------
//
TBool CAspSchedule::DailySyncEnabled()
	{
	return iDailySyncEnabled;
	}

// -----------------------------------------------------------------------------
// CAspSchedule::SetDailySyncEnabled
//
// -----------------------------------------------------------------------------
//
void CAspSchedule::SetDailySyncEnabled(TBool aSyncEnabled)
	{
	iDailySyncEnabled = aSyncEnabled;
	}


// -----------------------------------------------------------------------------
// CAspSchedule::PeakSyncEnabled
//
// -----------------------------------------------------------------------------
//
TBool CAspSchedule::PeakSyncEnabled()
	{
	return iPeakSyncEnabled;
	}

// -----------------------------------------------------------------------------
// CAspSchedule::SetPeakSyncEnabled
//
// -----------------------------------------------------------------------------
//
void CAspSchedule::SetPeakSyncEnabled(TBool aSyncEnabled)
	{
	iPeakSyncEnabled = aSyncEnabled;
	}
// -----------------------------------------------------------------------------
// CAspSchedule::OffPeakSyncEnabled
//
// -----------------------------------------------------------------------------
//

TBool CAspSchedule::OffPeakSyncEnabled()
	{
	return iOffPeakSyncEnabled;
	}

// -----------------------------------------------------------------------------
// CAspSchedule::SetOffPeakSyncEnabled
//
// -----------------------------------------------------------------------------
//
void CAspSchedule::SetOffPeakSyncEnabled(TBool aSyncEnabled)
	{
	iOffPeakSyncEnabled = aSyncEnabled;
	}


// -----------------------------------------------------------------------------
// CAspSchedule::WeekdayEnabled
//
// -----------------------------------------------------------------------------
//
TBool CAspSchedule::WeekdayEnabled(TInt aWeekday)
	{
	TInt dayFlag = WeekdayFlag(aWeekday);
	
	TFlag flag(iWeekdayFlags);
	
	return flag.IsOn(dayFlag);
	}


// -----------------------------------------------------------------------------
// CAspSchedule::SetWeekdayEnabled
//
// -----------------------------------------------------------------------------
//
void CAspSchedule::SetWeekdayEnabled(TInt aWeekday, TInt aEnabled)
	{
	TInt dayFlag = WeekdayFlag(aWeekday);
	
	TFlag flag(iWeekdayFlags);
	
	if (aEnabled)
		{
		flag.SetOn(dayFlag);
		}
	else
		{
		flag.SetOff(dayFlag);
		}
	iSettingChanged = ETrue;
	}


// -----------------------------------------------------------------------------
// CAspSchedule::WeekdayFlag
//
// -----------------------------------------------------------------------------
//
TInt CAspSchedule::WeekdayFlag(TInt aWeekday)
	{
	TInt ret = EFlagMonday;
	
	switch (aWeekday)
		{
		case EMonday:
		    ret = EFlagMonday;
		    break;
		case ETuesday:
		    ret = EFlagTuesday;
		    break;
		case EWednesday:
		    ret = EFlagWednesday;
		    break;
		case EThursday:
		    ret = EFlagThursday;
		    break;
		case EFriday:
		    ret = EFlagFriday;
		    break;
		case ESaturday:
		    ret = EFlagSaturday;
		    break;
		case ESunday:
		    ret = EFlagSunday;
		    break;
		default:
		    TUtil::Panic(KErrArgument);
		    break;
		}
		
	return ret;
	}


// -----------------------------------------------------------------------------
// CAspSchedule::SelectedDayInfo
//
// -----------------------------------------------------------------------------
//
void CAspSchedule::SelectedDayInfo(TInt& aDayCount, TInt& aWeekday)
	{
	aDayCount = 0;

	for (TInt i=EMonday; i<=ESunday; i++)
		{
		if (WeekdayEnabled(i))
			{
			aDayCount++;
			aWeekday = i;
			}
		}
	}

// -----------------------------------------------------------------------------
// CAspSchedule::SetContentEnabled
//
// -----------------------------------------------------------------------------
//
void CAspSchedule::SetContentEnabled(TInt aContent, TInt aEnabled)
	{
	TInt contentFlag = 1 << aContent;
	TFlag flag(iContentFlags);
	if (aEnabled)
		{
		flag.SetOn(contentFlag);
		}
	else
		{
		flag.SetOff(contentFlag);
		}
	iSettingChanged = ETrue;
	}

// -----------------------------------------------------------------------------
// CAspSchedule::ContentEnabled
// 
// -----------------------------------------------------------------------------
//

TBool CAspSchedule::ContentEnabled(TInt aContent)
	{
	TInt contentFlag = 1 << aContent;
	TFlag flag(iContentFlags);
	return flag.IsOn(contentFlag);
	}

// -----------------------------------------------------------------------------
// CAspSchedule::ContentSelectionInfo
//
// -----------------------------------------------------------------------------
//

void CAspSchedule::ContentSelectionInfo(TInt& aContentCnt, TInt& aContent)
	{
	aContentCnt = 0;
	OpenSyncSessionL();
	TAspParam param(EApplicationIdSync, &iSyncSession);
	param.iMode = CAspContentList::EInitDataProviders;
	CAspContentList* asContentList = CAspContentList::NewLC(param);		
	
	//contents are saved in the sorted order of data providers
	TInt providerCount = asContentList->ProviderCount();
	for (TInt i= 0; i < providerCount; i++)
		{
		if (ContentEnabled(i))
			{
			aContentCnt++;
			aContent = i;
			}
		}
	CleanupStack::PopAndDestroy(asContentList);
	
	}	
	

// -----------------------------------------------------------------------------
// CAspSchedule::StartPeakTime
//
// -----------------------------------------------------------------------------
//

TTime CAspSchedule::StartPeakTime()
	{
	TDateTime time(0, EJanuary, 0, iStartPeakTimeHr, iStartPeakTimeMin, 0, 0);
	TTime startTime(time);
	return startTime;
	}


// -----------------------------------------------------------------------------
// CAspSchedule::SetStartPeakTime
//
// -----------------------------------------------------------------------------
//

void CAspSchedule::SetStartPeakTime(TTime aStartTime)
	{
	TDateTime startTime  = aStartTime.DateTime();
	if (iStartPeakTimeHr != startTime.Hour() || iStartPeakTimeMin != startTime.Minute())
		{
		iStartPeakTimeHr = startTime.Hour();
		iStartPeakTimeMin = startTime.Minute();
		iSettingChanged = ETrue;
		}
	}

// -----------------------------------------------------------------------------
// CAspSchedule::EndPeakTime
//
// -----------------------------------------------------------------------------
//


TTime CAspSchedule::EndPeakTime()
	{
	TDateTime time(0, EJanuary, 0, iEndPeakTimeHr, iEndPeakTimeMin, 0, 0);
	TTime endTime(time);
	return endTime;
	}

// -----------------------------------------------------------------------------
// CAspSchedule::SetEndPeakTime
//
// -----------------------------------------------------------------------------
//

void CAspSchedule::SetEndPeakTime(TTime aEndTime)
	{
	TDateTime endTime  = aEndTime.DateTime();
	
	if (iEndPeakTimeHr != endTime.Hour() || iEndPeakTimeMin != endTime.Minute())
		{
		iEndPeakTimeHr = endTime.Hour();
		iEndPeakTimeMin = endTime.Minute();
		iSettingChanged = ETrue;
		}
	}

// -----------------------------------------------------------------------------
// CAspSchedule::SyncFrequency
//
// -----------------------------------------------------------------------------
//

TInt CAspSchedule::SyncFrequency()
	{
	return iSyncFrequency;
	}

// -----------------------------------------------------------------------------
// CAspSchedule::SetSyncFrequency
//
// -----------------------------------------------------------------------------
//

void CAspSchedule::SetSyncFrequency(TInt aSchedule)
	{
	if (aSchedule != iSyncFrequency)
		{
		iSyncFrequency = aSchedule;
		iSettingChanged = ETrue;
		}
	}

// -----------------------------------------------------------------------------
// CAspSchedule::SyncPeakSchedule
//
// -----------------------------------------------------------------------------
//

TInt CAspSchedule::SyncPeakSchedule()
	{
	return iSyncPeakSchedule;
	}

// -----------------------------------------------------------------------------
// CAspSchedule::SetSyncPeakSchedule
//
// -----------------------------------------------------------------------------
//

void CAspSchedule::SetSyncPeakSchedule(TInt aSchedule)
	{
	if (aSchedule != iSyncPeakSchedule)
		{
		iSyncPeakSchedule = aSchedule;
		iSettingChanged = ETrue;
		}
	}

// -----------------------------------------------------------------------------
// CAspSchedule::SyncOffPeakSchedule
//
// -----------------------------------------------------------------------------
//

TInt CAspSchedule::SyncOffPeakSchedule()
	{
	return iSyncOffPeakSchedule;
	}

// -----------------------------------------------------------------------------
// CAspSchedule::SetSyncOffPeakSchedule
//
// -----------------------------------------------------------------------------
//

void CAspSchedule::SetSyncOffPeakSchedule(TInt aSchedule)
	{
	if (aSchedule != iSyncOffPeakSchedule)
		{
		iSyncOffPeakSchedule = aSchedule;
		iSettingChanged = ETrue;
		}
	}

// -----------------------------------------------------------------------------
// CAspSchedule::AutoSyncScheduleTime
//
// -----------------------------------------------------------------------------
//

TTime CAspSchedule::AutoSyncScheduleTime()
	{
	TDateTime time(0, EJanuary, 0, iAutoSyncScheduleTimeHr, iAutoSyncScheduleTimeMin, 0, 0);
	TTime scheduleTime(time);
	return scheduleTime;
	}

// -----------------------------------------------------------------------------
// CAspSchedule::SetAutoSyncScheduleTime
//
// -----------------------------------------------------------------------------
//

void CAspSchedule::SetAutoSyncScheduleTime(TTime aSchedule)
	{
	TDateTime schedule  = aSchedule.DateTime();
	
	//To do- check value
	if (iAutoSyncScheduleTimeHr != schedule.Hour() || iAutoSyncScheduleTimeMin != schedule.Minute())
		{
		iAutoSyncScheduleTimeHr = schedule.Hour();
		iAutoSyncScheduleTimeMin = schedule.Minute();
		iSettingChanged = ETrue;
		}
	}


// -----------------------------------------------------------------------------
// CAspSchedule::RoamingAllowed
//
// -----------------------------------------------------------------------------
//
TBool CAspSchedule::RoamingAllowed()
	{
	return iRoamingAllowed;
	}


// -----------------------------------------------------------------------------
// CAspSchedule::SetRoamingAllowed
//
// -----------------------------------------------------------------------------
//
void CAspSchedule::SetRoamingAllowed(TBool aRoamingAllowed)
	{
	iRoamingAllowed = aRoamingAllowed;
	}


// -----------------------------------------------------------------------------
// CAspSchedule::Error
//
// -----------------------------------------------------------------------------
//
TInt CAspSchedule::Error()
	{
	return iError;
	}


// -----------------------------------------------------------------------------
// CAspSchedule::SetError
//
// -----------------------------------------------------------------------------
//
void CAspSchedule::SetError(TInt aError)
	{
	iError = aError;
	}



// ----------------------------------------------------------------------------
// CAspSchedule::UniversalStartTime
//
// ----------------------------------------------------------------------------
//
TTime CAspSchedule::UniversalStartTime(TInt& aStartHour)
	{
	// add two hours to current universal time and 
	// set minutes to 0 (eg. 13:45 -> 15:00)
    TTime time;
    time.UniversalTime();
 
    TTimeIntervalHours twoHours(2);
    time += twoHours;
	
    TDateTime dateTime = time.DateTime();
    dateTime.SetMinute(0);
   	dateTime.SetSecond(0);
	dateTime.SetMicroSecond(0);

	aStartHour = dateTime.Hour();  // return start hour to caller
	
    TTime time2(dateTime);
    return time2;
	}


// ----------------------------------------------------------------------------
// CAspSchedule::LocalStartTime
//
// ----------------------------------------------------------------------------
//
TTime CAspSchedule::LocalStartTime(TInt aStartHour)
	{
	TTime time;
	time.HomeTime();
	
	TDateTime dateTime = time.DateTime();
	dateTime.SetMinute(0);
	dateTime.SetSecond(0);
	dateTime.SetMicroSecond(0);
	dateTime.SetHour(aStartHour);
	
	TTime time2(dateTime); 
	TTime now;
	now.HomeTime();
	if (now > time2)
		{
	    TTimeIntervalDays oneDay(1);
	    time2 += oneDay;
		}
		
	return time2;
	}


// ----------------------------------------------------------------------------
// CAspSchedule::IsDaySelected
//
// ----------------------------------------------------------------------------
//
TBool CAspSchedule::IsDaySelected(TTime aTime)
	{
	TDay day = aTime.DayNoInWeek();
	TBool ret = WeekdayEnabled(day);

	return ret;
	}

// -----------------------------------------------------------------------------
// CAspSchedule::UpdateSyncScheduleL
// Called when any auto sync setting is changed
// -----------------------------------------------------------------------------
//
void CAspSchedule::UpdateSyncScheduleL()
	{
	FLOG( _L("CAspSchedule::UpdateSyncScheduleL START") );
	
	CAspSyncSchedule* schedule = CAspSyncSchedule::NewLC();
	
	TInt profileId      = ProfileId();
	TBool contentStatus = IsContentSelected();
		    
   	if (profileId == KErrNotFound || !contentStatus)
		{
		SetDailySyncEnabled(EFalse);
		SetPeakSyncEnabled(EFalse);
		SetOffPeakSyncEnabled(EFalse);
		schedule->DeleteScheduleL(this);
		CleanupStack::PopAndDestroy(schedule);
		return;
		}

   	if (!IsAutoSyncEnabled())
   		{
		//Manual Sync selected ---->Delete schedule, if exists
	 	 schedule->DeleteScheduleL(this);
    	}
   	else
   		{
   		if (iDailySyncEnabled)
   			{
			//Daily sync enabled ,use peak scheduling with intervals more than a day
	  	 	schedule->DeleteScheduleL(this);
			SetDailySyncEnabled(ETrue);
			schedule->CreatePeakScheduleL(this);
			}
  		else
   			{
   		 	if (iPeakSyncEnabled != 0 && iOffPeakSyncEnabled ==0)
  				{
  				//Only peak sync enabled
	  	 		schedule->DeleteScheduleL(this);
				SetPeakSyncEnabled(ETrue);
				schedule->CreatePeakScheduleL(this);
				}
   			else if (iPeakSyncEnabled == 0 && iOffPeakSyncEnabled !=0)
  				{			
  				//Only peak sync enabled
	  			schedule->DeleteScheduleL(this);
				SetOffPeakSyncEnabled(ETrue);
				schedule->CreateOffPeakScheduleL(this);
				}
   			else if (iPeakSyncEnabled && iOffPeakSyncEnabled)
    			{
      			schedule->DeleteScheduleL(this);
				SetPeakSyncEnabled(ETrue);
				SetOffPeakSyncEnabled(ETrue);
				schedule->CreatePeakScheduleL(this);
      			schedule->CreateOffPeakScheduleL(this);
				}   
   			}
   		}

  	
    	CleanupStack::PopAndDestroy(schedule);
    
#ifdef _DEBUG
    
    CAspSyncSchedule* s = CAspSyncSchedule::NewLC();
    FLOG( _L("") );
    LogScheduleL();
    FLOG( _L("") );
    s->LogSchedule(this);
    FLOG( _L("") );
    CleanupStack::PopAndDestroy(s);
    
#endif  //    _DEBUG 
    
    FLOG( _L("CAspSchedule::UpdateSyncScheduleL END") );
	}



// -----------------------------------------------------------------------------
// CAspSchedule::UpdateSyncSchedule
//
// -----------------------------------------------------------------------------
//
void CAspSchedule::UpdateSyncSchedule()
	{
	TRAPD(err, UpdateSyncScheduleL());
	
	if (err != KErrNone)
		{
		FLOG( _L("### CAspSchedule::UpdateSyncScheduleL failed (err=%d) ###"), err );
		}
	}

// -----------------------------------------------------------------------------
// CAspSchedule::GetStartTimeL
// -Get next auto sync start time ,peak/off-peak whichever is earlier and enabled
// -----------------------------------------------------------------------------
//
void CAspSchedule::GetStartTimeL(TTime& aTime, TBool aHomeTime)
	{

	CAspSyncSchedule* s = CAspSyncSchedule::NewLC();

	if (iDailySyncEnabled)
		{
		s->GetPeakStartTimeL(this, aTime, aHomeTime);
		}
	else if (iPeakSyncEnabled != 0 && iOffPeakSyncEnabled ==0)
		{
		s->GetPeakStartTimeL(this, aTime, aHomeTime);
		}
	else if (iPeakSyncEnabled == 0 && iOffPeakSyncEnabled !=0)
		{
		s->GetOffPeakStartTimeL(this, aTime, aHomeTime);
		}
	else //peak & off-peak schedules enabled
		{
		TTime peakStart;
		TTime offPeakStart;
		
		s->GetPeakStartTimeL(this, peakStart, aHomeTime);
		s->GetOffPeakStartTimeL(this, offPeakStart, aHomeTime);

		if (peakStart < offPeakStart)
			{
			aTime = peakStart;
			}
		else
			{
			aTime = offPeakStart;
			}
		}
    CleanupStack::PopAndDestroy(s);
	}

// -----------------------------------------------------------------------------
// CAspSchedule::CanSynchronizeL
// Check primary auto sync settings
// -----------------------------------------------------------------------------
//
TBool CAspSchedule::CanSynchronizeL()
	{
	if (iProfileId == KErrNotFound && iAutoSyncProfileId == KErrNotFound)
		{
        FLOG( _L("CAspSchedule::CanSynchronizeL Failed :CP1") );
		return EFalse;
		}
	OpenSyncSessionL();
	TAspParam param(EApplicationIdSync, &iSyncSession);
	CAspProfile* profile = CAspProfile::NewLC(param);

	TRAPD (err, profile->OpenL(iProfileId, CAspProfile::EOpenRead, 
	                           CAspProfile::EBaseProperties));
	if (err == KErrNotFound)
		{
		SetProfileId(KErrNotFound);
		if (iAutoSyncProfileId != KErrNotFound)
			{
			TRAPD (err1, profile->OpenL(iAutoSyncProfileId, CAspProfile::EOpenRead, 
	                           CAspProfile::EBaseProperties));
			if (err1 == KErrNotFound)
				{
				SetAutoSyncProfileId(KErrNotFound);
				}
			}		
		UpdateSyncSchedule();
		SaveL();
		CleanupStack::PopAndDestroy(profile);
		FLOG( _L("CAspSchedule::CanSynchronizeL Failed :CP2") );
		return EFalse;
		}
	CleanupStack::PopAndDestroy(profile);

	if (!IsContentSelected())
		{
		UpdateSyncSchedule();
		SaveL();
		FLOG( _L("CAspSchedule::CanSynchronizeL Failed :CP3") );
		return EFalse;
		}
	
	if (!IsAutoSyncEnabled())
		{
        FLOG( _L("CAspSchedule::CanSynchronizeL Failed :CP4") );
		return EFalse;
		}
	
	return ETrue;
	}

// -----------------------------------------------------------------------------
// CAspSchedule::IsValidPeakScheduleL
// Check if current time is peak
// -----------------------------------------------------------------------------
//
TInt CAspSchedule::IsValidPeakScheduleL()
	{
	//convert all time values to same day and comapre
	const TInt KPositive = 1;
	const TInt KZero = 0;
	const TInt KNegative = -1;

	TTime startPeak = StartPeakTime();
	TTime endPeak   = EndPeakTime();
	
	TInt retVal = 0;
	TTime now;
	now.HomeTime();
	
	if (!WeekdayEnabled(now.DayNoInWeek()))
		{
		retVal = KPositive; 
		return retVal;
		}

	TDateTime timenow = now.DateTime();
	TDateTime date = startPeak.DateTime();
	TInt nowHr  = timenow.Hour();
	TInt nowMin = timenow.Minute();
	TInt day    = date.Day();

    TDateTime time(0, EJanuary, day, nowHr, nowMin, 0, 0);
	TTime curTime(time);
	
	if ( curTime >= startPeak && curTime < endPeak)
		{
		retVal =  KZero; //In peak period
		}
	else if (curTime < startPeak)
		{
		retVal = KPositive;
		}
	else if (curTime > endPeak)
		{
		retVal = KNegative;
		}
    
    return retVal;
	
	}

// -----------------------------------------------------------------------------
// CAspSchedule::IsValidOffPeakScheduleL
// Check if the current time off-peak
// -----------------------------------------------------------------------------
//

TInt CAspSchedule::IsValidOffPeakScheduleL()
	{
	//convert all time values to same day and comapre
	const TInt KPositive = 1;
	const TInt KZero = 0;
		
	TTime startOffPeak = EndPeakTime()+ (TTimeIntervalMinutes)1;
	TTime endOffPeak   = StartPeakTime() + (TTimeIntervalDays)1 -(TTimeIntervalMinutes)1 ;
	TInt retVal = 0;
	TTime now;
	now.HomeTime();

	if (!WeekdayEnabled(now.DayNoInWeek()))
		{
		retVal = KZero; 
		return retVal;
		}

	TDateTime timenow = now.DateTime();
	TDateTime date = startOffPeak.DateTime();
	TInt nowHr  = timenow.Hour();
	TInt nowMin = timenow.Minute();
	TInt day    = date.Day();

    TDateTime time(0, EJanuary, day, nowHr, nowMin, 0, 0);
	TTime curTime(time);
	
	if (curTime < StartPeakTime() || (curTime >= startOffPeak && curTime < endOffPeak ))
		{
		retVal = KZero; 
		}
	else if (curTime < startOffPeak)
		{
		retVal = KPositive;		
		}
	
	return retVal;
	}

// -----------------------------------------------------------------------------
// CAspSchedule::IsValidNextPeakScheduleL
// -Add sync interval to current time and checks if the next schedule will be in peak period.
// -otherwise sync can be postponed until next peak start
// -----------------------------------------------------------------------------
//
TBool CAspSchedule::IsValidNextPeakScheduleL()
	{
	//convert all time vakues to same day and compare
	TTime startPeak = StartPeakTime();
	TTime endPeak   = EndPeakTime();
	
	TTime now;
	now.HomeTime();

	CAspSyncSchedule* syncSchedule = CAspSyncSchedule::NewLC();
	TInt interval = SyncPeakSchedule();
    TIntervalType intervalType = syncSchedule->IntervalType(interval);
  	
	TInt intervalVal = syncSchedule->Interval(interval);

	if (interval == EInterval15Mins
				|| interval == EInterval30Mins)
		{
		now = now + (TTimeIntervalMinutes)intervalVal;
		}
	else if (intervalType == EHourly)
		{
		now = now + (TTimeIntervalHours)intervalVal;
		}
		
	CleanupStack::PopAndDestroy(syncSchedule);	
	if (!WeekdayEnabled(now.DayNoInWeek()))
		{
		return EFalse;
		}
	TDateTime timenow = now.DateTime();
	TDateTime date = startPeak.DateTime();
	TInt nowHr  = timenow.Hour();
	TInt nowMin = timenow.Minute();
	TInt day    = date.Day();

    TDateTime time(0, EJanuary, day, nowHr, nowMin, 0, 0);
	TTime nextPeakSync(time);
	
	if ( nextPeakSync >= startPeak && nextPeakSync < endPeak)
		{
		return ETrue; //In peak period
		}

	return EFalse;
	}

// -----------------------------------------------------------------------------
// CAspSchedule::IsValidNextOffPeakScheduleL
// -Add sync interval to current time and checks if the next schedule will be in off-peak period.
// -otherwise sync can be postponed until next off-peak start
// -----------------------------------------------------------------------------
//
TBool CAspSchedule::IsValidNextOffPeakScheduleL()
	{

    TTime startOffPeak = EndPeakTime()+ (TTimeIntervalMinutes)1;
	TTime endOffPeak   = StartPeakTime() + (TTimeIntervalDays)1 -(TTimeIntervalMinutes)1 ;
	
	TTime now;
	now.HomeTime();

	CAspSyncSchedule* syncSchedule = CAspSyncSchedule::NewLC();
	TInt interval = SyncOffPeakSchedule();
    TIntervalType intervalType = syncSchedule->IntervalType(interval);
  	
	TInt intervalVal = syncSchedule->Interval(interval);

	if (interval == EInterval15Mins
				|| interval == EInterval30Mins)
		{
		now = now + (TTimeIntervalMinutes)intervalVal;
		}
	else if (intervalType == EHourly)
		{
		now = now + (TTimeIntervalHours)intervalVal;
		}
		
	CleanupStack::PopAndDestroy(syncSchedule);

	if (!WeekdayEnabled(now.DayNoInWeek()))
		{
		return ETrue;
		}
	
	TDateTime timenow = now.DateTime();
	TDateTime date = startOffPeak.DateTime();
	TInt nowHr  = timenow.Hour();
	TInt nowMin = timenow.Minute();
	TInt day    = date.Day();

    TDateTime time(0, EJanuary, day, nowHr, nowMin, 0, 0);
	TTime nextOffPeakSync(time);

	if (nextOffPeakSync < StartPeakTime() || (nextOffPeakSync >= startOffPeak && nextOffPeakSync < endOffPeak ))
		{
		return ETrue; //in off-peak period
		}
	return EFalse;
	}


// -----------------------------------------------------------------------------
// CAspSchedule::CreateAutoSyncProfileL
// -Creates a hidden profile that is used for auto sync.
// -It is created when auto sync is used for first time
// -----------------------------------------------------------------------------
//

void CAspSchedule::CreateAutoSyncProfileL()
	{
	if (iAutoSyncProfileId != KErrNotFound)
		{
		return;			
		}
	OpenSyncSessionL();

	TBuf<KBufSize> aBufName(KAutoSyncProfileName);
	
	TAspParam param(EApplicationIdSync, &iSyncSession);
	CAspProfile* profile = CAspProfile::NewLC(param);

	profile->CreateL(CAspProfile::EAllProperties);
	CAspProfile::SetDefaultValuesL(profile);
	profile->SetNameL(aBufName);
	profile->SetSASyncStateL(ESmlDisableSync);
	profile->SaveL();
	
	iAutoSyncProfileId = profile->ProfileId();
	SaveL();
	
	CleanupStack::PopAndDestroy(profile);
	
	
	}

// -----------------------------------------------------------------------------
// CAspSchedule::AutoSyncProfileId
// -----------------------------------------------------------------------------
//

TInt CAspSchedule::AutoSyncProfileId()
	{
	return iAutoSyncProfileId;
	}

// -----------------------------------------------------------------------------
// CAspSchedule::SetAutoSyncProfileId
// -----------------------------------------------------------------------------
//

void CAspSchedule::SetAutoSyncProfileId(TInt aProfileId)
	{
	iAutoSyncProfileId = aProfileId;
	}


// -----------------------------------------------------------------------------
// CAspSchedule::CopyAutoSyncContentsL
// -Copy contents selected in Automatic sync settings view to the profiles content list
// -----------------------------------------------------------------------------
//

void CAspSchedule::CopyAutoSyncContentsL()
	{
	FLOG( _L("CAspSchedule::CopyAutoSyncContentsL START") );
	if (iAutoSyncProfileId == KErrNotFound)
		{
		return;
		}
	OpenSyncSessionL();
	TAspParam param(EApplicationIdSync, &iSyncSession);
	
	param.iMode = CAspContentList::EInitDataProviders;
	CAspContentList* asContentList = CAspContentList::NewLC(param);		
	
	CAspProfile* profile = CAspProfile::NewLC(param);

	TRAPD (err, profile->OpenL(iAutoSyncProfileId, CAspProfile::EOpenReadWrite, 
	                           CAspProfile::EAllProperties));
	User::LeaveIfError(err);

	asContentList->SetProfile(profile);
	asContentList->InitAllTasksL();
		
	TBool aEnabled;
	
	TInt providerCount = asContentList->ProviderCount();
		
	for (TInt i= 0; i < providerCount; i++)
		{
		if(ContentEnabled(i))
			{
			aEnabled = ETrue;
			}
		else
			{
			aEnabled = EFalse;
			}
		TAspProviderItem& provider = asContentList->ProviderItem(i);	
		if (provider.iDataProviderId != KErrNotFound)
			{
			asContentList->ModifyTaskIncludedL(provider.iDataProviderId, aEnabled, KNullDesC);
			}
		}
	CleanupStack::PopAndDestroy(profile);
	CleanupStack::PopAndDestroy(asContentList);
	FLOG( _L("CAspSchedule::CopyAutoSyncContentsL END") );
     
	}

// -----------------------------------------------------------------------------
// CAspSchedule::UpdateProfileSettingsL
// -Copy latest settings from profile selected for sync to auto sync profile.
// -Called before every scheduled sync to get updated settings
// -Also called when any settings changed
// -----------------------------------------------------------------------------
//

void CAspSchedule::UpdateProfileSettingsL()
	{
	FLOG( _L("CAspSchedule::UpdateProfileSettingsL START") );
	if (iProfileId == KErrNotFound || iAutoSyncProfileId == KErrNotFound)
		{
		return;
		}
	OpenSyncSessionL();
	TAspParam param(EApplicationIdSync, &iSyncSession);

	param.iMode = CAspContentList::EInitDataProviders;
			
	CAspContentList* asContentList = CAspContentList::NewL(param);
	CleanupStack::PushL(asContentList);
	CAspContentList* contentList = CAspContentList::NewL(param);
	CleanupStack::PushL(contentList);
	
    CAspProfile* selectedProfile = CAspProfile::NewLC(param);
    selectedProfile->OpenL(iProfileId, CAspProfile::EOpenRead, CAspProfile::EAllProperties);

	CAspProfile* autosyncProfile = CAspProfile::NewLC(param);
	autosyncProfile->OpenL(iAutoSyncProfileId, CAspProfile::EOpenReadWrite, CAspProfile::EAllProperties);

	autosyncProfile->CopyValuesL(selectedProfile);
	autosyncProfile->SetSASyncStateL(ESmlDisableSync);
	autosyncProfile->SaveL();

    //Copy database names.
    contentList->SetProfile(selectedProfile);
	contentList->InitAllTasksL();
	
	TBuf<KBufSize> aLocalDatabase;
	TBuf<KBufSize> aRemoteDatabase;
	TInt aSyncDirection;
	TBool aTaskEnabled;
	
	asContentList->SetProfile(autosyncProfile);
	asContentList->InitAllTasksL();
	
	TInt providerCount = asContentList->ProviderCount();
	
 	for (TInt i= 0; i < providerCount; i++)
		{
		TAspProviderItem& provider = asContentList->ProviderItem(i);
			
		contentList->ReadTaskL(provider.iDataProviderId, aLocalDatabase, aRemoteDatabase,
		                    aTaskEnabled, aSyncDirection);
		
		
		if(ContentEnabled(i))
			{
			aTaskEnabled = ETrue;
			}
		else
			{
			aTaskEnabled = EFalse;
			}

		if (provider.iDataProviderId == KUidNSmlAdapterEMail.iUid)
			{
			if (!MailboxExistL())
				{
				continue;
				}
			RSyncMLDataProvider provider;
			TInt err;
			TRAP(err, provider.OpenL(iSyncSession, KUidNSmlAdapterEMail.iUid));
			aLocalDatabase = provider.DefaultDataStoreName();
			}
		
		if (aTaskEnabled)
			{		
			TBuf<KBufSize> asLocaldb;
			TBuf<KBufSize> asRemotedb;
			TBool asTask;
			TInt asDirection;
		
			asContentList->ReadTaskL(provider.iDataProviderId,asLocaldb,asRemotedb,
													asTask,asDirection);
													
		    if (asLocaldb != aLocalDatabase || asRemotedb != aRemoteDatabase ||
		    					asTask != aTaskEnabled || asDirection != aSyncDirection )
		    	{
		    	TRAPD(err,	asContentList->CreateTaskL(provider.iDataProviderId, aLocalDatabase, aRemoteDatabase,
	                              aTaskEnabled, aSyncDirection));
		    	User::LeaveIfError(err);
		    	}
			}
	
		}
	CleanupStack::PopAndDestroy(autosyncProfile);
	CleanupStack::PopAndDestroy(selectedProfile);
	
	CleanupStack::PopAndDestroy(contentList);
	CleanupStack::PopAndDestroy(asContentList);
	FLOG( _L("CAspSchedule::UpdateProfileSettingsL END") );
	
   }

// -----------------------------------------------------------------------------
// CAspSchedule::UpdatePeakSchedule
// -----------------------------------------------------------------------------
//
void CAspSchedule::UpdatePeakScheduleL()
	{
	//Today's peak time ended ,postpone the peak scheduling until tomorrow peak start time
	CAspSyncSchedule* syncSchedule = CAspSyncSchedule::NewLC();
    TIntervalType intervalType = syncSchedule->IntervalType(SyncPeakSchedule());
    if(intervalType == EDaily )
    	{
		CleanupStack::PopAndDestroy(syncSchedule);
		return;
		}
	syncSchedule->UpdatePeakScheduleL(this);
	CleanupStack::PopAndDestroy(syncSchedule);		

    }
// -----------------------------------------------------------------------------
// CAspSchedule::UpdateOffPeakSchedule
// -----------------------------------------------------------------------------
//

void CAspSchedule::UpdateOffPeakScheduleL()
	{
	//postpone the off-peak scheduling until tomorrow off-peak start time
	CAspSyncSchedule* syncSchedule = CAspSyncSchedule::NewLC();
    TIntervalType intervalType = syncSchedule->IntervalType(SyncOffPeakSchedule());
    if (intervalType == EDaily )
    	{
		CleanupStack::PopAndDestroy(syncSchedule);
		return;
		}
	syncSchedule->UpdateOffPeakScheduleL(this);
	CleanupStack::PopAndDestroy(syncSchedule);		

    }

// -----------------------------------------------------------------------------
// CAspSchedule::EnableSchedule
// -----------------------------------------------------------------------------
//
void CAspSchedule::EnableScheduleL()
	{
	if ( iPeakScheduleHandle == KErrNotFound && iOffPeakScheduleHandle == KErrNotFound)
		{
		return;
		}
	CAspSyncSchedule* syncSchedule = CAspSyncSchedule::NewLC();
	syncSchedule->EnableSchedule(this);
	CleanupStack::PopAndDestroy(syncSchedule);		
	}

// -----------------------------------------------------------------------------
// CAspSchedule::UpdateOffPeakSchedule
// -----------------------------------------------------------------------------
//
TIntervalType CAspSchedule::IntervalType(TInt aInterval)
	{
	if (aInterval == EInterval24hours
			  || aInterval == EInterval2days
			  || aInterval == EInterval4days
			  || aInterval == EInterval7days
			  || aInterval == EInterval14days
			  || aInterval == EInterval30days)
		{
		return EDaily;
		}
		
	return EHourly;
	}

// ----------------------------------------------------------------------------
// CAspSchedule::MailboxExistL()
// Return ETrue if any mail box exists, EFlase otherwise
// ----------------------------------------------------------------------------
//
TBool CAspSchedule::MailboxExistL()
	{
	OpenSyncSessionL();
	TBool mailboxExist = EFalse;
	TAspParam param(EApplicationIdSync, &iSyncSession);
	param.iMode = CAspContentList::EInitDataProviders;
	CAspContentList* contentList = CAspContentList::NewL(param);
	CleanupStack::PushL(contentList);
	// get email syncml mailboxes
   	CDesCArray* localDataStores = new (ELeave) CDesCArrayFlat(KDefaultArraySize);
	CleanupStack::PushL(localDataStores);
	contentList->GetLocalDatabaseList(KUidNSmlAdapterEMail.iUid, localDataStores);

    if (localDataStores->Count() > 0)
   		{
   		mailboxExist = ETrue;
   		}

	CleanupStack::PopAndDestroy(localDataStores);
	CleanupStack::PopAndDestroy(contentList);

	return mailboxExist;
	}


// ----------------------------------------------------------------------------
// CAspSchedule::ProtocolL()
// Return server protocol version 
// ----------------------------------------------------------------------------
//
TInt CAspSchedule::ProtocolL()
	{
	OpenSyncSessionL();
	TAspParam param(EApplicationIdSync, &iSyncSession);
	CAspProfile* profile = CAspProfile::NewLC(param);
	profile->OpenL(iProfileId, CAspProfile::EOpenRead,
                                         CAspProfile::EBaseProperties);
	TInt protocol = profile->ProtocolVersion();
	CleanupStack::PopAndDestroy(profile);
	return protocol;
	}


// ----------------------------------------------------------------------------
// CAspSchedule::IsAutoSyncEnabled()
// Return ETrue if autosync enable ,EFalse otherwise 
// ----------------------------------------------------------------------------
//
TBool CAspSchedule::IsAutoSyncEnabled()
	{
	if (iDailySyncEnabled || iPeakSyncEnabled || iOffPeakSyncEnabled)
		{
		return ETrue;
		}
	return EFalse;
	}


// -----------------------------------------------------------------------------
// CAspSchedule::OpenSyncSessionL
// -----------------------------------------------------------------------------
//

void CAspSchedule::OpenSyncSessionL()
	{
	if (!iSyncSessionOpen)
		{
        TRAPD(err, iSyncSession.OpenL());
        
        if (err != KErrNone)
        	{
        	FLOG( _L("### CAspSchedule: RSyncMLSession::OpenL failed (%d) ###"), err );
        	User::Leave(err);
        	}

		iSyncSessionOpen = ETrue;
		}
    }

// -----------------------------------------------------------------------------
// CAspSchedule::CloseSyncSession
// -----------------------------------------------------------------------------
//

void CAspSchedule::CloseSyncSession()
	{
	if (iSyncSessionOpen)
		{
		iSyncSession.Close();
		iSyncSessionOpen = EFalse;
		}
    }


// -----------------------------------------------------------------------------
// CAspSchedule::CheckMandatoryData
// 
// -----------------------------------------------------------------------------
//
TInt CAspSchedule::CheckMandatoryDataL(TInt& count,TInt& firstItem)
	{
	FLOG( _L("CAspSchedule::CheckMandatoryDataL START") );
	OpenSyncSessionL();
    TAspParam param(EApplicationIdSync, &iSyncSession);

    CAspProfile* profile = CAspProfile::NewLC(param);
    profile->OpenL(iProfileId, CAspProfile::EOpenRead, CAspProfile::EAllProperties);
    
	param.iProfile = profile;
	param.iMode = CAspContentList::EInitAll;
	CAspContentList* list = CAspContentList::NewLC(param);

   	TInt index = 0;
	count = 0;
	firstItem = 0;
	
	TInt providerCount = list->ProviderCount();
	
	for (TInt i= 0; i < providerCount; i++)
		{
		if (ContentEnabled(i))
			{
			FLOG( _L("Content Id : %d"),i );
			TAspProviderItem& provider = list->ProviderItem(i);	
			index = list->FindTaskIndexForProvider(provider.iDataProviderId);
			if (index == KErrNotFound )
				{
				FLOG( _L("Index not found , Id : %d"),i );
				count++;
				if(count == 1)
					{
					firstItem = i;
					}
				continue;
				}
			TAspTaskItem& task = list->TaskItem(index);
			if (provider.iDataProviderId == KUidNSmlAdapterEMail.iUid)
				{
				if(!task.iRemoteDatabaseDefined || !task.iLocalDatabaseDefined)
					{
					count++;
					if(count == 1)
						{
						firstItem = i;
						}
					}
				}
			else
				{
				if (!task.iRemoteDatabaseDefined)
					{
					count++;
					if(count == 1)
						{
						firstItem = i;
						}
					}
				}
			}
			}
		
	CleanupStack::PopAndDestroy(list);
	CleanupStack::PopAndDestroy(profile);
	
	if (count)
		{
		return EFalse;
		}

	FLOG( _L("CAspSchedule::CheckMandatoryDataL END") );
	return ETrue;

   }


// -----------------------------------------------------------------------------
// CAspSchedule::ShowAutoSyncLogL
//
// -----------------------------------------------------------------------------
//
void CAspSchedule::ShowAutoSyncLogL()
	{
	FLOG( _L("CAspSchedule::ShowAutoSyncLogL START") );
	OpenSyncSessionL();
    TAspParam param(EApplicationIdSync, &iSyncSession);
	param.iProfileId = iProfileId;

	CAspProfile* asProfile = CAspProfile::NewLC(param);
	asProfile->OpenL(iAutoSyncProfileId, CAspProfile::EOpenRead, CAspProfile::EAllProperties);
	
	CAspProfile* profile = CAspProfile::NewLC(param);
	profile->OpenL(iProfileId, CAspProfile::EOpenRead, CAspProfile::EAllProperties);

	if (asProfile->LastSync() > profile->LastSync())
		{
		param.iProfileId = iAutoSyncProfileId;
		}
	CleanupStack::PopAndDestroy(profile);
	CleanupStack::PopAndDestroy(asProfile);
	
    CAspLogDialog* dialog = CAspLogDialog::NewL(param);
    CleanupStack::PushL(dialog);
        
    dialog->ShowDialogL();
        
    CleanupStack::PopAndDestroy(dialog);
	FLOG( _L("CAspSchedule::ShowAutoSyncLogL END") );
	}


// -----------------------------------------------------------------------------
// CAspSchedule::UpdateServerIdL
// Do server id modifications so that SAN selectes correct profile instead of hidden
// auto sync profile.
// -----------------------------------------------------------------------------
//
void CAspSchedule::UpdateServerIdL()
	{
	FLOG( _L("CAspSchedule::ClearAutoSyncProfileServerL START") );
	OpenSyncSessionL();
	TAspParam param(EApplicationIdSync, &iSyncSession);
		
	CAspProfile* asprofile = CAspProfile::NewLC(param);
	TRAPD (err, asprofile->OpenL(iAutoSyncProfileId, CAspProfile::EOpenReadWrite, 
	                           CAspProfile::EAllProperties));
	User::LeaveIfError(err);

	CAspProfile* profile = CAspProfile::NewLC(param);
	TRAPD (err1, profile->OpenL(iProfileId, CAspProfile::EOpenReadWrite, 
	                           CAspProfile::EAllProperties));
	User::LeaveIfError(err1);

	asprofile->GetServerId(iBuf);
	profile->SetServerIdL(iBuf);
	asprofile->SetServerIdL(KNullDesC);
	profile->SaveL();
	asprofile->SaveL();
	
	CleanupStack::PopAndDestroy(profile);
	CleanupStack::PopAndDestroy(asprofile);
	FLOG( _L("CAspSchedule::ClearAutoSyncProfileServerL END") );
	}
// -----------------------------------------------------------------------------
// CAspSchedule::IsContentSelected
//
// -----------------------------------------------------------------------------
//
TBool CAspSchedule::IsContentSelected()
	{
	TInt selectedContentCnt = 0;
	TInt selectedContentIndex = 0;
	
	ContentSelectionInfo(selectedContentCnt, selectedContentIndex);
	if (selectedContentCnt)
		{
		return ETrue;
		}
	return EFalse;
	}


#ifdef _DEBUG

void CAspSchedule::LogSettings()
	{
	FLOG( _L("iProfileId   %d"),iProfileId);
	FLOG( _L("iPeakScheduleHandle   %d"),iPeakScheduleHandle);
	FLOG( _L("iOffPeakScheduleHandle   %d"),iOffPeakScheduleHandle);
   	FLOG( _L("iStartPeakTimeHr   %d"),iStartPeakTimeHr);
	FLOG( _L("iStartPeakTimeMin   %d"),iStartPeakTimeMin);
	FLOG( _L("iEndPeakTimeHr   %d"),iEndPeakTimeHr);
	FLOG( _L("iEndPeakTimeMin   %d"),iEndPeakTimeMin);
	FLOG( _L("iSyncPeakSchedule   %d"),iSyncPeakSchedule);
	FLOG( _L("iSyncOffPeakSchedule   %d"),iSyncOffPeakSchedule);
	FLOG( _L("iDailySyncEnabled   %d"),iDailySyncEnabled);
	FLOG( _L("iPeakSyncEnabled   %d"),iPeakSyncEnabled);
	FLOG( _L("iOffPeakSyncEnabled   %d"),iOffPeakSyncEnabled);
    FLOG( _L("iAutoSyncProfileId   %d"),iAutoSyncProfileId);
    FLOG( _L("iAutoSyncScheduleTimeHr   %d"),iAutoSyncScheduleTimeHr);
    FLOG( _L("iAutoSyncScheduleTimeMin   %d"),iAutoSyncScheduleTimeMin);
	
	
	}



// -----------------------------------------------------------------------------
// CAspSchedule::LogScheduleL
// 
// -----------------------------------------------------------------------------
//
void CAspSchedule::LogScheduleL()
	{
	TBuf<128> buf; TBuf<128> buf2;
	
	FLOG( _L("---- automatic sync settings ----") );
	
	TTime time;
	time.HomeTime();
    TUtil::GetDateTimeTextL(buf, time);
    buf2.Format(_L("current time: %S"), &buf);
    FLOG(buf2);
    
    if (PeakSyncEnabled())
    	{
    	buf = _L("Peak sync enabled");
    	}
   
    FLOG(buf);

	if (OffPeakSyncEnabled())
    	{
    	buf = _L("Off-Peak sync enabled");
    	}
    FLOG(buf);
	 
    TInt id = ProfileId();
    FLOG(_L("profile id: %d"), id);
    
    LogSyncInterval();
    LogSyncDays();

#if 0   
    if (RoamingAllowed())
    	{
    	buf = _L("roaming allowed: yes");
    	}
    else
    	{
    	buf = _L("roaming allowed: no");
       	}
    FLOG(buf);
#endif
    FLOG( _L("---- automatic sync settings ----") );
    }


// -----------------------------------------------------------------------------
// CAspSchedule::LogSyncDays
// 
// -----------------------------------------------------------------------------
//
void CAspSchedule::LogSyncDays()
	{
	TBuf<128> buf; TBuf<128> buf2;
	
	buf = KNullDesC;
	
	if (WeekdayEnabled(EMonday))
    	{
    	buf.Append(_L("monday "));
    	}
	if (WeekdayEnabled(ETuesday))
    	{
    	buf.Append(_L("tuesday "));
    	}
	if (WeekdayEnabled(EWednesday))
    	{
    	buf.Append(_L("wednesday "));
    	}
	if (WeekdayEnabled(EThursday))
    	{
    	buf.Append(_L("thursday "));
    	}
	if (WeekdayEnabled(EFriday))
    	{
    	buf.Append(_L("friday "));
    	}
	if (WeekdayEnabled(ESaturday))
    	{
    	buf.Append(_L("saturday "));
    	}
	if (WeekdayEnabled(ESunday))
    	{
    	buf.Append(_L("sunday "));
    	}
	if (buf.Length() == 0)
    	{
    	buf.Append(_L("none"));
    	}

    buf2.Format(_L("sync days: %S"), &buf);
    FLOG(buf2);
    }


// -----------------------------------------------------------------------------
// CAspSchedule::LogSyncInterval
// 
// -----------------------------------------------------------------------------
//
void CAspSchedule::LogSyncInterval()
	{
	TBuf<128> buf; TBuf<128> buf2;
	
	switch (iSyncPeakSchedule)
		{
		case CAspSchedule::EIntervalManual:
		    buf = _L("Manual");
		    break;
		case CAspSchedule::EInterval15Mins:
		    buf = _L("15 minutes");
		    break;
		case CAspSchedule::EInterval30Mins:
		    buf = _L("30 minutes");
		    break;
		case CAspSchedule::EInterval1hour:
		    buf = _L("1 hour");
		    break;
		case CAspSchedule::EInterval2hours:
		    buf = _L("2 hours");
		    break;
		case CAspSchedule::EInterval4hours:
		    buf = _L("4 hours");
		    break;
		case CAspSchedule::EInterval8hours:
		    buf = _L("8 hours");
		    break;
		case CAspSchedule::EInterval12hours:
		    buf = _L("12 hours");
		    break;
		case CAspSchedule::EInterval24hours:
		    buf = _L("24 hours");
		    break;
		case CAspSchedule::EInterval2days:
		    buf = _L("2 days");
		    break;
		case CAspSchedule::EInterval4days:
		    buf = _L("4 days");
		    break;
		case CAspSchedule::EInterval7days:
		    buf = _L("7 days");
		    break;
		case CAspSchedule::EInterval14days:
		    buf = _L("14 days");
		    break;
		case CAspSchedule::EInterval30days:
		    buf = _L("30 days");
		    break;
		default:
		    buf = _L("unknown sync interval");
		    break;
		}
		
    buf2.Format(_L("sync interval: %S"), &buf);
    FLOG(buf2);
    }

#endif  // _DEBUG


#ifdef _DEBUG


// ----------------------------------------------------------------------------
// CAspSchedule::PrintTimeL
//
// ----------------------------------------------------------------------------
//
void CAspSchedule::PrintTimeL(TTime aTime)
	{
	TBuf<128> buf;
	TUtil::GetDateTimeTextL(buf, aTime);
	TDialogUtil::ShowInformationQueryL(buf);
	}


// -----------------------------------------------------------------------------
// CAspSchedule::TestL
//
// -----------------------------------------------------------------------------
//
void CAspSchedule::TestL()
	{
	TRAPD(err, DoTestL());
	
	if (err != KErrNone)
		{
	    TBuf<KBufSize> buf;
        buf.Format(_L("TestL failed (%d)"), err);
        TDialogUtil::ShowErrorNoteL(buf);
		}
	}


// -----------------------------------------------------------------------------
// CAspSchedule::DoTestL
//
// -----------------------------------------------------------------------------
//
void CAspSchedule::DoTestL()
	{
	const TInt KTestCount = 500;
	
    TInt err = KErrNone;
    
    ResetRepositoryL();
    
    CAspSchedule* s = NULL;
    TInt count = 0;
    for (; count<KTestCount; count++)
    	{
        TRAP(err, InternalizeL());
        if (err != KErrNone)
        	{
        	User::Leave(err);
        	}
        
        if (s)
        	{
      		CompareValues(s);
        	CleanupStack::PopAndDestroy(s);
        	s = NULL;
        	}

        TRAP(err, UpdateValuesL(count));
        if (err != KErrNone)
        	{
        	User::Leave(err);
        	}
        
        s = CreateCopyLC();
        	
        TRAP(err, ExternalizeL());
        if (err != KErrNone)
        	{
        	User::Leave(err);
        	}
     	}
    	
    if (s)
    	{
    	CleanupStack::PopAndDestroy(s);
    	}
    	
    TBuf<KBufSize> buf;
    buf.Format(_L("centrep test ok (%d)"), count);
    TDialogUtil::ShowErrorNoteL(buf);
    
    	
	return;
	}


// ----------------------------------------------------------------------------
// UpdateValuesL
//
// ----------------------------------------------------------------------------
//
void CAspSchedule::UpdateValuesL(TInt aCount)
    {
    TInt num = 100 * aCount;
    TBool isEven = aCount % 2;
    
    iPeakScheduleHandle = num+1;
	iOffPeakScheduleHandle = num +1;
  
    iError          = num+3;
    
    iPeakSyncEnabled = isEven;
	iOffPeakSyncEnabled = isEven;
    iProfileId = num+4;
    
    iRoamingAllowed = isEven;
    
	if (isEven)
    	{
    	SetWeekdayEnabled(EMonday, ETrue);
    	SetWeekdayEnabled(ETuesday, ETrue);
    	SetWeekdayEnabled(EWednesday, ETrue);
    	SetWeekdayEnabled(EThursday, ETrue);
    	SetWeekdayEnabled(EFriday, ETrue);
    	SetWeekdayEnabled(ESaturday, EFalse);
    	SetWeekdayEnabled(ESunday, EFalse);
    	}
    else
    	{
    	SetWeekdayEnabled(EMonday, EFalse);
    	SetWeekdayEnabled(ETuesday, EFalse);
    	SetWeekdayEnabled(EWednesday, EFalse);
    	SetWeekdayEnabled(EThursday, EFalse);
    	SetWeekdayEnabled(EFriday, EFalse);
    	SetWeekdayEnabled(ESaturday, ETrue);
    	SetWeekdayEnabled(ESunday, ETrue);
    	}
    }


// ----------------------------------------------------------------------------
// CreateCopyLC
//
// ----------------------------------------------------------------------------
//
CAspSchedule* CAspSchedule::CreateCopyLC()
    {
    CAspSchedule* s = CAspSchedule::NewL();
    CleanupStack::PushL(s);
    
    s->iPeakScheduleHandle = iPeakScheduleHandle;
	s->iOffPeakScheduleHandle = iOffPeakScheduleHandle;
    s->iError = iError;
    s->SetProfileId(ProfileId());
    s->SetPeakSyncEnabled(PeakSyncEnabled());
    s->SetRoamingAllowed(RoamingAllowed());
    
    
    TBool enabled = EFalse;
    if (WeekdayEnabled(EMonday))
    	{
        enabled = ETrue;    	
    	}
    s->SetWeekdayEnabled(EMonday, enabled);    	
    	
    enabled = EFalse;
    if (WeekdayEnabled(ETuesday))
    	{
        enabled = ETrue;    	
    	}
    s->SetWeekdayEnabled(ETuesday, enabled);    	
    
    enabled = EFalse;
    if (WeekdayEnabled(EWednesday))
    	{
        enabled = ETrue;    	
    	}
    s->SetWeekdayEnabled(EWednesday, enabled);    	
    
    enabled = EFalse;
    if (WeekdayEnabled(EThursday))
    	{
        enabled = ETrue;    	
    	}
    s->SetWeekdayEnabled(EThursday, enabled);    	
    
    enabled = EFalse;
    if (WeekdayEnabled(EFriday))
    	{
        enabled = ETrue;    	
    	}
    s->SetWeekdayEnabled(EFriday, enabled);
    
    enabled = EFalse;
    if (WeekdayEnabled(ESaturday))
    	{
        enabled = ETrue;    	
    	}
    s->SetWeekdayEnabled(ESaturday, enabled);    	
    
    enabled = EFalse;
    if (WeekdayEnabled(ESunday))
    	{
        enabled = ETrue;    	
    	}
    s->SetWeekdayEnabled(ESunday, enabled);
    
    return s;
    }


// ----------------------------------------------------------------------------
// CompareValues
//
// ----------------------------------------------------------------------------
//
void CAspSchedule::CompareValues(CAspSchedule* aSchedule)
    {
    CAspSchedule* s = aSchedule;
    
    AssertEqual(s->PeakScheduleHandle(), PeakScheduleHandle());
    AssertEqual(s->OffPeakScheduleHandle(), OffPeakScheduleHandle());
    AssertEqual(s->Error(), Error());
    AssertEqual(s->ProfileId(), ProfileId());

	AssertEqual(s->PeakSyncEnabled(), PeakSyncEnabled());
	AssertEqual(s->OffPeakSyncEnabled(), OffPeakSyncEnabled());
   
    AssertEqual(s->RoamingAllowed(), RoamingAllowed());
    
    AssertEqual(s->iWeekdayFlags, iWeekdayFlags);    
  
    AssertEqual(s->WeekdayEnabled(EMonday), WeekdayEnabled(EMonday));
    AssertEqual(s->WeekdayEnabled(ETuesday), WeekdayEnabled(ETuesday));
    AssertEqual(s->WeekdayEnabled(EWednesday), WeekdayEnabled(EWednesday));
    AssertEqual(s->WeekdayEnabled(EThursday), WeekdayEnabled(EThursday));
    AssertEqual(s->WeekdayEnabled(EFriday), WeekdayEnabled(EFriday));
    AssertEqual(s->WeekdayEnabled(ESaturday), WeekdayEnabled(ESaturday));
    AssertEqual(s->WeekdayEnabled(ESunday), WeekdayEnabled(ESunday));
    }


// ----------------------------------------------------------------------------
// AssertEqual
//
// ----------------------------------------------------------------------------
//
void CAspSchedule::AssertEqual(TInt aValue1, TInt aValue2)
    {
    __ASSERT_DEBUG(aValue1 == aValue2, TUtil::Panic(KErrArgument));
    }

#endif  // _DEBUG



// End of file

