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
* Description:  Source code file for contact adapter's datastore unit tests.
*
*/

#include "CNSmlDummyDataStore_Test.h"
#include "CNSmlAdapterTestUtilities.h"
#include <EUnitMacros.h>
#include <EUnitDecorators.h>
#include <cntdb.h>
#include <cntitem.h>
#include <cntfield.h>
#include <cntfldst.h>
#include <s32mem.h>
#include <BAUTILS.H>
#include <SmlDataFormat.h>
#include <CVPbkContactViewDefinition.h>
#include <CVPbkContactStoreUriArray.h>
#include <tvpbkcontactstoreuriptr.h>
#include <VPbkContactStoreUris.h>
#include "nsmldummydatastore.h"
#include "cnsmldummydatastorewaiter.h"

namespace
    {
    // "abld test export" will export test data.
    _LIT( KVCardStandard, "c:\\Data\\TestData\\standard.vcf" );
    _LIT( KVCardBasic, "c:\\Data\\TestData\\basic.vcf" );
    _LIT( KVCardBasicInvalid, "c:\\Data\\TestData\\basicInvalid.vcf" );
    _LIT( KVCardMultiOccurence, "c:\\Data\\TestData\\multioccurence.vcf" );
    }


// - Construction -----------------------------------------------------------

CNSmlDummyDataStore_Test* CNSmlDummyDataStore_Test::NewL()
    {
    CNSmlDummyDataStore_Test* self = CNSmlDummyDataStore_Test::NewLC();
    CleanupStack::Pop( self );
    return self;
    }

CNSmlDummyDataStore_Test* CNSmlDummyDataStore_Test::NewLC()
    {
    CNSmlDummyDataStore_Test* self = new( ELeave ) CNSmlDummyDataStore_Test();
    CleanupStack::PushL( self );
    self->ConstructL();
    return self;
    }

CNSmlDummyDataStore_Test::~CNSmlDummyDataStore_Test()
    {
    iWriteStream.Close();
    iFsSession.Close();
    delete iCNSmlDummyDataStore;
    delete iTestUtilities;
    delete iContactDatabase;
    delete iIdArray;
    delete iStoreWaiter;
    }

CNSmlDummyDataStore_Test::CNSmlDummyDataStore_Test()
    {
    }

void CNSmlDummyDataStore_Test::ConstructL()
    {
    CEUnitTestSuiteClass::ConstructL();
    User::LeaveIfError( iFsSession.Connect() );
    }

// from MSmlSyncRelationship
TSmlSyncTaskKey CNSmlDummyDataStore_Test::SyncTaskKey() const
    {
    TSmlSyncTaskKey key = 0;
    return key;
    }

void CNSmlDummyDataStore_Test::OpenReadStreamLC(
        RReadStream& /*aReadStream*/, TUid /*aStreamUid*/)
    {
    }

void CNSmlDummyDataStore_Test::OpenWriteStreamLC(
        RWriteStream& aWriteStream, TUid /*aStreamUid*/ )
    {
    User::LeaveIfError(
            iWriteStream.Replace( iFsSession, _L("store.test"), EFileWrite  ) );
    aWriteStream = iWriteStream;
    CleanupClosePushL( aWriteStream );
    }

TBool CNSmlDummyDataStore_Test::IsStreamPresentL(TUid /*aStreamUid*/) const
    {
    return EFalse;
    }

// - Test methods -----------------------------------------------------------

// ++++++++++++ SETUP METHODS ++++++++++++

void CNSmlDummyDataStore_Test::SetupL(  )
    {
    iCNSmlDummyDataStore = CNSmlDummyDataStore::NewL();
    iTestUtilities = CNSmlAdapterTestUtilities::NewL();
    // Create contact manager
    CVPbkContactStoreUriArray* uriArray = CVPbkContactStoreUriArray::NewLC();
    uriArray->AppendL( TVPbkContactStoreUriPtr( VPbkContactStoreUris::DefaultCntDbUri() ) );
    iContactManager = CVPbkContactManager::NewL(*uriArray);
    CleanupStack::PopAndDestroy(uriArray);
    }

void CNSmlDummyDataStore_Test::SetupAndOpenStoreL(  )
    {
    SetupL();

    iStoreWaiter =
        CNSmlDummyDataStoreWaiter::NewL( *iCNSmlDummyDataStore );
    iStoreWaiter->OpenStoreL( *this );

    iContactDatabase = CContactDatabase::OpenL();
    }

void CNSmlDummyDataStore_Test::SetupPopulateDbAndOpenStoreL()
    {
    SetupAndPopulateContactDbL();

    EUNIT_ASSERT_NO_LEAVE(
    iCNSmlDummyDataStore = CNSmlDummyDataStore::NewL() );

    iStoreWaiter =
        CNSmlDummyDataStoreWaiter::NewL( *iCNSmlDummyDataStore );
    iStoreWaiter->OpenStoreL( *this );
    }

/**
 * Setup contact DB with the following contacts
 *   + Tim Tester (id index 0)
 *   + Teddy McTest (id index 1)
 *   + Tod TestHill (id index 2)
 *   + Simon Software (id index 3)
 *   + Cyd Effect (id index 4)
 *
 */
void CNSmlDummyDataStore_Test::SetupAndPopulateContactDbL()
    {
    if( !iContactDatabase )
        {
        iContactDatabase = CContactDatabase::OpenL();
        }
    if( !iTestUtilities )
        {
        iTestUtilities = CNSmlAdapterTestUtilities::NewL();
        }

    iIdArray = CContactIdArray::NewL();

    // Create test contacts
    iIdArray->AddL( AddContactL( _L( "Tim" ), _L( "Tester" ), _L( "+358501234567" ) ) );
    iIdArray->AddL( AddContactL( _L( "Teddy" ), _L("McTest" ), _L( "+358501112222" ) ) );
    iIdArray->AddL( AddContactL( _L( "Tod" ), _L( "TestHill" ), _L( "+358506666666" ) ) );
    iIdArray->AddL( AddContactL( _L( "Simon" ), _L( "Software" ), _L( "+3585011223333" ) ) );
    iIdArray->AddL( AddContactL( _L( "Cyd" ), _L( "Effect" ), _L( "+358503334444" ) ) );
    }

//// ++++++++++++ TEARDOWN METHODS ++++++++++++
//
void CNSmlDummyDataStore_Test::Teardown()
    {
    delete iCNSmlDummyDataStore;
    iCNSmlDummyDataStore = NULL;

    delete iTestUtilities;
    iTestUtilities = NULL;
    delete iContactManager;
    iContactManager = NULL;
    }

void CNSmlDummyDataStore_Test::TeardownAndCloseStore()
    {
    delete iStoreWaiter;
    iStoreWaiter = NULL;

    delete iContactDatabase;
    iContactDatabase = NULL;

    Teardown();
    }

void CNSmlDummyDataStore_Test::TeardownCloseAndEmptyContactDb()
    {
    delete iStoreWaiter;
    iStoreWaiter = NULL;

    delete iCNSmlDummyDataStore;
    iCNSmlDummyDataStore = NULL;

    TeardownAndEmptyContactDb();
    }

void CNSmlDummyDataStore_Test::TeardownAndEmptyContactDb()
    {
    if (iContactDatabase && iIdArray)
        {
        TRAP_IGNORE( iContactDatabase->DeleteContactsL( *iIdArray ) );
        }
    
    delete iIdArray;
    iIdArray = NULL;
    delete iContactDatabase;
    iContactDatabase = NULL;
    delete iTestUtilities;
    iTestUtilities = NULL;
    }

// ++++++++++++ UNIT TEST METHODS ++++++++++++

// ====================== CNSmlDummyDataStore ===========================

// MEMORY ALLOCATION FOR DATASTORE
void CNSmlDummyDataStore_Test::NewLL(  )
    {
    CNSmlDummyDataStore* dataStore = NULL;
    EUNIT_ASSERT_NO_LEAVE( dataStore = CNSmlDummyDataStore::NewL() );
    EUNIT_ASSERT( dataStore );
    delete dataStore;
    }

// DoOpenL()
void CNSmlDummyDataStore_Test::DoOpenLL()
    {
    CNSmlDummyDataStoreWaiter* storeWaiter =
    CNSmlDummyDataStoreWaiter::NewLC( *iCNSmlDummyDataStore );

    TRAPD( err, storeWaiter->OpenStoreL( *this ) );

    // Check for leave
    EUNIT_PRINT( _L( "Leave code: %d" ), err );
    EUNIT_ASSERT_DESC( err == KErrNone, "DoOpenL() leave" );
    CleanupStack::PopAndDestroy( storeWaiter );
    }

// DoHasSyncHistory()
void CNSmlDummyDataStore_Test::DoHasSyncHistoryL()
    {
    TBool ret = iCNSmlDummyDataStore->DoHasSyncHistory();
    EUNIT_ASSERT_DESC( ret == EFalse, "DoHasSyncHistory returns faulty value");
    }

// DoModifiedItems(), DoCommitChangeInfoL()
void CNSmlDummyDataStore_Test::DoModifiedItemsL()
    {
    const MSmlDataItemUidSet& uiSet = iCNSmlDummyDataStore->DoModifiedItems();
    EUNIT_ASSERT_DESC( uiSet.ItemCount() == 0, "DoModifiedItems returns faulty uid set");

    TInt returnCode = KErrNone;

    TRAPD( err, returnCode = iStoreWaiter->DoCommitChangeInfoL( uiSet ) );

    // Check for leave
    EUNIT_PRINT( _L( "Leave code: %d" ), err );
    EUNIT_ASSERT_DESC( err == KErrNone, "DoCommitChangeInfoL() leave" );

    // Check for return code
    EUNIT_PRINT( _L( "Return code: %d" ), returnCode );
    EUNIT_ASSERT_DESC( returnCode == KErrNone, "DoCommitChangeInfoL returns error");
    }


// DoResetChangeInfoL()
void CNSmlDummyDataStore_Test::DoResetChangeInfoLL()
    {
    // Temporarily remove ChangeFinders from Store to prevent leaves
    // when reseting ChangeFinders in code under test.
    CNSmlChangeFinder* cF = iCNSmlDummyDataStore->iChangeFinder;
    iCNSmlDummyDataStore->iChangeFinder = NULL;

    TInt returnCode = KErrNone;
    TRAPD( err, returnCode = iStoreWaiter->DoResetChangeInfoL() );

    // Return ChangeFinders
    iCNSmlDummyDataStore->iChangeFinder = cF;

    // Check for leave
    EUNIT_PRINT( _L( "Leave code: %d" ), err );
    EUNIT_ASSERT_DESC( err == KErrNone, "DoResetChangeInfoL() leave" );

    // Check for return code
    EUNIT_PRINT( _L( "Return code: %d" ), returnCode );
    EUNIT_ASSERT_DESC( returnCode == KErrNone, "DoResetChangeInfoL returns error");
    }

// DoCommitChangeInfoL()
void CNSmlDummyDataStore_Test::DoCommitChangeInfoLL()
    {
    TInt returnCode = KErrNone;
    TRAPD( err, returnCode = iStoreWaiter->DoCommitChangeInfoL() );

    // Check for leave
    EUNIT_PRINT( _L( "Leave code: %d" ), err );
    EUNIT_ASSERT_DESC( err == KErrNone, "DoCommitChangeInfoL() leave" );

    // Check for return code
    EUNIT_PRINT( _L( "Return code: %d" ), returnCode );
    EUNIT_ASSERT_DESC( returnCode == KErrNone, "DoCommitChangeInfoL returns error");
    }

// Item addition - "Normal" (non-batch) mode
//  (DoCreateItemL(), DoWriteItemL(), DoCommitItemL())
void CNSmlDummyDataStore_Test::AddSingleItemL()
    {
    TBuf8< 20 > type;
    TBuf8< 20 > ver = KNSmlvCard21Ver();
    TInt returnCode = KErrNone;
    iItemUid = KNullContactId;

    // Try creating contact with illegal MIME type
    EUNIT_PRINT( _L("Calling DoCreateItemL(), STEP 1") );
    TRAPD( err, returnCode = iStoreWaiter->DoCreateItemL(
        iItemUid, 0, type, ver ) );

    EUNIT_PRINT( _L( "DoCreateItemL(): leave code: %d, return code: %d" ),
        err, returnCode );
    EUNIT_ASSERT_DESC( returnCode == KErrNotSupported,
        "DoCreateItemL() accepts illegal MIME" );

    // Create legal contact
    type = KNSmlvCard21Name();
    EUNIT_PRINT( _L("Calling DoCreateItemL(), STEP 2") );
    TRAP( err, returnCode = iStoreWaiter->DoCreateItemL(
        iItemUid, 512, type, ver ) );

    EUNIT_PRINT( _L( "DoCreateItemL(): leave code: %d, return code: %d" ),
        err, returnCode );
    EUNIT_ASSERT_DESC( err == KErrNone, "DoCreateItemL() leave" );
    EUNIT_ASSERT_DESC( returnCode == KErrNone, "DoCreateItemL returns error");

    // Perform item writing to data store
    HBufC8* vCard = iTestUtilities->ReadVCard8FromFileLC( KVCardBasic() );
    EUNIT_PRINT( _L("Calling DoWriteItemL(), STEP 3") );
    // Non-async call (no need for iStoreWaiter usage)
    TRAP( err, iCNSmlDummyDataStore->DoWriteItemL( *vCard ) );
    EUNIT_PRINT( _L( "DoWriteItemL(): leave code: %d" ), err );
    EUNIT_ASSERT_DESC( err == KErrNone, "DoCreateItemL() leave" );
    CleanupStack::PopAndDestroy( vCard );

    // Commit the contact to be created
    EUNIT_PRINT( _L("Calling DoCommmitItemL(), STEP 4") );
    TRAP( err, returnCode = iStoreWaiter->DoCommitItemL() );

    EUNIT_PRINT( _L( "DoCommitItemL(): leave code: %d, return code: %d" ),
        err, returnCode );
    EUNIT_ASSERT_DESC( err == KErrNone, "DoCommitItemL() leave" );
    EUNIT_ASSERT_DESC( returnCode == KErrNone, "DoCommitItemL returns error");

    // Check the imported item
    EUNIT_ASSERT_DESC( iItemUid != KNullContactId, "Illegal UID for imported contact" );
    EUNIT_PRINT( _L("Item ID: %d"), iItemUid );

    // Print out the imported vCard
    CContactIdArray* idArr = CContactIdArray::NewLC();
    idArr->AddL( iItemUid );
    CBufFlat* buffer = CBufFlat::NewL( 1024 );
    CleanupStack::PushL( buffer );
    RBufWriteStream writeStream( *buffer );
    CleanupClosePushL( writeStream );

    TRAP( err, iContactDatabase->ExportSelectedContactsL( TUid::Uid(KVersitEntityUidVCard), *idArr,
        writeStream, CContactDatabase::EExcludeUid, Versit::EUTF8CharSet ) );

    EUNIT_ASSERT_EQUALS_DESC( err, KErrNone, "ExportSelectedContactsL leave" );

    CleanupStack::PopAndDestroy( &writeStream );

    EUNIT_PRINT( buffer->Ptr( 0 ) );

    // Do some asserts
    EUNIT_ASSERT( buffer->Ptr( 0 ).Find( _L8("N:Smith;John") ) != KErrNotFound );
    EUNIT_ASSERT( buffer->Ptr( 0 ).Find( _L8("TITLE:Boss") ) != KErrNotFound );

    CleanupStack::PopAndDestroy( 2, idArr );  // buffer, idArr

    // Do cleanup (remove added contact)
    CContactIdArray* tmpArray = CContactIdArray::NewLC();
    tmpArray->AddL( iItemUid );
    TRAP_IGNORE( iContactDatabase->DeleteContactsL( *tmpArray ) );
    CleanupStack::PopAndDestroy( tmpArray );
    }


// Item addition - Batch mode, empty batch
//  (DoBeginBatchL(), DoCommitBatchL())
void CNSmlDummyDataStore_Test::AddEmptyBatchL()
    {
    // DoBeginBatchL()
    TRAPD( err, iCNSmlDummyDataStore->DoBeginBatchL() );
    EUNIT_PRINT( _L( "DoBeginBatchL(): leave code: %d" ), err );
    EUNIT_ASSERT_DESC( err == KErrNone, "DoBeginBatchL() leave" );

    // DoCommitBatchL()
    TInt returnCode = KErrNone;
    RArray< TInt > array;
    TRAP( err, returnCode = iStoreWaiter->DoCommitBatchL( array ) );

    EUNIT_PRINT( _L( "DoCommitBatchL(): leave code: %d, return code: %d" ),
        err, returnCode );
    EUNIT_ASSERT_DESC( err == KErrNone, "DoCommitBatchL() leave" );
    EUNIT_ASSERT_DESC( returnCode == KErrNone, "DoCommitBatchL returns error");
    }

// Item addition - Batch mode with two contacts
void CNSmlDummyDataStore_Test::AddTwoItemBatchL()
    {
    TBuf8<20> type = KNSmlvCard21Name();
    TBuf8<20> ver = KNSmlvCard21Ver();
    TInt returnCode = KErrNone;
    HBufC8* vCard = NULL;

    // DoBeginBatchL()
    EUNIT_PRINT( _L("Calling DoCreateItemL(), STEP 1") );
    TRAPD( err, iCNSmlDummyDataStore->DoBeginBatchL() );
    EUNIT_PRINT( _L( "DoBeginBatchL(): leave code: %d" ), err );
    EUNIT_ASSERT_DESC( err == KErrNone, "DoBeginBatchL() leave" );

    // == == CONTACT ITEM NO 1 == ==
    vCard = iTestUtilities->ReadVCard8FromFileLC( KVCardBasic() );
    EUNIT_PRINT( _L("Calling DoCreateItemL(), STEP 2") );
    TRAP( err, returnCode = iStoreWaiter->DoCreateItemL(
        iItemUid, vCard->Size(), type, ver ) );

    EUNIT_PRINT( _L( "DoCreateItemL(): leave code: %d, return code: %d" ),
        err, returnCode );
    EUNIT_ASSERT_DESC( err == KErrNone, "DoCreateItemL() leave" );
    EUNIT_ASSERT_DESC( returnCode == KErrNone, "DoCreateItemL returns error");

    // Perform item writing to data store
    EUNIT_PRINT( _L("Calling DoWriteItemL(), STEP 3") );
    // Non-async call (no need for iStoreWaiter usage)
    TRAP( err, iCNSmlDummyDataStore->DoWriteItemL( *vCard ) );
    EUNIT_PRINT( _L( "DoWriteItemL(): leave code: %d" ), err );
    EUNIT_ASSERT_DESC( err == KErrNone, "DoCreateItemL() leave" );
    CleanupStack::PopAndDestroy( vCard );
    vCard = NULL;

    // Commit the contact to be created
    EUNIT_PRINT( _L("Calling DoCommitItemL(), STEP 4") );
    TRAP( err, returnCode = iStoreWaiter->DoCommitItemL() );

    EUNIT_PRINT( _L( "DoCommitItemL(): leave code: %d, return code: %d" ),
        err, returnCode );
    EUNIT_ASSERT_DESC( err == KErrNone, "DoCommitItemL() leave" );
    EUNIT_ASSERT_DESC( returnCode == KErrNone, "DoCommitItemL returns error");

    // == == CONTACT ITEM NO 2 == ==
    vCard = iTestUtilities->ReadVCard8FromFileLC( KVCardStandard() );

    EUNIT_PRINT( _L("Calling DoCreateItemL(), STEP 5") );
    TRAP( err, returnCode = iStoreWaiter->DoCreateItemL(
        iItemUid2, vCard->Size(), type, ver ) );
    EUNIT_PRINT( _L( "DoCreateItemL(): leave code: %d, return code: %d" ),
        err, returnCode );
    EUNIT_ASSERT_DESC( err == KErrNone, "DoCreateItemL() leave" );
    EUNIT_ASSERT_DESC( returnCode == KErrNone, "DoCreateItemL returns error" );

    // Perform item writing to data store
    EUNIT_PRINT( _L("Calling DoWriteItemL(), STEP 6") );
    // Non-async call (no need for iStoreWaiter usage)
    TRAP( err, iCNSmlDummyDataStore->DoWriteItemL( *vCard ) );
    EUNIT_PRINT( _L( "DoWriteItemL(): leave code: %d" ), err );
    EUNIT_ASSERT_DESC( err == KErrNone, "DoCreateItemL() leave" );
    CleanupStack::PopAndDestroy( vCard );

    // Commit the contact to be created
    EUNIT_PRINT( _L("Calling DoCommitItemL(), STEP 7") );
    TRAP( err, returnCode = iStoreWaiter->DoCommitItemL() );
    EUNIT_PRINT( _L( "DoCommitItemL(): leave code: %d, return code: %d" ),
        err, returnCode );
    EUNIT_ASSERT_DESC( err == KErrNone, "DoCommitItemL() leave" );
    EUNIT_ASSERT_DESC( returnCode == KErrNone, "DoCommitItemL returns error" );

    // Commit batch
    RArray<TInt> array;
    EUNIT_PRINT( _L("Calling DoCommitBatchL(), STEP 8") );
    TRAP( err, returnCode = iStoreWaiter->DoCommitBatchL( array ) );

    EUNIT_PRINT( _L( "DoCommitBatchL(): leave code: %d, return code: %d" ),
        err, returnCode );
    EUNIT_ASSERT_DESC( err == KErrNone, "DoCommitBatchL() leave" );
    EUNIT_ASSERT_DESC( returnCode == KErrNone, "DoCommitBatchL returns error" );

    // Verify results
    for( TInt i = 0; i < array.Count(); i++ )
        {
        EUNIT_PRINT( _L( "Contact: %d, Error code: %d "), i, array[ i ] );
        EUNIT_ASSERT( array[ i ] == KErrNone );
        }
    EUNIT_ASSERT_DESC( array.Count() == 2, "Wrong number of contacts added" );
    EUNIT_PRINT( _L("Item 1 ID = %d"), iItemUid );
    EUNIT_PRINT( _L("Item 2 ID = %d"), iItemUid2 );

    // Cleanup
    array.Reset();
    array.Close();

    CContactIdArray* tmpArray = CContactIdArray::NewLC();
    tmpArray->AddL( iItemUid );
    tmpArray->AddL( iItemUid2 );
    TRAP_IGNORE( iContactDatabase->DeleteContactsL( *tmpArray ) );
    CleanupStack::PopAndDestroy( tmpArray );
    }

// Item addition - cancel Batch mode 
void CNSmlDummyDataStore_Test::CancelBatchL()
    {
    TBuf8<20> type = KNSmlvCard21Name();
    TBuf8<20> ver = KNSmlvCard21Ver();
    TInt returnCode = KErrNone;
    HBufC8* vCard = NULL;

    // DoBeginBatchL()
    EUNIT_PRINT( _L("Calling DoCreateItemL(), STEP 1") );
    TRAPD( err, iCNSmlDummyDataStore->DoBeginBatchL() );
    EUNIT_PRINT( _L( "DoBeginBatchL(): leave code: %d" ), err );
    EUNIT_ASSERT_DESC( err == KErrNone, "DoBeginBatchL() leave" );

    // == == CONTACT ITEM NO 1 == ==
    vCard = iTestUtilities->ReadVCard8FromFileLC( KVCardBasic() );
    EUNIT_PRINT( _L("Calling DoCreateItemL(), STEP 2") );
    TRAP( err, returnCode = iStoreWaiter->DoCreateItemL(
        iItemUid, vCard->Size(), type, ver ) );

    EUNIT_PRINT( _L( "DoCreateItemL(): leave code: %d, return code: %d" ),
        err, returnCode );
    EUNIT_ASSERT_DESC( err == KErrNone, "DoCreateItemL() leave" );
    EUNIT_ASSERT_DESC( returnCode == KErrNone, "DoCreateItemL returns error");

    // Perform item writing to data store
    EUNIT_PRINT( _L("Calling DoWriteItemL(), STEP 3") );
    // Non-async call (no need for iStoreWaiter usage)
    TRAP( err, iCNSmlDummyDataStore->DoWriteItemL( *vCard ) );
    EUNIT_PRINT( _L( "DoWriteItemL(): leave code: %d" ), err );
    EUNIT_ASSERT_DESC( err == KErrNone, "DoCreateItemL() leave" );
    CleanupStack::PopAndDestroy( vCard );
    vCard = NULL;

    // Cancel Batch
    EUNIT_PRINT( _L("Calling DoCancelBatch(), STEP 4") );
    iStoreWaiter->DoCancelBatch();
    }

// DoOpenItemL(), DoReadItemL(), DoCloseItemL()
//  (LdoFetchItemL(), SingleContactOperationCompleteL())
void CNSmlDummyDataStore_Test::FetchItemL()
    {
    TBuf8<20> type;
    TBuf8<20> ver;
    TInt returnCode = KErrNone;
    TInt size = 0;
    TSmlDbItemUid parentId = KNullContactId;
    TBool fieldChange = EFalse;

    // Open item ("Tim Tester") for reading
    EUNIT_PRINT( _L("Calling DoOpenItemL()") );

    //Make some preparations:
    // (a) Set remote store format; needed by StripTxL()
    //      - Otherwise we get access violation
    EUNIT_ASSERT( iCNSmlDummyDataStore->iStoreFormat );
    iCNSmlDummyDataStore->DoSetRemoteStoreFormatL(
        *(iCNSmlDummyDataStore->iStoreFormat) );

    TRAPD( err, returnCode = iStoreWaiter->DoOpenItemL(
        ( *iIdArray )[ 0 ], fieldChange, size, parentId, type, ver ) );

    EUNIT_PRINT( _L( "DoOpenItemL(): leave code: %d, return code: %d" ),
        err, returnCode );
    EUNIT_ASSERT_DESC( err == KErrNone, "DoOpenItemL() leave" );
    EUNIT_ASSERT_DESC( returnCode == KErrNone, "DoOpenItemL returns error");

    // Execute some asserts to verify functionality
    EUNIT_PRINT( _L( "DoOpenItemL(): size: %d" ),
            size );
    EUNIT_ASSERT_DESC( size != 0, "Illegal size value" );
    EUNIT_ASSERT_DESC( parentId == KNullDataItemId, "Wrong parent ID" );
    EUNIT_ASSERT_DESC( type.Compare( KNSmlvCard21Name() ) == 0,
        "Illegal MIME type" );
    EUNIT_ASSERT_DESC( ver.Compare( KNSmlvCard21Ver() ) == 0,
        "Illegal MIME version" );

    // Read the item contents (i.e. vCard)
    HBufC8* vCard = HBufC8::NewLC( 86 );
    TPtr8 vCardPtr = vCard->Des();
    EUNIT_ASSERT_NO_LEAVE( iCNSmlDummyDataStore->DoReadItemL( vCardPtr ) );

    EUNIT_PRINT( *vCard );
    EUNIT_ASSERT( vCard->Find( _L8("N:Tester;Tim") ) != KErrNotFound );
    EUNIT_ASSERT( vCard->Find( _L8("TEL:+358501234567") ) != KErrNotFound );

    CleanupStack::PopAndDestroy( vCard );

    // Close the contact item
    iCNSmlDummyDataStore->DoCloseItem();
    }

void CNSmlDummyDataStore_Test::FetchItem2L()
    {
    TBuf8<20> type;
    TBuf8<20> ver;
    TInt returnCode = KErrNone;
    TInt size = 0;
    TSmlDbItemUid parentId = KNullContactId;
    TBool fieldChange = EFalse;

    TRAPD( err, returnCode = iStoreWaiter->DoOpenItemL(
        10000, fieldChange, size, parentId, type, ver ) );

    EUNIT_PRINT( _L( "DoOpenItemL(): leave code: %d, return code: %d" ),
        err, returnCode );
    EUNIT_ASSERT_DESC( err == KErrNone, "DoOpenItemL() leave" );
    EUNIT_ASSERT_DESC( returnCode == KErrNone || returnCode == KErrNotFound, "DoOpenItemL returns error");
    }

// Update a standard contact
//  (DoReplaceItemL(), ExecuteBufferL(), ExecuteUpdateL(), ...)
void CNSmlDummyDataStore_Test::UpdateItemL()
    {
    TInt returnCode = KErrNone;
    TSmlDbItemUid parentId = KNullContactId;
    TBool fieldChange = EFalse;

    HBufC8* vCard = iTestUtilities->ReadVCard8FromFileLC( KVCardStandard() );

    // Update "Tim Tester" contact
    TRAPD( err, returnCode = iStoreWaiter->DoReplaceItemL(
        ( *iIdArray )[ 0 ], vCard->Size(), parentId, fieldChange ) );

    EUNIT_PRINT( _L( "DoReplaceItemL(): leave code: %d, return code: %d" ),
        err, returnCode );
    EUNIT_ASSERT_DESC( err == KErrNone, "DoReplaceItemL() leave" );
    EUNIT_ASSERT_DESC( returnCode == KErrNone, "DoReplaceItemL returns error");

    // Perform item writing to data store
    EUNIT_PRINT( _L("Calling DoWriteItemL()") );
    TRAP( err, iCNSmlDummyDataStore->DoWriteItemL( *vCard ) );
    EUNIT_PRINT( _L( "DoWriteItemL(): leave code: %d" ), err );
    EUNIT_ASSERT_DESC( err == KErrNone, "DoWriteItemL() leave" );
    CleanupStack::PopAndDestroy( vCard );

    // Commit the contact to be created
    EUNIT_PRINT( _L("Calling DoCommmitItemL()") );
    TRAP( err, returnCode = iStoreWaiter->DoCommitItemL() );

    EUNIT_PRINT( _L( "DoCommitItemL(): leave code: %d, return code: %d" ),
        err, returnCode );
    EUNIT_ASSERT_DESC( err == KErrNone, "DoCommitItemL() leave" );
    EUNIT_ASSERT_DESC( returnCode == KErrNone, "DoCommitItemL returns error");

    // Print out the imported vCard
    CContactIdArray* idArr = CContactIdArray::NewLC();
    idArr->AddL( ( *iIdArray )[ 0 ] );
    CBufFlat* buffer = CBufFlat::NewL( 1024 );
    CleanupStack::PushL( buffer );
    RBufWriteStream writeStream( *buffer );
    CleanupClosePushL( writeStream );

    TRAP( err, iContactDatabase->ExportSelectedContactsL(
        TUid::Uid(KVersitEntityUidVCard), *idArr,
        writeStream, CContactDatabase::EExcludeUid, Versit::EUTF8CharSet ) );

    EUNIT_ASSERT_EQUALS_DESC( err, KErrNone, "ExportSelectedContactsL leave" );

    CleanupStack::PopAndDestroy( &writeStream );

    EUNIT_PRINT( buffer->Ptr( 0 ) );

    // Do some asserts
    EUNIT_ASSERT( buffer->Ptr( 0 ).Find( _L8("N:Smith;John") ) != KErrNotFound );
    EUNIT_ASSERT( buffer->Ptr( 0 ).Find( _L8("TITLE:Boss") ) != KErrNotFound );

    CleanupStack::PopAndDestroy( 2, idArr );  // buffer, idArr
    }

// Not working - memory leak: Heap cell count, 61, 73, Request count, 0, 1
//  The test case never receives OpetionComplete() callback from VPbk. Therefore
//  the request count & heap allocation do not match.
// DoDeleteItemL(), StepComplete()
void CNSmlDummyDataStore_Test::DeleteItemL()
    {
    // Delete item
    TInt returnCode = KErrNone;
    EUNIT_PRINT( _L("Calling DoDeleteItemL()") );
    TRAPD( err, returnCode = iStoreWaiter->DoDeleteItemL( ( *iIdArray )[ 0 ] ) );

    EUNIT_PRINT( _L( "DoDeleteItemL(): leave code: %d, return code: %d" ),
        err, returnCode );
    EUNIT_ASSERT_DESC( err == KErrNone, "DoDeleteItemL() leave" );
    EUNIT_ASSERT_DESC( returnCode == KErrNone, "DoDeleteItemL returns error");
    iCNSmlDummyDataStore->ExecuteDeleteL();
    // Check that the contact does not exist any more
    CContactItem* item = NULL;
    CContactDatabase* cntDb = NULL;
    TRAP( err, cntDb = CContactDatabase::OpenL() );
    TRAP( err, item = cntDb->ReadMinimalContactL(
        ( *iIdArray )[ 0 ] ) );
    EUNIT_ASSERT( !item );
    EUNIT_PRINT( _L("Error=%d"), err );
    EUNIT_ASSERT_EQUALS_DESC( err, KErrNotFound, "Deleted item found" );
    delete item;
    item = NULL;

    if( cntDb )
        {
        delete cntDb;
        }

    iIdArray->Remove( 0 );
    }

// DoDeleteAllItemsL(), DoDeleteAllContactsL(), ...
void CNSmlDummyDataStore_Test::DeleteAllItemsL()
    {
    // Temporarily remove ChangeFinders from Store to prevent leaves
    // when reseting ChangeFinders in code under test.
    CNSmlChangeFinder* cF = iCNSmlDummyDataStore->iChangeFinder;
    CleanupStack::PushL( cF );
    iCNSmlDummyDataStore->iChangeFinder = NULL;

    TInt returnCode = KErrNone;
    EUNIT_PRINT( _L("Calling DoDeleteAllItemsL()") );
    TRAPD( err, returnCode = iStoreWaiter->DoDeleteAllItemsL() );

    // Return ChangeFinders
    CleanupStack::Pop( cF );
    iCNSmlDummyDataStore->iChangeFinder = cF;

    EUNIT_PRINT( _L( "DoDeleteAllItemsL(): leave code: %d, return code: %d" ),
        err, returnCode );
    EUNIT_ASSERT_DESC( err == KErrNone, "DoDeleteAllItemsL() leave" );
    EUNIT_ASSERT_DESC( returnCode == KErrNone, "DoDeleteAllItemsL returns error");

    // Check that all normal contacts are removed
    TContactItemId ownCardId = iContactDatabase->OwnCardId();

    TInt contactCount = 0;
    TInt groupCount = iContactDatabase->GroupCount();
    TContactIter iter( *iContactDatabase );
    for( TContactItemId id = iter.FirstL(); id != KNullContactId; id = iter.NextL() )
        {
        // KUidContactGroup
        if( id != ownCardId )
            {
            contactCount++;
            }
        }
    EUNIT_PRINT( _L("Number of items in DB=%d"), contactCount );
    EUNIT_PRINT( _L("Number of groups in DB=%d"), groupCount );
    EUNIT_ASSERT_DESC( contactCount - groupCount == 0, "All contacts not deleted");
    }

// GetStoreFormatResourceFileL()
void CNSmlDummyDataStore_Test::GetStoreFormatResourceFileLL()
    {
    const TDesC& resourceFile = iCNSmlDummyDataStore->GetStoreFormatResourceFileL();
    EUNIT_PRINT( resourceFile );
    }


// VCARD IMPORT TEST CASES

void CNSmlDummyDataStore_Test::RemoveItemFromAddBatchL()
    {
    // Test case:
    // - Run RemoveItemFromAddBatch() and verify its functionality

    if( !iCNSmlDummyDataStore->iItemDataAddBatch )
        {
        iCNSmlDummyDataStore->iItemDataAddBatch =
            CBufFlat::NewL( KNSmlItemDataExpandSize );
        }

    HBufC8* vCard = NULL;
    // Test material: basic vCard
    vCard = iTestUtilities->ReadVCard8FromFileLC( KVCardBasic );
    iCNSmlDummyDataStore->iItemDataAddBatch->InsertL(
        iCNSmlDummyDataStore->iItemDataAddBatch->Size(),
        *vCard, vCard->Size() );
    iCNSmlDummyDataStore->iItemDataAddBatch->InsertL(
        iCNSmlDummyDataStore->iItemDataAddBatch->Size(),
        KVersitTokenCRLF() );
    CleanupStack::PopAndDestroy( vCard );

    // Test material 2: vCard with multiple TEL fields
    vCard = iTestUtilities->ReadVCard8FromFileLC( KVCardMultiOccurence );
    iCNSmlDummyDataStore->iItemDataAddBatch->InsertL(
        iCNSmlDummyDataStore->iItemDataAddBatch->Size(),
        *vCard, vCard->Size() );
    iCNSmlDummyDataStore->iItemDataAddBatch->InsertL(
        iCNSmlDummyDataStore->iItemDataAddBatch->Size(),
        KVersitTokenCRLF() );
    CleanupStack::PopAndDestroy( vCard );

//    iTestUtilities->PrintData( _L8("Buffer contents"),
//        *iCNSmlDummyDataStore->iItemDataAddBatch ); // DEBUG PRINT
    
    TInt count = 0;

    // ASSERTS
    count = CountItemsLeftInBatchL(
        *iCNSmlDummyDataStore->iItemDataAddBatch );
    EUNIT_PRINT( _L("Items left %d"), count ); // DEBUG PRINT
    EUNIT_ASSERT_DESC( count == 2, "Wrong initial item count in batch.")
//    EUNIT_PRINT( _L("Items left %d"), count ); // DEBUG PRINT
    }


void CNSmlDummyDataStore_Test::TestStoreSupportMimeTypeL()
    {
    EUNIT_ASSERT( iCNSmlDummyDataStore->iStoreFormat );
    EUNIT_ASSERT_EQUALS( iCNSmlDummyDataStore->iStoreFormat->MimeFormatCount() ,1 );
    EUNIT_ASSERT_EQUALS(
            iCNSmlDummyDataStore->iStoreFormat->MimeFormat(0).MimeType().DesC(),
            KNSmlvCard21Name() );
    EUNIT_ASSERT_EQUALS(
            iCNSmlDummyDataStore->iStoreFormat->MimeFormat(0).MimeVersion().DesC(),
                    KNSmlvCard21Ver() );
    }

void CNSmlDummyDataStore_Test::DoBeginTransactionL()
    {
    TRAPD( err, iCNSmlDummyDataStore->DoBeginTransactionL() );
    EUNIT_ASSERT( err == KErrNotSupported );
    }

void CNSmlDummyDataStore_Test::DoCommitTransactionL()
    {
    TRequestStatus status;
    iCNSmlDummyDataStore->DoCommitTransactionL( status );
    EUNIT_ASSERT_DESC( status.Int() == KErrNotSupported, "DoCommitTransactionL fail"  )
    }

void CNSmlDummyDataStore_Test::DoRevertTransaction()
    {
    TRequestStatus status;
    iCNSmlDummyDataStore->DoRevertTransaction( status );
    EUNIT_ASSERT_DESC( status.Int() == KErrNotSupported, "DoRevertTransaction fail"  )
    }

void CNSmlDummyDataStore_Test::StoreNameL()
    {
    CreateDataStoreL();
    EUNIT_PRINT( iDataStore->StoreName() );
    iDataStore->CancelRequest();
    DeleteDataStoreL();
    }

void CNSmlDummyDataStore_Test::FindChangedItemsL()
    {
    CreateDataStoreL();
    const MSmlDataItemUidSet& added = iDataStore->AddedItems();
    EUNIT_ASSERT_DESC( added.ItemCount() == 0, "AddedItems returns faulty uid set");
    const MSmlDataItemUidSet& deleted = iDataStore->DeletedItems();
    EUNIT_ASSERT_DESC( deleted.ItemCount() == 0, "DeletedItems returns faulty uid set");
    const MSmlDataItemUidSet& softDeleted = iDataStore->SoftDeletedItems();
    EUNIT_ASSERT_DESC( softDeleted.ItemCount() == 0, "SoftDeletedItems returns faulty uid set");
    const MSmlDataItemUidSet& moved = iDataStore->MovedItems();
    EUNIT_ASSERT_DESC( moved.ItemCount() == 0, "MovedItems returns faulty uid set");
    DeleteDataStoreL();
    }

void CNSmlDummyDataStore_Test::MoveAndDeleteL()
    {
    CreateDataStoreL();
    TSmlDbItemUid uid = 1;
    TSmlDbItemUid parent = 0;
    TRequestStatus status;
    iDataStore->MoveItemL( uid, parent, status );
    
    TRequestStatus status2;
    iDataStore->SoftDeleteItemL( uid, status2 );
        
    DeleteDataStoreL();
    }

void CNSmlDummyDataStore_Test::ExecuteMoveL()
    {
    iCNSmlDummyDataStore->ExecuteMoveL();
    }

void CNSmlDummyDataStore_Test::DoSetRemoteMaxObjectSize()
    {
    iCNSmlDummyDataStore->DoSetRemoteMaxObjectSize( 100000 );
    }

void CNSmlDummyDataStore_Test::DoMaxObjectSize()
    {
    TInt size = iCNSmlDummyDataStore->DoMaxObjectSize();
    EUNIT_ASSERT( size == 102400 );
    }

void CNSmlDummyDataStore_Test::ContactsSavingFailedL()
    {
    TBuf8< 20 > type= KNSmlvCard21Name();
    TBuf8< 20 > ver = KNSmlvCard21Ver();
    TInt returnCode = KErrNone;
    iItemUid = KNullContactId;
    // Create legal contact
    EUNIT_PRINT( _L("Calling DoCreateItemL(), STEP 1") );
    TRAPD( err, returnCode = iStoreWaiter->DoCreateItemL(
        iItemUid, 512, type, ver ) );

    EUNIT_PRINT( _L( "DoCreateItemL(): leave code: %d, return code: %d" ),
        err, returnCode );
    EUNIT_ASSERT_DESC( err == KErrNone, "DoCreateItemL() leave" );
    EUNIT_ASSERT_DESC( returnCode == KErrNone, "DoCreateItemL returns error");

    // Perform item writing to data store
    HBufC8* vCard = iTestUtilities->ReadVCard8FromFileLC( KVCardBasicInvalid() );
    EUNIT_PRINT( _L("Calling DoWriteItemL(), STEP 2") );
    // Non-async call (no need for iStoreWaiter usage)
    TRAP( err, iCNSmlDummyDataStore->DoWriteItemL( *vCard ) );
    EUNIT_PRINT( _L( "DoWriteItemL(): leave code: %d" ), err );
    EUNIT_ASSERT_DESC( err == KErrNone, "DoCreateItemL() leave" );
    CleanupStack::PopAndDestroy( vCard );

    // Commit the contact to be created
    EUNIT_PRINT( _L("Calling DoCommmitItemL(), STEP 3") );
    TRAP( err, returnCode = iStoreWaiter->DoCommitItemL() );

    EUNIT_PRINT( _L( "DoCommitItemL(): leave code: %d, return code: %d" ),
        err, returnCode );
    EUNIT_ASSERT_DESC( err == KErrNone, "DoCommitItemL() leave" );
    EUNIT_ASSERT_DESC( returnCode == KErrNotFound, "DoCommitItemL returns error");
    }

void CNSmlDummyDataStore_Test::StepFailed()
    {
    TInt returnCode = KErrNone;
    // delete contact which not exist
    EUNIT_PRINT( _L("Calling DoDeleteItemL()") );
    TRAPD( err, returnCode = iStoreWaiter->DoDeleteItemL( 10000 ) );
    EUNIT_PRINT( _L( "DoDeleteItemL(): leave code: %d, return code: %d" ),
        err, returnCode );
    }

void CNSmlDummyDataStore_Test::GetDataMod()
    {
    CNSmlDataModBase& dataMod = iCNSmlDummyDataStore->GetDataMod();
    }


// --------------- Helper methods --------------

// Helper method to add a simple contact to test contact DB
TContactItemId CNSmlDummyDataStore_Test::AddContactL(
        const TDesC& aFirstName,
        const TDesC& aLastName,
        const TDesC& aPhoneNumber )
    {
    _LIT( KLabelFamilyName, "Last name");
    _LIT( KLabelGivenName, "First name");
    _LIT( KLabelPhone, "Phone");

    CContactCard* card = CContactCard::NewLC();

    // Family name field
    CContactItemField* familyNameField =
        CContactItemField::NewLC( KStorageTypeText, KUidContactFieldFamilyName );
    familyNameField->SetLabelL( KLabelFamilyName );
    familyNameField->SetMapping( KUidContactFieldVCardMapUnusedN );
    familyNameField->TextStorage()->SetTextL( aLastName );
    card->AddFieldL( *familyNameField ); // Takes ownership
    CleanupStack::Pop( familyNameField );

    // First name field
    CContactItemField* nameField =
    CContactItemField::NewLC( KStorageTypeText, KUidContactFieldGivenName );
    nameField->SetLabelL( KLabelGivenName );
    nameField->SetMapping( KUidContactFieldVCardMapUnusedN );
    nameField->TextStorage()->SetTextL( aFirstName );
    card->AddFieldL( *nameField );
    CleanupStack::Pop(nameField);

    // Phone number field
    CContactItemField* phoneField =
        CContactItemField::NewLC( KStorageTypeText, KUidContactFieldPhoneNumber );
    phoneField->SetLabelL( KLabelPhone );
    phoneField->SetMapping( KUidContactFieldVCardMapTEL );
    phoneField->TextStorage()->SetTextL( aPhoneNumber );
    card->AddFieldL( *phoneField );
    CleanupStack::Pop( phoneField );

    TContactItemId cardId = iContactDatabase->AddNewContactL( *card );
    CleanupStack::PopAndDestroy( card );

    return cardId;
    }

TInt CNSmlDummyDataStore_Test::CountItemsLeftInBatchL( CBufBase& aItems )
    {
    _LIT8( KVCardBeginTag, "BEGIN:VCARD\r\n");
    _LIT8( KVCardEndTag, "\r\nEND:VCARD\r\n");

    TInt vCardCount = 0;
    TBool processed = EFalse;

    HBufC8* tmpBuf = HBufC8::NewLC( aItems.Size() );
    TPtr8 ptr = tmpBuf->Des();
    aItems.Read( 0, ptr, aItems.Size() );

    while( !processed )
        {
        TInt offsetBegin = ptr.FindF( KVCardBeginTag );
        TInt offsetEnd = ptr.FindF( KVCardEndTag );

        if( offsetBegin != KErrNotFound && offsetEnd != KErrNotFound )
            {
            offsetEnd += KVCardEndTag().Length();
            vCardCount++;
            ptr = ptr.MidTPtr( offsetEnd );
            }
        else
            {
            processed = ETrue;
            }
        }
    CleanupStack::PopAndDestroy( tmpBuf );
    return vCardCount;
    }

void CNSmlDummyDataStore_Test::CreateDataStoreL()
    {
    TSmlDataProviderId id = {0x101F6DDD};// Contact Adapter Uid
    CSmlDataProvider* dp = CSmlDataProvider::NewL( id );
    iDataStore = dp->NewStoreInstanceLC();
    CleanupStack::Pop( iDataStore );
    delete dp;
    }

void CNSmlDummyDataStore_Test::DeleteDataStoreL()
    {
    delete iDataStore;
    iDataStore = NULL;
    }

void CNSmlDummyDataStore_Test::CreateViewL()
    {
    CVPbkContactViewDefinition* viewDef = CVPbkContactViewDefinition::NewL();
    CleanupStack::PushL( viewDef );
    viewDef->SetType( EVPbkContactsView );

    iContactViewBase = iContactManager->CreateContactViewLC( 
                                         *this, 
                                         *viewDef, 
                                         iContactManager->FieldTypes() );
    CleanupStack::Pop();// iContactViewBase
    CleanupStack::PopAndDestroy( viewDef );
    } 

// ------------------  From MVPbkContactViewObserver ---------------------
void CNSmlDummyDataStore_Test::ContactViewReady( MVPbkContactViewBase& /*aView*/ )
    {
    }   
    
void CNSmlDummyDataStore_Test::ContactViewUnavailable(
            MVPbkContactViewBase& /*aView*/ )
    {
    }

void CNSmlDummyDataStore_Test::ContactAddedToView(
            MVPbkContactViewBase& /*aView*/, 
            TInt /*aIndex*/, 
            const MVPbkContactLink& /*aContactLink*/ )
    {
    }

void CNSmlDummyDataStore_Test::ContactRemovedFromView(
            MVPbkContactViewBase& /*aView*/, 
            TInt /*aIndex*/, 
            const MVPbkContactLink& /*aContactLink*/ )
    { 
    }
    
void CNSmlDummyDataStore_Test::ContactViewError(
            MVPbkContactViewBase& /*aView*/, 
            TInt /*aError*/, 
            TBool /*aErrorNotified*/ )
    { 
    }

// - EUnit test table -------------------------------------------------------

EUNIT_BEGIN_TEST_TABLE(
    CNSmlDummyDataStore_Test,
    "Unit tests Contact adapter datastore",
    "UNIT" )

EUNIT_TEST(
    "Test to verify memory allocation for the object",
    "CNSmlContactsDataStore",
    "NewL",
    "FUNCTIONALITY",
    Empty, NewLL, Empty)

EUNIT_TEST(
    "Open data store",
    "CNSmlContactsDataStore",
    "DoOpenL",
    "FUNCTIONALITY",
    SetupL, DoOpenLL, Teardown )

EUNIT_TEST(
    "DoHasSyncHistory",
    "CNSmlContactsDataStore",
    "DoHasSyncHistory",
    "FUNCTIONALITY",
    SetupAndOpenStoreL, DoHasSyncHistoryL, TeardownAndCloseStore )

EUNIT_TEST(
    "DoModifiedItems & DoCommitChangeInfoL",
    "CNSmlContactsDataStore",
    "DoModifiedItems",
    "FUNCTIONALITY",
    SetupAndOpenStoreL, DoModifiedItemsL, TeardownAndCloseStore )

EUNIT_TEST(
    "DoResetChangeInfoL",
    "CNSmlContactsDataStore",
    "DoResetChangeInfoL",
    "FUNCTIONALITY",
    SetupAndOpenStoreL, DoResetChangeInfoLL, TeardownAndCloseStore )

EUNIT_TEST(
    "DoCommitChangeInfoL",
    "CNSmlContactsDataStore",
    "DoCommitChangeInfoL",
    "FUNCTIONALITY",
    SetupAndOpenStoreL, DoCommitChangeInfoLL, TeardownAndCloseStore )

EUNIT_TEST(
    "Add single item (normal mode)",
    "CNSmlContactsDataStore",
    "DoCreateItemL, DoCommitItemL, ExecuteAddL, LdoAddItem",
    "FUNCTIONALITY",
    SetupAndOpenStoreL, AddSingleItemL, TeardownAndCloseStore )

/* TIMEOUT error
EUNIT_TEST(
    "Add items in batch mode (empty batch)",
    "CNSmlContactsDataStore",
    "DoBeginBatchL, DoCommitBatchL",
    "FUNCTIONALITY",
    SetupAndOpenStoreL, AddEmptyBatchL, TeardownAndCloseStore )*/
    
EUNIT_TEST(
    "Add items in batch mode (2 contacts)",
    "CNSmlContactsDataStore",
    "DoBeginBatchL, DoCommitBatchL",
    "FUNCTIONALITY",
    SetupAndOpenStoreL, AddTwoItemBatchL, TeardownAndCloseStore )

EUNIT_TEST(
    "CancelBatchL",
    "CNSmlContactsDataStore",
    "DoCancelBatch",
    "FUNCTIONALITY",
    SetupAndOpenStoreL, CancelBatchL, TeardownAndCloseStore ) 

EUNIT_TEST(
    "Fetch/read normal contact",
    "CNSmlContactsDataStore",
    "LdoFetchItemL",
    "FUNCTIONALITY",
    SetupPopulateDbAndOpenStoreL, FetchItemL, TeardownCloseAndEmptyContactDb )

EUNIT_TEST(
    "Fail fetch",
    "CNSmlContactsDataStore",
    "LdoFetchItemL",
    "FUNCTIONALITY",
    SetupPopulateDbAndOpenStoreL, FetchItem2L, TeardownCloseAndEmptyContactDb )
    
EUNIT_TEST(
    "Update normal contact",
    "CNSmlContactsDataStore",
    "ExecuteUpdateL",
    "FUNCTIONALITY",
    SetupPopulateDbAndOpenStoreL, UpdateItemL, TeardownCloseAndEmptyContactDb )

EUNIT_TEST(
    "Delete normal contact",
    "CNSmlContactsDataStore",
    "DoDeleteItemL",
    "FUNCTIONALITY",
    SetupPopulateDbAndOpenStoreL, DeleteItemL, TeardownCloseAndEmptyContactDb )

EUNIT_TEST(
    "Delete all contacts",
    "CNSmlContactsDataStore",
    "DoDeleteItemL",
    "FUNCTIONALITY",
    SetupPopulateDbAndOpenStoreL, DeleteAllItemsL, TeardownCloseAndEmptyContactDb )

EUNIT_TEST(
	"Test for GetStoreFormatResourceFileL function",
	"CNSmlContactsDataStore",
	"",
	"FUNCTIONALITY",
	SetupL, GetStoreFormatResourceFileLL, Teardown )

EUNIT_TEST(
    "RemoveItemFromAddBatch",
    "CNSmlContactsDataStore",
    "RemoveItemFromAddBatch",
    "FUNCTIONALITY",
    SetupL, RemoveItemFromAddBatchL, Teardown )

EUNIT_TEST(
    "Verify operator specific MIME type",
    "CNSmlContactsDataStore",
    "",
    "FUNCTIONALITY",
    SetupL, TestStoreSupportMimeTypeL, Teardown )

EUNIT_TEST(
    "GetDataMod",
    "CNSmlContactsDataStore",
    "",
    "FUNCTIONALITY",
    SetupL, GetDataMod, Teardown )

EUNIT_TEST(
    "DoBeginTransactionL",
    "CNSmlContactsDataStore",
    "",
    "FUNCTIONALITY",
    SetupL, DoBeginTransactionL, Teardown )

EUNIT_TEST(
    "DoCommitTransactionL",
    "CNSmlContactsDataStore",
    "",
    "FUNCTIONALITY",
    SetupL, DoCommitTransactionL, Teardown )

EUNIT_TEST(
    "DoRevertTransaction",
    "CNSmlContactsDataStore",
    "",
    "FUNCTIONALITY",
    SetupL, DoRevertTransaction, Teardown )

EUNIT_TEST(
    "StoreNameL",
    "CNSmlContactsDataStore",
    "",
    "FUNCTIONALITY",
    SetupL, StoreNameL, Teardown )
    
EUNIT_TEST(
    "FindChangedItemsL",
    "CNSmlContactsDataStore",
    "",
    "FUNCTIONALITY",
    SetupL, FindChangedItemsL, Teardown )   

EUNIT_TEST(
    "MoveAndDeleteL",
    "CNSmlContactsDataStore",
    "",
    "FUNCTIONALITY",
    SetupL, MoveAndDeleteL, Teardown )

EUNIT_TEST(
    "ExecuteMoveL (move command is not supported)",
    "CNSmlContactsDataStore",
    "",
    "FUNCTIONALITY",
    SetupL, ExecuteMoveL, Teardown )

EUNIT_TEST(
    "DoSetRemoteMaxObjectSize",
    "CNSmlContactsDataStore",
    "",
    "FUNCTIONALITY",
    SetupL, DoSetRemoteMaxObjectSize, Teardown )

EUNIT_TEST(
    "DoMaxObjectSize",
    "CNSmlContactsDataStore",
    "",
    "FUNCTIONALITY",
    SetupL, DoMaxObjectSize, Teardown )

EUNIT_TEST(
    "ContactsSavingFailedL",
    "CNSmlContactsDataStore",
    "",
    "FUNCTIONALITY",
    SetupAndOpenStoreL, ContactsSavingFailedL, TeardownAndCloseStore )

EUNIT_TEST(
    "StepFailed",
    "CNSmlContactsDataStore",
    "",
    "FUNCTIONALITY",
    SetupPopulateDbAndOpenStoreL, StepFailed, TeardownCloseAndEmptyContactDb )

EUNIT_END_TEST_TABLE
