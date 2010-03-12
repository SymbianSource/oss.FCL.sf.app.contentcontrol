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
* Description:  Source code file for contact adapter's modfetcher unit tests.
*
*/

#include "CNSmlDummyModsFetcher_Test.h"
#include <EUnitMacros.h>
#include <EUnitDecorators.h>
#include <CNTDB.H>
#include <cntfield.h>
#include <CNTFLDST.H>
#include <nsmlsnapshotitem.h>
#include <e32hashtab.h>
#include <nsmlchangefinder.h>

// VPbk includes
#include <cvpbkcontactstoreuriarray.h>
#include <tvpbkcontactstoreuriptr.h>
#include <cvpbkcontactmanager.h>
#include <VPbkContactStoreUris.h>
#include <MVPbkContactStoreList.h>
#include <CVPbkContactViewDefinition.h>
#include <MVPbkContactLink.h>

#include "CNSmlAdapterTestUtilities.h"
#include "nsmldummymodsfetcher.h"

namespace
    {
    _LIT( KTestDB, "c:eunitdb" );
    }

//// Implementation UID for contacts adapter
const TUint KNSmlContactsAdapterImplUid = 0x101F6DDD;

// - Construction -----------------------------------------------------------

CNSmlDummyModsFetcher_Test* CNSmlDummyModsFetcher_Test::NewL()
    {
    CNSmlDummyModsFetcher_Test* self = CNSmlDummyModsFetcher_Test::NewLC();
    CleanupStack::Pop();
    return self;
    }

CNSmlDummyModsFetcher_Test* CNSmlDummyModsFetcher_Test::NewLC()
    {
    CNSmlDummyModsFetcher_Test* self = new( ELeave ) CNSmlDummyModsFetcher_Test();
    CleanupStack::PushL( self );
    self->ConstructL();
    return self;
    }

CNSmlDummyModsFetcher_Test::~CNSmlDummyModsFetcher_Test()
    {
    iReadStream.Close();
    delete iTestData;

    delete iCNSmlDummyModsFetcher;

    delete iChangeFinder;

    delete iTestUtilities;

    delete iContactManager;
    }

CNSmlDummyModsFetcher_Test::CNSmlDummyModsFetcher_Test():
    iKey( TKeyArrayFix( _FOFF( TNSmlSnapshotItem, ItemId() ), ECmpTInt ))
    {
    }

void CNSmlDummyModsFetcher_Test::ConstructL()
    {
    CEUnitTestSuiteClass::ConstructL();
    }

// - Test methods -----------------------------------------------------------

// ++++++++++++ SETUP METHODS ++++++++++++

void CNSmlDummyModsFetcher_Test::SetupL(  )
    {
    EUNIT_PRINT( _L("SetupL: begin"));
    iTestUtilities = CNSmlAdapterTestUtilities::NewL();
    iTestUtilities->SetupAndPopulateContactDbL( KTestDB );
    iContactDatabase = iTestUtilities->GetDb();

    iTestData = HBufC8::NewL( 1024 );
    iReadStream.Open( *iTestData );

    EUNIT_PRINT( _L("SetupL: contact mgr"));

    // Create contact manager
    CVPbkContactStoreUriArray* uriArray = CVPbkContactStoreUriArray::NewLC();
    uriArray->AppendL( TVPbkContactStoreUriPtr( VPbkContactStoreUris::DefaultCntDbUri() ) );
    iContactManager = CVPbkContactManager::NewL(*uriArray);
    CleanupStack::PopAndDestroy(uriArray);

    // Create store and open it
    EUNIT_PRINT( _L("SetupL: store"));
    iStore = iContactManager->ContactStoresL().Find(
        TVPbkContactStoreUriPtr( VPbkContactStoreUris::DefaultCntDbUri() ) );
    iStore->OpenL( *this );

    iSyncHistory = EFalse;
    iSnapshotRegistered = EFalse;

    // Create platform ChangeFinder
    EUNIT_PRINT( _L("SetupL: ChangeFinder"));
    TRAPD(err, iChangeFinder = CNSmlChangeFinder::NewL(
        *this, iKey, iSyncHistory, KNSmlContactsAdapterImplUid ));
    if( err != KErrNone )
        {
        EUNIT_PRINT( _L("ChangeFinder Error: %d"), err );
        }

    // Finally, create ModsFetcher && assert allocation
    EUNIT_PRINT( _L("SetupL: ModsFetcher"));
    iCNSmlDummyModsFetcher = new (ELeave) CNSmlDummyModsFetcher(
        iSnapshotRegistered,
        *iContactManager,
        *iStore,
        iKey,
        *iChangeFinder );

    EUNIT_ASSERT( iCNSmlDummyModsFetcher );

    iCNSmlDummyModsFetcher->ConstructL();

    iLastChanged = Time::NullTTime();

    EUNIT_PRINT( _L("SetupL -end"));
    }

void CNSmlDummyModsFetcher_Test::SetupCreateItemL()
    {
    SetupL();
    iCardId = iTestUtilities->AddContactL( _L("Test"), KNullDesC(),
        KNullDesC() );
    }


// ++++++++++++ TEARDOWN METHODS ++++++++++++

void CNSmlDummyModsFetcher_Test::Teardown()
    {
    iReadStream.Close();
    delete iTestData;
    iTestData = NULL;

    EUNIT_PRINT( _L("Teardown"));
    delete iCNSmlDummyModsFetcher;
    iCNSmlDummyModsFetcher = NULL;
    EUNIT_PRINT( _L("Teardown-2"));

    delete iChangeFinder;
    EUNIT_PRINT( _L("Teardown-3"));

    EUNIT_PRINT( _L("Teardown-4"));

    EUNIT_PRINT( _L("Teardown-5"));
    if ( iContactManager )
        {
            if(iStore)
            {
            iStore->Close( *this );
            }
        delete iContactManager;
        iContactManager = NULL;
        }
    EUNIT_PRINT( _L("Teardown-6"));
    TRAP_IGNORE( iTestUtilities->TeardownAndEmptyContactDbL( KTestDB ) );
    delete iTestUtilities;
    iTestUtilities = NULL;
    iCardId = 0;
    EUNIT_PRINT( _L("Teardown-7"));
    }

void CNSmlDummyModsFetcher_Test::CreateViewL()
    {
    CVPbkContactViewDefinition* viewDef = CVPbkContactViewDefinition::NewL();
    CleanupStack::PushL( viewDef );
    viewDef->SetType( EVPbkContactsView );

    //viewDef->SetUriL( iStore->StoreProperties().Uri().UriDes() );

    iContactViewBase = iContactManager->CreateContactViewLC( 
                                         *this, 
                                         *viewDef, 
                                         iContactManager->FieldTypes() );
    CleanupStack::Pop();
    CleanupStack::PopAndDestroy( viewDef );
    } 

// ++++++++++++ ACTUAL TEST METHODS ++++++++++++

// ------------------ From MSmlSyncRelationship ---------------------
TSmlSyncTaskKey CNSmlDummyModsFetcher_Test::SyncTaskKey() const
    {
    EUNIT_PRINT( _L("SyncTaskKey"));
    return 0;
    }

void CNSmlDummyModsFetcher_Test::OpenReadStreamLC(RReadStream& aReadStream, TUid /*aStreamUid*/)
    {
    EUNIT_PRINT( _L("OpenReadStreamLC"));
    aReadStream = iReadStream;
    CleanupClosePushL( iReadStream );

    EUNIT_PRINT( _L("OpenReadStreamLC - end"));
    }

void CNSmlDummyModsFetcher_Test::OpenWriteStreamLC(RWriteStream& /*aWriteStream*/, TUid /*aStreamUid*/)
    {
    EUNIT_PRINT( _L("OpenWriteStreamLC"));
    }

TBool CNSmlDummyModsFetcher_Test::IsStreamPresentL(TUid /*aStreamUid*/) const
    {
    EUNIT_PRINT( _L("IsStreamPresentL"));
    return EFalse;
    }
// ---------------- MSmlSyncRelationship END -----------------------

// ------------------ From MVPbkContactStoreObserver ---------------------
void CNSmlDummyModsFetcher_Test::StoreReady(
    MVPbkContactStore& /*aContactStore*/ )
    {
    }

void CNSmlDummyModsFetcher_Test::StoreUnavailable(
    MVPbkContactStore& /*aContactStore*/, TInt /*aReason*/)
    {
    }

void CNSmlDummyModsFetcher_Test::HandleStoreEventL(
        MVPbkContactStore& /*aContactStore*/,
        TVPbkContactStoreEvent /*aStoreEvent*/)
    {
    }

// ------------------  From MVPbkContactViewObserver ---------------------
void CNSmlDummyModsFetcher_Test::ContactViewReady( MVPbkContactViewBase& /*aView*/ )
    {
    }   
    
void CNSmlDummyModsFetcher_Test::ContactViewUnavailable(
            MVPbkContactViewBase& /*aView*/ )
    {
    }

void CNSmlDummyModsFetcher_Test::ContactAddedToView(
            MVPbkContactViewBase& /*aView*/, 
            TInt /*aIndex*/, 
            const MVPbkContactLink& /*aContactLink*/ )
    {
    }

void CNSmlDummyModsFetcher_Test::ContactRemovedFromView(
            MVPbkContactViewBase& /*aView*/, 
            TInt /*aIndex*/, 
            const MVPbkContactLink& /*aContactLink*/ )
    { 
    }
    
void CNSmlDummyModsFetcher_Test::ContactViewError(
            MVPbkContactViewBase& /*aView*/, 
            TInt /*aError*/, 
            TBool /*aErrorNotified*/ )
    { 
    }

// ---------------- MVPbkContactStoreObserver END -----------------------

// Helper functions

CContactItemField* CNSmlDummyModsFetcher_Test::CreateConfidentialLC()
    {
    CContactItemField* confidentialField =
        CContactItemField::NewLC( KStorageTypeText, KUidContactFieldClass );
    confidentialField->SetMapping( KUidContactFieldVCardMapClass );
    confidentialField->TextStorage()->SetTextL( KNSmlContactSyncNoSync );
    EUNIT_PRINT( _L("CreateConfidentialLC - END"));
    return confidentialField;
    }


// - EUnit test table -------------------------------------------------------

EUNIT_BEGIN_TEST_TABLE(
    CNSmlDummyModsFetcher_Test,
    "Unit tests for CNSmlDummyModsFetcher.",
    "UNIT" )

EUNIT_TEST(
    "CNSmlContactsModsFetcher allocation",
    "CNSmlContactsModsFetcher",
    "Allocation",
    "FUNCTIONALITY",
    SetupL, Empty, Teardown)

EUNIT_END_TEST_TABLE
