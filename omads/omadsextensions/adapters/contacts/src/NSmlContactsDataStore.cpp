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
* Description:  DS contacts dataprovider
*
*/


// INCLUDE FILES
#include <barsc.h>
#include <bautils.h>
#include <e32base.h>
#include <s32strm.h>
#include <e32cmn.h>
#include <e32des16.h>
#include <s32mem.h>
#include <implementationproxy.h>
#include <vtoken.h>
#include <sysutil.h>
#include <NSmlContactsDataStoreFormat_1_1_2.rsg>
#include <NSmlContactsDataStoreFormat_1_2.rsg>
#include <data_caging_path_literals.hrh>
#include <SmlDataFormat.h>
#include <SmlDataProvider.h>
#include <e32property.h>
#include <DataSyncInternalPSKeys.h>
#include <e32hashtab.h>

#include <CVPbkContactStoreUriArray.h>
#include <VPbkContactStoreUris.h>
#include <TVPbkContactStoreUriPtr.h>
#include <MVPbkContactStoreList.h>
#include <MVPbkContactStoreProperties.h>
#include <MVPbkStoreContactProperties.h>
#include <MVPbkViewContact.h>
#include <MVPbkStoreContact.h>
#include <MVPbkStoreContact2.h>
#include <MVPbkFieldType.h>
#include <MVPbkContactFieldTextData.h>
#include <CVPbkContactFieldIterator.h>
#include <CVPbkContactViewDefinition.h>

#include <MVPbkContactLink.h>
#include <s32mem.h>

#include <NSmlContactsDataStore.h>
#include "NSmlContactsModsFetcher.h"
#include "nsmldebug.h"
#include "nsmlconstants.h"
#include <NSmlDataModBase.h>
#include "nsmlcontactsdefs.h"
#include "nsmldsimpluids.h"
#include "nsmlsnapshotitem.h"
#include "nsmlchangefinder.h"
#include <VPbkEng.rsg>

#ifndef __WINS__
// This lowers the unnecessary compiler warning (armv5) to remark.
// "Warning:  #174-D: expression has no effect..." is caused by 
// DBG_ARGS8 macro in no-debug builds.
#pragma diag_remark 174
#endif


// ----------------------------------------------------------------------------
// CNSmlContactsDataStore::CNSmlContactsBufferItem::~CNSmlContactsBufferItem
// ----------------------------------------------------------------------------
EXPORT_C CNSmlContactsDataStore::CNSmlContactsBufferItem::~CNSmlContactsBufferItem()
	{
	delete iItemData;
	delete iMimeType;
	delete iMimeVersion;
	}
	
// ----------------------------------------------------------------------------
// CNSmlContactsDataStore::NewL
// ----------------------------------------------------------------------------
EXPORT_C CNSmlContactsDataStore* CNSmlContactsDataStore::NewL()
	{
	_DBG_FILE("CNSmlContactsDataStore::NewL: BEGIN");
	CNSmlContactsDataStore* self = new ( ELeave ) CNSmlContactsDataStore();
	CleanupStack::PushL(self);
		
	self->ConstructL();
	CleanupStack::Pop(self); // self
		
	_DBG_FILE("CNSmlContactsDataStore::NewL: END");
	return self;
	}

// ----------------------------------------------------------------------------
// CNSmlContactsDataStore::CNSmlContactsDataStore() 
// ----------------------------------------------------------------------------
EXPORT_C CNSmlContactsDataStore::CNSmlContactsDataStore() : 
	iKey( TKeyArrayFix( _FOFF( TNSmlSnapshotItem,ItemId() ),ECmpTInt )),
	iContactManager(NULL),
	iStore(NULL),
	iIdConverter(NULL),
	iContactViewBase(NULL),
	iSize(NULL),	
	iBuf(NULL),
	iVCardEngine(NULL),
	iContactLnks(NULL)
	{
	_DBG_FILE("CNSmlContactsDataStore::CNSmlContactsDataStore(): begin");

	iSyncHistory = EFalse;
	iSnapshotRegistered = EFalse;

	iServerMaxObjectSize = 0; // Maximum size default value 
	iItemPos = -1;
	iModificationCount = KNSmlCompactAfterChanges;
	
	iState = ENSmlClosed;
	iStoreName = NULL;
	iRetCommand = KErrNone;
	iLastOperation = ENSmlNone;
	if(iDeleteAllOperation)
	    {
	    delete iDeleteAllOperation;
	    iDeleteAllOperation = NULL;
	    }
	_DBG_FILE("CNSmlContactsDataStore::CNSmlContactsDataStore(): end");
	}
	
// ----------------------------------------------------------------------------
// CNSmlContactsDataStore::ConstructL()
// ----------------------------------------------------------------------------
EXPORT_C void CNSmlContactsDataStore::ConstructL()
	{
	_DBG_FILE("CNSmlContactsDataStore::ConstructL(): begin");
	
	ConstructL( ( VPbkContactStoreUris::DefaultCntDbUri() ), KLegacySymbianDatabase );
		
	_DBG_FILE("CNSmlContactsDataStore::ConstructL(): end");
	}

// ----------------------------------------------------------------------------
// CNSmlContactsDataStore::ConstructL()
// ----------------------------------------------------------------------------
EXPORT_C void CNSmlContactsDataStore::ConstructL(
    const TDesC& aStoreName, const TDesC& aLegacyStore )
    {
    _DBG_FILE("CNSmlContactsDataStore::ConstructL(): begin");
    
    User::LeaveIfError( iRfs.Connect() );

    iStringPool.OpenL();
    
    // Uid Sets
    iNewUids = new (ELeave) CNSmlDataItemUidSet();
    iDeletedUids = new (ELeave) CNSmlDataItemUidSet();
    iSoftDeletedUids = new (ELeave) CNSmlDataItemUidSet();
    iMovedUids = new (ELeave) CNSmlDataItemUidSet();
    iReplacedUids = new (ELeave) CNSmlDataItemUidSet();

    // Create iDataMod
    // This should be done after OpenL if there are more than one database
    iDataMod = new (ELeave) CNSmlVCardMod();

    SetOwnStoreFormatL();
    // open  contact database
    CVPbkContactStoreUriArray* uriArray = CVPbkContactStoreUriArray::NewLC();
    uriArray->AppendL( TVPbkContactStoreUriPtr( aStoreName ));
    
    iContactManager = CVPbkContactManager::NewL(*uriArray);
    CleanupStack::PopAndDestroy(uriArray);
    //Default store name
    iDefaultStoreName = aStoreName.AllocL();
    
    //Fill iPacketStoreName buffer with legacy store name
    TBuf<KOldSymbianDBLength> dataBase(aLegacyStore);
    iPacketStoreName = HBufC::NewL(dataBase.Length());
    TPtr pktStorePtr(iPacketStoreName->Des());
    pktStorePtr.Copy(dataBase);
    iDeleteAllOperation = NULL;
        
    _DBG_FILE("CNSmlContactsDataStore::ConstructL(): end");    
    }

// ----------------------------------------------------------------------------
// CNSmlContactsDataStore::~CNSmlContactsDataStore()
// ----------------------------------------------------------------------------
EXPORT_C CNSmlContactsDataStore::~CNSmlContactsDataStore()
	{
	_DBG_FILE("CNSmlContactsDataStore::~CNSmlContactsDataStore(): begin");
	
	delete iStoreFormat; // SetOwnStoreFormatL creates
	iStringPool.Close();

	iRfs.Close();
	iWriteStream.Close();
	iReadStream.Close();
	delete iBuf;
	
	if ( iChangeFinder )
		{
		TRAP_IGNORE( iChangeFinder->CloseL() );
		delete iChangeFinder;
		iChangeFinder = NULL;
		}
	
	if(iIdConverter)
		{
		delete iIdConverter;
		iIdConverter = NULL;
		}
	if(iContactLnks)
		{
        delete iContactLnks;
		iContactLnks = NULL;
		}

	if(iContactViewBase)
	    {
	    delete iContactViewBase;
	    iContactViewBase = NULL;
	    }
	if ( iContactsModsFetcher )
		{
		iContactsModsFetcher->CancelRequest(); 
		delete iContactsModsFetcher;
		iContactsModsFetcher = NULL;
		}
	if ( iContactManager )
		{
			if(iStore)
			{
			iStore->Close( *this );	
			}
	    delete iContactManager;
	    iContactManager = NULL;
		}
	
	
	delete iVCardEngine;
	//  Delete UidSets		
	//
	delete iNewUids;
	delete iDeletedUids;
	delete iSoftDeletedUids;
	delete iMovedUids;
	delete iReplacedUids;
	
	//
	//	Empty BufferItemList if any
	//
	if ( iContactsBufferItemList.Count() > 0 )
		{
		if ( iContactsBufferItemList[iContactsBufferItemList.Count() - 1]->iItemData == iItemData )
			{
			iItemData = NULL;
			}
		}
	iContactsBufferItemList.ResetAndDestroy();
	delete iDataMod;
	delete iMergeItem;

	if(iItemData)
    	{
	    if(iItemData->Size() > 0 )
	        {
		    delete iItemData;       
	        }
	    }
	
	if(iItemDataAddBatch != iItemData)
		{
		delete iItemDataAddBatch;	
		}
		
	delete iDefaultStoreName;
	delete iPacketStoreName;

	delete iStoreName;

	iAddResultArray.Close();
	
	if(iDeleteAllOperation)
	    {
	    delete iDeleteAllOperation;
	    iDeleteAllOperation = NULL;
	    }
	
	}

// ----------------------------------------------------------------------------
// void CNSmlContactsDataStore::DoOpenL(const TDesC& 		  aStoreName, 
//									    MSmlSyncRelationship& aContext,
//									    TRequestStatus& 	  aStatus)
// ----------------------------------------------------------------------------
EXPORT_C void CNSmlContactsDataStore::DoOpenL( const TDesC& aStoreName, 
									MSmlSyncRelationship& aContext, 
									TRequestStatus& aStatus )
	{
	DBG_ARGS(_S("CNSmlContactsDataStore::DoOpenL(): '%S' begin"), &aStoreName);

	iCallerStatus = &aStatus;
	*iCallerStatus = KRequestPending;
	
	if( iState != ENSmlClosed )
		{
		User::RequestComplete( iCallerStatus, KErrInUse );	
		return;	
		}

	if( RFs::CharToDrive(aStoreName[0], iDrive) != KErrNone )
		{
		RFs::CharToDrive( KNSmlDriveC()[0], iDrive );
		}
	iOpened = EFalse;
	
	if( iStoreName )
		{
		delete iStoreName;
		iStoreName = NULL;
		}
		
		if ( iChangeFinder )
		{
		iChangeFinder->CloseL();
		delete iChangeFinder;
		iChangeFinder = NULL;
		}
	// The Change finder maintains a list of all changes to the database.
	iChangeFinder = CNSmlChangeFinder::NewL(
					 aContext, iKey, iSyncHistory, KNSmlContactsAdapterImplUid );
	
	iState = ENSmlOpenAndWaiting;			
	
	//find and open the store
	if ( aStoreName == KNSmlContactStoreNameForDefaultDB )
	    {
	iStore = iContactManager->ContactStoresL().Find(
	TVPbkContactStoreUriPtr(*iDefaultStoreName));
	    }
	else
	   {
	    iStore = iContactManager->ContactStoresL().Find(
	TVPbkContactStoreUriPtr(aStoreName));
	    }
	iStore->OpenL(*this);

	iStoreName = aStoreName.AllocL();
	
	iVCardEngine = CVPbkVCardEng::NewL( *iContactManager );
	iIdConverter = CVPbkContactIdConverter::NewL(*iStore );
	if ( iContactsModsFetcher )
		{
		iContactsModsFetcher->CancelRequest();
		delete iContactsModsFetcher;
		iContactsModsFetcher = NULL;
		}
		
	iContactsModsFetcher = CreateModsFetcherL();
	
#ifdef __NSML_MODULETEST__
	CActiveScheduler::Start();
#endif
	
	_DBG_FILE("CNSmlContactsDataStore::DoOpenL(): end");
	}

// ----------------------------------------------------------------------------
// void CNSmlContactsDataStore::DoCancelRequest()
// ----------------------------------------------------------------------------
EXPORT_C void CNSmlContactsDataStore::DoCancelRequest()
	{
	_DBG_FILE("CNSmlContactsDataStore::DoCancelRequest(): begin");
	if(iLastOperation == ENSMLDeleteAllOp)
	    {   
	    if(iDeleteAllOperation)
	        {
	        delete iDeleteAllOperation;
	        iDeleteAllOperation = NULL;
	        }

	    if( iChangeFinder )
	        {
            TRAP_IGNORE(iChangeFinder->ResetL());
	        }
	    iSnapshotRegistered = EFalse;

	    if(iContactLnks)
	        {
	        delete iContactLnks;
	        iContactLnks = NULL;
			iContactLink = NULL;
	        }

	    if(iContactViewBase)
	        {
	        delete iContactViewBase;
	        iContactViewBase = NULL;
	        }
	    User::RequestComplete( iCallerStatus, KErrCancel );  
	    }
		if(iContactsModsFetcher)
		{
		iContactsModsFetcher->CancelRequest();
		}
	
	_DBG_FILE("CNSmlContactsDataStore::DoCancelRequest(): NOT NEEDED end");
	}

// ----------------------------------------------------------------------------
// void CNSmlContactsDataStore::DoStoreName()
// ----------------------------------------------------------------------------
EXPORT_C const TDesC& CNSmlContactsDataStore::DoStoreName() const
	{
	_DBG_FILE("CNSmlContactsDataStore::DoStoreName(): begin");
	_DBG_FILE("CNSmlContactsDataStore::DoStoreName(): end");
	return *iStoreName;
	}

// ----------------------------------------------------------------------------
// void CNSmlContactsDataStore::DefaultStoreNameL()
// ----------------------------------------------------------------------------
EXPORT_C const TDesC& CNSmlContactsDataStore::DefaultStoreNameL() const
	{
	_DBG_FILE("CNSmlContactsDataStore::DefaultStoreNameL(): begin");
	
	if ( !iDefaultStoreName )
		{
		return KNullDesC;
		}

	_DBG_FILE("CNSmlContactsDataStore::DefaultStoreNameL(): end");
	//The engine methods that create syncml packet, use this information in Database fields
	return *iPacketStoreName;
	}
	
// ----------------------------------------------------------------------------
// void CNSmlContactsDataStore::DoBeginTransactionL()
// ----------------------------------------------------------------------------
EXPORT_C void CNSmlContactsDataStore::DoBeginTransactionL()
	{
	_DBG_FILE("CNSmlContactsDataStore::DoBeginTransactionL(): begin");
	User::Leave( KErrNotSupported );
	_DBG_FILE("CNSmlContactsDataStore::DoBeginTransactionL(): end");
	}

// ----------------------------------------------------------------------------
// void CNSmlContactsDataStore::DoCommitTransactionL()
// ----------------------------------------------------------------------------
EXPORT_C void CNSmlContactsDataStore::DoCommitTransactionL(TRequestStatus& aStatus)
	{
	_DBG_FILE("CNSmlContactsDataStore::DoCommitTransactionL(): begin");
	
	iCallerStatus = &aStatus;
	*iCallerStatus = KRequestPending;
	User::RequestComplete( iCallerStatus, KErrNotSupported );
	
	_DBG_FILE("CNSmlContactsDataStore::DoCommitTransactionL(): end");
	}

// ----------------------------------------------------------------------------
// void CNSmlContactsDataStore::DoRevertTransaction()
// ----------------------------------------------------------------------------
EXPORT_C void CNSmlContactsDataStore::DoRevertTransaction(TRequestStatus& aStatus)
	{
	_DBG_FILE("CNSmlContactsDataStore::DoRevertTransaction(): begin");
	
	iCallerStatus = &aStatus;
	*iCallerStatus = KRequestPending;
	User::RequestComplete( iCallerStatus, KErrNotSupported );

	_DBG_FILE("CNSmlContactsDataStore::DoRevertTransaction(): end");
	}

// ----------------------------------------------------------------------------
// void CNSmlContactsDataStore::DoBeginBatchL()
// ----------------------------------------------------------------------------
EXPORT_C void CNSmlContactsDataStore::DoBeginBatchL()
	{
	_DBG_FILE("CNSmlContactsDataStore::DoBeginBatchL(): begin");
	
	iBatchMode = ETrue;			// SUPPORTED on Contacts

	_DBG_FILE("CNSmlContactsDataStore::DoBeginBatchL(): end");
	}

// ----------------------------------------------------------------------------
// void CNSmlContactsDataStore::DoCommitBatchL()
// ----------------------------------------------------------------------------
EXPORT_C void CNSmlContactsDataStore::DoCommitBatchL( RArray<TInt>& aResultArray, TRequestStatus& aStatus )
	{
	_DBG_FILE("CNSmlContactsDataStore::DoCommitBatchL(): begin");

	iCallerStatus = &aStatus;
	*iCallerStatus = KRequestPending;
	
	if( !iBatchMode )
		{
		User::RequestComplete( iCallerStatus, KErrNotReady );
		_DBG_FILE("CNSmlContactsDataStore::DoCommitBatchL(): NOT ON BATCH MODE end");
		return;
		}
//		
// Go through Buffer and execute all commands return aResultArray
//
	
	iResultArray = &aResultArray;
	
	iIndex = 0;
	TRAPD( leavecode,ExecuteBufferL() );
	
	if( leavecode != KErrNone )
		{
		User::RequestComplete( iCallerStatus, leavecode );
		}

	_DBG_FILE("CNSmlContactsDataStore::DoCommitBatchL(): end");
	}
	
// ----------------------------------------------------------------------------
// void CNSmlContactsDataStore::ExecuteBufferL()
// ----------------------------------------------------------------------------
EXPORT_C void CNSmlContactsDataStore::ExecuteBufferL() 
	{
	_DBG_FILE("CNSmlContactsDataStore::ExecuteBufferL(): begin");
	
	TInt  retCommand( KErrNone );
	
	//
	// Loop through buffered commands
	//
		if( iIndex < iContactsBufferItemList.Count())
		{
		
		if( iContactsBufferItemList[iIndex]->iStatus!= KNSmlNoError )
			{
			retCommand = iContactsBufferItemList[iIndex]->iStatus;
			}
		else if( iContactsBufferItemList[iIndex]->iModType == ENSmlCntItemAdd )
			{
			retCommand = ExecuteAddL();
			}
		else if( iContactsBufferItemList[iIndex]->iModType == ENSmlCntItemReplace ||
            iContactsBufferItemList[iIndex]->iModType == ENSmlCntItemFieldLevelReplace )
			{
			ExecuteUpdateL();		
			}
		else if( iContactsBufferItemList[iIndex]->iModType == ENSmlCntItemMove )
			{
			ExecuteMoveL();
			}
		else if( iContactsBufferItemList[iIndex]->iModType == ENSmlCntItemDelete ||
				 iContactsBufferItemList[iIndex]->iModType == ENSmlCntItemSoftDelete )
			{
			ExecuteDeleteL();
			}
		//
		//	Set allready set commands from iContactsBufferItemList
		//
		if( ( iContactsBufferItemList[iIndex]->iModType == ENSmlCntItemAdd     ||
		      iContactsBufferItemList[iIndex]->iModType == ENSmlCntItemReplace ||
		      iContactsBufferItemList[iIndex]->iModType == ENSmlCntItemFieldLevelReplace ||
		      iContactsBufferItemList[iIndex]->iModType == ENSmlCntItemDelete  ||
		      iContactsBufferItemList[iIndex]->iModType == ENSmlCntItemSoftDelete ) &&
		      iContactsBufferItemList[iIndex]->iStatus  != KNSmlNoError )
			{
			retCommand = iContactsBufferItemList[iIndex]->iStatus;
			}
			
		
		//if an error occurs when performing any operation
		//return the error status 
			else if(retCommand!=KErrNone)
			{
			User::RequestComplete( iCallerStatus, retCommand );	
			}
			
			iRetCommand = retCommand;
		}
	}
	
// ----------------------------------------------------------------------------
// void CNSmlContactsDataStore::ExecuteAddL()
// ----------------------------------------------------------------------------
	
EXPORT_C TInt CNSmlContactsDataStore::ExecuteAddL()
	{
	TInt  retCommand( KErrNone );
	StripPropertyL( iItemDataAddBatch, KVersitTokenUID() ); // Remove UID's from data

	TInt err( KErrNone );
	if(iBatchMode)			
		{
		TRAP( err,LdoAddItemsL( iItemDataAddBatch,
		                   iItemDataAddBatch->Ptr(0).Length()) );
		if(err!=KErrNone)			
			{
			retCommand = err;
			TInt j = iIndex;
			while( j < iContactsBufferItemList.Count() )
				{
				if( iContactsBufferItemList[j]->iModType == ENSmlCntItemAdd &&
				iContactsBufferItemList[j]->iStatus  == KNSmlNoError )
					{
					iContactsBufferItemList[j]->iStatus = KErrNotSupported;
					iResultArray->AppendL(KErrNotSupported);
					}
				j++;
				}
			}			                          
		}
	else
		{
		const TPtr8 dataPtr = iItemDataAddBatch->Ptr(0);
		TRAP( err, LdoAddItemL( dataPtr,
					iContactsBufferItemList[iIndex]->iSize));
		if(err!=KErrNone)			
			{
			retCommand = err;
			}	
		}
	return retCommand; 
	}
	
// ----------------------------------------------------------------------------
// void CNSmlContactsDataStore::ExecuteDeleteL()
// ----------------------------------------------------------------------------
	
EXPORT_C void CNSmlContactsDataStore:: ExecuteDeleteL()
	{
	
	if(iBatchMode)			
		{
		iContactLnks = CVPbkContactLinkArray::NewL();
		
		TInt j=iIndex;
		while( j < iContactsBufferItemList.Count() )
			{
			if( (iContactsBufferItemList[j]->iModType == ENSmlCntItemDelete ||
			iContactsBufferItemList[j]->iModType == ENSmlCntItemSoftDelete) &&
			iContactsBufferItemList[j]->iStatus  == KNSmlNoError )
				{
				TContactItemId id = iContactsBufferItemList[j]->iUid; 

				if( id != 0 )
					{
					MVPbkContactLink* contactLink = iIdConverter->IdentifierToLinkLC(id);
					//check if the contact link is valid
					TInt index = iContactViewBase->IndexOfLinkL(*contactLink);
					if(index != KErrNotFound)
						{
						iContactLnks->AppendL(contactLink);	
						}
						else
						{
						iContactsBufferItemList[j]->iStatus  = KErrNotFound;
						}

					CleanupStack::Pop();//contactLink				
					}
					else 
					{
					iContactsBufferItemList[j]->iStatus  = KErrNotFound;
					}
				}
			j++;
			}

		delete iContactViewBase;
		iContactViewBase = NULL;

		iContactManager->DeleteContactsL( *iContactLnks, *this );
		iLastOperation = ENSmlDeleteOp;
		}

	}
	
// ----------------------------------------------------------------------------
// void CNSmlContactsDataStore::ExecuteUpdateL()
// ----------------------------------------------------------------------------
	
EXPORT_C void CNSmlContactsDataStore::ExecuteUpdateL()
	{
	iUid  = iContactsBufferItemList[iIndex]->iUid;

	iDataMod->SetUsedMimeType( iServerMimeType, iServerMimeVersion );

	// Check whether merging is needed or not.
	if( iDataMod->NeedsMerge() )
		{
		iMergeItem = CBufFlat::NewL(64);
		iWriteStream.Open(*iMergeItem);

		MVPbkContactLink* contactLink = iIdConverter->IdentifierToLinkLC(iUid);

		iVCardEngine->ExportVCardForSyncL(iWriteStream,*contactLink,*this);
		CleanupStack::PopAndDestroy();

		iLastOperation = ENSMLUpdateExportOp;
		}
	else
		{
			LdoUpdateItemL();
		}
	}

// ----------------------------------------------------------------------------
// void CNSmlContactsDataStore::ExecuteMoveL
// ----------------------------------------------------------------------------

EXPORT_C void CNSmlContactsDataStore::ExecuteMoveL()
	{
	// move command is not supported
	if(!iBatchMode)
		{
		ResetBuffer();
		User::RequestComplete( iCallerStatus, KErrNotSupported );	
		}
	else
		{
		iResultArray->AppendL(KErrNotSupported);
		iIndex++;
		if(iIndex == iContactsBufferItemList.Count() )
			{
			ResetBuffer();
			User::RequestComplete( iCallerStatus, KErrNone );	
			}
		else
			{
				ExecuteBufferL();
			}		
		}
	}
// ----------------------------------------------------------------------------
// void CNSmlContactsDataStore::DoCancelBatch()
// ----------------------------------------------------------------------------
EXPORT_C void CNSmlContactsDataStore::DoCancelBatch()
	{
	_DBG_FILE("CNSmlContactsDataStore::DoCancelBatch(): begin");
	if( iBatchMode )
		{
		iBatchMode = EFalse;
	//
	//	Empty BufferItemList if any
	//
		iContactsBufferItemList.ResetAndDestroy();
		iItemData = NULL;
		}

	_DBG_FILE("CNSmlContactsDataStore::DoCancelBatch(): end");
	}

// ----------------------------------------------------------------------------
// void CNSmlContactsDataStore::DoSetRemoteStoreFormatL()
// ----------------------------------------------------------------------------
EXPORT_C void CNSmlContactsDataStore::DoSetRemoteStoreFormatL( const CSmlDataStoreFormat& aServerDataStoreFormat )
	{
	_DBG_FILE("CNSmlContactsDataStore::DoSetRemoteStoreFormatL(): begin");
	
	TBool foundMime = EFalse;
	
	//
	// KErrNotSupported is returned if server does not support same mime type 
	// and same mime type version as Data Provider.
	
	TInt i=0;
	while( i< aServerDataStoreFormat.MimeFormatCount() && !foundMime )
		{
		if( iUsedMimeType.MatchF(aServerDataStoreFormat.MimeFormat(i).MimeType().DesC())      ==0 &&
		    iUsedMimeVersion.MatchF(aServerDataStoreFormat.MimeFormat(i).MimeVersion().DesC())==0 )
			{
			foundMime = ETrue;
			iServerMimeType    = aServerDataStoreFormat.MimeFormat(i).MimeType();
			iServerMimeVersion = aServerDataStoreFormat.MimeFormat(i).MimeVersion();
			}
		i++;
		}
		
	if( !foundMime )
		{
		_DBG_FILE("CNSmlContactsDataStore::DoSetRemoteStoreFormatL(): SERVER MIMETYPE NOTSUPPORTED end");
		User::Leave( KErrNotSupported );
		}
		
	iDataMod->SetPartnerStoreFormat( (CSmlDataStoreFormat&) aServerDataStoreFormat );
	
	SetOwnStoreFormatL();
			
	TInt error = iDataMod->SetUsedMimeType( iServerMimeType, iServerMimeVersion );
	
	User::LeaveIfError( error );
	
	_DBG_FILE("CNSmlContactsDataStore::DoSetRemoteStoreFormatL(): end");
	return;
	}
	
// ----------------------------------------------------------------------------
// void CNSmlContactsDataStore::SetOwnStoreFormatL()
// ----------------------------------------------------------------------------
EXPORT_C void CNSmlContactsDataStore::SetOwnStoreFormatL()
	{
	_DBG_FILE("CNSmlContactsDataStore::SetOwnStoreFormatL(): begin");
	
	if( !iStoreFormat )
		{
		TFileName fileName;
		TParse parse;

		parse.Set( GetStoreFormatResourceFileL(), &KDC_RESOURCE_FILES_DIR, NULL  );

		fileName = parse.FullName();

		RResourceFile resourceFile; 
		BaflUtils::NearestLanguageFile( iRfs, fileName );

		resourceFile.OpenL( iRfs, fileName );
		CleanupClosePushL( resourceFile );
		
		HBufC8* contactsDataFormat = resourceFile.AllocReadLC( NSML_CONTACTS_DATA_STORE_FORMAT );

		TResourceReader reader;
		reader.SetBuffer( contactsDataFormat );

		iStoreFormat = CSmlDataStoreFormat::NewLC( iStringPool, reader );
		CleanupStack::Pop(iStoreFormat); // iStoreFormat
		
		CleanupStack::PopAndDestroy(2); // contactsDataFormat, resourceFile  
		}
		
	iDataMod->SetOwnStoreFormat( *iStoreFormat );

	// Set own MIME type based on store format resource definition. Allows inherited classes to 
    // use non-standard MIME type by recource change. E.g. Operator specific MIME types can be used.
    iUsedMimeType.Set( iStoreFormat->MimeFormat(0).MimeType().DesC() );
    iUsedMimeVersion.Set( iStoreFormat->MimeFormat(0).MimeVersion().DesC() );

	_DBG_FILE("CNSmlContactsDataStore::SetOwnStoreFormatL(): end");
	return;
	}

// ----------------------------------------------------------------------------
// void CNSmlContactsDataStore::DoSetRemoteMaxObjectSize()
// ----------------------------------------------------------------------------
EXPORT_C void CNSmlContactsDataStore::DoSetRemoteMaxObjectSize( TInt aServerMaxObjectSize )
	{
	_DBG_FILE("CNSmlContactsDataStore::DoSetRemoteMaxObjectSize(): begin");
	
	iServerMaxObjectSize = aServerMaxObjectSize;
	
	_DBG_FILE("CNSmlContactsDataStore::DoSetRemoteMaxObjectSize(): end");
	}

// ----------------------------------------------------------------------------
// void CNSmlContactsDataStore::DoMaxObjectSize()
// ----------------------------------------------------------------------------
EXPORT_C TInt CNSmlContactsDataStore::DoMaxObjectSize() const
	{
	_DBG_FILE("CNSmlContactsDataStore::DoMaxObjectSize(): begin");
	_DBG_FILE("CNSmlContactsDataStore::DoMaxObjectSize() 100 k: end");
	return KNSmlDataStoreMaxSize;  // 100 k
	}

// ----------------------------------------------------------------------------
// void CNSmlContactsDataStore::DoOpenItemL()
// ----------------------------------------------------------------------------
EXPORT_C void CNSmlContactsDataStore::DoOpenItemL( TSmlDbItemUid aUid, 
										  TBool& aFieldChange, 
										  TInt& aSize, 
										  TSmlDbItemUid& aParent, 
										  TDes8& aMimeType, 
										  TDes8& aMimeVer, 
										  TRequestStatus& aStatus )
	{
	_DBG_FILE("CNSmlContactsDataStore::DoOpenItemL(): begin");

	iCallerStatus = &aStatus;
	*iCallerStatus = KRequestPending;
	
	if ( iState != ENSmlOpenAndWaiting )
		{
		User::RequestComplete( iCallerStatus, KErrNotReady );
		return;
		}
		
	iItemPos = 0;
	
	aFieldChange = EFalse; // Whole item sent NOT SUPPORT ON FIELDCHANGE
	
	aMimeType    = iStoreFormat->MimeFormat( 0 ).MimeType().DesC();
	aMimeVer	 = iStoreFormat->MimeFormat( 0 ).MimeVersion().DesC();
	aParent      = KNullDataItemId;
	
	iModType = ENSmlCntItemRead;
	
	if( !iItemData )
		{
		iItemData= CBufFlat::NewL( KNSmlItemDataExpandSize );
		}
	else
		{
		iItemData->Reset();
		}
	
	 LdoFetchItemL( aUid, *iItemData );
	 iSize = &aSize;
	
	_DBG_FILE("CNSmlContactsDataStore::DoOpenItemL(): end");
	}

// ----------------------------------------------------------------------------
// void CNSmlContactsDataStore::DoCreateItemL()
// ----------------------------------------------------------------------------
EXPORT_C void CNSmlContactsDataStore::DoCreateItemL( TSmlDbItemUid& aUid, 
											TInt aSize, 
											TSmlDbItemUid /*aParent*/, 
											const TDesC8& aMimeType, 
											const TDesC8& aMimeVer, 
											TRequestStatus& aStatus )
	{
	_DBG_FILE("CNSmlContactsDataStore::DoCreateItemL(): begin");

	iCallerStatus = &aStatus;
	*iCallerStatus = KRequestPending;
	
	TInt err = KErrNone;
	
	if ( iState != ENSmlOpenAndWaiting )
		{
		User::RequestComplete( iCallerStatus, KErrNotReady );
		_DBG_FILE("CNSmlContactsDataStore::DoCreateItemL - KErrNotReady: END");
		return;
		}
		
	iItemSize = aSize; // Size sent from server
	iModType  = ENSmlCntItemAdd;
	
	iMimeTypeItem.Set( aMimeType );
	iMimeVersionItem.Set( aMimeVer );
	
	if ( iItemPos == -1 || !iBatchMode )
		{
		iItemPos = 0;
		}
	
	
	if( iItemData )
		{
		iItemData->Reset();
		}
		
	if( !iItemDataAddBatch )
		{
		iItemDataAddBatch= CBufFlat::NewL( KNSmlItemDataExpandSize );
		}
		
	// Check MimeType and MimeVersion
	if( ( aMimeType.MatchF( KNSmlvCard21Name ) < 0 ) &&
	    ( aMimeType.MatchF( KNSmlvCard30Name ) < 0 ) &&
	    // Allow using custom MIME type defined in store format resource file
	    ( aMimeType.MatchF( iUsedMimeType ) < 0 ) )
		{
		User::RequestComplete( iCallerStatus, KErrNotSupported );
		_DBG_FILE("CNSmlContactsDataStore::DoCreateItemL - KErrNotSupported: END");
		return;
		}
    
    // Is there enough space on drive 
	if( DriveBelowCriticalLevelL( aSize ) )
		{
		err = KErrDiskFull;
		}
				
	iItemData = AddBufferListL( aUid,aSize, err );
	
	if( iBatchMode )
		{
		iState = ENSmlItemCreating;
		User::RequestComplete( iCallerStatus, KErrNone );
		_DBG_FILE("CNSmlContactsDataStore::DoCreateItemL(): begin BATCH");
		return ; // Put into buffer
		}
				
	if ( err == KErrNone )
		{
		iState = ENSmlItemCreating;
		}
	
	User::RequestComplete( iCallerStatus, err );

	_DBG_FILE("CNSmlContactsDataStore::DoCreateItemL(): end");
	}

// ----------------------------------------------------------------------------
// void CNSmlContactsDataStore::DoReplaceItemL()
// ----------------------------------------------------------------------------
EXPORT_C void CNSmlContactsDataStore::DoReplaceItemL( TSmlDbItemUid aUid, 
											 TInt aSize, 
											 TSmlDbItemUid /*aParent*/,
											 TBool aFieldChange, 
											 TRequestStatus& aStatus )
	{
	_DBG_FILE("CNSmlContactsDataStore::DoReplaceItemL(): begin");

	iCallerStatus = &aStatus;
	*iCallerStatus = KRequestPending;
	
	TInt err = KErrNone;
			
	iUid      = aUid;
	iItemSize = aSize;
	iModType  =  aFieldChange ? ENSmlCntItemFieldLevelReplace : ENSmlCntItemReplace;
	
	if ( iItemPos == -1 || !iBatchMode )
		{
		iItemPos = 0;
		}
	
	if ( iItemData )
		{
		iItemData->Reset();
		}

	iFieldLevelReplace = aFieldChange;

	iItemData = AddBufferListL( aUid, aSize, err );
	
	if( iBatchMode )
		{
		iState = ENSmlItemUpdating;
		_DBG_FILE("CNSmlContactsDataStore::DoReplaceItemL(): BATCHMODE end");
		User::RequestComplete( iCallerStatus, KErrNone );
		return;
		}
	if ( err == KErrNone )
	    {
	    iState = ENSmlItemUpdating;
	    }
	
	User::RequestComplete( iCallerStatus, err );
		
	_DBG_FILE("CNSmlContactsDataStore::DoReplaceItemL(): end");
	}

// ----------------------------------------------------------------------------
// void CNSmlContactsDataStore::DoReadItemL()
// ----------------------------------------------------------------------------
EXPORT_C void CNSmlContactsDataStore::DoReadItemL( TDes8& aBuffer )
	{
	_DBG_FILE("CNSmlContactsDataStore::DoReadItemL(): begin");

    if ( iState != ENSmlItemOpen || !iItemData )
        {
        iItemPos = -1;
        User::Leave( KErrNotReady );
        }

    if ( iItemPos == -1 )
        {
        User::Leave( KErrEof );
        }

    if ( aBuffer.Size() < iItemData->Size() - iItemPos )
        {
        iItemData->Read( iItemPos, aBuffer );
        iItemPos += aBuffer.Size();
        }
    else
        {
        iItemData->Read( iItemPos, aBuffer, iItemData->Size() - iItemPos );
        iItemPos = -1;
        }

	_DBG_FILE("CNSmlContactsDataStore::DoReadItemL(): end");
	}

// ----------------------------------------------------------------------------
// void CNSmlContactsDataStore::DoWriteItemL()
// ----------------------------------------------------------------------------
EXPORT_C void CNSmlContactsDataStore::DoWriteItemL( const TDesC8& aData )
	{
	_DBG_FILE("CNSmlContactsDataStore::DoWriteItemL(): begin");

	/* Data must be accumulated from pieces of information */
	if ( iState == ENSmlItemCreating || iState == ENSmlItemUpdating )
		{
		if ( iItemData )
			{
			if ( iItemPos == -1 )
				{
            	iStateItem = KErrEof;
            	if( iBatchMode )
            		{
            		return;
            		}
				User::Leave( KErrEof );
				}
			iItemData->InsertL( iItemData->Size(), aData );
			iItemPos += aData.Size();
			
			if ( aData.Size() > iItemSize )
				{
				iState =     ENSmlItemOverflow;
            	iStateItem = KErrOverflow;
            	_DBG_FILE("CNSmlContactsDataStore::DoWriteItemL(): KErrOverflow End");
        		DBG_ARGS(_S("CNSmlContactsDataStore::DoWriteItemL(): Server sent=(%d) Item=(%d)"),
        		            iItemSize, aData.Size()); 
            	if( iBatchMode )
            		{
            		return;
            		}
				User::Leave( KErrOverflow );				
				}
			if ( aData.Size() == iItemSize )
			    {
            	iStateItem = KErrNone;
			    }
			return;
			}
		}
		
	iStateItem = KErrNotReady;

	if( iBatchMode )
		{
       	_DBG_FILE("CNSmlContactsDataStore::DoWriteItemL(): KErrNotReady End");
		return;
		}
		
	User::Leave( KErrNotReady );
	
	_DBG_FILE("CNSmlContactsDataStore::DoWriteItemL(): end");
	}

// ----------------------------------------------------------------------------
// void CNSmlContactsDataStore::DoCommitItemL()
// ----------------------------------------------------------------------------
EXPORT_C void CNSmlContactsDataStore::DoCommitItemL( TRequestStatus& aStatus )
	{
	iCallerStatus = &aStatus;
	*iCallerStatus = KRequestPending;
	
	_DBG_FILE("CNSmlContactsDataStore::DoCommitItemL(): begin");
		
	// Data OK => insert to total buffer
	if (iModType   == ENSmlCntItemAdd && 
      	iStateItem == KErrNone )
	    {
	    iItemDataAddBatch->InsertL( iItemDataAddBatch->Size(),iItemData->Ptr(0), iItemData->Size() );
	    iItemDataAddBatch->InsertL( iItemDataAddBatch->Size(), KVersitTokenCRLF() );
      	if( iContactsBufferItemList.Count() > 0 )
      	    {
            iContactsBufferItemList[iContactsBufferItemList.Count()-1]->iStatus  = KNSmlNoError; 
      	    }
	    }
	 else if ((iModType   == ENSmlCntItemAdd ||         // If some write problems
         iModType   == ENSmlCntItemReplace ||
         iModType   == ENSmlCntItemFieldLevelReplace ) &&  
      	 iStateItem != KErrNone &&
      	 iContactsBufferItemList.Count() > 0) 
	    {
        iContactsBufferItemList[iContactsBufferItemList.Count()-1]->iStatus  = iStateItem; 
	    }

	if( iBatchMode )
		{
		iItemData = NULL;
		iState = ENSmlOpenAndWaiting;
		User::RequestComplete( iCallerStatus, KErrNone );
		return;
		}
		

	iIndex = 0;
	ExecuteBufferL();
	
	iItemData = NULL;
	iState = ENSmlOpenAndWaiting;
	
	_DBG_FILE("CNSmlContactsDataStore::DoCommitItemL(): end");
	}

// ----------------------------------------------------------------------------
// void CNSmlContactsDataStore::DoCloseItem()
// ----------------------------------------------------------------------------
EXPORT_C void CNSmlContactsDataStore::DoCloseItem()
	{
	_DBG_FILE("CNSmlContactsDataStore::DoCloseItem(): begin");

	if ( iState == ENSmlItemOpen )
		{
		iItemPos = -1;
		if ( iItemData )
			{
			delete iItemData;
			iItemData = NULL;
			}
		iState = ENSmlOpenAndWaiting;
		}
	
	_DBG_FILE("CNSmlContactsDataStore::DoCloseItem(): end");
	}

// ----------------------------------------------------------------------------
// void CNSmlContactsDataStore::DoMoveItemL()
// ----------------------------------------------------------------------------
EXPORT_C void CNSmlContactsDataStore::DoMoveItemL( TSmlDbItemUid aUid, TSmlDbItemUid /*aNewParent*/, TRequestStatus& aStatus )
	{
	_DBG_FILE("CNSmlContactsDataStore::DoMoveItemL(): begin");
	
	iCallerStatus = &aStatus;
	*iCallerStatus = KRequestPending;
	
	if( iBatchMode )
		{
		iModType = ENSmlCntItemMove;
		TInt fSize = 0;
		AddBufferListL( aUid, fSize, KErrNotSupported );	
		User::RequestComplete( iCallerStatus, KErrNone );
		return;
		}
	User::RequestComplete( iCallerStatus, KErrNotSupported );
	
	_DBG_FILE("CNSmlContactsDataStore::DoMoveItemL(): end");
	}

// ----------------------------------------------------------------------------
// void CNSmlContactsDataStore::DoDeleteItemL()
// ----------------------------------------------------------------------------
EXPORT_C void CNSmlContactsDataStore::DoDeleteItemL( TSmlDbItemUid aUid, TRequestStatus& aStatus )
	{
	_DBG_FILE("CNSmlContactsDataStore::DoDeleteItemL(): begin");
	iCallerStatus = &aStatus;
	*iCallerStatus = KRequestPending;
		
	if( iBatchMode )
		{
		if(iContactViewBase ==NULL)
		{
		CreateViewL();	
		}
	if( iModType != ENSmlCntItemSoftDelete ) // Treated like deletes
		{
		iModType  = ENSmlCntItemDelete;	
		}
			
	TInt fSize = 0;
	AddBufferListL( aUid, fSize, KErrNone );
	User::RequestComplete( iCallerStatus, KErrNone );
	return;
	}
	iUid = aUid;
	
	//add the contactlink to CVPbkContactLinkArray
	MVPbkContactLink* contactLink  = iIdConverter->IdentifierToLinkLC(aUid);
	CleanupStack::Pop();
	CVPbkContactLinkArray* contactLnks = CVPbkContactLinkArray::NewLC();
	contactLnks->AppendL( contactLink );
	
 	iContactManager->DeleteContactsL( *contactLnks, *this );
	CleanupStack::PopAndDestroy(); 
	iLastOperation = ENSmlDeleteOp;
	
	_DBG_FILE("CNSmlContactsDataStore::DoDeleteItemL(): end");
	}

// ----------------------------------------------------------------------------
// void CNSmlContactsDataStore::DoSoftDeleteItemL()
// ----------------------------------------------------------------------------
EXPORT_C void CNSmlContactsDataStore::DoSoftDeleteItemL( TSmlDbItemUid /*aUid*/, TRequestStatus& aStatus )
	{
	_DBG_FILE("CNSmlContactsDataStore::DoSoftDeleteItemL(): begin");
	iCallerStatus = &aStatus;
	*iCallerStatus = KRequestPending;
	User::RequestComplete( iCallerStatus, KErrNotSupported );
	_DBG_FILE("CNSmlContactsDataStore::DoSoftDeleteItemL(): end");
	}

// ----------------------------------------------------------------------------
// void CNSmlContactsDataStore::DoDeleteAllItemsL()
// ----------------------------------------------------------------------------
EXPORT_C void CNSmlContactsDataStore::DoDeleteAllItemsL( TRequestStatus& aStatus )
	{
	_DBG_FILE("CNSmlContactsDataStore::DoDeleteAllItemsL(): begin");
	
	iCallerStatus = &aStatus;
	*iCallerStatus = KRequestPending;
	
	//create view to get all the contactlinks to delete
	if(iContactViewBase ==NULL)
		{
		CreateViewL();	
		}
		
	iLastOperation = ENSMLDeleteAllOp;
	
	_DBG_FILE("CNSmlContactsDataStore::DoDeleteAllItemsL(): end");
	}

// ----------------------------------------------------------------------------
// TBool CNSmlContactsDataStore::DoHasSyncHistory()
// ----------------------------------------------------------------------------
EXPORT_C TBool CNSmlContactsDataStore::DoHasSyncHistory() const
	{
	_DBG_FILE("CNSmlContactsDataStore::DoHasSyncHistory(): begin");
	
	TBool doHasSyncHistory = EFalse;
	
	if( !iOpened ) 
		{
		_DBG_FILE("CNSmlContactsDataStore::DoHasSyncHistory(): NOT OPEN end");
		return doHasSyncHistory;
		}
	
	if( iSyncHistory )
		{
		if ( iOpenedStoreId != iChangeFinder->DataStoreUid() )
			{
			iChangeFinder->SetDataStoreUid( iOpenedStoreId );
			doHasSyncHistory = EFalse;
			}
		else
			{
			doHasSyncHistory = ETrue;
			}
		}
	else
		{
		iChangeFinder->SetDataStoreUid( iOpenedStoreId );
		}
		
	_DBG_FILE("CNSmlContactsDataStore::DoHasSyncHistory(): end");
	return doHasSyncHistory;
	}

// ----------------------------------------------------------------------------
// void CNSmlContactsDataStore::DoAddedItems()
// ----------------------------------------------------------------------------
EXPORT_C const MSmlDataItemUidSet& CNSmlContactsDataStore::DoAddedItems() const
	{
	_DBG_FILE("CNSmlContactsDataStore::DoAddedItems(): begin");
	if ( iState == ENSmlOpenAndWaiting )
		{
		iNewUids->Reset();
		
		if( iChangeFinder )
			{
			TRAPD( err,iChangeFinder->FindNewItemsL( *iNewUids ) );
			if( err != KErrNone )
				{
				_DBG_FILE("CNSmlContactsDataStore::DoAddedItems(): LEAVE end");				
				}
			}
		}	
	_DBG_FILE("CNSmlContactsDataStore::DoAddedItems(): end");

	return *iNewUids;
	}

// ----------------------------------------------------------------------------
// void CNSmlContactsDataStore::DoDeletedItems()
// ----------------------------------------------------------------------------
EXPORT_C const MSmlDataItemUidSet& CNSmlContactsDataStore::DoDeletedItems() const
	{
	_DBG_FILE("CNSmlContactsDataStore::DoDeletedItems(): begin");	
	if ( iState == ENSmlOpenAndWaiting )
		{
		iDeletedUids->Reset();
		
		if( iChangeFinder )
			{
			TRAPD(err,iChangeFinder->FindDeletedItemsL( *iDeletedUids ) );
			if( err != KErrNone )
				{
				_DBG_FILE("CNSmlContactsDataStore::DoDeletedItems(): LEAVE end");				
				}
			}
		}
	_DBG_FILE("CNSmlContactsDataStore::DoDeletedItems(): end");

	return *iDeletedUids;
	}

// ----------------------------------------------------------------------------
// void CNSmlContactsDataStore::DoSoftDeletedItems()
// ----------------------------------------------------------------------------
EXPORT_C const MSmlDataItemUidSet& CNSmlContactsDataStore::DoSoftDeletedItems() const
	{
	_DBG_FILE("CNSmlContactsDataStore::DoSoftDeletedItems(): begin");
	if ( iState == ENSmlOpenAndWaiting )
		{
		iSoftDeletedUids->Reset();
		
		if( iChangeFinder )
			{
			TRAPD( err, iChangeFinder->FindSoftDeletedItemsL( *iSoftDeletedUids ) );
			if( err != KErrNone )
				{
				_DBG_FILE("CNSmlContactsDataStore::DoSoftDeletedItems(): LEAVE end");				
				}
			}
		}
	_DBG_FILE("CNSmlContactsDataStore::DoSoftDeletedItems: END");
	
	return *iSoftDeletedUids;
	}

// ----------------------------------------------------------------------------
// void CNSmlContactsDataStore::DoModifiedItems()
// ----------------------------------------------------------------------------
EXPORT_C const MSmlDataItemUidSet& CNSmlContactsDataStore::DoModifiedItems() const
	{
	_DBG_FILE("CNSmlContactsDataStore::DoModifiedItems(): begin");
	if ( iState == ENSmlOpenAndWaiting )
		{
		iReplacedUids->Reset();
		
		if( iChangeFinder )
			{
			TRAPD( err, iChangeFinder->FindChangedItemsL( *iReplacedUids ) );
			if( err != KErrNone )
				{
				_DBG_FILE("CNSmlContactsDataStore::DoModifiedItems(): LEAVE end");				
				}
			}
		}
	_DBG_FILE("CNSmlContactsDataStore::DoModifiedItems: END");
	
	return *iReplacedUids;
	}

// ----------------------------------------------------------------------------
// void CNSmlContactsDataStore::DoMovedItems()
// ----------------------------------------------------------------------------
EXPORT_C const MSmlDataItemUidSet& CNSmlContactsDataStore::DoMovedItems() const
	{
	_DBG_FILE("CNSmlContactsDataStore::DoMovedItems(): begin");
	
	if ( iState == ENSmlOpenAndWaiting )
		{
		iMovedUids->Reset();
		
		if( iChangeFinder )
			{
			TRAPD( err, iChangeFinder->FindMovedItemsL( *iMovedUids ) );	
			if( err != KErrNone )
				{
				_DBG_FILE("CNSmlContactsDataStore::DoMovedItems(): LEAVE end");				
				}
			}
		}
	_DBG_FILE("CNSmlContactsDataStore::DoMovedItems: END");
	
	return *iMovedUids;
	}

// ----------------------------------------------------------------------------
// void CNSmlContactsDataStore::DoResetChangeInfoL()
// ----------------------------------------------------------------------------
EXPORT_C void CNSmlContactsDataStore::DoResetChangeInfoL( TRequestStatus& aStatus )
	{
	_DBG_FILE("CNSmlContactsDataStore::DoResetChangeInfoL(): begin");
	iCallerStatus = &aStatus;
	*iCallerStatus = KRequestPending;
	
	if( iChangeFinder )
		{
		iChangeFinder->ResetL();
		}
		
	iSnapshotRegistered = EFalse;
	
		if ( iContactsModsFetcher )
		{
		iContactsModsFetcher->FetchModificationsL( aStatus );
#ifdef __NSML_MODULETEST__
		CActiveScheduler::Start();
#endif
		}
	else
		{
		User::RequestComplete( iCallerStatus, KErrNone );
		}
	
	_DBG_FILE("CNSmlContactsDataStore::DoResetChangeInfoL(): end");
	}

// ----------------------------------------------------------------------------
// void CNSmlContactsDataStore::DoCommitChangeInfoL()
// ----------------------------------------------------------------------------
EXPORT_C void CNSmlContactsDataStore::DoCommitChangeInfoL( TRequestStatus& aStatus, 
												  const MSmlDataItemUidSet& aItems )
	{
	_DBG_FILE("CNSmlContactsDataStore::DoCommitChangeInfoL(aItems): begin");
	iCallerStatus = &aStatus;
	*iCallerStatus = KRequestPending;
	
	if( iChangeFinder )
		{
		iChangeFinder->CommitChangesL( aItems );
		User::RequestComplete( iCallerStatus, KErrNone );
		return;
		}
		
	User::RequestComplete( iCallerStatus, KErrNotReady );
	
	_DBG_FILE("CNSmlContactsDataStore::DoCommitChangeInfoL(aItems): end");
	}

// ----------------------------------------------------------------------------
// void CNSmlContactsDataStore::DoCommitChangeInfoL()
// ----------------------------------------------------------------------------
EXPORT_C void CNSmlContactsDataStore::DoCommitChangeInfoL( TRequestStatus& aStatus )
	{
	_DBG_FILE("CNSmlContactsDataStore::DoCommitChangeInfoL(): begin");
	iCallerStatus = &aStatus;
	*iCallerStatus = KRequestPending;
	
	if( iChangeFinder )
		{
		iChangeFinder->CommitChangesL();
		User::RequestComplete( iCallerStatus, KErrNone );
		return;
		}
	
	User::RequestComplete( iCallerStatus, KErrNotReady );
	
	_DBG_FILE("CNSmlContactsDataStore::DoCommitChangeInfoL(): end");
	}

// ----------------------------------------------------------------------------
// CDesCArray* CNSmlContactsDataStore::DoListStoresLC()
// ----------------------------------------------------------------------------
EXPORT_C CDesCArray* CNSmlContactsDataStore::DoListStoresLC()
	{
	_DBG_FILE("CNSmlContactsDataStore::DoListStoresLC(): begin");
	
	MVPbkContactStoreList& storeList = iContactManager->ContactStoresL();
	CDesCArrayFlat* stores = new (ELeave) CDesCArrayFlat(storeList.Count());
	CleanupStack::PushL( stores );
	
  TBuf<KVPbhkPrefixLength> phoneDataBasePrefix(KVPbhkSymbianDBPrefix);
	
	for(TInt i=0 ; i< storeList.Count(); i++)
	{
	    TBuf<KNSmlDefaultStoreNameMaxSize>  currentStore(storeList.At(i).StoreProperties().Uri().UriDes());	    	   
	    
	    if(currentStore.Find(phoneDataBasePrefix) == 0)//Check if Symbian Database
	    {	    
	    		//For backward compatability with local and remote servers
        	    //The Symbian Db name remains unchanged	   
        	    //The engine methods that create syncml packet, use this information in Database fields
	            stores->AppendL(*iPacketStoreName);
	    }
	    else//End Symbian Database check
	    {
		     stores->AppendL(storeList.At(i).StoreProperties().Uri().UriDes());
	    }
	}
	
	_DBG_FILE("CNSmlContactsDataStore::DoListStoresLC(): end");

	return stores;
	}
	
// ------------------------------------------------------------------------------------------------
// TInt CNSmlContactsDataStore::LdoFetchItemL( TSmlDbItemUid& aUid, CBufBase& aItem )
// ------------------------------------------------------------------------------------------------
EXPORT_C void CNSmlContactsDataStore::LdoFetchItemL( TSmlDbItemUid& aUid, CBufBase& aItem )
	{
	_DBG_FILE("CNSmlContactsDataStore::LdoFetchItemL(): begin");
	
	 iWriteStream.Open(aItem);
		
	 MVPbkContactLink* contactLink  = iIdConverter->IdentifierToLinkLC(aUid);
	 iVCardEngine->ExportVCardForSyncL(iWriteStream,*contactLink,*this);
	 CleanupStack::PopAndDestroy();
	 
	 iLastOperation = ENSMLFetchOp;
	
	_DBG_FILE("CNSmlContactsDataStore::LdoFetchItemL(): end");

	}

// ------------------------------------------------------------------------------------------------
// TInt CNSmlContactsDataStore::LdoAddItemL( TSmlDbItemUid& aUid,
//                                           const TDesC8& aItem,
//                                           TInt aSize,
//                                           TTime& aLastModified,
//                                           TBool& aConfidential )
// ------------------------------------------------------------------------------------------------
EXPORT_C void CNSmlContactsDataStore::LdoAddItemL( const TDesC8& aItem,
                                          TInt aSize)
	{
	_DBG_FILE("CNSmlContactsDataStore::LdoAddItemL(): begin");

	if( !iOpened )
		{
		User::Leave( KErrNotReady );
		}
		
	// Is there enough space on drive 
	if( DriveBelowCriticalLevelL( aSize ) )
		{
		User::Leave( KErrDiskFull );
		}

	 iBuf = aItem.AllocLC();
	StripPropertyL( iBuf, KVersitTokenUID() );

	 iReadStream.Open( *iBuf );
	// import a vcard without checking for duplicates
	iVCardEngine->ImportVCardForSyncL(*iStore,iReadStream,*this);
	iLastOperation = ENSmlAddOp;
	CleanupStack::Pop(iBuf);

	_DBG_FILE("CNSmlContactsDataStore::LdoAddItemL(): end");
	
	}
// ------------------------------------------------------------------------------------------------
// TInt CNSmlContactsDataStore::LdoAddItemsL( CBufBase*& aItems,
//                                           TInt aSize)
// ------------------------------------------------------------------------------------------------
EXPORT_C void CNSmlContactsDataStore::LdoAddItemsL( CBufBase*& aItems,
                                          TInt aSize)
	{
	_DBG_FILE("CNSmlContactsDataStore::LdoAddItemL(): begin");

	if( !iOpened )
		{
		User::Leave( KErrNotReady );
		}
		
	// Is there enough space on drive 
	if( DriveBelowCriticalLevelL( aSize ) )
		{
		User::Leave( KErrDiskFull );
		}

	iReadStream.Open( aItems->Ptr(0) );
	 // import vcards without checking for duplicates
	iVCardEngine->ImportVCardForSyncL(*iStore,iReadStream,*this);
	iLastOperation = ENSmlAddOp;
	_DBG_FILE("CNSmlContactsDataStore::LdoAddItemL(): end");
	
	}
		
// ------------------------------------------------------------------------------------------------
// TInt CNSmlContactsDataStore::LdoUpdateItemL(TSmlDbItemUid aUid,
//                                             const TDesC8& aItem,
//                                             TInt aSize,
//                                             TTime& aLastModified )
// ------------------------------------------------------------------------------------------------
EXPORT_C void CNSmlContactsDataStore::LdoUpdateItemL()
	{
	_DBG_FILE("CNSmlContactsDataStore::LdoUpdateItemL(): begin");
	
	iBuf = iContactsBufferItemList[iIndex]->iItemData->Ptr(0).AllocLC();
	// Remove UID's from data
	StripPropertyL( iBuf, KVersitTokenUID() );

	iReadStream.Open( *iBuf );
	iContactLink = iIdConverter->IdentifierToLinkLC(iUid);
	iVCardEngine->ImportVCardMergeL(*iContactLink, *iStore, iReadStream, *this);

	CleanupStack::Pop();
	iLastOperation = ENSMLUpdateImportOp;
	CleanupStack::Pop(iBuf);
	
	_DBG_FILE("CNSmlContactsDataStore::LdoUpdateItemL(): end");

	}


// ------------------------------------------------------------------------------------------------
// CNSmlContactsDataStore::DriveBelowCriticalLevelL()
// ------------------------------------------------------------------------------------------------
EXPORT_C TBool CNSmlContactsDataStore::DriveBelowCriticalLevelL( TInt aSize )
	{
	return SysUtil::DiskSpaceBelowCriticalLevelL( &iRfs, aSize, iDrive );
	}

// ----------------------------------------------------------------------------
// CBufBase* CNSmlContactsDataStore::AddBufferListL()
// ----------------------------------------------------------------------------
EXPORT_C CBufBase* CNSmlContactsDataStore::AddBufferListL( TSmlDbItemUid& aUid, TInt aSize, TInt aStatus )
	{
	_DBG_FILE("CNSmlContactsDataStore::AddBufferListL(): begin");
	
	CNSmlContactsBufferItem* newBufferItem = new (ELeave) CNSmlContactsBufferItem;
	CleanupStack::PushL( newBufferItem );
	
	if( iModType == ENSmlCntItemAdd )
		{
		newBufferItem->iPUid     = &aUid;
		newBufferItem->iItemData = CBufFlat::NewL( KNSmlItemDataExpandSize );
    	newBufferItem->iStatus   = KErrGeneral; // Default for add
		}
	else
		{
		newBufferItem->iPUid = &aUid;
		newBufferItem->iUid = aUid;		
		newBufferItem->iItemData = CBufFlat::NewL( KNSmlItemDataExpandSize );
    	newBufferItem->iStatus = KNSmlNoError; // Default if not handled
		}		
	
	newBufferItem->iModType = iModType;
	newBufferItem->iMimeType = iMimeTypeItem.AllocL();
	newBufferItem->iMimeVersion = iMimeVersionItem.AllocL();
	newBufferItem->iSize = aSize;
	
	if( aStatus != KErrNone )
		{
		newBufferItem->iStatus  = aStatus;
		}
		
	iContactsBufferItemList.AppendL( newBufferItem );
	
	iMimeTypeItem.Set( KNullDesC8 ); 
	iMimeVersionItem.Set( KNullDesC8 );
	
	
	CleanupStack::Pop();	
	
	if( iModType == ENSmlCntItemAdd && 
	    aStatus  == KErrNotSupported )
		{
		iStateItem = KErrNotSupported;
		}
		
	_DBG_FILE("CNSmlContactsDataStore::AddBufferListL(): end");

	return newBufferItem->iItemData;

	}
	
// ------------------------------------------------------------------------------------------------
// void CNSmlContactsDataStore::StripPropertyL( HBufC8*& aItem, const TDesC8& aProperty )
// ------------------------------------------------------------------------------------------------
EXPORT_C void CNSmlContactsDataStore::StripPropertyL( HBufC8*& aItem, const TDesC8& aProperty ) const
	{
	_DBG_FILE("CNSmlContactsDataStore::StripPropertyL(): begin");

	if( !aItem )
		{
		_DBG_FILE("CNSmlContactsDataStore::StripPropertyL(): NULL pointer");
		return;
		}

	TPtr8 hItemPtr( aItem->Des() );
	HBufC8* propBuf = HBufC8::NewLC( aProperty.Length() + 2 );
	TPtr8 propPtr( propBuf->Des() );

	propPtr.Append( KVersitTokenCRLF() );
	propPtr.Append( aProperty );
	TInt propPos = hItemPtr.Find( propPtr );
	if( propPos != KErrNotFound )
		{
		_DBG_FILE("CNSmlContactsDataStore::StripPropertyL(): property found in item data -> removing it");
		_DBG_FILE( aProperty.Ptr() );
		TPtrC8 propPosPtr( hItemPtr.Mid( propPos + KVersitTokenCRLF().Length() ) );
		TInt propEndPos = propPosPtr.Find( KVersitTokenCRLF() );
		hItemPtr.Delete( propPos, propEndPos + KVersitTokenCRLF().Length() );
		_DBG_FILE("CNSmlContactsDataStore::StripPropertyL(): property removed");
		}
	CleanupStack::PopAndDestroy(); // propBuf
	_DBG_FILE("CNSmlContactsDataStore::StripPropertyL(): end");
	}
	
// ------------------------------------------------------------------------------------------------
// void CNSmlContactsDataStore::StripPropertyL( CBufBase*& aItem, const TDesC8& aProperty )
// ------------------------------------------------------------------------------------------------
EXPORT_C void CNSmlContactsDataStore::StripPropertyL( CBufBase*& aItem, const TDesC8& aProperty ) const
	{
	_DBG_FILE("CNSmlContactsDataStore::StripPropertyL(): begin");
	
	if( !aItem )
		{
		_DBG_FILE("CNSmlContactsDataStore::StripPropertyL(): NULL pointer");
		return;
		}
		
	HBufC8* propBuf = HBufC8::NewLC( aProperty.Length() + 2 );
	TPtr8 propPtr( propBuf->Des() );
	propPtr.Append( KVersitTokenCRLF() );
	propPtr.Append( aProperty );
	TInt propPos = aItem->Ptr(0).Find( propPtr );
	
	while( propPos != KErrNotFound )
		{
		_DBG_FILE("CNSmlContactsDataStore::StripPropertyL(): property found in item data -> removing it");
		_DBG_FILE( aProperty.Ptr() );
		TInt propEndPos = aItem->Ptr( propPos + KVersitTokenCRLF().Length()).Find(KVersitTokenCRLF() );
		if( propEndPos != KErrNotFound )
			{
			aItem->Delete( propPos + KVersitTokenCRLF().Length(), propEndPos );		
			}
		
		propPos = aItem->Ptr(0).Find( propPtr );
		_DBG_FILE("CNSmlContactsDataStore::StripPropertyL(): property removed");
		}
	CleanupStack::PopAndDestroy(); // propBuf
	_DBG_FILE("CNSmlContactsDataStore::StripPropertyL(): end");
	}
	
// ----------------------------------------------------------------------------
// CNSmlContactsDataStore::IsConfidentialL
// ----------------------------------------------------------------------------
//	
EXPORT_C TBool CNSmlContactsDataStore::IsConfidentialL(  MVPbkStoreContact& aItem )
    {
    _DBG_FILE("CNSmlContactsDataStore::IsConfidentialL(): begin");
    TBool ret( EFalse );
    
    //find X-CLASS field type
	const MVPbkFieldType* syncclass = 
        iContactManager->FieldTypes().Find(R_VPBK_FIELD_TYPE_SYNCCLASS );
	CVPbkBaseContactFieldTypeIterator* itr = 
        CVPbkBaseContactFieldTypeIterator::NewLC( *syncclass, 
            aItem.Fields() );
	while ( itr->HasNext() )
        {
        const MVPbkBaseContactField* field = itr->Next();
        const MVPbkContactFieldTextData& data = MVPbkContactFieldTextData::Cast(field->FieldData());
        TPtrC ptr = data.Text();
        //compare the sync type
        if ( ptr.Compare( KNSmlContactSyncNoSync ) == 0 )
            {
            _DBG_FILE("CNSmlContactsDataStore::IsConfidentialL(): \
                       find confidential");
            ret = ETrue;
            }
         
        }
	CleanupStack::PopAndDestroy( itr );
    
    _DBG_FILE("CNSmlContactsDataStore::IsConfidentialL(): end");
    return ret;
    }
    
 // ----------------------------------------------------------------------------
// CNSmlContactsDataStore::ResetBuffer
// ----------------------------------------------------------------------------
//	
EXPORT_C void CNSmlContactsDataStore:: ResetBuffer()
	{
	if( iItemDataAddBatch )
		{
		iItemDataAddBatch->Reset();
		}
	iContactsBufferItemList.ResetAndDestroy();
	}
// -----------------------------------------------------------------------------
// From MVPbkContactStoreListObserver  
// Called when a contact store is ready to use.
// -----------------------------------------------------------------------------
 
EXPORT_C void CNSmlContactsDataStore::StoreReady( MVPbkContactStore& /*aContactStore*/ )
	{
	_DBG_FILE("CNSmlContactsDataStore::StoreReady(): begin");
	//The contact data base opened successfully
	iOpened = ETrue;
	//id to uniquely identify the opened store 
	iOpenedStoreId = DefaultHash::Des16(
						iStore->StoreProperties().Uri().UriDes());
	
	TRAPD(error, iContactsModsFetcher->FetchModificationsL( *iCallerStatus ));
	if(error != KErrNone)
		{
		User::RequestComplete( iCallerStatus, error );
		}
	_DBG_FILE("CNSmlContactsDataStore::StoreReady(): end");
	}


// ---------------------------------------------------------------------------
//  From MVPbkContactStoreListObserver  
//	Called when a contact store becomes unavailable.
// ---------------------------------------------------------------------------
//
EXPORT_C void CNSmlContactsDataStore::StoreUnavailable(
        MVPbkContactStore& /*aContactStore*/,
        TInt aReason )
    {
    _DBG_FILE("CNSmlContactsDataStore::StoreUnavailable(): begin");

    //Unable open the default contact data base
	iOpened = EFalse;
	delete iStoreName;
	iStoreName = NULL;
	iOpenedStoreId = 0;
	User::RequestComplete( iCallerStatus, aReason );
	_DBG_FILE("CNSmlContactsDataStore::StoreUnavailable(): end");		
	return;
	
    }

// ---------------------------------------------------------------------------
// From MVPbkContactStoreListObserver  
// ---------------------------------------------------------------------------
//
EXPORT_C void CNSmlContactsDataStore::HandleStoreEventL(
        MVPbkContactStore& /*aContactStore*/,
        TVPbkContactStoreEvent /*aStoreEvent*/ )
	{
	//Indication not needed.
	}
    
// ---------------------------------------------------------------------------
//  CNSmlContactsDataStore::VPbkSingleContactOperationComplete
// ---------------------------------------------------------------------------
//
EXPORT_C void CNSmlContactsDataStore::VPbkSingleContactOperationComplete(
		MVPbkContactOperationBase& aOperation,
		MVPbkStoreContact* aContact )
	{
	MVPbkContactOperationBase* operation = &aOperation;
	if ( operation )
	{
		delete operation;
		operation = NULL;
	}
	TRAPD(error ,SingleContactOperationCompleteL(aContact));
	delete aContact;
	if(error!= KErrNone)
		{
			User::RequestComplete( iCallerStatus, error );
		}
	}

// ---------------------------------------------------------------------------
//  CNSmlContactsDataStore::VPbkSingleContactOperationFailed
// ---------------------------------------------------------------------------
//
EXPORT_C void CNSmlContactsDataStore::VPbkSingleContactOperationFailed(
    MVPbkContactOperationBase& aOperation,
    TInt aError )
    {
	MVPbkContactOperationBase* operation = &aOperation;
	if ( operation )
	    {
		delete operation;
		operation = NULL;
	    }
	
	 if(iLastOperation == ENSMLFetchOp)
		{
	 	iWriteStream.Close();
	 	User::RequestComplete( iCallerStatus, aError );
	 	}
	else if (iLastOperation == ENSMLUpdateExportOp)
        {
        iWriteStream.Close();
        delete iMergeItem;
        iMergeItem = NULL;
        User::RequestComplete( iCallerStatus, aError );
        }
	else if (iLastOperation == ENSMLUpdateImportOp)
        {
        if(iBatchMode)
            {
            iResultArray->Append( aError ); 
            } 
		delete iContactLink;
        iContactLink = NULL;	
        delete iBuf;
        iBuf = NULL;
        iReadStream.Close();
        iIndex++;
        if(iIndex == iContactsBufferItemList.Count() )
            {
            ResetBuffer();
            User::RequestComplete( iCallerStatus, aError );    
            }
        else
            {
            _DBG_FILE("VPbkSingleContactOperationFailed, Continuing to complete the Batch Operation");
            ExecuteBufferL();
            }
        }
	else if(iLastOperation == ENSmlRetrieveOp)
	    {
        iRetrieveCount--;	        
        if(iBatchMode)
            {
            iResultArray->AppendL( aError ); 
            }        
        iIndex++;
        if(iIndex == iContactsBufferItemList.Count() )
            {
            ResetBuffer();
            User::RequestComplete( iCallerStatus, aError );    
            }
        else if(!iRetrieveCount)
            {
            _DBG_FILE("VPbkSingleContactOperationFailed, Continuing to complete the Batch Operation");
            ExecuteBufferL();
            }
	    }
    else
        {
        _DBG_FILE("VPbkSingleContactOperationFailed, No Matching LastOperation Completing the Operation");
        User::RequestComplete( iCallerStatus, aError );    
        }	
	_DBG_FILE("CNSmlContactsDataStore::VPbkSingleContactOperationFailed(): end");
	}
 // ---------------------------------------------------------------------------
//  CNSmlContactsDataStore::VPbkSingleContactOperationFailed
// ---------------------------------------------------------------------------
//
EXPORT_C void CNSmlContactsDataStore::SingleContactOperationCompleteL(
 		MVPbkStoreContact* aContact)
	{
	if(iLastOperation == ENSMLFetchOp)
		{
		iWriteStream.Close();

		iItemData->Compress();
		//strip the fields not supported by server
		iDataMod->StripTxL( *iItemData );
		*iSize = iItemData->Size();
		iState = ENSmlItemOpen;
		if ( iServerMaxObjectSize == 0 || *iSize <= iServerMaxObjectSize )
			{
			User::RequestComplete( iCallerStatus, KErrNone );
			}
		else
			{
			User::RequestComplete( iCallerStatus, KErrTooBig );
			}
		}
	else if (iLastOperation == ENSMLUpdateExportOp)
		{
		iDataMod->MergeRxL( *iContactsBufferItemList[iIndex]->iItemData, 
		    *iMergeItem, iFieldLevelReplace );
		iWriteStream.Close();
		delete iMergeItem;
		iMergeItem = NULL;
		LdoUpdateItemL();
		}
	else if (iLastOperation == ENSMLUpdateImportOp)
		{
		iFieldLevelReplace = EFalse;
		
		if(!IsConfidentialL(*aContact))
			{
			TInt32 id = iIdConverter->LinkToIdentifier(*aContact->CreateLinkLC());
			
			TNSmlSnapshotItem item(id);
			
			MVPbkStoreContact2* tempContact = 
					reinterpret_cast<MVPbkStoreContact2*> (aContact->StoreContactExtension (KMVPbkStoreContactExtension2Uid));
       		
			MVPbkStoreContactProperties *contactProterties 
										= tempContact ->PropertiesL();;
			CleanupDeletePushL(contactProterties);				
			item.SetLastChangedDate(contactProterties->LastModifiedL());
			CleanupStack::PopAndDestroy();
			
			item.SetSoftDelete( EFalse );
			
			TRAP_IGNORE(iChangeFinder->ItemUpdatedL( item ));
			
			CleanupStack::PopAndDestroy();
			if(iBatchMode)
				{
				iResultArray->Append(KErrNone);	
				}
			
	        delete iContactLink;
	        iContactLink = NULL;
			
			delete iBuf;
			iBuf = NULL;
			iReadStream.Close();
			
			iIndex++;
			if(iIndex == iContactsBufferItemList.Count() )
				{
				ResetBuffer();
				User::RequestComplete( iCallerStatus, iRetCommand );	
				}
			else
				{
					ExecuteBufferL();
				}
			}
		
		}
	else if(iLastOperation == ENSmlRetrieveOp)
		{
		iRetrieveCount--;	
		if(!IsConfidentialL(*aContact))
			{
			TInt32 id = iIdConverter->LinkToIdentifier(*aContact->CreateLinkLC());
			*iContactsBufferItemList[iIndex]->iPUid = id;
			
			TNSmlSnapshotItem item(id);
			
       		MVPbkStoreContact2* tempContact = 
       		    reinterpret_cast<MVPbkStoreContact2*> (aContact->StoreContactExtension (KMVPbkStoreContactExtension2Uid));  
       		       			
			MVPbkStoreContactProperties *contactProterties 
										= tempContact ->PropertiesL();;
			CleanupDeletePushL(contactProterties);				
			item.SetLastChangedDate(contactProterties->LastModifiedL());
			CleanupStack::PopAndDestroy();
			
			item.SetSoftDelete( EFalse );
			// Add the snapshot item to the Change Finder.
			TRAP_IGNORE(iChangeFinder->ItemAddedL( item ));
			
			CleanupStack::PopAndDestroy();
			}
			if(iBatchMode)
			{
			iResultArray->AppendL(iRetCommand);	
			}
			
			iIndex++;
			if(iIndex == iContactsBufferItemList.Count() )
				{
				ResetBuffer();
				User::RequestComplete( iCallerStatus, iRetCommand );	
				}
			else if(!iRetrieveCount)
				{
					ExecuteBufferL();
				}
			
			}
 }
 // ---------------------------------------------------------------------------
//  CNSmlContactsDataStore::ContactsSaved.
// ---------------------------------------------------------------------------
//
EXPORT_C void CNSmlContactsDataStore::ContactsSaved( 
		MVPbkContactOperationBase& aOperation,
        MVPbkContactLinkArray* aResults ) 
	{
	MVPbkContactOperationBase* operation = &aOperation;
	if ( operation )
	{
		delete operation;
		operation = NULL;
	}

	if(iLastOperation == ENSmlAddOp)
		{
		//Count of contacts to be added from server to client 
		TInt  cntAddCount( 0 );
		TInt i(iIndex);
		while( i<iContactsBufferItemList.Count() )
			{
			if ( iContactsBufferItemList[i]->iModType == ENSmlCntItemAdd &&
			     iContactsBufferItemList[i]->iStatus  == KNSmlNoError )
				{
				cntAddCount++;
				}
			i++;
			}
		
		//Count of contacts successfully added to client	
		iRetrieveCount = aResults->Count();
		
		//Count of contacts which could not be added
		TInt cntAddFailedCount(0);
		cntAddFailedCount = cntAddCount-iRetrieveCount;
		while(cntAddFailedCount > 0)
		{
			iContactsBufferItemList[iIndex++]->iStatus  = KErrGeneral;
			iResultArray->Append(KErrGeneral);
			cntAddFailedCount--;
		}
		
		if ( aResults->Count() > 0 )
			{
			for(TInt i=0;i<aResults->Count();i++)
				{
				//retrieve the contact
				TRAPD(error,iContactManager->RetrieveContactL( aResults->At(i), *this ));
				if(error != KErrNone)
					{
					User::RequestComplete( iCallerStatus, error );	
					}
			iLastOperation = ENSmlRetrieveOp;				 	
				}
			}
		delete iBuf;
		delete aResults;
		iBuf = NULL;
		iReadStream.Close();	
		}
	
	}
        
 // ---------------------------------------------------------------------------
//  CNSmlContactsDataStore::ContactsSavingFailed
// ---------------------------------------------------------------------------
//       
EXPORT_C void CNSmlContactsDataStore::ContactsSavingFailed( 
        MVPbkContactOperationBase& aOperation, TInt aError )
	{
	MVPbkContactOperationBase* operation = &aOperation;
	if ( operation )
	{
		delete operation;
		operation = NULL;
	}
	
	if(iLastOperation == ENSmlAddOp)
	{
	if(iBatchMode)
		{
		while( iIndex < iContactsBufferItemList.Count() &&
		iContactsBufferItemList[iIndex]->iModType == ENSmlCntItemAdd )
			{
			iContactsBufferItemList[iIndex++]->iStatus  = KErrGeneral;
			iResultArray->Append(KErrGeneral);
			}	
		}
	else
		{
		iIndex++;			
		}

	if(iIndex == iContactsBufferItemList.Count())
		{
		User::RequestComplete( iCallerStatus, aError );
		ResetBuffer();
		delete iBuf;
	 	iBuf = NULL;
	 	iReadStream.Close();	
		}
		//Execute the remaining commands in batch
		else
		{
			TRAPD(error,ExecuteBufferL());
			if(error != KErrNone)
			{
			User::RequestComplete( iCallerStatus, error );	
			}
		}
		}
	}
// ---------------------------------------------------------------------------
//  CNSmlContactsDataStore::StepComplete
// ---------------------------------------------------------------------------
//
EXPORT_C void CNSmlContactsDataStore::StepComplete(
		 MVPbkContactOperationBase& /*aOperation*/, 
        TInt aStepSize )
	{
	if(iLastOperation == ENSmlDeleteOp)
		{
		if(iBatchMode)
			{
			for(TInt i = 0 ; i<aStepSize && iIndex < iContactsBufferItemList.Count() ;i++)
				{
				iResultArray->Append(KErrNone);
				if( iContactsBufferItemList[iIndex]->iModType == ENSmlCntItemDelete &&
			    iContactsBufferItemList[iIndex]->iStatus  == KNSmlNoError )
					{
					iContactsBufferItemList[iIndex]->iStatus  = KErrNone;
					iUid  = iContactsBufferItemList[iIndex]->iUid;
					iChangeFinder->ItemDeleted( TNSmlSnapshotItem( iUid ));
					}						
					else if( iContactsBufferItemList[iIndex]->iModType == ENSmlCntItemSoftDelete &&
				   		 iContactsBufferItemList[iIndex]->iStatus  == KNSmlNoError )
					{
					
					iContactsBufferItemList[iIndex]->iStatus  = KErrNone;
					iUid  = iContactsBufferItemList[iIndex]->iUid;
					TRAP_IGNORE(iChangeFinder->ItemSoftDeletedL( TNSmlSnapshotItem( iUid ) ));
					}
					iIndex++;
				}
			}
			else
			{
			iChangeFinder->ItemDeleted( TNSmlSnapshotItem( iUid ) );	
			User::RequestComplete( iCallerStatus, KErrNone );
			}
		}
	}
 // ---------------------------------------------------------------------------
//  CNSmlContactsDataStore::StepFailed
// ---------------------------------------------------------------------------
//
EXPORT_C TBool CNSmlContactsDataStore::StepFailed(
        MVPbkContactOperationBase& /*aOperation*/,
        TInt /*aStepSize*/,
        TInt aError )
 {
 if(iLastOperation == ENSmlDeleteOp)
	{
	 	if(iBatchMode)
	 	{
			if( iIndex < iContactsBufferItemList.Count() &&
			iContactsBufferItemList[iIndex]->iModType == ENSmlCntItemDelete )
			{
			iContactsBufferItemList[iIndex++]->iStatus  = KErrGeneral;
			iResultArray->Append(KErrGeneral);
			}
		 	
	 	}
	 	else
	 	{
	 		User::RequestComplete( iCallerStatus, aError );	
	 	}
	}
	return ETrue;
 }
// ---------------------------------------------------------------------------
//  CNSmlContactsDataStore::OperationComplete
// ---------------------------------------------------------------------------
//
EXPORT_C void CNSmlContactsDataStore::OperationComplete
		( MVPbkContactOperationBase& aOperation )
	{
	MVPbkContactOperationBase* operation = &aOperation;
	if ( operation )
	{
		delete operation;
		operation = NULL;
	}

	TRAPD(error ,OperationCompleteL());	
	if(error != KErrNone)
		{
		User::RequestComplete( iCallerStatus, error );
		}
	}

// ---------------------------------------------------------------------------
//  CNSmlContactsDataStore::OperationCompleteL
// ---------------------------------------------------------------------------
//
EXPORT_C void CNSmlContactsDataStore::OperationCompleteL()
	{
	if(iLastOperation == ENSmlDeleteOp)
		{
		if(iBatchMode )
			{
			if(iResultArray->Count() != iContactsBufferItemList.Count())
				{
				while( iIndex < iContactsBufferItemList.Count() && 
				iContactsBufferItemList[iIndex]->iModType == ENSmlCntItemDelete 
				&& iContactsBufferItemList[iIndex]->iStatus  == KErrNotFound )
					{
					iResultArray->Append(KErrNotFound);
					iIndex++;
					}
				}
			
			if(iIndex == iContactsBufferItemList.Count())
				{
				if(iItemData)
					{
					iItemData->Reset();    
					}

				ResetBuffer();
				User::RequestComplete( iCallerStatus, KErrNone );
				}
				else
				{
				ExecuteBufferL();
				}	
			}
			else
			{
			
			if(iItemData)
				{
				iItemData->Reset();    
				}

			ResetBuffer();
			}
		}
		else
		{
		if(iLastOperation == ENSMLDeleteAllOp)
	        {
	        iDeleteAllOperation = NULL;
	        }
		if( iChangeFinder )
			{
			iChangeFinder->ResetL();		
			}
		iSnapshotRegistered = EFalse;
		
		delete iContactLnks;
		iContactLnks = NULL;
		iContactLink = NULL;
		
		delete iContactViewBase;
		iContactViewBase = NULL;
		User::RequestComplete( iCallerStatus, KErrNone );	
		
		}
		
	}

// ---------------------------------------------------------------------------
// CNSmlContactsDataStore::ContactViewReady
// Implements the view ready function of MVPbkContactViewObserver
// --------------------------------------------------------------------------- 
EXPORT_C void CNSmlContactsDataStore::ContactViewReady(
                MVPbkContactViewBase& /*aView*/ ) 
	{
	if(iLastOperation == ENSMLDeleteAllOp)
		{
		TRAPD(error,DoDeleteAllContactsL());
		if(error != KErrNone)
			{
			User::RequestComplete( iCallerStatus, error );
			}
		}
	}
// ---------------------------------------------------------------------------
// CNSmlContactsDataStore::PrepareForContactsDeleteL()
// Delete all the contacts at client
// ---------------------------------------------------------------------------	
EXPORT_C void CNSmlContactsDataStore::DoDeleteAllContactsL()
	{
	iContactLnks = CVPbkContactLinkArray::NewL();
	TInt contactCount = iContactViewBase->ContactCountL();
	//add all the contact links to  ContactLinkArray
	for( TInt i = 0; i < contactCount; i++ )
		{
			const MVPbkViewContact& contact = 
						iContactViewBase->ContactAtL(i);
			iContactLink = contact.CreateLinkLC();
			iContactLnks->AppendL(iContactLink);
			CleanupStack::Pop();
		}
	
	iDeleteAllOperation = iContactManager->DeleteContactsL( *iContactLnks, *this );
	
	}

// ---------------------------------------------------------------------------
// CNSmlContactsDataStore::ContactViewUnavailable
// Implements the view unavailable function of MVPbkContactViewObserver
// --------------------------------------------------------------------------- 
EXPORT_C void CNSmlContactsDataStore::ContactViewUnavailable(
                MVPbkContactViewBase& /*aView*/ )  
	{
	   
	}

// ---------------------------------------------------------------------------
// CNSmlContactsDataStore::ContactAddedToView
// Implements the add contact function of MVPbkContactViewObserver
// --------------------------------------------------------------------------- 
EXPORT_C void CNSmlContactsDataStore::ContactAddedToView(
            MVPbkContactViewBase& /*aView*/, 
            TInt /*aIndex*/, 
            const MVPbkContactLink& /*aContactLink*/ ) 
	{
	}

// ---------------------------------------------------------------------------
// CNSmlContactsDataStore::ContactRemovedFromView
// Implements the remove contact function of MVPbkContactViewObserver
// --------------------------------------------------------------------------- 
EXPORT_C void CNSmlContactsDataStore::ContactRemovedFromView(
                MVPbkContactViewBase& /*aView*/, 
                TInt /*aIndex*/, 
                const MVPbkContactLink& /*aContactLink*/ )  
	{
	}

// ---------------------------------------------------------------------------
// CNSmlContactsDataStore::ContactViewError
// Implements the view error function of MVPbkContactViewObserver
// --------------------------------------------------------------------------- 
EXPORT_C void CNSmlContactsDataStore::ContactViewError(
            MVPbkContactViewBase& /*aView*/, 
            TInt /*aError*/, 
            TBool /*aErrorNotified*/ )  
	{

	}

// -----------------------------------------------------------------------------
// CNSmlContactsDataStore::CreateViewL()
// Create a contact view 
// -----------------------------------------------------------------------------
EXPORT_C void CNSmlContactsDataStore::CreateViewL()
	{
	
	CVPbkContactViewDefinition* viewDef = CVPbkContactViewDefinition::NewL();
	CleanupStack::PushL( viewDef );
	viewDef->SetType( EVPbkContactsView );

	viewDef->SetUriL(iStore->StoreProperties().Uri().UriDes());

	 iContactViewBase = iContactManager->CreateContactViewLC( 
				                         *this, 
				                         *viewDef, 
				                         iContactManager->FieldTypes()  );
	CleanupStack::Pop();
	CleanupStack::PopAndDestroy(viewDef);	
	}

// ------------------------------------------------------------------------------------------------
// CNSmlContactsDataStore::GetStoreFormatResourceFileL
// ------------------------------------------------------------------------------------------------
EXPORT_C const TDesC& CNSmlContactsDataStore::GetStoreFormatResourceFileL()
    {
    _DBG_FILE("CNSmlContactsDataStore::GetStoreFormatResourceFileL(): begin");
    
    // Check correct Data Sync protocol
    TInt value( EDataSyncNotRunning );
    TInt error = RProperty::Get( KPSUidDataSynchronizationInternalKeys,
        KDataSyncStatus, value );
    if ( error == KErrNone &&
         value == EDataSyncRunning )
        {
        return KNSmlContactsStoreFormatRsc_1_1_2;
        }
    else // error or protocol version 1.2 
        {
        return KNSmlContactsStoreFormatRsc_1_2;
        }
    _DBG_FILE("CNSmlContactsDataStore::GetStoreFormatResourceFileL(): end");
    }

// ------------------------------------------------------------------------------------------------
// CNSmlContactsDataStore::CreateModsFetcherL
// ------------------------------------------------------------------------------------------------
EXPORT_C MContactsModsFetcher* CNSmlContactsDataStore::CreateModsFetcherL()
    {
    _DBG_FILE("CNSmlContactsDataStore::CreateModsFetcherL(): begin");
    CNSmlContactsModsFetcher* modsFetcher = 
        new ( ELeave ) CNSmlContactsModsFetcher( iSnapshotRegistered, 
        *iContactManager,*iStore, iKey, *iChangeFinder );
    CleanupStack::PushL( modsFetcher );
    modsFetcher->ConstructL();
    CleanupStack::Pop( modsFetcher );
    _DBG_FILE("CNSmlContactsDataStore::CreateModsFetcherL(): end");
    return modsFetcher;
    }

// ------------------------------------------------------------------------------------------------
// CNSmlContactsDataStore::GetDataMod
// ------------------------------------------------------------------------------------------------
EXPORT_C CNSmlDataModBase& CNSmlContactsDataStore::GetDataMod()
    {
    _DBG_FILE("CNSmlContactsDataStore::GetDataMod()");
    return *iDataMod;
    }

// End of File  
