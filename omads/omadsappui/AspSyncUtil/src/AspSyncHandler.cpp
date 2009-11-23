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



// INCLUDES
#include "AspSyncHandler.h"
#include "AspDebug.h"
#include "AspSettingDialog.h"

#include <aspsyncutil.mbg>  // for progress dialog bitmap enumerations

#include <SyncMLErr.h>  // sync error codes

#include <ecom/ecom.h>
#include <ecom/implementationinformation.h>
#include <rconnmon.h>
#include <e32std.h> //For TRequestStatus
#include "CPreSyncPlugin.h"
#include "AspPreSyncPluginInterface.h"



/******************************************************************************
 * class TAspSyncParam
 ******************************************************************************/


// -------------------------------------------------------------------------------
// TAspSyncParam::TAspSyncParam
//
// -------------------------------------------------------------------------------
//
TAspSyncParam::TAspSyncParam(TInt aApplicationId)
 : iApplicationId(aApplicationId), iProfileId(KErrNotFound), iJobId(KErrNotFound),
   iTaskId(KErrNotFound), iSyncDirection(KErrNotFound), iDialogMode(EModeDialogNoWait)
	 {
	 }



/******************************************************************************
 * class CAspSyncHandler
 ******************************************************************************/


// -----------------------------------------------------------------------------
// CAspSyncHandler::NewL
//
// -----------------------------------------------------------------------------
//
CAspSyncHandler* CAspSyncHandler::NewL(const TAspParam& aParam)
    {
    FLOG( _L("CAspSyncHandler::NewL START") );

    CAspSyncHandler* self = new (ELeave) CAspSyncHandler(aParam);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);

    FLOG( _L("CAspSyncHandler::NewL END") );
    return self;
    }


// -----------------------------------------------------------------------------
// Destructor
//
// -----------------------------------------------------------------------------
//
CAspSyncHandler::~CAspSyncHandler()
    {
    FLOG( _L("CAspSyncHandler::~CAspSyncHandler START") );
    
    delete iProgressDialog;
	delete iState;
	delete iResHandler;
	delete iActiveCaller;
	delete iTextResolver;
	
	delete iContentList;
	
    if (iWait && iWait->IsStarted())
        {
        iWait->AsyncStop();
        }
	delete iWait;
	
	
	CloseSyncSession();
	
	FLOG( _L("CAspSyncHandler::~CAspSyncHandler END") );
    }


// -----------------------------------------------------------------------------
// CAspSyncHandler::ConstructL
//
// -----------------------------------------------------------------------------
//
void CAspSyncHandler::ConstructL()
    {
	FLOG( _L("CAspSyncHandler::ConstructL START") );
	
	iState = CAspState::NewL();

	iActiveCaller = CAspActiveCaller::NewL(this);
	iTextResolver = CTextResolver::NewL();
	
	iServerAlertSync = EFalse;
	iSyncRunning = EFalse;
	iSyncError = KErrNone;
	iCurrentSyncJobId = KErrNotFound;
	iSyncSessionOpen = EFalse;
	
	iWait = new (ELeave) CActiveSchedulerWait();
	
	FLOG( _L("CAspSyncHandler::ConstructL END") );
    }


// -----------------------------------------------------------------------------
// CAspSyncHandler::CAspSyncHandler
//
// -----------------------------------------------------------------------------
//
CAspSyncHandler::CAspSyncHandler(const TAspParam& /*aParam*/)
	{
	}


// -----------------------------------------------------------------------------
// CAspSyncHandler::SetObserver
//
// -----------------------------------------------------------------------------
//
void CAspSyncHandler::SetObserver(MAspSyncObserver* aObserver)
	{
	iSyncObserver = aObserver;
	}


// -----------------------------------------------------------------------------
// CAspSyncHandler::HandleDialogEventL (from MAspProgressDialogObserver)
//
// 
// Canceling sequence after user presses "Cancel":
//
// 1. framework calls CAspProgressDialog2::OkToExitL
// 2. OkToExitL calls CAspProgressDialog::DialogDismissedL
// 3. DialogDismissedL calls CAspSyncHandler::HandleDialogEventL
// 4. HandleDialogEventL cancels sync (RSyncMLDataSyncJob::StopL)
//    and updates progress dialog text ("Cancelling")
// 5. MSyncMLEventObserver::EJobStop arrives (asynchronously)
//    and CAspProgressDialog is closed
//
// -----------------------------------------------------------------------------
//
void CAspSyncHandler::HandleDialogEventL(TInt aButtonId)
    {
    FLOG( _L("CAspSyncHandler::HandleDialogEventL START") );
    
    TBool syncRunning = SyncRunning();
    
	//  close handling (eg. end key)
	if (aButtonId == EKeyPhoneEnd && syncRunning)
		{
		TRAPD(err, iSyncJob.StopL());

		if (err != KErrNone)
			{
			FLOG( _L("### RSyncMLDataSyncJob::StopL failed (%d) ###"), err );
			}

	    iSyncJob.Close(); // this can be done when job has not been opened

	    CloseSyncSession();

		iSyncRunning = EFalse;
		
	 	if (iIsSynchronousOperation && iWait->IsStarted())
		    {
	        iWait->AsyncStop();
		    }

		
		FLOG( _L("CAspSyncHandler::HandleDialogEventL: end key handling done") );
		FLOG( _L("CAspSyncHandler::HandleDialogEventL END") );
		return;
		}
		
	// cancel key handling
	if (aButtonId == EEikBidCancel && syncRunning)
		{
		if (iStopEventReceived)
			{
			CompleteWithDelay(KErrNone);
        	FLOG( _L("### EJobStop already received ###") );
        	FLOG( _L("CAspSyncHandler::HandleDialogEventL END") );
	        return;
			}
			
		TInt jobId = CAspProfile::CurrentJob(&iSyncSession);
       	if (jobId != iCurrentSyncJobId)
	        {
	        CompleteWithDelay(KErrNotFound);

	        FLOG( _L("### job conflict (%d/%d) ###"), iCurrentSyncJobId, jobId );
        	FLOG( _L("CAspSyncHandler::HandleDialogEventL END") );
	        return;
	        }

		TRAPD(err, iSyncJob.StopL());
       	if (err != KErrNone)
	        {
	        CompleteWithDelay(err);

	        FLOG( _L("### RSyncMLDataSyncJob::StopL failed (%d) ###"), err );
        	FLOG( _L("CAspSyncHandler::HandleDialogEventL END") );
	        return;
	        }
		    
	    State()->SetSyncPhase(CAspState::EPhaseCanceling);
        UpdateDialog();
        
        FLOG( _L("CAspSyncHandler::HandleDialogEventL: cancel key handling done") );
		}
		
	FLOG( _L("CAspSyncHandler::HandleDialogEventL END") );
    }


TBool CAspSyncHandler::IsRoamingL()
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
// CAspSyncHandler::ReadRepositoryL
//
// -----------------------------------------------------------------------------
//
void CAspSyncHandler::ReadRepositoryL(TInt aKey, TInt& aValue)
    {
    CRepository* rep = CRepository::NewLC(KCRUidNSmlDSApp);
    TInt err = rep->Get(aKey, aValue);
    User::LeaveIfError(err);
    
    CleanupStack::PopAndDestroy(rep);
    }

TInt CAspSyncHandler::BearerTypeL(TInt aProfileId)
    {    
    TInt BearerType = KErrNotFound;
    
    // sync session
    RSyncMLSession syncSession;
    syncSession.OpenL();
    CleanupClosePushL( syncSession );

    RSyncMLDataSyncProfile syncProfile;
    syncProfile.OpenL( syncSession, aProfileId, ESmlOpenRead );
    CleanupClosePushL( syncProfile );
    
    // Check if always ask is selected as accesspoint
    RSyncMLConnection connection;

    TInt err = KErrNone;
    RArray<TSmlTransportId> arr;
    
    TRAP(err, syncProfile.ListConnectionsL(arr));
    if (err != KErrNone)
        {
        syncProfile.Close();
        CleanupStack::Pop( &syncProfile );
        return KErrNotFound; 
        }
    if (arr.Count() == 0)
        {
        syncProfile.Close();
        CleanupStack::Pop( &syncProfile );
        arr.Close();  
        return KErrNotFound; // no connection
        }
            
    TInt transportId = arr[0];
    arr.Close();
        
    TRAP(err, connection.OpenL( syncProfile, transportId ) );
    CleanupClosePushL( connection );

    if ( err == KErrNone )
        {
        TInt id = connection.Identifier();
        if (id == KUidNSmlMediumTypeInternet.iUid)
            {
            BearerType = EAspBearerInternet;
            }
        else if (id == KUidNSmlMediumTypeBluetooth.iUid)
            {
            BearerType = EAspBearerBlueTooth;
            }
        else if (id == KUidNSmlMediumTypeUSB.iUid)
            {
            BearerType = EAspBearerUsb;
            }
        else if (id == KUidNSmlMediumTypeIrDA.iUid)
            {
            BearerType = EAspBearerIrda;
            }
        }

    connection.Close();
    CleanupStack::Pop( &connection );
    
    syncProfile.Close();
    CleanupStack::Pop( &syncProfile );
            
    syncSession.Close();
    CleanupStack::Pop( &syncSession );
    return BearerType;
    }
// -----------------------------------------------------------------------------
// CAspSyncHandler::SynchronizeL
//
// -----------------------------------------------------------------------------
//
void CAspSyncHandler::SynchronizeL(TAspSyncParam& aSyncParam)
	{
	iServerAlertSync = EFalse;
	iSyncRunning = EFalse;
	iCurrentSyncJobId = KErrNotFound;
	iSyncError = KErrNone;
	iCompleteEventReceived = EFalse;
	iStopEventReceived = EFalse;
	iIsSynchronousOperation = EFalse;
	TBool bCanSync = ETrue;
	
    TInt aValue = 0;
    ReadRepositoryL(KNSmlDSRoamingFeature, aValue);
    if(( IsRoamingL() ) && (aValue == EAspRoamingSettingFeatureEnabled))
        {
           TInt bearerType = BearerTypeL(aSyncParam.iProfileId);
           if (bearerType == EAspBearerInternet)           
            {
            CPreSyncPluginInterface* syncPluginInterface = CPreSyncPluginInterface::NewL();
            CPreSyncPlugin* syncPlugin = 
                syncPluginInterface->InstantiateRoamingPluginLC(aSyncParam.iProfileId);

            if(syncPlugin->IsSupported())
                {
                bCanSync = syncPlugin->CanSyncL();
                }

            CleanupStack::PopAndDestroy(syncPlugin);
            //syncPluginInterface->UnloadPlugIns();
            delete  syncPluginInterface;   

            if(!bCanSync)
                {
                return;
                }
            }        
        }	
	
	State()->SetSyncPhase(CAspState::EPhaseConnecting);
	State()->ResetProgress();
	//State()->SetTotalProgress(KTotalProgress);
	
	TRAPD(err, DoSynchronizeL(aSyncParam));
	
	if (err != KErrNone)
		{
		SynchronizeCompleted(err);
     	return;
		}
	}


// -----------------------------------------------------------------------------
// CAspSyncHandler::DoSynchronizeL
//
// -----------------------------------------------------------------------------
//
void CAspSyncHandler::DoSynchronizeL(TAspSyncParam& aSyncParam)
	{
	FLOG( _L("CAspSyncHandler::DoSynchronizeL START") );
    
	iApplicationId = aSyncParam.iApplicationId;
	iProfileId = aSyncParam.iProfileId;
	if (aSyncParam.iDialogMode == EModeDialogWait)
		{
		iIsSynchronousOperation = ETrue; // email 3.2 needs synchronous syncing
		}

	
	ShowProgressDialogL();
    OpenSyncSessionL();
    CreateContentListL();
    
    iSyncSession.RequestEventL(*this);     // for MSyncMLEventObserver events
    iSyncSession.RequestProgressL(*this);  // for MSyncMLProgressObserver events
    
 	if (aSyncParam.iTaskId != KErrNotFound && iProfileId != KErrNotFound)
		{
		RArray<TSmlTaskId> arr(1);
		CleanupClosePushL(arr);
		User::LeaveIfError(arr.Append(aSyncParam.iTaskId));
		if (aSyncParam.iSyncDirection != KErrNotFound)
			{
			TSmlSyncType syncDirection = TAspTask::SmlSyncDirection(aSyncParam.iSyncDirection);
			iSyncJob.CreateL(iSyncSession, iProfileId, syncDirection, arr);
			}
		else
			{
			iSyncJob.CreateL(iSyncSession, iProfileId, arr);
			}
			
		CleanupStack::PopAndDestroy(&arr);
		}
	else if (iProfileId != KErrNotFound)
		{
		if (aSyncParam.iSyncDirection != KErrNotFound)
			{
		    TSmlSyncType syncDirection = TAspTask::SmlSyncDirection(aSyncParam.iSyncDirection);
			iSyncJob.CreateL(iSyncSession, iProfileId, syncDirection);
			}
		else
			{
		    iSyncJob.CreateL(iSyncSession, iProfileId);
			}
		}
	else
		{
		User::Leave(KErrArgument);
		}
		
	iCurrentSyncJobId = iSyncJob.Identifier();
	iSyncRunning = ETrue;
	
	if (iIsSynchronousOperation)
		{
		iWait->Start();  // stop here until sync has completed
		}
   
    FLOG( _L("sync job created: app=%d, profile=%d, job=%d, task=%d"), iApplicationId, iProfileId, aSyncParam.iJobId, aSyncParam.iTaskId );
    FLOG( _L("CAspSyncHandler::DoSynchronizeL END") );
	}


// -----------------------------------------------------------------------------
// CAspSyncHandler::SynchronizeCompleted
//
// -----------------------------------------------------------------------------
//
void CAspSyncHandler::SynchronizeCompleted(TInt aError)
	{
	FLOG( _L("CAspSyncHandler::SynchronizeCompleted START") );
	
	iSyncRunning = EFalse;
	
	if (aError != KErrNone)
		{
		iSyncError = aError;
		}
   	
   	iSyncJob.Close(); // this can be done when job has not been opened
   	
    TRAP_IGNORE(Dialog()->CancelProgressDialogL());
	
	if (iSyncError != KErrNone)
		{
	   	CloseSyncSession();
   		ShowErrorNote();
      	
      	FLOG( _L("### sync failed (%d) ###"), iSyncError );
		}
	else
	    {
      	CloseSyncSession();
      	TRAP_IGNORE(TDialogUtil::ShowInformationNoteL(R_ASP_SYNC_COMPLETED));
	    }
	    
    if (iSyncObserver)
    	{
        TRAP_IGNORE(iSyncObserver->HandleSyncEventL(iSyncError, iProfileId));
    	}
    	
 	if (iIsSynchronousOperation && iWait->IsStarted())
		{
	    iWait->AsyncStop();
		}
	    	
	FLOG( _L("CAspSyncHandler::SynchronizeCompleted END") );
	}


// -----------------------------------------------------------------------------
// CAspSyncHandler::OnSyncMLSessionEvent (from MSyncMLEventObserver)
//
// -----------------------------------------------------------------------------
//
void CAspSyncHandler::OnSyncMLSessionEvent(TEvent aEvent, TInt aIdentifier, TInt aError, TInt /*aAdditionalData*/)
	{
	FLOG( _L("CAspSyncHandler::OnSyncMLSessionEvent START") );

#ifdef _DEBUG
    LogSessionEvent(aEvent, aIdentifier, aError);
#endif

    if (iCurrentSyncJobId != aIdentifier)
    	{
    	FLOG( _L("### job conflict (%d/%d) ###"), iCurrentSyncJobId, aIdentifier );
    	FLOG( _L("CAspSyncHandler::OnSyncMLSessionEvent END") );
    	return; // not our sync job
    	}
	
	if (aEvent == EJobStop)
		{
		iStopEventReceived = ETrue;
		if (aError != KErrNone)
			{
			iSyncError = aError;
			}
	
	    if (State()->SyncPhase() == CAspState::EPhaseCanceling)
		    {
		    CompleteWithDelay(aError);
		    }
        else if (iCompleteEventReceived || iSyncError != KErrNone)
		    {
		    CompleteWithDelay(aError);
		    }
		}
    	
	if (aEvent == EJobStartFailed || aEvent == EJobRejected)
		{
		CompleteWithDelay(aError);
		}
		
	FLOG( _L("CAspSyncHandler::OnSyncMLSessionEvent END") );
    }


// -----------------------------------------------------------------------------
// CAspSyncHandler::CompleteWithDelay
//
// -----------------------------------------------------------------------------
//
void CAspSyncHandler::CompleteWithDelay(TInt aError)
	{
	iActiveCaller->Start(aError, 0); // call SynchronizeCompleted with delay
	}


// -----------------------------------------------------------------------------
// CAspSyncHandler::OnSyncMLSyncError (from MSyncMLProgressObserver)
//
// -----------------------------------------------------------------------------
//
void CAspSyncHandler::OnSyncMLSyncError(TErrorLevel aErrorLevel, TInt aError, TInt /*aTaskId*/, TInt /*aInfo1*/, TInt /*aInfo2*/)
	{
	FLOG( _L("CAspSyncHandler::OnSyncMLSyncError START") );
	
	// just store error code - sync is terminated only in function OnSyncMLSessionEvent
	if (aErrorLevel == ESmlFatalError)
		{
		iSyncError = aError;
		}
		
#ifdef _DEBUG
    LogErrorEvent(aErrorLevel, aError);
#endif

	FLOG( _L("CAspSyncHandler::OnSyncMLSyncError END") );
	}


// -----------------------------------------------------------------------------
// CAspSyncHandler::OnSyncMLSyncProgress (from MSyncMLProgressObserver)
//
// -----------------------------------------------------------------------------
//
void CAspSyncHandler::OnSyncMLSyncProgress(TStatus aStatus, TInt aInfo1, TInt /*aInfo2*/)
	{
	FLOG( _L("CAspSyncHandler::OnSyncMLSyncProgress START") );
	
	if (State()->SyncPhase() == CAspState::EPhaseCanceling)
		{
		return; // no progress dialog updating while canceling
		}

#ifdef _DEBUG
    LogProgressEvent(aStatus, aInfo1);
#endif
	
	switch (aStatus)
		{
		case ESmlConnecting:
		    State()->SetSyncPhase(CAspState::EPhaseConnecting);
	        UpdateDialog();
		    break;
		case ESmlConnected:
		    break;
		case ESmlLoggingOn:
		    break;
		case ESmlLoggedOn:
		    State()->SetSyncPhase(CAspState::EPhaseSynchronizing);
		    UpdateDialog();
		    break;
		case ESmlDisconnected:
		    State()->SetSyncPhase(CAspState::EPhaseDisconnecting);
		    UpdateDialog();
		    break;
		case ESmlCompleted:
		    iCompleteEventReceived = ETrue;
		    
			if (iStopEventReceived)
			    {
			    FLOG( _L("### EJobStop already received ###") );
			    CompleteWithDelay(iSyncError);
			    }
            else
            	{
            	State()->SetSyncPhase(CAspState::EPhaseDisconnecting);
		        UpdateDialog();
            	}
		    break;

		case ESmlSendingModificationsToServer:
		    State()->SetSyncPhase(CAspState::EPhaseSending);
   		    State()->ResetProgress();
		    State()->SetTotalProgress(aInfo1); // set total sync event count

     	    UpdateDialog();
		    break;
		case ESmlReceivingModificationsFromServer:
		    State()->SetSyncPhase(CAspState::EPhaseReceiving);
   		    State()->ResetProgress();
		    State()->SetTotalProgress(aInfo1); // set total sync event count

	        UpdateDialog();
		    break;
		    
		case ESmlSendingMappingsToServer:
		    State()->SetSyncPhase(CAspState::EPhaseSynchronizing);
		    UpdateDialog();
		    break;
		
		default:
		    break;
		}
		
	FLOG( _L("CAspSyncHandler::OnSyncMLSyncProgress END") );
	}


// -----------------------------------------------------------------------------
// CAspSyncHandler::OnSyncMLDataSyncModifications (from MSyncMLProgressObserver)
//
// -----------------------------------------------------------------------------
//
void CAspSyncHandler::OnSyncMLDataSyncModifications(TInt aTaskId, 
                      const TSyncMLDataSyncModifications& aClientModifications,
                      const TSyncMLDataSyncModifications& aServerModifications)
	{
	
    if (!Dialog()->ProgressDialog())
    	{
    	//it is observed that some times this call back comes 
    	//just after deleting progress dialog.
    	return;
    	}
	
	TRAPD(err, OnSyncMLDataSyncModificationsL(aTaskId, aClientModifications, aServerModifications));
	if (err != KErrNone)
		{
		FLOG( _L("### CAspSyncHandler::OnSyncMLDataSyncModificationsL failed (%d) ###"), err );
		FLOG( _L("### CAspSyncHandler::OnSyncMLDataSyncModificationsL failed (%d) ###"), err );
		}
	}


// -----------------------------------------------------------------------------
// CAspSyncHandler::OnSyncMLDataSyncModificationsL
//
// -----------------------------------------------------------------------------
//
void CAspSyncHandler::OnSyncMLDataSyncModificationsL(TInt aTaskId,
                      const TSyncMLDataSyncModifications& aClientModifications,
                      const TSyncMLDataSyncModifications& aServerModifications)
	{
	FLOG( _L("CAspSyncHandler::OnSyncMLDataSyncModificationsL START") );
	

	if (State()->Progress() == 0)
		{
	    TInt index = iContentList->FindProviderIndexForTask(aTaskId);
	
	    if (index != KErrNotFound)
		    {
		    TAspProviderItem& item = iContentList->ProviderItem(index);
		    HBufC* hBuf = CAspResHandler::GetContentNameLC(item.iDataProviderId,
		                                                   item.iDisplayName);
	        State()->SetContentL(hBuf->Des());

#ifdef _DEBUG
            LogModificationEvent(aTaskId, hBuf->Des());
#endif

	        CleanupStack::PopAndDestroy(hBuf);
		    }
	    else
		    {
		    State()->SetContentL(KNullDesC);
		    FLOG( _L("### task not found (%d) ###"), aTaskId );
		    }
		}
	
	// Set sync event count. On last OnSyncMLDataSyncModifications call it 
	// must match total sync event count (set in OnSyncMLSyncProgress). 
	TInt count = ProgressCount(aClientModifications, aServerModifications);
	State()->IncreaseProgress(count);
	
	UpdateDialog();

	
#ifdef _DEBUG
    LogModificationEvent(aTaskId, aClientModifications, aServerModifications);
#endif


	FLOG( _L("CAspSyncHandler::OnSyncMLDataSyncModificationsL END") );
	FLOG( _L(" ") );
	}



// -----------------------------------------------------------------------------
// CAspSyncHandler::UpdateDialog
//
// -----------------------------------------------------------------------------
//
void CAspSyncHandler::UpdateDialog()
	{
	TRAPD(err, UpdateDialogL());
	
	if (err != KErrNone)
		{
		FLOG( _L("### CAspSyncHandler::UpdateDialogL failed (%d) ###"), err );
		}
	}


// -----------------------------------------------------------------------------
// CAspSyncHandler::UpdateDialogL
//
// -----------------------------------------------------------------------------
//
void CAspSyncHandler::UpdateDialogL()
	{
	if (!SyncRunning())
		{
		FLOG( _L("### CAspSyncHandler::UpdateDialogL: Sync not running ###") );
		return;
		}

	TInt phase = State()->SyncPhase();
	
	if (phase == CAspState::EPhaseConnecting)
		{
		Dialog()->StartAnimationL();
		HBufC* hBuf = ResHandlerL()->ReadLC(R_SYNCSTATUS_SYNC_CONN);  // "Connecting"
		Dialog()->UpdateTextL(hBuf->Des());
		CleanupStack::PopAndDestroy(hBuf);
		}

	if (phase == CAspState::EPhaseSynchronizing)
		{
		Dialog()->StartAnimationL();
		HBufC* hBuf = ResHandlerL()->ReadLC(R_SYNCSTATUS_SYNC_SYNC); // "Synchronizing"
		Dialog()->UpdateTextL(hBuf->Des());
		CleanupStack::PopAndDestroy(hBuf);
		}

	if (phase == CAspState::EPhaseDisconnecting)
		{
		Dialog()->StartAnimationL();
		HBufC* hBuf = ResHandlerL()->ReadLC(R_SYNCSTATUS_SYNC_DISC);  // "Disconnecting"
		Dialog()->UpdateTextL(hBuf->Des());
		CleanupStack::PopAndDestroy(hBuf);
		}

	if (phase == CAspState::EPhaseCanceling)
		{
		Dialog()->StartAnimationL();
		HBufC* hBuf = ResHandlerL()->ReadLC(R_SYNCSTATUS_CANC);  // "Canceling"
		Dialog()->UpdateTextL(hBuf->Des());
		CleanupStack::PopAndDestroy(hBuf);
		Dialog()->HideButtonL();
		}

//The number of Emails-Synced NOT printed if Sync is started from Sync E-mail Application
	if (iApplicationId == EApplicationIdEmail)
	     return;
	
	if (phase == CAspState::EPhaseSending || phase == CAspState::EPhaseReceiving)
		{
		if (State()->Progress() == 0 || State()->TotalProgress() == 0)
			{
			return; 
			}
			
		if (State()->FirstProgress())
			{
			if (State()->ProgressKnown())
				{
    			Dialog()->CancelAnimation();
	    		Dialog()->SetFinalProgress(State()->TotalProgress());
     	    	Dialog()->SetProgress(State()->Progress());
				}
			else
				{
    			Dialog()->SetFinalProgress(0);
	    	    Dialog()->StartAnimationL();
				}
			}
		else if (State()->ProgressKnown())
			{
			Dialog()->SetProgress(State()->Progress());
			}
			
		HBufC* hBuf = ResHandlerL()->ReadProgressTextLC(State()->Content(), phase);
		

#ifdef _DEBUG
		TBuf<KBufSize> buf1; TBuf<KBufSize> buf2;
		buf1 = hBuf->Des();
		buf2.Format(_L("%S\n (%d/%d)"), &buf1, State()->Progress(), State()->TotalProgress());
		CleanupStack::PopAndDestroy(hBuf);
		hBuf = NULL;
		hBuf = buf2.AllocLC();
#endif
		
   	
    	Dialog()->UpdateTextL(hBuf->Des());
		CleanupStack::PopAndDestroy(hBuf);
		}
	}


// -----------------------------------------------------------------------------
// CAspSyncHandler::UpdateRequired
//
// -----------------------------------------------------------------------------
//
TBool CAspSyncHandler::UpdateRequired()
	{
    const TInt KProgressLevel_1 = 10;
    const TInt KProgressLevel_2 = 20;
    const TInt KProgressLevel_3 = 50;
    
    const TInt KProgressStep_1 = 5;
    const TInt KProgressStep_2 = 10;
    const TInt KProgressStep_3 = 20;
    
    TInt total = State()->TotalProgress();
    TInt current = State()->Progress();
    TBool firstProgress = State()->FirstProgress();
    TBool progressKnown = State()->ProgressKnown();
    
    // always show first progress
    if (firstProgress)
    	{
    	return ETrue;
    	}
    
    // handle case where total progress count is not known
    if (!progressKnown)
    	{
    	if (current <= KProgressStep_2)
    		{
    		return ETrue;
    		}
    	else if (current % KProgressStep_1 == 0)
    		{
    		return ETrue;
    		}
    		
    	return EFalse;
    	}
    
    // handle case where total progress count is known
    if (current == total)
    	{
    	return ETrue; // always show last progress
    	}
    else if (total <= KProgressLevel_1)
    	{
    	return ETrue;
    	}
    else if (total > KProgressLevel_1 && total <= KProgressLevel_2)
		{
    	if (current % KProgressStep_1 == 0)
			{
			return ETrue;
			}
		}
    else if (total > KProgressLevel_2 && total <= KProgressLevel_3)
		{
    	if (current % KProgressStep_2 == 0)
			{
			return ETrue;
			}
		}
    else if (total > KProgressLevel_3)
		{
    	if (current % KProgressStep_3 == 0)
			{
			return ETrue;
			}
		}

	return EFalse;
	}


// -----------------------------------------------------------------------------
// CAspSyncHandler::HandleActiveCallL (from MAspActiveCallerObserver)
//
// -----------------------------------------------------------------------------
//
void CAspSyncHandler::HandleActiveCallL(TInt aCallId)
	{
    SynchronizeCompleted(aCallId);
	}


// -----------------------------------------------------------------------------
// CAspSyncHandler::ShowErrorNote
//
// -----------------------------------------------------------------------------
//
void CAspSyncHandler::ShowErrorNote()
	{
	if (iSyncError == KErrNone || iSyncError == KErrCancel)
		{
		return; // nothing to show
		}

	// show sync engine error
	if (iSyncError <= SyncMLError::KErrSyncEngineErrorBase)
		{
		HBufC* hBuf = NULL;
		hBuf = TAspSyncError::GetSmlErrorText(iSyncError, 0);
		if (hBuf)  // hBuf can be NULL
 			{
     	   	TRAP_IGNORE(TDialogUtil::ShowErrorQueryL(hBuf->Des()));
	     	delete hBuf;
			}
			
		return;
		}

    // show system error
    TPtrC errorMsg;
    
    errorMsg.Set(iTextResolver->ResolveErrorString(iSyncError, CTextResolver::ECtxNoCtxNoSeparator));
	
	TRAP_IGNORE(TDialogUtil::ShowErrorQueryL(errorMsg));
    }

// -----------------------------------------------------------------------------
// CAspSyncHandler::ShowProgressDialogL
//
// -----------------------------------------------------------------------------
//
void CAspSyncHandler::ShowProgressDialogL()
	{
	delete iProgressDialog;
	iProgressDialog = NULL;
	iProgressDialog = CAspProgressDialog::NewL(this);
	
	TInt resId = R_SYNCSTATUS_SYNC_CONN;
	if (iApplicationId == EApplicationIdEmail)
		{
		resId = R_SYNCSTATUS_SYNC_SYNC;
		}
	
	HBufC* hBuf = CAspResHandler::ReadLC(resId);
	
	iProgressDialog->SetApplicationId(iApplicationId);
	
   	TFileName bitmapName;
	CAspResHandler::GetBitmapFileName(bitmapName);
	
	iProgressDialog->LaunchProgressDialogL(hBuf->Des(),
	                                       KAknsIIDQgnNoteSml, 
	                                       bitmapName,
	                                       EMbmAspsyncutilQgn_note_sml,
	                                       EMbmAspsyncutilQgn_note_sml_mask);
	CleanupStack::PopAndDestroy(hBuf);
	}


// -----------------------------------------------------------------------------
// CAspSyncHandler::CreateContentListL
//
// -----------------------------------------------------------------------------
//
void CAspSyncHandler::CreateContentListL()
    {
    TAspParam param(iApplicationId, &iSyncSession);

    CAspProfile* profile = CAspProfile::NewLC(param);
    profile->OpenL(iProfileId, CAspProfile::EOpenRead, CAspProfile::EBaseProperties);
    
	param.iProfile = profile;
	param.iMode = CAspContentList::EInitAll;
	
	delete iContentList;
	iContentList = NULL;
	iContentList = CAspContentList::NewL(param);
	
    CleanupStack::PopAndDestroy(profile);	
    }


// -----------------------------------------------------------------------------
// CAspSyncHandler::CheckLocalDatabaseL
//
// -----------------------------------------------------------------------------
//
void CAspSyncHandler::CheckLocalDatabaseL(TInt aProfileId)
    {
    TAspParam param(iApplicationId, &iSyncSession);

    CAspProfile* profile = CAspProfile::NewLC(param);
    profile->OpenL(aProfileId, CAspProfile::EOpenRead, CAspProfile::EBaseProperties);
    
	TInt dataProviderId = KErrNotFound;
	TBool databaseFound = TAspTask::CheckLocalDatabase(profile, dataProviderId);
	CleanupStack::PopAndDestroy(profile);
	
	if (!databaseFound)
		{
		User::Leave(SyncMLError::KErrClientDatabaseNotFound);
		}
    }



// -----------------------------------------------------------------------------
// CAspSyncHandler::Dialog
//
// -----------------------------------------------------------------------------
//
CAspProgressDialog* CAspSyncHandler::Dialog()
	{
	__ASSERT_DEBUG(iProgressDialog, TUtil::Panic(KErrGeneral));

	return iProgressDialog;
	}


// -----------------------------------------------------------------------------
// CAspSyncHandler::State
//
// -----------------------------------------------------------------------------
//
CAspState* CAspSyncHandler::State()
	{
	__ASSERT_DEBUG(iState, TUtil::Panic(KErrGeneral));

	return iState;
	}


// -----------------------------------------------------------------------------
// CAspSyncHandler::ResHandlerL
//
// -----------------------------------------------------------------------------
//
CAspResHandler* CAspSyncHandler::ResHandlerL()
	{
	if (!iResHandler)
		{
		iResHandler = CAspResHandler::NewL();
		}

	return iResHandler;
	}


// -----------------------------------------------------------------------------
// SyncRunning
// 
// -----------------------------------------------------------------------------
//
TBool CAspSyncHandler::SyncRunning()
	{
	return iSyncRunning;
	}

// -----------------------------------------------------------------------------
// IsServerAlertSync
// 
// -----------------------------------------------------------------------------
//
TBool CAspSyncHandler::IsServerAlertSync()
	{
	return iServerAlertSync;
	}


// -----------------------------------------------------------------------------
// OpenSyncSessionL
// 
// -----------------------------------------------------------------------------
//
void CAspSyncHandler::OpenSyncSessionL()
	{
	if (!iSyncSessionOpen)
		{
        TRAPD(err, iSyncSession.OpenL());
        
        if (err != KErrNone)
        	{
        	FLOG( _L("### CAspSyncHandler: RSyncMLSession::OpenL failed (%d) ###"), err );
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
void CAspSyncHandler::CloseSyncSession()
	{
	if (iSyncSessionOpen)
		{
		iSyncSession.Close();
		iSyncSessionOpen = EFalse;
		}
    }


// -----------------------------------------------------------------------------
// ProgressCount
// 
// -----------------------------------------------------------------------------
//
TInt CAspSyncHandler::ProgressCount(const TSyncMLDataSyncModifications& aC, 
                                    const TSyncMLDataSyncModifications& aS)
	{
	TInt num1 = aC.iNumAdded + aC.iNumReplaced + aC.iNumMoved + aC.iNumDeleted + aC.iNumFailed;
	TInt num2 = aS.iNumAdded + aS.iNumReplaced + aS.iNumMoved + aS.iNumDeleted + aS.iNumFailed;
	
	return num1 + num2;
	}
	
	

#ifdef _DEBUG


// -----------------------------------------------------------------------------
// LogModificationEvent
//
// -----------------------------------------------------------------------------
//
void CAspSyncHandler::LogModificationEvent(TInt aTaskId, const TSyncMLDataSyncModifications& aC, const TSyncMLDataSyncModifications& aS)
	{
	TInt num1 = State()->Progress();
	TInt num2 = State()->TotalProgress();
	
	if (num1 > num2 && num2 != -1)
		{
		FTRACE( RDebug::Print(_L("### syncui: task=%d, progress=%d, totalprogress=%d ###"), aTaskId, num1, num2) );
		FLOG(_L("### syncui: task=%d, progress=%d, totalprogress=%d ###"), aTaskId, num1, num2);
		}
	else
		{
		FTRACE( RDebug::Print(_L("syncui: task=%d, progress=%d, totalprogress=%d"), aTaskId, num1, num2) );
		FLOG(_L("syncui: task=%d, progress=%d, totalprogress=%d"), aTaskId, num1, num2);
		}
		
	num1 = aC.iNumAdded + aC.iNumReplaced + aC.iNumMoved + aC.iNumDeleted + aC.iNumFailed;
	num2 = aS.iNumAdded + aS.iNumReplaced + aS.iNumMoved + aS.iNumDeleted + aS.iNumFailed;
	
	if ( (num1>0 && num2>0) || (num1<0 || num2<0) || (num1==0 && num2==0) )
		{
		FLOG( _L("### parameter error ###") );
		}
		
    FTRACE( RDebug::Print(_L("client: add=%d, rep=%d, mov=%d del=%d fai=%d"), aC.iNumAdded, aC.iNumReplaced, aC.iNumMoved, aC.iNumDeleted, aC.iNumFailed) );
    FTRACE( RDebug::Print(_L("server: add=%d, rep=%d, mov=%d del=%d fai=%d"), aS.iNumAdded, aS.iNumReplaced, aS.iNumMoved, aS.iNumDeleted, aS.iNumFailed) );
    
    FLOG(_L("client: add=%d, rep=%d, mov=%d del=%d fai=%d"), aC.iNumAdded, aC.iNumReplaced, aC.iNumMoved, aC.iNumDeleted, aC.iNumFailed);
    FLOG(_L("server: add=%d, rep=%d, mov=%d del=%d fai=%d"), aS.iNumAdded, aS.iNumReplaced, aS.iNumMoved, aS.iNumDeleted, aS.iNumFailed);
	}


// -----------------------------------------------------------------------------
// LogModificationEvent
//
// -----------------------------------------------------------------------------
//
void CAspSyncHandler::LogModificationEvent(TInt aTaskId, const TDesC& aText)
	{
	FTRACE( RDebug::Print(_L("progress = 0, task id = %d, task name ='%S'"), aTaskId, &aText) );
	FLOG(_L("progress = 0, task id = %d, task name ='%S'"), aTaskId, &aText);
	}


// -----------------------------------------------------------------------------
// LogSessionEvent
//
// -----------------------------------------------------------------------------
//
void CAspSyncHandler::LogSessionEvent(TEvent& aEvent, TInt aIdentifier, TInt aError)
	{
    TBuf<KBufSize> eventStr;
    GetSyncEventText(eventStr, aEvent);

    TBuf<KBufSize> errStr;
    TAspSyncError::GetSyncMLErrorText(errStr, aError);

	if (aError == KErrNone)
		{
		FTRACE( RDebug::Print(_L("event='%S' err='%S' job= %d curjob= %d"), &eventStr, &errStr, aIdentifier, iCurrentSyncJobId) );
		FLOG(_L("event='%S' err='%S' job= %d curjob= %d"), &eventStr, &errStr, aIdentifier, iCurrentSyncJobId);
		}
	else
		{
		FTRACE( RDebug::Print(_L("### event='%S' err='%S' job= %d curjob= %d ###"), &eventStr, &errStr, aIdentifier, iCurrentSyncJobId) );
		FLOG(_L("### event='%S' err='%S' job= %d curjob= %d ###"), &eventStr, &errStr, aIdentifier, iCurrentSyncJobId);
		}
	}
	

// -----------------------------------------------------------------------------
// LogProgressEvent
//
// -----------------------------------------------------------------------------
//
void CAspSyncHandler::LogProgressEvent(TStatus& aStatus, TInt aInfo1)
	{
    TBuf<KBufSize> statusStr;
    GetSyncStatusText(statusStr, aStatus);
    FTRACE( RDebug::Print(_L("OnSyncMLSyncProgress: status = '%S' info1 = %d"), &statusStr, aInfo1) );
    FLOG(_L("OnSyncMLSyncProgress: status = '%S' info1 = %d"), &statusStr, aInfo1);
    
   	TInt num1 = State()->Progress();
   	TInt num2 = State()->TotalProgress();
   	if (num1 != num2 && num2 != -1)
   		{
   		FTRACE( RDebug::Print(_L("### progress count error (%d/%d) ###"), num1, num2) );
   		FLOG(_L("### progress count error (%d/%d) ###"), num1, num2);
   		}
	}
	
	
// -----------------------------------------------------------------------------
// LogErrorEvent
//
// -----------------------------------------------------------------------------
//
void CAspSyncHandler::LogErrorEvent(TErrorLevel aErrorLevel, TInt aError)
	{
    TBuf<KBufSize> errStr;
    TAspSyncError::GetSyncMLErrorText(errStr, aError);
    
    if (aErrorLevel == ESmlFatalError)
    	{
    	FTRACE( RDebug::Print(_L("### OnSyncMLSyncError: fatalerror = %S ###"), &errStr) );
    	FLOG(_L("### OnSyncMLSyncError: fatalerror = %S ###"), &errStr);
    	}
    else
    	{
    	FTRACE( RDebug::Print(_L("### OnSyncMLSyncError: error = %S ###"), &errStr) );
    	FLOG(_L("### OnSyncMLSyncError: error = %S ###"), &errStr);
    	}
	}



// -----------------------------------------------------------------------------
// GetSyncStatusText
//
// -----------------------------------------------------------------------------
//
void CAspSyncHandler::GetSyncStatusText(TDes& aText, MSyncMLProgressObserver::TStatus aStatus)
	{
    aText = _L("Unknown");
    		
	if (aStatus == MSyncMLProgressObserver::ESmlConnecting)
		{
		aText = _L("ESmlConnecting");
		}
	if (aStatus == MSyncMLProgressObserver::ESmlConnected)
		{
		aText = _L("ESmlConnected");
		}
	if (aStatus == MSyncMLProgressObserver::ESmlLoggingOn)
		{
		aText = _L("ESmlLoggingOn");
		}
	if (aStatus == MSyncMLProgressObserver::ESmlLoggedOn)
		{
		aText = _L("ESmlLoggedOn");
		}
	if (aStatus == MSyncMLProgressObserver::ESmlDisconnected)
		{
		aText = _L("ESmlDisconnected");
		}
	if (aStatus == MSyncMLProgressObserver::ESmlCompleted)
		{
		aText = _L("ESmlCompleted");
		}
	if (aStatus == MSyncMLProgressObserver::ESmlSendingModificationsToServer)
		{
		aText = _L("ESmlSendingModificationsToServer");
		}
	if (aStatus == MSyncMLProgressObserver::ESmlReceivingModificationsFromServer)
		{
		aText = _L("ESmlReceivingModificationsFromServer");
		}
	if (aStatus == MSyncMLProgressObserver::ESmlSendingMappingsToServer)
		{
		aText = _L("ESmlSendingMappingsToServer");
		}
	}


// -----------------------------------------------------------------------------
// GetSyncEventText
//
// -----------------------------------------------------------------------------
//
void CAspSyncHandler::GetSyncEventText(TDes& aText, MSyncMLEventObserver::TEvent aEvent)
	{
    aText = _L("Unknown");
    		
	if (aEvent == MSyncMLEventObserver::EJobStart)
		{
		aText = _L("EJobStart");
		}
	if (aEvent == MSyncMLEventObserver::EJobStartFailed)
		{
		aText = _L("EJobStartFailed");
		}
	if (aEvent == MSyncMLEventObserver::EJobStop)
		{
		aText = _L("EJobStop");
		}
	if (aEvent == MSyncMLEventObserver::EJobRejected)
		{
		aText = _L("EJobRejected");
		}
	if (aEvent == MSyncMLEventObserver::EProfileCreated)
		{
		aText = _L("EProfileCreated");
		}
	if (aEvent == MSyncMLEventObserver::EProfileChanged)
		{
		aText = _L("EProfileChanged");
		}
	if (aEvent == MSyncMLEventObserver::EProfileDeleted)
		{
		aText = _L("EProfileDeleted");
		}
	if (aEvent == MSyncMLEventObserver::ETransportTimeout)
		{
		aText = _L("ETransportTimeout");
		}
	if (aEvent == MSyncMLEventObserver::EServerSuspended)
		{
		aText = _L("EServerSuspended");
		}
	if (aEvent == MSyncMLEventObserver::EServerTerminated)
		{
		aText = _L("EServerTerminated");
		}
	}

#endif





// End of file

