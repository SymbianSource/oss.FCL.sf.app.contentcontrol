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
* Description:  DS Dummy dataprovider
*
*/

//  CLASS HEADER
#include "nsmldummydataprovider.h"

//  EXTERNAL INCLUDES
#include <DataSyncInternalPSKeys.h>
#include <e32property.h>

// INTERNAL INCLUDES
#include "nsmldebug.h"
#include "nsmldummydatastore.h"
#include "nsmldummyutility.h"


// -----------------------------------------------------------------------------
// CNSmlDummyDataProvider::NewL
// -----------------------------------------------------------------------------
//
CNSmlDummyDataProvider* CNSmlDummyDataProvider::NewL()
    {
    CNSmlDummyDataProvider* self = CNSmlDummyDataProvider::NewLC();
    CleanupStack::Pop( self );
    return self;
    }

// -----------------------------------------------------------------------------
// CNSmlDummyDataProvider::NewLC
// -----------------------------------------------------------------------------
//
CNSmlDummyDataProvider* CNSmlDummyDataProvider::NewLC()
    {
    CNSmlDummyDataProvider* self = new( ELeave ) CNSmlDummyDataProvider;
    CleanupStack::PushL( self );
    self->ConstructL();
    return self;
    }

// -----------------------------------------------------------------------------
// CNSmlDummyDataProvider::~CNSmlDummyDataProvider
// -----------------------------------------------------------------------------
//
CNSmlDummyDataProvider::~CNSmlDummyDataProvider()
    {
    _DBG_FILE("CNSmlDummyDataProvider::~CNSmlDummyDataProvider(): begin");

    _DBG_FILE("CNSmlDummyDataProvider::~CNSmlDummyDataProvider(): end");
    }

// -----------------------------------------------------------------------------
// CNSmlDummyDataProvider::CNSmlDummyDataProvider
// -----------------------------------------------------------------------------
//
CNSmlDummyDataProvider::CNSmlDummyDataProvider()
    {
    _DBG_FILE("CNSmlDummyDataProvider::CNSmlDummyDataProvider(): begin");

    _DBG_FILE("CNSmlDummyDataProvider::CNSmlDummyDataProvider(): end");
    }

// -----------------------------------------------------------------------------
// CNSmlDummyDataProvider::ConstructL
// -----------------------------------------------------------------------------
//
void CNSmlDummyDataProvider::ConstructL()
    {
    _DBG_FILE("CNSmlDummyDataProvider::ConstructL(): begin");
    CNSmlContactsDataProvider::ConstructL();
    _DBG_FILE("CNSmlDummyDataProvider::ConstructL(): end");
    }

// -----------------------------------------------------------------------------
// CNSmlDummyDataProvider::DoStoreFormatL
// -----------------------------------------------------------------------------
//
const CSmlDataStoreFormat& CNSmlDummyDataProvider::DoStoreFormatL()
    {
    _DBG_FILE("CNSmlDummyDataProvider::DoStoreFormatL()");
    return CNSmlContactsDataProvider::DoStoreFormatL();
    }

// -----------------------------------------------------------------------------
// CNSmlDummyDataProvider::GetStoreFormatResourceFileL
// -----------------------------------------------------------------------------
//
const TDesC& CNSmlDummyDataProvider::GetStoreFormatResourceFileL() const
    {
    _DBG_FILE("CNSmlDummyDataProvider::DoStoreFormatL()");

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

// ------------------------------------------------------------------------------------------------
// CNSmlDummyDataProvider::CreateDataStoreLC
//
// ------------------------------------------------------------------------------------------------
CNSmlContactsDataStore* CNSmlDummyDataProvider::CreateDataStoreLC() const
    {
    _DBG_FILE("CNSmlDummyDataProvider::CreateDataStoreLC()");
    CNSmlContactsDataStore* dataStore = CNSmlDummyDataStore::NewLC();
    return dataStore;
    }

// -----------------------------------------------------------------------------
// CNSmlDummyDataProvider::DoListStoresLC
// -----------------------------------------------------------------------------
//
CDesCArray* CNSmlDummyDataProvider::DoListStoresLC()
    {
    _DBG_FILE("CNSmlDummyDataProvider::DoListStoresLC()");
    return CNSmlContactsDataProvider::DoListStoresLC();
    }

// End of file
