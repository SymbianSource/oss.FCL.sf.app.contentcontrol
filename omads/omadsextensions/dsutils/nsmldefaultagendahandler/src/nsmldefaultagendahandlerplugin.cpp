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
* Description:  
*
*/

#include <gdi.h>
#include <utf.h>
#include <barsc.h>
#include <e32std.h>
#include <bautils.h>
#include <implementationproxy.h>
#include <calsession.h>
#include <calenmulticalutil.h>
#include <calenmulticaluids.hrh>
#include <calcalendarinfo.h>
#include <nsmlfolderparser.h>
#include <nsmldefaultagendahandler_1_2.rsg>
#include "nsmldefaultagendahandlerplugin.h"
#include "nsmldefaultagendahandlerutil.h"
#include "nsmldefaultagendahandlerdebug.h"



// CONSTANTS
const TInt KBuffLength = 128;

// ----------------------------------------------------------------------------
// CNSmlDefaultAgendaHandlerPlugin::NewL
// Two Phase Construction
// ----------------------------------------------------------------------------
EXPORT_C CNSmlDefaultAgendaHandlerPlugin* CNSmlDefaultAgendaHandlerPlugin::NewL()
	{
	FLOG( _L("CNSmlDefaultAgendaHandlerPlugin::NewL, BEGIN"));

	CNSmlDefaultAgendaHandlerPlugin* self = new ( ELeave ) CNSmlDefaultAgendaHandlerPlugin();	
	self->ConstructL();
	
	FLOG( _L("CNSmlDefaultAgendaHandlerPlugin::NewL, END"));
	return self;
	}

// ----------------------------------------------------------------------------
// CNSmlDefaultAgendaHandlerPlugin::~CNSmlDefaultAgendaHandlerPlugin
// Destructor
// ----------------------------------------------------------------------------
CNSmlDefaultAgendaHandlerPlugin::~CNSmlDefaultAgendaHandlerPlugin()
	{
	FLOG( _L("CNSmlDefaultAgendaHandlerPlugin::~CNSmlDefaultAgendaHandlerPlugin, BEGIN"));	 
  
	FLOG( _L("CNSmlDefaultAgendaHandlerPlugin::~CNSmlDefaultAgendaHandlerPlugin, END"));
	}

// ----------------------------------------------------------------------------  
// C++ constructor - not exported;
// implicitly called from NewL()
// ----------------------------------------------------------------------------  
CNSmlDefaultAgendaHandlerPlugin::CNSmlDefaultAgendaHandlerPlugin() 
	{
	FLOG( _L("CNSmlDefaultAgendaHandlerPlugin::CNSmlDefaultAgendaHandlerPlugin, BEGIN"));  
	FLOG( _L("CNSmlDefaultAgendaHandlerPlugin::CNSmlDefaultAgendaHandlerPlugin, END"));
	}

// ----------------------------------------------------------------------------  
// 2nd phase constructor
// ----------------------------------------------------------------------------       
void CNSmlDefaultAgendaHandlerPlugin::ConstructL()
    {
    FLOG( _L("CNSmlDefaultAgendaHandlerPlugin::ConstructL, BEGIN"));
    
    // Initialize server supports for folder
    iServerSupportsForFolder.iSSName = EFalse;
    iServerSupportsForFolder.iSSCreated = EFalse;
    iServerSupportsForFolder.iSSModified = EFalse;
        
    FLOG( _L("CNSmlDefaultAgendaHandlerPlugin::ConstructL, END"));
    }

// -----------------------------------------------------------------------------
// CNSmlDefaultAgendaHandlerPlugin::CheckServerSupportForFolder
// Method to check if needed properties are supported by remote server.
// -----------------------------------------------------------------------------
void CNSmlDefaultAgendaHandlerPlugin::CheckServerSupportForFolder( const CSmlDataStoreFormat& aServerDataStoreFormat )
    {
    FLOG(_L("CNSmlDefaultAgendaHandlerPlugin::CheckServerSupportForFolder: BEGIN"));
        
    iServerSupportsForFolder.iSSName = EFalse;
    iServerSupportsForFolder.iSSCreated = EFalse;
    iServerSupportsForFolder.iSSModified = EFalse;
    
    if ( aServerDataStoreFormat.FolderPropertyCount() == 0 )
        {
        // Server has no restricition on folder property
        iServerSupportsForFolder.iSSName = ETrue;
        iServerSupportsForFolder.iSSCreated = ETrue;
        iServerSupportsForFolder.iSSModified = ETrue;
        return;
        }
    else
        {
        for ( TInt i = 0; 
                   i < aServerDataStoreFormat.FolderPropertyCount(); 
                   i++ )
            {
            if ( aServerDataStoreFormat.FolderProperty( i ).Field().Name().DesC().Compare( KNSmlDataObjectNAME ) == 0 )
                {
                FLOG(_L("CNSmlDefaultAgendaHandlerPlugin::CheckServerSupportForFolder: Supports NAME Property"));
                iServerSupportsForFolder.iSSName = ETrue;
                }
            else if ( aServerDataStoreFormat.FolderProperty( i ).Field().Name().DesC().Compare( KNSmlDataObjectCREATED ) == 0 )
                {
                FLOG(_L("CNSmlDefaultAgendaHandlerPlugin::CheckServerSupportForFolder: Supports CREATED Property"));
                iServerSupportsForFolder.iSSCreated = ETrue;
                }
            else if ( aServerDataStoreFormat.FolderProperty( i ).Field().Name().DesC().Compare( KNSmlDataObjectMODIFIED ) == 0 )
                {
                FLOG(_L("CNSmlDefaultAgendaHandlerPlugin::CheckServerSupportForFolder: Supports MODIFIED Property"));
                iServerSupportsForFolder.iSSModified = ETrue;
                }
            }
        }
    FLOG(_L("CNSmlDefaultAgendaHandlerPlugin::CheckServerSupportForFolder: END"));
    }
         
// -----------------------------------------------------------------------------
// CNSmlDefaultAgendaHandlerPlugin::FetchFolderL
// Method to read folder item from database.
// -----------------------------------------------------------------------------
void CNSmlDefaultAgendaHandlerPlugin::FetchFolderL( const TSmlDbItemUid aUid, RBufWriteStream& aStream )
    {
    FLOG(_L("CNSmlDefaultAgendaHandlerPlugin::FetchFolderL: begin"));
            
    // Create parser and set properties
    CNSmlFolderParser* parser = CNSmlFolderParser::NewLC();
    TNSmlParserGeneratorError parseError = EErrorNone;
    HBufC8* generatedBuf = NULL;
    HBufC8* nameBuf = NULL;
    
    //Get the Folder Information
    CNSmlDefaultAgendaHandlerUtil* agendautil = CalendarInfoL(aUid);
    if( NULL == agendautil )
        {
        FLOG(_L("CNSmlDefaultAgendaHandlerPlugin::FetchFolderL:Invalid CalendarInfo"));
        CleanupStack::PopAndDestroy( parser );
        User::Leave( KErrNotFound );
        }
        
    if ( iServerSupportsForFolder.iSSCreated )
        {
        parser->iCreated = agendautil->iCreated;
        }
    else
        {
        parser->iCreated = Time::NullTTime();
        }
    
    if ( iServerSupportsForFolder.iSSName )
        {
        FLOG(_L("CNSmlDefaultAgendaHandlerPlugin::FetchFolderL: retrieving name"));
        nameBuf = HBufC8::NewL( agendautil->iName->Length() );
        TPtr8 name( nameBuf->Des() );
        CnvUtfConverter::ConvertFromUnicodeToUtf8(name , agendautil->iName->Des());
        FLOG(_L("CNSmlDefaultAgendaHandlerPlugin::FetchFolderL: name converted"));
        parser->iName = nameBuf;        
        }    

    // Generate data and write it to buffer   
    generatedBuf = HBufC8::NewLC( 1 );
    generatedBuf->Des() = KNullDesC8;
    CleanupStack::Pop(); // generatedBuf
    parseError = parser->GenerateXml( generatedBuf );
    CleanupStack::PushL( generatedBuf );
    
    aStream.WriteL( generatedBuf->Des() );
    
    delete agendautil;
    CleanupStack::PopAndDestroy( generatedBuf ); 
    CleanupStack::PopAndDestroy( parser );
    
    if ( parseError != EErrorNone )
        {       
        FLOG(_L("CNSmlDefaultAgendaHandlerPlugin::FetchFolderL(): end with leave"));
        User::Leave( KErrGeneral );
        }
    FLOG(_L("CNSmlDefaultAgendaHandlerPlugin::FetchFolderL: end"));
    }

// -----------------------------------------------------------------------------
// CNSmlDefaultAgendaHandlerPlugin::SynchronizableCalendarIdsL
// Retrieve the CalendarFile ID
// -----------------------------------------------------------------------------
void CNSmlDefaultAgendaHandlerPlugin::SynchronizableCalendarIdsL( CArrayFixFlat<TUint>* aFolderUidArray )
    {
    FLOG(_L("CNSmlDefaultAgendaHandlerPlugin::SynchronizableCalendarIdsL BEGIN"));
        
    CCalSession* vCalSession = NULL;
    CCalSession* vCalSubSession = NULL;   
    TBuf8<KBuffLength> keyBuff;
    
    aFolderUidArray->Reset();
            
    vCalSession = CCalSession::NewL();
    CleanupStack::PushL(vCalSession);
    
    CDesCArray* calfilearr = vCalSession->ListCalFilesL();            
    CleanupStack::PushL(calfilearr);
    
    for(TInt i = 0; i < calfilearr->Count(); i++)
        {
        TInt err = KErrNone;
        vCalSubSession = CCalSession::NewL();
        CleanupStack::PushL(vCalSubSession);
        vCalSubSession->OpenL(calfilearr->MdcaPoint(i));
        
        CCalCalendarInfo* caleninfo = vCalSubSession->CalendarInfoL(); 
        CleanupStack::PushL(caleninfo);
        
        //Get UID value associated with the CalFile
        keyBuff.Zero();
        TUint UidVal;
        keyBuff.AppendNum( EFolderLUID );
        TPckgC<TUint> pckUidValue(UidVal);
        TRAP(err,pckUidValue.Set(caleninfo->PropertyValueL( keyBuff )));
        UidVal = pckUidValue();
                    
        if( err != KErrNone )
            {
            FLOG(_L("CNSmlDefaultAgendaHandlerPlugin::SynchronizableCalendarIdsL: Error while retrieving CalFile Property: '%d'"), err);
            }
        
        //Get OWNER value associated with the CalFile
        keyBuff.Zero();
        TInt SyncServiceOwnerVal;
        keyBuff.AppendNum( EDeviceSyncServiceOwner );
        TPckgC<TInt> pckOwnerValue(SyncServiceOwnerVal);
        TRAP(err,pckOwnerValue.Set(caleninfo->PropertyValueL( keyBuff )));
        SyncServiceOwnerVal = pckOwnerValue();
                    
        if( err == KErrNotFound )
            {
            // Set DS Application as owner
            SyncServiceOwnerVal = KCRUidNSmlDSApp.iUid;
            TPckgC<TInt> pckOwnerValue(SyncServiceOwnerVal);
            caleninfo->SetPropertyL( keyBuff, pckOwnerValue );
            vCalSubSession->SetCalendarInfoL( *caleninfo );
            FLOG(_L("CNSmlDefaultAgendaHandlerPlugin::SynchronizableCalendarIdsL: Empty Owner LUID '%d'"), UidVal );
            aFolderUidArray->AppendL(UidVal);
            }
        else if( err == KErrNone )
            {
            if( KCRUidNSmlDSApp.iUid == SyncServiceOwnerVal )
                {
                FLOG(_L("CNSmlDefaultAgendaHandlerPlugin::SynchronizableCalendarIdsL: DS Owner LUID '%d'"), UidVal );
                aFolderUidArray->AppendL(UidVal);
                }
            }
        else
            {
            FLOG(_L("CNSmlDefaultAgendaHandlerPlugin::SynchronizableCalendarIdsL: Error while retrieving Owner Property: '%d'"), err);
            }
        CleanupStack::PopAndDestroy(caleninfo);
        CleanupStack::PopAndDestroy(vCalSubSession); 
        }
    CleanupStack::PopAndDestroy(calfilearr);
    CleanupStack::PopAndDestroy(vCalSession);
    
    FLOG(_L("CNSmlDefaultAgendaHandlerPlugin::SynchronizableCalendarIdsL END"));
    }
 
// -----------------------------------------------------------------------------
// CNSmlDefaultAgendaHandlerPlugin::CreateFolderItemL
// Creates the Folder item with the stream provided 
// -----------------------------------------------------------------------------
TCalLocalUid CNSmlDefaultAgendaHandlerPlugin::CreateFolderL( RBufReadStream& aStream )
    {
    FLOG(_L("CNSmlDefaultAgendaHandlerPlugin::CreateFolderItemL BEGIN"));
   
    TBuf8<KBuffLength> keyBuff;
    TUint calValue = 0;
    CNSmlDefaultAgendaHandlerUtil* agendautil = NULL;
    CCalCalendarInfo* calinfo = CCalCalendarInfo::NewL();
    CleanupStack::PushL(calinfo);    
    
    // Parse the buffer for the folder data and construct the calendar info   
    agendautil = ParseFolderDataL( aStream );
    
    if( agendautil )
        {
        // Visibility
        calinfo->SetEnabled( agendautil->iEnabled );
        
        // LUID Meta Property
        keyBuff.Zero();
        keyBuff.AppendNum( EFolderLUID );
        calValue = CCalenMultiCalUtil::GetNextAvailableOffsetL();
        FLOG(_L("CNSmlDefaultAgendaHandlerPlugin::CreateFolderItemL: nextoffset: '%d'"), calValue);
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
        
        // Sync Status Meta Property
        keyBuff.Zero();
        keyBuff.AppendNum( ESyncStatus );
        TBool syncstatus( ETrue );
        TPckgC<TBool> pckgSyncStatusValue( syncstatus );
        calinfo->SetPropertyL( keyBuff, pckgSyncStatusValue );
        
        // Add
        CCalSession* calsession = CCalSession::NewL();
        TInt err(KErrNone);
        if( agendautil->iName != NULL )
            {
            calinfo->SetNameL( agendautil->iName->Des() );
			HBufC* calfilename = CCalenMultiCalUtil::GetNextAvailableCalFileL();
            TRAP( err, calsession->CreateCalFileL( *calfilename, *calinfo ) );
			delete calfilename;
			}
        else
            {
            err = KErrNotFound;
            }
        
        delete calsession;
        delete agendautil;
        CleanupStack::PopAndDestroy(calinfo);
        
        if( err != KErrNone )
            {
            FLOG(_L("CNSmlDefaultAgendaHandlerPlugin::CreateFolderItemL: Error while Creating Calfile"));
            User::Leave( KErrGeneral );
            }
        }
    else
        {
        CleanupStack::PopAndDestroy(calinfo);
        User::Leave( KErrGeneral );
        }
    FLOG(_L("CNSmlDefaultAgendaHandlerPlugin::CreateFolderItemL END"));
    return calValue;   
    }
 
// -----------------------------------------------------------------------------
// CNSmlDefaultAgendaHandlerPlugin::ReplaceFolderItemL
// Replaces the Folder item with the ID provided 
// -----------------------------------------------------------------------------
void CNSmlDefaultAgendaHandlerPlugin::ReplaceFolderL( const TCalLocalUid& aUid, 
                                                      RBufReadStream& aStream, TBool& aSyncStatus )
    {
    FLOG(_L("CNSmlDefaultAgendaHandlerPlugin::ReplaceFolderItemL: BEGIN"));
    
    TBuf8<KBuffLength> keyBuff;
    CNSmlDefaultAgendaHandlerUtil* agendautil = NULL;
    
    // Parse the buffer for the folder data and construct the calendar info   
    agendautil = ParseFolderDataL( aStream );
    
    if( agendautil )
        {
        //Get the associated Calendar File 
        CNSmlDefaultAgendaHandlerUtil* updateagendautil = CalendarInfoL( aUid );
        FLOG(_L("CNSmlDefaultAgendaHandlerPlugin::ReplaceFolderItemL: After CalendarInfoL '%d'"), updateagendautil);
        
        if( NULL == updateagendautil )
            {
            FLOG(_L("CNSmlDefaultAgendaHandlerPlugin::ReplaceFolderItemL:Invalid Updatable CalendarInfo"));
            delete agendautil;
            User::Leave( KErrNotFound );
            }
        FLOG(_L("CNSmlDefaultAgendaHandlerPlugin::ReplaceFolderItemL:valid CalendarInfo"));
        if( updateagendautil->iFileName != NULL )            
            {            
            FLOG(_L("CNSmlDefaultAgendaHandlerPlugin::ReplaceFolderItemL:valid name '%S'"), updateagendautil->iFileName);
            CCalSession* vCalSession = CCalSession::NewL();
            CleanupStack::PushL(vCalSession);
            FLOG(_L("CNSmlDefaultAgendaHandlerPlugin::ReplaceFolderItemL:Open the session")); 
            vCalSession->OpenL(updateagendautil->iFileName->Des());          
            FLOG(_L("CNSmlDefaultAgendaHandlerPlugin::ReplaceFolderItemL:Get the Calendar Info")); 
            CCalCalendarInfo* caleninfo = vCalSession->CalendarInfoL();  
            CleanupStack::PushL(caleninfo);
            FLOG(_L("CNSmlDefaultAgendaHandlerPlugin::ReplaceFolderItemL:Set the status"));
            caleninfo->SetEnabled(agendautil->iEnabled);            
            FLOG(_L("CNSmlDefaultAgendaHandlerPlugin::ReplaceFolderItemL:Set the name"));
            caleninfo->SetNameL(agendautil->iName->Des());
            
            // Modified Time Meta Property
            keyBuff.Zero();
            keyBuff.AppendNum( EModificationTime );
            TTime time;
            time.HomeTime();
            TPckgC<TTime> pckgCreateTimeValue( time );
            caleninfo->SetPropertyL( keyBuff, pckgCreateTimeValue );
            
            aSyncStatus = updateagendautil->iSyncStatus;
            
            // Update
            vCalSession->SetCalendarInfoL(*caleninfo);
            
            CleanupStack::PopAndDestroy(caleninfo);
            CleanupStack::PopAndDestroy( vCalSession );
            }
        else
            {
            FLOG(_L("CNSmlDefaultAgendaHandlerPlugin::ReplaceFolderItemL:Invalid Updatable CalendarInfo Name"));
            delete agendautil;
            delete updateagendautil;
            User::Leave( KErrNotFound );
            }
        }
    else
        {
        FLOG(_L("CNSmlDefaultAgendaHandlerPlugin::ReplaceFolderItemL:Invalid Parsed CalendarInfo"));
        User::Leave( KErrNotFound );
        }
    
    FLOG(_L("CNSmlDefaultAgendaHandlerPlugin::ReplaceFolderItemL: END"));
    }

// -----------------------------------------------------------------------------
// CNSmlDefaultAgendaHandlerPlugin::FolderNameL
// Returns the FolderName for the given ID
// -----------------------------------------------------------------------------
HBufC* CNSmlDefaultAgendaHandlerPlugin::FolderNameL( TSmlDbItemUid aUid )
    {
    FLOG(_L("CNSmlDefaultAgendaHandlerPlugin::FolderNameL: BEGIN"));
    
    HBufC* calfilename = NULL;
    CNSmlDefaultAgendaHandlerUtil* agendautil = CalendarInfoL( aUid );
    
    if( agendautil != NULL )
        {
        calfilename = agendautil->iFileName->Alloc();
        FLOG(_L("CNSmlDefaultAgendaHandlerPlugin::foldername: '%S'"), calfilename);
        }    
    delete agendautil;
    
    FLOG(_L("CNSmlDefaultAgendaHandlerPlugin::FolderNameL: END"));
    
    return calfilename;
    }

// -----------------------------------------------------------------------------
// CNSmlDefaultAgendaHandlerPlugin::StoreFormatL
// Returns adapters supported store format which is read from Calendar Plug
// In Adapter own resource file.
// -----------------------------------------------------------------------------
CSmlDataStoreFormat* CNSmlDefaultAgendaHandlerPlugin::StoreFormatL( RStringPool& aStringPool )
    {
    FLOG(_L("CNSmlDefaultAgendaHandlerPlugin::StoreFormatL: BEGIN"));
    
    TFileName fileName;
    TParse* parse = new(ELeave) TParse();
    CleanupStack::PushL( parse );
    
    parse->Set( KNSmlDefaultAgendaHandlerRsc_1_2,
                &KDC_RESOURCE_FILES_DIR, NULL );
    
    fileName = parse->FullName();
    RResourceFile resourceFile;
    RFs rfs;
    User::LeaveIfError(rfs.Connect());
    
    BaflUtils::NearestLanguageFile( rfs, fileName );

    TRAPD( leavecode, resourceFile.OpenL( rfs,fileName ) );
    if ( leavecode != 0 )
        {
        CleanupStack::PopAndDestroy(); // parse
        rfs.Close();
        FLOG(_L("CNSmlDefaultAgendaHandlerPlugin:::StoreFormatL(): Error END"));
        User::Leave( leavecode );
        }
    
    CleanupClosePushL( resourceFile );
    HBufC8* buffer = resourceFile.AllocReadLC( NSML_DEFAULT_AGENDA_HANDLER );
    TResourceReader reader;
    reader.SetBuffer( buffer );

    CSmlDataStoreFormat* dsFormat = NULL;
    dsFormat = CSmlDataStoreFormat::NewLC( aStringPool, reader );
    CleanupStack::Pop(); // dsFormat
    CleanupStack::PopAndDestroy( 3 ); // buffer, resourceFile, parse
    rfs.Close();
    
    FLOG(_L("CNSmlDefaultAgendaHandlerPlugin::StoreFormatL: END"));
    return dsFormat;
    }

// -----------------------------------------------------------------------------
// CNSmlDefaultAgendaHandlerPlugin::DeviceSyncOwner
// Returns the DS Application UID
// -----------------------------------------------------------------------------
TInt CNSmlDefaultAgendaHandlerPlugin::DeviceSyncOwner()
    {
    FLOG(_L("CNSmlDefaultAgendaHandlerPlugin::DeviceSyncOwner: BEGIN"));
    FLOG(_L("CNSmlDefaultAgendaHandlerPlugin::DeviceSyncOwner: '%d' END"), KCRUidNSmlDSApp.iUid );
    return KCRUidNSmlDSApp.iUid;
    }

// -----------------------------------------------------------------------------
// CNSmlDefaultAgendaHandlerPlugin::FolderSyncStatusL
// Returns the Sync Status for the given UID
// -----------------------------------------------------------------------------
TBool CNSmlDefaultAgendaHandlerPlugin::FolderSyncStatusL( TSmlDbItemUid aUid )
    {
    FLOG(_L("CNSmlDefaultAgendaHandlerPlugin::FolderSyncStatusL: BEGIN"));
    
    TBool syncstatus( EFalse );
    CNSmlDefaultAgendaHandlerUtil* agendautil = CalendarInfoL( aUid );        
    if( agendautil != NULL )
        {
        syncstatus = agendautil->iSyncStatus;
        FLOG(_L("CNSmlDefaultAgendaHandlerPlugin::syncstatus: '%d'"), syncstatus );
        }    
    delete agendautil;
        
    FLOG(_L("CNSmlDefaultAgendaHandlerPlugin::FolderSyncStatusL: END"));
    
    return syncstatus;
    }

// -----------------------------------------------------------------------------
// CNSmlDefaultAgendaHandlerPlugin::FolderSyncStatusL
// Returns the Sync Status for the given FolderName
// -----------------------------------------------------------------------------
TBool CNSmlDefaultAgendaHandlerPlugin::FolderSyncStatusL( HBufC* aFolderName )
    {
    FLOG(_L("CNSmlDefaultAgendaHandlerPlugin::FolderSyncStatusL: BEGIN"));
    
    TBool syncstatus( EFalse );
    TBuf8<KBuffLength> keyBuff;    
    CCalSession *calsession = CCalSession::NewL();
    CleanupStack::PushL(calsession);
    calsession->OpenL(aFolderName->Des());    
    CCalCalendarInfo* caleninfo = calsession->CalendarInfoL();
    CleanupStack::PushL(caleninfo);
    // Sync Status
    keyBuff.Zero();
    keyBuff.AppendNum( ESyncStatus ); 
    TPckgC<TBool> pckgSyncStatusValue(syncstatus);
    FLOG(_L("CNSmlDefaultAgendaHandlerPlugin::FolderSyncStatusL: Get sync status"));
    TRAPD(err,pckgSyncStatusValue.Set(caleninfo->PropertyValueL( keyBuff )));
                
    syncstatus = pckgSyncStatusValue();
    if( err != KErrNone )
        {
        FLOG(_L("CNSmlDefaultAgendaHandlerPlugin::FolderSyncStatusL: Error while retrieving CalFile SYNC Property"));
        }
    CleanupStack::PopAndDestroy(caleninfo);
    CleanupStack::PopAndDestroy(calsession);
        
    
    FLOG(_L("CNSmlDefaultAgendaHandlerPlugin::FolderSyncStatusL: END"));
    
    return syncstatus;
    }

// -----------------------------------------------------------------------------
// CNSmlDefaultAgendaHandlerPlugin::CreateFolderSnapShotItemL
// Create the Folder SnapShot Item
// -----------------------------------------------------------------------------
TNSmlSnapshotItem CNSmlDefaultAgendaHandlerPlugin::CreateFolderSnapShotItemL( const TCalLocalUid& aUid )
    {
    FLOG(_L("CNSmlDefaultAgendaHandlerPlugin::CreateFolderSnapShotItemL: BEGIN"));
    
    TNSmlSnapshotItem item( 0 );
    
    CNSmlDefaultAgendaHandlerUtil* agendautil = CalendarInfoL( aUid );
    if( NULL == agendautil )
        {
        FLOG(_L("CNSmlDefaultAgendaHandlerPlugin::CreateFolderSnapShotItemL:Invalid CalendarInfo"));
        return item;
        }
    item.SetItemId( aUid );   
    item.SetLastChangedDate(agendautil->iModified);
    item.SetSoftDelete(EFalse);
    delete agendautil; 
    FLOG(_L("CNSmlDefaultAgendaHandlerPlugin::CreateFolderSnapShotItemL: END"));
    
    return item; 
    }

// -----------------------------------------------------------------------------
// CNSmlDefaultAgendaHandlerPlugin::ParseFolderDataL
// Method to Parse the Folder Data Obtained
// -----------------------------------------------------------------------------
CNSmlDefaultAgendaHandlerUtil* CNSmlDefaultAgendaHandlerPlugin::ParseFolderDataL( RBufReadStream& aStream )
    {
    FLOG(_L("CNSmlDefaultAgendaHandlerPlugin::ParseFolderDataL: BEGIN"));
        
    CNSmlDefaultAgendaHandlerUtil* agendautil = CNSmlDefaultAgendaHandlerUtil::NewL();
    
    // Create parser and parse data from Buffer
    CNSmlFolderParser* folderparser = CNSmlFolderParser::NewLC();
    TParse name;
         
    TInt size = aStream.Source()->SizeL();
    FLOG(_L("CNSmlDefaultAgendaHandlerPlugin::ParseFolderDataL: size:'%d'"), size);
    CBufFlat* buffer = CBufFlat::NewL( 64 );
    CleanupStack::PushL( buffer );
    buffer->ResizeL( size );
    TPtr8 ptr = buffer->Ptr( 0 );
    aStream.ReadL( ptr, size );
    HBufC8* dataBuffer = HBufC8::NewLC( buffer->Size() );
    TPtr8 dataBufferPtr = dataBuffer->Des();
    buffer->Read( 0, dataBufferPtr, buffer->Size() );
        
    TNSmlParserGeneratorError parseError = 
                                    folderparser->ParseXml( dataBuffer );
    CleanupStack::PopAndDestroy( 2 ); // dataBuffer, buffer
    
    if ( parseError != EErrorNone )
        {
        delete agendautil;
        CleanupStack::PopAndDestroy( ); // folderparser
        FLOG(_L("CNSmlDefaultAgendaHandlerPlugin::ParseFolderDataL: end with leave"));
        User::Leave( KErrGeneral );
        return NULL;
        }
    
    // Every folder item must have NAME property. Even replace because this 
    // Plug In does not support field level update for folders.
    if ( !folderparser->iName )
        {
        delete agendautil;
        CleanupStack::PopAndDestroy( ); // folderparser
        FLOG(_L("CNSmlDefaultAgendaHandlerPlugin::ParseFolderDataL: end with leave"));
        User::Leave( KErrArgument );
        return NULL;
        }
    
    // Create Calendar info
    agendautil->iName = CnvUtfConverter::ConvertToUnicodeFromUtf8L(folderparser->iName->Des());
    CleanupStack::PopAndDestroy( ); // folderparser;    
    FLOG(_L("CNSmlDefaultAgendaHandlerPlugin::ParseFolderDataL: END"));
    return agendautil;
    }   

// -----------------------------------------------------------------------------
// CNSmlDefaultAgendaHandlerPlugin::CalendarInfoL
// Retrieve the CalendarFile Info using the aUid value
// -----------------------------------------------------------------------------
CNSmlDefaultAgendaHandlerUtil* CNSmlDefaultAgendaHandlerPlugin::CalendarInfoL( TSmlDbItemUid aUid )
    {
    FLOG(_L("CNSmlDefaultAgendaHandlerPlugin::CalendarInfoL: BEGIN"));   
        
    TBuf8<KBuffLength> keyBuff;
    CNSmlDefaultAgendaHandlerUtil* agendautil = NULL;
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
        
        TUint UidVal;
        keyBuff.Zero();
        keyBuff.AppendNum( EFolderLUID );
        TPckgC<TUint> intBuf(UidVal);
        TRAPD(err,intBuf.Set(caleninfo->PropertyValueL(keyBuff)));
                    
        if( err != KErrNone )
            {
            FLOG(_L("CNSmlDefaultAgendaHandlerPlugin::CalendarInfoL: Error while retrieving CalFile Property"));
            CleanupStack::PopAndDestroy(caleninfo); 
            CleanupStack::PopAndDestroy(vCalSubSession); 
            CleanupStack::PopAndDestroy(calfilearr); 
            CleanupStack::PopAndDestroy(vCalSession); 
            return agendautil;
            }
        
        UidVal = intBuf();
        
        if(aUid == UidVal)
            {
            FLOG(_L("CNSmlDefaultAgendaHandlerPlugin::CalendarInfoL: Found a match"));
            agendautil = CNSmlDefaultAgendaHandlerUtil::NewL();
            agendautil->iEnabled  = caleninfo->Enabled();
            agendautil->iName     = caleninfo->NameL().AllocL();
            agendautil->iFileName = caleninfo->FileNameL().AllocL();
            
            TInt err(KErrNone);
            
            // Created Time
            TTime timeval;
            keyBuff.Zero();
            keyBuff.AppendNum( ECreationTime );     
            TPckgC<TTime> pckgCreateTimeValue( timeval );
            FLOG(_L("CNSmlDefaultAgendaHandlerPlugin::CalendarInfoL: Get Creation date"));
            TRAP(err,pckgCreateTimeValue.Set(caleninfo->PropertyValueL( keyBuff )));
                        
            if( err != KErrNone )
                {
                FLOG(_L("CNSmlDefaultAgendaHandlerPlugin::CalendarInfoL: Error while retrieving CalFile CREATED Property"));
                }
            agendautil->iCreated = pckgCreateTimeValue();
            
            // Modified Time
            keyBuff.Zero();
            keyBuff.AppendNum( EModificationTime );      
            TPckgC<TTime> pckgModifyTimeValue( timeval );
            FLOG(_L("CNSmlDefaultAgendaHandlerPlugin::CalendarInfoL: Get Modified date"));
            TRAP(err,pckgModifyTimeValue.Set(caleninfo->PropertyValueL( keyBuff )));
                        
            if( err != KErrNone )
                {
                FLOG(_L("CNSmlDefaultAgendaHandlerPlugin::CalendarInfoL: Error while retrieving CalFile MODIFIED Property"));
                }
            agendautil->iModified = pckgModifyTimeValue();
            
            // Sync Status
            TBool syncstatus;
            keyBuff.Zero();
            keyBuff.AppendNum( ESyncStatus ); 
            TPckgC<TBool> pckgSyncStatusValue(syncstatus);
            FLOG(_L("CNSmlDefaultAgendaHandlerPlugin::CalendarInfoL(): Get sync status"));
            TRAP(err,pckgSyncStatusValue.Set(caleninfo->PropertyValueL( keyBuff )));
                        
            if( err != KErrNone )
                {
                FLOG(_L("CNSmlDefaultAgendaHandlerPlugin::CalendarInfoL: Error while retrieving CalFile SYNC Property"));
                }
            agendautil->iSyncStatus = pckgSyncStatusValue();
            }        
        CleanupStack::PopAndDestroy(caleninfo);
        CleanupStack::PopAndDestroy(vCalSubSession); 
        }  
    CleanupStack::PopAndDestroy(calfilearr);
    CleanupStack::PopAndDestroy(vCalSession); 
    
    FLOG(_L("CNSmlDefaultAgendaHandlerPlugin::CalendarInfoL: END"));   
    
    return agendautil;
    }

/** 
 *  Provides a key value pair table, this is used to identify
 *  the correct construction function for the requested interface.
 */
const TImplementationProxy ImplementationTable[] =
{
        IMPLEMENTATION_PROXY_ENTRY(0x20029F16, CNSmlDefaultAgendaHandlerPlugin::NewL )
};
    
/** 
 * Function used to return an instance of the proxy table.+
 */
EXPORT_C const TImplementationProxy* ImplementationGroupProxy( TInt& aTableCount )
{
        aTableCount = sizeof( ImplementationTable ) / sizeof( TImplementationProxy );
        return ImplementationTable;
}

#ifdef EKA2 
GLDEF_C TInt E32Dll() 
#else 
GLDEF_C TInt E32Dll(TDllReason /*aReason*/) 
#endif 
// DLL entry point
    {
    return(KErrNone);
    }

//End of file

