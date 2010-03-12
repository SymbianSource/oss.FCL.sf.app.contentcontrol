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
* Description:  Header file for contact adapter's datastore unit tests.
*
*/

#ifndef __CNSMLDUMMYDATASTORE_TEST_H__
#define __CNSMLDUMMYDATASTORE_TEST_H__

// INCLUDES
#include <CEUnitTestSuiteClass.h>
#include <SmlDataSyncDefs.h>
#include <cntdb.h>
#include <s32mem.h>
#include <MVPbkContactViewBase.h>
#include <MVPbkContactViewObserver.h>
#include <CVPbkContactManager.h>

// FORWARD DECLARATIONS
class CNSmlDummyDataStore;
class CNSmlAdapterTestUtilities;
class CNSmlDummyDataStoreWaiter;
class CSmlDataStore;
// CLASS DEFINITION

/**
 * Generated EUnit test suite class.
 */
NONSHARABLE_CLASS( CNSmlDummyDataStore_Test )
	: public CEUnitTestSuiteClass, 
	public MSmlSyncRelationship,
    public MVPbkContactViewObserver
    {
    public:  // Constructors and destructor

        static CNSmlDummyDataStore_Test* NewL();
        static CNSmlDummyDataStore_Test* NewLC();
        ~CNSmlDummyDataStore_Test();

    private: // Constructors

        CNSmlDummyDataStore_Test();
        void ConstructL();

    private: // New methods

        void Empty() {};

        // ++ Setup methods ++
        void SetupL();
        
        void SetupAndOpenStoreL();
        
        void SetupAndPopulateContactDbL();
        
        void SetupPopulateDbAndOpenStoreL();

        // ++ Teardown methods ++
        void Teardown();
        
        void TeardownAndCloseStore();
        
        void TeardownAndEmptyContactDb();
        
        void TeardownCloseAndEmptyContactDb();

        // ++ DataStore test cases ++
        void NewLL();

        void DoOpenLL();

        void DoHasSyncHistoryL();

        void DoModifiedItemsL();

        void DoResetChangeInfoLL();

        void DoCommitChangeInfoLL();

        void AddSingleItemL();

        void AddEmptyBatchL();

        void AddTwoItemBatchL();
        
        void CancelBatchL();

        void FetchItemL();

        void FetchItem2L();
        
        void UpdateItemL();

        void DeleteItemL();

        void DeleteAllItemsL();

        void GetStoreFormatResourceFileLL();

        void RemoveItemFromAddBatchL();

        void TestStoreSupportMimeTypeL();
        
        void DoBeginTransactionL();
        
        void DoCommitTransactionL();
        
        void DoRevertTransaction();
        
        void StoreNameL();
        
        void FindChangedItemsL();
        
        void MoveAndDeleteL();
        
        void ExecuteMoveL();
        
        void DoSetRemoteMaxObjectSize();
        
        void DoMaxObjectSize();
        
        void LdoFetchItemL();
        
        void ContactsSavingFailedL();
        
        void StepFailed();
        
        void GetDataMod();
 
//

        // ++ Helper methods ++
        TContactItemId AddContactL( const TDesC& aFirstName,
                 const TDesC& aLastName,
                 const TDesC& aPhoneNumber );
        TInt CountItemsLeftInBatchL( CBufBase& aItems );
        
        void CreateViewL();
        
        void CreateDataStoreL();

        void DeleteDataStoreL();
        
        // From MVPbkContactViewObserver  
        void ContactViewReady( MVPbkContactViewBase& aView ) ;
       
        void ContactViewUnavailable(
                    MVPbkContactViewBase& aView ) ;
        
        void ContactAddedToView(
                    MVPbkContactViewBase& aView, 
                    TInt aIndex, 
                    const MVPbkContactLink& aContactLink );
        
        void ContactRemovedFromView(
                    MVPbkContactViewBase& aView, 
                    TInt aIndex, 
                    const MVPbkContactLink& aContactLink ) ;
             
        void ContactViewError(
                    MVPbkContactViewBase& aView, 
                    TInt aError, 
                    TBool aErrorNotified ) ;
        
    private: // from MSmlSyncRelationship
        TSmlSyncTaskKey SyncTaskKey() const;
        
        void OpenReadStreamLC(RReadStream& aReadStream, TUid aStreamUid);
        
        void OpenWriteStreamLC(RWriteStream& aWriteStream, TUid aStreamUid);
        
        TBool IsStreamPresentL(TUid aStreamUid) const;

    private: // Data

        CNSmlDummyDataStore* iCNSmlDummyDataStore;
        CNSmlAdapterTestUtilities* iTestUtilities;

        CContactDatabase*   iContactDatabase;
        CContactIdArray*    iIdArray;
        RFs                 iFsSession;
        RFileWriteStream    iWriteStream;
        TSmlDbItemUid       iItemUid;
        TSmlDbItemUid       iItemUid2;

        CNSmlDummyDataStoreWaiter* iStoreWaiter;
        MVPbkContactViewBase* iContactViewBase;
        CVPbkContactManager* iContactManager;
        
        CSmlDataStore* iDataStore;

        EUNIT_DECLARE_TEST_TABLE;
    };

#endif      //  __CNSMLDUMMYDATASTORE_TEST_H__
