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

#ifndef CNSMLDUMMYDATASTORE_H
#define CNSMLDUMMYDATASTORE_H

//  EXTERNAL INCLUDES
#include <NSmlContactsDataStore.h>

// Device Info resource filenames
_LIT( KNSmlDummyContactsStoreFormatRsc_1_1_2, "z:NSmlDummyDataStoreFormat_1_1_2.rsc" );
_LIT( KNSmlDummyContactsStoreFormatRsc_1_2, "z:NSmlDummyDataStoreFormat_1_2.rsc" );

//  CLASS DEFINITION
/**
 * Datastore class for Dummy adapters
 * @lib nsmldummydataprovider.lib
 * @since 3.23
 */
class CNSmlDummyDataStore : public CNSmlContactsDataStore
    {
    private:    // Friend class definitions
        friend class CNSmlDummyDataStore_Test;
        friend class CNSmlDummyDataStoreWaiter;
        
    public:     // Constructors and destructor

        static CNSmlDummyDataStore* NewL();
        static CNSmlDummyDataStore* NewLC();
        ~CNSmlDummyDataStore();

    protected:    // Constructors

        CNSmlDummyDataStore();
        void ConstructL();

    protected:  // From CNSmlContactsDataStore

        void DoOpenL( const TDesC& aStoreName, MSmlSyncRelationship& aContext,
            TRequestStatus& aStatus );

        void DoBeginBatchL();

        void DoOpenItemL( TSmlDbItemUid aUid, TBool& aFieldChange,
            TInt& aSize, TSmlDbItemUid& aParent, TDes8& aMimeType,
            TDes8& aMimeVer, TRequestStatus& aStatus );

        void DoCommitItemL( TRequestStatus& aStatus );

        void DoDeleteItemL( TSmlDbItemUid aUid, TRequestStatus& aStatus );

        TBool DoHasSyncHistory() const;

        const MSmlDataItemUidSet& DoModifiedItems() const;

        void DoResetChangeInfoL( TRequestStatus& aStatus );

        void DoCommitChangeInfoL( TRequestStatus& aStatus,
            const MSmlDataItemUidSet& aItems );

        void DoCommitChangeInfoL( TRequestStatus& aStatus );

        void LdoFetchItemL( TSmlDbItemUid& aUid, CBufBase& aItem );

        void LdoAddItemL( const TDesC8& aItem, TInt aSize );

        void LdoAddItemsL( CBufBase*& aItems,TInt aSize );

        TInt ExecuteAddL();

        void ExecuteDeleteL();

        void ExecuteUpdateL();

        void SingleContactOperationCompleteL( MVPbkStoreContact* aContact );

        void StepComplete( MVPbkContactOperationBase& aOperation,
            TInt aStepSize );

        void OperationCompleteL();

        void DoDeleteAllContactsL();

        const TDesC& GetStoreFormatResourceFileL() const;

        MContactsModsFetcher* CreateModsFetcherL();

    protected:  // From MVPbkContactCopyObserver

        void ContactsSaved( MVPbkContactOperationBase& aOperation,
            MVPbkContactLinkArray* aResults );

        void ContactsSavingFailed( MVPbkContactOperationBase& aOperation,
            TInt aError );

    };

#endif      //  CNSMLDUMMYDATASTORE_H

// End of file
