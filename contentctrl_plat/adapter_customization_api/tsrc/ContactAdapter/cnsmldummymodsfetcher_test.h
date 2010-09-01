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
* Description:  Header file for contact adapter's modfetcher unit tests.
*
*/

#ifndef __CNSMLDUMMYMODSFETCHER_TEST_H__
#define __CNSMLDUMMYMODSFETCHER_TEST_H__

// INCLUDES
#include <CEUnitTestSuiteClass.h>
#include <EUnitDecorators.h>
#include <SmlDataSyncDefs.h>
#include <S32MEM.H>
#include <CNTDEF.H>
#include <MVPbkContactStore.h>
#include <MVPbkContactStoreObserver.h>
#include <MVPbkContactViewObserver.h>
#include <MVPbkContactViewBase.h>

// FORWARD DECLARATIONS
class CNSmlDummyModsFetcher;
class CContactDatabase;
class CNSmlChangeFinder;
class CNSmlAdapterTestUtilities;
class CContactItemField;
class CVPbkContactManager;


// CLASS DEFINITION
/**
 * Generated EUnit test suite class.
 */
NONSHARABLE_CLASS( CNSmlDummyModsFetcher_Test ):
    public CEUnitTestSuiteClass,
	public MSmlSyncRelationship,
	public MVPbkContactStoreObserver,
	public MVPbkContactViewObserver
    {
    public:  // Constructors and destructor

        static CNSmlDummyModsFetcher_Test* NewL();
        static CNSmlDummyModsFetcher_Test* NewLC();
        ~CNSmlDummyModsFetcher_Test();

    private: // Constructors

        CNSmlDummyModsFetcher_Test();
        void ConstructL();

    private: // New methods

        void Empty() {};

        void SetupL();
        void SetupCreateItemL();

        void Teardown();
        
        void CreateViewL();

        void RunError();
        
        void ContactViewUnavailable();
        
        void ContactRemovedFromView();
        
        void ContactViewError();
        
        void VPbkSingleContactOperationFailed();

    public: // MSmlSyncRelationship

        /**
        Returns the unique identifier for the synchronisation relationship.
        @return             A unique identifier for the sync relationship.
        */
        TSmlSyncTaskKey SyncTaskKey() const;

        /**
        Opens a specified stream for reading, and places an item on the cleanup stack to close it.
        @param               aReadStream    On return, an open read stream
        @param               aStreamUid     The UID of the stream to open for reading.
        */
        void OpenReadStreamLC(RReadStream& aReadStream, TUid aStreamUid);

        /**
        Opens the specified stream, or creates a new one if it does not exist, and places an item on the cleanup stack to close it.
        @param              aWriteStream    On return, an open write stream
        @param              aStreamUid      The UID of the stream to open or create
        */
        void OpenWriteStreamLC(RWriteStream& aWriteStream, TUid aStreamUid);

        /**
        Tests if the specified stream identified exists in the store.
        @param              aStreamUid The stream UID
        @return             Non-zero if the stream exists, otherwise EFalse.
        */
        TBool IsStreamPresentL(TUid aStreamUid) const;

    private: // from MVPbkContactStoreObserver
        
        void StoreReady(MVPbkContactStore& aContactStore);
        
        void StoreUnavailable(MVPbkContactStore& aContactStore,
                TInt aReason);
        
        void HandleStoreEventL(
                MVPbkContactStore& aContactStore,
                TVPbkContactStoreEvent aStoreEvent);

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
                    
        
    private: // helper

        CContactItemField* CreateConfidentialLC();


    private: // Data

        CNSmlDummyModsFetcher* iCNSmlDummyModsFetcher;
        CContactDatabase* iContactDatabase;  // Not owned
        CNSmlChangeFinder* iChangeFinder;
        TKeyArrayFix iKey;
        TBool iSyncHistory;
        TBool iSnapshotRegistered;
        HBufC8* iTestData;
        RDesReadStream iReadStream;

        CNSmlAdapterTestUtilities* iTestUtilities;
        TContactItemId iCardId;
        TTime iLastChanged;

        // VPbk related
        CVPbkContactManager* iContactManager;
        MVPbkContactStore* iStore;
        MVPbkContactViewBase* iContactViewBase;

        EUNIT_DECLARE_TEST_TABLE;

    };

#endif      //  __CNSMLDUMMYMODSFETCHER_TEST_H__
