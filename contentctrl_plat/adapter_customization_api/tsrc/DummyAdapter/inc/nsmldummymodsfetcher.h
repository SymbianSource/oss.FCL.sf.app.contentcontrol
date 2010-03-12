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
* Description:  DS Dummy Adapter ModsFetcher
*
*/

#ifndef __NSMLDUMMYMODSFETCHER_H__
#define __NSMLDUMMYMODSFETCHER_H__

// INCLUDES
#include <SmlDataProvider.h>
#include <SmlDataFormat.h>
#include <f32file.h>
#include <cntdb.h>
#include <cntitem.h>
#include <MContactsModsFetcher.h>

#include <cvpbkcontactmanager.h>
#include <mvpbkcontactstore.h>
#include <MVPbkContactViewObserver.h>
#include <CVPbkSortOrder.h>
#include <MVPbkContactViewBase.h>
#include <MVPbkSingleContactOperationObserver.h>
#include <MVPbkContactOperationBase.h>

// CONSTANTS
const TInt KNSmlSnapshotSize = 56;
const TInt KNSmlSnapshotSmallSize = 8;
const TInt KNSmlSnapshotCompressSize = 20;
_LIT( KNSmlContactSyncNoSync, "none" ); // Taken from phonebook

// CLASS DECLARATION
class CNSmlChangeFinder;
class TNSmlSnapshotItem;

// ------------------------------------------------------------------------------------------------
// CNSmlDummyModsFetcher
//
// @lib nsmlcontactsdataprovider.lib
// ------------------------------------------------------------------------------------------------
class CNSmlDummyModsFetcher : public CActive, public MContactsModsFetcher,
				              public MVPbkContactViewObserver,
						      public MVPbkSingleContactOperationObserver,
						      public MVPbkContactOperationBase

	{
	
    private:    // Friend class definitions
        friend class CNSmlDummyModsFetcher_Test;
	
    public:
		/**
		* C++ constructor.
		* @param		aSnapshotRegistered	    Is snapshot registered.
		* @param        aContactManager         Reference to ContactManager.
		* @param        aStore                  Reference to ContactStore.
		* @param        aKey                    Key array.
		* @param        aChangeFinder           Reference to Changefinder.
		*/
		CNSmlDummyModsFetcher( TBool& aSnapshotRegistered,CVPbkContactManager& aContactManager,MVPbkContactStore& aStore,TKeyArrayFix& aKey, CNSmlChangeFinder& aChangeFinder );
		/**
		* Second phase constructor.
		*/
        void ConstructL();

		/**
		* ~CNSmlDummyModsFetcher() destructor.
		*/
		virtual ~CNSmlDummyModsFetcher();

    private:   // From MVPbkContactViewObserver

		/**
		* Called when a view is ready for use.
		*/

		void ContactViewReady(
		            MVPbkContactViewBase& aView ) ;
		/**
		* Called when a view is unavailable for a while.
		*/
		void ContactViewUnavailable(
		            MVPbkContactViewBase& aView ) ;
		/**
		* Called when a contact has been added to the view.
		*/
		void ContactAddedToView(
		            MVPbkContactViewBase& aView,
		            TInt aIndex,
		            const MVPbkContactLink& aContactLink );
		/**
		* Called when a contact has been removed from a view.
		*/
		void ContactRemovedFromView(
		            MVPbkContactViewBase& aView,
		            TInt aIndex,
		            const MVPbkContactLink& aContactLink ) ;
		/**
		* Called when an error occurs in the view.
		*/
		void ContactViewError(
		            MVPbkContactViewBase& aView,
		            TInt aError,
		            TBool aErrorNotified ) ;
		            /**
		* Called when the operation is completed.
		*/
        void VPbkSingleContactOperationComplete(MVPbkContactOperationBase& aOperation,MVPbkStoreContact* aContact );

		/**
		*  Called if the operation fails.
		*/
 		void VPbkSingleContactOperationFailed(
 					MVPbkContactOperationBase& aOperation,
 					TInt aError );

    private:    //From CActive

		/**
		* Called when asynchronous operation completes.
		*/
		void RunL();

		/**
		* Called when RunL leaves.
		*/
		TInt RunError( TInt aError );

		/**
		*  Cancels operation.
		*/
		void DoCancel();

    private:   //new method
		/**
		* Fetches next chunk of data (client contacts data changes).
		*/
		void FetchNextContactL();
		/**
		* Checks if aItem is confidential or not.
		* @param    aItem   Item that is checked.
		* @return   TBool   ETrue if item is confidential.
		*                   Otherwise EFalse is returned.
		*/
        TBool IsConfidentialL( MVPbkStoreContact& aItem );
        /**
		* Create a contact view of the store which is opened
		*/
        void CreateViewL();
        /**
		* Add the fetched contacts id and timestamp
		* to the snapshot.
		* @param    aContact   Fetched contact to be
		*						added to snapshot
		*/
        void UpdateSnapshotL( MVPbkStoreContact* aContact );

    private: // from MContactsModsFetcher

        /**
        * Cancels the current asynchronous request
        */
        void CancelRequest( );

        /**
        * Reads all modifications from clients contacts databse.
        * @param    aStatus   On completion of the operation, contains the result code.
        */
        void FetchModificationsL( TRequestStatus& aStatus );

    private: // data
		TRequestStatus* iCallerStatus;
		TTime iFromTime;


		CArrayFixSeg<TNSmlSnapshotItem>* iSnapshot;

		TBool& iSnapshotRegistered;


		TKeyArrayFix& iKey;

		CNSmlChangeFinder& iChangeFinder;
		TInt iCurrentSnapshotCounter;

		CVPbkContactManager& iContactManager;
        MVPbkContactStore& iStore;
        MVPbkContactViewBase* iContactViewBase;
        TInt iContactCount;

	};

#endif // __NSMLDUMMYMODSFETCHER_H__

// End of File
