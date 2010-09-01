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


#ifndef __NSMLCONTACTSDATASTORE_H__
#define __NSMLCONTACTSDATASTORE_H__

// INCLUDE FILES
#include <SmlDataProvider.h>
#include <SmlDataFormat.h>
#include <f32file.h>

//vpbk headers
#include <CVPbkContactManager.h>
#include <MVPbkContactStore.h>
#include <MVPbkContactStoreObserver.h>
#include <MVPbkBatchOperationObserver.h>
#include <MVPbkContactViewObserver.h>
#include <CVPbkSortOrder.h>
#include <CVPbkContactIdConverter.h>
#include <CVPbkContactLinkArray.h>
#include <MVPbkContactViewBase.h>
#include <MVPbkSingleContactOperationObserver.h>
#include <CVPbkVCardEng.h>
#include <MVPbkContactCopyObserver.h>
#include <MVPbkBatchOperationObserver.h>
#include <MVPbkContactViewObserver.h>
#include <MVPbkContactOperationBase.h>
#include <CVPbkSortOrder.h>
#include <s32mem.h>

// MACROS
#define KNSmlvCard21Version TVersion(2,1,0);
#define KNSmlvCard30Version TVersion(3,0,0);

// CONSTANTS
_LIT(  KNSmlContactStoreNameForDefaultDB, "C:Contacts.cdb" );
_LIT(  KNSmlDriveC, "C" );
_LIT8( KNSmlvCard30Name, "text/vcard" );
_LIT8( KNSmlvCard30Ver, "3.0" );
_LIT8( KNSmlvCard21Name, "text/x-vcard" );
_LIT8( KNSmlvCard21Ver, "2.1" );

_LIT8( KVersitTokenHOME, "HOME"  );
_LIT8( KVersitTokenWORK, "WORK"  );
_LIT8( KVersitTokenCELL, "CELL"  );
_LIT8( KVersitTokenPAGER,"PAGER" );
_LIT8( KVersitTokenFAX,  "FAX"   );
_LIT8( KVersitTokenVOICE,"VOICE" );
_LIT8( KVersitTokenVIDEO,"VIDEO" );

//CONSTANTS INTRODUCED FOR HAVING BACKWARD COMPATIBLE PHONE DB SENT TO SERVER
//THE CHANGE WAS NEEDED AFTER ADAPTING TO VIRTUAL PHONEBOOK
_LIT( KLegacySymbianDatabase,"C:Contacts.cdb");
const TInt KOldSymbianDBLength = 20;

_LIT( KVPbhkSymbianDBPrefix,"cntdb://");
const TInt KVPbhkPrefixLength = 10;

//End NEW CONSTANTS
const TInt KNSmlContactsGranularity = 8;
const TInt KNSmlCompactAfterChanges = 16;
const TInt KNSmlDataStoreMaxSize = 102400;		// 100 k
const TInt KNSmlDefaultStoreNameMaxSize = 256;
const TInt KNSmlItemDataExpandSize = 1024;
const TInt KNSmlNoError = 1;

_LIT(KNSmlContactsStoreFormatRsc_1_1_2,"NSmlContactsDataStoreFormat_1_1_2.rsc");
_LIT(KNSmlContactsStoreFormatRsc_1_2,"NSmlContactsDataStoreFormat_1_2.rsc");

// FORWARD DECLARATIONS
class MContactsModsFetcher;
class CNSmlDataModBase;
class CNSmlChangeFinder;
class CNSmlDataItemUidSet;
	
// CLASS DECLARATION

// ------------------------------------------------------------------------------------------------
// CNSmlContactsDataStore
//
// @lib nsmlcontactsdataprovider.lib
// ------------------------------------------------------------------------------------------------
class CNSmlContactsDataStore : public CSmlDataStore,
    public MVPbkContactStoreObserver,
    public MVPbkSingleContactOperationObserver,
	public MVPbkContactCopyObserver,
	public MVPbkBatchOperationObserver,
	public MVPbkContactViewObserver,
	public MVPbkContactOperationBase
	{
	public:	
		/**
		* Two-phased constructor.
		*/
	    IMPORT_C static CNSmlContactsDataStore* NewL();

		/**
		* Destructor.
		*/
	    IMPORT_C virtual ~CNSmlContactsDataStore();

		/**
		* Default store name of client.
		* @return Default store name.
		*/
	    IMPORT_C const TDesC& DefaultStoreNameL() const;

		/**
		* Gets a list of all contacts databases on client.
		* @return List of databases.
		*/
	    IMPORT_C CDesCArray* DoListStoresLC();
		

	protected:
		/**
		* From MVPbkContactStoreListObserver  
		* Called when a contact store is ready to use.
		*/
	    IMPORT_C void StoreReady( MVPbkContactStore& aContactStore );
		 
		/**
		* From MVPbkContactStoreListObserver  
		* Called when a contact store becomes unavailable.
		*/
	    IMPORT_C void StoreUnavailable(MVPbkContactStore& aContactStore,TInt aReason );
	
		/**
		* From MVPbkContactStoreListObserver  
		*  Called when changes occur in the contact store.
		*/
	    IMPORT_C void HandleStoreEventL(MVPbkContactStore& aContactStore,TVPbkContactStoreEvent aStoreEvent );
		 
		/**
		* From MVPbkSingleContactOperationObserver  
		* Called when the operation is completed.
		*/
	    IMPORT_C void VPbkSingleContactOperationComplete(
         	MVPbkContactOperationBase& aOperation,
         	MVPbkStoreContact* aContact ); 
         
		/**
		* From MVPbkSingleContactOperationObserver  
		*  Called if the operation fails.
		*/
	    IMPORT_C void VPbkSingleContactOperationFailed(MVPbkContactOperationBase& aOperation, TInt aError );
 		
 		/**
		* From MVPbkContactCopyObserver  
		* Called when the contact has been successfully commited or
        * copied. Caller takes the ownership of results.
   		*/
	    IMPORT_C void ContactsSaved( MVPbkContactOperationBase& aOperation,
            MVPbkContactLinkArray* aResults ) ;
        
        /**
		* From MVPbkContactCopyObserver  
		*Called when there was en error while saving contact(s).
		*/
	    IMPORT_C void ContactsSavingFailed( 
                MVPbkContactOperationBase& aOperation, 
                TInt aError );
        // From MVPbkBatchOperationObserver
    
	    /**
	     * Called when one step of the operation is complete.
	     * @param aOperation Operation whose step has completed
	     * @param aStepSize Size of the performed step
	     */     
	    IMPORT_C void StepComplete( MVPbkContactOperationBase& aOperation, 
	        TInt aStepSize );
	    
	    /**
	     * Called when one step of the operation fails
	     * @param aOperation Operation whose step has failed
	     * @param aStepSize Size of the performed step
	     * @param aError Error that occured
	     * @return ETrue if the batch operation should continue, 
	     *               EFalse otherwise
	     */     
	    IMPORT_C TBool StepFailed(
	        MVPbkContactOperationBase& aOperation,
	        TInt aStepSize,
	        TInt aError );
	    
	    /**
	     * Called when operation is completed
	     * @param aOperation the completed operation
	     */    
	    IMPORT_C void OperationComplete( MVPbkContactOperationBase& aOperation );  
	    
	    /**
		* From MVPbkContactViewObserver  
		* Called when a view is ready for use. 
		*/

	    IMPORT_C void ContactViewReady(
		            MVPbkContactViewBase& aView ) ;
		/**
		* From MVPbkContactViewObserver  
		* Called when a view is unavailable for a while. 
		*/            
	    IMPORT_C void ContactViewUnavailable(
		            MVPbkContactViewBase& aView ) ;
		/**
		* From MVPbkContactViewObserver  
		* Called when a contact has been added to the view.
		*/            
	    IMPORT_C void ContactAddedToView(
		            MVPbkContactViewBase& aView, 
		            TInt aIndex, 
		            const MVPbkContactLink& aContactLink );
		/**
		* From MVPbkContactViewObserver  
		* Called when a contact has been removed from a view.
		*/            
	    IMPORT_C void ContactRemovedFromView(
		            MVPbkContactViewBase& aView, 
		            TInt aIndex, 
		            const MVPbkContactLink& aContactLink ) ;
		/**
		* From MVPbkContactViewObserver  
		* Called when an error occurs in the view.
		*/            
	    IMPORT_C void ContactViewError(
		            MVPbkContactViewBase& aView, 
		            TInt aError, 
		            TBool aErrorNotified ) ;
	
	protected:
		/**
		* 2nd phase constructor.
		*/
	    IMPORT_C void ConstructL();

	    /*
	     * 2nd phase constructor.
	     * @param aStorename Name of the contact database instance.
	     * @param aLegaceStore Legacy name of the contact database instance.
	     *  This store name is used in the communication with server.
	     */
	    IMPORT_C void ConstructL( const TDesC& aStoreName, const TDesC& aLegacyStore );
		/**
		* DoOpenL() opens the data store specified by aStoreName asynchronously.
		* @param		aStoreName			The name of the data store to open.
		* @param		aContext			Identifies the specific synchronisation relationship to use for the synchronisation.
		* @param		aStatus				On completion of the open, contains the result code.
		*/
	    IMPORT_C void DoOpenL(const TDesC& aStoreName, MSmlSyncRelationship& aContext, TRequestStatus& aStatus);

		/**
		* DoCancelRequest() cancels the current asynchronous request, including open. Only one asynchronous request may be outstanding at any one time.
		*/
	    IMPORT_C void DoCancelRequest();

		/**
		* DoStoreName() returns the name of the open data store.
		* @return The name of the currently opened data store.
		*/
	    IMPORT_C const TDesC& DoStoreName() const;

		/**
		* DoBeginTransactionL() starts the transaction mode. During this mode calls to CreateItemL, ReplaceItemL,
		* WriteItemL, CommitItemL, MoveItemL, DeleteItemL and SoftDeleteItemL will be part of this transaction.
		* Their RequestStatus must be completed, even if the change is not yet really executed in the Data Store.
		* If a RequestStatus is completed with an error code, the transaction has failed and a rollback must be
		* done. In this case RevertTransaction will be called.
		*/
	    IMPORT_C void DoBeginTransactionL();

		/**
		* DoCommitTransactionL() will be called at the end of a successful transaction. At this point in time the
		* operations within the transaction are applied to the Data Store in an atomic way. If all operations
		* succeed, the RequestStatus must be completed with KErrNone. If an operation fails, a rollback must be
		* done and the RequestStatus must be completed with an appropriate error code.
		*/
	    IMPORT_C void DoCommitTransactionL(TRequestStatus& aStatus);

		/**
		* DoRevertTransaction() will be called to abort an ongoing transaction. None of the operations already
		* submitted may be applied to the Data Store. The RequestStatus must be completed with KErrNone as a revert
		* cannot fail.
		*/
	    IMPORT_C void DoRevertTransaction(TRequestStatus& aStatus);

		/**
		* DoBeginBatchL() starts the batch mode. During this mode calls to CreateItemL, ReplaceItemL,
		* WriteItemL, CommitItemL, MoveItemL, DeleteItemL and SoftDeleteItemL will be part of this batch.
		* Their RequestStatus must be completed with KErrNone, which only signals acceptance of the operation
		* for batch processing.
		*/
	    IMPORT_C void DoBeginBatchL();

		/**
		* DoCommitBatchL() will be called at the end of the batch mode. This tells the Data Store to
		* process the batched operations (in the order they were submitted), and to append the error code
		* for each operation to aResultArray.
		* The error codes in aResultArray are only valid if the RequestStatus is completed with KErrNone.
		* If the RequestStatus is completed with an error code none of the operations in the batch mode
		* were applied to the Data Store.
		*/
	    IMPORT_C void DoCommitBatchL(RArray<TInt>& aResultArray, TRequestStatus& aStatus);

		/**
		* DoCancelBatch() will be called to abort an ongoing batch mode. None of the operations already
		* submitted may be applied to the Data Store.
		*/
	    IMPORT_C void DoCancelBatch();

		/**
		* DoSetRemoteStoreFormatL() sets the SyncML server Data Format - this may optionally be used by the Data 
		* Provider to filter out properties that the server does not support, and should be used to avoid deleting 
		* these properties in case the server sends a changed item to the Data Provider
		*/
	    IMPORT_C void DoSetRemoteStoreFormatL(const CSmlDataStoreFormat& aServerDataStoreFormat);

		/**
		* DoSetRemoteMaxObjectSize() sets the SyncML server maximum object size - this may optionally be used by the 
		* Data Provider to not send items to the server exceeding its maximum size. 0 means there is no limit.
		*/
	    IMPORT_C void DoSetRemoteMaxObjectSize(TInt aServerMaxObjectSize);

		/**
		* DoMaxObjectSize() gets the Data Store maximum object size which is reported to the SyncML server. 0 means 
		* there is no limit.
		* @return The maximum object size.
		*/
	    IMPORT_C TInt DoMaxObjectSize() const;

		/**
		* DoOpenItemL() opens the data item specified by aUid asynchronously for reading.
		* @param		aUid				Item UID which going to be read.
		* @param		aFieldChange		Accept field changes.
		* @param		aParent				Parent of the item.
		* @param		aSize				Size of the item data.
		* @param		aMimeType			MIME type of the item.
		* @param		aMimeVer			MIME version used on item.
		* @param		aStatus				On completion of the opening of item, contains the result code.
		*/
	    IMPORT_C void DoOpenItemL(TSmlDbItemUid aUid, TBool& aFieldChange, TInt& aSize, TSmlDbItemUid& aParent, TDes8& aMimeType, TDes8& aMimeVer, TRequestStatus& aStatus);

		/**
		* DoCreateItemL() sets the item properties and reference to aUid which will be created.
		* @param		aUid				Reference to item UID which going to be created.
		* @param		aSize				Size of the item to be created.
		* @param		aParent				Parent of the item.
		* @param		aMimeType			MIME type of the item.
		* @param		aMimeVer			MIME version used on item.
		* @param		aStatus				On completion of the creating an item, contains the result code.
		*/
	    IMPORT_C void DoCreateItemL(TSmlDbItemUid& aUid, TInt aSize, TSmlDbItemUid aParent, const TDesC8& aMimeType, const TDesC8& aMimeVer, TRequestStatus& aStatus);

		/**
		* DoReplaceItemL() opens the data item specified by aUid asynchronously to be updated.
		* @param		aUid				Item UID which going to be updated.
		* @param		aSize				Size of the item data.
		* @param		aParent				Parent of the item.
		* @param		aFieldChange		Accept field changes.
		* @param		aStatus				On completion of the updating of item, contains the result code.
		*/
	    IMPORT_C void DoReplaceItemL(TSmlDbItemUid aUid, TInt aSize, TSmlDbItemUid aParent, TBool aFieldChange, TRequestStatus& aStatus);

		/**
		* DoReadItemL() reads data(or size of aBuffer) of an item opened in DoOpenItemL() to given aBuffer.
		* @param		aBuffer				Buffer to item data.
		*/
	    IMPORT_C void DoReadItemL(TDes8& aBuffer);

		/**
		* DoWriteItemL() writes aData of an item opened in DoCreateItemL() or DoReplaceItemL() to be saved on database.
		* @param		aData				Item data (or part of data).
		*/
	    IMPORT_C void DoWriteItemL(const TDesC8& aData);

		/**
		* DoCommitItemL() completes an item operation started in DoCreateItemL() or DoReplaceItemL().
		* @param		aStatus				On completion of the operation, contains the result code.
		*/
	    IMPORT_C void DoCommitItemL(TRequestStatus& aStatus);

		/**
		* DoCloseItem() completes an item operation started in DoOpenItemL().
		*/
	    IMPORT_C void DoCloseItem();

		/**
		* DoMoveItemL() moves item specified by aUid asynchronously.
		* @param		aUid				Item UID which going to be moved.
		* @param		aNewParent			A new parent of the item.
		* @param		aStatus				On completion of the moving an item, contains the result code.
		*/
	    IMPORT_C void DoMoveItemL(TSmlDbItemUid aUid, TSmlDbItemUid aNewParent, TRequestStatus& aStatus);

		/**
		* DoDeleteItemL() deletes item specified by aUid asynchronously.
		* @param		aUid				Item UID which going to be deleted.
		* @param		aStatus				On completion of the deleting an item, contains the result code.
		*/
	    IMPORT_C void DoDeleteItemL(TSmlDbItemUid aUid, TRequestStatus& aStatus);

		/**
		* DoSoftDeleteItemL() soft deletes item specified by aUid asynchronously.
		* @param		aUid				Item UID which going to be softdeleted.
		* @param		aStatus				On completion of the softdeleting an item, contains the result code.
		*/
	    IMPORT_C void DoSoftDeleteItemL(TSmlDbItemUid aUid, TRequestStatus& aStatus);

		/**
		* DoDeleteAllItemsL() deletes all items from opened database asynchronously.
		* @param		aStatus				On completion of delete, contains the result code.
		*/
	    IMPORT_C void DoDeleteAllItemsL(TRequestStatus& aStatus);

		/**
		* DoHasSyncHistory() checks if previous sync with opened server and context.
		* @return ETrue if there is synchonization history.
		*/
	    IMPORT_C TBool DoHasSyncHistory() const;

		/**
		* DoAddedItems() gets all added items on client since previous synchronization.
		* @return Added items.
		*/
	    IMPORT_C const MSmlDataItemUidSet& DoAddedItems() const;

		/**
		* DoDeletedItems() gets all deleted items on client since previous synchronization.
		* @return Deleted items.
		*/
	    IMPORT_C const MSmlDataItemUidSet& DoDeletedItems() const;

		/**
		* DoSoftDeletedItems() gets all softdeleted items on client since previous synchronization.
		* @return Soft deleted items.
		*/
	    IMPORT_C const MSmlDataItemUidSet& DoSoftDeletedItems() const;

		/**
		* DoModifiedItems() gets all modified items on client since previous synchronization.
		* @return Modified items.
		*/
	    IMPORT_C const MSmlDataItemUidSet& DoModifiedItems() const;

		/**
		* DoMovedItems() gets all moved items on client since previous synchronization.
		* @return Moved items.
		*/
	    IMPORT_C const MSmlDataItemUidSet& DoMovedItems() const;

		/**
		* DoResetChangeInfoL() resets client synchronization data => next time will be slow sync.
		* @param		aStatus				On completion of reset, contains the result code.
		*/
	    IMPORT_C void DoResetChangeInfoL(TRequestStatus& aStatus);

		/**
		* DoCommitChangeInfoL() commits client synchronization changes for given aItems list.
		* @param		aStatus				On completion of given items, contains the result code.
		* @param		aItems				Item ids to be commited.
		*/
	    IMPORT_C void DoCommitChangeInfoL(TRequestStatus& aStatus, const MSmlDataItemUidSet& aItems);

		/**
		* DoCommitChangeInfoL() commits all client synchronization changes.
		* @param		aStatus				On completion of all items, contains the result code.
		*/
	    IMPORT_C void DoCommitChangeInfoL(TRequestStatus& aStatus);

		/**
		* Default constructor.
		*/
	    IMPORT_C CNSmlContactsDataStore();

		/**
		* SetOwnStoreFormatL() Sets dataproviders own storeformat.
		*/
	    IMPORT_C void SetOwnStoreFormatL();

		/**
		* LdoFetchItemL() Fetches item data from database.
		* @param		aUid				Items uid for fetching.
		* @param		aItem				Items data after fetch.
		* 
		*/
	    IMPORT_C virtual void LdoFetchItemL( TSmlDbItemUid& aUid, CBufBase& aItem );

		/*
		* LdoAddItemL() Adds item data to database.
		* @param		aItem				Item data to be added.
		* @param		aSize				Item data size.
		*/
	    IMPORT_C virtual void LdoAddItemL( const TDesC8& aItem,
		                  TInt aSize);
		/*
		* LdoAddItemL() Adds several items to database.
		* @param		aItem				Items data to be added.
		* @param		aSize				Items data size.
		*/
	    IMPORT_C virtual void LdoAddItemsL( CBufBase*& aItems,TInt aSize);
		
		/**
		* LdoUpdateItemL() Updates item data to database.
		* 
		*/
	    IMPORT_C virtual void LdoUpdateItemL();
		
		/**
		* DriveBelowCriticalLevelL() Checks if there is enough space on client to store added item data.
		* @param		aSize				Item size to be added.
		* @return ETrue if there isn't enough drive space.
		*/
	    IMPORT_C TBool DriveBelowCriticalLevelL( TInt aSize );


		/**
		* StripPropertyL() Removes aProperty from aItem data.
		* @param		aItem				Item data to be stripped.
		* @param		aProperty			Property to be removed from aItem.
		*/
	    IMPORT_C void StripPropertyL( HBufC8*& aItem, const TDesC8& aProperty ) const;

		/**
		* StripPropertyL() Remove aPropertys from aItem data.
		* @param		aItem				Item(s) data to be stripped.
		* @param		aProperty			Property to be removed from aItem.
		*/
	    IMPORT_C void StripPropertyL( CBufBase*& aItem, const TDesC8& aProperty ) const;

		/**
		* ExecuteBufferL() Executes all buffered items from buffer.
		* @param		aResultArray		Array to return statuscodes for each command.
		*/
	    IMPORT_C virtual void ExecuteBufferL();
		
		/**
		* ExecuteAddL() Executes all the add commands in buffer.
		*/
	    IMPORT_C virtual TInt ExecuteAddL();
		
		/**
		* ExecuteDeleteL() Executes all the delete commands in buffer.
		*/
	    IMPORT_C virtual void ExecuteDeleteL();
		
		/**
		* ExecuteBufferL() Executes all the update commands in buffer.
		*/
	    IMPORT_C virtual void ExecuteUpdateL();
		
		/**
		* ExecuteMoveL() Executes all the move commands in buffer.
		*/
	    IMPORT_C virtual void ExecuteMoveL();
	
		/**
		* AddBufferListL()Adds a new item to buffer.
		* @param 		aUid 				New item's uid.
		* @param 		aSize 				New item's size.
		* @param 		aStatus 			New item's status.
		* @return Pointer to the buffer.
		*/
	    IMPORT_C CBufBase* AddBufferListL(TSmlDbItemUid& aUid, TInt aSize, TInt aStatus);
		
		/**
		* Checks if aItem is confidential or not.
		* @param    aItem   Item that is checked.
		* @return   TBool   ETrue if item is confidential.
		*                   Otherwise EFalse is returned.
		*/
	    IMPORT_C TBool IsConfidentialL( MVPbkStoreContact& aItem );
        
        /**
		* Leaving function called when fetch/retrieve 
		* operation is complete
		* @param    aContact   contact fetched/retrieved
		* 
		*/
	    IMPORT_C void SingleContactOperationCompleteL(MVPbkStoreContact* aContact);
        
        /**
		* Create a contact view of the store which is opened
		*/
	    IMPORT_C void CreateViewL();
        
        /**
		* Delete all the contacts
		*/
	    IMPORT_C virtual void DoDeleteAllContactsL();
        /**
		* Leaving function called when the operation is complete
		*/
	    IMPORT_C void OperationCompleteL();
        /**
		* Reset the contacts data buffer
		*/
	    IMPORT_C void ResetBuffer();

    protected: // New

        /**
         * Returns the file name for the store format resources.
         * @return Store format resource file name. 
         */
        IMPORT_C virtual const TDesC& GetStoreFormatResourceFileL();

        /**
         * Creates the ModsFetcher object for the DataStore.
         * @return ContactsModsFetcher object. 
         */
        IMPORT_C virtual MContactsModsFetcher* CreateModsFetcherL();

        /**
         * Get datamod instance
         * @return reference to datamod instance.
         */
        IMPORT_C virtual CNSmlDataModBase& GetDataMod();
        
	protected: // data
	
			// MODULE DATA STRUCTURES
		enum TNSmlDataStoreStatus  // DataStore status
			{
			ENSmlClosed = 1,
			ENSmlOpenAndWaiting,
			ENSmlItemOverflow,
			ENSmlItemOpen,
			ENSmlItemCreating,
			ENSmlItemUpdating
			};

		enum TNSmlCntCommand		// Modification type
			{
			ENSmlCntItemAdd = 1,
			ENSmlCntItemDelete,
			ENSmlCntItemSoftDelete,
			ENSmlCntItemRead,
			ENSmlCntItemMove,
			ENSmlCntItemReplace,
			ENSmlCntItemFieldLevelReplace
			};
			
			enum TNSmlLastOp
			{
			ENSmlRetrieveOp = 1,
			ENSmlAddOp,
			ENSmlDeleteOp,
			ENSMLFetchOp,
			ENSMLDeleteAllOp,
			ENSMLUpdateExportOp,
			ENSMLUpdateImportOp,
			ENSmlNone
			};
	

		// ------------------------------------------------------------------------------------------------
		// Buffering Stuff for BatchMode operations
		// ------------------------------------------------------------------------------------------------
		class CNSmlContactsBufferItem : public CBase
			{
			public:	
				/**
				* Destructor.
				*/
				~CNSmlContactsBufferItem();
				
			public: // data
				TSmlDbItemUid	*iPUid;			// New item ID 
				TSmlDbItemUid	iUid;			// Item ID
				CBufBase		*iItemData;		// Item data
				HBufC8 			*iMimeType;		// Mimetype
				HBufC8 			*iMimeVersion;	// Mime version
				TNSmlCntCommand iModType;		// Commands type
				TInt			iStatus;		// Command status
				TInt 			iSize;			// Item size
			};
		
	protected:
        CNSmlChangeFinder* iChangeFinder;
        TRequestStatus* iCallerStatus;
        TBool iSnapshotRegistered;
	    TBool iBatchMode;
        TBool iOpened;
	    TInt64 iOpenedStoreId;
        TInt iServerMaxObjectSize;
        TSmlDbItemUid iUid;
        TInt iItemPos;
        TNSmlCntCommand iModType;
        CBufBase* iItemData;
        CBufBase* iItemDataAddBatch;
        CBufFlat* iMergeItem;
        RPointerArray<CNSmlContactsBufferItem> iContactsBufferItemList;
        TKeyArrayFix iKey;
        TBool iSyncHistory;
        CSmlDataStoreFormat* iStoreFormat;
        TNSmlDataStoreStatus iState;

        CVPbkContactManager* iContactManager;
        MVPbkContactStore* iStore;
        CVPbkContactIdConverter* iIdConverter;
        MVPbkContactViewBase* iContactViewBase;
        
        TInt *iSize;
        RBufWriteStream iWriteStream;
        RDesReadStream iReadStream;
        HBufC8* iBuf;
        TInt  iRetCommand;
        TInt iIndex;
        TInt iRetrieveCount;
        RArray<TInt> *iResultArray;
        TNSmlLastOp iLastOperation;
        
        TInt iStateItem;

	    TBool iFieldLevelReplace;

	private:
		TBool iTransactionMode;
		TInt iModificationCount;
		TPtrC8 iMimeTypeItem;
		TPtrC8 iMimeVersionItem;
		TPtrC8 iUsedMimeType;
		TPtrC8 iUsedMimeVersion;
		RStringF iServerMimeType;
		RStringF iServerMimeVersion;	
		HBufC* iDefaultStoreName;
				
		//The symbian Db name that is used for communication with servers
		//Changes done to mantain backward compatibility of function
		HBufC* iPacketStoreName;

		CNSmlDataModBase* iDataMod;
		MContactsModsFetcher* iContactsModsFetcher; 

		RFs iRfs;
		TInt iDrive;	
		TInt iItemSize;

		HBufC* iStoreName;
		
		RStringPool iStringPool;
		
		CNSmlDataItemUidSet* iNewUids;
		CNSmlDataItemUidSet* iDeletedUids;
		CNSmlDataItemUidSet* iSoftDeletedUids;
		CNSmlDataItemUidSet* iReplacedUids;
		CNSmlDataItemUidSet* iMovedUids;
		
		RArray<TInt> iAddResultArray; 

        CVPbkVCardEng* iVCardEngine ;
        MVPbkContactLink* iContactLink;
        CVPbkContactLinkArray* iContactLnks;
		MVPbkContactOperationBase* iDeleteAllOperation;
	};


#endif // __NSMLCONTACTSDATASTORE_H__

// End of File  
