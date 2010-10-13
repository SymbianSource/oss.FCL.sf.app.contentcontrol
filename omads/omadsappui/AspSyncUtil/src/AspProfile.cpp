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



// INCLUDES

#include <SyncMLTransportProperties.h>
#include <featmgr.h>   // FeatureManager
#include <centralrepository.h> // CRepository
#include <NsmlOperatorDataCRKeys.h> // KCRUidOperatorDatasyncInternalKeys

#include <calsession.h>
#include <calcalendarinfo.h>
#include <e32math.h>
#include <gdi.h>

#include "AspProfile.h"
#include "AspResHandler.h"
#include "AspDialogUtil.h"
#include "AspDebug.h"
#include "AspSchedule.h"
#include "AspDefines.h"
#include <centralrepository.h> //CRepository
#include <calenmulticaluids.hrh> // Calendar File Meta Data Properties
#include <calenmulticalutil.h>
#include <CalenInterimUtils2.h>

_LIT(KDrive ,"C:");
_LIT(KCharUnderscore, "_");

/*******************************************************************************
 * class TAspProviderItem
 *******************************************************************************/


// -----------------------------------------------------------------------------
// TAspProviderItem::CompareItems
//
// -----------------------------------------------------------------------------
//
TInt TAspProviderItem::CompareItems(const TAspProviderItem& aFirst, const TAspProviderItem& aSecond)
	{
	const TInt KNegative = -1;
	const TInt KPositive = 1;
	
	TInt firstOrder = SortOrder(aFirst.iDataProviderId);
	TInt secondOrder = SortOrder(aSecond.iDataProviderId);
	
	if (firstOrder > secondOrder)
		{
		return KNegative;
		}
	else if (secondOrder > firstOrder)
		{
		return KPositive;
		}
	else
		{
		return 0;
		}
	}


// -----------------------------------------------------------------------------
// TAspProviderItem::CompareItems
//
// -----------------------------------------------------------------------------
//
TInt TAspProviderItem::SortOrder(TInt aDataProviderId)
	{
	TInt ret = EOrderOther;
	if (aDataProviderId == KUidNSmlAdapterContact.iUid)
		{
		ret = EOrderContact;
		}
	else if (aDataProviderId == KUidNSmlAdapterCalendar.iUid)
		{
		ret = EOrderCalendar;
		}
	else if (aDataProviderId == KUidNSmlAdapterNote.iUid)
		{
		ret = EOrderNotes;
		}
	else if (aDataProviderId == KUidNSmlAdapterEMail.iUid)
		{
		ret = EOrderEmail;
		}
	return ret;
	}
	
	

/*******************************************************************************
 * class CAspContentList
 *******************************************************************************/



// -----------------------------------------------------------------------------
// CAspContentList::NewLC
//
// -----------------------------------------------------------------------------
//
CAspContentList* CAspContentList::NewLC(const TAspParam& aParam)
    {
   	FLOG( _L("CAspContentList::NewLC START") );
   	
    CAspContentList* self = new (ELeave) CAspContentList(aParam);
	CleanupStack::PushL(self);
	self->ConstructL();

    FLOG( _L("CAspContentList::NewLC END") );
	return self;
    }


// -----------------------------------------------------------------------------
// CAspContentList::NewL
//
// -----------------------------------------------------------------------------
//
CAspContentList* CAspContentList::NewL(const TAspParam& aParam)
    {
   	FLOG( _L("CAspContentList::NewL START") );
   	
    CAspContentList* self = new (ELeave) CAspContentList(aParam);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);

    FLOG( _L("CAspContentList::NewL END") );
	return self;
    }


// -----------------------------------------------------------------------------
// Destructor
//
// -----------------------------------------------------------------------------
//
CAspContentList::~CAspContentList()
    {
	iProviderList.Close();
	iTaskList.Close();
    }


// -----------------------------------------------------------------------------
// CAspContentList::ConstructL
//
// -----------------------------------------------------------------------------
//
void CAspContentList::ConstructL(void)
    {
	if (iListMode == EInitDataProviders)
		{
        InitDataProvidersL();
		}
	else if (iListMode == EInitTasks)
		{
		__ASSERT_ALWAYS(iProfile, TUtil::Panic(KErrGeneral));
		
        InitAllTasksL();
		}
	else 
		{
		__ASSERT_ALWAYS(iProfile, TUtil::Panic(KErrGeneral));
		
		InitDataProvidersL();
		InitAllTasksL();
		}
    }


// -----------------------------------------------------------------------------
// CAspContentList::CAspContentList
//
// -----------------------------------------------------------------------------
//
CAspContentList::CAspContentList(const TAspParam& aParam)
	{
	iSyncSession = aParam.iSyncSession;
	iProfile = aParam.iProfile;
	iListMode = aParam.iMode;
	iApplicationId = aParam.iApplicationId;

	__ASSERT_ALWAYS(iSyncSession, TUtil::Panic(KErrGeneral));
	}


// -----------------------------------------------------------------------------
// CAspContentList::SetProfile
//
// -----------------------------------------------------------------------------
//
void CAspContentList::SetProfile(CAspProfile* aProfile)
	{
	if (aProfile)
		{

	    __ASSERT_ALWAYS(iProfile == NULL, TUtil::Panic(KErrGeneral));

		}

	iProfile = aProfile;
	}


// -----------------------------------------------------------------------------
// CAspContentList::IsLocked
//
// -----------------------------------------------------------------------------
//
TBool CAspContentList::IsLocked()
	{
	if (iProfile)
		{
		return ETrue;
		}
		
	return EFalse;
	}


// -----------------------------------------------------------------------------
// CAspContentList::ProviderCount
//
// -----------------------------------------------------------------------------
//
TInt CAspContentList::ProviderCount()
	{
	return iProviderList.Count();
	}


// -----------------------------------------------------------------------------
// CAspContentList::ProviderItem
// 
// -----------------------------------------------------------------------------
//
TAspProviderItem& CAspContentList::ProviderItem(TInt aIndex)
	{
	if (aIndex < 0 || aIndex >= ProviderCount())
	    {
		TUtil::Panic(KErrArgument);
	    }

	return iProviderList[aIndex];
	}


// -----------------------------------------------------------------------------
// CAspContentList::FindProviderIndex
//
// -----------------------------------------------------------------------------
//
TInt CAspContentList::FindProviderIndex(TInt aId)
	{
	TInt count = iProviderList.Count();
	
	for (TInt i=0; i<count; i++)
		{
		TAspProviderItem& item = iProviderList[i];
		if (item.iDataProviderId == aId)
			{
			return i;
			}
		}
		
	return KErrNotFound;
	}


// -----------------------------------------------------------------------------
// CAspContentList::FindProviderIndexForTask
//
// -----------------------------------------------------------------------------
//
TInt CAspContentList::FindProviderIndexForTask(TInt aId)
	{
	TInt index = FindTaskIndex(aId);
	if (index == KErrNotFound)
		{
		return KErrNotFound;
		}
	
	TAspTaskItem& item = TaskItem(index);
	
	return FindProviderIndex(item.iDataProviderId);
	}


// -----------------------------------------------------------------------------
// CAspContentList::TaskCount
//
// -----------------------------------------------------------------------------
//
TInt CAspContentList::TaskCount()
	{
	return iTaskList.Count();
	}


// -----------------------------------------------------------------------------
// CAspContentList::TaskItem
// 
// -----------------------------------------------------------------------------
//
TAspTaskItem& CAspContentList::TaskItem(TInt aIndex)
	{
	if ((aIndex < 0) || (aIndex >= TaskCount()))
	    {
		TUtil::Panic(KErrArgument);
	    }

	return iTaskList[aIndex];
	}


// -----------------------------------------------------------------------------
// CAspContentList::FindTaskIndex
//
// -----------------------------------------------------------------------------
//
TInt CAspContentList::FindTaskIndex(TInt aId)
	{
	TInt count = iTaskList.Count();
	
	for (TInt i=0; i<count; i++)
		{
		TAspTaskItem& task = iTaskList[i];
		if (task.iTaskId == aId)
			{
			return i;
			}
		}
		
	return KErrNotFound;
	}


// -----------------------------------------------------------------------------
// CAspContentList::FindTaskIndexForProvider
//
// -----------------------------------------------------------------------------
//
TInt CAspContentList::FindTaskIndexForProvider(TInt aId)
	{
	TInt count = iTaskList.Count();
	
	for (TInt i=0; i<count; i++)
		{
		TAspTaskItem& item = iTaskList[i];
		if (item.iDataProviderId == aId)
			{
			return i;
			}
		}
		
	return KErrNotFound;
	}


// -----------------------------------------------------------------------------
// CAspContentList::FindTaskIdForProvider
//
// -----------------------------------------------------------------------------
//
TInt CAspContentList::FindTaskIdForProvider(TInt aId)
	{
	TInt index = FindTaskIndexForProvider(aId);
	if (index == KErrNotFound)
		{
		return KErrNotFound;
		}
		
	TAspTaskItem& item = iTaskList[index];
	return item.iTaskId;
	}


// -----------------------------------------------------------------------------
// CAspContentList::DoInitDataProvidersL
// 
// Initialize data provider list.
// -----------------------------------------------------------------------------
//
void CAspContentList::DoInitDataProvidersL()
	{
	FLOG( _L("CAspContentList::DoInitDataProvidersL START") );
	
	iProviderList.Reset();
	
	RArray<TSmlDataProviderId> arr;
	
	Session().ListDataProvidersL(arr);
	CleanupClosePushL(arr);

	TBool operatorProfile = CAspProfile::IsOperatorProfileL(iProfile);
	TInt operatorUid = CAspProfile::OperatorAdapterUidL();
	TInt profileUid = CAspProfile::ProfileAdapterUidL();
	
	TInt count = arr.Count();
	for (TInt i=0; i<count; i++)
		{
		RSyncMLDataProvider provider;
		TInt id = arr[i];
		
		TRAPD(err, provider.OpenL(Session(), id));
	
		if (err == KErrNone)
			{
    		CleanupClosePushL(provider);

	    	TAspProviderItem item;
		    ReadDataProviderItemL(provider, item);
		    if( (item.iDataProviderId == KUidNsmlAdapterCntGrps.iUid) ||
		             (item.iDataProviderId == KUidNSmlAdapterSms.iUid) || 
		             (item.iDataProviderId == KUidNSmlAdapterMMS.iUid) ||
		             (item.iDataProviderId == KUidNsmlAdapterMedia.iUid))
		        {
		          CleanupStack::PopAndDestroy(&provider);
		          continue;
		        }
            item.iIsIncludedInSync = EFalse;
		    
		    if (TUtil::IsEmpty(item.iDisplayName))
		    	{
		    	// providers must have display name
		    	CleanupStack::PopAndDestroy(&provider);
		    	FTRACE( RDebug::Print(_L("### provider has no display name (id=%x) ###"), item.iDataProviderId) );
		    	continue;
		    	}

		    if ( iApplicationId == 0 )
		        {
                if( operatorUid != KUidNSmlAdapterContact.iUid &&
                    operatorUid != 0 )
                    {
                    // Dedicated operator specific adapter in use
                    if ( operatorProfile )
                        {
                        // Operator profile: show other adapters than platform's 
                        //  contacts adapter
                        if ( id != KUidNSmlAdapterContact.iUid )
                            {
                            User::LeaveIfError(iProviderList.Append(item)); 
                            }
                        }
                    else
                        {
                        // Non-operator profile: show other adapters than
                        //  operator specific contacts adapter & profile adapter
                        if ( id != operatorUid && id != profileUid )
                            {
                            User::LeaveIfError(iProviderList.Append(item));
                            }
                        }
                    }
                else
                    {
                    // Platform's contacts adapter used as operator adapter
                    if( operatorProfile || 
                        ( !operatorProfile && id != profileUid ) )
                        {
                        User::LeaveIfError(iProviderList.Append(item));
                        }
                    }
                }
		    else
		        {
                User::LeaveIfError(iProviderList.Append(item));
		        }
            
	        CleanupStack::PopAndDestroy(&provider);
		    }
		    
		if (err != KErrNone)
			{
			FTRACE( RDebug::Print(_L("### RSyncMLDataProvider::Open failed (id=%x, err=%d) ###"), id, err) );
			}
		
		}
		
	CleanupStack::PopAndDestroy(&arr);
	
	FLOG( _L("CAspContentList::DoInitDataProvidersL END") );
	}


// -----------------------------------------------------------------------------
// CAspContentList::UpdateDataProviderL
// 
// -----------------------------------------------------------------------------
//
void CAspContentList::UpdateDataProviderL(TInt aDataProviderId)
	{
	TInt index = FindProviderIndex(aDataProviderId);
	if (index == KErrNotFound)
		{
		return;  // unknown provider
		}
	TAspProviderItem& providerItem = ProviderItem(index);
		
	RArray<TSmlDataProviderId> arr;
	Session().ListDataProvidersL(arr);
	CleanupClosePushL(arr);

	TInt count = arr.Count();
	for (TInt i=0; i<count; i++)
		{
		RSyncMLDataProvider provider;
		TInt id = arr[i];
		
		TRAPD(err, provider.OpenL(Session(), id));
	
		if (err == KErrNone)
			{
    		CleanupClosePushL(provider);

		    TInt id = provider.Identifier();
		    if (id == aDataProviderId)
		    	{
       		    ReadDataProviderItemL(provider, providerItem);
       		    CleanupStack::PopAndDestroy(&provider);
       		    break;
		    	}
       
	        CleanupStack::PopAndDestroy(&provider);
		    }
		}
		
	CleanupStack::PopAndDestroy(&arr);
	}


// -----------------------------------------------------------------------------
// CAspContentList::ReadDataProviderItemL
// 
// -----------------------------------------------------------------------------
//
void CAspContentList::ReadDataProviderItemL(RSyncMLDataProvider& aProvider, 
                                            TAspProviderItem& aItem)
	{
    aItem.iTaskId=KErrNotFound;
	aItem.iDataProviderId = aProvider.Identifier();
		    
    aItem.iSyncProtocol = ESmlVersion1_2;
    //item.iSyncProtocol = provider.RequiredProtocolVersionL();
		    
		    
    aItem.iAllowMultipleDataStores = aProvider.AllowsMultipleDataStores();
    TUtil::StrCopy(aItem.iDisplayName, aProvider.DisplayName());
	
    TPtrC ptr = aProvider.DefaultDataStoreName();
    if (TUtil::IsEmpty(ptr))
    	{
    	aItem.iHasDefaultDataStore = EFalse;
    	aItem.iDefaultDataStore = KNullDesC;
    	}
    else
    	{
    	aItem.iHasDefaultDataStore = ETrue;
    	TUtil::StrCopy(aItem.iDefaultDataStore, ptr);
    	}
	}


// -----------------------------------------------------------------------------
// CAspContentList::InitDataProvidersL
// 
// Initialize data provider list.
// -----------------------------------------------------------------------------
//
void CAspContentList::InitDataProvidersL()
	{

#ifdef _DEBUG
   	TTime time_1 = TUtil::TimeBefore();
#endif

    DoInitDataProvidersL();
    Sort();
    
#ifdef _DEBUG
   	TUtil::TimeAfter(time_1, _L("CAspContentList::DoInitDataProvidersL took"));
#endif


#ifndef __SYNCML_DS_EMAIL   // KFeatureIdSyncMlDsEmail
    RemoveDataProvider(KUidNSmlAdapterEMail.iUid);
#endif

	}
	
	
// -----------------------------------------------------------------------------
// CAspContentList::RemoveDataProvider
//
// -----------------------------------------------------------------------------
//
void CAspContentList::RemoveDataProvider(TInt aDataProviderId)
	{
    TInt index = FindProviderIndex(aDataProviderId);
        
    // remove data provider with id aDataProviderId
    if (index != KErrNotFound)
    	{
    	iProviderList.Remove(index);
    	}
	}


// -----------------------------------------------------------------------------
// CAspContentList::InitAllTasksL
//
// Initialize task list.
// -----------------------------------------------------------------------------
//
void CAspContentList::InitAllTasksL()
	{
	FLOG( _L("CAspContentList::InitAllTasksL START") );
	
	__ASSERT_ALWAYS(iProfile, TUtil::Panic(KErrGeneral));
	
	iTaskList.Reset();
	
	RArray<TSmlTaskId> arr;
    
	iProfile->Profile().ListTasksL(arr); // IPC call
	CleanupClosePushL(arr);
	
	TInt count = arr.Count();
	for (TInt i=0; i<count; i++)
		{
		TAspTaskItem item = ReadTaskItemL(iProfile->Profile(), arr[i]);
		
		item.iLastSync = iProfile->LastSync(item.iTaskId);
		
    	User::LeaveIfError(iTaskList.Append(item));
		}
		
	
    CleanupStack::PopAndDestroy(&arr);
    
    FLOG( _L("CAspContentList::InitAllTasksL END") );
	}

// -----------------------------------------------------------------------------
// CAspContentList::UpdateLocalDatabaseL()
//
// -----------------------------------------------------------------------------
//
void CAspContentList::UpdateLocalDatabaseL()
{
	InitDataProvidersL();
	InitAllTasksL();
	RSyncMLDataProvider provider;
	TRAPD(err, provider.OpenL(Session(), KUidNSmlAdapterEMail.iUid));
	
	if (err == KErrNone)
		{
    	CleanupClosePushL(provider);
    	TAspProviderItem item;
		ReadDataProviderItemL(provider, item);
		}
	TInt index = FindTaskIdForProvider( KUidNSmlAdapterEMail.iUid);
	if (index != KErrNotFound)
		{
		TAspTaskItem emailItem = ReadTaskItemL(iProfile->Profile(), index);

		CDesCArray* databaseList = new (ELeave) CDesCArrayFlat(KDefaultArraySize);
		CleanupStack::PushL(databaseList);
		GetLocalDatabaseList(KUidNSmlAdapterEMail.iUid, databaseList);
	
		TInt count = databaseList->Count();
		TBool taskUpdated = EFalse;
		for (TInt i = 0; i < count ;i++)
			{
			
			if ((*databaseList)[i].Compare(emailItem.iClientDataSource) == 0)
				{
				taskUpdated = ETrue;
				break;
				}
			}
		if (!taskUpdated)
			{
			TBuf<KBufSize> aLocalDatabase;
			TBuf<KBufSize> aRemoteDatabase;
			TInt aSyncDirection;
			TBool aTaskEnabled;
			ReadTaskL(KUidNSmlAdapterEMail.iUid, aLocalDatabase, aRemoteDatabase,
		    	               aTaskEnabled, aSyncDirection);
			
			if(count == 0) 
			{
				if(aLocalDatabase != KNullDesC)
				{
					aLocalDatabase = KNullDesC;
					aTaskEnabled = EFalse;
					
					CreateTaskL(KUidNSmlAdapterEMail.iUid,aLocalDatabase , aRemoteDatabase,
	                              aTaskEnabled, aSyncDirection);
	                				
				}
			}
			else
			{
				aLocalDatabase = provider.DefaultDataStoreName();
				CreateTaskL(KUidNSmlAdapterEMail.iUid,aLocalDatabase , aRemoteDatabase,
	                              aTaskEnabled, aSyncDirection);
			
			}
			
			

			}
		
		
		CleanupStack::PopAndDestroy(databaseList);
		}

	if (err == KErrNone)
	{
	CleanupStack::PopAndDestroy(&provider);
	}
}

// -----------------------------------------------------------------------------
// CAspContentList::InitTaskL
//
// -----------------------------------------------------------------------------
//
void CAspContentList::InitTaskL(TInt aTaskId)
	{
	__ASSERT_ALWAYS(iProfile, TUtil::Panic(KErrGeneral));
	
    TAspTaskItem task = ReadTaskItemL(iProfile->Profile(), aTaskId);
    
    // try to remove old task
    TInt index = FindTaskIndex(aTaskId);
    if (index != KErrNotFound)
    	{
    	iTaskList.Remove(index);
    	}
    
    task.iLastSync = iProfile->LastSync(aTaskId);
	
	// add new task
	User::LeaveIfError(iTaskList.Append(task));
	}


// -----------------------------------------------------------------------------
// CAspContentList::RemoveTask
//
// -----------------------------------------------------------------------------
//
void CAspContentList::RemoveTask(TInt aDataProviderId)
	{
    TInt index = FindTaskIndexForProvider(aDataProviderId);
        
    // remove task that uses dataprovider aDataProviderId
    if (index != KErrNotFound)
    	{
    	iTaskList.Remove(index);
    	}
	}


// -----------------------------------------------------------------------------
// CAspContentList::RemoveAllTasks
//
// -----------------------------------------------------------------------------
//
void CAspContentList::RemoveAllTasks()
	{
   	iTaskList.Reset();
	}


// -----------------------------------------------------------------------------
// CAspContentList::ReadTaskItemL
//
// Reads one task from sync profile.
// -----------------------------------------------------------------------------
//
TAspTaskItem CAspContentList::ReadTaskItemL(RSyncMLDataSyncProfile& aProfile, TInt aTaskId)
	{
	RSyncMLTask task;
	TAspTaskItem item;

#ifdef _DEBUG	
	TTime _time = TUtil::TimeBefore();
#endif
	
	TRAPD(err, task.OpenL(aProfile, aTaskId));  // IPC call
	if (err != KErrNone)
		{
		FLOG( _L("### RSyncMLTask::OpenL failed (id=%d, err=%d) ###"), aTaskId, err );
	    User::Leave(err);
		}

#ifdef _DEBUG		
	TUtil::TimeAfter(_time, _L("*RSyncMLTask::Open took"));
#endif	
		
   	item.iTaskId = aTaskId; 
	TUtil::StrCopy(item.iDisplayName, task.DisplayName());
	item.iSyncDirection = TAspTask::SyncDirection(task.DefaultSyncType());
	
	item.iDataProviderId = task.DataProvider();
	item.iEnabled = task.Enabled();

	item.iClientDataSource = task.ClientDataSource();
    item.iLocalDatabaseDefined = EFalse;
    item.iRemoteDatabaseDefined = EFalse;
	if (!TUtil::IsEmpty(task.ClientDataSource()))
		{
		item.iLocalDatabaseDefined = ETrue;
		}
	if (!TUtil::IsEmpty(task.ServerDataSource()))
		{
		item.iRemoteDatabaseDefined = ETrue;
		}
	
	task.Close();
	
	return item;
	}


// -----------------------------------------------------------------------------
// CAspContentList::Session
// 
// -----------------------------------------------------------------------------
//
RSyncMLSession& CAspContentList::Session()
	{
	__ASSERT_DEBUG(iSyncSession, TUtil::Panic(KErrGeneral));
	
	return *iSyncSession;
	}


// -----------------------------------------------------------------------------
// CAspContentList::CheckMandatoryDataL
//
// Function checks that all tasks have mandatory data.
// -----------------------------------------------------------------------------
//
TInt CAspContentList::CheckMandatoryDataL(TInt& aContentCount)
	{
	aContentCount = 0;
	TInt goodContentCount = 0;

	TInt count = iTaskList.Count();

	for (TInt i=0; i<count; i++)
		{
		TAspTaskItem& task = iTaskList[i];

		if (!task.iEnabled)
			{
			continue; // task not part of sync - no need to check mandatory fields
			}
			
		if (iApplicationId != KErrNotFound && iApplicationId != EApplicationIdSync)
			{
			TInt appId = TUtil::AppIdFromProviderId(task.iDataProviderId);
			if (appId != iApplicationId)
				{
				continue; // task not part of sync - no need to check mandatory fields
				}
			}
			
       	if (!task.iRemoteDatabaseDefined)
			{
			aContentCount = goodContentCount;
		    return EMandatoryNoRemoteDatabase;
			}
       	
       	if (task.iDataProviderId == KUidNSmlAdapterCalendar.iUid)
       	    {
            TBool status = EFalse;
            status = IsValidClientDataSourceL( task.iClientDataSource );
       	    if ( !status )
       	        {
       	        TInt index = FindProviderIndex(task.iDataProviderId);
       	        TAspProviderItem& provider = ProviderItem(index);
       	              	        
                TBuf<KBufSize> localDatabase;
       	        TBuf<KBufSize> remoteDatabase;
       	        TInt syncDirection;
       	        TBool taskEnabled;
       	        ReadTaskL(provider.iDataProviderId, localDatabase, remoteDatabase,
       	           taskEnabled, syncDirection);
       	        
       	        //Creating new task creates new calendar local database
                CreateTaskL(provider.iDataProviderId, localDatabase, remoteDatabase,
                   taskEnabled, syncDirection);
                
                InitAllTasksL();
                InitDataProvidersL();
       	        }
       	    
       	    }

		goodContentCount++;
		}

    aContentCount = goodContentCount;
    
    if (goodContentCount > 0)
		{
       	return EMandatoryOk;
		}
	else
		{
		return EMandatoryNoContent;
		}
	}


// -----------------------------------------------------------------------------
// CAspContentList::IncludeTasks
//
// -----------------------------------------------------------------------------
//
void CAspContentList::IncludeTasks(const CArrayFix<TInt>* aIncludedProviderList)
	{
	// update iIsIncludedInSync for all provider items (used in IncludeTaskL)
	SetIncludedProviders(aIncludedProviderList);
	
	TInt count = iProviderList.Count();
	
	for (TInt i=0; i<count; i++)
		{
		TAspProviderItem& item = iProviderList[i];
		TRAPD(err, IncludeTaskL(item));
		
		if (err != KErrNone)
			{
			FLOG( _L("### CAspContentList::IncludeTaskL failed (id=%x, err=%d) ###"), item.iDataProviderId, err );
			}
		}
	}


// -----------------------------------------------------------------------------
// CAspContentList::SetIncludedProviders
//
// -----------------------------------------------------------------------------
//
void CAspContentList::SetIncludedProviders(const CArrayFix<TInt>* aIncludedProviderList)
	{
	// exclude all providers
	TInt providerCount = iProviderList.Count();
	for (TInt i=0; i<providerCount; i++)
		{
        (iProviderList[i]).iIsIncludedInSync = EFalse;
		}
		
	// include selected providers (aIncludedProviderList comes from ui)
	TInt count = aIncludedProviderList->Count();
	for (TInt i=0; i<count; i++)
		{
        TInt index = (*aIncludedProviderList)[i];
        
        __ASSERT_ALWAYS(index>=0, TUtil::Panic(KErrGeneral));
        __ASSERT_ALWAYS(index<providerCount, TUtil::Panic(KErrGeneral));
        
        (iProviderList[index]).iIsIncludedInSync = ETrue;
		}
    }


// -----------------------------------------------------------------------------
// CAspContentList::GetIncludedProviders
//
// -----------------------------------------------------------------------------
//
void CAspContentList::GetIncludedProviders(CArrayFix<TInt>* aIncludedProviderList)
	{

	// get list indexes of data providers with enabled task
	
	TInt count = iProviderList.Count();
	
	for (TInt i=0; i<count; i++)
		{
        TAspProviderItem& item = iProviderList[i];
        TInt index = FindTaskIndexForProvider(item.iDataProviderId);
        if (index != KErrNotFound)
        	{
        	TAspTaskItem& item2 = iTaskList[index];
        	if (item2.iEnabled)
        		{
        		TRAP_IGNORE(aIncludedProviderList->AppendL(i));
        		}
        	}
		}
    }


// -----------------------------------------------------------------------------
// CAspContentList::IncludeTaskL
//
// -----------------------------------------------------------------------------
//
void CAspContentList::IncludeTaskL(TAspProviderItem& aDataProvider)
	{
	if (!aDataProvider.iIsIncludedInSync)
		{
		// exclude task (if exists) from sync
		ModifyTaskIncludedL(aDataProvider, EFalse, KNullDesC);
		return;
		}
	
	TInt index = FindTaskIndexForProvider(aDataProvider.iDataProviderId);
	if (index == KErrNotFound)
		{
		// task does not exist - create new
	    TInt id = CreateTaskL(aDataProvider);
	    InitTaskL(id);
		}
	else
	 	{
	 	// include in sync
		ModifyTaskIncludedL(aDataProvider, ETrue, KNullDesC);
	 	}
	}


// -----------------------------------------------------------------------------
// CAspContentList::ModifyTaskIncludedL
//
// -----------------------------------------------------------------------------
//
void CAspContentList::ModifyTaskIncludedL(TInt aDataProviderId, TBool aIsIncluded, const TDesC& aLocalDatabase)
	{
	__ASSERT_ALWAYS(iProfile, TUtil::Panic(KErrGeneral));
	
    TInt index = FindProviderIndex(aDataProviderId);
	if (index != KErrNotFound)
		{
		TAspProviderItem& item = iProviderList[index];
		if (!TUtil::IsEmpty(aLocalDatabase))
		{
			ModifyTaskIncludedL(item, aIsIncluded, aLocalDatabase);	
		}
		else
		{
			ModifyTaskIncludedL(item, aIsIncluded, KNullDesC);
		}
	}
	}

// -----------------------------------------------------------------------------
// CAspContentList::ModifyTaskIncludedL
//
// -----------------------------------------------------------------------------
//
void CAspContentList::ModifyTaskIncludedL(TAspProviderItem& aProvider, TBool aIsIncluded, const TDesC& aLocalDatabase)
	{
	__ASSERT_ALWAYS(iProfile, TUtil::Panic(KErrGeneral));
	
	TInt index = FindTaskIndexForProvider(aProvider.iDataProviderId);
	if (index == KErrNotFound)
		{
		if (!TUtil::IsEmpty(aLocalDatabase) && (aProvider.iDataProviderId == KUidNSmlAdapterEMail.iUid))
		{
			CreateTaskL(aProvider.iDataProviderId,aLocalDatabase,KNullDesC,ETrue,TAspTask::SyncDirection(SmlSyncDirection()));
	 		iProfile->Profile().UpdateL();
	 			 
	    }
		return; // no task for this provider
		}
		
	TAspTaskItem& item = iTaskList[index];
	if ((item.iEnabled == aIsIncluded) && (aProvider.iDataProviderId != KUidNSmlAdapterEMail.iUid))
		{
		return; // nothing to modify
		}
	
	if(aProvider.iDataProviderId == KUidNSmlAdapterEMail.iUid)
	{
	RSyncMLTask task;
	CleanupClosePushL(task);
    
	task.OpenL(iProfile->Profile(), item.iTaskId);
	if (!TUtil::IsEmpty(aLocalDatabase))
	{
		CreateTaskL(aProvider.iDataProviderId,aLocalDatabase,task.ServerDataSource(),
	 			 ETrue,TAspTask::SyncDirection(task.DefaultSyncType()));
	
	}

	task.SetEnabledL(aIsIncluded);
	// store task into database
	task.UpdateL();
	
	iProfile->Profile().UpdateL();
	
	item.iEnabled = aIsIncluded;
	CleanupStack::PopAndDestroy(&task);
	}
	else
	{
		RSyncMLTask task;
		CleanupClosePushL(task);
    
	task.OpenL(iProfile->Profile(), item.iTaskId);
	task.SetEnabledL(aIsIncluded);
	
	// store task into database
	task.UpdateL();
	iProfile->Profile().UpdateL();
	
	item.iEnabled = aIsIncluded;
	
	CleanupStack::PopAndDestroy(&task);
	}
	}

// -----------------------------------------------------------------------------
// CAspContentList::ModifyTaskDirectionL
//
// -----------------------------------------------------------------------------
//
void CAspContentList::ModifyTaskDirectionL(TAspProviderItem& aProvider, TInt aSyncDirection)
	{
	__ASSERT_ALWAYS(iProfile, TUtil::Panic(KErrGeneral));
	
	TInt index = FindTaskIndexForProvider(aProvider.iDataProviderId);
	if (index == KErrNotFound)
		{
		return; // no task for this provider
		}
		
	TAspTaskItem& item = iTaskList[index];
	if (item.iSyncDirection == aSyncDirection)
		{
		return; // nothing to modify
		}
	
	RSyncMLTask task;
	CleanupClosePushL(task);
    
	task.OpenL(iProfile->Profile(), item.iTaskId);
	task.SetDefaultSyncTypeL(TAspTask::SmlSyncDirection(aSyncDirection));
	
	// store task into database
	task.UpdateL();
	iProfile->Profile().UpdateL();
	
	item.iSyncDirection = aSyncDirection;
	
	CleanupStack::PopAndDestroy(&task);
	}


// -----------------------------------------------------------------------------
// CAspContentList::ModifyTaskDirectionsL
//
// -----------------------------------------------------------------------------
//
void CAspContentList::ModifyTaskDirectionsL(TInt aSyncDirection)
	{
	__ASSERT_ALWAYS(iProfile, TUtil::Panic(KErrGeneral));
	
	TInt count = iProviderList.Count();
	for (TInt i=0; i<count; i++)
		{
        TAspProviderItem& item = iProviderList[i];
        ModifyTaskDirectionL(item, aSyncDirection);
		}
	}


// -----------------------------------------------------------------------------
// CAspContentList::CheckTaskDirectionsL
//
// -----------------------------------------------------------------------------
//
TBool CAspContentList::CheckTaskDirectionsL()
	{
	__ASSERT_ALWAYS(iProfile, TUtil::Panic(KErrGeneral));

    TInt previous = KErrNotFound;
    
	TInt count = iTaskList.Count();
	for (TInt i=0; i<count; i++)
		{
        TAspTaskItem& item = iTaskList[i];
        if (item.iSyncDirection != previous && previous != KErrNotFound)
        	{
        	return EFalse;
        	}
        	
        previous = item.iSyncDirection;
		}
		
	return ETrue;
	}


// -----------------------------------------------------------------------------
// CAspContentList::CreateTaskL
//
// -----------------------------------------------------------------------------
//
TInt CAspContentList::CreateTaskL(TAspProviderItem& aDataProvider)
	{
	__ASSERT_ALWAYS(iProfile, TUtil::Panic(KErrGeneral));
		
	if (!aDataProvider.iHasDefaultDataStore)
		{
		User::Leave(KErrNotFound);
		}
		
	RSyncMLTask task;
	CleanupClosePushL(task);
	
	if (aDataProvider.iDataProviderId == KUidNSmlAdapterCalendar.iUid )
        {
        TBuf<128> calLocalDb ;
        CreateCalLocalDatabaseL(calLocalDb);
        task.CreateL(iProfile->Profile(), aDataProvider.iDataProviderId, 
                KNullDesC, calLocalDb);
        }
    else
        {
        task.CreateL(iProfile->Profile(), aDataProvider.iDataProviderId, 
	              KNullDesC, aDataProvider.iDefaultDataStore);
        }
	task.SetEnabledL(aDataProvider.iIsIncludedInSync);
	task.SetDefaultSyncTypeL(SmlSyncDirection());
	
	// store task into database
	task.UpdateL();
	iProfile->Profile().UpdateL();
	
	TInt id = task.Identifier();
	
	CleanupStack::PopAndDestroy(&task);
	
	return id;
	}


// -----------------------------------------------------------------------------
// CAspContentList::CreateTaskL
// 
// -----------------------------------------------------------------------------
//
void CAspContentList::CreateTaskL(TInt aDataProviderId,
                                  const TDesC& aLocalDatabase,
                                  const TDesC& aRemoteDatabase,
                                  TBool aEnabled, TInt aSyncDirection)
	{
	__ASSERT_ALWAYS(iProfile, TUtil::Panic(KErrGeneral));
		
	TInt providerIndex = FindProviderIndex(aDataProviderId);
	if (providerIndex == KErrNotFound)
		{
		return;
		}
	 
	// remove existing task for aDataProviderId
    TInt taskIndex = FindTaskIndexForProvider(aDataProviderId);
    if (taskIndex != KErrNotFound)
    	{
    	TAspTaskItem& item = TaskItem(taskIndex);
        iProfile->Profile().DeleteTaskL(item.iTaskId);
        iProfile->Profile().UpdateL();
        RemoveTask(aDataProviderId);  // remove deleted task from task list   	
    	}
    
    TAspProviderItem& item = ProviderItem(providerIndex);
    
	RSyncMLTask task;
	CleanupClosePushL(task);
	
	if (TUtil::IsEmpty(aLocalDatabase))
		{
		if(!ProviderItem(providerIndex).iHasDefaultDataStore)
			{
			// defult data store name was probably truncated
			task.CreateL(iProfile->Profile(), aDataProviderId, 
			             aRemoteDatabase, KNullDesC);
			aEnabled = EFalse;
			}
		else
			{
			if (aDataProviderId == KUidNSmlAdapterCalendar.iUid )
                {
                TBuf<KBufSize> calLocalDb ;
				TRAPD(err ,RetrieveCalLocalDatabaseL(calLocalDb));
				if (err != KErrNone)
					{
					CreateCalLocalDatabaseL(calLocalDb);
					}				
                
                task.CreateL(iProfile->Profile(), aDataProviderId, 
                                              aRemoteDatabase, calLocalDb);
                }
            else
                {
                task.CreateL(iProfile->Profile(), aDataProviderId, 
			             aRemoteDatabase, item.iDefaultDataStore);
                }
			}
		}
	else
		{
		if (aDataProviderId == KUidNSmlAdapterCalendar.iUid )
		    {
		    TBuf<128> calLocalDb ;
		    TRAPD(err ,RetrieveCalLocalDatabaseL(calLocalDb));
		    if (err != KErrNone)
		        {
		        CreateCalLocalDatabaseL(calLocalDb);
		        }
		    
		    task.CreateL(iProfile->Profile(), aDataProviderId, 
		                                  aRemoteDatabase, calLocalDb);
		    }
		else
		    {
		    task.CreateL(iProfile->Profile(), aDataProviderId, 
		                                              aRemoteDatabase, aLocalDatabase);
		    }
		}
	
	task.SetEnabledL(aEnabled);
	task.SetDefaultSyncTypeL(TAspTask::SmlSyncDirection(aSyncDirection));
	
	// store task into database
	task.UpdateL();
	iProfile->Profile().UpdateL();
	
	CleanupStack::PopAndDestroy(&task);
	}


// -----------------------------------------------------------------------------
// CAspContentList::CreateTask
// 
// -----------------------------------------------------------------------------
//
void CAspContentList::CreateTask(TInt aDataProviderId, 
                                 const TDesC& aLocalDatabase,
                                 const TDesC& aRemoteDatabase,
                                 TBool aEnabled, TInt aSyncDirection)
	{
	FLOG( _L("CAspContentList::CreateTask START") );
	
	TRAPD(err, CreateTaskL(aDataProviderId, aLocalDatabase, aRemoteDatabase, aEnabled, aSyncDirection));
	
	if (err != KErrNone)
		{
		FLOG( _L("### CreateTask failed (id=%x err=%d) ###"), aDataProviderId, err );
		}
		
	FLOG( _L("CAspContentList::CreateTask END") );
	}

// -----------------------------------------------------------------------------
// CAspContentList::CreateTask
// 
// -----------------------------------------------------------------------------
//
void CAspContentList::CreateCalLocalDatabaseL(TDes& aCalName)
    {
            
    aCalName.Copy(KDrive);
    
    TBuf<KBufSize> buffer;
    iProfile->GetName(buffer);
	
	TInt currentProfileId = iProfile->ProfileId();
	
	if (buffer.Compare(KAutoSyncProfileName) == 0)
		{
		CAspSchedule* schedule = CAspSchedule::NewLC();
		currentProfileId = schedule->ProfileId();
		TAspParam param(iApplicationId, iSyncSession);
		CAspProfile* selectedProfile = CAspProfile::NewLC(param);
		selectedProfile->OpenL(currentProfileId, CAspProfile::EOpenRead, CAspProfile::EAllProperties);
		selectedProfile->GetName(buffer);
		CleanupStack::PopAndDestroy(selectedProfile);
		CleanupStack::PopAndDestroy(schedule);
		}
		
    CCalSession* calSession = CCalSession::NewL();
    CleanupStack::PushL(calSession);
    
        
    TInt suffix = 0;
    TInt suffixLen = 0;
    TInt delPos = buffer.Length();
    
    while (!IsCalNameAvailableL(*calSession ,buffer))
        {
        //A db with profile name already exists , try profile_1 ,profile_2 etc..
        if (!suffix)
            {
            buffer.Append(KCharUnderscore);
            ++delPos;
            }
        else
            {
            while (suffix/10)
                {
                ++suffixLen;
                }
            
            buffer.Delete(delPos ,++suffixLen);
            suffixLen = 0;
            }   
        buffer.AppendNum(++suffix);
        }
    
    TBuf8<128> keyBuff;
    TUint calValue = 0;
    CCalCalendarInfo* calinfo = CCalCalendarInfo::NewL();
    CleanupStack::PushL(calinfo);
    //Visibility
    calinfo->SetEnabled(ETrue);
    
    calinfo->SetNameL(buffer);
    calinfo->SetColor(Math::Random());
    
    // Set Meta Data Properties
    // LUID Meta Property
    keyBuff.Zero();
    keyBuff.AppendNum( EFolderLUID );
    calValue = CCalenMultiCalUtil::GetNextAvailableOffsetL();
    FLOG(_L("CNSmlOviAgendaAdapterPlugin::CreateFolderItemL: nextoffset: '%d'"), calValue);
    TPckgC<TUint> pckgUidValue( calValue );
    calinfo->SetPropertyL( keyBuff, pckgUidValue );
    
    // Create & Modified Time Meta Property
    keyBuff.Zero();
    keyBuff.AppendNum( ECreationTime );
    TTime time;
    time.HomeTime();
    TPckgC<TTime> pckgCreateTimeValue( time );
    calinfo->SetPropertyL( keyBuff, pckgCreateTimeValue );
    keyBuff.Zero();
    keyBuff.AppendNum( EModificationTime );
    calinfo->SetPropertyL( keyBuff, pckgCreateTimeValue );
    
    // Sync Status
    keyBuff.Zero();
    keyBuff.AppendNum( ESyncStatus );
    TBool syncstatus( ETrue );
    TPckgC<TBool> pckgSyncStatusValue( syncstatus );
    calinfo->SetPropertyL( keyBuff, pckgSyncStatusValue );
    
    // Global UID MetaDataProperty 
    keyBuff.Zero();
    keyBuff.AppendNum( EGlobalUUID );
    CCalenInterimUtils2* interimUtils = CCalenInterimUtils2::NewL();
    CleanupStack::PushL( interimUtils );
    HBufC8* guuid = interimUtils->GlobalUidL();
    TPtr8 guuidPtr = guuid->Des();
    CleanupStack::PushL( guuid );
    calinfo->SetPropertyL( keyBuff, guuidPtr );
    CleanupStack::PopAndDestroy( guuid );
    CleanupStack::PopAndDestroy( interimUtils );
          
    // Owner Name
    keyBuff.Zero();
    keyBuff.AppendNum( EDeviceSyncServiceOwner );
    TPckgC<TInt> pckgAppUIDValue( KCRUidNSmlDSApp.iUid );    
    calinfo->SetPropertyL( keyBuff, pckgAppUIDValue );

    // Profile ID Meta Property
    keyBuff.Zero();
    keyBuff.AppendNum( EDeviceSyncProfileID );
    TPckgC<TInt> pckgProfileIdValue( currentProfileId );    
    calinfo->SetPropertyL( keyBuff, pckgProfileIdValue );

    // Lock the SYNC option
   keyBuff.Zero();
   keyBuff.AppendNum( ESyncConfigEnabled );
   TBool synclockstatus( ETrue );
   TPckgC<TBool> pckgSyncLockValue( synclockstatus );
   calinfo->SetPropertyL( keyBuff, pckgSyncLockValue );
    
    // Create the CalFile
    HBufC* calfilename = CCalenMultiCalUtil::GetNextAvailableCalFileL();
    calSession->CreateCalFileL( calfilename->Des(), *calinfo );
	
	aCalName.Copy( calfilename->Des() );
    
    delete calfilename;
    
    CleanupStack::PopAndDestroy(calinfo);
    CleanupStack::PopAndDestroy(calSession);
    }

// -----------------------------------------------------------------------------
// CAspContentList::IsCalNameAvailableL
// 
// -----------------------------------------------------------------------------
//
TBool CAspContentList::IsCalNameAvailableL(CCalSession& aSession ,TDes& aCalName)
    {
    CCalSession* vCalSubSession = NULL;  
    CDesCArray* calfilearr = aSession.ListCalFilesL();
    
    for(TInt i = 0; i < calfilearr->Count(); i++)
        {
        vCalSubSession = CCalSession::NewL(aSession);
        CleanupStack::PushL(vCalSubSession);
        vCalSubSession->OpenL(calfilearr->MdcaPoint(i));
    
        CCalCalendarInfo* caleninfo = vCalSubSession->CalendarInfoL(); 
        
        if (aCalName == caleninfo->NameL())
            {
            delete caleninfo;
            delete calfilearr;
            CleanupStack::PopAndDestroy(vCalSubSession);
            return EFalse;
            }
    
        delete caleninfo;    
        CleanupStack::PopAndDestroy(vCalSubSession); 
        }
    delete calfilearr;
    return ETrue;
    
    }
// -----------------------------------------------------------------------------
// CAspContentList::RetrieveCalLocalDatabaseL
// 
// -----------------------------------------------------------------------------
//
void CAspContentList::RetrieveCalLocalDatabaseL(TDes& aCalName)
    {
      
    FLOG(_L("CAspContentList::RetrieveCalLocalDatabaseL: BEGIN"));   
      
    TBuf8<128> keyBuff;
    CCalSession* vCalSession = NULL;
    CCalSession* vCalSubSession = NULL;   
    
    vCalSession = CCalSession::NewL();
    CleanupStack::PushL(vCalSession);
    
    CDesCArray* calfilearr = vCalSession->ListCalFilesL();            
    
    TBool dbFound = EFalse;
    for(TInt i = 0; i < calfilearr->Count(); i++)
        {
        vCalSubSession = CCalSession::NewL(*vCalSession);
        CleanupStack::PushL(vCalSubSession);
        vCalSubSession->OpenL(calfilearr->MdcaPoint(i));
        
        CCalCalendarInfo* caleninfo = vCalSubSession->CalendarInfoL(); 
        CleanupStack::PushL(caleninfo);
		
		//Get MARKASDELETE MetaData property
		keyBuff.Zero();
		TBool markAsdelete = EFalse;
		keyBuff.AppendNum( EMarkAsDelete );
		TPckgC<TBool> pckMarkAsDelete(markAsdelete);
		TRAPD(err,pckMarkAsDelete.Set(caleninfo->PropertyValueL(keyBuff)));
		if ( err == KErrNone )
			{
			markAsdelete = pckMarkAsDelete();
			if( markAsdelete )
				{
				CleanupStack::PopAndDestroy(caleninfo);
				CleanupStack::PopAndDestroy(vCalSubSession);  	  	 
				continue;
				}
			}
        
        TInt profileId;
        keyBuff.Zero();
        keyBuff.AppendNum( EDeviceSyncProfileID );
        TPckgC<TInt> intBuf(profileId);
        TRAP_IGNORE(intBuf.Set(caleninfo->PropertyValueL(keyBuff)));
        profileId = intBuf();
		
        TBuf<KBufSize> buffer;
		iProfile->GetName(buffer);
		
		TInt currentProfileId = iProfile->ProfileId();
	
		if (buffer.Compare(KAutoSyncProfileName) == 0)
			{
			CAspSchedule* schedule = CAspSchedule::NewLC();
			currentProfileId = schedule->ProfileId();
			CleanupStack::PopAndDestroy(schedule);		
			}
		
        if ( profileId == currentProfileId)
            {
            aCalName.Append(caleninfo->FileNameL());
            dbFound = ETrue;
            CleanupStack::PopAndDestroy(caleninfo);   
            CleanupStack::PopAndDestroy(vCalSubSession);
            break;
            }
        CleanupStack::PopAndDestroy(caleninfo);    
        CleanupStack::PopAndDestroy(vCalSubSession); 
        }
		
		if( dbFound == EFalse )
			{
			delete calfilearr; 
			User::Leave( KErrNotFound );
			}
 
    delete calfilearr;     
    CleanupStack::PopAndDestroy(vCalSession);
    }

// -----------------------------------------------------------------------------
// CAspContentList::IsValidClientDataSource
// 
// -----------------------------------------------------------------------------
//
TBool CAspContentList::IsValidClientDataSourceL( TDes& aCalName )
    {
    TBool datasourcefound = ETrue;
    TBuf8<KBufSize> keyBuff;
    CCalCalendarInfo* caleninfo = NULL;
    CCalSession* session = CCalSession::NewL();
    CleanupStack::PushL(session);
    TRAPD (err, session->OpenL(aCalName));
    
    if( err == KErrNotFound )
        {
        datasourcefound = EFalse;
        }
    else
        {
        caleninfo = session->CalendarInfoL(); 
        CleanupStack::PushL(caleninfo);
                
        //Get MARKASDELETE MetaData property
        keyBuff.Zero();
        TBool markAsdelete = EFalse;
        keyBuff.AppendNum( EMarkAsDelete );
        TPckgC<TBool> pckMarkAsDelete(markAsdelete);
        TRAP(err,pckMarkAsDelete.Set(caleninfo->PropertyValueL(keyBuff)));
        if ( err == KErrNone )
            {
            markAsdelete = pckMarkAsDelete();
            if( markAsdelete )
                {
                datasourcefound = EFalse;
                }
            }
        CleanupStack::PopAndDestroy(caleninfo);
        }
    CleanupStack::PopAndDestroy(session);
    return datasourcefound;
    }

// -----------------------------------------------------------------------------
// CAspContentList::ReadTaskL
// 
// -----------------------------------------------------------------------------
//
void CAspContentList::ReadTaskL(TInt aDataProviderId, TDes& aLocalDatabase,
                                TDes& aRemoteDatabase, TBool& aEnable, TInt& aSyncDirection)

	{
	__ASSERT_ALWAYS(iProfile, TUtil::Panic(KErrGeneral));

	aLocalDatabase = KNullDesC;
	aRemoteDatabase = KNullDesC;
	aSyncDirection = SyncDirection();
	aEnable = EFalse;

	TInt index = FindTaskIndexForProvider(aDataProviderId);
	if (index == KErrNotFound)
		{
		return;
		}
	 
    TAspTaskItem& item = TaskItem(index);
    
	RSyncMLTask task;
	CleanupClosePushL(task);
	
	task.OpenL(iProfile->Profile(), item.iTaskId);
	
	aLocalDatabase = task.ClientDataSource();
	aRemoteDatabase = task.ServerDataSource();
	aEnable = task.Enabled();
	aSyncDirection = TAspTask::SyncDirection(task.DefaultSyncType());
		
	CleanupStack::PopAndDestroy(&task);
	}


// -----------------------------------------------------------------------------
// CAspContentList::GetLocalDatabaseListL
// 
// -----------------------------------------------------------------------------
//
void CAspContentList::GetLocalDatabaseListL(TInt aDataProviderId, CDesCArray* aList)
	{
	__ASSERT_ALWAYS(iSyncSession, TUtil::Panic(KErrGeneral));
	
    RSyncMLDataProvider provider;	
    CleanupClosePushL(provider);

    provider.OpenL(*iSyncSession, aDataProviderId);
    provider.GetDataStoreNamesL(*aList);
    
    CleanupStack::PopAndDestroy(&provider);
    }


// -----------------------------------------------------------------------------
// CAspContentList::GetLocalDatabaseList
// 
// -----------------------------------------------------------------------------
//
void CAspContentList::GetLocalDatabaseList(TInt aDataProviderId, CDesCArray* aList)
	{
	TRAP_IGNORE(GetLocalDatabaseListL(aDataProviderId, aList));
    }


// -----------------------------------------------------------------------------
// CAspContentList::SmlSyncDirection
//
// -----------------------------------------------------------------------------
//
TSmlSyncType CAspContentList::SmlSyncDirection()
	{
	TInt syncDirection = SyncDirection();
	return TAspTask::SmlSyncDirection(syncDirection);
	}


// -----------------------------------------------------------------------------
// CAspContentList::SyncDirection
//
// -----------------------------------------------------------------------------
//
TInt CAspContentList::SyncDirection()
	{
	if (iTaskList.Count() == 0)
		{
		return ESyncDirectionTwoWay; // default sync direction
		}
		
	TAspTaskItem& item = iTaskList[0];
	return item.iSyncDirection;  // all tasks have same sync direction
	}


// -----------------------------------------------------------------------------
// CAspContentList::TaskEnabled
//
// -----------------------------------------------------------------------------
//
TBool CAspContentList::TaskEnabled(TInt aDataProviderId)
	{
	TBool ret = EFalse;
	
    TInt index = FindTaskIndexForProvider(aDataProviderId);
    if (index != KErrNotFound)
    	{
    	TAspTaskItem& item = TaskItem(index);
    	if (item.iEnabled)
    		{
    		ret = ETrue;
    		}
    	}
    	
	return ret;
	}


// -----------------------------------------------------------------------------
// CAspContentList::Sort
//
// -----------------------------------------------------------------------------
//
void CAspContentList::Sort()
	{
	iProviderList.Sort(TLinearOrder<TAspProviderItem>(TAspProviderItem::CompareItems));
	}


#ifdef _DEBUG

// -----------------------------------------------------------------------------
// CAspContentList::LogL
//
// -----------------------------------------------------------------------------
//
void CAspContentList::LogL(const TDesC& aText)
	{
	FLOG( _L("CAspContentList::LogL START") );

	if (aText.Length() > 0)
		{
		FTRACE( RDebug::Print(_L("Profile = '%S'"), &aText) );
		}
		
	LogTasksL();
	LogDataProvidersL();
	
	FLOG( _L("CAspContentList::LogL END") );
	}
	

// -----------------------------------------------------------------------------
// CAspContentList::LogTasksL
//
// -----------------------------------------------------------------------------
//
void CAspContentList::LogTasksL()
	{
    FLOG( _L("---- sync tasks ----") );
    
	TInt count = iTaskList.Count();
	
	for (TInt i=0; i<count; i++)
		{
		TAspTaskItem& t = iTaskList[i];
		TBuf<128> buf;
		GetTaskLogText(buf, t);
		FLOG(buf);
		}
		
	FLOG( _L("---- sync tasks ----") );
	}


// -----------------------------------------------------------------------------
// CAspContentList::GetTaskLogText
//
// -----------------------------------------------------------------------------
//
void CAspContentList::GetTaskLogText(TDes& aText, TAspTaskItem& aTask)
	{
	TBuf<KBufSize64> buf;
	GetDataProviderIdText(buf, aTask.iDataProviderId);
	
	aText.Format(_L("id=%d, remote_db=%d, local_db=%d, enabled=%d, providerId=%x, providername='%S'"),
	             aTask.iTaskId, aTask.iRemoteDatabaseDefined, aTask.iLocalDatabaseDefined,
	             aTask.iEnabled, aTask.iDataProviderId, &buf);
    }


// -----------------------------------------------------------------------------
// CAspContentList::LogDataProvidersL
//
// -----------------------------------------------------------------------------
//
void CAspContentList::LogDataProvidersL()
	{
    FLOG( _L("---- data providers ----") );
    
	TInt count = iProviderList.Count();
	
	for (TInt i=0; i<count; i++)
		{
		TAspProviderItem& item = iProviderList[i];
		TBuf<32> buf;
		GetDataProviderIdText(buf, item.iDataProviderId);
		if (item.iHasDefaultDataStore)
			{
			FTRACE( RDebug::Print(_L("id=%S (%x), name='%S'"), &buf, item.iDataProviderId, &item.iDisplayName) );
			}
		else
			{
			FTRACE( RDebug::Print(_L("id=%S (%x), name='%S', ### no default data provider ###"), &buf, item.iDataProviderId, &item.iDisplayName) );
			}
		}
		
	FLOG( _L("---- data providers ----") );
	}

    
// -----------------------------------------------------------------------------
// CAspContentList::GetDataProviderIdText
//
// -----------------------------------------------------------------------------
//
void CAspContentList::GetDataProviderIdText(TDes& aText, TInt aId)
	{
	aText = _L("unknown");
	
	if (aId == KUidNSmlAdapterContact.iUid)
		{
		aText = _L("KUidNSmlAdapterContact");
		}
	if (aId == KUidNSmlAdapterCalendar.iUid)
		{
		aText = _L("KUidNSmlAdapterCalendar");
		}
	if (aId == KUidNSmlAdapterEMail.iUid)
		{
		aText = _L("KUidNSmlAdapterEMail");
		}
	if (aId == KUidNSmlAdapterNote.iUid)
		{
		aText = _L("KUidNSmlAdapterNote");
    	}
	if (aId == KUidNSmlAdapterSms.iUid)
		{
		aText = _L("KUidNSmlAdapterSms");
    	}
	if (aId == KUidNSmlAdapterMMS.iUid)
		{
		aText = _L("KUidNSmlAdapterMMS");
    	}
	if (aId == KUidNSmlAdapterBookmarks.iUid)
		{
		aText = _L("KUidNSmlAdapterBookmarks");
    	}
	}
	
#endif



/*******************************************************************************
 * class TAspProfileItem
 *******************************************************************************/


// -----------------------------------------------------------------------------
// TAspProfileItem::Init
//
// -----------------------------------------------------------------------------
//
void TAspProfileItem::Init()
	{
	iProfileName = KNullDesC;
	iProfileId = KErrNotFound;
	iSynced = EFalse;
	iLastSync = 0;
	iBearer = KErrNotFound;
	iActive = EFalse;
	iDeleteAllowed = EFalse;
	iApplicationId = KErrNotFound;
	iMandatoryCheck = KErrNotFound;
	iTaskCount = KErrNotFound;
	iTaskId = KErrNotFound;
	}




// -----------------------------------------------------------------------------
// TAspProfileItem::NameLC
//
// -----------------------------------------------------------------------------
//
HBufC* TAspProfileItem::NameLC()
	{
	return iProfileName.AllocLC();
	}


// -----------------------------------------------------------------------------
// TAspProfileItem::Name
//
// -----------------------------------------------------------------------------
//
const TDesC& TAspProfileItem::Name()
	{
	return iProfileName;
	}


// -----------------------------------------------------------------------------
// TAspProfileItem::SetName
//
// -----------------------------------------------------------------------------
//
void TAspProfileItem::SetName(const TDesC& aName)
	{
	TUtil::StrCopy(iProfileName, aName);
	}


// -----------------------------------------------------------------------------
// TAspProfileItem::CompareItems
//
// -----------------------------------------------------------------------------
//
TInt TAspProfileItem::CompareItems(const TAspProfileItem& aFirst, const TAspProfileItem& aSecond)
	{
	return aFirst.iProfileName.CompareC(aSecond.iProfileName);
	}






/*******************************************************************************
 * class CAspProfileList
 *******************************************************************************/



// -----------------------------------------------------------------------------
// CAspProfileList::NewLC
//
// -----------------------------------------------------------------------------
//
CAspProfileList* CAspProfileList::NewLC(const TAspParam& aParam)
    {
    FLOG( _L("CAspProfileList::NewLC START") );
    
    CAspProfileList* self = new (ELeave) CAspProfileList(aParam);
	CleanupStack::PushL(self);
	self->ConstructL();
	
	
    FLOG( _L("CAspProfileList::NewLC END") );
	return self;
    }


// -----------------------------------------------------------------------------
// CAspProfileList::NewL
//
// -----------------------------------------------------------------------------
//
CAspProfileList* CAspProfileList::NewL(const TAspParam& aParam)
    {
    FLOG( _L("CAspProfileList::NewL START") );
    
    CAspProfileList* self = new (ELeave) CAspProfileList(aParam);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);

    FLOG( _L("CAspProfileList::NewL END") );
	return self;
    }


// -----------------------------------------------------------------------------
// Destructor
//
// -----------------------------------------------------------------------------
//
CAspProfileList::~CAspProfileList()
    {
	iList.Close();
    }


// -----------------------------------------------------------------------------
// CAspProfileList::ConstructL
//
// -----------------------------------------------------------------------------
//
void CAspProfileList::ConstructL(void)
    {
    }


// -----------------------------------------------------------------------------
// CAspProfileList::CAspProfileList
//
// -----------------------------------------------------------------------------
//
CAspProfileList::CAspProfileList(const TAspParam& aParam)
	{
	__ASSERT_ALWAYS(aParam.iSyncSession, TUtil::Panic(KErrGeneral));

	iSyncSession = aParam.iSyncSession;
	iApplicationId = aParam.iApplicationId;
	}


// -----------------------------------------------------------------------------
// CAspProfileList::Count
//
// -----------------------------------------------------------------------------
//
TInt CAspProfileList::Count()
	{
	return iList.Count();
	}


// -----------------------------------------------------------------------------
// CAspProfileList::Count
//
// -----------------------------------------------------------------------------
//
TInt CAspProfileList::Count(TInt aApplicationId)
	{
	TInt profileCount = 0;
	TInt count = iList.Count();

	for (TInt i=0; i<count; i++)
		{
		TAspProfileItem& item = iList[i];
		if (item.iApplicationId == aApplicationId)
			{
			profileCount++;
			}
		}
	
	return profileCount;
	}


// -----------------------------------------------------------------------------
// CAspProfileList::DeletableProfileCount
//
// -----------------------------------------------------------------------------
//
TInt CAspProfileList::DeletableProfileCount()
	{
	TInt profileCount = 0;
	TInt count = iList.Count();

	for (TInt i=0; i<count; i++)
		{
		TAspProfileItem& item = iList[i];
		if (item.iDeleteAllowed)
			{
			profileCount++;
			}
		}
	
	return profileCount;
	}


// -----------------------------------------------------------------------------
// CAspProfileList::Item
// 
// -----------------------------------------------------------------------------
//
TAspProfileItem& CAspProfileList::Item(TInt aIndex)
	{
	if ((aIndex < 0) || (aIndex >= Count()))
	    {
		TUtil::Panic(KErrArgument);
	    }

	return iList[aIndex];
	}


// -----------------------------------------------------------------------------
// CAspProfileList::FindProfileIndex
//
// -----------------------------------------------------------------------------
//
TInt CAspProfileList::FindProfileIndex(const TDesC& aProfileName)
	{
	TInt count = iList.Count();

	for (TInt i=0; i<count; i++)
		{
		TPtrC ptr = Item(i).Name();
		if (ptr.Compare(aProfileName) == 0)
			{
			return i;
			}
		}

	return KErrNotFound;
	}

// -----------------------------------------------------------------------------
// CAspProfileList::GetDefaultProfileIdL
//
// -----------------------------------------------------------------------------
//
void CAspProfileList::GetDefaultProfileIdL(TInt& aValue)
	{
	const TUid KRepositoryId = KCRUidDSDefaultProfileInternalKeys;
	
    CRepository* rep = CRepository::NewLC(KRepositoryId);
    TInt err = rep->Get(KNsmlDsDefaultProfile, aValue);
	User::LeaveIfError(err);
	CleanupStack::PopAndDestroy(rep);    
	}
	
// -----------------------------------------------------------------------------
// CAspProfileList::FindLastSyncedProfileIndexL
//
// -----------------------------------------------------------------------------
//
TInt CAspProfileList::FindLastSyncedProfileIndexL()
	{
	TInt index = KErrNotFound;
	TTime syncTime = 0;
	
	TInt count = iList.Count();

	for (TInt i=0; i<count; i++)
		{
		TAspProfileItem& item = iList[i];
		if (item.iLastSync > syncTime)
			{
			syncTime = item.iLastSync;
			index = i;
			}
		}
	
	if (index == KErrNotFound && count > 0)
		{
		//Read the Default Profile Id from the Cenrep
		TInt defaultprofileId;
		GetDefaultProfileIdL(defaultprofileId);
		for (TInt i=0; i<count; i++)
			{
			TAspProfileItem& item = iList[i];
			if ( item.iProfileId == defaultprofileId )
				{
				index = i;
				break;
				}
			}
		
		if(index == KErrNotFound)
			{
			TAspParam param(iApplicationId, iSyncSession);
	    	    	
			for( TInt i=0; i<count; i++)
				{
				CAspProfile* profile = CAspProfile::NewLC(param);
				TAspProfileItem& item = iList[i];
				profile->OpenL(item.iProfileId, CAspProfile::EOpenRead, CAspProfile::EAllProperties);	
				if( CAspProfile::IsPCSuiteProfile(profile) )
					{
					index = i;
					CleanupStack::PopAndDestroy(profile);
					break;
					}
				else
					{
					CleanupStack::PopAndDestroy(profile);
					}			
				}
			}
		}
	return index;
	}


// -----------------------------------------------------------------------------
// CAspProfileList::FilteredList
//
// -----------------------------------------------------------------------------
//
CAspProfileList* CAspProfileList::FilteredListL(TAspFilterInfo& aFilterInfo)
	{
	TInt type = aFilterInfo.iFilterType;
	
	if (type != TAspFilterInfo::EIncludeDeletableProfile &&
	    type != TAspFilterInfo::EIncludeRemoteProfile)
		{
		TUtil::Panic(KErrNotSupported);
		}
		
	TAspParam param(iApplicationId, iSyncSession);
	CAspProfileList* profileList = CAspProfileList::NewLC(param);
	
	TInt count = iList.Count();

	for (TInt i=0; i<count; i++)
		{
		TAspProfileItem& item = iList[i];
		
		if (type == TAspFilterInfo::EIncludeDeletableProfile)
			{
		    if (item.iDeleteAllowed)
			    {
			    profileList->AddL(item);
			    }
			}

		if (type == TAspFilterInfo::EIncludeRemoteProfile)
			{
		    if (item.iBearer == EAspBearerInternet)
			    {
			    profileList->AddL(item);
			    }
			}
    	}
	
		
	CleanupStack::Pop(profileList);
	return profileList;
	}


// -----------------------------------------------------------------------------
// CAspProfileList::ReplaceProfileItemL
//
// -----------------------------------------------------------------------------
//
void CAspProfileList::ReplaceProfileItemL(TAspProfileItem& aProfileItem)
	{
	// remove old profile with same id
	Remove(aProfileItem.iProfileId);
	
	// add new profile 
    User::LeaveIfError(iList.Append(aProfileItem));
	}
	
	
// -----------------------------------------------------------------------------
// CAspProfileList::ReadProfileL
//
// -----------------------------------------------------------------------------
//
void CAspProfileList::ReadProfileL(TInt aProfileId)
	{
	FLOG( _L("CAspProfileList::ReadProfileL START") );
	
	TAspParam param(iApplicationId, iSyncSession);
    CAspProfile* profile = CAspProfile::NewLC(param);
	
	profile->OpenL(aProfileId, CAspProfile::EOpenRead, CAspProfile::EAllProperties);
	
	TAspProfileItem item;
	item.Init();

	if (iApplicationId != EApplicationIdSync)
		{
		TInt dataProviderId = TUtil::ProviderIdFromAppId(iApplicationId, CAspProfile::IsOperatorProfileL(profile));
		TInt taskId = TAspTask::FindTaskIdL(profile, dataProviderId, KNullDesC);
		item.iTaskId = taskId; // needed for syncing only one content 
		}
	
	ReadProfileItemL(profile, item);
	
	//item.iMandatoryCheck = CAspProfile::CheckMandatoryData(profile, item.iTaskCount);
	
	ReplaceProfileItemL(item);

#ifdef _DEBUG
    TAspParam param2(iApplicationId, iSyncSession);
	param2.iProfile = profile;
	param2.iMode = CAspContentList::EInitAll;
	CAspContentList* contentList = CAspContentList::NewLC(param2);

    contentList->LogL(item.iProfileName);
    
    CleanupStack::PopAndDestroy(contentList);
#endif	
	
	CleanupStack::PopAndDestroy(profile);
	
	FLOG( _L("CAspProfileList::ReadProfileL END") );
	}
	
	
// -----------------------------------------------------------------------------
// CAspProfileList::ReadAllProfilesL
//
// -----------------------------------------------------------------------------
//
void CAspProfileList::ReadAllProfilesL(TInt aListMode)
	{
	FLOG( _L("CAspProfileList::ReadAllProfilesL START") );
	
#ifdef _DEBUG
    	TTime time_1 = TUtil::TimeBefore();
#endif
	
	iList.Reset();
	
	RArray<TSmlProfileId> arr;
    Session().ListProfilesL(arr, ESmlDataSync);
   	CleanupClosePushL(arr);

	TInt count = arr.Count();

	//Hiding OVI Sync profile
	const TUid KUidOtaSyncCenRep                = { 0x20016C06 };
	const TUid KCRUidOtaSyncProfileId           = { 0x102 };
	TInt oviProfileId = KErrNotFound;
	CRepository* centRep = NULL;

	TRAPD(err ,centRep = CRepository::NewL(KUidOtaSyncCenRep));
	if (err == KErrNone)
		{
		CleanupStack::PushL(centRep);
		centRep->Get(KCRUidOtaSyncProfileId.iUid, oviProfileId);
    	CleanupStack::PopAndDestroy();// centRep
		}
	for (TInt i=0; i<count; i++)
		{
		TAspParam param(iApplicationId, iSyncSession);
		CAspProfile* profile = CAspProfile::NewLC(param);
		TInt id = arr[i];
		
		//Hiding OVI Sync profile
		if (id == oviProfileId)
			{
			CleanupStack::PopAndDestroy(profile);
			continue;
		    }
		if (aListMode == EBasePropertiesOnly)
			{
			profile->OpenL(id, CAspProfile::EOpenRead, CAspProfile::EBaseProperties);
			}
		else
			{
			profile->OpenL(id, CAspProfile::EOpenRead, CAspProfile::EAllProperties);
			}
			
		
		// Hidden operator specific profile should not be visible
		if (profile->IsHiddenOperatorProfileL())
		    {
		    CleanupStack::PopAndDestroy(profile);
		    continue;
		    }

		TAspProfileItem item;
		item.Init();
		
		if (iApplicationId != EApplicationIdSync)
			{
			TInt dataProviderId = TUtil::ProviderIdFromAppId(iApplicationId, CAspProfile::IsOperatorProfileL(profile));
			TInt taskId = TAspTask::FindTaskIdL(profile, dataProviderId, KNullDesC);
			item.iTaskId = taskId; // needed for syncing only one content 
			}
		
		if (aListMode == EBasePropertiesOnly)
			{
			profile->GetName(item.iProfileName);
			if (TUtil::IsEmpty(item.iProfileName))
				{
				CAspResHandler::ReadL(item.iProfileName,R_ASP_UNNAMED_SET);
				}
			item.iApplicationId = profile->CreatorId();
			item.iProfileId = profile->ProfileId();
			}
		else
			{
			ReadProfileItemL(profile, item);
			}
		
		if (aListMode == EMandatoryCheck || aListMode == EMandatoryCheckEx)
			{
			item.iMandatoryCheck = CAspProfile::CheckMandatoryData(profile, item.iTaskCount);
			}

		if (aListMode == EMandatoryCheckEx && item.iMandatoryCheck != EMandatoryOk)
			{
			CleanupStack::PopAndDestroy(profile);
			continue;  // ignore profile that misses some mandatory data
			}
		
		if(IsAutoSyncProfile(profile))
			{
			CleanupStack::PopAndDestroy(profile);
			continue;  // auto sync profile should be hidden
			}
		
		User::LeaveIfError(iList.Append(item));
		
		CleanupStack::PopAndDestroy(profile);
		}
        
    CleanupStack::PopAndDestroy(&arr);
    
#ifdef _DEBUG    
    TUtil::TimeAfter(time_1, _L("ReadAllProfilesL took"));
#endif
    
    FLOG( _L("CAspProfileList::ReadAllProfilesL END") );
	}


// -----------------------------------------------------------------------------
// CAspProfileList::ReadEMailProfilesL
//
// -----------------------------------------------------------------------------
//
void CAspProfileList::ReadEMailProfilesL(const TDesC& aLocalDatabase, TInt /*aListMode*/)
	{
	FLOG( _L("CAspProfileList::ReadEMailProfilesL START") );
	
	iList.Reset();
	
	RArray<TSmlProfileId> arr;
    Session().ListProfilesL(arr, ESmlDataSync);
   	CleanupClosePushL(arr);

	TInt count = arr.Count();
	for (TInt i=0; i<count; i++)
		{
		TAspParam param(iApplicationId, iSyncSession);
		CAspProfile* profile = CAspProfile::NewLC(param);
		
		TInt id = arr[i];
		profile->OpenL(id, CAspProfile::EOpenRead, CAspProfile::EBaseProperties);
		
		TAspProfileItem item;
		item.Init();
		
		TInt taskId = TAspTask::FindTaskIdL(profile, KUidNSmlAdapterEMail.iUid, aLocalDatabase);
		item.iTaskId = taskId; // needed for syncing only one content
		
		if( taskId != KErrNotFound )
		{
		profile->GetName(item.iProfileName);
		item.iApplicationId = profile->CreatorId();
		item.iProfileId = profile->ProfileId();
		
		if(IsAutoSyncProfile(profile))
			{
			CleanupStack::PopAndDestroy(profile);
			continue;  // auto sync profile should be hidden
			}
		
		User::LeaveIfError(iList.Append(item));
		}
		CleanupStack::PopAndDestroy(profile);
		}
        
    CleanupStack::PopAndDestroy(&arr);
    
    FLOG( _L("CAspProfileList::ReadEMailProfilesL END") );
	}



// -----------------------------------------------------------------------------
// CAspProfileList::ReadProfileItemL
//
// -----------------------------------------------------------------------------
//
void CAspProfileList::ReadProfileItemL(CAspProfile* aProfile, TAspProfileItem& aItem)
	{
	aProfile->GetName(iBuf);
	if (TUtil::IsEmpty(iBuf))
		{
		CAspResHandler::ReadL(iBuf, R_ASP_UNNAMED_SET);
		}
    aItem.SetName(iBuf);
	
	aProfile->GetServerId(iBuf);
	TInt len = iBuf.Size();  // number of bytes
	TUint16 checkSum = 0;
	if (len > 0)
		{
		Mem::Crc(checkSum, iBuf.Ptr(), len);
		}
	aItem.iServerIdCheckSum = checkSum;

	aItem.iProfileId = aProfile->ProfileId();
	aItem.iSynced    = aProfile->IsSynced();
	aItem.iLastSync  = aProfile->LastSync();
	aItem.iBearer    = aProfile->BearerType();
    
    aItem.iActive = EFalse;
    if (aProfile->SASyncState() != ESASyncStateDisable)
    	{
    	aItem.iActive = ETrue;
    	}
    	
	aItem.iDeleteAllowed = aProfile->DeleteAllowed();
	aItem.iMandatoryCheck = EMandatoryOk;
	aItem.iApplicationId = aProfile->CreatorId();
	
	aItem.iTaskCount = 0;
    }
		
	
// -----------------------------------------------------------------------------
// CAspProfileList::Sort
//
// -----------------------------------------------------------------------------
//
void CAspProfileList::Sort()
	{
	iList.Sort(TLinearOrder<TAspProfileItem>(TAspProfileItem::CompareItems));
	}


// -----------------------------------------------------------------------------
// CAspProfileList::Remove
//
// -----------------------------------------------------------------------------
//
void CAspProfileList::Remove(TInt aProfileId)
	{
	TInt count = iList.Count();
	for (TInt i=0; i<count; i++)
		{
		TAspProfileItem& item = iList[i];
		if (item.iProfileId == aProfileId)
			{
			iList.Remove(i);
			return;
			}
		}
	}


// -----------------------------------------------------------------------------
// CAspProfileList::AddL
//
// -----------------------------------------------------------------------------
//
void CAspProfileList::AddL(TAspProfileItem& aProfileItem)
	{
	User::LeaveIfError(iList.Append(aProfileItem));
	}


// -----------------------------------------------------------------------------
// CAspProfileList::ListIndex
// 
// -----------------------------------------------------------------------------
//
TInt CAspProfileList::ListIndex(TInt aProfileId)
	{
	TInt count = iList.Count();
	for (TInt i=0; i<count; i++)
		{
		TAspProfileItem& item = iList[i];
		if (item.iProfileId == aProfileId)
			{
			return i;
			}
		}

	return KErrNotFound;
	}


// -----------------------------------------------------------------------------
// CAspProfileList::Session
// 
// -----------------------------------------------------------------------------
//
RSyncMLSession& CAspProfileList::Session()
	{
	__ASSERT_DEBUG(iSyncSession, TUtil::Panic(KErrGeneral));
	
	return *iSyncSession;
	}


// -----------------------------------------------------------------------------
// CAspProfileList::IsUniqueServerId
// 
// -----------------------------------------------------------------------------
//
TBool CAspProfileList::IsUniqueServerId(const TDesC& aServerId, TInt aProfileId)
	{
	TInt serverIdLen = aServerId.Length();
	if (serverIdLen == 0)
		{
		return ETrue;
		}

    // Operator specific serverID is not considered as unique
	if (CAspProfile::EqualsToOperatorServerId(aServerId))
	    {
	    return EFalse;
	    }

	TBool serverIdFound = EFalse;
	
	TInt count = iList.Count();
	for (TInt i=0; i<count; i++)
		{
	   	TAspProfileItem& item = iList[i];
		if (item.iProfileId == aProfileId)
			{
			continue;
			}

		TUint16 checkSum = 0;
		TInt len = aServerId.Size(); // number of bytes
	
		Mem::Crc(checkSum, aServerId.Ptr(), len);
		if (item.iServerIdCheckSum == checkSum)
			{
			serverIdFound = ETrue;
			break;
			}
		}

    if (serverIdFound)
    	{
    	return EFalse;
    	}
    	
    return ETrue;
	}


TBool CAspProfileList::IsAutoSyncProfile(CAspProfile* aProfile)
	{
	TBuf<KBufSize> profileName;
	aProfile->GetName(profileName);
	if (profileName.Compare(KAutoSyncProfileName) == 0)
		{
		return ETrue;
		}
	return EFalse;
	}

/******************************************************************************
 * class CAspProfile
 ******************************************************************************/


// -----------------------------------------------------------------------------
// CAspProfile::NewLC
//
// -----------------------------------------------------------------------------
//
CAspProfile* CAspProfile::NewLC(const TAspParam& aParam)
    {
    FLOG( _L("CAspProfile::NewLC START") );
    
    CAspProfile* self = new (ELeave) CAspProfile(aParam);
	CleanupStack::PushL(self);
	self->ConstructL();

	FLOG( _L("CAspProfile::NewLC END") );
	return self;
    }

// -----------------------------------------------------------------------------
// CAspProfile::NewL
//
// -----------------------------------------------------------------------------
//
CAspProfile* CAspProfile::NewL (const TAspParam& aParam)
    {
    FLOG( _L("CAspProfile::NewL START") );
    
    CAspProfile* self = new (ELeave) CAspProfile(aParam);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);

    FLOG( _L("CAspProfile::NewL END") );
	return self;
    }


// -----------------------------------------------------------------------------
// Destructor
//
// -----------------------------------------------------------------------------
//
CAspProfile::~CAspProfile()
    {
    if (iHistoryLogOpen)
    	{
    	iHistoryLog.Close();
    	}

    if (iConnectionOpen)
    	{
    	iConnection.Close();
    	}
    
    iProfile.Close();
    }


// -----------------------------------------------------------------------------
// CAspProfile::ConstructL
//
// -----------------------------------------------------------------------------
//
void CAspProfile::ConstructL()
    {
    }


// -----------------------------------------------------------------------------
// CAspProfile::CAspProfile
//
// -----------------------------------------------------------------------------
//
CAspProfile::CAspProfile(const TAspParam& aParam)
	{
	__ASSERT_ALWAYS(aParam.iSyncSession, TUtil::Panic(KErrGeneral));

	iSyncSession = aParam.iSyncSession;
	iApplicationId = aParam.iApplicationId;
	
	
    iHistoryLogOpen = EFalse;
    iConnectionOpen = EFalse;
	}


// -----------------------------------------------------------------------------
// CAspProfile::OpenL
//
// -----------------------------------------------------------------------------
//
void CAspProfile::OpenL(TInt aProfileId, TInt aReadWrite, TInt aOpenMode)
    {
    FLOG( _L("CAspProfile::OpenL START") );
    
    if (aReadWrite == EOpenRead)
    	{
    	iProfile.OpenL(*iSyncSession, aProfileId, ESmlOpenRead);
    	}
    else
    	{
    	iProfile.OpenL(*iSyncSession, aProfileId, ESmlOpenReadWrite);
    	}
    
    if (aOpenMode == EAllProperties)
    	{
    	OpenConnection();  // this updates iConnectionOpen
    	OpenHistoryLog();  // this updates iHistoryLogOpen
    	}
    	
    FLOG( _L("CAspProfile::OpenL END") );
    }


// -----------------------------------------------------------------------------
// CAspProfile::CreateL
//
// -----------------------------------------------------------------------------
//
void CAspProfile::CreateL(TInt aOpenMode)
    {
    FLOG( _L("CAspProfile::CreateL START") );
    
   	iProfile.CreateL(*iSyncSession);
   	iProfile.SetCreatorId(iApplicationId);  // creator application id
   	iProfile.UpdateL();
   	
	// create connection for new profile (convergence)
    //TInt transportId = DefaultTransportIdL();
    //iConnection.CreateL(iProfile, transportId);
    //iConnection.UpdateL();
    //iConnection.Close();
    //iProfile.UpdateL();	
   	
    if (aOpenMode == EAllProperties)
    	{
    	OpenConnection();  // this updates iConnectionOpen
    	OpenHistoryLog();  // this updates iHistoryLogOpen
    	}
    	
    FLOG( _L("CAspProfile::CreateL END") );
    }


// -----------------------------------------------------------------------------
// CAspProfile::DefaultTransportIdL 
// -----------------------------------------------------------------------------
//
TInt CAspProfile::DefaultTransportIdL()
    {
    TAspParam param(EApplicationIdSync, iSyncSession);
    CAspBearerHandler* bearerHandler = CAspBearerHandler::NewL(param);
    CleanupStack::PushL(bearerHandler);
    
    TInt id = bearerHandler->DefaultBearer(); 
    
    CleanupStack::PopAndDestroy(bearerHandler);
    
    if (id == KErrNotFound)
    	{
    	User::Leave(KErrNotFound);
    	}
    	
    return CAspBearerHandler::SmlBearerId(id);
    }


// -----------------------------------------------------------------------------
// CAspProfile::CreateCopyL
//
// -----------------------------------------------------------------------------
//
void CAspProfile::CreateCopyL(TInt aProfileId)
    {
    FLOG( _L("CAspProfile::CreateCopyL START") );
    
    CreateL(EAllProperties);
    
    TAspParam param(iApplicationId, iSyncSession);
    CAspProfile* profile = CAspProfile::NewLC(param);
    profile->OpenL(aProfileId, CAspProfile::EOpenRead, CAspProfile::EAllProperties);
    
    CopyValuesL(profile);
    
    CleanupStack::PopAndDestroy(profile);
    
    FLOG( _L("CAspProfile::CreateCopyL END") );
    }


// -----------------------------------------------------------------------------
// CAspProfile::SaveL
//
// -----------------------------------------------------------------------------
//
void CAspProfile::SaveL()
    {
    //iProfile.UpdateL();  // convergence
    
    if (iConnectionOpen)
    	{
    	iConnection.UpdateL();
    	}
    	
   	iProfile.UpdateL();
    }


// -----------------------------------------------------------------------------
// CAspProfile::Save
//
// -----------------------------------------------------------------------------
//
void CAspProfile::Save()
    {
    TRAPD(err, SaveL());
    
    if (err != KErrNone)
    	{
    	FLOG( _L("###  CAspProfile::SaveL failed (%d) ###"), err );
    	}
    }


// -----------------------------------------------------------------------------
// CAspProfile::GetName
//
// -----------------------------------------------------------------------------
//
void CAspProfile::GetName(TDes& aText)
	{
 		if(!IsPCSuiteProfile(this))
 		{
			TUtil::StrCopy(aText, iProfile.DisplayName());
			return;
 		}
 		//For PC suite profile Localise as per specifications
 		GetLocalisedPCSuite(aText);
	}


// -----------------------------------------------------------------------------
// CAspProfile::NameL
//
// -----------------------------------------------------------------------------
//
HBufC* CAspProfile::NameL()
	{
	return iProfile.DisplayName().AllocL();
	}


// -----------------------------------------------------------------------------
// CAspProfile::SetNameL
//
// -----------------------------------------------------------------------------
//
void CAspProfile::SetNameL(const TDesC& aText)
	{
	iProfile.SetDisplayNameL(aText);
	}


// -----------------------------------------------------------------------------
// CAspProfile::CreatorId
//
// -----------------------------------------------------------------------------
//
TInt CAspProfile::CreatorId()
	{
	return iProfile.CreatorId();
	}


// -----------------------------------------------------------------------------
// CAspProfile::SetCreatorId
//
// -----------------------------------------------------------------------------
//
void CAspProfile::SetCreatorId(TInt aCreatorId)
	{
	iProfile.SetCreatorId(aCreatorId);
	}


// -----------------------------------------------------------------------------
// CAspProfile::ProfileId
//
// -----------------------------------------------------------------------------
//
TInt CAspProfile::ProfileId()
	{
	return iProfile.Identifier();
	}


// -----------------------------------------------------------------------------
// CAspProfile::DeleteAllowed
//
// -----------------------------------------------------------------------------
//
TBool CAspProfile::DeleteAllowed()
    {
    TBool operatorProfile = EFalse;
    TRAPD( err, operatorProfile = IsReadOnlyOperatorProfileL() );

    // Disallow deletion of operator specific profile
    if( err == KErrNone && operatorProfile )
        {
        return EFalse;
        }
    else
        {
        return iProfile.DeleteAllowed();
        }
    }


// -----------------------------------------------------------------------------
// CAspProfile::IsSynced
//
// -----------------------------------------------------------------------------
//
TBool CAspProfile::IsSynced()
    {
    if (!iHistoryLogOpen)
    	{
    	return EFalse;
    	}
    	
    if (iHistoryLog.Count() == 0)
        {
        return EFalse;
        }
    
    return ETrue;
    }


// -----------------------------------------------------------------------------
// CAspProfile::LastSync
//
// -----------------------------------------------------------------------------
//
TTime CAspProfile::LastSync()
    {
    if (!iHistoryLogOpen)
    	{
    	return 0;
    	}

    if (iHistoryLog.Count() == 0)
        {
        return 0;  // profile has not been synced
        }
        
    const CSyncMLHistoryJob* job = LatestHistoryJob();
    if (!job)
    	{
    	return 0;
    	}
    
    return job->TimeStamp();
    }


// -----------------------------------------------------------------------------
// CAspProfile::LastSync
//
// -----------------------------------------------------------------------------
//
TTime CAspProfile::LastSync(TInt aTaskId)
    {
    if (!iHistoryLogOpen)
    	{
    	return 0;
    	}

    if (iHistoryLog.Count() == 0)
        {
        return 0;  // profile has not been synced
        }
        
    const CSyncMLHistoryJob* job = LatestHistoryJob(aTaskId);
    if (!job)
    	{
    	return 0;
    	}
    
    return job->TimeStamp();
    }


// -----------------------------------------------------------------------------
// CAspProfile::GetServerId
//
// -----------------------------------------------------------------------------
//
void CAspProfile::GetServerId(TDes& aText)
	{
	TUtil::StrCopy(aText, iProfile.ServerId());  // convert TDes8 to TDes
	}


// -----------------------------------------------------------------------------
// CAspProfile::SetGetServerIdL
//
// -----------------------------------------------------------------------------
//
void CAspProfile::SetServerIdL(const TDesC& aText)
	{
	TUtil::StrCopy(iBuf8, aText);  // convert TDes to TDes8
	iProfile.SetServerIdL(iBuf8);
	}


// -----------------------------------------------------------------------------
// CAspProfile::ProtocolVersion
//
// -----------------------------------------------------------------------------
//
TInt CAspProfile::ProtocolVersion()
	{
	TSmlProtocolVersion version = iProfile.ProtocolVersion();
	if (version == ESmlVersion1_1_2)
		{
		return EAspProtocol_1_1;
		}
		
	return EAspProtocol_1_2;
	}
	
	
// -----------------------------------------------------------------------------
// CAspProfile::SetProtocolVersionL
//
// -----------------------------------------------------------------------------
//
void CAspProfile::SetProtocolVersionL(TInt aProtocolVersion)
	{
	if (aProtocolVersion == EAspProtocol_1_1)
		{
		iProfile.SetProtocolVersionL(ESmlVersion1_1_2);
		}
	else
		{
		iProfile.SetProtocolVersionL(ESmlVersion1_2);
		}
    }


// -----------------------------------------------------------------------------
// CAspProfile::BearerType
//
// -----------------------------------------------------------------------------
//
TInt CAspProfile::BearerType()
	{
	if (!iConnectionOpen)
		{
	    return KErrNotFound;	
		}

	TInt id = iConnection.Identifier();
   	return CAspBearerHandler::AspBearerId(id);
    }


// -----------------------------------------------------------------------------
// CAspProfile::SetBearerTypeL
//
// -----------------------------------------------------------------------------
//
void CAspProfile::SetBearerTypeL(TInt aId)
	{
	if (!iConnectionOpen)
		{
		return;
		}
	
	TInt newBearer = CAspBearerHandler::SmlBearerId(aId);
    
    if (newBearer != KErrNotFound)
    	{
    	TInt currentBearer = iConnection.Identifier();
    	if (currentBearer != newBearer)
    		{
    		iConnection.CreateL(Profile(), newBearer);
    		}
    	}
	}



// -----------------------------------------------------------------------------
// CAspProfile::SetBearerTypeL (convergence)
//
// -----------------------------------------------------------------------------
//
/*
void CAspProfile::SetBearerTypeL(TInt aId)
	{
	TInt newBearer = CAspBearerHandler::SmlBearerId(aId);
	if (newBearer == KErrNotFound)
		{
		return;  // unknown bearer
		}
	
   	TInt currentBearer = KErrNotFound;
   	if (iConnectionOpen)
   	    {
   		currentBearer = iConnection.Identifier();
   	    }
   	if (currentBearer == newBearer)
   		{
   		return;	// bearer has not changed
   		}

	if (iConnectionOpen)
	    {
		iConnection.Close();
		iConnectionOpen = EFalse;
	    }

	DeleteConnectionsL(0);
    			
	TRAPD(err, iConnection.CreateL(iProfile, newBearer));
   	if (err == KErrNone)
    	{
    	iConnection.UpdateL();
        iConnection.Close();
        iProfile.UpdateL();
        iConnection.OpenL(iProfile, newBearer);
        iConnectionOpen = ETrue;    		    				    
      	}
    }
*/


// -----------------------------------------------------------------------------
// CAspProfile::DeleteConnectionsL (convergence)
//
// -----------------------------------------------------------------------------
//
/*
void CAspProfile::DeleteConnectionsL()
    {
   	RArray<TSmlConnectionId> arr;
   	Profile().ListConnectionsL(arr);
   	CleanupClosePushL(arr);
   	
   	TInt count = arr.Count();
    	
   	for (TInt i=0; i<count; i++)
   	    {
   	    TInt id = arr[i];
   	    iProfile.DeleteConnectionL(id);
   	    iProfile.UpdateL();
   	    }
   	    
   	CleanupStack::PopAndDestroy(&arr);
    }
*/



// -----------------------------------------------------------------------------
// CAspProfile::AccessPointL
//
// -----------------------------------------------------------------------------
//
TInt CAspProfile::AccessPointL()
	{
	if (!iConnectionOpen)
		{
    	return KErrNotFound;
		}
	
	if (iConnection.Identifier() != KUidNSmlMediumTypeInternet.iUid)
		{
		return KErrNotFound; // this setting is only for internet bearer
		}
		
	TBuf8<KBufSize32> key;
	TBuf<KBufSize32> value;
	TInt num = KErrNotFound;

	GetConnectionPropertyNameL(key, EPropertyIntenetAccessPoint);
	TUtil::StrCopy(value, iConnection.GetPropertyL(key)); // convert TDes8 to TDes
	if (TUtil::IsEmpty(value))
		{
		return KErrNotFound;
		}
		
	User::LeaveIfError(TUtil::StrToInt(value, num));
	return num;
	}
		
	
// -----------------------------------------------------------------------------
// CAspProfile::SetAccessPointL
//
// -----------------------------------------------------------------------------
//
void CAspProfile::SetAccessPointL(const TInt aId)
	{
    if (!iConnectionOpen)	
    	{
    	return;
    	}
    	
	if (iConnection.Identifier() != KUidNSmlMediumTypeInternet.iUid)
		{
		return; // this setting is only for internet bearer
		}

	TBuf<KBufSize32> buf;
	TBuf8<KBufSize32> key;
	TBuf8<KBufSize32> value;
	
	buf.Num(aId);
	TUtil::StrCopy(value, buf);  // convwert TDes to TDes8
	
	GetConnectionPropertyNameL(key, EPropertyIntenetAccessPoint);
	iConnection.SetPropertyL(key, value);
    }


// -----------------------------------------------------------------------------
// CAspProfile::GetHostAddress
//
// -----------------------------------------------------------------------------
//
void CAspProfile::GetHostAddress(TDes& aText, TInt& aPort)
	{
	aText = KNullDesC;
	aPort = TURIParser::EDefaultHttpPort;
	
    if (!iConnectionOpen)	
    	{
      	return;
    	}
	
	if (BearerType() != EAspBearerInternet)
		{
		TUtil::StrCopy(aText, iConnection.ServerURI()); // convert TDes8 to TDes
		return;  // port number handling is for internet bearer only 
		}

	TUtil::StrCopy(iBuf, iConnection.ServerURI()); // convert TDes8 to TDes
	
	TURIParser parser(iBuf);
	
	parser.GetUriWithoutPort(aText);

	aPort = parser.Port();
	if (aPort == KErrNotFound)
		{
		aPort = parser.DefaultPort();
		}
	}
	
	
// -----------------------------------------------------------------------------
// CAspProfile::SetHostAddressL
//
// -----------------------------------------------------------------------------
//
void CAspProfile::SetHostAddressL(const TDesC& aText, const TInt aPort)
	{
    if (!iConnectionOpen)	
    	{
    	return;
    	}
	
	if (BearerType() != EAspBearerInternet)
		{
		TUtil::StrCopy(iBuf, aText); // port number handling is for inteernet bearer only 
		}
	else
		{
		TURIParser parser(aText); // add port number to internet host address
	    parser.GetUriWithPort(iBuf, aPort);
		}
	
	TUtil::StrCopy(iBuf8, iBuf);  // convert TDes to TDes8
	iConnection.SetServerURIL(iBuf8);
	}


// -----------------------------------------------------------------------------
// CAspProfile::GetUserName
//
// -----------------------------------------------------------------------------
//
void CAspProfile::GetUserName(TDes& aText)
	{
	TUtil::StrCopy(aText, iProfile.UserName());   // convert TDes8 to TDes
	}
	
	
// -----------------------------------------------------------------------------
// CAspProfile::SetUserNameL
//
// -----------------------------------------------------------------------------
//
void CAspProfile::SetUserNameL(const TDesC& aText)
	{
	TUtil::StrCopy(iBuf8, aText);  // conver TDes to TDes8
	iProfile.SetUserNameL(iBuf8);
	}


// -----------------------------------------------------------------------------
// CAspProfile::GetPassword
//
// -----------------------------------------------------------------------------
//
void CAspProfile::GetPassword(TDes& aText)
	{
	TUtil::StrCopy(aText, iProfile.Password());   // convert TDes8 to TDes
	}
	
	
// -----------------------------------------------------------------------------
// CAspProfile::SetPasswordL
//
// -----------------------------------------------------------------------------
//
void CAspProfile::SetPasswordL(const TDesC& aText)
	{
	TUtil::StrCopy(iBuf8, aText);    // conver TDes to TDes8
	iProfile.SetPasswordL(iBuf8);
	}

	
// -----------------------------------------------------------------------------
// CAspProfile::SASyncState
//
// -----------------------------------------------------------------------------
//
TInt CAspProfile::SASyncState()
	{
	TSmlServerAlertedAction state = iProfile.SanUserInteraction();

	if (state == ESmlConfirmSync)
		{
		return ESASyncStateConfirm;
		}
	else if (state == ESmlDisableSync)
		{
		return ESASyncStateDisable;
		}
	else 
		{
		return ESASyncStateEnable;
		}
	}


// -----------------------------------------------------------------------------
// CAspProfile::SetSASyncStateL
//
// -----------------------------------------------------------------------------
//
void CAspProfile::SetSASyncStateL(TInt aState)
    {
	if (aState == ESASyncStateConfirm)
		{
		Profile().SetSanUserInteractionL(ESmlConfirmSync);
		}
	else if (aState == ESASyncStateDisable)
		{
		Profile().SetSanUserInteractionL(ESmlDisableSync);
		}
	else
		{
		Profile().SetSanUserInteractionL(ESmlEnableSync);
		}
    }


// -----------------------------------------------------------------------------
// CAspProfile::SetHttpUsedL
//
// -----------------------------------------------------------------------------
//
void CAspProfile::SetHttpUsedL(TBool aEnable)
	{
    if (!iConnectionOpen)	
    	{
    	return;
    	}

	if (iConnection.Identifier() != KUidNSmlMediumTypeInternet.iUid)
		{
		return; // this setting is only for internet bearer
		}
	
	TBuf8<KBufSize32> key;
	TBuf8<KBufSize32> value;
	
	if (aEnable)
		{
		value.Num(1); 
		}
	else
		{
		value.Num(0);
		}
		
	GetConnectionPropertyNameL(key, EPropertyHttpUsed);
	iConnection.SetPropertyL(key, value);
	}
	
	
// -----------------------------------------------------------------------------
// CAspProfile::HttpUsedL
//
// -----------------------------------------------------------------------------
//
TBool CAspProfile::HttpUsedL()
	{
    if (!iConnectionOpen)	
    	{
    	return EFalse;
    	}
	
	if (iConnection.Identifier() != KUidNSmlMediumTypeInternet.iUid)
		{
		return EFalse; // this setting is only for internet bearer
		}

	TBuf8<KBufSize32> key; 
	TBuf<KBufSize32> value;
	TInt intValue;
	
	GetConnectionPropertyNameL(key, EPropertyHttpUsed);
	TUtil::StrCopy(value, iConnection.GetPropertyL(key));  // convert TDes8 to TDes
	
    User::LeaveIfError(TUtil::StrToInt(value, intValue));
	
    if (intValue == 0)
    	{
    	return EFalse; 
    	}
    return ETrue;
	}
	

// -----------------------------------------------------------------------------
// CAspProfile::GetHttpUsernameL
//
// -----------------------------------------------------------------------------
//
void CAspProfile::GetHttpUserNameL(TDes& aText)
	{
	aText = KNullDesC;
	
    if (!iConnectionOpen)	
    	{
    	return;
    	}

	if (iConnection.Identifier() != KUidNSmlMediumTypeInternet.iUid)
		{
		return; // this setting is only for internet bearer
		}
	

	TBuf8<KBufSize32> key; 
	
	GetConnectionPropertyNameL(key, EPropertyHttpUserName);
	TUtil::StrCopy(aText, iConnection.GetPropertyL(key));  // convert TDes8 to TDes
  	}
	

// -----------------------------------------------------------------------------
// CAspProfile::SetHttpUsernameL
//
// -----------------------------------------------------------------------------
//
void CAspProfile::SetHttpUserNameL(const TDesC& aText)
	{
    if (!iConnectionOpen)	
    	{
    	return;
    	}
	
	if (iConnection.Identifier() != KUidNSmlMediumTypeInternet.iUid)
		{
		return; // this setting is only for internet bearer
		}

	TBuf8<KBufSize32> key;
	
	GetConnectionPropertyNameL(key, EPropertyHttpUserName);
	TUtil::StrCopy(iBuf8, aText);  // convert TDes to TDes8
	iConnection.SetPropertyL(key, iBuf8);
	}


// -----------------------------------------------------------------------------
// CAspProfile::GetHttpPasswordL
//
// -----------------------------------------------------------------------------
//
void CAspProfile::GetHttpPasswordL(TDes& aText)
	{
   	aText = KNullDesC;
   	
    if (!iConnectionOpen)	
    	{
    	return;
    	}

	if (iConnection.Identifier() != KUidNSmlMediumTypeInternet.iUid)
		{
		return; // this setting is only for internet bearer
		}


	TBuf8<KBufSize32> key; 
	
	GetConnectionPropertyNameL(key, EPropertyHttpPassword);
	TUtil::StrCopy(aText, iConnection.GetPropertyL(key));  // convert TDes8 to TDes
	}
	

// -----------------------------------------------------------------------------
// CAspProfile::SetHttpPasswordL
//
// -----------------------------------------------------------------------------
//
void CAspProfile::SetHttpPasswordL(const TDesC& aText)
	{
    if (!iConnectionOpen)	
    	{
    	return;
    	}
	
	if (iConnection.Identifier() != KUidNSmlMediumTypeInternet.iUid)
		{
		return; // this setting is only for internet bearer
		}
    		
	TBuf8<KBufSize32> key;
	
	GetConnectionPropertyNameL(key, EPropertyHttpPassword);
	TUtil::StrCopy(iBuf8, aText);  // convert TDes to TDes8
	iConnection.SetPropertyL(key, iBuf8);
	}


// -----------------------------------------------------------------------------
// CAspProfile::Profile
//
// -----------------------------------------------------------------------------
//
RSyncMLDataSyncProfile& CAspProfile::Profile()
    {
    return iProfile;
    }


// -----------------------------------------------------------------------------
// CAspProfile::LatestHistoryJob
//
// -----------------------------------------------------------------------------
//
const CSyncMLHistoryJob* CAspProfile::LatestHistoryJob()
    {
    if (!iHistoryLogOpen)
    	{
    	return NULL;
    	}

    TInt count = iHistoryLog.Count();
    if (count == 0)
        {
        return NULL;  // profile has no history job
        }
        
    iHistoryLog.SortEntries(CSyncMLHistoryEntry::ESortByTime);    
    
    // try to find latest sync job (start from last array entry)
    for (TInt i=count-1; i>=0; i--)
    	{
        const CSyncMLHistoryEntry& entry = iHistoryLog[i];
        const CSyncMLHistoryJob* jobEntry = CSyncMLHistoryJob::DynamicCast(&entry);
        if (jobEntry)
        	{
        	return jobEntry;
        	}
    	}

    return NULL;  // profile has no history job
    }


// -----------------------------------------------------------------------------
// CAspProfile::LatestHistoryJob
//
// -----------------------------------------------------------------------------
//
const CSyncMLHistoryJob* CAspProfile::LatestHistoryJob(TInt aTaskId)
    {
    if (!iHistoryLogOpen)
    	{
    	return NULL;
    	}

    TInt count = iHistoryLog.Count();
    if (count == 0)
        {
        return NULL;  // profile has no history job
        }
        
    iHistoryLog.SortEntries(CSyncMLHistoryEntry::ESortByTime);    
    
    // try to find latest sync job (start from last array entry)
    for (TInt i=count-1; i>=0; i--)
    	{
        const CSyncMLHistoryEntry& entry = iHistoryLog[i];
        const CSyncMLHistoryJob* jobEntry = CSyncMLHistoryJob::DynamicCast(&entry);
        if (jobEntry)
        	{
        	if (TaskExist(jobEntry, aTaskId))
        		{
        		return jobEntry;
        		}
        	}
    	}

    return NULL;
    }

// -----------------------------------------------------------------------------
// CAspProfile::DeleteHistory
//
// -----------------------------------------------------------------------------
//
void CAspProfile::DeleteHistory()
	{
	if (iHistoryLogOpen)
    	{
    	iHistoryLog.DeleteAllEntriesL();	
    	}  
    	
	}


// -----------------------------------------------------------------------------
// CAspProfile::TaskExist
// 
// -----------------------------------------------------------------------------
TBool CAspProfile::TaskExist(const CSyncMLHistoryJob* aHistoryJob, TInt aTaskId)
	{
	TInt taskCount = aHistoryJob->TaskCount();
	for (TInt i=0; i<taskCount; i++)
		{
		const CSyncMLHistoryJob::TTaskInfo& taskInfo = aHistoryJob->TaskAt(i);
        
        if (taskInfo.iTaskId == aTaskId)
        	{
        	return ETrue;
        	}
		}
		
	return EFalse;
	}



/* 
// -----------------------------------------------------------------------------
// CAspProfile::HistoryJobsLC (debugging code)
//
// -----------------------------------------------------------------------------
//
HBufC* CAspProfile::HistoryJobsLC()
    {
   	HBufC* hBuf = HBufC::NewLC(1024);
	TPtr ptr = hBuf->Des();

    if (!iHistoryLogOpen)
    	{
    	return hBuf;
    	}

    TInt count = iHistoryLog.Count();
    if (count == 0)
        {
        return hBuf;  // profile has no history job
        }
        
    // sort array
    iHistoryLog.SortEntries(CSyncMLHistoryEntry::ESortByTime);    
    
    // try to find latest sync job (start from last array entry)
    for (TInt i=count-1; i>=0; i--)
    //for (TInt i=0; i<count; i++)
    	{
        const CSyncMLHistoryEntry& entry = iHistoryLog[i];
        const CSyncMLHistoryJob* jobEntry = CSyncMLHistoryJob::DynamicCast(&entry);
        if (jobEntry)
        	{
        	TTime t = jobEntry->TimeStamp();
        	TBuf<128> buf;
        	TUtil::GetDateTimeTextL(buf, t);
        	ptr.Append(buf);
        	ptr.Append(_L("\n"));
        	}
    	}

    return hBuf;
    }
*/


// -----------------------------------------------------------------------------
// CAspProfile::OpenHistoryLog
//
// -----------------------------------------------------------------------------
//
void CAspProfile::OpenHistoryLog()
    {
    TInt id = ProfileId();
   	TRAPD(err, iHistoryLog.OpenL(Session(), id));
   	if (err == KErrNone)
   		{
   		iHistoryLogOpen = ETrue;
    	}
    }


// -----------------------------------------------------------------------------
// CAspProfile::OpenConnection
//
// -----------------------------------------------------------------------------
//
void CAspProfile::OpenConnection()
    {
   	TInt err = KErrNone;
   	RArray<TSmlTransportId> arr;
   	
    TRAP(err, Profile().ListConnectionsL(arr));
    if (err != KErrNone)
    	{
    	return; 
    	}
    if (arr.Count() == 0)
    	{
     	arr.Close();  
    	return; // no connection
    	}
        	
    TInt transportId = arr[0];
    arr.Close();
    	
    TRAP(err, iConnection.OpenL(iProfile, transportId));
    if (err == KErrNone)
    	{
    	iConnectionOpen = ETrue;
    	}
    }


// -----------------------------------------------------------------------------
// CAspProfile::Session
// 
// -----------------------------------------------------------------------------
//
RSyncMLSession& CAspProfile::Session()
	{
	__ASSERT_DEBUG(iSyncSession, TUtil::Panic(KErrGeneral));
	
	return *iSyncSession;
	}


// -----------------------------------------------------------------------------
// CAspProfile::ApplicationId
// 
// -----------------------------------------------------------------------------
//
TInt CAspProfile::ApplicationId()
	{
	return iApplicationId;
	}


// -----------------------------------------------------------------------------
// CAspProfile::GetConnectionPropertyNameL
//
// -----------------------------------------------------------------------------
//
void CAspProfile::GetConnectionPropertyNameL(TDes8& aText, TInt aPropertyPos)
	{
	//
	// at the moment RSyncMLTransport is only needed for internet connection settings
	//
	RSyncMLTransport transport;
	transport.OpenL(Session(), KUidNSmlMediumTypeInternet.iUid);  // no ICP call
	CleanupClosePushL(transport);
	
	const CSyncMLTransportPropertiesArray&  arr = transport.Properties();
	
    __ASSERT_DEBUG(arr.Count()>aPropertyPos, TUtil::Panic(KErrGeneral));
    	
	const TSyncMLTransportPropertyInfo& info = arr.At(aPropertyPos);
	aText = info.iName;
	
	CleanupStack::PopAndDestroy(&transport);
	}


// -----------------------------------------------------------------------------
// CAspProfile::CopyValuesL
//
// -----------------------------------------------------------------------------
//
void CAspProfile::CopyValuesL(CAspProfile* aSource)
    {
    TBuf<KBufSize255> buf;
    TInt num = 0;
    
    num = aSource->ProtocolVersion();
    SetProtocolVersionL(num);
    if (num == EAspProtocol_1_2)
    	{
    	if (!IsPCSuiteProfile(aSource))
    		{
        	aSource->GetServerId(buf);
        	SetServerIdL(buf);
    		}
    	}
    	
    num = aSource->BearerType();
    SetBearerTypeL(num);
    
    num = aSource->AccessPointL();

	//check if the access point is valid
	TAspAccessPointItem item;
	item.iUid = num;
	TAspParam param(iApplicationId, iSyncSession);
	CAspAccessPointHandler* apHandler = CAspAccessPointHandler::NewL(param);
	CleanupStack::PushL(apHandler);
	TInt ret = apHandler->GetInternetApInfo(item);
	if (ret == KErrNone)
	    {
	    SetAccessPointL(num);
	    }
	else
	    {
	    SetAccessPointL(CAspAccessPointHandler::KDefaultConnection);
	    }    
    CleanupStack::PopAndDestroy(apHandler);
	
    aSource->GetHostAddress(buf, num);
    SetHostAddressL(buf, num);
     
    aSource->GetUserName(buf);
    SetUserNameL(buf);
     
    aSource->GetPassword(buf);
    SetPasswordL(buf);
    
    num = aSource->SASyncState();
    SetSASyncStateL(num);
    
    if (aSource->BearerType() == EAspBearerInternet)
    	{
    	num = aSource->HttpUsedL();
    	SetHttpUsedL(num);
    	
    	aSource->GetHttpUserNameL(buf);
    	SetHttpUserNameL(buf);
    	
    	aSource->GetHttpPasswordL(buf);
    	SetHttpPasswordL(buf);
    	}
    }


// -----------------------------------------------------------------------------
// CAspProfile::SetDefaultValuesL
// 
// -----------------------------------------------------------------------------
//
void CAspProfile::SetDefaultValuesL(CAspProfile* aProfile)
	{
	aProfile->SetSASyncStateL(ESASyncStateConfirm);
	aProfile->SetHttpUsedL(EFalse);
	aProfile->SetAccessPointL(CAspAccessPointHandler::KDefaultConnection);
	aProfile->SetProtocolVersionL(EAspProtocol_1_2);
    }


// -----------------------------------------------------------------------------
// CAspProfile::GetNewProfileNameL
// 
// -----------------------------------------------------------------------------
//
HBufC* CAspProfile::GetNewProfileNameLC(CAspProfileList* aList, TInt aApplicationId)
	{
    HBufC* hBuf = NULL;
    
    for (TInt i=1; i<=KMaxProfileCount; i++)
		{
        hBuf = CAspResHandler::GetNewProfileNameLC(i, aApplicationId);
		if (aList->FindProfileIndex(hBuf->Des()) == KErrNotFound)
			{
			break;
     		}
     	
     	if (i == KMaxProfileCount)
     		{
     		break; // too many profiles - use existing name
     		}
     		
     	CleanupStack::PopAndDestroy(hBuf);
     	hBuf = NULL;
		}
		
	if (!hBuf)
		{
		User::Leave(KErrNotFound);
		}
		
	return hBuf;
	}


// -----------------------------------------------------------------------------
// CAspProfile::CheckMandatoryDataL
// 
// -----------------------------------------------------------------------------
//
TInt CAspProfile::CheckMandatoryData(CAspProfile* aProfile, TInt& aContentCount)
	{
	aContentCount = 0;
	
	TInt ret = CheckMandatoryConnData(aProfile);
	if (ret != EMandatoryOk)
		{
		return ret;
		}

    ret = CheckMandatoryTaskData(aProfile, aContentCount);
    
    return ret;
	}


// -----------------------------------------------------------------------------
// CAspProfile::CheckMandatoryConnData
//
// -----------------------------------------------------------------------------
//
TInt CAspProfile::CheckMandatoryConnData(CAspProfile* aProfile)
	{
	const TInt KMaxHostAddressLength = KAspMaxURILength + 5;
	
	TBuf<KMaxHostAddressLength> buf;
	TInt num;

	aProfile->GetName(buf);
	if (TUtil::IsEmpty(buf))
		{
		return EMandatoryNoProfileName;
		}

	aProfile->GetHostAddress(buf, num);
	if (TUtil::IsEmpty(buf))
		{
		return EMandatoryNoHostAddress;
		}

    return EMandatoryOk;
	}


// -----------------------------------------------------------------------------
// CAspProfile::CheckMandatoryTaskDataL
// 
// -----------------------------------------------------------------------------
//
TInt CAspProfile::CheckMandatoryTaskDataL(CAspProfile* aProfile, TInt& aContentCount)
	{
    TAspParam param(aProfile->ApplicationId(), &(aProfile->Session()));
	param.iProfile = aProfile;
	param.iMode = CAspContentList::EInitTasks;
	
	CAspContentList* list = CAspContentList::NewLC(param);
    
    TInt ret = list->CheckMandatoryDataL(aContentCount);
	CleanupStack::PopAndDestroy(list);
	
	return ret;
	}


// -----------------------------------------------------------------------------
// CAspProfile::CheckMandatoryTaskData
// 
// -----------------------------------------------------------------------------
//
TInt CAspProfile::CheckMandatoryTaskData(CAspProfile* aProfile, TInt& aContentCount)
	{
	TInt ret = EMandatoryNoContent;
	
	TRAP_IGNORE(ret = CheckMandatoryTaskDataL(aProfile, aContentCount));
	
	return ret;
	}


// -----------------------------------------------------------------------------
// CAspProfile::OtherSyncRunning
//
// -----------------------------------------------------------------------------
//
TBool CAspProfile::OtherSyncRunning(RSyncMLSession* aSyncSession)
	{
	TInt currentJob = CurrentJob(aSyncSession);

    if (currentJob != KErrNotFound)
		{
		return ETrue;
		}
	
	return EFalse;
	}


// -----------------------------------------------------------------------------
// CAspProfile::CurrentJob
//
// -----------------------------------------------------------------------------
//
TInt CAspProfile::CurrentJob(RSyncMLSession* aSyncSession)
	{
	TInt currentJob = KErrNotFound;
	TSmlUsageType usageType = ESmlDataSync;
	TRAP_IGNORE(aSyncSession->CurrentJobL(currentJob, usageType));
		
	if (currentJob >= 0)
		{
		return currentJob;	
		}
	return KErrNotFound;
	}


//-----------------------------------------------------------------------------
// CAspProfile:::IsPCSuiteProfile
// 
//-----------------------------------------------------------------------------
//
TBool CAspProfile::IsPCSuiteProfile(CAspProfile* aProfile)
	{
	_LIT(KPCSuite, "pc suite");

	if (aProfile->DeleteAllowed())
		{
		return EFalse; // this is not "PC Suite" profile 
		}
	
	TInt num = KErrNotFound;
	TBuf<KBufSize64> buf;
	aProfile->GetHostAddress(buf, num);
	buf.LowerCase();
	
	if (buf.Compare(KPCSuite) == 0)
		{
		return ETrue;
		}
		
	return EFalse;
	}

// -----------------------------------------------------------------------------
// CAspProfile::GetLocalisedPCSuite
//
// Returns "PC Suite" string localised to specific language
// 
// Current requirement is to localise to 2 languages TaiwanChinese and ELangHongKongChinese
// For all the rest it will be mapped to "PC suite" only
// -----------------------------------------------------------------------------
//
void CAspProfile::GetLocalisedPCSuite(TDes& aText)
{
 		TLanguage language = User::Language();
         
 		switch(language)
 		{
 			case ELangTaiwanChinese:
            case ELangHongKongChinese:
 			case ELangEnglish_HongKong:
 			case ELangPrcChinese:
 			case ELangEnglish_Taiwan:
			case ELangEnglish_Prc:
 			case ELangMalay_Apac:
 			case ELangIndon_Apac: 
            {
            	HBufC* hBuf = CAspResHandler::ReadLC(R_ASP_PC_SUITE_PROFILE);
            	TUtil::StrCopy(aText,*hBuf);
            	CleanupStack::PopAndDestroy(hBuf);
            	return;
            }
            default:
            {
            	TUtil::StrCopy(aText, iProfile.DisplayName());
            	return;
            }
 		}
}

//-----------------------------------------------------------------------------
// CAspProfile:::IsOperatorProfileL
// 
//-----------------------------------------------------------------------------
//
TBool CAspProfile::IsOperatorProfileL( CAspProfile* aProfile )
    {
    if ( !aProfile )
        {
        return EFalse;
        }

    TBuf<KBufSize255> serverId;
    TBuf8<KBufSize255> serverIdUtf8;
    TBuf8<KBufSize255> value;
    
    aProfile->GetServerId( serverId );
    if ( serverId.Length() > 0 )
        {
        serverIdUtf8.Copy( serverId );
    
        CRepository* rep = CRepository::NewLC( KCRUidOperatorDatasyncInternalKeys );
        TInt err = rep->Get( KNsmlOpDsOperatorSyncServerId, value );
        CleanupStack::PopAndDestroy( rep );
    
        if ( !err && serverIdUtf8.Compare( value ) == 0 )
            {
            return ETrue;
            }
        }
    return EFalse;
    }

//-----------------------------------------------------------------------------
// CAspProfile:::OperatorAdapterUidL
// 
//-----------------------------------------------------------------------------
//
TInt CAspProfile::OperatorAdapterUidL()
    {
    TInt value = 0;
    CRepository* rep = CRepository::NewLC( KCRUidOperatorDatasyncInternalKeys );
    rep->Get( KNsmlOpDsOperatorAdapterUid, value );
    CleanupStack::PopAndDestroy( rep );
    return value;
    }

//-----------------------------------------------------------------------------
// CAspProfile:::ProfileAdapterUidL
// 
//-----------------------------------------------------------------------------
//
TInt CAspProfile::ProfileAdapterUidL()
    {
    TInt value = 0;
    CRepository* rep = CRepository::NewLC( KCRUidOperatorDatasyncInternalKeys );
    rep->Get( KNsmlOpDsProfileAdapterUid, value );
    CleanupStack::PopAndDestroy( rep );
    return value;
    }

//-----------------------------------------------------------------------------
// CAspProfile:::ProfileVisibilityL
// 
//-----------------------------------------------------------------------------
//
TInt CAspProfile::ProfileVisibilityL()
    {
    TInt value = 0;
    CRepository* rep = CRepository::NewLC( KCRUidOperatorDatasyncInternalKeys );
    rep->Get( KNsmlOpDsSyncProfileVisibility, value );
    CleanupStack::PopAndDestroy( rep );
    return value;
    }

//-----------------------------------------------------------------------------
// CAspProfile:::IsReadOnlyOperatorProfileL
// 
//-----------------------------------------------------------------------------
//
TBool CAspProfile::IsReadOnlyOperatorProfileL()
    {
    return ( IsOperatorProfileL( this ) && 
        ProfileVisibilityL() == EProfileVisibilityReadOnly );
    }

//-----------------------------------------------------------------------------
// CAspProfile:::IsHiddenOperatorProfileL
// 
//-----------------------------------------------------------------------------
//
TBool CAspProfile::IsHiddenOperatorProfileL()
    {
    return ( IsOperatorProfileL( this ) && 
        ProfileVisibilityL() == EProfileVisibilityHidden );
    }

//-----------------------------------------------------------------------------
// CAspProfile:::EqualsToOperatorServerIdL
// 
//-----------------------------------------------------------------------------
//
TBool CAspProfile::EqualsToOperatorServerId( const TDesC& aServerId )
    {
    if ( aServerId.Length() > 0 )
        {
        TBuf8<KBufSize255> serverIdUtf8;
        TBuf8<KBufSize255> value;
        serverIdUtf8.Copy( aServerId );

        // Read operator specific serverId from cenrep
        CRepository* rep = NULL;
        TRAPD( err, rep = CRepository::NewL( KCRUidOperatorDatasyncInternalKeys ) );
        if( err == KErrNone )
            {
            rep->Get( KNsmlOpDsOperatorSyncServerId, value );
            delete rep;

            if ( serverIdUtf8.Compare( value ) == 0 )
                {
                return ETrue;
                }
            }
        }

    return EFalse;
    }

/*******************************************************************************
 * class TAspTask
 *******************************************************************************/



// -----------------------------------------------------------------------------
// TAspTask::SmlSyncDirection
//
// -----------------------------------------------------------------------------
//
TSmlSyncType TAspTask::SmlSyncDirection(TInt aSyncDirection)
	{
	TSmlSyncType type = ESmlTwoWay;
	
	if (aSyncDirection == ESyncDirectionOneWayFromDevice)
		{
		type = ESmlOneWayFromClient;
		}
	if (aSyncDirection == ESyncDirectionOneWayIntoDevice)
		{
		type = ESmlOneWayFromServer;
		}
	if (aSyncDirection == ESyncDirectionRefreshFromServer)
		{
		type = ESmlRefreshFromServer;
		}
		
	return type;
	}


// -----------------------------------------------------------------------------
// TAspTask::SmlSyncDirection
//
// -----------------------------------------------------------------------------
//
TInt TAspTask::SyncDirection(TSmlSyncType aSmlSyncDirection)
    {
	TInt type = ESyncDirectionTwoWay;
	
	if (aSmlSyncDirection == ESmlOneWayFromClient)
		{
		type = ESyncDirectionOneWayFromDevice;
		}
	if (aSmlSyncDirection == ESmlOneWayFromServer)
		{
		type = ESyncDirectionOneWayIntoDevice;
		}
	if (aSmlSyncDirection == ESmlRefreshFromServer)
		{
		type = ESyncDirectionRefreshFromServer;
		}
		

	return type;
	}


// -----------------------------------------------------------------------------
// TAspTask::FindTaskIdL
//
// -----------------------------------------------------------------------------
//

TInt TAspTask::FindTaskIdL(CAspProfile* aProfile, TInt aDataProviderId, const TDesC& aLocalDatabase)
	{
	TInt ret = KErrNotFound;
	
	RArray<TSmlTaskId> arr;
	aProfile->Profile().ListTasksL(arr); // IPC call
	CleanupClosePushL(arr);
	
	TInt count = arr.Count();
	for (TInt i=0; i<count; i++)
		{
		TInt taskId = arr[i];
	    RSyncMLTask task;
    	task.OpenL(aProfile->Profile(), taskId);  // IPC call
		
	    if (aDataProviderId != task.DataProvider())
	    	{
	    	task.Close();
	    	continue; // wrong data provider
	    	}
    	if (!task.Enabled())
    		{
    		task.Close();
    		continue; // task not in use
    		}

	    if (TUtil::IsEmpty(aLocalDatabase))
	    	{
	    	ret = taskId;  // parameter aLocalDatabase is empty - no db name check
	    	}
	    else
	    	{
	    	TPtrC ptr = task.ClientDataSource();
	        if (ptr.Compare(aLocalDatabase) == 0)
	    	    {
	    	    ret = taskId;
	    	    }
	    	}
	
	    task.Close();
		
		if (ret != KErrNotFound)
			{
			break;
			}
		}
		
    CleanupStack::PopAndDestroy(&arr);		
    
    return ret;
	}


// -----------------------------------------------------------------------------
// TAspTask::CheckLocalDatabase
// 
// -----------------------------------------------------------------------------
//
TInt TAspTask::CheckLocalDatabase(CAspProfile* aProfile, TInt& aDataProviderId)
	{
	TBool ret = ETrue;
	
	TRAPD(err, ret = CheckLocalDatabaseL(aProfile, aDataProviderId));
	if (err != KErrNone)
		{
		return ETrue; // check failed, ETrue means profile has local database
		}
		
	return ret;
	}


// -----------------------------------------------------------------------------
// TAspTask::CheckLocalDatabaseL
//
// -----------------------------------------------------------------------------
//

TBool TAspTask::CheckLocalDatabaseL(CAspProfile* aProfile, TInt& aDataProviderId)
	{
	FLOG( _L("TAspTask::CheckLocalDatabaseL START") );
	
	TInt ret = ETrue;
	aDataProviderId = KErrNotFound;
	
	RArray<TSmlTaskId> arr;
	aProfile->Profile().ListTasksL(arr); // IPC call
	CleanupClosePushL(arr);
	
	TInt count = arr.Count();
	for (TInt i=0; i<count; i++)
		{
		// open sync task
		RSyncMLTask task;
        TInt taskId = arr[i];
    	TRAPD(err, task.OpenL(aProfile->Profile(), taskId));  // IPC call
	    if (err != KErrNone)
		    {
		    FTRACE( RDebug::Print(_L("### RSyncMLTask::OpenL failed (id=%d, err=%d) ###"), taskId, err) );
	        User::Leave(err);
		    }
		    
		CleanupClosePushL(task);
		    
    	TInt dataProviderId = task.DataProvider();
    	TBool enabled = task.Enabled();
    	
        if (dataProviderId != KUidNSmlAdapterEMail.iUid)
        	{
	    	CleanupStack::PopAndDestroy(&task);
	    	continue;  // only email sync task is checked
        	}

	    if (!enabled)
	    	{
	    	CleanupStack::PopAndDestroy(&task);
	    	continue;  // this task is not included in sync
	    	}
     	
        
        // open data provider
       	RSyncMLDataProvider dataProvider;
        TRAPD(err2, dataProvider.OpenL(aProfile->Session(), dataProviderId));
   	    if (err2 != KErrNone)
		    {
		    FTRACE( RDebug::Print(_L("### RSyncMLDataProvider::OpenL failed (id=%xd, err=%d) ###"), dataProviderId, err2) );
	        User::Leave(err);
		    }
		CleanupClosePushL(dataProvider);
        
        TPtrC localDatabase = task.ClientDataSource();
        
        if (TUtil::IsEmpty(localDatabase))
        	{
        	TPtrC defaultDatabase = dataProvider.DefaultDataStoreName();
        	if (TUtil::IsEmpty(defaultDatabase))
        		{
        		aDataProviderId = dataProviderId;
        		ret = EFalse;
        		}
        	}
        else
        	{
        	CDesCArrayFlat* localDataStores= new (ELeave) CDesCArrayFlat(KDefaultArraySize);
        	CleanupStack::PushL(localDataStores);
        	dataProvider.GetDataStoreNamesL(*localDataStores);
        	TInt dataStorecount = localDataStores->Count();
        	
        	if (dataStorecount > 0)
        		{
	        	TInt found = localDataStores->Find(localDatabase, count);
        	   	if (found != 0)  // 0 means database was found
        		    {
        		    aDataProviderId = dataProviderId;
        		    ret = EFalse;
        		    }
        		}
        	else
        		{
        		aDataProviderId = dataProviderId;
        		ret = EFalse;
        		}
        		
        	CleanupStack::PopAndDestroy(localDataStores);
        	}
	
	    CleanupStack::PopAndDestroy(&dataProvider);
	    CleanupStack::PopAndDestroy(&task);
		
		if (!ret)
			{
			break;
			}
		}
		
    CleanupStack::PopAndDestroy(&arr);
    
    FLOG( _L("TAspTask::CheckLocalDatabaseL END") );
    return ret;
	}





// End of file

