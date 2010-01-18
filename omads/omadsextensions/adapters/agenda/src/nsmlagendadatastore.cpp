/*
* Copyright (c) 2005 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:  DS agenda data store
*
*/



// INCLUDES
#include <ecom.h>
#include <barsc.h>
#include <bautils.h>
#include <calsession.h> 
#include <caldataexchange.h> 
#include <calentryview.h>
#include <caliterator.h>
#include <calentry.h>
#include <caldataformat.h>
#include <caltime.h>
#include <nsmldsimpluids.h>
#include <sysutil.h>
#include <SmlDataProvider.h>
#include <SmlDataFormat.h>
#include <SmlDataSyncDefs.h>
#include <data_caging_path_literals.hrh>
#include <nsmlagendadatastore_1_1_2.rsg>
#include <e32property.h>
#include <DataSyncInternalPSKeys.h>
#include <CalenImporter.h>
#include <CalenInterimUtils2.h>
#include <utf.h>
#include <nsmldebug.h>
#include <nsmlagendaadapterhandler.h>
#include <calenmulticaluids.hrh>
#include <calenmulticalutil.h>
#include <nsmlconstants.h>
#include <centralrepository.h> 
#include "nsmlagendadebug.h"
#include "nsmlagendadatastore.h"
#include "nsmlagendadataprovider.h"
#include "nsmlchangefinder.h"
#include "NSmlDataModBase.h"
#include "nsmlagendadefines.hrh"

#ifndef __WINS__
// This lowers the unnecessary compiler warning (armv5) to remark.
// "Warning:  #174-D: expression has no effect..." is caused by 
// DBG_ARGS8 macro in no-debug builds.
#pragma diag_remark 174
#endif
// ============================= MEMBER FUNCTIONS ==============================
// -----------------------------------------------------------------------------
// CNSmlAgendaDataStore::CNSmlAgendaDataStore
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CNSmlAgendaDataStore::CNSmlAgendaDataStore() :
	iKey( TKeyArrayFix( _FOFF( TNSmlSnapshotItem, ItemId() ), ECmpTInt ) ),
	iPos( -1 ),
	iHasHistory( EFalse ),
	iModificationCount( KNSmlCompactAfterChanges ),
	iState( ENSmlClosed ),
	iSnapshotRegistered( EFalse ),
	iDrive( -1 ),
	iReplaceItemId( -1 ),
	iRXEntryType( ENSmlNotSet ),
	iTXEntryType( ENSmlNotSet ),
	iIsHierarchicalSyncSupported( EFalse ),
	iParentItemId( 0 )
	{
	FLOG(_L("CNSmlAgendaDataStore::CNSmlAgendaDataStore(): BEGIN"));
	// RD_MULTICAL
	iDataMimeType = ENSmlNone;
	iAgendaAdapterHandler = NULL;
	// RD_MULTICAL
	FLOG(_L("CNSmlAgendaDataStore::CNSmlAgendaDataStore(): END"));
	}

// -----------------------------------------------------------------------------
// CNSmlAgendaDataStore::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CNSmlAgendaDataStore::ConstructL()
	{
	FLOG(_L("CNSmlAgendaDataStore::ConstructL: BEGIN"));

	iStringPool.OpenL();
	User::LeaveIfError( iRfs.Connect() );

	iDataMod = new ( ELeave ) CNSmlVCalMod();
	
	iNewUids = new ( ELeave ) CNSmlDataItemUidSet();
	iDeletedUids = new ( ELeave ) CNSmlDataItemUidSet();
	iSoftDeletedUids = new ( ELeave ) CNSmlDataItemUidSet();
	iMovedUids = new ( ELeave ) CNSmlDataItemUidSet();
	iReplacedUids = new ( ELeave ) CNSmlDataItemUidSet();
	
	iDefaultStoreFileName = HBufC::NewL( KNSmlDefaultStoreNameMaxSize );
	*iDefaultStoreFileName = KNSmlAgendaFileNameForDefaultDB; 
	
	
	// RD_MULTICAL
	iAgendaAdapterLog = NULL;
	iCalOffsetArr = new (ELeave) CArrayFixFlat<TUint>( KArrayGranularity );
	iCommittedUidArr = new ( ELeave ) CNSmlDataItemUidSet();
	//iCalOffsetVal = CCalenMultiCalUtil::GetOffsetL();
	// TODO: Tweak code: BEGIN
	iCalOffsetVal = 100000;	
	ListAllAgendaPluginAdaptersL();
	// END  
		
	iOwnFormat = DoOwnStoreFormatL();
	// RD_MULTICAL
	iDataMod->SetOwnStoreFormat( *iOwnFormat );	
    iInterimUtils = CCalenInterimUtils2::NewL();
    
    // Initialize the FieldId to a default value [UID3 of the application]
    // This value will be rewritten once the exact single CalendarDB is determined
    // In case of HF synchronization involving multiple CalendarDB's the below value 
    // will be retained.
    iOpenedStoreId = KNSmlAgendaAdapterUid;
    
	FLOG(_L("CNSmlAgendaDataStore::ConstructL: END"));
	}

// -----------------------------------------------------------------------------
// CNSmlAgendaDataStore::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CNSmlAgendaDataStore* CNSmlAgendaDataStore::NewL()
	{
	FLOG(_L("CNSmlAgendaDataStore::NewL: BEGIN"));
	
	CNSmlAgendaDataStore* self = new ( ELeave ) CNSmlAgendaDataStore();
	CleanupStack::PushL( self );
	
	self->ConstructL();
	CleanupStack::Pop(); // self
	
	FLOG(_L("CNSmlAgendaDataStore::NewL: END"));
	return self;
	}

// -----------------------------------------------------------------------------
// CNSmlAgendaDataStore::~CNSmlAgendaDataStore
// Destructor.
// -----------------------------------------------------------------------------
//
CNSmlAgendaDataStore::~CNSmlAgendaDataStore()
	{
	FLOG(_L("CNSmlAgendaDataStore::~CNSmlAgendaDataStore(): BEGIN"));

    // Enable notifications
    TInt error( KErrNone );
   
    // TODO:
    /*if ( iVCalSession )
        {
        TRAP_IGNORE( iVCalSession->EnablePubSubNotificationsL() );
        TRAP_IGNORE( iVCalSession->EnableChangeBroadcast() );
        }*/  
	
    if ( iChangeFinder )
		{
		TRAP( error, iChangeFinder->CloseL() );
		}

    delete iChangeFinder;
	delete iNewUids;
	delete iDeletedUids;
	delete iSoftDeletedUids;
	delete iMovedUids;
	delete iReplacedUids;

    delete iDefaultStoreFileName;
    delete iOpenedStoreName;
    delete iReceivedStoreName;
	delete iDataMod;
	delete iItemData;
	
    delete iInterimUtils;
    
    iAgendaPluginAdapters.ResetAndDestroy();    
    
    if( iOwnFormat )
       {
       delete iOwnFormat;
       iOwnFormat = NULL;
       }
   
    iRfs.Close();
    iStringPool.Close();
    
    if ( iCalOffsetArr )
       {
       iCalOffsetArr->Reset();
       delete iCalOffsetArr;
       }
    
    if ( iCommittedUidArr )
       {
       iCommittedUidArr->Reset();
       delete iCommittedUidArr;
       }
	FLOG(_L("CNSmlAgendaDataStore::~CNSmlAgendaDataStore(): END"));
	}

// -----------------------------------------------------------------------------
// CNSmlAgendaDataStore::DoOpenL
// Open calendar database for access.
// -----------------------------------------------------------------------------
//
void CNSmlAgendaDataStore::DoOpenL( const TDesC& aStoreName,
                MSmlSyncRelationship& aContext, TRequestStatus& aStatus )
    {
	FLOG(_L("CNSmlAgendaDataStore::DoOpenL: BEGIN"));
	
	iCallerStatus = &aStatus;
	*iCallerStatus = KRequestPending;
	if ( iState != ENSmlClosed )
		{
		User::RequestComplete( iCallerStatus, KErrInUse );
		return;
		}

	if( RFs::CharToDrive( aStoreName[0], iDrive ) != KErrNone )
		{
		RFs::CharToDrive( KNSmlDriveC()[0], iDrive );
		}
	
	if ( iReceivedStoreName )
        {
        delete iReceivedStoreName;
        iReceivedStoreName = NULL;
        } 
	iReceivedStoreName = aStoreName.AllocL();
    // Open database
	TInt err( KErrNone );	
	if ( aStoreName == KNSmlAgendaFileNameForDefaultDB )
		{
		FLOG(_L("CNSmlAgendaDataStore::DoOpenL: Calling the OpenStoreL: '%S'"), &aStoreName);
		TRAP( err, OpenStoreL() );
		}
    else 
    	{
    	// Provided profile is created from the DS Application
    	FLOG(_L("CNSmlAgendaDataStore::DoOpenL: storename: '%S'"), &aStoreName);    	
    	if ( iOpenedStoreName )
            {
            delete iOpenedStoreName;
            iOpenedStoreName = NULL;
            }
        iOpenedStoreName = aStoreName.AllocL();  
        
        CCalSession* calsession = CCalSession::NewL();
        CleanupStack::PushL(calsession);
        TRAP( err, calsession->OpenL( aStoreName ) );
        if ( err )
            {
            FLOG(_L("CNSmlAgendaDataStore::DoOpenL: Cannot open the session: '%d'"), err); 
            CleanupStack::PopAndDestroy( calsession ); 
            User::RequestComplete( iCallerStatus, err );
            return;
            }
            
        // Disable notifications
        TRAP_IGNORE( calsession->DisablePubSubNotificationsL() );
        TRAP_IGNORE( calsession->DisableChangeBroadcast() );    
        // Get ID of database
        calsession->FileIdL( iOpenedStoreId );
        
        // Close the session
        CleanupStack::PopAndDestroy( calsession ); 
    	}
		
	if ( err )
	    {
	    User::RequestComplete( iCallerStatus, err );
	    return;
	    }
	
	if ( iChangeFinder )
		{
		iChangeFinder->CloseL();
		delete iChangeFinder;
		iChangeFinder = NULL;
		}
	
	iChangeFinder = CNSmlChangeFinder::NewL( aContext, iKey, iHasHistory,
	                                         KNSmlAgendaAdapterImplUid );
	iAgendaAdapterLog = CNSmlAgendaAdapterLog::NewL( aContext );
	iState = ENSmlOpenAndWaiting;
    User::RequestComplete( iCallerStatus, err );
	    
	FLOG(_L("CNSmlAgendaDataStore::DoOpenL: END"));
    }

// -----------------------------------------------------------------------------
// CNSmlAgendaDataStore::DoCancelRequest
// Set internal module state to previous state.
// -----------------------------------------------------------------------------
//
void CNSmlAgendaDataStore::DoCancelRequest()
    {
	FLOG(_L("CNSmlAgendaDataStore::DoCancelRequest: BEGIN"));
    if ( iState == ENSmlOpenAndWaiting )
        {
    	iState = ENSmlClosed;
        }
    else
        {
	    iState = ENSmlOpenAndWaiting;
        }
	FLOG(_L("CNSmlAgendaDataStore::DoCancelRequest: END"));
    }

// -----------------------------------------------------------------------------
// CNSmlAgendaDataStore::DoStoreName
// Returns previously opened database name.
// -----------------------------------------------------------------------------
//
const TDesC& CNSmlAgendaDataStore::DoStoreName() const
    {
	FLOG(_L("CNSmlAgendaDataStore::DoStoreName: BEGIN"));
	FLOG(_L("CNSmlAgendaDataStore::DoStoreName: END"));
	return *iReceivedStoreName;
    }

// -----------------------------------------------------------------------------
// CNSmlAgendaDataStore::DoBeginTransactionL
// Not supported.
// -----------------------------------------------------------------------------
//
void CNSmlAgendaDataStore::DoBeginTransactionL()
    {
	FLOG(_L("CNSmlAgendaDataStore::DoBeginTransactionL: BEGIN"));
	User::Leave( KErrNotSupported );
	FLOG(_L("CNSmlAgendaDataStore::DoBeginTransactionL: END"));
    }

// -----------------------------------------------------------------------------
// CNSmlAgendaDataStore::DoCommitTransactionL
// Not supported.
// -----------------------------------------------------------------------------
//
void CNSmlAgendaDataStore::DoCommitTransactionL( TRequestStatus& aStatus )
    {
	FLOG(_L("CNSmlAgendaDataStore::DoCommitTransactionL: BEGIN"));
	iCallerStatus = &aStatus;
	*iCallerStatus = KRequestPending;
	User::RequestComplete( iCallerStatus, KErrNotSupported );
	FLOG(_L("CNSmlAgendaDataStore::DoCommitTransactionL: END"));
    }

// -----------------------------------------------------------------------------
// CNSmlAgendaDataStore::DoRevertTransaction
// Not supported.
// -----------------------------------------------------------------------------
//
void CNSmlAgendaDataStore::DoRevertTransaction( TRequestStatus& aStatus )
    {
	FLOG(_L("CNSmlAgendaDataStore::DoRevertTransaction: BEGIN"));
	iCallerStatus = &aStatus;
	*iCallerStatus = KRequestPending;
	User::RequestComplete( iCallerStatus, KErrNotSupported );
	FLOG(_L("CNSmlAgendaDataStore::DoRevertTransaction: END"));
    }

// -----------------------------------------------------------------------------
// CNSmlAgendaDataStore::DoBeginBatchL
// Not supported.
// -----------------------------------------------------------------------------
//
void CNSmlAgendaDataStore::DoBeginBatchL()
    {
	FLOG(_L("CNSmlAgendaDataStore::DoBeginBatchL: BEGIN"));
	User::Leave( KErrNotSupported );
	FLOG(_L("CNSmlAgendaDataStore::DoBeginBatchL: END"));
    }

// -----------------------------------------------------------------------------
// CNSmlAgendaDataStore::DoCommitBatchL
// Not supported.
// -----------------------------------------------------------------------------
//
void CNSmlAgendaDataStore::DoCommitBatchL( RArray<TInt>& /*aResultArray*/,
                TRequestStatus& aStatus )
    {
	FLOG(_L("CNSmlAgendaDataStore::DoCommitBatchL: BEGIN"));
	iCallerStatus = &aStatus;
	*iCallerStatus = KRequestPending;
	User::RequestComplete( iCallerStatus, KErrNotSupported );
	FLOG(_L("CNSmlAgendaDataStore::DoCommitBatchL: END"));
    }

// -----------------------------------------------------------------------------
// CNSmlAgendaDataStore::DoCancelBatch
// Not supported.
// -----------------------------------------------------------------------------
//
void CNSmlAgendaDataStore::DoCancelBatch()
    {
	FLOG(_L("CNSmlAgendaDataStore::DoCancelBatch: BEGIN"));
	// Nothing to do
	FLOG(_L("CNSmlAgendaDataStore::DoCancelBatch: END"));
    }

// -----------------------------------------------------------------------------
// CNSmlAgendaDataStore::DoSetRemoteStoreFormatL
// Set SyncML Remote Server data store format.
// -----------------------------------------------------------------------------
//
void CNSmlAgendaDataStore::DoSetRemoteStoreFormatL(
                const CSmlDataStoreFormat& aServerDataStoreFormat )
    {
	FLOG(_L("CNSmlAgendaDataStore::DoSetRemoteStoreFormatL: BEGIN"));
	
	if ( iOwnFormat )
	    {
	    delete iOwnFormat;
	    iOwnFormat = NULL;    
	    }
	
	iOwnFormat = DoOwnStoreFormatL();
	// RD_MULTICAL
	if ( aServerDataStoreFormat.IsSupported( 
	                                CSmlDataStoreFormat::EOptionHierarchial ) )
        {
        FLOG(_L("CNSmlAgendaDataStore::DoSetRemoteStoreFormatL: Supports HIERARCHIAL"));
       
		
        iIsHierarchicalSyncSupported = ETrue;
	
		// Check what properties are supported for folders
        iAgendaAdapterHandler->CheckServerSupportForFolder( aServerDataStoreFormat );
        }
	else
	    {
	    FLOG(_L("CNSmlAgendaDataStore::DoSetRemoteStoreFormatL: Do not support HIERARCHIAL"));	           
	    iIsHierarchicalSyncSupported = EFalse;
	    }
	// RD_MULTICAL       
	iDataMod->SetOwnStoreFormat( *iOwnFormat );
	
    iDataMod->SetPartnerStoreFormat( ( CSmlDataStoreFormat& )
                aServerDataStoreFormat );
    
    // Check which calendar type (vCal/iCal) is used
    // If server supports iCal then it is used
    // Otherwise vCal is used
    TInt returnValue( KErrNotSupported );

#ifdef __NSML_USE_ICAL_FEATURE

    FLOG(_L("CNSmlAgendaDataStore::DoSetRemoteStoreFormatL: Support iCal"));
   	returnValue = iDataMod->SetUsedMimeType(
       	    iOwnFormat->MimeFormat( 1 ).MimeType(),
           	iOwnFormat->MimeFormat( 1 ).MimeVersion() );

#endif // __NSML_USE_ICAL_FEATURE

    if ( returnValue == KErrNone )
        {
        FLOG(_L("CNSmlAgendaDataStore::DoSetRemoteStoreFormatL: Sets iCal"));
        iRXEntryType = ENSmlICal;
        iTXEntryType = ENSmlICal;
        }
    else
        {
        FLOG(_L("CNSmlAgendaDataStore::DoSetRemoteStoreFormatL: Support vCal"));
        returnValue = iDataMod->SetUsedMimeType(
            iOwnFormat->MimeFormat( 0 ).MimeType(),
            iOwnFormat->MimeFormat( 0 ).MimeVersion() );
        if ( returnValue == KErrNone )
            {
            FLOG(_L("CNSmlAgendaDataStore::DoSetRemoteStoreFormatL: Sets vCal"));
            iRXEntryType = ENSmlVCal;
            iTXEntryType = ENSmlVCal;
            }
        }
    if ( iRXEntryType == ENSmlNotSet || iTXEntryType == ENSmlNotSet )
        {
        // Leave if server does not support either vCal or iCal
        FLOG(_L("CNSmlAgendaDataStore::DoSetRemoteStoreFormatL: MimeType Not supported"));
        User::Leave( KErrNotFound );        
        }
    
    // RD_MULTICAL
    if( !iSnapshotRegistered )
        {
        RegisterSnapshotL();
        }
    // RD_MULTICAL
    
	FLOG(_L("CNSmlAgendaDataStore::DoSetRemoteStoreFormatL: END"));
    }

// -----------------------------------------------------------------------------
// CNSmlAgendaDataStore::DoSetRemoteMaxObjectSize
// Set SyncML Remote Server maximum object size.
// -----------------------------------------------------------------------------
//
void CNSmlAgendaDataStore::DoSetRemoteMaxObjectSize( TInt aServerMaxObjectSize )
    {
	FLOG(_L("CNSmlAgendaDataStore::DoSetRemoteMaxObjectSize: BEGIN"));
	iServerMaxObjectSize = aServerMaxObjectSize;
	FLOG(_L("CNSmlAgendaDataStore::DoSetRemoteMaxObjectSize: END"));
    }

// -----------------------------------------------------------------------------
// CNSmlAgendaDataStore::DoMaxObjectSize
// Set SyncML Component maximum object size.
// -----------------------------------------------------------------------------
//
TInt CNSmlAgendaDataStore::DoMaxObjectSize() const
    {
	FLOG(_L("CNSmlAgendaDataStore::DoMaxObjectSize: BEGIN"));
	FLOG(_L("CNSmlAgendaDataStore::DoMaxObjectSize - Default: END"));
	return KNSmlAgendaOwnMaxObjectSize;
    }

// -----------------------------------------------------------------------------
// CNSmlAgendaDataStore::DoOpenItemL
// Open calendar item for reading.
// -----------------------------------------------------------------------------
//
void CNSmlAgendaDataStore::DoOpenItemL( TSmlDbItemUid aUid,
                                        TBool& aFieldChange,
                                        TInt& aSize,
                                        TSmlDbItemUid& aParent,
                                        TDes8& aMimeType,
                                        TDes8& aMimeVer,
                                        TRequestStatus& aStatus )
    {
	FLOG(_L("CNSmlAgendaDataStore::DoOpenItemL: BEGIN"));
    iCallerStatus = &aStatus;
	*iCallerStatus = KRequestPending;
	// Leave if Data Store is in wrong state
	if ( iState != ENSmlOpenAndWaiting )
		{
		FLOG(_L("CNSmlAgendaDataStore::DoOpenItemL:Returing due to invalid state"));
		User::RequestComplete( iCallerStatus, KErrNotReady );
		return;
		}
		
	// Add field change info
	aFieldChange = EFalse;
	
	// Update the SnapShots
	if ( !iSnapshotRegistered )
        {
        RegisterSnapshotL();
        }
        
	// Initialize the Buffer
    delete iItemData;
    iItemData = NULL;
    iItemData = CBufFlat::NewL( KNSmlItemDataExpandSize );

    RBufWriteStream writeStream( *iItemData );
    writeStream.PushL();

	// RD_MULTICAL
  	//Determine the Mime Type
    DataMimeType( aUid );
	TInt fetchError( KErrNone );
	switch( iDataMimeType )
	    {
	    case ENSmlFolder:
    	    {
    	    //Set the Parent UID
    	    aParent = KDbItemUidRoot; // parent is root
    	    FLOG(_L("CNSmlAgendaDataStore::DoOpenItemL: calling fetch"));
	        FLOG(_L("CNSmlAgendaDataStore::DoOpenItemL:writestream before size: '%d'"), writeStream.Sink()->SizeL());
	        TRAP( fetchError, iAgendaAdapterHandler->FetchFolderL( aUid, writeStream ) );
	        FLOG(_L("CNSmlAgendaDataStore::DoOpenItemL:writestream size: '%d'"), writeStream.Sink()->SizeL());
    	
	        if(fetchError != KErrNone)
    	        {
    	        FLOG(_L("CNSmlAgendaDataStore::DoOpenItemL: fetch error: '%d'"), fetchError);
    	        CleanupStack::PopAndDestroy( ); // writeStream
    	        User::RequestComplete( iCallerStatus, fetchError );
    	        return;
    	        }
    	    
            aMimeType.Append( KNSmlContentTypeFolder );
            aMimeVer.Append( KNSmlContentTypeFolderVersion );
            
            writeStream.CommitL();
            iItemData->Compress();
            iPos = 0;
            }
    	    break;
	    case ENSmlCalendar:
    	    {
    	    FLOG(_L("CNSmlAgendaDataStore::DoOpenItemL: calendar item"));
            CNSmlAgendaDataStoreUtil* agendautil = NULL;
            HBufC* calfilename = NULL;
            TCalLocalUid parentid(NULL);
            TCalLocalUid entryid(aUid);
            TInt err( KErrNone );
            
            if( iIsHierarchicalSyncSupported )
                {
                TRAP( err, GetCalendarEntryIdL( parentid, entryid ));
                if ( err )
                    {
                    FLOG(_L("CNSmlAgendaDataStore::DoOpenItemL:Parent Id is not Valid one"));
                    User::RequestComplete( iCallerStatus, KErrNotFound );
                    return;
                    }
                aParent = parentid;
                //Get the Folder Name
                TRAP(err, calfilename = iAgendaAdapterHandler->FolderNameL( parentid ));  
                }
            else
                {
                calfilename = iOpenedStoreName->AllocL();
                }
            
            if( err != KErrNone || NULL == calfilename )
                {
                FLOG(_L("CNSmlAgendaDataStore::DoOpenItemL:Invalid CalendarInfo"));
                User::RequestComplete( iCallerStatus, KErrNotFound );
                return;
                }     
            
            iReplaceItemId = entryid;            
            agendautil = CNSmlAgendaDataStoreUtil::NewL();
            if( agendautil )
                {
                CleanupStack::PushL(agendautil);
                TRAP(err, agendautil->InitializeCalAPIsL( calfilename, entryid ));
                CleanupStack::Pop(agendautil);
                }
            if ( err || !agendautil )
                {
                FLOG(_L("CNSmlAgendaDataStore::DoOpenItemL: entry is not valid"));
                delete agendautil; 
                delete calfilename;
                User::RequestComplete( iCallerStatus, KErrNotFound );
                return;
                }    
            // Export item from database
            if ( iTXEntryType == ENSmlICal )
                {
                FLOG(_L("CNSmlAgendaDataStore::DoOpenItemL: Export - iCal DB"));
                agendautil->iExporter->ExportICalL( *agendautil->iEntry, writeStream );
                aMimeType = iOwnFormat->MimeFormat( 1 ).MimeType().DesC();
                aMimeVer = iOwnFormat->MimeFormat( 1 ).MimeVersion().DesC();
                }
            else if ( iTXEntryType == ENSmlVCal )
                {
                FLOG(_L("CNSmlAgendaDataStore::DoOpenItemL: Export - vCal DB"));
                agendautil->iExporter->ExportVCalL( *agendautil->iEntry, writeStream );
                aMimeType = iOwnFormat->MimeFormat( 0 ).MimeType().DesC();
                aMimeVer = iOwnFormat->MimeFormat( 0 ).MimeVersion().DesC();
                }
            else
                {
                FLOG(_L("CNSmlAgendaDataStore::DoOpenItemL: Export - DB Not Supported"));
                delete agendautil;
                delete calfilename;
                CleanupStack::PopAndDestroy( &writeStream ); 
                User::RequestComplete( iCallerStatus, KErrNotSupported );
                return;
                }
            writeStream.CommitL();
            iItemData->Compress();
            iPos = 0;
     
            iDataMod->StripTxL( *iItemData );
            delete agendautil;
            delete calfilename;
            }
    	    break;
	    default:
    	    {
    	    iDataMimeType = ENSmlNone;
    	    CleanupStack::PopAndDestroy( ); // writeStream
    	    User::RequestComplete( iCallerStatus, KErrNotFound );
    	    return;
    	    }
	    }
    
	CleanupStack::PopAndDestroy( ); // writeStream
	
	FLOG(_L("CNSmlAgendaDataStore::DoOpenItemL: destroying the stream"));
	
	// Set the Size
	aSize = iItemData->Size();
	
	iState = ENSmlItemOpen;
	
	FLOG(_L("CNSmlAgendaDataStore::DoOpenItemL: user complete"));
	
	if ( iServerMaxObjectSize == 0 || aSize <= iServerMaxObjectSize )
		{
		FLOG(_L("CNSmlAgendaDataStore::DoOpenItemL: error none"));
		User::RequestComplete( iCallerStatus, KErrNone );
		}
	else
		{
		FLOG(_L("CNSmlAgendaDataStore::DoOpenItemL: error too big"));
		User::RequestComplete( iCallerStatus, KErrTooBig );
		}
	FLOG(_L("CNSmlAgendaDataStore::DoOpenItemL: END"));
    }

// -----------------------------------------------------------------------------
// CNSmlAgendaDataStore::DoCreateItemL
// Prepare item data for writing to database. WriteItemL() writes item's data as
// buffered.
// -----------------------------------------------------------------------------
//
void CNSmlAgendaDataStore::DoCreateItemL( TSmlDbItemUid& aUid,
                                          TInt aSize,
                                          TSmlDbItemUid aParent,
                                          const TDesC8& aMimeType,
                                          const TDesC8& /*aMimeVer*/,
                                          TRequestStatus& aStatus )
    {
	FLOG(_L("CNSmlAgendaDataStore::DoCreateItemL: BEGIN"));
	FLOG(_L("CNSmlAgendaDataStore::DoCreateItemL: Parent id: '%d'"), aParent);
	FLOG(_L("CNSmlAgendaDataStore::DoCreateItemL: UID: '%d'"), aUid);
	iCallerStatus = &aStatus;
	*iCallerStatus = KRequestPending;
	iAddItemId = &aUid;
	
    //Leave if Data Store is in wrong state
    if ( iState != ENSmlOpenAndWaiting )
        {
        User::RequestComplete( iCallerStatus, KErrNotReady );
        FLOG(_L("CNSmlAgendaDataStore::DoCreateItemL - KErrNotReady: END"));
        return;
        }

    // Leave if item is larger than we support
    if ( KNSmlAgendaOwnMaxObjectSize < aSize )
        {
        User::RequestComplete( iCallerStatus, KErrTooBig );
        FLOG(_L("CNSmlAgendaDataStore::DoCreateItemL - KErrTooBig: END"));
        return;
        }
        
    // Check the drive free space
    if( SysUtil::DiskSpaceBelowCriticalLevelL( &iRfs, aSize, iDrive ) )
        {
        User::RequestComplete( iCallerStatus, KErrDiskFull );
        FLOG(_L("CNSmlAgendaDataStore::DoCreateItemL - KErrDiskFull: END"));
        return;
        }
    
    // Check the MIME type
    if ( 0 == iOwnFormat->MimeFormat( 0 ).MimeType().DesC().Compare( aMimeType ) )
        {
        // vcal
        FLOG(_L("CNSmlAgendaDataStore::DoCreateItemL: received vCal"));
        iRXEntryType = ENSmlVCal;
        iDataMimeType = ENSmlCalendar;
        
        TInt err( KErrNone );
        // Check the Sync Status
        if( iIsHierarchicalSyncSupported )
            {
            if( !iAgendaAdapterHandler->FolderSyncStatusL( aParent ) )
                {
                User::RequestComplete( iCallerStatus, KErrGeneral );
                FLOG(_L("CNSmlAgendaDataStore::DoCreateItemL - Sync Disabled: END"));
                return;
                }
            }
        else
            {
            TBool syncstatus( EFalse );
            HBufC* calfilename = iOpenedStoreName->AllocL();
            TRAP( err, syncstatus = iAgendaAdapterHandler->FolderSyncStatusL( calfilename ) );
            if( err || !syncstatus )
                {
                delete calfilename;
                User::RequestComplete( iCallerStatus, KErrGeneral );
                FLOG(_L("CNSmlAgendaDataStore::DoCreateItemL - Sync Disabled: END"));
                return;
                }
            delete calfilename;            
            }
        }
#ifdef __NSML_USE_ICAL_FEATURE     
    else if ( 0 == iOwnFormat->MimeFormat( 1 ).MimeType().DesC().Compare( aMimeType ) ) 
        {
        //ical
        FLOG(_L("CNSmlAgendaDataStore::DoCreateItemL: received iCal"));
        iRXEntryType = ENSmlICal;
        iDataMimeType = ENSmlCalendar;
        }
#endif // __NSML_USE_ICAL_FEATURE
    // RD_MULTICAL
    else if( aMimeType == KNSmlContentTypeFolder() && iIsHierarchicalSyncSupported )
        {
        // Check the validity 
        /*if ( aUid <= 0 || aParent <= 0 )
          {
          FLOG(_L("CNSmlAgendaDataStore::DoCreateItemL: Corrupt ids"));
          User::RequestComplete( iCallerStatus, KErrCorrupt );
          return;
          }*/
        iDataMimeType = ENSmlFolder;
        }
    // RD_MULTICAL
    else
        {
        FLOG(_L("CNSmlAgendaDataStore::DoCreateItemL: \
                               mime type not received"));
        iRXEntryType = iTXEntryType;
        iDataMimeType = ENSmlNone;
        User::RequestComplete( iCallerStatus, KErrNotSupported );
        FLOG(_L("CNSmlAgendaDataStore::DoCreateItemL: end with leave"));
        return;
        }
    
	if( iIsHierarchicalSyncSupported )
		{	
	    // Check that parent exists
	    iParentItemId = aParent;
	    if ( iParentItemId == KDbItemUidRoot 
	         && iDataMimeType == ENSmlCalendar )
	        {
	        FLOG(_L("CNSmlAgendaDataStore::DoCreateItemL: ParentId is NULL"));
	        iParentItemId = NULL;
	        User::RequestComplete( iCallerStatus, KErrGeneral );
	        return;
	        }
	  
	    TInt index = KErrNotFound;
	    TInt err = KErrNone;
	    TKeyArrayFix key( 0, ECmpTInt ); // Find key for Ids.
	    
	    err = iCalOffsetArr->Find( iParentItemId, key, index );
	 
	    if( err != KErrNone && iDataMimeType == ENSmlCalendar )
	        {
	        FLOG(_L("CNSmlAgendaDataStore::DoCreateItemL: ParentId is NotFound"));
	        iParentItemId = NULL;
	        User::RequestComplete( iCallerStatus, KErrNotFound );
	        return;
	        }
		}
    
    // TODO: Any other validation is required ?
    
    // Register snapshots if needed
    if( !iSnapshotRegistered )
        {
        RegisterSnapshotL();
        }
    
    // Buffer is reinitialized
    delete iItemData;
    iItemData = NULL;
    iItemData = CBufFlat::NewL( KNSmlItemDataExpandSize );
    iPos = 0;
    
    iState = ENSmlItemCreating;
    User::RequestComplete( iCallerStatus, KErrNone );

	FLOG(_L("CNSmlAgendaDataStore::DoCreateItemL: END"));
    }

// -----------------------------------------------------------------------------
// CNSmlAgendaDataStore::DoReplaceItemL
// Prepare item data for writing and replacing item in database. WriteItemL()
// writes item's data as buffered.
// -----------------------------------------------------------------------------
//
void CNSmlAgendaDataStore::DoReplaceItemL( TSmlDbItemUid aUid,
                                           TInt aSize,
                                           TSmlDbItemUid aParent,
                                           TBool aFieldChange,
                                           TRequestStatus& aStatus )
    {
	FLOG(_L("CNSmlAgendaDataStore::DoReplaceItemL: BEGIN"));
	FLOG(_L("CNSmlAgendaDataStore::DoReplaceItemL: Parent id: '%d'"), aParent);
	FLOG(_L("CNSmlAgendaDataStore::DoReplaceItemL: UID: '%d'"), aUid);
	
	iCallerStatus = &aStatus;
	*iCallerStatus = KRequestPending;
	TInt err( KErrNone );
	
    // Leave if Data Store is in wrong state
    if ( iState != ENSmlOpenAndWaiting )
        {
        User::RequestComplete( iCallerStatus, KErrNotReady );
        FLOG(_L("CNSmlAgendaDataStore::DoReplaceItemL - KErrNotReady: END"));
        return;
        }

    // Leave if item is larger than we support
    if ( KNSmlAgendaOwnMaxObjectSize < aSize )
        {
        User::RequestComplete( iCallerStatus, KErrTooBig );
        FLOG(_L("CNSmlAgendaDataStore::DoReplaceItemL - KErrTooBig: END"));
        return;
        }

    // This adapter does not support Field Level sync
    if ( aFieldChange )
        {
        User::RequestComplete( iCallerStatus, KErrNotSupported );
        FLOG(_L("CNSmlAgendaDataStore::DoReplaceItemL \
                   - KErrNotSupported: END"));
        return;
        }
    
    // Check the drive free space
    if ( SysUtil::DiskSpaceBelowCriticalLevelL( &iRfs, aSize, iDrive ) )
        {
        User::RequestComplete( iCallerStatus, KErrDiskFull );
        FLOG(_L("CNSmlAgendaDataStore::DoReplaceItemL - KErrDiskFull: END"));
        return;
        }
    
    // Check the Sync Status
    if( iIsHierarchicalSyncSupported )
        {
        TCalLocalUid parentid( aParent );
        TCalLocalUid entryid( aUid );
        
        TRAP( err, GetCalendarEntryIdL( parentid, entryid ));
        if ( err )
            {
            FLOG(_L("CNSmlAgendaDataStore::DoReplaceItemL: Invalid UID"));
            User::RequestComplete( iCallerStatus, KErrGeneral );
            FLOG(_L("CNSmlAgendaDataStore::DoReplaceItemL - Sync Disabled: END"));
            return;
            }         
        
        if( !iAgendaAdapterHandler->FolderSyncStatusL( parentid ) )
            {
            User::RequestComplete( iCallerStatus, KErrGeneral );
            FLOG(_L("CNSmlAgendaDataStore::DoReplaceItemL - Sync Disabled: END"));
            return;
            }
        }
    else
        {
        TBool syncstatus( EFalse );
        HBufC* calfilename = iOpenedStoreName->AllocL();
        TRAP( err, syncstatus = iAgendaAdapterHandler->FolderSyncStatusL( calfilename ) );
        if( err || !syncstatus )
            {
            delete calfilename;
            User::RequestComplete( iCallerStatus, KErrGeneral );
            FLOG(_L("CNSmlAgendaDataStore::DoReplaceItemL - Sync Disabled: END"));
            return;
            }
        delete calfilename;            
        }
    
    // Determine the Mime Type
    DataMimeType( aUid ); 
    switch( iDataMimeType )
        {
        case ENSmlFolder:
            {
            iReplaceItemId = aUid;
            }
            break;
        case ENSmlCalendar:
            {
            CNSmlAgendaDataStoreUtil* agendautil = NULL;
            HBufC* calfilename = NULL;
            iParentItemId = aParent;
            
            if( iIsHierarchicalSyncSupported )
                {
                iReplaceItemId = aUid - aParent;
                //Get the Folder Information
                TRAP(err, calfilename = iAgendaAdapterHandler->FolderNameL( aParent ));
                }
            else
                {
                iReplaceItemId = aUid;
                calfilename = iOpenedStoreName->AllocL();
                }
                
            if( err != KErrNone || NULL == calfilename )
                {
                FLOG(_L("CNSmlAgendaDataStore::DoReplaceItemL:Invalid CalendarInfo"));
                User::RequestComplete( iCallerStatus, KErrNotFound );
                return;
                }
            
            FLOG(_L("CNSmlAgendaDataStore::DoReplaceItemL: entry id to be fetched: '%d'"), iReplaceItemId);             
            agendautil = CNSmlAgendaDataStoreUtil::NewL();
            if( agendautil )
                {
                CleanupStack::PushL(agendautil);
                TRAP(err, agendautil->InitializeCalAPIsL( calfilename, iReplaceItemId ));
                CleanupStack::Pop(agendautil);
                }
                
            if ( err || !agendautil )
                {
                FLOG(_L("CNSmlAgendaDataStore::DoReplaceItemL: entry is not valid"));  
                delete agendautil;
                delete calfilename;             
                User::RequestComplete( iCallerStatus, KErrNotFound );               
                return;
                }
            delete agendautil;
            delete calfilename;  
            }
            break;
        default:
            {
            iDataMimeType = ENSmlNone;
            User::RequestComplete( iCallerStatus, KErrNotSupported );
            FLOG(_L("CNSmlAgendaDataStore::DoReplaceItemL \
                       - KErrNotSupported: END"));
            return;
            }
        }
    
    if ( !iSnapshotRegistered )
        {
        RegisterSnapshotL();
        }
   
    //Reinitialize the Buffer
    delete iItemData;
    iItemData = NULL;
    iItemData = CBufFlat::NewL( KNSmlItemDataExpandSize );
    iPos = 0;
    
    iState = ENSmlItemUpdating;
    User::RequestComplete( iCallerStatus, KErrNone );
	
	FLOG(_L("CNSmlAgendaDataStore::DoReplaceItemL: END"));
    }

// -----------------------------------------------------------------------------
// CNSmlAgendaDataStore::DoReadItemL
// Read item data to given buffer.
// -----------------------------------------------------------------------------
//
void CNSmlAgendaDataStore::DoReadItemL( TDes8& aBuffer )
    {
	FLOG(_L("CNSmlAgendaDataStore::DoReadItemL: BEGIN"));
    if ( iState != ENSmlItemOpen || !iItemData )
        {
        iPos = -1;
        User::Leave( KErrNotReady );
        }

    if ( iPos == -1 )
        {
        User::Leave( KErrEof );
        }

    if ( aBuffer.Size() < iItemData->Size() - iPos )
        {
        iItemData->Read( iPos, aBuffer );
        iPos += aBuffer.Size();
        }
    else
        {
        iItemData->Read( iPos, aBuffer, iItemData->Size() - iPos );
        iPos = -1;
        }
	FLOG(_L("CNSmlAgendaDataStore::DoReadItemL: END"));
    }

// -----------------------------------------------------------------------------
// CNSmlAgendaDataStore::DoWriteItemL
// Write item data as buffered.
// -----------------------------------------------------------------------------
//
void CNSmlAgendaDataStore::DoWriteItemL( const TDesC8& aData )
    {
	FLOG(_L("CNSmlAgendaDataStore::DoWriteItemL: BEGIN"));
	if ( iState == ENSmlItemCreating || iState == ENSmlItemUpdating )
		{
		if ( iItemData )
			{
			if ( iPos == -1 )
				{
				User::Leave( KErrEof );
				}
			iItemData->InsertL( iPos, aData );
			iPos += aData.Size();
			return;
			}
		}
	User::Leave( KErrNotReady );
	FLOG(_L("CNSmlAgendaDataStore::DoWriteItemL: END"));
    }

// -----------------------------------------------------------------------------
// CNSmlAgendaDataStore::DoCommitItemL
// Commit item data to database when adding or replacing item.
// -----------------------------------------------------------------------------
//
void CNSmlAgendaDataStore::DoCommitItemL( TRequestStatus& aStatus )
    {
    FLOG(_L("CNSmlAgendaDataStore::DoCommitItemL: BEGIN"));
    iCallerStatus = &aStatus;
    *iCallerStatus = KRequestPending;
    
    // Leave if Data Store is in wrong state
    if ( iState != ENSmlItemCreating && iState != ENSmlItemUpdating )
        {
        User::RequestComplete( iCallerStatus, KErrNotReady );
        FLOG(_L("CNSmlAgendaDataStore::DoCommitItemL - KErrNotReady: END"));
        return;
        }
    
    iItemData->Compress();
    TInt error( KErrNone );
    
    // RD_MULTICAL
    switch(iDataMimeType)
        {
        case ENSmlCalendar:
            {
            if( iState == ENSmlItemCreating )
                {
                TRAP( error, DoCommitCreateCalItemL() );
                }
            else
                {
                TRAP( error, DoCommitReplaceCalItemL() );
                }
            }
            break;
        case ENSmlFolder:
            {
            if( iState == ENSmlItemCreating )
                {
                TRAP( error, DoCommitCreateFolderItemL() );
                }
            else
                {
                TRAP( error, DoCommitReplaceFolderItemL() );
                }
            }
            break;
        default:
            {
            FLOG(_L("CNSmlAgendaDataStore::DoCommitCreateItemL - \
                                           KErrNotSupported: END"));
            User::Leave( KErrNotSupported );
            }                
        }
    // RD_MULTICAL
    
    iReplaceItemId = -1;
    iPos = -1;
    iState = ENSmlOpenAndWaiting;
    iRXEntryType = iTXEntryType;
    User::RequestComplete( iCallerStatus, error );    
    FLOG(_L("CNSmlAgendaDataStore::DoCommitItemL: END"));
    }

// -----------------------------------------------------------------------------
// CNSmlAgendaDataStore::DoCloseItem
// Return to previous state and clean item buffer.
// -----------------------------------------------------------------------------
//
void CNSmlAgendaDataStore::DoCloseItem()
    {
	FLOG(_L("CNSmlAgendaDataStore::DoCloseItem: BEGIN"));
	if ( iState == ENSmlItemOpen )
		{
		iPos = -1;
		iState = ENSmlOpenAndWaiting;
		}
	FLOG(_L("CNSmlAgendaDataStore::DoCloseItem: END"));
    }

// -----------------------------------------------------------------------------
// CNSmlAgendaDataStore::DoMoveItemL
// Not supported.
// -----------------------------------------------------------------------------
//
void CNSmlAgendaDataStore::DoMoveItemL( TSmlDbItemUid /*aUid*/,
                                        TSmlDbItemUid /*aNewParent*/,
                                        TRequestStatus& aStatus )
    {
	FLOG(_L("CNSmlAgendaDataStore::DoMoveItemL: BEGIN"));
	
    iCallerStatus = &aStatus;
    *iCallerStatus = KRequestPending;
    User::RequestComplete( iCallerStatus, KErrNotSupported );
	    
	FLOG(_L("CNSmlAgendaDataStore::DoMoveItemL: END"));
    }

// -----------------------------------------------------------------------------
// CNSmlAgendaDataStore::DoDeleteItemL
// Delete item from database.
// -----------------------------------------------------------------------------
//
void CNSmlAgendaDataStore::DoDeleteItemL( TSmlDbItemUid aUid,
                                          TRequestStatus& aStatus )
    {
	FLOG(_L("CNSmlAgendaDataStore::DoDeleteItemL: BEGIN"));
	FLOG(_L("CNSmlAgendaDataStore::DoDeleteItemL: aUid: '%d'"), aUid);
	
	TInt err(KErrNone);
	iCallerStatus = &aStatus;
    *iCallerStatus = KRequestPending;
    
    // Leave is Data Store is in wrong state
    if ( iState != ENSmlOpenAndWaiting )
        {
        User::RequestComplete( iCallerStatus, KErrNotReady );
        return;
        }
    
    //Check validity of UID
    if ( aUid <= 0 )
        {
        User::RequestComplete( iCallerStatus, KErrCorrupt );
        return;
        }

    // Check the Sync Status
    if( iIsHierarchicalSyncSupported )
        {
        TCalLocalUid parentid( NULL );
        TCalLocalUid entryid( aUid );
       
        TRAP( err, GetCalendarEntryIdL( parentid, entryid ));
        if ( err )
            {
            FLOG(_L("CNSmlAgendaDataStore::DoDeleteItemL: Invalid UID"));
            User::RequestComplete( iCallerStatus, KErrGeneral );
            FLOG(_L("CNSmlAgendaDataStore::DoDeleteItemL - Sync Disabled: END"));
            return;
            }    
        
        if( !iAgendaAdapterHandler->FolderSyncStatusL( parentid ) )
            {
            User::RequestComplete( iCallerStatus, KErrGeneral );
            FLOG(_L("CNSmlAgendaDataStore::DoDeleteItemL - Sync Disabled: END"));
            return;
            }
        }
     else
        {
        TBool syncstatus( EFalse );
        HBufC* calfilename = iOpenedStoreName->AllocL();
        TRAP( err, syncstatus = iAgendaAdapterHandler->FolderSyncStatusL( calfilename ) );
        if( err || !syncstatus )
            {
            delete calfilename;
            User::RequestComplete( iCallerStatus, KErrGeneral );
            FLOG(_L("CNSmlAgendaDataStore::DoReplaceItemL - Sync Disabled: END"));
            return;
            }
        delete calfilename;            
        }
     
    //Determine the Mime Type
    DataMimeType( aUid );
    switch( iDataMimeType )
        {
        case ENSmlFolder:
            {        
            HBufC* calfilename = NULL;
            //Get the Calendar information
            TRAP(err, calfilename = iAgendaAdapterHandler->FolderNameL(aUid) );
            if( err != KErrNone || NULL == calfilename )
                {
                FLOG(_L("CNSmlAgendaDataStore::DoDeleteItemL:Invalid CalendarInfo"));
                User::RequestComplete( iCallerStatus, KErrNotFound );
                return;
                }
            
            FLOG(_L("CNSmlAgendaDataStore::DoDeleteItemL: name exists"));
            CCalSession* vCalSession = CCalSession::NewL();
            CleanupStack::PushL(vCalSession);
            vCalSession->OpenL(calfilename->Des());
            FLOG(_L("CNSmlAgendaDataStore::DoDeleteItemL: before deleting"));
            TRAP(err, vCalSession->DeleteCalFileL(calfilename->Des()));
            FLOG(_L("CNSmlAgendaDataStore::DoDeleteItemL: after deleting err: '%d'"), err);
            if( err == KErrInUse )
                {
                // Delete all the entries associated with this CalFile
                CNSmlAgendaDataStoreUtil* agendautil = NULL;
                TBuf8<KBuffLength> keyBuff;
                TInt aNumSuccessfulDeleted( 0 );
                RArray<TCalLocalUid> uidArray;
                CleanupClosePushL( uidArray );
                TCalTime zeroTime;
                zeroTime.SetTimeUtcL( Time::NullTTime() );
                
                agendautil = CNSmlAgendaDataStoreUtil::NewL();
                if( agendautil )
                    {
                    CleanupStack::PushL(agendautil);
                    TRAP(err, agendautil->InitializeCalAPIsL( calfilename ));
                    CleanupStack::Pop(agendautil);
                    }                      
                if ( err || !agendautil )
                    {
                    FLOG(_L("CNSmlAgendaDataStore::DoDeleteItemL:Invalid CalendarInfo"));
                    delete calfilename;
                    User::RequestComplete( iCallerStatus, KErrNotFound );
                    return;
                    }            
                agendautil->iEntryView->GetIdsModifiedSinceDateL( zeroTime, uidArray );
                // ... and then delete them
                agendautil->iEntryView->DeleteL( uidArray, aNumSuccessfulDeleted );
                CleanupStack::PopAndDestroy( &uidArray ); // uidArray
                delete agendautil;    
                
                // Get the CalFile
                CCalCalendarInfo* caleninfo = vCalSession->CalendarInfoL();
                CleanupStack::PushL(caleninfo);
                
                // Mark the CalFile as Hidden
                caleninfo->SetEnabled( EFalse );
                            
                // Set the SyncStatus to False
                keyBuff.Zero();
                keyBuff.AppendNum( ESyncStatus );
                TBool syncstatus( EFalse );
                TPckgC<TBool> pckgSyncStatusValue( syncstatus );
                caleninfo->SetPropertyL( keyBuff, pckgSyncStatusValue );
                
                // Mark the meta property as SoftDeleted
                keyBuff.Zero();
                keyBuff.AppendNum(EMarkAsDelete);
                TPckgC<TBool> pkgSoftDelete( ETrue );
                caleninfo->SetPropertyL(keyBuff, pkgSoftDelete);
                
                vCalSession->SetCalendarInfoL( *caleninfo );
                CleanupStack::PopAndDestroy(caleninfo);            
                }
            else if( err != KErrNone )
                {
                CleanupStack::PopAndDestroy(vCalSession);
                delete calfilename;
                User::RequestComplete( iCallerStatus, KErrGeneral );
                return;
                }
            CleanupStack::PopAndDestroy(vCalSession);            
            delete calfilename;  
            
            //Update the array
            TInt index = KErrNotFound;
            TInt err = KErrNone;
            TKeyArrayFix key( 0, ECmpTInt ); // Find key for Ids.
       
            err = iCalOffsetArr->Find( aUid, key, index );
                  
            if( err == KErrNone )
                {
                iCalOffsetArr->Delete(index);
                FLOG(_L("CNSmlAgendaDataStore::DoDeleteItemL: updated the array"));
                }
            }
            break;
        case ENSmlCalendar:
            {
            CNSmlAgendaDataStoreUtil* agendautil = NULL;
            HBufC* calfilename = NULL;
            TCalLocalUid parentid(NULL);
            TCalLocalUid entryid(aUid);
            TInt err( KErrNone );
            
            if( iIsHierarchicalSyncSupported )
                {
                TRAP( err, GetCalendarEntryIdL( parentid, entryid ));
                if ( err )
                    {
                    FLOG(_L("CNSmlAgendaDataStore::DoDeleteItemL:Parent Id is not Valid one"));
                    User::RequestComplete( iCallerStatus, KErrNotFound );
                    return;
                    }
                //Get the Folder Information
                TRAP(err, calfilename = iAgendaAdapterHandler->FolderNameL(parentid) );
                }
            else
                {
                calfilename = iOpenedStoreName->AllocL();
                }
                
            if( err != KErrNone || NULL == calfilename )
                {
                FLOG(_L("CNSmlAgendaDataStore::DoDeleteItemL:Invalid CalendarInfo"));
                User::RequestComplete( iCallerStatus, KErrNotFound );
                return;
                }
            
            agendautil = CNSmlAgendaDataStoreUtil::NewL();
            if( agendautil )
                {
                CleanupStack::PushL(agendautil);
                TRAP(err, agendautil->InitializeCalAPIsL( calfilename, entryid ));
                CleanupStack::Pop(agendautil);
                }                      
            if ( err || !agendautil )
                {
                FLOG(_L("CNSmlAgendaDataStore::DoOpenItemL: entry is not valid"));
                delete agendautil;
                delete calfilename;
                User::RequestComplete( iCallerStatus, KErrNotFound );
                return;
                }                
            agendautil->iEntryView->DeleteL( *agendautil->iEntry );
            delete agendautil;
            delete calfilename;
            }
            break;
        default:
            break;
        }
    
    //Update the Snapshots
    if ( iChangeFinder )
        {
        TNSmlSnapshotItem item( aUid );
        iChangeFinder->ItemDeleted( item );        
        }
        
    User::RequestComplete( iCallerStatus, KErrNone );
	    
	FLOG(_L("CNSmlAgendaDataStore::DoDeleteItemL: END"));
    }

// -----------------------------------------------------------------------------
// CNSmlAgendaDataStore::DoSoftDeleteItemL
// Delete item from database.
// -----------------------------------------------------------------------------
//
void CNSmlAgendaDataStore::DoSoftDeleteItemL( TSmlDbItemUid /*aUid*/,
                                              TRequestStatus& aStatus )
    {
	FLOG(_L("CNSmlAgendaDataStore::DoSoftDeleteItemL: BEGIN"));
	iCallerStatus = &aStatus;
	*iCallerStatus = KRequestPending;
	User::RequestComplete( iCallerStatus, KErrNotSupported );
	FLOG(_L("CNSmlAgendaDataStore::DoSoftDeleteItemL: END"));
    }

// -----------------------------------------------------------------------------
// CNSmlAgendaDataStore::DoDeleteAllItemsL
// Delete all items from database.
// -----------------------------------------------------------------------------
//
void CNSmlAgendaDataStore::DoDeleteAllItemsL( TRequestStatus& aStatus )
    {
    FLOG(_L("CNSmlAgendaDataStore::DoDeleteAllItemsL: BEGIN"));
    iCallerStatus = &aStatus;
    *iCallerStatus = KRequestPending;
    if ( iState != ENSmlOpenAndWaiting ) 
        {
        User::RequestComplete( iCallerStatus, KErrNotReady );
        return;
        }
    
    if( iIsHierarchicalSyncSupported )
        {
        // TODO: Have to enable once the delete issue is fixed by Organizer
        FLOG(_L("CNSmlAgendaDataStore::DoDeleteAllItemsL: Temporarily doesnot support"));
        User::RequestComplete( iCallerStatus, KErrNotSupported );
        return;
        }
    else
        {
        CNSmlAgendaDataStoreUtil* agendautil = NULL;
        HBufC* calfilename = NULL;
        
        agendautil = CNSmlAgendaDataStoreUtil::NewL();
        calfilename = iOpenedStoreName->AllocL();
        if( agendautil )
            {
            CleanupStack::PushL(agendautil);
            agendautil->InitializeCalAPIsL( calfilename ); 
            CleanupStack::Pop(agendautil);
            // Delete all items
            // First searh every UIDs ...
            TInt aNumSuccessfulDeleted( 0 );
            RArray<TCalLocalUid> uidArray;
            CleanupClosePushL( uidArray );
            TCalTime zeroTime;
            zeroTime.SetTimeUtcL( Time::NullTTime() );
            agendautil->iEntryView->GetIdsModifiedSinceDateL( zeroTime, uidArray );
            
            // ... and then delete them
            agendautil->iEntryView->DeleteL( uidArray, aNumSuccessfulDeleted );
            CleanupStack::PopAndDestroy( &uidArray ); // uidArray            
            }
        delete agendautil;
        delete calfilename;
        }
        
    // Update changefinder
    if ( iChangeFinder )
        {
        iChangeFinder->ResetL();
        }
    iSnapshotRegistered = EFalse;
    RegisterSnapshotL();
    
    User::RequestComplete( iCallerStatus, KErrNone );
    
    FLOG(_L("CNSmlAgendaDataStore::DoDeleteAllItemsL: END"));
    }

// -----------------------------------------------------------------------------
// CNSmlAgendaDataStore::DoHasSyncHistory
// Return ETrue if syncronization history is available.
// -----------------------------------------------------------------------------
//
TBool CNSmlAgendaDataStore::DoHasSyncHistory() const
    {
	FLOG(_L("CNSmlAgendaDataStore::DoHasSyncHistory: BEGIN"));
	TBool ret = EFalse;
	if ( iHasHistory )
		{
		if ( iOpenedStoreId != iChangeFinder->DataStoreUid() )
			{
			iChangeFinder->SetDataStoreUid( iOpenedStoreId );
			}
		else
			{
			ret = ETrue;
			}
		}
	else
		{
		iChangeFinder->SetDataStoreUid( iOpenedStoreId );
		}
	FLOG(_L("CNSmlAgendaDataStore::DoHasSyncHistory: END"));
	return ret;
    }

// -----------------------------------------------------------------------------
// CNSmlAgendaDataStore::DoAddedItems
// Give uid list of added items since last syncronization.
// -----------------------------------------------------------------------------
//
const MSmlDataItemUidSet& CNSmlAgendaDataStore::DoAddedItems() const
    {
	FLOG(_L("CNSmlAgendaDataStore::DoAddedItems: BEGIN"));
	if ( iState == ENSmlOpenAndWaiting )
		{
		iNewUids->Reset();
		TRAP_IGNORE( iChangeFinder->FindNewItemsL( *iNewUids ) );
		
		// RD_MULTICAL		
        if( iIsHierarchicalSyncSupported )
            {
            CNSmlDataItemUidSet* inactiveuids = NULL;
            TRAP_IGNORE(inactiveuids = ActiveItemsL( *iNewUids ));
            delete inactiveuids;
            }
        else
            {
            TBool syncstatus( EFalse );
            HBufC* calfilename = NULL;
            TRAP_IGNORE( calfilename = iOpenedStoreName->AllocL() );
            TRAP_IGNORE( syncstatus = iAgendaAdapterHandler->FolderSyncStatusL( calfilename ) );
            if( !syncstatus )
                {
                iNewUids->Reset();
                }
            delete calfilename;
            }
        // RD_MULTICAL		
		}
	
	for( TInt count = 0; count < iNewUids->ItemCount(); count++ )
        {
        FLOG(_L("CNSmlAgendaDataStore::DoAddedItems: list '%d'"), iNewUids->ItemAt(count) );
        }
	
	FLOG(_L("CNSmlAgendaDataStore::DoAddedItems: END"));
	return *iNewUids;
    }

// -----------------------------------------------------------------------------
// CNSmlAgendaDataStore::DoDeletedItems
// Give uid list of deleted items since last syncronization.
// -----------------------------------------------------------------------------
//
const MSmlDataItemUidSet& CNSmlAgendaDataStore::DoDeletedItems() const
    {
	FLOG(_L("CNSmlAgendaDataStore::DoDeletedItems: BEGIN"));
	if ( iState == ENSmlOpenAndWaiting )
		{
		iDeletedUids->Reset();
		TRAP_IGNORE( iChangeFinder->FindDeletedItemsL( *iDeletedUids ) );		
	
    	// RD_MULTICAL
    	if( iIsHierarchicalSyncSupported )
    	    {
        	CArrayFixFlat<TInt>* folderuidarr; 
        	CNSmlDataItemUidSet* tempdeleteuids; 
        	TSmlDbItemUid uid(0);
        	
        	folderuidarr = new CArrayFixFlat<TInt>( KArrayGranularity );
        	tempdeleteuids= new CNSmlDataItemUidSet();
        	
        	// Get the list of Folder ids
        	for( TInt count =0; count < iDeletedUids->ItemCount(); count++ )
                {
                uid = iDeletedUids->ItemAt(count);
                
                if( 0 == (uid % iCalOffsetVal) )
                    {
                    TRAP_IGNORE( folderuidarr->AppendL(uid) );
                    FLOG(_L("CNSmlAgendaDataStore::DoDeletedItems: Folderuid: '%d'"), uid);
                    }        
                }
        	
        	CNSmlDataItemUidSet* inactiveuids = NULL;
        	TRAP_IGNORE( inactiveuids = ActiveItemsL( *iDeletedUids ) );
            delete inactiveuids;
        	
        	if( folderuidarr->Count() > 0 )
        	    {
        	    TInt index = KErrNotFound;
                TInt err = KErrNone;
                TKeyArrayFix key( 0, ECmpTInt ); // Find key for Ids.
                
                // Fill the temp uid array
                for (TInt folderidcount = 0; folderidcount< folderuidarr->Count(); folderidcount++)
                    {
                    tempdeleteuids->AddItem(folderuidarr->At(folderidcount));
                    }
                
        	    // Filter out ID's of entries for Folder deletion
                for( TInt count =0; count < iDeletedUids->ItemCount(); count++ )
                    {
                    uid = iDeletedUids->ItemAt(count);
                    uid = (uid/iCalOffsetVal)*iCalOffsetVal;
                    err = folderuidarr->Find( uid, key, index );
                    if (err == KErrNone)
                        {
                        break;
                        }
                    tempdeleteuids->AddItem(uid);
                    }
                // Store the proper UID values
                iDeletedUids->Reset();
                for( TInt count =0; count < tempdeleteuids->ItemCount(); count++ )
                    {
                    iDeletedUids->AddItem(tempdeleteuids->ItemAt(count));
                    }
        	    }	
        	delete tempdeleteuids;
        	delete folderuidarr;
    	    }
    	else
    	    {
    	    TBool syncstatus( EFalse );
    	    HBufC* calfilename = NULL;
    	    TRAP_IGNORE( calfilename = iOpenedStoreName->AllocL() );
    	    TRAP_IGNORE( syncstatus = iAgendaAdapterHandler->FolderSyncStatusL( calfilename ) );
            if( !syncstatus )
                {
                iDeletedUids->Reset();
                }
            delete calfilename;
    	    }	    
    	// RD_MULTICAL
		}
	
	for( TInt count = 0; count < iDeletedUids->ItemCount(); count++ )
        {
        FLOG(_L("CNSmlAgendaDataStore::DoDeletedItems: list '%d'"), iDeletedUids->ItemAt(count) );
        }
	
	FLOG(_L("CNSmlAgendaDataStore::DoDeletedItems: END"));	
	return *iDeletedUids;
    }

// -----------------------------------------------------------------------------
// CNSmlAgendaDataStore::DoSoftDeletedItems
// Give uid list of deleted items since last syncronization.
// -----------------------------------------------------------------------------
//
const MSmlDataItemUidSet& CNSmlAgendaDataStore::DoSoftDeletedItems() const
    {
	FLOG(_L("CNSmlAgendaDataStore::DoSoftDeletedItems: BEGIN"));
	if ( iState == ENSmlOpenAndWaiting )
		{
		iSoftDeletedUids->Reset();
		TRAP_IGNORE(
		    iChangeFinder->FindSoftDeletedItemsL( *iSoftDeletedUids ) );
		}
	FLOG(_L("CNSmlAgendaDataStore::DoSoftDeletedItems: END"));
	return *iSoftDeletedUids;
    }

// -----------------------------------------------------------------------------
// CNSmlAgendaDataStore::DoModifiedItems
// Give uid list of modified items since last syncronization.
// -----------------------------------------------------------------------------
//
const MSmlDataItemUidSet& CNSmlAgendaDataStore::DoModifiedItems() const
    {
	FLOG(_L("CNSmlAgendaDataStore::DoModifiedItems: BEGIN"));
	if ( iState == ENSmlOpenAndWaiting )
		{
		iReplacedUids->Reset();
		TRAP_IGNORE( iChangeFinder->FindChangedItemsL( *iReplacedUids ) );
		
		// RD_MULTICAL
	    if( iIsHierarchicalSyncSupported )
            {
            CNSmlDataItemUidSet* inactiveuids = NULL;
            TRAP_IGNORE( inactiveuids = ActiveItemsL( *iReplacedUids ) );
            
            if( inactiveuids )
                {
                TRAP_IGNORE( InternalizeCommittedUidL() );
                for( TInt count = 0; count < inactiveuids->ItemCount(); count++ )
                    {
                    TSmlDbItemUid inactiveuid = inactiveuids->ItemAt(count);
                    FLOG(_L("CNSmlAgendaDataStore::DoModifiedItems: inactive '%d'"), inactiveuid );
                    for( TInt uidcount = 0; uidcount < iCommittedUidArr->ItemCount(); uidcount++ )
                        {
                        if( inactiveuid == iCommittedUidArr->ItemAt( uidcount ) )
                            {
                            FLOG(_L("CNSmlAgendaDataStore::DoModifiedItems: '%d'"), inactiveuid );
                            iReplacedUids->AddItem( inactiveuid );
                            }
                        }
                    }            
                delete inactiveuids;
                }
            }
        else
            {
            TBool syncstatus( EFalse );
            HBufC* calfilename = NULL;
            TRAP_IGNORE( calfilename= iOpenedStoreName->AllocL() );
            TRAP_IGNORE( syncstatus = iAgendaAdapterHandler->FolderSyncStatusL( calfilename ) );
            if( !syncstatus )
                {
                iReplacedUids->Reset();
                }
            delete calfilename;
            }	    
        // RD_MULTICAL
		}
	
	 for( TInt count = 0; count < iReplacedUids->ItemCount(); count++ )
	     {
	     FLOG(_L("CNSmlAgendaDataStore::DoModifiedItems: list '%d'"), iReplacedUids->ItemAt(count) );
	     }
	
	FLOG(_L("CNSmlAgendaDataStore::DoModifiedItems: END"));
	return *iReplacedUids;
    }

// -----------------------------------------------------------------------------
// CNSmlAgendaDataStore::DoMovedItems
// Give uid list of moved items since last syncronization.
// -----------------------------------------------------------------------------
//
const MSmlDataItemUidSet& CNSmlAgendaDataStore::DoMovedItems() const
    {
	FLOG(_L("CNSmlAgendaDataStore::DoMovedItems: BEGIN"));
	if ( iState == ENSmlOpenAndWaiting )
		{
		iMovedUids->Reset();
		TRAP_IGNORE( iChangeFinder->FindMovedItemsL( *iMovedUids ) );
		}
	
	for( TInt count = 0; count < iMovedUids->ItemCount(); count++ )
        {
        FLOG(_L("CNSmlAgendaDataStore::DoMovedItems: list '%d'"), iMovedUids->ItemAt(count) );
        }
	
	FLOG(_L("CNSmlAgendaDataStore::DoMovedItems: END"));
	return *iMovedUids;
    }

// -----------------------------------------------------------------------------
// CNSmlAgendaDataStore::DoResetChangeInfoL
// Reset change info that exist since last syncronization.
// -----------------------------------------------------------------------------
//
void CNSmlAgendaDataStore::DoResetChangeInfoL( TRequestStatus& aStatus )
    {
	FLOG(_L("CNSmlAgendaDataStore::DoResetChangeInfoL: BEGIN"));
	iCallerStatus = &aStatus;
	*iCallerStatus = KRequestPending;
	if ( iState != ENSmlOpenAndWaiting ) 
		{
		User::RequestComplete( iCallerStatus, KErrNotReady );
		return;
		}
	iChangeFinder->ResetL();
	iSnapshotRegistered = EFalse;
	if( !iSnapshotRegistered )
		{
		RegisterSnapshotL();
		}
	User::RequestComplete( iCallerStatus, KErrNone );
	FLOG(_L("CNSmlAgendaDataStore::DoResetChangeInfoL: END"));
    }

// -----------------------------------------------------------------------------
// CNSmlAgendaDataStore::DoCommitChangeInfoL
// Commit change info that exist since last syncronization for given uid list.
// -----------------------------------------------------------------------------
//
void CNSmlAgendaDataStore::DoCommitChangeInfoL( TRequestStatus& aStatus,
                const MSmlDataItemUidSet& aItems )
    {
	FLOG(_L("CNSmlAgendaDataStore::DoCommitChangeInfoL: BEGIN"));
	iCallerStatus = &aStatus;
	*iCallerStatus = KRequestPending;
	if ( iState != ENSmlOpenAndWaiting ) 
		{
		User::RequestComplete( iCallerStatus, KErrNotReady );
		return;
		}
	iChangeFinder->CommitChangesL( aItems );
	
	// Save the UIDs to the list
	iCommittedUidArr->Reset();
	for ( TInt count = 0; count < aItems.ItemCount(); count++ )
        {
        iCommittedUidArr->AddItem( aItems.ItemAt( count ) );
        }
	CNSmlDataItemUidSet* inactiveuids = NULL;
	TRAP_IGNORE( inactiveuids = ActiveItemsL( *iCommittedUidArr ) ); 
	delete inactiveuids;
	
	// Print the iCommittedUidArr array
	FLOG(_L("CNSmlAgendaAdapter::DoCommitChangeInfoL(): CommittedUidArr"));
    for( TInt count = 0; count < iCommittedUidArr->ItemCount(); count++ )
        {
        FLOG(_L("CNSmlAgendaDataStore::iCommittedUidArr: list '%d'"), iCommittedUidArr->ItemAt(count) );
        }   
    
    // Store the array in the stream
    ExternalizeCommittedUidL();
	User::RequestComplete( iCallerStatus, KErrNone );
	FLOG(_L("CNSmlAgendaDataStore::DoCommitChangeInfoL: END"));
    }

// -----------------------------------------------------------------------------
// CNSmlAgendaDataStore::DoCommitChangeInfoL
// Commit change info that exist since last syncronization.
// -----------------------------------------------------------------------------
//
void CNSmlAgendaDataStore::DoCommitChangeInfoL( TRequestStatus& aStatus )
    {
	FLOG(_L("CNSmlAgendaDataStore::DoCommitChangeInfoL: BEGIN"));
	iCallerStatus = &aStatus;
	*iCallerStatus = KRequestPending;
	if ( iState != ENSmlOpenAndWaiting ) 
		{
		User::RequestComplete( iCallerStatus, KErrNotReady );
		return;
		}
	iChangeFinder->CommitChangesL();
    // Print the iInterCommittedUidArr array
    FLOG(_L("CNSmlAgendaAdapter::DoCommitChangeInfoL() Without Param: CommittedUidArr"));
    for( TInt count = 0; count < iCommittedUidArr->ItemCount(); count++ )
        {
        FLOG(_L("CNSmlAgendaDataStore::iCommittedUidArr: list '%d'"), iCommittedUidArr->ItemAt(count) );
        }
    
    // Store the array in the stream
    ExternalizeCommittedUidL();
	User::RequestComplete( iCallerStatus, KErrNone );
	FLOG(_L("CNSmlAgendaDataStore::DoCommitChangeInfoL: END"));
    }
    
// -----------------------------------------------------------------------------
// CNSmlAgendaDataStore::RegisterSnapshotL
// Register snapshot.
// -----------------------------------------------------------------------------
//
void CNSmlAgendaDataStore::RegisterSnapshotL()
    {
	FLOG(_L("CNSmlAgendaAdapter::RegisterSnapshotL(): begin"));
	CArrayFixSeg<TNSmlSnapshotItem>* snapshot =
	                new ( ELeave ) CArrayFixSeg<TNSmlSnapshotItem>( 64 );
	CleanupStack::PushL( snapshot );
	RArray<TCalLocalUid> uidArray;
	CleanupClosePushL( uidArray );
	
	// RD_MULTICAL
	if( iIsHierarchicalSyncSupported )
	    {
    	// First find all entries ...
	    iAgendaAdapterHandler->SynchronizableCalendarIdsL( iCalOffsetArr );  
	    // Populate the Entry ID's associated with the CalFile
	    SynchronizableCalEntryIdsL( uidArray );
        }
	else
	    {
	    HBufC* calfilename = iOpenedStoreName->AllocL();
	    CNSmlAgendaDataStoreUtil* agendautil = CNSmlAgendaDataStoreUtil::NewL();
        if( agendautil )
            {
            CleanupStack::PushL(agendautil);
            agendautil->InitializeCalAPIsL( calfilename );
            CleanupStack::Pop(agendautil);
            }	   
	    // First find all entries ...
        TCalTime zeroTime;
        zeroTime.SetTimeUtcL( Time::NullTTime() );
        agendautil->iEntryView->GetIdsModifiedSinceDateL( zeroTime, uidArray );
        delete agendautil;
        delete calfilename;
	    }
	// RD_MULTICAL
    
	// ... and then create snapshot items
	for ( TInt i = 0; i < uidArray.Count(); i++ )
	    {
	    TNSmlSnapshotItem newItem = CreateSnapshotItemL( uidArray[i] );
	    if ( newItem.ItemId() != 0 )
	        {
	        snapshot->InsertIsqL( newItem, iKey );
	        }
	    }
	
	CleanupStack::PopAndDestroy( &uidArray );	
	
    for ( TInt i = 0; i < snapshot->Count(); i++ )
        {
        TNSmlSnapshotItem item = snapshot->At(i);
        FLOG(_L("CNSmlAgendaAdapter::RegisterSnapshotL(): id: '%d'"), item.ItemId());
        }
		
	iChangeFinder->SetNewSnapshot( snapshot );
	
	// Save the UIDs to the list
	iCommittedUidArr->Reset();
	for ( TInt count = 0; count < snapshot->Count(); count++ )
        {
        iCommittedUidArr->AddItem( snapshot->At( count ).ItemId() );
        }
	CNSmlDataItemUidSet* inactiveuids = NULL;
	TRAP_IGNORE( inactiveuids = ActiveItemsL( *iCommittedUidArr ) );
	delete inactiveuids;
	// Print the iCommittedUidArr array
	FLOG(_L("CNSmlAgendaAdapter::RegisterSnapshotL(): CommittedUidArr"));
	for( TInt count = 0; count < iCommittedUidArr->ItemCount(); count++ )
        {
        FLOG(_L("CNSmlAgendaDataStore::RegisterSnapshotL: list '%d'"), iCommittedUidArr->ItemAt(count) );
        }
	
	// iChangeFinder takes ownership of items
	CleanupStack::Pop( snapshot );
	    
	iSnapshotRegistered = ETrue;
	
	FLOG(_L("CNSmlAgendaAdapter::RegisterSnapshotL(): end"));
    }

// -----------------------------------------------------------------------------
// CNSmlAgendaDataStore::CreateSnapshotItemL
// Creates new snapshot. Method gets data from database.
// -----------------------------------------------------------------------------
TNSmlSnapshotItem CNSmlAgendaDataStore::CreateSnapshotItemL(
                const TCalLocalUid& aUid )
    {
    FLOG(_L("CNSmlAgendaAdapter::CreateSnapshotItemL(): Begin"));
    
    TNSmlSnapshotItem item( 0 );
    DataMimeType( aUid );
    
    switch( iDataMimeType )
        {
        case ENSmlFolder:
            {
            item = iAgendaAdapterHandler->CreateFolderSnapShotItemL( aUid );
            }
            break;
        case ENSmlCalendar:
            {
            CNSmlAgendaDataStoreUtil* agendautil = NULL;
            HBufC* calfilename = NULL;
            TCalLocalUid parentid(NULL);
            TCalLocalUid entryid(aUid);
            TInt err( KErrNone );
            
            if( iIsHierarchicalSyncSupported )
                {
                TRAP( err, GetCalendarEntryIdL( parentid, entryid ));
                if ( err )
                    {
                    FLOG(_L("CNSmlAgendaDataStore::CreateSnapshotItemL:Parent Id is not Valid one"));
                    return item;
                    }
                //Get the Folder Information
                TRAP(err, calfilename = iAgendaAdapterHandler->FolderNameL(parentid) );
                }  
            else
                {
                calfilename = iOpenedStoreName->AllocL();
                }
                
             if( err != KErrNone || NULL == calfilename )
                {
                FLOG(_L("CNSmlAgendaDataStore::CreateSnapshotItemL:Invalid CalendarInfo"));
                return item;
                }
            
            FLOG(_L("CNSmlAgendaDataStore::CreateSnapshotItemL:Parent Id: '%d'"), parentid);
            FLOG(_L("CNSmlAgendaDataStore::CreateSnapshotItemL:Entry Id: '%d'"), entryid);
            
            agendautil = CNSmlAgendaDataStoreUtil::NewL();
            if( agendautil )
                {
                CleanupStack::PushL(agendautil);
                TRAP(err, agendautil->InitializeCalAPIsL( calfilename, entryid ));
                CleanupStack::Pop(agendautil);
                }            
            if ( err || !agendautil )
                {
                FLOG(_L("CNSmlAgendaDataStore::CreateSnapshotItemL: entry is not valid"));  
                delete agendautil;
                delete calfilename; 
                return item;
                }
         
            CCalEntry::TReplicationStatus replicationStatus =
                                            agendautil->iEntry->ReplicationStatusL();
            if ( CanBeSynchronized( replicationStatus ) )
                {
                TUint intUid = agendautil->iEntry->LocalUidL() + parentid;
                item.SetItemId( intUid );
                item.SetLastChangedDate(
                        agendautil->iEntry->LastModifiedDateL().TimeUtcL() );
                item.SetSoftDelete( EFalse );
                }
            delete agendautil;
            delete calfilename;
            }
            break;
        default:
            break;
        }
    FLOG(_L("CNSmlAgendaAdapter::CreateSnapshotItemL(): end"));
    
	return item;
    }

// -----------------------------------------------------------------------------
// CNSmlAgendaDataStore::DoListAgendaFilesL
// List possible calendar database file names.
// -----------------------------------------------------------------------------
//
CDesCArray* CNSmlAgendaDataStore::DoListAgendaFilesLC() const
    {
    CDesCArray* array = new (ELeave) CDesCArrayFlat(1);
    array->AppendL(*iDefaultStoreFileName);
    CleanupStack::PushL( array );
    return array;
    }

// -----------------------------------------------------------------------------
// CNSmlAgendaDataStore::DoGetDefaultFileNameL
// Return default calendar database name.
// -----------------------------------------------------------------------------
//
const TDesC& CNSmlAgendaDataStore::DoGetDefaultFileNameL() const
    {
	if ( !iDefaultStoreFileName )
		{
        User::Leave( KErrGeneral );
		}
	return *iDefaultStoreFileName;
    }

// -----------------------------------------------------------------------------
// CNSmlAgendaDataStore::CanBeSynchronized
// Return ETrue if entry can be synchronized.
// -----------------------------------------------------------------------------
//
TBool CNSmlAgendaDataStore::CanBeSynchronized(
            const CCalEntry::TReplicationStatus&
            aReplicationStatus ) const
	{
	return ( aReplicationStatus != CCalEntry::ERestricted );	
	}

// -----------------------------------------------------------------------------
// CNSmlAgendaDataStore::DoOwnStoreFormatL
// Returns adapters supported store format which is read from Calendar Plug
// In Adapter own resource file.
// -----------------------------------------------------------------------------
//
CSmlDataStoreFormat* CNSmlAgendaDataStore::DoOwnStoreFormatL()
	{
	FLOG(_L("CNSmlAgendaDataStore:::DoOwnStoreFormatL(): BEGIN"));
	
	AgendaAdapterHandlerL();
	
	if( NULL == iAgendaAdapterHandler )
	    {
        FLOG(_L("CNSmlAgendaDataStore:::DoOwnStoreFormatL(): Invalid AgendaAdapterHandler Error END"));
        User::Leave( KErrGeneral );
	    }
	
    TFileName fileName;
    TParse* parse = NULL;
    parse = new(ELeave) TParse();
    CleanupStack::PushL( parse );
    
    // Check correct Data Sync protocol
    TInt value( EDataSyncNotRunning );
    TInt error = RProperty::Get( KPSUidDataSynchronizationInternalKeys,
                                 KDataSyncStatus,
                                 value );
    
    FLOG(_L("CNSmlAgendaDataStore:::DoOwnStoreFormatL(): SyncStatus: '%d'"), KDataSyncStatus);
    FLOG(_L("CNSmlAgendaDataStore:::DoOwnStoreFormatL(): value: '%d'"), value);
    
    if ( error == KErrNone &&
         value == EDataSyncRunning )
        {
        parse->Set( KNSmlDSAgendaDataStoreRsc_1_1_2,
                          &KDC_RESOURCE_FILES_DIR, NULL );
        }
    else
        { 
        FLOG(_L("CNSmlAgendaDataStore:::DoOwnStoreFormatL(): Invoking AdapterHandler Implementation"));
        CleanupStack::PopAndDestroy( parse );
        return iAgendaAdapterHandler->StoreFormatL( iStringPool );
        }
    
    fileName = parse->FullName();
    RResourceFile resourceFile;
    BaflUtils::NearestLanguageFile( iRfs, fileName );

    TRAPD( leavecode, resourceFile.OpenL( iRfs,fileName ) );
    if ( leavecode != 0 )
        {
        CleanupStack::PopAndDestroy(); // parse
        FLOG(_L("CNSmlAgendaDataStore:::StoreFormatL(): Error END"));
        User::Leave( leavecode );
        }
    
    CleanupClosePushL( resourceFile );
    HBufC8* buffer = resourceFile.AllocReadLC( NSML_AGENDA_DATA_STORE );
    TResourceReader reader;
    reader.SetBuffer( buffer );

    CSmlDataStoreFormat* dsFormat = NULL;
    dsFormat = CSmlDataStoreFormat::NewLC( iStringPool, reader );
    CleanupStack::Pop(); // dsFormat
    CleanupStack::PopAndDestroy( 3 ); // buffer, resourceFile, parse
    
    FLOG(_L("CNSmlAgendaDataStore::DoOwnStoreFormatL: END"));
    return dsFormat;
	}

// -----------------------------------------------------------------------------
// CNSmlAgendaDataStore::DoCommitCreateCalItemL
// Commit Calendar item data to database when adding item.
// -----------------------------------------------------------------------------
//
void CNSmlAgendaDataStore::DoCommitCreateCalItemL()
    {
    FLOG(_L("CNSmlAgendaDataStore::DoCommitCreateCalItemL: BEGIN"));
    FLOG(_L("CNSmlAgendaDataStore::DoCommitCreateCalItemL: Parentid: '%d'"), iParentItemId);
    iState = ENSmlOpenAndWaiting; // iState set to closed to handle leave
	CCalEntry::TReplicationStatus  replicationStatus;
	TInt err(KErrNone);
	CNSmlAgendaDataStoreUtil* agendautil = NULL;
	HBufC* calfilename = NULL;
	RBufReadStream readStream;
	readStream.Open( *iItemData );
	readStream.PushL();

    RPointerArray<CCalEntry> rdArray;
	CleanupStack::PushL( PtrArrCleanupItemRArr ( CCalEntry, &rdArray ) );
	
    if( iIsHierarchicalSyncSupported )
        {
        //Get the Folder Information
        TRAP(err, calfilename = iAgendaAdapterHandler->FolderNameL(iParentItemId) );
        }
    else
        {
        calfilename = iOpenedStoreName->AllocL();
        }
        
    if( err != KErrNone || NULL == calfilename )
        {
        FLOG(_L("CNSmlAgendaDataStore::DoCommitCreateCalItemL:Invalid CalendarInfo"));
        CleanupStack::PopAndDestroy( 2 ); // rdArray, readStream
        User::Leave( KErrNotFound );
        }
        
    agendautil = CNSmlAgendaDataStoreUtil::NewL();
    if( agendautil )
        {
        CleanupStack::PushL(agendautil);
        TRAP(err, agendautil->InitializeCalAPIsL( calfilename ));
        CleanupStack::Pop(agendautil);
        }   
        
    if ( err || !agendautil )
        {
        FLOG(_L("CNSmlAgendaDataStore::DoCommitCreateCalItemL:Invalid CalendarInfo"));
        CleanupStack::PopAndDestroy( 2 ); // rdArray, readStream
        delete calfilename;
        User::Leave( KErrNotFound );
        }
        
    if ( iRXEntryType == ENSmlICal )
        {
        FLOG(_L("CNSmlAgendaDataStore::DoCommitCreateCalItemL: ImportICalendarL"));           
        agendautil->iImporter->ImportICalendarL( readStream, rdArray );
        }
    else if ( iRXEntryType == ENSmlVCal )
        {
        FLOG(_L("CNSmlAgendaDataStore::DoCommitCreateCalItemL: ImportVCalendarL"));
        agendautil->iImporter->ImportVCalendarL( readStream, rdArray );
        }
    else
        {
        CleanupStack::PopAndDestroy( 2 ); // rdArray, readStream
        delete agendautil;
        delete calfilename; 
        FLOG(_L("CNSmlAgendaDataStore::DoCommitCreateCalItemL - \
                   KErrNotSupported: END"));
        User::Leave( KErrNotSupported );
        }
    
    // If rdArray is empty or there is multiple items then return error
    // Multiple items are not supported
    if ( rdArray.Count() != 1 )
        {
        CleanupStack::PopAndDestroy( 2 ); // rdArray, readStream
        delete agendautil;
        delete calfilename; 
        FLOG(_L("CNSmlAgendaDataStore::DoCommitCreateCalItemL - \
                   Multiple items are not supported: END"));
        User::Leave( KErrNotSupported );
        }           
        
    err = KErrNone;    
    FLOG(_L("CNSmlAgendaDataStore::DoCommitCreateCalItemL: before StoreL"));
    TRAP( err, iInterimUtils->StoreL( *agendautil->iEntryView, *rdArray[0], ETrue ) );
    FLOG(_L("CNSmlAgendaDataStore::DoCommitCreateCalItemL: after StoreL '%d'"), err );
    if ( err )
        {
        CleanupStack::PopAndDestroy( 2 ); // rdArray, readStream
        delete agendautil;
        delete calfilename; 
        FLOG(_L("CNSmlAgendaDataStore::DoCommitCreateCalItemL - \
                   Error at storing item to database: END"));
        User::Leave( KErrGeneral );
        }

    *iAddItemId = rdArray[0]->LocalUidL();        
    CCalEntry* newEntry = agendautil->iEntryView->FetchL( *iAddItemId );
    
    // RD_MULTICAL
    if( iIsHierarchicalSyncSupported )
        {
        *iAddItemId = *iAddItemId + iParentItemId;
        }
    // RD_MULTICAL
    
    if( newEntry )
        {
        CleanupStack::PushL( newEntry );
        
        replicationStatus = newEntry->ReplicationStatusL();
    
        if ( CanBeSynchronized( replicationStatus ) )
            {
            if ( iChangeFinder )
                {
                TNSmlSnapshotItem item( *iAddItemId );
                    item.SetLastChangedDate(
                    newEntry->LastModifiedDateL().TimeUtcL() );
                item.SetSoftDelete( EFalse );
                TRAPD( changeFinderError, iChangeFinder->ItemAddedL( item ) );
                if ( changeFinderError == KErrAlreadyExists )
                    {
                    iChangeFinder->ItemUpdatedL( item );
                    }
                else
                    {
                    User::LeaveIfError( changeFinderError );    
                    }
                }
            }
        CleanupStack::PopAndDestroy();// newEntry,
        }
	CleanupStack::PopAndDestroy( 2 ); //  rdArray, readStream 
    delete agendautil;
	delete calfilename;     	
      
    FLOG(_L("CNSmlAgendaDataStore::DoCommitCreateCalItemL: END"));
    }
    
// -----------------------------------------------------------------------------
// CNSmlAgendaDataStore::DoCommitReplaceCalItemL
// Commit Calendar item data to database when replacing item.
// -----------------------------------------------------------------------------
//
void CNSmlAgendaDataStore::DoCommitReplaceCalItemL()
    {
    FLOG(_L("CNSmlAgendaDataStore::DoCommitReplaceCalItemL: BEGIN"));
    iState = ENSmlOpenAndWaiting; // iState set to closed to handle leave
	CBufFlat* oldItem = CBufFlat::NewL( KNSmlItemDataExpandSize );
	CleanupStack::PushL( oldItem );
	RBufWriteStream writeStream( *oldItem );
	writeStream.PushL();
	CNSmlAgendaDataStoreUtil* agendautil = NULL;
	HBufC* calfilename = NULL;
	TInt err( KErrNone );
	
    
    if( iIsHierarchicalSyncSupported )
        {
        TRAP(err, calfilename = iAgendaAdapterHandler->FolderNameL(iParentItemId) );
        }
    else
        {
        calfilename = iOpenedStoreName->AllocL();
        }
    
    if( err != KErrNone || NULL == calfilename )
        {
        FLOG(_L("CNSmlAgendaDataStore::DoCommitReplaceCalItemL:Invalid CalendarInfo"));
        CleanupStack::PopAndDestroy( 2 ); // olditem, writeStream
        User::Leave( KErrNotFound );
        }
    
    FLOG(_L("CNSmlAgendaDataStore::DoCommitReplaceCalItemL: entry id: '%d'"), iReplaceItemId);
    
    agendautil = CNSmlAgendaDataStoreUtil::NewL();
    if( agendautil )
        {
        CleanupStack::PushL(agendautil);
        TRAP(err, agendautil->InitializeCalAPIsL( calfilename, iReplaceItemId ));
        CleanupStack::Pop(agendautil);
        }
        
    if ( err || !agendautil )
        {
        FLOG(_L("CNSmlAgendaDataStore::DoCommitReplaceCalItemL: entry is not valid"));
        CleanupStack::PopAndDestroy( 2 ); // olditem, writeStream
        delete agendautil;
        delete calfilename;
        User::Leave( KErrGeneral );
        }  
        
    // Export item from database depending on transmitted item entry type
    if ( iTXEntryType == ENSmlVCal )
        {
        FLOG(_L("CNSmlAgendaDataStore::DoCommitReplaceCalItemL: ExportVCalL"));
        agendautil->iExporter->ExportVCalL( *agendautil->iEntry, writeStream );        
        }
#ifdef __NSML_USE_ICAL_FEATURE
    else if ( iTXEntryType == ENSmlICal )
        {
        FLOG(_L("CNSmlAgendaDataStore::DoCommitReplaceCalItemL: ExportICalL"));
        agendautil->iExporter->ExportICalL( *agendautil->iEntry, writeStream );
        }
#endif // __NSML_USE_ICAL_FEATURE
    else
        {
        CleanupStack::PopAndDestroy( 2 ); // olditem, writeStream
        delete agendautil;
        delete calfilename;
        FLOG(_L("CNSmlAgendaDataStore::DoCommitReplaceCalItemL - \
                   KErrNotSupported: END"));
        User::Leave( KErrNotSupported );
        }
        
    writeStream.CommitL();
    oldItem->Compress();

    CleanupStack::PopAndDestroy( 1 ); 
    
    // Get original UID, geoId and Recurrence-ID properties
    HBufC8* uid = NULL;
    HBufC8* recurrenceId = NULL;
    HBufC8* xRecurrenceId = NULL;
    HBufC8* geoId = NULL;
    GetPropertiesFromDataL( oldItem, uid, KVersitTokenUID()  );
    GetPropertiesFromDataL( oldItem, recurrenceId, KNSmlVersitTokenRecurrenceID() );
    GetPropertiesFromDataL( oldItem, xRecurrenceId, KNSmlVersitTokenXRecurrenceID() );
    GetPropertiesFromDataL( oldItem, geoId, KNSmlVersitTokenGeoID() );
    CleanupStack::PushL( uid );
    CleanupStack::PushL( recurrenceId );
    CleanupStack::PushL( xRecurrenceId );
    CleanupStack::PushL( geoId );

    if ( iDataMod->NeedsMerge() )
        {
        // Merge data
        iDataMod->MergeRxL( *iItemData, *oldItem );
        }
    
    // Add original UID and Recurrence-ID to merged data
    // This first removes UID and Recurrence-ID from merged data
    // and then adds original ones
    if ( uid )
        {
        FLOG(_L("CNSmlAgendaDataStore::DoCommitReplaceCalItemL: SetPropertiesToDataL :uid"));
        SetPropertiesToDataL( uid, KVersitTokenUID() );    
        }
    else
        {
        CleanupStack::PopAndDestroy( 5 ); // xRecurrenceId, recurrenceId,
                                              // uid, oldItem, geoId
        delete agendautil;
        delete calfilename;
        
        User::Leave( KErrNotSupported );
        }
    if ( recurrenceId )
        {
        FLOG(_L("CNSmlAgendaDataStore::DoCommitReplaceCalItemL: SetPropertiesToDataL :recurrenceId"));
        SetPropertiesToDataL( recurrenceId, KNSmlVersitTokenRecurrenceID() );    
        }
    if ( xRecurrenceId )
        {
        FLOG(_L("CNSmlAgendaDataStore::DoCommitReplaceCalItemL: SetPropertiesToDataL :xRecurrenceId"));
        SetPropertiesToDataL( xRecurrenceId, KNSmlVersitTokenXRecurrenceID() );    
        }
    if ( geoId )
        {
        FLOG(_L("CNSmlAgendaDataStore::DoCommitReplaceCalItemL: SetPropertiesToDataL :geoId"));
        SetPropertiesToDataL( geoId, KNSmlVersitTokenGeoID() );    
        }
    
    CleanupStack::PopAndDestroy( 5 ); // xRecurrenceId, recurrenceId,
                                      // uid, oldItem, geoId
    
    // Replace item to database
    RBufReadStream readStream;
    readStream.Open( *iItemData );
    readStream.PushL();

    RPointerArray<CCalEntry> rdArray;
    CleanupStack::PushL( PtrArrCleanupItemRArr ( CCalEntry, &rdArray ) );

    // Import item to database depending on received item entry type
    if ( iRXEntryType == ENSmlVCal )
        {
        FLOG(_L("CNSmlAgendaDataStore::DoCommitReplaceCalItemL: ImportVCalendarL"));
        TRAP( err, agendautil->iImporter->ImportVCalendarL( readStream, rdArray ));
        FLOG(_L("CNSmlAgendaDataStore::DoCommitReplaceCalItemL: ImportVCalendarL error: '%d'"), err);
        if( err != KErrNone)
            {
            User::Leave(err);
            }
        }
#ifdef __NSML_USE_ICAL_FEATURE
    else if ( iRXEntryType == ENSmlICal )
        {
        FLOG(_L("CNSmlAgendaDataStore::DoCommitReplaceCalItemL: ImportICalendarL"));
        agendautil->iImporter->ImportICalendarL( readStream, rdArray );
        }
#endif // __NSML_USE_ICAL_FEATURE
    else
        {
        CleanupStack::PopAndDestroy( 2 ); // rdArray, readStream
        delete agendautil;
        delete calfilename;
        FLOG(_L("CNSmlAgendaDataStore::DoCommitReplaceItemL - \
                   KErrNotSupported: END"));
        User::Leave( KErrNotSupported );
        }

    // If rdArray is empty or there is multiple items then return error
    // Multiple items are not supported
    if ( rdArray.Count() != 1 )
        {
        FLOG(_L("CNSmlAgendaDataStore::DoCommitReplaceCalItemL: Multiple items are not supported "));
        CleanupStack::PopAndDestroy( 2 ); // rdArray, readStream
        delete agendautil;
        delete calfilename;
        FLOG(_L("CNSmlAgendaDataStore::DoCommitReplaceItemL - \
                   Multiple items are not supported: END"));
        User::Leave( KErrNotSupported );
        }
        
    err = KErrNone;

    FLOG(_L("CNSmlAgendaDataStore::DoCommitReplaceCalItemL: before StoreL"));
    TRAP( err, iInterimUtils->StoreL( *agendautil->iEntryView, *rdArray[0], ETrue ) );
    FLOG(_L("CNSmlAgendaDataStore::DoCommitReplaceCalItemL: after StoreL '%d'"), err );
    
    if ( err )
        {
        CleanupStack::PopAndDestroy( 2 ); // rdArray, readStream
        delete agendautil;
        delete calfilename;
        FLOG(_L("CNSmlAgendaDataStore::DoCommitReplaceCalItemL - \
                   Error at storing item to database: END"));
        User::Leave( KErrGeneral );
        }

    CCalEntry::TReplicationStatus  replicationStatus;
    
    CCalEntry* replacedEntry = agendautil->iEntryView->FetchL( iReplaceItemId );
    if( replacedEntry )
    {
    CleanupStack::PushL( replacedEntry );
    
    replicationStatus = replacedEntry->ReplicationStatusL();
    
    // RD_MULTICAL
    if( !iIsHierarchicalSyncSupported )
        {
        iParentItemId = 0;
        }
    // RD_MULTICAL
                
    if ( CanBeSynchronized( replicationStatus ) )
        {
        if ( iChangeFinder )
            {
            TNSmlSnapshotItem item( iReplaceItemId + iParentItemId );
            item.SetLastChangedDate(
                        replacedEntry->LastModifiedDateL().TimeUtcL());
            item.SetSoftDelete( EFalse );
            iChangeFinder->ItemUpdatedL( item );
            }
        }
    CleanupStack::PopAndDestroy(); // replacedEntry
    }
    
	CleanupStack::PopAndDestroy( 2 ); // rdArray, readStream
    delete agendautil;
	delete calfilename;	
	FLOG(_L("CNSmlAgendaDataStore::DoCommitReplaceCalItemL: END"));
    }

// -----------------------------------------------------------------------------
// CNSmlAgendaDataStore::DoCommitCreateFolderItemL
// Commit Folder item data to database when adding item.
// -----------------------------------------------------------------------------
//
void CNSmlAgendaDataStore::DoCommitCreateFolderItemL()
    {
    FLOG(_L("CNSmlAgendaDataStore::DoCommitCreateFolderItemL: BEGIN"));
    
    TInt err(KErrNone);
    RBufReadStream readStream;
    readStream.Open( *iItemData );
    readStream.PushL();
    
    TRAP(err, *iAddItemId = iAgendaAdapterHandler->CreateFolderL( readStream ));
    if( err != KErrNone )
        {
        FLOG(_L("CNSmlAgendaDataStore::DoCommitCreateFolderItemL: Error in Create Folder"));
        CleanupStack::PopAndDestroy( &readStream );
        User::Leave( err );    
        }
   
    // Add snapshotitem
    if ( iChangeFinder )
        {
        FLOG(_L("CNSmlAgendaDataStore::DoCommitCreateFolderItemL: item id: '%d'"), *iAddItemId );
        TNSmlSnapshotItem item( *iAddItemId );
        TTime time;
        time.HomeTime();
        item.SetLastChangedDate( time );
        item.SetSoftDelete( EFalse );
        TRAPD( changeFinderError, iChangeFinder->ItemAddedL( item ) );
        if ( changeFinderError == KErrAlreadyExists )
            {
            iChangeFinder->ItemUpdatedL( item );
            }
        else
            {
            User::LeaveIfError( changeFinderError );    
            }
        
        iCalOffsetArr->AppendL(*iAddItemId);
            
        }
    
    CleanupStack::PopAndDestroy( &readStream );
    
    FLOG(_L("CNSmlAgendaDataStore::DoCommitCreateFolderItemL: END"));
    }
    

// -----------------------------------------------------------------------------
// CNSmlAgendaDataStore::DoCommitReplaceFolderItemL
// Commit Folder item data to database when replacing item.
// -----------------------------------------------------------------------------
//
void CNSmlAgendaDataStore::DoCommitReplaceFolderItemL()
    {
    FLOG(_L("CNSmlAgendaDataStore::DoCommitReplaceFolderItemL: BEGIN"));
    
    TBool syncstatus(ETrue);
    TInt err(KErrNone);
    RBufReadStream readStream;
    readStream.Open( *iItemData );
    readStream.PushL();
    
    TRAP(err, iAgendaAdapterHandler->ReplaceFolderL( iReplaceItemId, readStream, syncstatus ));
    if(err != KErrNone)
        {
        FLOG(_L("CNSmlAgendaDataStore::DoCommitReplaceFolderItemL: Error while replacing Folder"));
        CleanupStack::PopAndDestroy( &readStream );
        User::Leave(err);
        }
   
    // Update snapshotitem
    if ( iChangeFinder && syncstatus )
        {
        TNSmlSnapshotItem item( iReplaceItemId );
        TTime time;
        time.HomeTime();
        item.SetLastChangedDate(time);
        item.SetSoftDelete( EFalse );
        iChangeFinder->ItemUpdatedL( item );
        }
        
    CleanupStack::PopAndDestroy( &readStream );    
    FLOG(_L("CNSmlAgendaDataStore::DoCommitReplaceFolderItemL: END"));
    }
// -----------------------------------------------------------------------------
// CNSmlAgendaDataStore::GetPropertiesFromDataL
// Gets property from old item.
// -----------------------------------------------------------------------------
//
void CNSmlAgendaDataStore::GetPropertiesFromDataL( CBufFlat* aOldItem,
                                                   HBufC8*& aValue,
	                                               const TDesC8& aProperty )
    {
    FLOG(_L("CNSmlAgendaDataStore::GetPropertiesFromDataL(): begin"));
    
    // Gemerate property that is searched (Linebreak + property + tokencolon)
    HBufC8* startBuffer = HBufC8::NewLC( KVersitTokenCRLF().Size() +
                                         aProperty.Length() );
    TPtr8 startBufferPtr( startBuffer->Des() );
	startBufferPtr.Append( KVersitTokenCRLF() );
	startBufferPtr.Append( aProperty );
	
	// Get start position of property value
	TInt startPos = aOldItem->Ptr( 0 ).Find( startBufferPtr );
	
	TInt endPos( KErrNotFound );
	if ( startPos != KErrNotFound )
	    {
	    // startPos is before UID word
	    startPos = startPos + KVersitTokenCRLF().Length();
	    
	    // End end position of property value
	    endPos = startPos + 
	             aOldItem->Ptr( startPos ).Find( KVersitTokenCRLF() );
	    
	    TPtrC8 tempPtr( aOldItem->Ptr( 0 ).Mid( endPos ) );
	    
	    // If next char after linebreak is ' ' or '=' then it is only
	    // linebreak inside of value. This does not work with base64 coding!
	    while ( tempPtr[KVersitTokenCRLF().Length()] == ' ' ||
	            tempPtr[KVersitTokenCRLF().Length()] == '=' )
	        {
	        endPos = endPos + KVersitTokenCRLF().Length();
	        TInt tempPos = aOldItem->Ptr( endPos ).Find( KVersitTokenCRLF() );
	        endPos = endPos + tempPos;
	        tempPtr.Set( aOldItem->Ptr( 0 ).Mid( endPos ) );
	        }
        
        // Then cut value to value parameter
        delete aValue;
		aValue = NULL;
		TInt length = endPos - startPos;
		aValue = HBufC8::NewLC( length );
		TPtr8 valuePtr( aValue->Des() );
		aOldItem->Read( startPos, valuePtr, length );
		CleanupStack::Pop( aValue ); // aValue
	    }
    CleanupStack::PopAndDestroy( startBuffer ); // startBuffer
    
    FLOG(_L("CNSmlAgendaDataStore::GetPropertiesFromDataL(): end"));
    }
  
// -----------------------------------------------------------------------------
// Adds property and it's value to new item.
// This method also removes original property and it's value.
// -----------------------------------------------------------------------------
//  
void CNSmlAgendaDataStore::SetPropertiesToDataL( HBufC8*& aValue,
	                                             const TDesC8& aProperty )
    {
    FLOG(_L("CNSmlAgendaDataStore::SetPropertiesToDataL(): begin"));
    
    // Gemerate property that is searched (Linebreak + property + tokencolon)
    HBufC8* startBuffer = HBufC8::NewLC( KVersitTokenCRLF().Size() +
                                         aProperty.Length() );
    TPtr8 startBufferPtr( startBuffer->Des() );
	startBufferPtr.Append( KVersitTokenCRLF() );
	startBufferPtr.Append( aProperty );
	
	// Get start position of property value
	TInt startPos = iItemData->Ptr( 0 ).Find( startBufferPtr );
	
	TInt endPos( KErrNotFound );
	if ( startPos != KErrNotFound )
	    {
	    // startPos is before UID word
	    startPos = startPos + KVersitTokenCRLF().Length();
	    
	    // End end position of property value
	    endPos = startPos + 
	             iItemData->Ptr( startPos ).Find( KVersitTokenCRLF() );
	    
	    TPtrC8 tempPtr( iItemData->Ptr( 0 ).Mid( endPos ) );
	    
	    // If next char after linebreak is ' ' or '=' then it is only
	    // linebreak inside of value. This does not work with base64 coding!
	    while ( tempPtr[KVersitTokenCRLF().Length()] == ' ' ||
	            tempPtr[KVersitTokenCRLF().Length()] == '=' )
	        {
	        endPos = endPos + KVersitTokenCRLF().Length();
	        TInt tempPos = iItemData->Ptr( endPos ).Find( KVersitTokenCRLF() );
	        endPos = endPos + tempPos;
	        tempPtr.Set( iItemData->Ptr( 0 ).Mid( endPos ) );
	        }
        
        // Delete original property and value
        iItemData->Delete( startPos, endPos - startPos );
        
	    }

    // Add property and new value from parameter.
    // First find end of VEVENT or VTODO
    
    // Generate VEVENT END property
    HBufC8* endVEvent = HBufC8::NewLC( KVersitTokenCRLF().Size() + 
                                     KVersitTokenEND().Length() +
                                     KVersitTokenColon().Length() +
                                     KVersitVarTokenVEVENT().Length() );
	TPtr8 endVEventPtr( endVEvent->Des() );
	endVEventPtr.Append( KVersitTokenCRLF() );
	endVEventPtr.Append( KVersitTokenEND() );
	endVEventPtr.Append( KVersitTokenColon() );
	endVEventPtr.Append( KVersitVarTokenVEVENT() );
	
	// Generate VTODO END property
    HBufC8* endVTodo = HBufC8::NewLC( KVersitTokenCRLF().Size() + 
                                     KVersitTokenEND().Length() +
                                     KVersitTokenColon().Length() +
                                     KVersitVarTokenVTODO().Length() );
	TPtr8 endVTodoPtr( endVTodo->Des() );
	endVTodoPtr.Append( KVersitTokenCRLF() );
	endVTodoPtr.Append( KVersitTokenEND() );
	endVTodoPtr.Append( KVersitTokenColon() );
	endVTodoPtr.Append( KVersitVarTokenVTODO() );
    
    // Find end of VEVENT or VTODO
    endPos = iItemData->Ptr( 0 ).Find( endVEventPtr );
    if ( endPos == KErrNotFound )
        {
        endPos = iItemData->Ptr( 0 ).Find( endVTodoPtr );
        if ( endPos == KErrNotFound )
            {
            User::Leave( KErrNotFound );
            }
        }

    // Add property and value from parameter
	iItemData->InsertL( endPos, KVersitTokenCRLF() );
	iItemData->InsertL( endPos + KVersitTokenCRLF().Size(),
	                    aValue->Des() );
    iItemData->Compress();
    
    CleanupStack::PopAndDestroy( 3 ); // endVTodo, endVEvent, startBuffer
    
    FLOG(_L("CNSmlAgendaDataStore::SetPropertiesToDataL(): end"));
    }

// -----------------------------------------------------------------------------
// CNSmlAgendaDataStore::GetCalendarEntryIdL
// Method to retrieve the Id of the Calendar Entry
// -----------------------------------------------------------------------------
void CNSmlAgendaDataStore::GetCalendarEntryIdL( TCalLocalUid& aParentId, TCalLocalUid& aCalId ) const
    {
    FLOG(_L("CNSmlAgendaDataStore::GetCalendarEntryIdL: BEGIN"));
    
    FLOG(_L("CNSmlAgendaDataStore::GetCalendarEntryIdL: parentid: '%d'"), aParentId);
    FLOG(_L("CNSmlAgendaDataStore::GetCalendarEntryIdL: aCalId: '%d'"), aCalId);
    
    TInt uidcalentry = aCalId;
    
    if( uidcalentry < iCalOffsetVal )
        {
        FLOG(_L("CNSmlAgendaDataStore::GetCalendarEntryIdL: Invalid CalendarId: '%d'"), aCalId);
        User::Leave( KErrGeneral );
        }
    else if( uidcalentry == iCalOffsetVal ||
             0 == (uidcalentry % iCalOffsetVal) )
        {
        aParentId = uidcalentry;
        }
    else
        {
        while( uidcalentry > iCalOffsetVal )
            {
            uidcalentry = uidcalentry - iCalOffsetVal;
            }    
        aParentId = aCalId - uidcalentry;
        aCalId = uidcalentry;
        }
    
    TInt err(KErrNone);
    TInt index = KErrNotFound;
    TKeyArrayFix key( 0, ECmpTInt ); // Find key for Ids.
    
    err = iCalOffsetArr->Find( aParentId, key, index );
    
    if( err != KErrNone )
        {
        User::Leave(err);
        }   
    FLOG(_L("CNSmlAgendaDataStore::GetCalendarEntryIdL: END"));    
    }

// -----------------------------------------------------------------------------
// CNSmlAgendaDataStore::DataMimeType
// Method to determine the MIME type, provided the UID
// -----------------------------------------------------------------------------
void CNSmlAgendaDataStore::DataMimeType( TSmlDbItemUid aUid )
    {
    FLOG(_L("CNSmlAgendaDataStore::DataMimeType: BEGIN"));    
    
    FLOG(_L("CNSmlAgendaDataStore::DataMimeType: LUID '%d'"), aUid );
    
    if( iIsHierarchicalSyncSupported )
        {
        // Find a match with the list of Offset values available
        TInt index = KErrNotFound;
        TInt err = KErrNone;
        TKeyArrayFix key( 0, ECmpTInt ); // Find key for Ids.
        
        err = iCalOffsetArr->Find( aUid, key, index );
                   
        // Determine the Data Mime Type
        if( err == KErrNone )
            {
            iDataMimeType = ENSmlFolder;
            }
        else
            {
            iDataMimeType = ENSmlCalendar;
            }
        }
    else
        {
        iDataMimeType = ENSmlCalendar;
        }
    
    FLOG(_L("CNSmlAgendaDataStore::DataMimeType: END, DataMimeType '%d' "), iDataMimeType);    
    }

// -----------------------------------------------------------------------------
// CNSmlAgendaDataStore::ListAllAgendaPluginAdaptersL
// Get All the AdapterHandler instance
// -----------------------------------------------------------------------------
void CNSmlAgendaDataStore::ListAllAgendaPluginAdaptersL()
    {
    FLOG(_L("CNSmlAgendaDataStore::ListAllAgendaPluginAdaptersL BEGIN"));
    
    RImplInfoPtrArray adapterInfoArray;        
    CNSmlAgendaAdapterHandler* adapterInstance(NULL);
    
    REComSession::ListImplementationsL( KAgendaAdapterHandlerInterfaceUid, adapterInfoArray );
    TInt adapterCount = adapterInfoArray.Count();
    FLOG(_L("adaptercount:'%d'"),adapterCount); 
    CImplementationInformation* adapterInfo = NULL;
    
    for (TInt adapterIndex = 0 ; adapterIndex < adapterCount; adapterIndex++)
        {
        adapterInfo = adapterInfoArray[ adapterIndex ];
        TUid adapterUid = { adapterInfo->ImplementationUid().iUid  };
        FLOG(_L("CNSmlAgendaDataStore::ListAllAgendaPluginAdaptersL before newl"));
        adapterInstance = CNSmlAgendaAdapterHandler::NewL(adapterUid); 
        FLOG(_L("CNSmlAgendaDataStore::ListAllAgendaPluginAdaptersL after newl"));
        if( adapterInstance )
            {
            adapterInstance->iOpaqueData = adapterInfo->OpaqueData().Alloc();
            iAgendaPluginAdapters.AppendL( adapterInstance );
            }
        }
    adapterInfoArray.ResetAndDestroy();
    FLOG(_L("CNSmlAgendaDataStore::ListAllAgendaPluginAdaptersL END"));
    }

// -----------------------------------------------------------------------------
// CNSmlAgendaDataStore::AgendaAdapterHandlerL
// Get the AdapterHandler instance
// -----------------------------------------------------------------------------
void CNSmlAgendaDataStore::AgendaAdapterHandlerL()
    {
    FLOG(_L("CNSmlAgendaDataStore::AgendaAdapterHandlerL BEGIN"));
    
    // Get the Opaque data / server id from the cenrep
    TBuf<KBuffLength> serverid;
    HBufC8* opaquedata;
    CRepository* rep = CRepository::NewLC( KNsmlDsSessionInfoKey );
    TInt err = rep->Get(EDSSessionServerId, serverid );
    FLOG(_L("CNSmlAgendaDataStore:::AgendaAdapterHandlerL():serverid '%S'"), &serverid );
    User::LeaveIfError(err);
    CleanupStack::PopAndDestroy(rep);
    
    if( 0 == serverid.Length() )
        {
        FLOG(_L("CNSmlAgendaDataStore:::AgendaAdapterHandlerL():serverid length is zero"));
        opaquedata = KNSmlDefaultOpaqueData().AllocL();
        }
    else
        {
        opaquedata = HBufC8::NewL( serverid.Length() );
        TPtr8 name( opaquedata->Des() );
        CnvUtfConverter::ConvertFromUnicodeToUtf8(name , serverid );
        }
    iAgendaAdapterHandler = NULL;
    for( TInt adaptercount = 0; adaptercount < iAgendaPluginAdapters.Count(); adaptercount++ )
        {
        if( 0 == opaquedata->Compare(iAgendaPluginAdapters[adaptercount]->iOpaqueData->Des()) )
            {
            iAgendaAdapterHandler = iAgendaPluginAdapters[adaptercount];
            break;
            }        
        }
    
    delete opaquedata;
    
    if( NULL == iAgendaAdapterHandler )
        {
        opaquedata = KNSmlDefaultOpaqueData().AllocL();
        for( TInt adaptercount = 0; adaptercount < iAgendaPluginAdapters.Count(); adaptercount++ )
            {
            if( 0 == opaquedata->Compare(iAgendaPluginAdapters[adaptercount]->iOpaqueData->Des()) )
                {
                iAgendaAdapterHandler = iAgendaPluginAdapters[adaptercount];
                break;
                }        
            }
        delete opaquedata;
        }
    
    FLOG(_L("CNSmlAgendaDataStore::AgendaAdapterHandlerL END"));
    }

// -----------------------------------------------------------------------------
// CNSmlAgendaDataStore::SynchronizableCalEntryIdsL
// Retrieve the CalendarFile and associated entries ID
// -----------------------------------------------------------------------------
void CNSmlAgendaDataStore::SynchronizableCalEntryIdsL( RArray<TCalLocalUid>& aUidArray )
    {
    FLOG(_L("CNSmlAgendaDataStore::SynchronizableCalEntryIdsL BEGIN"));
    
    TInt err(KErrNone);
    
    for( TInt calfilecount = 0; calfilecount < iCalOffsetArr->Count(); calfilecount++ )
        {
        TUint calfileid = iCalOffsetArr->At(calfilecount);
        CNSmlAgendaDataStoreUtil* agendautil = NULL;
        HBufC* calfilename = NULL;
        
        TRAP(err, calfilename = iAgendaAdapterHandler->FolderNameL(calfileid) );
        FLOG(_L("CNSmlAgendaDataStore::SynchronizableCalEntryIdsL calfilename: '%S'"), calfilename);
           
        if( err!= KErrNone || NULL == calfilename)
            {
            FLOG(_L("CNSmlAgendaDataStore::SynchronizableCalEntryIdsL Invalid FileName"));
            User::Leave(KErrGeneral);
            }
        FLOG(_L("CNSmlAgendaDataStore::SynchronizableCalEntryIdsL Initialize"));
       
        agendautil = CNSmlAgendaDataStoreUtil::NewL();
        if( agendautil )
            {
            CleanupStack::PushL(agendautil);
            TRAP(err, agendautil->InitializeCalAPIsL( calfilename ));
            CleanupStack::Pop(agendautil);
            }
        if ( err || !agendautil )
            {
            FLOG(_L("CNSmlAgendaDataStore::SynchronizableCalEntryIdsL Initialization failed"));
            delete calfilename;
            User::Leave(KErrGeneral);
            }
        
        TCalTime zeroTime;
        RArray<TCalLocalUid> entryarray;
        CleanupClosePushL( entryarray );
       
        aUidArray.AppendL(calfileid);            
    
        zeroTime.SetTimeUtcL( Time::NullTTime() );
        agendautil->iEntryView->GetIdsModifiedSinceDateL( zeroTime, entryarray );  
        
        for( TInt i=0; i<entryarray.Count(); i++)
            {
            aUidArray.AppendL(calfileid + entryarray[i]);
            FLOG(_L("CNSmlAgendaDataStore::SynchronizableCalEntryIdsL, Entry ID: '%d'"), (calfileid + entryarray[i]));
            }    
        FLOG(_L("CNSmlAgendaDataStore::SynchronizableCalEntryIdsL Clean up start"));
        CleanupStack::PopAndDestroy( &entryarray );
        delete agendautil;
        delete calfilename;
        FLOG(_L("CNSmlAgendaDataStore::SynchronizableCalEntryIdsL Cleanup end"));
        }    
    FLOG(_L("CNSmlAgendaDataStore::SynchronizableCalEntryIdsL END"));
    }

// -----------------------------------------------------------------------------
// CNSmlAgendaDataStore::StoreFormatL
// Providing DataStore access to CNSmlDataProvider Class
// -----------------------------------------------------------------------------
CSmlDataStoreFormat* CNSmlAgendaDataStore::StoreFormatL()
    {
    return DoOwnStoreFormatL();
    }

// -----------------------------------------------------------------------------
// CNSmlAgendaDataStore::OpenStoreL
// Open the Store if present else create one to work upon
// -----------------------------------------------------------------------------
void CNSmlAgendaDataStore::OpenStoreL()
    {
    FLOG(_L("CNSmlAgendaDataStore::OpenStoreL BEGIN"));

    TInt profileid = NULL;
    TBuf<KBuffLength> serverid;
    TBuf<KBuffLength> profilename;
	TBool isHandlerAvailable( EFalse );
    
    // Get the ServerId, ProfileId and ProfileName from the cenrep
    CRepository* rep = CRepository::NewLC( KNsmlDsSessionInfoKey );
    TInt err = rep->Get( EDSSessionProfileId, profileid );
    err = rep->Get( EDSSessionProfileName, profilename );
    err = rep->Get(EDSSessionServerId, serverid );
    User::LeaveIfError(err);
    CleanupStack::PopAndDestroy(rep);
    
    // Reintialize the StoreName
    if ( iOpenedStoreName )
        {
        delete iOpenedStoreName;
        iOpenedStoreName = NULL;
        }
    
    // Find the CalendarFile having the given ProfileID
    CDesCArray* calfilearr = new (ELeave) CDesCArrayFlat(1);
    CleanupStack::PushL(calfilearr);
    FLOG(_L("CNSmlAgendaDataStore:::OpenStoreL():Profilename '%S'"), &profilename );
    FLOG(_L("CNSmlAgendaDataStore:::OpenStoreL():serverid '%S'"), &serverid );
	
	if( 0 != serverid.Length() )
        {   
		HBufC8* opaquedata = HBufC8::NewL( serverid.Length() );
        TPtr8 name( opaquedata->Des() );
        CnvUtfConverter::ConvertFromUnicodeToUtf8(name , serverid );
		
		for( TInt adaptercount = 0; adaptercount < iAgendaPluginAdapters.Count(); adaptercount++ )
	        {
	        if( 0 == opaquedata->Compare(iAgendaPluginAdapters[adaptercount]->iOpaqueData->Des()) )
	            {
	            FLOG(_L("CNSmlAgendaDataStore::Handler available"));			
				isHandlerAvailable = ETrue;
	            break;
	            }        
	        }
			delete opaquedata;
        }	
    
	if( isHandlerAvailable )
		{
		iOpenedStoreName = iDefaultStoreFileName->AllocL();
		}	
    else if( IsCalFileAvailableL( profileid, calfilearr ) )
        {
        FLOG(_L("CNSmlAgendaDataStore::OpenStoreL Found the assoicated calfile"));
        
        // TODO: Yet to get clarification for enable/disable the notification
        iOpenedStoreName = calfilearr->MdcaPoint(0).AllocL();
        }
    else
        {
        FLOG(_L("CNSmlAgendaDataStore::OpenStoreL CouldNot Find the Calfile"));
        
        // No association found thus creating a CalendarFile
        HBufC* name = profilename.AllocL();
        HBufC* calfilename = CreateCalFileL( name, profileid );
        delete name;
        iOpenedStoreName = calfilename;
        }
        
    CCalSession* calsession = CCalSession::NewL();
    CleanupStack::PushL(calsession);
    TRAP( err, calsession->OpenL( iOpenedStoreName->Des() ) );
    // Disable notifications
    TRAP_IGNORE( calsession->DisablePubSubNotificationsL() );
    TRAP_IGNORE( calsession->DisableChangeBroadcast() );        
    // Get ID of database
    calsession->FileIdL( iOpenedStoreId );
    CleanupStack::PopAndDestroy( calsession );    
      
    CleanupStack::PopAndDestroy(calfilearr);
    FLOG(_L("CNSmlAgendaDataStore::OpenStoreL END"));    
    }

// -----------------------------------------------------------------------------
// CNSmlAgendaDataStore::IsCalFileAvailableL    
// Check existance of CalFile with the given ProfileID associativity
// -----------------------------------------------------------------------------
TBool CNSmlAgendaDataStore::IsCalFileAvailableL( TInt aProfileId, CDesCArray* aCalFileArr )
    {
    FLOG(_L("CNSmlAgendaDataStore::IsCalFileAvailable: BEGIN"));   
    
    TBool calfilestatus(EFalse);
    TBuf8<KBuffLength> keyBuff;
    CCalSession* vCalSession = NULL;
    CCalSession* vCalSubSession = NULL;   
    
    vCalSession = CCalSession::NewL();
    CleanupStack::PushL(vCalSession);
    
    CDesCArray* calfilearr = vCalSession->ListCalFilesL();            
    CleanupStack::PushL(calfilearr);
    
    for(TInt i = 0; i < calfilearr->Count(); i++)
        {
        vCalSubSession = CCalSession::NewL(*vCalSession);
        CleanupStack::PushL(vCalSubSession);
        vCalSubSession->OpenL(calfilearr->MdcaPoint(i));
        
        CCalCalendarInfo* caleninfo = vCalSubSession->CalendarInfoL();
        CleanupStack::PushL(caleninfo);
        
        TInt ProfileId;
        keyBuff.Zero();
        keyBuff.AppendNum( EDeviceSyncProfileID );
        TPckgC<TInt> intBuf(ProfileId);
        TRAPD(err,intBuf.Set(caleninfo->PropertyValueL(keyBuff)));
                    
        if( err != KErrNone )
            {
            FLOG(_L("CNSmlAgendaDataStore::IsCalFileAvailable: Error while retrieving CalFile Property"));
            CleanupStack::PopAndDestroy(caleninfo);   
            CleanupStack::PopAndDestroy(vCalSubSession);             
            continue;
            }
        
        ProfileId = intBuf();
        
        if(aProfileId == ProfileId)
            {
            aCalFileArr->AppendL(calfilearr->MdcaPoint(i));
            }
        CleanupStack::PopAndDestroy(caleninfo);    
        CleanupStack::PopAndDestroy(vCalSubSession); 
        }
    
    if( aCalFileArr->Count() > 0 )
        {
        calfilestatus = ETrue;
        }
    else
        {
        calfilestatus = EFalse;
        }
    CleanupStack::PopAndDestroy(calfilearr);     
    CleanupStack::PopAndDestroy(vCalSession);
    
    FLOG(_L("CNSmlAgendaDataStore::IsCalFileAvailable: END"));    
    return calfilestatus;
    }

// -----------------------------------------------------------------------------
// CNSmlAgendaDataStore::OpenStoreL
// Create CalFile with the attributes provided
// -----------------------------------------------------------------------------
HBufC* CNSmlAgendaDataStore::CreateCalFileL( HBufC* aProfileName, TInt aProfileId )
    {
    FLOG(_L("CNSmlAgendaDataStore::CreateCalFileL: BEGIN"));
    
    TBuf8<KBuffLength> keyBuff;
    TUint calValue = 0;
    CCalSession* calSession = CCalSession::NewL();
    CleanupStack::PushL(calSession);
    CCalCalendarInfo* calinfo = CCalCalendarInfo::NewL();
    CleanupStack::PushL(calinfo);   
    
    //Visibility
    calinfo->SetEnabled(ETrue);
    calinfo->SetNameL(aProfileName->Des());
    // TODO
    calinfo->SetColor(Math::Random());
    
    // Set Meta Data Properties
    // LUID Meta Property
    keyBuff.Zero();
    keyBuff.AppendNum( EFolderLUID );
    calValue = CCalenMultiCalUtil::GetNextAvailableOffsetL();
    FLOG(_L("CNSmlAgendaDataStore::CreateCalFileL: nextoffset: '%d'"), calValue);
    TPckgC<TUint> pckgUidValue( calValue );
    calinfo->SetPropertyL( keyBuff, pckgUidValue );
    
    // Create & Modified Time Meta Property
    keyBuff.Zero();
    keyBuff.AppendNum( ECreationTime );
    TTime time;
    time.HomeTime();
    TPckgC<TTime> pckgCreateTimeValue( time );
    calinfo->SetPropertyL( keyBuff, pckgCreateTimeValue );
    keyBuff.Zero();
    keyBuff.AppendNum( EModificationTime );
    calinfo->SetPropertyL( keyBuff, pckgCreateTimeValue );
    
    // Sync Status
    keyBuff.Zero();
    keyBuff.AppendNum( ESyncStatus );
    TBool syncstatus( ETrue );
    TPckgC<TBool> pckgSyncStatusValue( syncstatus );
    calinfo->SetPropertyL( keyBuff, pckgSyncStatusValue );
    
    // Global UID MetaDataProperty 
    keyBuff.Zero();
    keyBuff.AppendNum( EGlobalUUID );
    HBufC8* guuid = iInterimUtils->GlobalUidL();
    TPtr8 guuidPtr = guuid->Des();
    CleanupStack::PushL( guuid );
    calinfo->SetPropertyL( keyBuff, guuidPtr );
    CleanupStack::PopAndDestroy( guuid );
    
    // Owner
    keyBuff.Zero();
    TInt syncowner = iAgendaAdapterHandler->DeviceSyncOwner(); 
    keyBuff.AppendNum( EDeviceSyncServiceOwner );
    TPckgC<TInt> pckgAppUIDValue( syncowner );    
    calinfo->SetPropertyL( keyBuff, pckgAppUIDValue );
    
    // Profile ID Meta Property
    keyBuff.Zero();
    keyBuff.AppendNum( EDeviceSyncProfileID );
    TPckgC<TInt> pckgProfileIdValue( aProfileId );    
    calinfo->SetPropertyL( keyBuff, pckgProfileIdValue );
       
    // Create the CalFile
    HBufC* calfilename = CCalenMultiCalUtil::GetNextAvailableCalFileL();
    calSession->CreateCalFileL( calfilename->Des(), *calinfo );
    CleanupStack::PopAndDestroy(calinfo);
    CleanupStack::PopAndDestroy(calSession);
    
    FLOG(_L("CNSmlAgendaDataStore::CreateCalFileL: END"));
    
    return calfilename;    
    }

// -----------------------------------------------------------------------------
// CNSmlAgendaDataStore::ActiveItemsL
// Filters out the non-active items from the given array
// -----------------------------------------------------------------------------
CNSmlDataItemUidSet* CNSmlAgendaDataStore::ActiveItemsL( CNSmlDataItemUidSet& aUids ) const
    {
    FLOG(_L("CNSmlAgendaDataStore::ActiveItemsL: BEGIN"));
    
    TInt err(KErrNone);
    TCalLocalUid uid(0);
    TCalLocalUid parentid(0);
    CNSmlDataItemUidSet* tempuids = new ( ELeave ) CNSmlDataItemUidSet();
    CNSmlDataItemUidSet* inactiveuids = new ( ELeave ) CNSmlDataItemUidSet();
    for( TInt count =0; count < aUids.ItemCount(); count++ )
        {
        parentid = 0;
        uid = aUids.ItemAt(count);
        TRAP( err, GetCalendarEntryIdL( parentid, uid ));
        if ( err && iIsHierarchicalSyncSupported )
            {
            FLOG(_L("CNSmlAgendaDataStore::ActiveItemsL: Invalid UID"));
            delete tempuids;
            delete inactiveuids;
            User::Leave( KErrGeneral );
            }                
        if( iAgendaAdapterHandler->FolderSyncStatusL( parentid ) )
            {
            FLOG(_L("CNSmlAgendaDataStore::ActiveItemsL: ActiveItem: '%d'"), aUids.ItemAt(count));
            tempuids->AddItem( aUids.ItemAt(count) );
            }
        else
            {
            FLOG(_L("CNSmlAgendaDataStore::ActiveItemsL: InActiveItem: '%d'"), aUids.ItemAt(count));
            inactiveuids->AddItem( aUids.ItemAt(count) );
            }
        }
    if( inactiveuids->ItemCount() > 0 )
        {
        aUids.Reset();
        for( TInt count =0; count < tempuids->ItemCount(); count++ )
            {
            aUids.AddItem( tempuids->ItemAt(count) );
            }
        delete tempuids;
        FLOG(_L("CNSmlAgendaDataStore::ActiveItemsL: END"));
        return inactiveuids;
        }
    else
        {
        delete tempuids;
        delete inactiveuids;
        FLOG(_L("CNSmlAgendaDataStore::ActiveItemsL: END"));
        return NULL;
        }
    }


// -----------------------------------------------------------------------------
// CNSmlAgendaDataStore::ExternalizeCommittedUidL
// Populate iCommittedUidArr from the Stream
// -----------------------------------------------------------------------------
void CNSmlAgendaDataStore::ExternalizeCommittedUidL() const
    {
    FLOG(_L("CNSmlAgendaDataStore::ExternalizeCommittedUidL: BEGIN"));
    
    TUid uid = {KNSmlAgendaAdapterStreamUid};
    RWriteStream writeStream;
    
    // Open stream for writing
    iAgendaAdapterLog->iSyncRelationship.OpenWriteStreamLC(writeStream, uid);
      
    // Write the snapshot to the stream
    TInt itemCount = iCommittedUidArr->ItemCount();
    FLOG(_L("CNSmlAgendaDataStore::ExternalizeCommittedUidL: count '%d'"), itemCount);
    writeStream.WriteInt32L(itemCount);
    
    if( itemCount > 0 )
        {    
        for (TInt i = 0; i < itemCount; ++i)
            {
            writeStream.WriteInt32L(iCommittedUidArr->ItemAt(i));
            FLOG(_L("CNSmlAgendaDataStore::ExternalizeCommittedUidL: item '%d'"), iCommittedUidArr->ItemAt(i));
            }        
        }
    writeStream.CommitL();
    CleanupStack::PopAndDestroy(); // writeStream
    FLOG(_L("CNSmlAgendaDataStore::ExternalizeCommittedUidL: END"));
    }

// -----------------------------------------------------------------------------
// CNSmlAgendaDataStore::InternalizeCommittedUidL
// Write the contents of iCommittedUidArr to the Stream
// -----------------------------------------------------------------------------
void CNSmlAgendaDataStore::InternalizeCommittedUidL() const
    {
    FLOG(_L("CNSmlAgendaDataStore::InternalizeCommittedUidL: BEGIN"));
  
    TUid uid = {KNSmlAgendaAdapterStreamUid};        
    TBool hashistory = iAgendaAdapterLog->iSyncRelationship.IsStreamPresentL(uid);
    
    if (hashistory)
        {
        FLOG(_L("CNSmlAgendaDataStore::InternalizeCommittedUidL: HasHistory"));
        
        // Open stream for reading
        RReadStream readStream;
        iAgendaAdapterLog->iSyncRelationship.OpenReadStreamLC(readStream, uid);
        
        // Read snapshot from the stream
        TSmlDbItemUid item;
        TInt itemCount(readStream.ReadInt32L());
        FLOG(_L("CNSmlAgendaDataStore::InternalizeCommittedUidL: count '%d'"), itemCount);
        iCommittedUidArr->Reset();
        for (TInt i = 0; i < itemCount; ++i)
            {
            item = readStream.ReadInt32L();
            FLOG(_L("CNSmlAgendaDataStore::InternalizeCommittedUidL: item '%d'"), item);
            iCommittedUidArr->AddItem(item);
            }        
        CleanupStack::PopAndDestroy(); // readStream
        }

    FLOG(_L("CNSmlAgendaDataStore::InternalizeCommittedUidL: END"));
    }

// -----------------------------------------------------------------------------
// CNSmlAgendaDataStoreUtil::CNSmlAgendaDataStoreUtil
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CNSmlAgendaDataStoreUtil::CNSmlAgendaDataStoreUtil() :
    iCalSession( NULL ),
    iExporter( NULL ),
    iImporter( NULL ),
    iProgressView( NULL ),
    iEntryView( NULL ),
    iEntry( NULL ),
    iFileName( NULL )
    {
    FLOG(_L("CNSmlAgendaDataStoreUtil::CNSmlAgendaDataStoreUtil(): BEGIN"));
    
    FLOG(_L("CNSmlAgendaDataStoreUtil::CNSmlAgendaDataStoreUtil(): END"));
    }

// -----------------------------------------------------------------------------
// CNSmlAgendaDataStoreUtil::InitializeCalAPIsL
// Initialize Calendar APIs for database access using the provided name and id
// -----------------------------------------------------------------------------
//
void CNSmlAgendaDataStoreUtil::InitializeCalAPIsL( HBufC* aFileName, TSmlDbItemUid aUid )
    {
    FLOG(_L("CNSmlAgendaDataStoreUtil::InitializeCalAPIsL: BEGIN"));
    
    iCalSession = CCalSession::NewL();
    iCalSession->OpenL(aFileName->Des());        
    iExporter = CCalenExporter::NewL( *iCalSession );
    iImporter = CCalenImporter::NewL( *iCalSession );
    iProgressView = CNSmlAgendaProgressview::NewL();
    iEntryView = CCalEntryView::NewL( *iCalSession, *iProgressView );
    CActiveScheduler::Start();
    TInt completedStatus = iProgressView->GetCompletedStatus();
    if ( completedStatus != KErrNone )
        {
        FLOG(_L("CNSmlAgendaDataStoreUtil::ConstructL: ERROR in Progressview"));
        }
    if( aUid != NULL )
        {
        iEntry = iEntryView->FetchL( aUid );
        if( NULL == iEntry )
            {
            User::Leave( KErrNotFound );
            }
        
        }    
    FLOG(_L("CNSmlAgendaDataStoreUtil::InitializeCalAPIsL: END"));
    }

// -----------------------------------------------------------------------------
// CNSmlAgendaDataStoreUtil::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CNSmlAgendaDataStoreUtil* CNSmlAgendaDataStoreUtil::NewL()
    {
    FLOG(_L("CNSmlAgendaDataStoreUtil::NewL: BEGIN"));
    
    CNSmlAgendaDataStoreUtil* self = new ( ELeave ) CNSmlAgendaDataStoreUtil();
    
    FLOG(_L("CNSmlAgendaDataStoreUtil::NewL: END"));
    return self;
    }

// -----------------------------------------------------------------------------
// CNSmlAgendaDataStoreUtil::~CNSmlAgendaDataStoreUtil
// Destructor.
// -----------------------------------------------------------------------------
//
CNSmlAgendaDataStoreUtil::~CNSmlAgendaDataStoreUtil()
    {
    FLOG(_L("CNSmlAgendaDataStoreUtil::~CNSmlAgendaDataStoreUtil(): BEGIN"));

    delete iEntry;
    delete iEntryView;
    delete iExporter;
    delete iImporter;
    delete iCalSession;
    delete iProgressView;
    
    FLOG(_L("CNSmlAgendaDataStoreUtil::~CNSmlAgendaDataStoreUtil(): END"));
    }

// -----------------------------------------------------------------------------
// CNSmlAgendaAdapterLog::CNSmlAgendaAdapterLog
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CNSmlAgendaAdapterLog::CNSmlAgendaAdapterLog( MSmlSyncRelationship& aSyncRelationship ) :
    iSyncRelationship( aSyncRelationship )
    {
    FLOG(_L("CNSmlAgendaAdapterLog::CNSmlAgendaAdapterLog(): BEGIN"));
    
    FLOG(_L("CNSmlAgendaAdapterLog::CNSmlAgendaAdapterLog(): END"));
    }

// -----------------------------------------------------------------------------
// CNSmlAgendaAdapterLog::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CNSmlAgendaAdapterLog* CNSmlAgendaAdapterLog::NewL( MSmlSyncRelationship& aSyncRelationship )
    {
    FLOG(_L("CNSmlAgendaAdapterLog::NewL: BEGIN"));
    
    CNSmlAgendaAdapterLog* self = new ( ELeave ) CNSmlAgendaAdapterLog(aSyncRelationship);
    
    FLOG(_L("CNSmlAgendaAdapterLog::NewL: END"));
    return self;
    }

// -----------------------------------------------------------------------------
// CNSmlAgendaAdapterLog::~CNSmlAgendaAdapterLog
// Destructor.
// -----------------------------------------------------------------------------
//
CNSmlAgendaAdapterLog::~CNSmlAgendaAdapterLog()
    {
    FLOG(_L("CNSmlAgendaAdapterLog::~CNSmlAgendaAdapterLog(): BEGIN"));
    
    FLOG(_L("CNSmlAgendaAdapterLog::~CNSmlAgendaAdapterLog(): END"));
    }

   
//  End of File  
