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

#ifndef C_NSMLAGENDA_ADAPTERHANDLERPLUGIN_H
#define C_NSMLAGENDA_ADAPTERHANDLERPLUGIN_H

// SYSTEM INCLUDES
#include <e32base.h>
#include <s32mem.h>
#include <ecom.h>
#include <SmlDataFormat.h>
#include <nsmlchangefinder.h>
#include <calcalendarinfo.h>
#include <e32cmn.h>

// Constants
const TUid KAgendaAdapterHandlerInterfaceUid = { 0x20029F14 };

/**
* Interface definition for ECoM plug-in adapters
*/
class CNSmlAgendaAdapterHandler: public CBase
    {
	public:
	    /**
        * Ecom interface static factory method implementation.
        * @param aImpUid Ecom's implementation uid
        * @return A pointer to the created instance of CNSmlAgendaAdapterHandler
        */
		static inline CNSmlAgendaAdapterHandler* NewL(TUid aImpUid);

		/**
		* Destructor
		*/
		virtual ~CNSmlAgendaAdapterHandler();

	public:
	    /**
        * Determines the Server supported folder properties
        * 
        * @param aServerDataStoreFormat Class structure of server device information
        *         
        */
	    virtual void CheckServerSupportForFolder( const CSmlDataStoreFormat& aServerDataStoreFormat ) = 0;
	    
	    /**
        * Retrieve the folder from the Organizer db
        * 
        * @param aUid variable specifing which Folder to be fetched
        * @param aStream on return will have the Folder details
        *         
        */
	    virtual void FetchFolderL( const TSmlDbItemUid aUid, RBufWriteStream& aStream ) = 0;
	    
	    /**
        * Determine the list of folders in the Organizer db owned by the calling application
        * 
        * @param aFolderUidArray array on return will have UID's of owned folders
        *         
        */
	    virtual void SynchronizableCalendarIdsL( CArrayFixFlat<TUint>* aFolderUidArray ) = 0;
	    
	    /**
        * Create a folder on to Organizer db
        * 
        * @param aStream containing the folder details to be created with
        * @return TCalLocalUid of the newly created folder
        *         
        */
	    virtual TCalLocalUid CreateFolderL( RBufReadStream& aStream ) = 0;
	    
	    /**
        * Replace the existing folder with updated info
        * 
        * @param aUid determines folder to act upon
        * @param aStream containing the folder information to be updated
        * @param aSyncStatus updated folders current syncstatus
        *         
        */
	    virtual void ReplaceFolderL( const TCalLocalUid& aUid, RBufReadStream& aStream, TBool& aSyncStatus ) = 0;
	    
	    /**
        * Retrieves the foldername 
        * 
        * @param aUid of the folder whose name to be determined
        * @return HBufC* name of the folder
        *         
        */
	    virtual HBufC* FolderNameL( TSmlDbItemUid aUid ) = 0;
	    
	    /**
        * Retrieve the calendar sync capability supported by the device
        * 
        * @param aStringPool 
        * @return CSmlDataStoreFormat* Class structure of supported capabilities
        *         
        */	 
	    virtual CSmlDataStoreFormat* StoreFormatL( RStringPool& aStringPool ) = 0;
	    
	    /**
        * Create a snap shot item
        * 
        * @param aUid of folder whose snapshot to be created
        * @retrun TNSmlSnapshotItem class 
        *         
        */
	    virtual TNSmlSnapshotItem CreateFolderSnapShotItemL( const TCalLocalUid& aUid ) = 0;
	    
	    /**
        * Determine the folder owner
        * 
        * @return TInt uid of the owner application
        *         
        */	    
	    virtual TInt DeviceSyncOwner() = 0;
	    
	    /**
        * Determines the sync status of the folder
        * 
        * @param aUid determine the folder whose sync status to be retrieved
        * @return TBool sync status
        *         
        */
	    virtual TBool FolderSyncStatusL( TSmlDbItemUid aUid ) = 0;
	    
	    /**
        * Determines the sync status of the folder
        * 
        * @param aFolderName determine the folder whose sync status to be retrieved
        * @return TBool sync status
        *         
        */	    
	    virtual TBool FolderSyncStatusL( HBufC* aFolderName ) = 0; 
	    
	public:
	    /**
        * Variable to hold the Plugin's Opaque Data
        */
	    HBufC8* iOpaqueData;
   
	private:    
		/**
		* Used internally to create ECOM implementation
	    */
  	   	TUid iDtor_ID_Key;  	   
    
    };

#include "nsmlagendaadapterhandler.inl"

#endif		// C_NSMLAGENDA_ADAPTERHANDLERPLUGIN_H

// End of File

