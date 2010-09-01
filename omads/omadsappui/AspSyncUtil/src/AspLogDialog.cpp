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
* Description:  
*
*/


// INCLUDE FILES

#include "AspLogDialog.h"
#include "AspDialogUtil.h"
#include "AspUtil.h"
#include "AspDebug.h"

#include <SyncMLErr.h>  // sync error codes
#include "AspSchedule.h"



/*******************************************************************************
 * class CAspLogDialog
 *******************************************************************************/




// -----------------------------------------------------------------------------
// CAspLogDialog::ShowDialogL
// 
// -----------------------------------------------------------------------------
void CAspLogDialog::ShowDialogL()
	{
	HBufC* hBuf = CAspResHandler::ReadLC(R_SML_LOG_SYNCLOG_TITLE);

    TDialogUtil::ShowPopupDialog2L(hBuf->Des(), iList);
    
   	CleanupStack::PopAndDestroy(hBuf);
	}


// -----------------------------------------------------------------------------
// CAspLogDialog::NewL
//
// -----------------------------------------------------------------------------
CAspLogDialog* CAspLogDialog::NewL(const TAspParam& aParam)
    {
    CAspLogDialog* self = new (ELeave) CAspLogDialog(aParam);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);

    return(self);
    }


// -----------------------------------------------------------------------------
// CAspLogDialog::CAspLogDialog
// 
// -----------------------------------------------------------------------------
//
CAspLogDialog::CAspLogDialog(const TAspParam& aParam)
	{
    iProfileId = aParam.iProfileId;
	iSyncSession = aParam.iSyncSession;

	__ASSERT_ALWAYS(iSyncSession, TUtil::Panic(KErrArgument));
    }


// -----------------------------------------------------------------------------
// CAspLogDialog::ConstructL
//
// -----------------------------------------------------------------------------
//
void CAspLogDialog::ConstructL()
    {
    iResHandler = CAspResHandler::NewL();
    iList = new (ELeave) CDesCArrayFlat(1);
   		
    TAspParam param(KErrNotFound, iSyncSession);
	CAspProfile* profile = CAspProfile::NewLC(param);   		
	profile->OpenL(iProfileId, CAspProfile::EOpenRead, CAspProfile::EAllProperties);
	
    TAspParam param2(KErrNotFound, iSyncSession);
	param2.iProfile = profile;
	param2.iMode = CAspContentList::EInitAll;
	CAspContentList* taskList = CAspContentList::NewLC(param2);
	
	CreateListL(iList, profile, taskList);
	
	CleanupStack::PopAndDestroy(taskList);
	CleanupStack::PopAndDestroy(profile);
    } 


// ----------------------------------------------------------------------------
// Destructor
//
// ----------------------------------------------------------------------------
//
CAspLogDialog::~CAspLogDialog()
    {
	delete iResHandler;
    delete iList;
    }


// -----------------------------------------------------------------------------
// CAspLogDialog::CreateListL
// 
// Example log format:
//
// Profile: PC Suite
// Server: PC Suite
// Date: 13/10/2004
// Time: 11:04
// Status: Complete
//
// Phone:  
//
// Added Contacts:
//  4
// Added Notes:
//  12
// Deleted Calendar:
//  2
//
// Server:
//
// Added contacts:
//  12
// 
// -----------------------------------------------------------------------------
void CAspLogDialog::CreateListL(CDesCArray* aList, CAspProfile* aProfile,
                                CAspContentList* aTaskList)
	{
	_LIT(KTab, "\t");
	
	aList->Reset();
	
	const CSyncMLHistoryJob* historyJob = aProfile->LatestHistoryJob();
   	if (!historyJob)
   		{
   		User::Leave(KErrNotFound);  // this profile has not been synced
   		}

		
	TTime time = historyJob->TimeStamp();
	
	CAspResHandler::ReadL(iBuf, R_QTN_SML_LOG_PROFILE); //profile name heading
	iBuf.Append(KTab);

#ifdef RD_DSUI_TIMEDSYNC
	 CAspSchedule* schedule = CAspSchedule::NewLC();
	 TInt asProfileId = schedule->AutoSyncProfileId();
	 if (asProfileId == aProfile->ProfileId())
		{
		TInt profileId = schedule->ProfileId();
		TAspParam param(KErrNotFound, iSyncSession);
		CAspProfile* profile = CAspProfile::NewLC(param);
		profile->OpenL(profileId, CAspProfile::EOpenRead, CAspProfile::EAllProperties);

		profile->GetName(iBuf2);
		CleanupStack::PopAndDestroy(profile);
		}
	 else
		{
		aProfile->GetName(iBuf2);
		}
	 CleanupStack::PopAndDestroy(schedule);
#else	 
	aProfile->GetName(iBuf2);
#endif
	iBuf.Append(iBuf2);
	AddLineL(aList, iBuf);
    
	CAspResHandler::ReadL(iBuf, R_QTN_SML_LOG_REMOTE); //server name heading
	iBuf.Append(KTab);

	if(aProfile->IsPCSuiteProfile(aProfile))
	{
		aProfile->GetLocalisedPCSuite(iBuf2);
	}
	else
	{
		TInt num = 0;
		aProfile->GetHostAddress(iBuf2, num);
	}	
	iBuf.Append(iBuf2);
	AddLineL(aList, iBuf);
		
    CAspResHandler::ReadL(iBuf, R_QTN_SML_LOG_DATE); // sync date heading
	iBuf.Append(KTab);
	TUtil::GetDateTextL(iBuf2, time);
	iBuf.Append(iBuf2);
	AddLineL(aList, iBuf);

	CAspResHandler::ReadL(iBuf, R_QTN_SML_LOG_TIME); // sync time heading
	iBuf.Append(KTab);
    TUtil::GetTimeTextL(iBuf2, time);
	iBuf.Append(iBuf2);
	AddLineL(aList, iBuf);	
		

	//
	// check sync errors
	//
	CAspResHandler::ReadL(iBuf, R_QTN_SML_LOG_STATUS); // status heading
	iBuf.Append(KTab);	
		
	TInt syncError = historyJob->ResultCode();
	if (syncError != KErrNone && syncError != SyncMLError::KErrSlowSync)
		{
		if (syncError == KErrCancel)
			{
			CAspResHandler::ReadL(iBuf2, R_QTN_SML_LOG_CANCELLED);
			}
		else
			{
			CAspResHandler::ReadL(iBuf2, R_QTN_SML_LOG_ERR);
			}
		
		iBuf.Append(iBuf2);
		AddLineL(aList, iBuf);
		
		// Added for syncml engine error handling and displaying in view logs
		if (syncError <= SyncMLError::KErrSyncEngineErrorBase)
			{
			TInt id = TAspSyncError::GetSmlErrorValue (syncError) ;
			CAspResHandler::ReadL(iBuf, R_QTN_SML_LOG_ERROR); // sync error heading
			iBuf.Append(KTab);
			CAspResHandler::ReadL(iBuf2, id);
			iBuf.Append(iBuf2);
			AddLineL(aList, iBuf);
			}
		
		return;
		}
	
	if (HasTaskErrors(historyJob))
		{
		CAspResHandler::ReadL(iBuf2, R_QTN_SML_LOG_ERR);
		iBuf.Append(iBuf2);
    	AddLineL(aList, iBuf);
		}
    else
    	{
    	CAspResHandler::ReadL(iBuf2, R_QTN_SML_LOG_OK);
	    iBuf.Append(iBuf2);
    	AddLineL(aList, iBuf);
    	}
		
	
	if (PhoneModified(historyJob))
		{
		CAspResHandler::ReadL(iBuf, R_QTN_SML_LOG_PHONE);
	    iBuf.Append(KTab);
   	    AddLineL(aList, iBuf);

        AddLogItemsL(aList, historyJob, aTaskList, ELogAddedPhone);
        AddLogItemsL(aList, historyJob, aTaskList, ELogUpdatedPhone);
        AddLogItemsL(aList, historyJob, aTaskList, ELogDeletedPhone);
        AddLogItemsL(aList, historyJob, aTaskList, ELogDiscardedPhone);
        AddLogItemsL(aList, historyJob, aTaskList, ELogMovedPhone);
		}

	
	if (ServerModified(historyJob))
		{
		CAspResHandler::ReadL(iBuf, R_QTN_SML_LOG_SERVER);
	    iBuf.Append(KTab);
   	    AddLineL(aList, iBuf);

        AddLogItemsL(aList, historyJob, aTaskList, ELogAddedServer);
        AddLogItemsL(aList, historyJob, aTaskList, ELogUpdatedServer);
        AddLogItemsL(aList, historyJob, aTaskList, ELogDeletedServer);
        AddLogItemsL(aList, historyJob, aTaskList, ELogDiscardedServer);
        AddLogItemsL(aList, historyJob, aTaskList, ELogMovedServer);
		}

	if (HasTaskErrors(historyJob))
		{
		CAspResHandler::ReadL(iBuf, R_QTN_SML_LOG_ERROR);
	    iBuf.Append(KTab);
   	    AddLineL(aList, iBuf);

        AddTaskErrorsL(aList, historyJob, aTaskList);
		}
	}


// -----------------------------------------------------------------------------
// CAspLogDialog::AddLogItemsL
// 
// -----------------------------------------------------------------------------
void CAspLogDialog::AddLogItemsL(CDesCArray* aList, const CSyncMLHistoryJob* aHistoryJob, 
                                 CAspContentList* aTaskList, TInt aType)
	{
	_LIT(KFormat, "%S\t%d");
	
	TInt taskCount = aHistoryJob->TaskCount();
	for (TInt i=0; i<taskCount; i++)
		{
		const CSyncMLHistoryJob::TTaskInfo& taskInfo = aHistoryJob->TaskAt(i);
        	
        TInt index = aTaskList->FindProviderIndexForTask(taskInfo.iTaskId);
        if (index == KErrNotFound)
    	    {
       	    continue; // profile no longer has this task
    	    }
       	TAspProviderItem& provider = aTaskList->ProviderItem(index);
     
     
		TInt count = SyncedItemCount(taskInfo, aType);
		if (count == 0)
			{
			continue;
			}

		HBufC* hBuf = LogItemTextLC(aType, provider.iDataProviderId, provider.iDisplayName);
	    TPtr ptr = hBuf->Des();
	    iBuf.Format(KFormat, &ptr, count);
	    CleanupStack::PopAndDestroy(hBuf);
	    
       	AddLineL(aList, iBuf);
		}
	}


// -----------------------------------------------------------------------------
// CAspLogDialog::AddTaskErrorsL
// 
// -----------------------------------------------------------------------------
void CAspLogDialog::AddTaskErrorsL(CDesCArray* aList, 
                                   const CSyncMLHistoryJob* aHistoryJob, 
                                   CAspContentList* aTaskList)
	{
	_LIT(KTab, "\t");
	
	TInt taskCount = aHistoryJob->TaskCount();
	for (TInt i=0; i<taskCount; i++)
		{
		const CSyncMLHistoryJob::TTaskInfo& taskInfo = aHistoryJob->TaskAt(i);
        	
        TInt index = aTaskList->FindProviderIndexForTask(taskInfo.iTaskId);
        if (index == KErrNotFound)
    	    {
       	    continue; // profile no longer has this task
    	    }
    
    	TAspProviderItem& provider = aTaskList->ProviderItem(index);
  	
		if (taskInfo.iError != KErrNone && taskInfo.iError != SyncMLError::KErrSlowSync)
		    {
	    	iBuf = KNullDesC;
	    	HBufC* hBuf = CAspResHandler::GetContentNameLC(provider.iDataProviderId,
	    	                                               provider.iDisplayName);
	        iBuf.Append(hBuf->Des());
	        CleanupStack::PopAndDestroy(hBuf);

		    CAspResHandler::ReadL(iBuf2, R_QTN_SML_LOG_ERR); // "Incomplete"
		    iBuf.Append(KTab);
     	    iBuf.Append(iBuf2);
     	    
#ifdef _DEBUG 
       	    TBuf<64> buf;
     	    buf.Format(_L(" (%d)"), taskInfo.iError);
     	    iBuf.Append(buf);
#endif
     	    
     	    AddLineL(aList, iBuf);
    		}
		}
	}


// -----------------------------------------------------------------------------
// CAspLogDialog::PhoneModified
// 
// -----------------------------------------------------------------------------
TBool CAspLogDialog::PhoneModified(const CSyncMLHistoryJob* aHistoryJob)
	{
	TInt taskCount = aHistoryJob->TaskCount();
	for (TInt i=0; i<taskCount; i++)
		{
		const CSyncMLHistoryJob::TTaskInfo& task = aHistoryJob->TaskAt(i);
	    
	    TInt count = task.iServerItemsAdded + task.iServerItemsChanged +
	                 task.iServerItemsDeleted + task.iServerItemsFailed +
	                 task.iServerItemsMoved;
        
        if (count > 0)
        	{
        	return ETrue;
        	}
		}
		
	return EFalse;
	}


// -----------------------------------------------------------------------------
// CAspLogDialog::ServerModified
// 
// -----------------------------------------------------------------------------
TBool CAspLogDialog::ServerModified(const CSyncMLHistoryJob* aHistoryJob)
	{
	TInt taskCount = aHistoryJob->TaskCount();
	for (TInt i=0; i<taskCount; i++)
		{
		const CSyncMLHistoryJob::TTaskInfo& task = aHistoryJob->TaskAt(i);

	    TInt count = task.iItemsAdded + task.iItemsChanged +
	                 task.iItemsDeleted + task.iItemsFailed +
	                 task.iItemsMoved;
        
        if (count > 0)
        	{
        	return ETrue;
        	}
		}
		
	return EFalse;
	}


// -----------------------------------------------------------------------------
// CAspLogDialog::HasTaskErrors
// 
// -----------------------------------------------------------------------------
TBool CAspLogDialog::HasTaskErrors(const CSyncMLHistoryJob* aHistoryJob)
	{
	TInt taskCount = aHistoryJob->TaskCount();
	for (TInt i=0; i<taskCount; i++)
		{
		const CSyncMLHistoryJob::TTaskInfo& taskInfo = aHistoryJob->TaskAt(i);
        
        if (taskInfo.iError != KErrNone && taskInfo.iError != SyncMLError::KErrSlowSync)
        	{
        	FLOG(_L("### CAspLogDialog::HasTaskErrors (%d) ###"), taskInfo.iError);
        	return ETrue;
        	}
		}
		
	return EFalse;
	}


// -----------------------------------------------------------------------------
// CAspLogDialog::AddLineL
// 
// -----------------------------------------------------------------------------
void CAspLogDialog::AddLineL(CDesCArray* aList, TDes& aText)
	{
	AknTextUtils::DisplayTextLanguageSpecificNumberConversion(aText);
	aList->AppendL(aText);
	}


// -----------------------------------------------------------------------------
// CAspLogDialog::SyncedItemCount
// 
// -----------------------------------------------------------------------------
TInt CAspLogDialog::SyncedItemCount(const CSyncMLHistoryJob::TTaskInfo& aTaskInfo, TInt aType)
	{
	TInt count = 0;

	switch (aType)
		{
		case ELogAddedPhone:
			count = aTaskInfo.iServerItemsAdded;
			break;
		case ELogUpdatedPhone:
			count = aTaskInfo.iServerItemsChanged;
			break;
		case ELogDeletedPhone:
			count = aTaskInfo.iServerItemsDeleted;
			break;
		case ELogDiscardedPhone:
			count = aTaskInfo.iServerItemsFailed;
			break;
		case ELogMovedPhone:
			count = aTaskInfo.iServerItemsMoved;
			break;

		case ELogAddedServer:
			count = aTaskInfo.iItemsAdded;
			break;
		case ELogUpdatedServer:
			count = aTaskInfo.iItemsChanged;
			break;
		case ELogDeletedServer:
			count = aTaskInfo.iItemsDeleted;
			break;
		case ELogDiscardedServer:
			count = aTaskInfo.iItemsFailed;
			break;
		case ELogMovedServer:
			count = aTaskInfo.iItemsMoved;
			break;

		default:
			break;
		}

	return count;
	}


// -----------------------------------------------------------------------------
// CAspLogDialog::LogItemTextLC
// 
// -----------------------------------------------------------------------------
HBufC* CAspLogDialog::LogItemTextLC(TInt aType, TInt aDataProviderId, const TDesC& aDisplayName)
	{
	TInt resourceId = KErrNotFound;
	
	if (aDataProviderId == KUidNSmlAdapterContact.iUid)
		{
		if (aType == ELogAddedPhone || aType == ELogAddedServer)
			{
			resourceId = R_QTN_SML_LOG_ADDED_CONTACTS;
			}
		if (aType == ELogUpdatedPhone || aType == ELogUpdatedServer)
			{
			resourceId = R_QTN_SML_LOG_UPDATED_CONTACTS;
			}
		if (aType == ELogDeletedPhone || aType == ELogDeletedServer)
			{
			resourceId = R_QTN_SML_LOG_DELETED_CONTACTS;
			}
		if (aType == ELogDiscardedPhone || aType == ELogDiscardedServer)
			{
			resourceId = R_QTN_SML_LOG_DISCARDED_CONTACTS;
			}
		if (aType == ELogMovedPhone || aType == ELogMovedServer)
			{
			resourceId = R_QTN_SML_LOG_MOVED_CONTACTS;
			}
		}
	else if (aDataProviderId == KUidNSmlAdapterCalendar.iUid)
		{
		if (aType == ELogAddedPhone || aType == ELogAddedServer)
			{
			resourceId = R_QTN_SML_LOG_ADDED_CALENDAR;
			}
		if (aType == ELogUpdatedPhone || aType == ELogUpdatedServer)
			{
			resourceId = R_QTN_SML_LOG_UPDATED_CALENDAR;
			}
		if (aType == ELogDeletedPhone || aType == ELogDeletedServer)
			{
			resourceId = R_QTN_SML_LOG_DELETED_CALENDAR;
			}
		if (aType == ELogDiscardedPhone || aType == ELogDiscardedServer)
			{
			resourceId = R_QTN_SML_LOG_DISCARDED_CALENDAR;
			}
		if (aType == ELogMovedPhone || aType == ELogMovedServer)
			{
			resourceId = R_QTN_SML_LOG_MOVED_CALENDAR;
			}
		}
	else if (aDataProviderId == KUidNSmlAdapterEMail.iUid)
		{
		if (aType == ELogAddedPhone || aType == ELogAddedServer)
			{
			resourceId = R_QTN_SML_LOG_ADDED_EMAIL;
			}
		if (aType == ELogUpdatedPhone || aType == ELogUpdatedServer)
			{
			resourceId = R_QTN_SML_LOG_UPDATED_EMAIL;
			}
		if (aType == ELogDeletedPhone || aType == ELogDeletedServer)
			{
			resourceId = R_QTN_SML_LOG_DELETED_EMAIL;
			}
		if (aType == ELogDiscardedPhone || aType == ELogDiscardedServer)
			{
			resourceId = R_QTN_SML_LOG_DISCARDED_EMAIL;
			}
		if (aType == ELogMovedPhone || aType == ELogMovedServer)
			{
			resourceId = R_QTN_SML_LOG_MOVED_EMAIL;
			}
		}
	else if (aDataProviderId == KUidNSmlAdapterNote.iUid)
		{
		if (aType == ELogAddedPhone || aType == ELogAddedServer)
			{
			resourceId = R_QTN_SML_LOG_ADDED_NOTES;
			}
		if (aType == ELogUpdatedPhone || aType == ELogUpdatedServer)
			{
			resourceId = R_QTN_SML_LOG_UPDATED_NOTES;
			}
		if (aType == ELogDeletedPhone || aType == ELogDeletedServer)
			{
			resourceId = R_QTN_SML_LOG_DELETED_NOTES;
			}
		if (aType == ELogDiscardedPhone || aType == ELogDiscardedServer)
			{
			resourceId = R_QTN_SML_LOG_DISCARDED_NOTES;
			}
		if (aType == ELogMovedPhone || aType == ELogMovedServer)
			{
			resourceId = R_QTN_SML_LOG_MOVED_NOTES;
			}
		}
	else if (aDataProviderId == KUidNSmlAdapterSms.iUid)
		{
		if (aType == ELogAddedPhone || aType == ELogAddedServer)
			{
			resourceId = R_QTN_SML_LOG_ADDED_SMS;
			}
		if (aType == ELogUpdatedPhone || aType == ELogUpdatedServer)
			{
			resourceId = R_QTN_SML_LOG_UPDATED_SMS;
			}
		if (aType == ELogDeletedPhone || aType == ELogDeletedServer)
			{
			resourceId = R_QTN_SML_LOG_DELETED_SMS;
			}
		if (aType == ELogDiscardedPhone || aType == ELogDiscardedServer)
			{
			resourceId = R_QTN_SML_LOG_DISCARDED_SMS;
			}
		if (aType == ELogMovedPhone || aType == ELogMovedServer)
			{
			resourceId = R_QTN_SML_LOG_MOVED_SMS;
			}
		}
	else if (aDataProviderId == KUidNSmlAdapterMMS.iUid)
		{
		if (aType == ELogAddedPhone || aType == ELogAddedServer)
			{
			resourceId = R_QTN_SML_LOG_ADDED_MMS;
			}
		if (aType == ELogUpdatedPhone || aType == ELogUpdatedServer)
			{
			resourceId = R_QTN_SML_LOG_UPDATED_MMS;
			}
		if (aType == ELogDeletedPhone || aType == ELogDeletedServer)
			{
			resourceId = R_QTN_SML_LOG_DELETED_MMS;
			}
		if (aType == ELogDiscardedPhone || aType == ELogDiscardedServer)
			{
			resourceId = R_QTN_SML_LOG_DISCARDED_MMS;
			}
		if (aType == ELogMovedPhone || aType == ELogMovedServer)
			{
			resourceId = R_QTN_SML_LOG_MOVED_MMS;
			}
		}

	else if (aDataProviderId == KUidNSmlAdapterBookmarks.iUid)
		{
		if (aType == ELogAddedPhone || aType == ELogAddedServer)
			{
			resourceId = R_QTN_SML_LOG_ADDED_BKM;
			}
		if (aType == ELogUpdatedPhone || aType == ELogUpdatedServer)
			{
			resourceId = R_QTN_SML_LOG_UPDATED_BKM;
			}
		if (aType == ELogDeletedPhone || aType == ELogDeletedServer)
			{
			resourceId = R_QTN_SML_LOG_DELETED_BKM;
			}
		if (aType == ELogDiscardedPhone || aType == ELogDiscardedServer)
			{
			resourceId = R_QTN_SML_LOG_DISCARDED_BKM;
			}
		if (aType == ELogMovedPhone || aType == ELogMovedServer)
			{
			resourceId = R_QTN_SML_LOG_MOVED_BKM;
			}
		}
	
	else  // unknown data provider
		{
		if (aType == ELogAddedPhone || aType == ELogAddedServer)
			{
			resourceId = R_QTN_SML_LOG_ADDED_CONTENT;
			}
		if (aType == ELogUpdatedPhone || aType == ELogUpdatedServer)
			{
			resourceId = R_QTN_SML_LOG_UPDATED_CONTENT;
			}
		if (aType == ELogDeletedPhone || aType == ELogDeletedServer)
			{
			resourceId = R_QTN_SML_LOG_DELETED_CONTENT;
			}
		if (aType == ELogDiscardedPhone || aType == ELogDiscardedServer)
			{
			resourceId = R_QTN_SML_LOG_DISCARDED_CONTENT;
			}
		if (aType == ELogMovedPhone || aType == ELogMovedServer)
			{
			resourceId = R_QTN_SML_LOG_MOVED_CONTENT;
			}

        __ASSERT_DEBUG(resourceId != KErrNotFound, TUtil::Panic(KErrGeneral));
        
		return CAspResHandler::ReadLC(resourceId, aDisplayName);
		}
	
	__ASSERT_DEBUG(resourceId != KErrNotFound, TUtil::Panic(KErrGeneral));
	
    return CAspResHandler::ReadLC(resourceId);
	}






//  End of File  
