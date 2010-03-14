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
* Description:  DS agenda data provider
*
*/


// INCLUDE FILES
#include <SmlDataProvider.h>
#include <ecom.h>
#include <barsc.h>
#include <bautils.h>
#include <NSmlAgendaDataStore_1_1_2.rsg>
#include <SmlDataFormat.h>
#include <implementationproxy.h>
#include <data_caging_path_literals.hrh>
#include <e32property.h>
#include <DataSyncInternalPSKeys.h>
#include <nsmldebug.h>
#include "nsmlagendadataprovider.h"
#include "nsmlagendadatastore.h"
#include "nsmlagendadebug.h"
#include "nsmldsimpluids.h"

// ====================================== MEMBER FUNCTIONS ========================================

// ------------------------------------------------------------------------------------------------
// CNSmlAgendaDataProvider::CNSmlAgendaDataProvider
// C++ default constructor can NOT contain any code, that
// might leave.
// ------------------------------------------------------------------------------------------------
//
CNSmlAgendaDataProvider::CNSmlAgendaDataProvider()
	{
	FLOG(_L("CNSmlAgendaDataProvider::CNSmlAgendaDataProvider(): BEGIN"));
	FLOG(_L("CNSmlAgendaDataProvider::CNSmlAgendaDataProvider(): END"));
	}

// ------------------------------------------------------------------------------------------------
// CNSmlAgendaDataProvider::ConstructL
// Symbian 2nd phase constructor can leave.
// ------------------------------------------------------------------------------------------------
//
void CNSmlAgendaDataProvider::ConstructL( )
	{
	FLOG(_L("CNSmlAgendaDataProvider::NewL: BEGIN"));
	iAgnDataStore = CNSmlAgendaDataStore::NewL();
	FLOG(_L("CNSmlAgendaDataProvider::NewL: END"));
	}

// ------------------------------------------------------------------------------------------------
// CNSmlAgendaDataProvider::NewL
// Two-phased constructor.
// ------------------------------------------------------------------------------------------------
//
CNSmlAgendaDataProvider* CNSmlAgendaDataProvider::NewL()
	{
	FLOG(_L("CNSmlAgendaDataProvider::NewL: BEGIN"));
	CNSmlAgendaDataProvider* self = new ( ELeave ) CNSmlAgendaDataProvider();
	CleanupStack::PushL( self );
	self->ConstructL();
	CleanupStack::Pop( self );
	FLOG(_L("CNSmlAgendaDataProvider::NewL: END"));
	return self;
	}

// ------------------------------------------------------------------------------------------------
// CNSmlAgendaDataProvider::~CNSmlAgendaDataProvider
// Destructor.
// ------------------------------------------------------------------------------------------------
//
CNSmlAgendaDataProvider::~CNSmlAgendaDataProvider()
	{
	FLOG(_L("CNSmlAgendaDataProvider::~CNSmlAgendaDataProvider(): BEGIN"));
	delete iDSFormat;
	delete iAgnDataStore;
	iFilterArray.ResetAndDestroy();
	iFilterArray.Close();
	FLOG(_L("CNSmlAgendaDataProvider::~CNSmlAgendaDataProvider(): END"));
	}

// ------------------------------------------------------------------------------------------------
// CNSmlAgendaDataProvider::DoOnFrameworkEvent
// Not supported.
// ------------------------------------------------------------------------------------------------
//
void CNSmlAgendaDataProvider::DoOnFrameworkEvent( TSmlFrameworkEvent /*aEvent*/, TInt /*aParam1*/, TInt /*aParam2*/ )
    {
	FLOG(_L("CNSmlAgendaDataProvider::DoOnFrameworkEvent: BEGIN"));
	FLOG(_L("CNSmlAgendaDataProvider::DoOnFrameworkEvent: END"));
    }

// ------------------------------------------------------------------------------------------------
// CNSmlAgendaDataProvider::DoSupportsOperation
// Return supported operation uid.
// ------------------------------------------------------------------------------------------------
//
TBool CNSmlAgendaDataProvider::DoSupportsOperation( TUid aOpId ) const
    {
	FLOG(_L("CNSmlAgendaDataProvider::DoSupportsOperation: BEGIN"));
	if ( aOpId == KUidSmlSupportMultipleStores )
		{
		FLOG(_L("CNSmlAgendaDataProvider::DoSupportsOperation: END"));
		return ETrue;
		}
	FLOG(_L("CNSmlAgendaDataProvider::DoSupportsOperation: END"));
	return EFalse;
    }

// ------------------------------------------------------------------------------------------------
// CNSmlAgendaDataProvider::DoStoreFormatL
// Return the current data store format.
// ------------------------------------------------------------------------------------------------
//
const CSmlDataStoreFormat& CNSmlAgendaDataProvider::DoStoreFormatL()
    {
	FLOG(_L("CNSmlAgendaDataProvider::DoStoreFormatL(): BEGIN"));
	
	if ( iDSFormat )
		{
       	delete iDSFormat;
       	iDSFormat = NULL;
		}
	// RD_MULTICAL
	iDSFormat = iAgnDataStore->StoreFormatL();
	// RD_MULTICAL
	return *iDSFormat;
    }

// ------------------------------------------------------------------------------------------------
// CNSmlAgendaDataProvider::DoListStoresLC
// Return the list of datastore names.
// ------------------------------------------------------------------------------------------------
//
CDesCArray* CNSmlAgendaDataProvider::DoListStoresLC()
    {
	FLOG(_L("CNSmlAgendaDataProvider::DoListStoresLC(): BEGIN"));
	FLOG(_L("CNSmlAgendaDataProvider::DoListStoresLC(): END"));
	return iAgnDataStore->DoListAgendaFilesLC();
    }

// ------------------------------------------------------------------------------------------------
// CNSmlAgendaDataProvider::DoDefaultStoreL
// Return the name of default store name.
// ------------------------------------------------------------------------------------------------
//
const TDesC& CNSmlAgendaDataProvider::DoDefaultStoreL()
    {
	FLOG(_L("CNSmlAgendaDataProvider::DoDefaultStoreL(): BEGIN"));
	FLOG(_L("CNSmlAgendaDataProvider::DoDefaultStoreL(): END"));
	return iAgnDataStore->DoGetDefaultFileNameL();
    }

// ------------------------------------------------------------------------------------------------
// CNSmlAgendaDataProvider::DoNewStoreInstanceLC
// Return instance of data store
// ------------------------------------------------------------------------------------------------
//
CSmlDataStore* CNSmlAgendaDataProvider::DoNewStoreInstanceLC()
    {
	FLOG(_L("CNSmlAgendaDataProvider::DoNewStoreInstanceLC(): BEGIN"));
	CNSmlAgendaDataStore* newStore = CNSmlAgendaDataStore::NewL();
	CleanupStack::PushL( newStore );
	FLOG(_L("CNSmlAgendaDataProvider::DoNewStoreInstanceLC(): END"));
	return newStore;
    } 

// ------------------------------------------------------------------------------------------------
// CNSmlAgendaDataProvider::DoSupportedServerFiltersL
// 
// ------------------------------------------------------------------------------------------------
const RPointerArray<CSyncMLFilter>& CNSmlAgendaDataProvider::DoSupportedServerFiltersL()
	{
	// This method returns empty array. It means that this Data Provider does not support filtering
	FLOG(_L("CNSmlAgendaDataProvider::DoSupportedServerFiltersL(): BEGIN"));
	FLOG(_L("CNSmlAgendaDataProvider::DoSupportedServerFiltersL(): END"));
	return iFilterArray;
	}

// ------------------------------------------------------------------------------------------------
// CNSmlAgendaDataProvider::DoCheckSupportedServerFiltersL
// 
// ------------------------------------------------------------------------------------------------
void CNSmlAgendaDataProvider::DoCheckSupportedServerFiltersL( const CSmlDataStoreFormat& /*aServerDataStoreFormat*/, RPointerArray<CSyncMLFilter>& /*aFilters*/, TSyncMLFilterChangeInfo& /*aChangeInfo*/ )
	{
	FLOG(_L("CNSmlAgendaDataProvider::DoCheckSupportedServerFiltersL(): BEGIN"));
	FLOG(_L("CNSmlAgendaDataProvider::DoCheckSupportedServerFiltersL(): END"));
	User::Leave( KErrNotSupported );
	}

// ------------------------------------------------------------------------------------------------
// CNSmlAgendaDataProvider::CheckServerFiltersL
// 
// ------------------------------------------------------------------------------------------------
void CNSmlAgendaDataProvider::DoCheckServerFiltersL( RPointerArray<CSyncMLFilter>& /*aFilters*/, TSyncMLFilterChangeInfo& /*aChangeInfo*/ )
	{
	FLOG(_L("CNSmlAgendaDataProvider::DoCheckServerFiltersL(): BEGIN"));
	FLOG(_L("CNSmlAgendaDataProvider::DoCheckServerFiltersL(): END"));
	User::Leave( KErrNotSupported );
	}

// ------------------------------------------------------------------------------------------------
// CNSmlAgendaDataProvider::DoGenerateRecordFilterQueryLC
// 
// ------------------------------------------------------------------------------------------------
HBufC* CNSmlAgendaDataProvider::DoGenerateRecordFilterQueryLC( const RPointerArray<CSyncMLFilter>& /*aFilters*/, TSyncMLFilterMatchType /*aMatch*/, TDes& /*aFilterMimeType*/, TSyncMLFilterType& /*aFilterType*/, TDesC& /*aStoreName*/ )
	{
	FLOG(_L("CNSmlAgendaDataProvider::DoGenerateRecordFilterQueryLC(): BEGIN"));
	FLOG(_L("CNSmlAgendaDataProvider::DoGenerateRecordFilterQueryLC(): END"));
	User::Leave( KErrNotSupported );
	return NULL;
	}
	
// ------------------------------------------------------------------------------------------------
// CNSmlAgendaDataProvider::DoGenerateFieldFilterQueryL
// 
// ------------------------------------------------------------------------------------------------
void CNSmlAgendaDataProvider::DoGenerateFieldFilterQueryL( const RPointerArray<CSyncMLFilter>& /*aFilters*/, TDes& /*aFilterMimeType*/, RPointerArray<CSmlDataProperty>& /*aProperties*/, TDesC& /*aStoreName*/ )
	{
	FLOG(_L("CNSmlAgendaDataProvider::DoGenerateFieldFilterQueryL(): BEGIN"));
	FLOG(_L("CNSmlAgendaDataProvider::DoGenerateFieldFilterQueryL(): END"));
	User::Leave( KErrNotSupported );
	}

// =================================== OTHER EXPORTED FUNCTIONS ===================================
#ifndef IMPLEMENTATION_PROXY_ENTRY
#define IMPLEMENTATION_PROXY_ENTRY( aUid, aFuncPtr ) { { aUid }, ( aFuncPtr ) }
#endif

const TImplementationProxy ImplementationTable[] = 
    {
	IMPLEMENTATION_PROXY_ENTRY( KNSmlAgendaAdapterImplUid, CNSmlAgendaDataProvider::NewL )
    };

// ------------------------------------------------------------------------------------------------
// ImplementationGroupProxy
// Framework needs this.
// ------------------------------------------------------------------------------------------------
//
EXPORT_C const TImplementationProxy* ImplementationGroupProxy( TInt& aTableCount )
    {
	FLOG(_L("ImplementationGroupProxy() for CNSmlAgendaDataProvider: begin"));
    aTableCount = sizeof( ImplementationTable ) / sizeof( TImplementationProxy );
	FLOG(_L("ImplementationGroupProxy() for CNSmlAgendaDataProvider: end"));
    return ImplementationTable;
	}


//  End of File  
