/*
* Copyright (c) 2006 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:  DS contacts datastore.
*
*/


#ifndef __NSMLCONTACTSMODSFETCHER_H__
#define __NSMLCONTACTSMODSFETCHER_H__

// INCLUDES
#include <SmlDataProvider.h>
#include <SmlDataFormat.h>
#include <f32file.h>
#include <cntdb.h>
#include <cntitem.h>
#include "nsmlchangefinder.h"

#include <CVPbkContactManager.h>
#include <MVPbkContactStore.h>
#include <MVPbkContactViewObserver.h>
#include <CVPbkSortOrder.h>
#include <MVPbkContactViewBase.h>
#include <MVPbkSingleContactOperationObserver.h>
#include <MVPbkContactOperationBase.h>

// CONSTANTS
const TInt KNSmlSnapshotSize = 56;
const TInt KNSmlSnapshotSmallSize = 8;
_LIT( KNSmlContactSyncNoSync, "none" ); // Taken from phonebook

// CLASS DECLARATION

// ------------------------------------------------------------------------------------------------
// CNSmlContactsModsFetcher
//
// @lib nsmlcontactsdataprovider.lib
// ------------------------------------------------------------------------------------------------
class CNSmlContactsModsFetcher : public CActive,
								MVPbkContactViewObserver,
								MVPbkSingleContactOperationObserver,
								MVPbkContactOperationBase
								
	{
	public:
		/**
		* C++ constructor.
		* @param		aSnapshotRegistered	Is snapshot registered.
		* @param		aCntDb				Pointer to opened database.
		* @param		aKey				Key array.
		* @param		aChangeFinder		Pointer to Changefinder.
		*/
		CNSmlContactsModsFetcher( TBool& aSnapshotRegistered,CVPbkContactManager& aContactManager,MVPbkContactStore& aStore,TKeyArrayFix& aKey, CNSmlChangeFinder& aChangeFinder );
		/**
		* Second phase constructor.
		*/
		void ConstructL();

		/**
		* Reads all modifications from clients contacts databse.
		* @param		aStatus				On completion of the operation, contains the result code.
		*/
		void FetchModificationsL( TRequestStatus& aStatus );

		/**
		* ~CNSmlContactsModsFetcher() desctructor.
		*/
		virtual ~CNSmlContactsModsFetcher();
		
	private:
	
		/**
		* From MVPbkContactViewObserver  
		* Called when a view is ready for use. 
		*/

		void ContactViewReady(
		            MVPbkContactViewBase& aView ) ;
		/**
		* From MVPbkContactViewObserver  
		* Called when a view is unavailable for a while. 
		*/            
		void ContactViewUnavailable(
		            MVPbkContactViewBase& aView ) ;
		/**
		* From MVPbkContactViewObserver  
		* Called when a contact has been added to the view.
		*/            
		void ContactAddedToView(
		            MVPbkContactViewBase& aView, 
		            TInt aIndex, 
		            const MVPbkContactLink& aContactLink );
		/**
		* From MVPbkContactViewObserver  
		* Called when a contact has been removed from a view.
		*/            
		void ContactRemovedFromView(
		            MVPbkContactViewBase& aView, 
		            TInt aIndex, 
		            const MVPbkContactLink& aContactLink ) ;
		/**
		* From MVPbkContactViewObserver  
		* Called when an error occurs in the view.
		*/            
		void ContactViewError(
		            MVPbkContactViewBase& aView, 
		            TInt aError, 
		            TBool aErrorNotified ) ;
		            /**
		* From MVPbkSingleContactOperationObserver  
		* Called when the operation is completed.
		*/
         void VPbkSingleContactOperationComplete(MVPbkContactOperationBase& aOperation,MVPbkStoreContact* aContact ); 
         
		/**
		* From MVPbkSingleContactOperationObserver  
		*  Called if the operation fails.
		*/
 		void VPbkSingleContactOperationFailed(
 					MVPbkContactOperationBase& aOperation, 
 					TInt aError );

	private:
		/**
		* Fetches next chunk of data (client contacts data changes).
		*/
		void FetchNextContactL();

		/**
		* From CActive. Called when asynchronous operation completes.
		*/
		void RunL();
		
		/**
		* From CActive. Called when RunL leaves.
		*/    
		TInt RunError( TInt aError );

		/**
		* From CActive. Cancels operation.
		*/
		void DoCancel();
		
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

#endif // __NSMLCONTACTSMODSFETCHER_H__

// End of File  
