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
 
#ifndef C_NSML_DEFAULTAGENDAHANDLER_PLUGIN_H
#define C_NSML_DEFAULTAGENDAHANDLER_PLUGIN_H

#include <s32mem.h>
#include <e32def.h>
#include <nsmlagendaadapterhandler.h>
#include <data_caging_path_literals.hrh>

// CONSTANTS
_LIT( KNSmlDefaultAgendaHandlerRsc_1_2,"z:NSMLDEFAULTAGENDAHANDLER_1_2.RSC" );
const TUid KCRUidNSmlDSApp       = { 0x101F9A1D };
const TInt KArrayGranularity    = 30;

_LIT8( KNSmlDataObjectCREATED,          "CREATED" );
_LIT8( KNSmlDataObjectNAME,             "NAME" );
_LIT8( KNSmlDataObjectMODIFIED,         "MODIFIED" );

struct TNSmlServerSupportsForFolder
    {
    TBool iSSName;
    TBool iSSCreated;
    TBool iSSModified;
    };  

// FORWARD DECLARATIONS
class CNSmlDefaultAgendaHandlerUtil;
class CCalSession;

/**
 *  This class represents the Adapter Object plugin
 *  It is used by ECoM client to initialize calendar notifier
 *
 *  @lib nsmloviagendaadapter.lib
 */
class CNSmlDefaultAgendaHandlerPlugin: public CNSmlAgendaAdapterHandler
  	{
public:
    /**
	 * Two phase construction
	 */
	IMPORT_C static CNSmlDefaultAgendaHandlerPlugin* NewL();
	
    /**
	 * Destructor
	 */
	~CNSmlDefaultAgendaHandlerPlugin();

private:    
    /**
	* C++ constructor - not exported;
	* implicitly called from NewLC()	
	*/
	CNSmlDefaultAgendaHandlerPlugin::CNSmlDefaultAgendaHandlerPlugin();	
	
	void ConstructL(); 
	
	/**
     * Inherited from CAgendaAdapterHandler
     *
     * @param 
    */
	void CheckServerSupportForFolder( const CSmlDataStoreFormat& aServerDataStoreFormat );
	     
    void FetchFolderL( const TSmlDbItemUid aUid, RBufWriteStream& aStream );
 
    void SynchronizableCalendarIdsL( CArrayFixFlat<TUint>* aFolderUidArray );
 
    TCalLocalUid CreateFolderL( RBufReadStream& aStream );
 
    void ReplaceFolderL( const TCalLocalUid& aUid, RBufReadStream& aStream, TBool& aSyncStatus );   
 
    HBufC* FolderNameL( TSmlDbItemUid aUid );
 
    CSmlDataStoreFormat* StoreFormatL( RStringPool& aStringPool );
    
    TInt DeviceSyncOwner();
    
    TBool FolderSyncStatusL( TSmlDbItemUid aUid );
    
    TBool FolderSyncStatusL( HBufC* aFolderName );
 
    TNSmlSnapshotItem CreateFolderSnapShotItemL( const TCalLocalUid& aUid );      
    
public:
    
    void SynchronizableCalEntryIdsL( CCalSession* aCalSession, TCalLocalUid aUId, RArray<TCalLocalUid>& aUidArray );
    
     
    CNSmlDefaultAgendaHandlerUtil* CalendarInfoL( TSmlDbItemUid aUid );
    
    CNSmlDefaultAgendaHandlerUtil* ParseFolderDataL( RBufReadStream& aStream );
    
public:
                  
private: 
    
    TNSmlServerSupportsForFolder    iServerSupportsForFolder;
     
	};

# endif // C_NSML_DEFAULTAGENDAHANDLER_PLUGIN_H
