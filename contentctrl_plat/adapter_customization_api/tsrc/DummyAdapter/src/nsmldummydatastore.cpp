/*
* Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  DS Dummy datastore
*
*/

//  CLASS HEADER
#include "nsmldummydatastore.h"

//  EXTERNAL INCLUDES
#include <DataSyncInternalPSKeys.h>
#include <e32property.h>

// INTERNAL INCLUDES
#include "nsmldebug.h"
#include "nsmldummyutility.h"

// -----------------------------------------------------------------------------
// CNSmlDummyDataStore::NewL
// -----------------------------------------------------------------------------
//
CNSmlDummyDataStore* CNSmlDummyDataStore::NewL()
    {
    CNSmlDummyDataStore* self = CNSmlDummyDataStore::NewLC();
    CleanupStack::Pop( self );
    return self;
    }

// -----------------------------------------------------------------------------
// CNSmlDummyDataStore::NewLC
// -----------------------------------------------------------------------------
//
CNSmlDummyDataStore* CNSmlDummyDataStore::NewLC()
    {
    CNSmlDummyDataStore* self = new( ELeave ) CNSmlDummyDataStore;
    CleanupStack::PushL( self );
    self->ConstructL();
    return self;
    }

// -----------------------------------------------------------------------------
// CNSmlDummyDataStore::CNSmlDummyDataStore
// -----------------------------------------------------------------------------
//
CNSmlDummyDataStore::CNSmlDummyDataStore()
    {
    _DBG_FILE("CNSmlDummyDataStore::CNSmlDummyDataStore(): begin");

    _DBG_FILE("CNSmlDummyDataStore::CNSmlDummyDataStore(): end");
    }

// -----------------------------------------------------------------------------
// CNSmlDummyDataStore::ConstructL
// -----------------------------------------------------------------------------
//
void CNSmlDummyDataStore::ConstructL()
    {
    _DBG_FILE("CNSmlDummyDataStore::ConstructL(): begin");

    CNSmlContactsDataStore::ConstructL();

    _DBG_FILE("CNSmlDummyDataStore::ConstructL(): end");
    }

// -----------------------------------------------------------------------------
// CNSmlDummyDataStore::~CNSmlDummyDataStore
// -----------------------------------------------------------------------------
//
CNSmlDummyDataStore::~CNSmlDummyDataStore()
    {
    _DBG_FILE("CNSmlDummyDataStore::~CNSmlDummyDataStore(): begin");

    _DBG_FILE("CNSmlDummyDataStore::~CNSmlDummyDataStore(): end");
    }

// -----------------------------------------------------------------------------
// void CNSmlDummyDataStore::DoOpenL
// -----------------------------------------------------------------------------
//
void CNSmlDummyDataStore::DoOpenL( const TDesC& aStoreName,
    MSmlSyncRelationship& aContext, TRequestStatus& aStatus )
    {
    _DBG_FILE("CNSmlDummyDataStore::DoOpenL(): begin");

    CNSmlContactsDataStore::DoOpenL( aStoreName, aContext, aStatus );

    _DBG_FILE("CNSmlDummyDataStore::DoOpenL(): end");
    }

// ----------------------------------------------------------------------------
// void CNSmlDummyDataStore::DoBeginBatchL
// ----------------------------------------------------------------------------
void CNSmlDummyDataStore::DoBeginBatchL()
    {
    _DBG_FILE("CNSmlDummyDataStore::DoBeginBatchL(): begin");

    CNSmlContactsDataStore::DoBeginBatchL();

    _DBG_FILE("CNSmlDummyDataStore::DoBeginBatchL(): end");
    }

// ----------------------------------------------------------------------------
// void CNSmlDummyDataStore::DoOpenItemL
// ----------------------------------------------------------------------------
void CNSmlDummyDataStore::DoOpenItemL( TSmlDbItemUid aUid,
    TBool& aFieldChange, TInt& aSize, TSmlDbItemUid& aParent,
    TDes8& aMimeType, TDes8& aMimeVer, TRequestStatus& aStatus )
    {
    _DBG_FILE("CNSmlDummyDataStore::DoOpenItemL(): begin");

	CNSmlContactsDataStore::DoOpenItemL( aUid, aFieldChange,
			aSize, aParent, aMimeType, aMimeVer, aStatus );

    _DBG_FILE("CNSmlDummyDataStore::DoOpenItemL(): end");
    }

// ----------------------------------------------------------------------------
// void CNSmlDummyDataStore::DoCommitItemL()
// ----------------------------------------------------------------------------
void CNSmlDummyDataStore::DoCommitItemL( TRequestStatus& aStatus )
    {
    _DBG_FILE("CNSmlDummyDataStore::DoCommitItemL(): begin");

    CNSmlContactsDataStore::DoCommitItemL( aStatus );

    _DBG_FILE("CNSmlDummyDataStore::DoCommitItemL(): end");
    }

// ----------------------------------------------------------------------------
// void CNSmlDummyDataStore::DoDeleteItemL
// ----------------------------------------------------------------------------
void CNSmlDummyDataStore::DoDeleteItemL(
    TSmlDbItemUid aUid, TRequestStatus& aStatus )
    {
    _DBG_FILE("CNSmlDummyDataStore::DoDeleteItemL(): begin");

    CNSmlContactsDataStore::DoDeleteItemL( aUid, aStatus );

    _DBG_FILE("CNSmlDummyDataStore::DoDeleteItemL(): end");
    }

// ----------------------------------------------------------------------------
// TBool CNSmlDummyDataStore::DoHasSyncHistory
// ----------------------------------------------------------------------------
TBool CNSmlDummyDataStore::DoHasSyncHistory() const
    {
    _DBG_FILE("CNSmlDummyDataStore::DoHasSyncHistory()");
    return CNSmlContactsDataStore::DoHasSyncHistory();
    }

// ----------------------------------------------------------------------------
// void CNSmlDummyDataStore::DoModifiedItems
// ----------------------------------------------------------------------------
//
const MSmlDataItemUidSet& CNSmlDummyDataStore::DoModifiedItems() const
    {
    _DBG_FILE("CNSmlDummyDataStore::DoModifiedItems()");
    return CNSmlContactsDataStore::DoModifiedItems();
    }

// ----------------------------------------------------------------------------
// void CNSmlDummyDataStore::DoResetChangeInfoL
// ----------------------------------------------------------------------------
//
void CNSmlDummyDataStore::DoResetChangeInfoL( TRequestStatus& aStatus )
    {
    _DBG_FILE("CNSmlDummyDataStore::DoResetChangeInfoL(): begin");

    CNSmlContactsDataStore::DoResetChangeInfoL( aStatus );

    _DBG_FILE("CNSmlDummyDataStore::DoResetChangeInfoL(): end");
    }

// ----------------------------------------------------------------------------
// void CNSmlDummyDataStore::DoCommitChangeInfoL
// ----------------------------------------------------------------------------
//
void CNSmlDummyDataStore::DoCommitChangeInfoL( TRequestStatus& aStatus,
    const MSmlDataItemUidSet& aItems )
    {
    _DBG_FILE("CNSmlDummyDataStore::DoCommitChangeInfoL(aItems): begin");

    CNSmlContactsDataStore::DoCommitChangeInfoL( aStatus, aItems );

    _DBG_FILE("CNSmlDummyDataStore::DoCommitChangeInfoL(aItems): end");
    }

// ----------------------------------------------------------------------------
// void CNSmlDummyDataStore::DoCommitChangeInfoL
// ----------------------------------------------------------------------------
//
void CNSmlDummyDataStore::DoCommitChangeInfoL( TRequestStatus& aStatus )
    {
    _DBG_FILE("CNSmlDummyDataStore::DoCommitChangeInfoL(): begin");

    CNSmlContactsDataStore::DoCommitChangeInfoL( aStatus );

    _DBG_FILE("CNSmlDummyDataStore::DoCommitChangeInfoL(): end");
    }

// ----------------------------------------------------------------------------
// void CNSmlDummyDataStore::LdoFetchItemL
// ----------------------------------------------------------------------------
void CNSmlDummyDataStore::LdoFetchItemL(
    TSmlDbItemUid& aUid, CBufBase& aItem )
    {
    _DBG_FILE("CNSmlDummyDataStore::LdoFetchItemL(): begin");

    CNSmlContactsDataStore::LdoFetchItemL( aUid, aItem );

    _DBG_FILE("CNSmlDummyDataStore::LdoFetchItemL(): end");
    }

// ----------------------------------------------------------------------------
// void CNSmlDummyDataStore::LdoAddItemL
// ----------------------------------------------------------------------------
void CNSmlDummyDataStore::LdoAddItemL( const TDesC8& aItem, TInt aSize )
    {
    _DBG_FILE("CNSmlDummyDataStore::LdoAddItemL(): begin");

    CNSmlContactsDataStore::LdoAddItemL( aItem, aSize );

    _DBG_FILE("CNSmlDummyDataStore::LdoAddItemL(): end");
    }

// ----------------------------------------------------------------------------
// void CNSmlDummyDataStore::LdoAddItemsL
// ----------------------------------------------------------------------------
void CNSmlDummyDataStore::LdoAddItemsL( CBufBase*& aItems, TInt aSize)
    {
    _DBG_FILE("CNSmlDummyDataStore::LdoAddItemsL(): begin");

	CNSmlContactsDataStore::LdoAddItemsL( aItems, aSize );

    _DBG_FILE("CNSmlDummyDataStore::LdoAddItemsL(): end");
    }

// ----------------------------------------------------------------------------
// TInt CNSmlDummyDataStore::ExecuteAddL
// ----------------------------------------------------------------------------
TInt CNSmlDummyDataStore::ExecuteAddL()
    {
    _DBG_FILE("CNSmlDummyDataStore::ExecuteAddL()");
    return CNSmlContactsDataStore::ExecuteAddL();
    }

// ----------------------------------------------------------------------------
// void CNSmlDummyDataStore::ExecuteDeleteL
// ----------------------------------------------------------------------------

void CNSmlDummyDataStore::ExecuteDeleteL()
    {
    _DBG_FILE("CNSmlDummyDataStore::ExecuteDeleteL(): begin");

    CNSmlContactsDataStore::ExecuteDeleteL();

    _DBG_FILE("CNSmlDummyDataStore::ExecuteDeleteL(): end");
    }

// ----------------------------------------------------------------------------
// void CNSmlDummyDataStore::ExecuteUpdateL
// ----------------------------------------------------------------------------
void CNSmlDummyDataStore::ExecuteUpdateL()
    {
	_DBG_FILE("CNSmlDummyDataStore::ExecuteUpdateL(): begin");

    CNSmlContactsDataStore::ExecuteUpdateL();

	_DBG_FILE("CNSmlDummyDataStore::ExecuteUpdateL(): end");
    }

// ----------------------------------------------------------------------------
// void CNSmlDummyDataStore::SingleContactOperationCompleteL
// ----------------------------------------------------------------------------
void CNSmlDummyDataStore::SingleContactOperationCompleteL(
    MVPbkStoreContact* aContact )
    {
    _DBG_FILE("CNSmlDummyDataStore::SingleContactOperationCompleteL(): begin");

	CNSmlContactsDataStore::SingleContactOperationCompleteL( aContact );

    _DBG_FILE("CNSmlDummyDataStore::SingleContactOperationCompleteL(): end");
    }

// ----------------------------------------------------------------------------
// void CNSmlDummyDataStore::ContactsSaved
// ----------------------------------------------------------------------------
void CNSmlDummyDataStore::ContactsSaved(
    MVPbkContactOperationBase& aOperation, MVPbkContactLinkArray* aResults )
    {
    _DBG_FILE("CNSmlDummyDataStore::ContactsSaved(): begin");

	CNSmlContactsDataStore::ContactsSaved( aOperation, aResults );

    _DBG_FILE("CNSmlDummyDataStore::ContactsSaved(): end");
    }

// ----------------------------------------------------------------------------
// void CNSmlDummyDataStore::ContactsSavingFailed
// ----------------------------------------------------------------------------
void CNSmlDummyDataStore::ContactsSavingFailed(
    MVPbkContactOperationBase& aOperation, TInt aError )
    {
    _DBG_FILE("CNSmlDummyDataStore::ContactsSavingFailed(): begin");

	CNSmlContactsDataStore::ContactsSavingFailed( aOperation, aError );

    _DBG_FILE("CNSmlDummyDataStore::ContactsSavingFailed(): end");
    }

// ----------------------------------------------------------------------------
// void CNSmlDummyDataStore::StepComplete
// ----------------------------------------------------------------------------
void CNSmlDummyDataStore::StepComplete(
    MVPbkContactOperationBase& aOperation, TInt aStepSize )
    {
    _DBG_FILE("CNSmlDummyDataStore::StepComplete(): begin");

	CNSmlContactsDataStore::StepComplete( aOperation, aStepSize );

    _DBG_FILE("CNSmlDummyDataStore::StepComplete(): end");
    }

// ----------------------------------------------------------------------------
// void CNSmlDummyDataStore::OperationCompleteL
// ----------------------------------------------------------------------------
void CNSmlDummyDataStore::OperationCompleteL()
    {
    _DBG_FILE("CNSmlDummyDataStore::OperationCompleteL(): begin");

    CNSmlContactsDataStore::OperationCompleteL();

    _DBG_FILE("CNSmlDummyDataStore::OperationCompleteL(): end");
    }


// ----------------------------------------------------------------------------
// void CNSmlDummyDataStore::DoDeleteAllContactsL
// ----------------------------------------------------------------------------
void CNSmlDummyDataStore::DoDeleteAllContactsL()
    {
    _DBG_FILE("CNSmlDummyDataStore::DoDeleteAllContactsL(): begin");

    CNSmlContactsDataStore::DoDeleteAllContactsL();

    _DBG_FILE("CNSmlDummyDataStore::DoDeleteAllContactsL(): end");
    }

// -----------------------------------------------------------------------------
// const TDesC& CNSmlDummyDataStore::GetStoreFormatResourceFileL
// -----------------------------------------------------------------------------
const TDesC& CNSmlDummyDataStore::GetStoreFormatResourceFileL() const
    {
    _DBG_FILE("CNSmlDummyDataStore::GetStoreFormatResourceFileL()");
    // Check correct Data Sync protocol
    TInt value( EDataSyncNotRunning );
    TInt error = RProperty::Get( KPSUidDataSynchronizationInternalKeys,
                                 KDataSyncStatus,
                                 value );
    if ( error == KErrNone && value == EDataSyncRunning )
        {
        return KNSmlDummyContactsStoreFormatRsc_1_1_2;
        }
    else // error or protocol version 1.2
        {
        return KNSmlDummyContactsStoreFormatRsc_1_2;
        }
    }

// ----------------------------------------------------------------------------
// MContactsModsFetcher* CNSmlDummyDataStore::CreateModsFetcherL
// ----------------------------------------------------------------------------
MContactsModsFetcher* CNSmlDummyDataStore::CreateModsFetcherL()
    {
    _DBG_FILE("CNSmlDummyDataStore::CreateModsFetcherL()");

    return CNSmlContactsDataStore::CreateModsFetcherL();
    }


// END OF FILE
