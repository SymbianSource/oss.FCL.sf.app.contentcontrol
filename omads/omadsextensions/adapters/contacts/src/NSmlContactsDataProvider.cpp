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
* Description:  DS contacts dataprovider
*
*/


// INCLUDE FILES
#include <utf.h>
#include <e32base.h>
#include <s32strm.h>
#include <cntdb.h>
#include <e32cmn.h>
#include <cntitem.h>
#include <sysutil.h>
#include <barsc.h>
#include <bautils.h>
#include <SmlDataProvider.h>
#include <implementationproxy.h>
#include <NSmlContactsDataStoreFormat_1_1_2.rsg>
#include <NSmlContactsDataStoreFormat_1_2.rsg>
#include <data_caging_path_literals.hrh>
#include <vtoken.h>
#include <e32property.h>
#include <DataSyncInternalPSKeys.h>
#include "NSmlContactsDataProvider.h"
#include "nsmldebug.h"
#include "nsmlconstants.h"
#include "NSmlDataModBase.h"
#include "nsmldsimpluids.h"
#include "nsmlchangefinder.h"

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CNSmlContactsDataProvider::NewL
// -----------------------------------------------------------------------------
//
CNSmlContactsDataProvider* CNSmlContactsDataProvider::NewL()
	{
	_DBG_FILE("CNSmlContactsDataProvider::NewL(): begin");
	CNSmlContactsDataProvider* self = NewLC();
	CleanupStack::Pop();
	_DBG_FILE("CNSmlContactsDataProvider::NewL(): end");
	return self;
	}

// -----------------------------------------------------------------------------
// CNSmlContactsDataProvider::NewLC
// -----------------------------------------------------------------------------
//
CNSmlContactsDataProvider* CNSmlContactsDataProvider::NewLC()
	{
	_DBG_FILE("CNSmlContactsDataProvider::NewLC(): begin");
	
	CNSmlContactsDataProvider* self = new (ELeave) CNSmlContactsDataProvider();
	CleanupStack::PushL( self );
	self->ConstructL();
	_DBG_FILE("CNSmlContactsDataProvider::NewLC(): end");
	return self;
	}
	
// -----------------------------------------------------------------------------
// CNSmlContactsDataProvider::CNSmlContactsDataProvider
// -----------------------------------------------------------------------------
//
CNSmlContactsDataProvider::CNSmlContactsDataProvider()
	{
	_DBG_FILE("CNSmlContactsDataProvider::CNSmlContactsDataProvider(): begin");
	_DBG_FILE("CNSmlContactsDataProvider::CNSmlContactsDataProvider(): end");
	}
	
// -----------------------------------------------------------------------------
// CNSmlContactsDataProvider::ConstructL
// -----------------------------------------------------------------------------
//
void CNSmlContactsDataProvider::ConstructL()
	{
	_DBG_FILE("CNSmlContactsDataProvider::ConstructL(): begin");
	
	User::LeaveIfError( iRfs.Connect() );

	iContactsDataStore = CNSmlContactsDataStore::NewL();	
	
	iStringPool.OpenL();
	
	_DBG_FILE("CNSmlContactsDataProvider::ConstructL(): end");
	}
	
// -----------------------------------------------------------------------------
// CNSmlContactsDataProvider::~CNSmlContactsDataProvider
// -----------------------------------------------------------------------------
//
CNSmlContactsDataProvider::~CNSmlContactsDataProvider()
	{
	_DBG_FILE("CNSmlContactsDataProvider::~CNSmlContactsDataProvider(): begin");

	iRfs.Close();

	delete iContactsDataStore;
	delete iStoreFormat;
	iFilterArray.ResetAndDestroy();
	iFilterArray.Close();

	iStringPool.Close();

	_DBG_FILE("CNSmlContactsDataProvider::~CNSmlContactsDataProvider(): end");
	}

// -----------------------------------------------------------------------------
// CNSmlContactsDataProvider::DoOnFrameworkEvent
// -----------------------------------------------------------------------------
//
void CNSmlContactsDataProvider::DoOnFrameworkEvent( TSmlFrameworkEvent /*aFrameworkEvent*/, TInt /*aParam1*/, TInt /*aParam2*/ )
	{
	_DBG_FILE("CNSmlContactsDataProvider::DoOnFrameworkEvent(): begin");
	_DBG_FILE("CNSmlContactsDataProvider::DoOnFrameworkEvent(): end");
	}

// -----------------------------------------------------------------------------
// CNSmlContactsDataProvider::DoSupportsOperation
// -----------------------------------------------------------------------------
//
TBool CNSmlContactsDataProvider::DoSupportsOperation( TUid aOpId ) const
	{
	_DBG_FILE("CNSmlContactsDataProvider::DoSupportsOperation(): begin");

	if ( aOpId == KUidSmlSupportBatch ||
	     aOpId == KUidSmlSupportMultipleStores )
		{
		return ETrue;
		}

	_DBG_FILE("CNSmlContactsDataProvider::DoSupportsOperation(): end");

	return EFalse; // KUidSmlSupportTransaction ,KUidSmlSupportSuspendResume, KUidSmlSupportUserSelectableMatchType
	}

// -----------------------------------------------------------------------------
// CNSmlContactsDataProvider::DoStoreFormatL
// -----------------------------------------------------------------------------
//
const CSmlDataStoreFormat& CNSmlContactsDataProvider::DoStoreFormatL()
	{
	_DBG_FILE("CNSmlContactsDataProvider::DoStoreFormatL(): begin");
	
	delete iStoreFormat;
	iStoreFormat = NULL;
	
	TFileName fileName;
	TParse parse;
	
	// Locate the resource file
	TFileName dllFileName;
	Dll::FileName( dllFileName );
    
	TFileName resourceFileName;
	resourceFileName.Copy( TParsePtrC( dllFileName ).Drive() );  
	// Check correct Data Sync protocol
    TInt value( EDataSyncNotRunning );
    TInt error = RProperty::Get( KPSUidDataSynchronizationInternalKeys,
                                 KDataSyncStatus,
                                 value );
    if ( error == KErrNone &&
         value == EDataSyncRunning )
        {
        resourceFileName.Append(KNSmlContactsStoreFormatRsc_1_1_2);
        parse.Set( resourceFileName, &KDC_RESOURCE_FILES_DIR, NULL );
        }
    else // error or protocol version 1.2 
        {
        resourceFileName.Append(KNSmlContactsStoreFormatRsc_1_2);
        parse.Set( resourceFileName, &KDC_RESOURCE_FILES_DIR, NULL );
        }		

	fileName = parse.FullName();

	RResourceFile resourceFile; 
	BaflUtils::NearestLanguageFile( iRfs, fileName );

	resourceFile.OpenL( iRfs,fileName );
	CleanupClosePushL( resourceFile );
	
	HBufC8* contactsDataFormat = resourceFile.AllocReadLC( NSML_CONTACTS_DATA_STORE_FORMAT );

	TResourceReader reader;
	reader.SetBuffer( contactsDataFormat );

	iStoreFormat = CSmlDataStoreFormat::NewLC( iStringPool, reader );
	CleanupStack::Pop(); // iStoreFormat
	
	CleanupStack::PopAndDestroy( 2 ); // contactsDataFormat,resourceFile  
	
	_DBG_FILE("CNSmlContactsDataProvider::DoStoreFormatL(): end");

	return *iStoreFormat;
	}

// -----------------------------------------------------------------------------
// CNSmlContactsDataProvider::DoListStoresLC
// -----------------------------------------------------------------------------
//
CDesCArray* CNSmlContactsDataProvider::DoListStoresLC()
	{
	_DBG_FILE("CNSmlContactsDataProvider::DoListStoresLC(): begin");
	_DBG_FILE("CNSmlContactsDataProvider::DoListStoresLC(): end");
	
	return iContactsDataStore->DoListStoresLC();
	}

// -----------------------------------------------------------------------------
// CNSmlContactsDataProvider::DoDefaultStoreL
// -----------------------------------------------------------------------------
//
const TDesC& CNSmlContactsDataProvider::DoDefaultStoreL()
	{
	_DBG_FILE("CNSmlContactsDataProvider::DoIdentifier(): begin");
	_DBG_FILE("CNSmlContactsDataProvider::DoIdentifier(): end");

	return iContactsDataStore->DefaultStoreNameL();
	}

// -----------------------------------------------------------------------------
// CNSmlContactsDataProvider::DoNewStoreInstanceLC
// -----------------------------------------------------------------------------
//
CSmlDataStore* CNSmlContactsDataProvider::DoNewStoreInstanceLC()
	{
	_DBG_FILE("CNSmlContactsDataProvider::DoNewStoreInstanceLC(): begin");

	CNSmlContactsDataStore* newDataStore = CNSmlContactsDataStore::NewL();
	CleanupStack::PushL( newDataStore );

	_DBG_FILE("CNSmlContactsDataProvider::DoNewStoreInstanceLC(): end");

	return newDataStore;
	}

// ------------------------------------------------------------------------------------------------
// CNSmlContactsDataProvider::DoSupportedServerFiltersL
// 
// ------------------------------------------------------------------------------------------------
const RPointerArray<CSyncMLFilter>& CNSmlContactsDataProvider::DoSupportedServerFiltersL()
	{
	// This method returns empty array. It means that this Data Provider does not support filtering
	_DBG_FILE("CNSmlContactsDataProvider::DoSupportedServerFiltersL(): BEGIN");
	_DBG_FILE("CNSmlContactsDataProvider::DoSupportedServerFiltersL(): END");
	return iFilterArray;
	}

// ------------------------------------------------------------------------------------------------
// CNSmlContactsDataProvider::DoCheckSupportedServerFiltersL
// 
// ------------------------------------------------------------------------------------------------
void CNSmlContactsDataProvider::DoCheckSupportedServerFiltersL( const CSmlDataStoreFormat& /*aServerDataStoreFormat*/, RPointerArray<CSyncMLFilter>& /*aFilters*/, TSyncMLFilterChangeInfo& /*aChangeInfo*/ )
	{
	_DBG_FILE("CNSmlContactsDataProvider::DoCheckSupportedServerFiltersL(): BEGIN");
	User::Leave( KErrNotSupported );
	_DBG_FILE("CNSmlContactsDataProvider::DoCheckSupportedServerFiltersL(): END");
	}

// ------------------------------------------------------------------------------------------------
// CNSmlContactsDataProvider::CheckServerFiltersL
// 
// ------------------------------------------------------------------------------------------------
void CNSmlContactsDataProvider::DoCheckServerFiltersL( RPointerArray<CSyncMLFilter>& /*aFilters*/, TSyncMLFilterChangeInfo& /*aChangeInfo*/ )
	{
	_DBG_FILE("CNSmlContactsDataProvider::DoCheckServerFiltersL(): BEGIN");
	User::Leave( KErrNotSupported );
	_DBG_FILE("CNSmlContactsDataProvider::DoCheckServerFiltersL(): END");
	}

// ------------------------------------------------------------------------------------------------
// CNSmlContactsDataProvider::DoGenerateRecordFilterQueryLC
// 
// ------------------------------------------------------------------------------------------------
HBufC* CNSmlContactsDataProvider::DoGenerateRecordFilterQueryLC( const RPointerArray<CSyncMLFilter>& /*aFilters*/, TSyncMLFilterMatchType /*aMatch*/, TDes& /*aFilterMimeType*/, TSyncMLFilterType& /*aFilterType*/, TDesC& /*aStoreName*/ )
	{
	_DBG_FILE("CNSmlContactsDataProvider::DoGenerateRecordFilterQueryLC(): BEGIN");
	User::Leave( KErrNotSupported );
	_DBG_FILE("CNSmlContactsDataProvider::DoGenerateRecordFilterQueryLC(): END");
	return NULL;
	}
	
// ------------------------------------------------------------------------------------------------
// CNSmlContactsDataProvider::DoGenerateFieldFilterQueryL
// 
// ------------------------------------------------------------------------------------------------
void CNSmlContactsDataProvider::DoGenerateFieldFilterQueryL( const RPointerArray<CSyncMLFilter>& /*aFilters*/, TDes& /*aFilterMimeType*/, RPointerArray<CSmlDataProperty>& /*aProperties*/, TDesC& /*aStoreName*/ )
	{
	_DBG_FILE("CNSmlContactsDataProvider::DoGenerateFieldFilterQueryL(): BEGIN");
	User::Leave( KErrNotSupported );
	_DBG_FILE("CNSmlContactsDataProvider::DoGenerateFieldFilterQueryL(): END");
	}

// -----------------------------------------------------------------------------
// ImplementationGroupProxy array
// -----------------------------------------------------------------------------
//
#ifndef IMPLEMENTATION_PROXY_ENTRY
#define IMPLEMENTATION_PROXY_ENTRY( aUid, aFuncPtr ) {{aUid},(aFuncPtr)}
#endif

const TImplementationProxy ImplementationTable[] = 
    {
	IMPLEMENTATION_PROXY_ENTRY( KNSmlContactsAdapterImplUid, CNSmlContactsDataProvider::NewL )
    };

// -----------------------------------------------------------------------------
// ImplementationGroupProxy
// -----------------------------------------------------------------------------
//
EXPORT_C const TImplementationProxy* ImplementationGroupProxy( TInt& aTableCount )
    {
	_DBG_FILE("ImplementationGroupProxy() for CNSmlContactsDataProvider: begin");
    aTableCount = sizeof( ImplementationTable ) / sizeof( TImplementationProxy );
	_DBG_FILE("ImplementationGroupProxy() for CNSmlContactsDataProvider: end");
    return ImplementationTable;
	}

// End of File  
