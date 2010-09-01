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



#include "AspDbNotifier.h"
#include "AspDebug.h"


// ============================ MEMBER FUNCTIONS ===============================


/******************************************************************************
 * class TAspDbEvent
 ******************************************************************************/


// -----------------------------------------------------------------------------
// TAspDbEvent::TAspDbEvent
//
// -----------------------------------------------------------------------------
//
TAspDbEvent::TAspDbEvent(TInt aType)
 : iType(aType), iProfileId(KErrNotFound), iError(KErrNone)
	 {
	 }



/******************************************************************************
 * class CAspDbNotifier
 ******************************************************************************/


// -----------------------------------------------------------------------------
// CAspDbNotifier::CAspDbNotifier
//
// -----------------------------------------------------------------------------
//
CAspDbNotifier::CAspDbNotifier(const TAspParam& aParam, MAspDbEventHandler* aHandler)
 : iSyncSession(aParam.iSyncSession), iHandler(aHandler)
 	{
 	iSessionEventRequested = EFalse;
 	}


// -----------------------------------------------------------------------------
// CAspDbNotifier::NewL
//
// -----------------------------------------------------------------------------
//
CAspDbNotifier* CAspDbNotifier::NewL(const TAspParam& aParam, MAspDbEventHandler* aHandler)
	{
	CAspDbNotifier* self = new (ELeave) CAspDbNotifier(aParam, aHandler);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
	}


// -----------------------------------------------------------------------------
// CAspDbNotifier::ConstructL
//
// -----------------------------------------------------------------------------
//
void CAspDbNotifier::ConstructL()
	{
    FLOG( _L("CAspDbNotifier::ConstructL START") );
    
	iActiveCaller = CAspActiveCaller::NewL(this);
    
    FLOG( _L("CAspDbNotifier::ConstructL END") );
	}


// -----------------------------------------------------------------------------
// Destructor
//
// -----------------------------------------------------------------------------
//
CAspDbNotifier::~CAspDbNotifier()
	{		
    FLOG( _L("CAspDbNotifier::~CAspDbNotifier START") );
    
	delete iActiveCaller;
	iList.Close();
    
    if (iSessionEventRequested)
    	{
    	Session().CancelEvent();
    	}
    	
    if (iDestroyedPtr)
        {
        *iDestroyedPtr = ETrue;
        iDestroyedPtr = NULL;
        }

    FLOG( _L("CAspDbNotifier::~CAspDbNotifier END") );
	}


// -----------------------------------------------------------------------------
// CAspDbNotifier::RequestL
//
// -----------------------------------------------------------------------------
//
void CAspDbNotifier::RequestL()
	{
	if (!iSessionEventRequested)
		{
		Session().RequestEventL(*this);  // request MSyncMLEventObserver events
		iSessionEventRequested = ETrue;
		}
	}


// -----------------------------------------------------------------------------
// CAspDbNotifier::OnSyncMLSessionEvent (from MSyncMLEventObserver)
//
// -----------------------------------------------------------------------------
//
void CAspDbNotifier::OnSyncMLSessionEvent(TEvent aEvent, TInt aIdentifier, TInt aError, TInt aAdditionalData)
	{
    FLOG( _L("CAspDbNotifier::OnSyncMLSessionEvent START") );

#ifdef _DEBUG
    LogSessionEvent(aEvent, aIdentifier, aError);
#endif

    TRAP_IGNORE(HandleSessionEventL(aEvent, aIdentifier, aError, aAdditionalData));
	
    FLOG( _L("CAspDbNotifier::OnSyncMLSessionEvent END") );
    }


// -----------------------------------------------------------------------------
// CAspDbNotifier::HandleSessionEventL
//
// -----------------------------------------------------------------------------
//
void CAspDbNotifier::HandleSessionEventL(TEvent aEvent, TInt aIdentifier, TInt aError, TInt /*aAdditionalData*/)
	{
	if (aEvent != MSyncMLEventObserver::EProfileCreated &&
	    aEvent != MSyncMLEventObserver::EProfileChanged &&
	    aEvent != MSyncMLEventObserver::EProfileDeleted &&
	    aEvent != MSyncMLEventObserver::EServerTerminated)
		{
		return; // setting dialog does not need this event
		}
	
	TAspDbEvent event(aEvent);

    event.iError = aError;
		
	if (aEvent == MSyncMLEventObserver::EProfileCreated || 
	    aEvent == MSyncMLEventObserver::EProfileChanged ||
	    aEvent == MSyncMLEventObserver::EProfileDeleted )
		{
		event.iProfileId = aIdentifier;
		}
	else
		{
		event.iProfileId = KErrNotFound;
		}
	
	iList.AppendL (event);
	
	CallObserverWithDelay();
	RequestL();
    }


// -----------------------------------------------------------------------------
// CAspDbNotifier::CreateUpdateEventL
//
// -----------------------------------------------------------------------------
//
void CAspDbNotifier::CreateUpdateEventL(TInt aIdentifier, TInt aError)
	{
	TAspDbEvent event(MSyncMLEventObserver::EProfileChanged);
	event.iProfileId = aIdentifier;
    event.iError = aError;
	
	iList.Reset();
	iList.AppendL (event);
	
	CallObserverWithDelay();
	RequestL();
    }


// -----------------------------------------------------------------------------
// CAspDbNotifier::CheckUpdateEventL
//
// -----------------------------------------------------------------------------
//
void CAspDbNotifier::CheckUpdateEventL()
	{
	if (iList.Count() == 0)
		{
		return; // no database events - no need to update UI
		}
		
	CallObserverWithDelay();
	RequestL();
    }


// -----------------------------------------------------------------------------
// CAspDbNotifier::SetDisabled
//
// -----------------------------------------------------------------------------
//
void CAspDbNotifier::SetDisabled(TBool aDisable)
	{
    iDisabled = aDisable;
	}


// -----------------------------------------------------------------------------
// CAspDbNotifier::Reset
//
// -----------------------------------------------------------------------------
//
void CAspDbNotifier::Reset()
	{
    iDisabled = EFalse;
	iList.Reset();
	}


// -----------------------------------------------------------------------------
// CAspDbNotifier::EventCount
//
// -----------------------------------------------------------------------------
//
TInt CAspDbNotifier::EventCount()
	{
	return iList.Count();
	}


// -----------------------------------------------------------------------------
// CAspDbNotifier::Event
//
// -----------------------------------------------------------------------------
//
TAspDbEvent CAspDbNotifier::Event(TInt aIndex)
	{
	__ASSERT_DEBUG(aIndex>=0 && aIndex<iList.Count(), TUtil::Panic(KErrGeneral));

	return iList[aIndex];
	}


// -----------------------------------------------------------------------------
// CAspDbNotifier::Session
// 
// -----------------------------------------------------------------------------
//
RSyncMLSession& CAspDbNotifier::Session()
	{
	__ASSERT_DEBUG(iSyncSession, TUtil::Panic(KErrGeneral));
	
	return *iSyncSession;
	}


// -----------------------------------------------------------------------------
// CAspDbNotifier::CallObserverWithDelay
//
// This is needed to avoid unnecessary observer calls (eg. in case 10 events are
// reported in short time only last is reported to observer). 
// -----------------------------------------------------------------------------
//
void CAspDbNotifier::CallObserverWithDelay(void)
	{
    FLOG( _L("CAspDbNotifier::CallObserverWithDelay START") );
    
    iActiveCaller->Cancel();
	iActiveCaller->Start(KErrNone, KListBoxUpdateDelay);
		
    FLOG( _L("CAspDbNotifier::CallObserverWithDelay END") );
	}


// -----------------------------------------------------------------------------
// CAspDbNotifier::CallObserver
//
// this function investigates received events from last observer call
// and calls observer to inform UI what to do. 
// -----------------------------------------------------------------------------
//
void CAspDbNotifier::CallObserver()
	{
    FLOG( _L("CAspDbNotifier::CallObserver START") );
	
	TAspDbEvent event(EUpdateAll);

	//
	// check for critical error
	//
	if (FindCloseEvent() != KErrNotFound)
		{
		event.iType = EClose;
		TRAP_IGNORE(iHandler->HandleDbEventL(event));
		
		FLOG( _L("CAspDbNotifier::CallObserver END") );
		return; // some database problem - UI should close
		}
	
	
	//
	// check if only one profile has changed
	//
	TInt index = FindSingleProfileEvent();
	if (index != KErrNotFound)
		{
		TAspDbEvent& e = iList[index];
        event.iProfileId = e.iProfileId; 
        
        event.iType = EUpdate;
        if (ProfileDeleted())
			{
			event.iType = EDelete;
			}
	
		TRAP_IGNORE(iHandler->HandleDbEventL(event));
		
        FLOG( _L("CAspDbNotifier::CallObserver END") );
        return;
		}
	
		
	//
	// check if 2 or more profiles have changed
	//
	if (FindProfileEvent() != KErrNotFound)
		{
	    event.iType = EUpdateAll;
    	TRAP_IGNORE(iHandler->HandleDbEventL(event));
	
        FLOG( _L("CAspDbNotifier::CallObserver END") );
        return;
    	}
    
    FLOG( _L("CAspDbNotifier::CallObserver END") );
	}


// -----------------------------------------------------------------------------
// CAspDbNotifier::FindCloseEvent
//
// -----------------------------------------------------------------------------
//
TInt CAspDbNotifier::FindCloseEvent()
	{
	TInt count = iList.Count();

	for (TInt i=0; i<count; i++)
		{
		TAspDbEvent& event = iList[i];
		if (event.iType == MSyncMLEventObserver::EServerTerminated)
			{
			return i;
			}
		}

	return KErrNotFound;
	}


// -----------------------------------------------------------------------------
// CAspDbNotifier::FindProfileEvent
//
// -----------------------------------------------------------------------------
//
TInt CAspDbNotifier::FindProfileEvent()
	{
	TInt count = iList.Count();

    for (TInt i=0; i<count; i++)
		{
		TAspDbEvent& event = iList[i];
		if (event.iProfileId != KErrNotFound)
			{
			return i;
			}
		}

	return KErrNotFound;
	}


// -----------------------------------------------------------------------------
// CAspDbNotifier::FindSingleProfileEvent
//
// -----------------------------------------------------------------------------
//
TInt CAspDbNotifier::FindSingleProfileEvent()
	{
	TInt count = iList.Count();

	// find out whether list contains update events for one profile only
	for (TInt i=0; i<count; i++)
		{
		TAspDbEvent& event = iList[i];
		if (event.iProfileId != KErrNotFound && IsUniqueProfileId(event.iProfileId))
			{
			return i;
        	}
		}

	return KErrNotFound;
	}


// -----------------------------------------------------------------------------
// CAspDbNotifier::ProfileDeleted
//
// -----------------------------------------------------------------------------
//
TBool CAspDbNotifier::ProfileDeleted()
	{
	TInt count = iList.Count();

	for (TInt i=0; i<count; i++)
		{
		TAspDbEvent& event = iList[i];
		if (event.iType == MSyncMLEventObserver::EProfileDeleted)
			{
			return ETrue;
			}
		}

	return EFalse;
	}


// -----------------------------------------------------------------------------
// CAspDbNotifier::IsUniqueProfileId
//
// -----------------------------------------------------------------------------
//
TBool CAspDbNotifier::IsUniqueProfileId(TInt aId)
	{
	TInt count = iList.Count();

    for (TInt i=0; i<count; i++)
		{
		TAspDbEvent& event = iList[i];
		if (event.iProfileId != KErrNotFound && event.iProfileId != aId)
			{
			return EFalse;
			}
		}

	return ETrue;
	}


// -----------------------------------------------------------------------------
// CAspDbNotifier::HandleActiveCallL (from MAspActiveCallerObserver)
//
// -----------------------------------------------------------------------------
//
void CAspDbNotifier::HandleActiveCallL(TInt /*aCallId*/)
	{
    FLOG( _L("CAspDbNotifier::HandleActiveCallL START") );

	if (iDisabled)
		{
		FLOG( _L("CAspDbNotifier::HandleActiveCallL: event observing disabled") );
		FLOG( _L("CAspDbNotifier::HandleActiveCallL END") );
		return;  // UI has disabled notifications
		}

    TBool destroyed(EFalse);
    iDestroyedPtr = &destroyed;

	CallObserver();
	
    if (!destroyed)   // destructor sets this to ETrue
        {
    	Reset();
    	iDestroyedPtr = NULL;
        }
	
    FLOG( _L("CAspDbNotifier::HandleActiveCallL END") );
	}



#ifdef _DEBUG

// -----------------------------------------------------------------------------
// LogSessionEvent
//
// -----------------------------------------------------------------------------
//
void CAspDbNotifier::LogSessionEvent(TEvent& aEvent, TInt aIdentifier, TInt aError)
	{
    TBuf<KBufSize> eventStr;
    GetSyncEventText(eventStr, aEvent);

	if (aError == KErrNone)
		{
		FTRACE( RDebug::Print(_L("event='%S' id=%d"), &eventStr, aIdentifier) );
		}
	else
		{
		FTRACE( RDebug::Print(_L("### event='%S' id=%d err=%d ###"), &eventStr, aIdentifier, aError) );
		}
	}


// -----------------------------------------------------------------------------
// GetSyncEventText
//
// -----------------------------------------------------------------------------
//
void CAspDbNotifier::GetSyncEventText(TDes& aText, MSyncMLEventObserver::TEvent aEvent)
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
