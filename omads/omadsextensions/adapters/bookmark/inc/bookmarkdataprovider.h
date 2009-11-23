/*
* Copyright (c) 2005-2007 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:  Part of SyncML Data Synchronization Plug In Adapter
*
*/


#ifndef __BOOKMARKDATAPROVIDER_H__
#define __BOOKMARKDATAPROVIDER_H__

#include <SmlDataProvider.h>
#include <f32file.h>

class CBookmarkDataProvider : public CSmlDataProvider
	{
private:
	// Default C++ constructor
	CBookmarkDataProvider();

	// 2nd phase of constructor
	void ConstructL();

public:	
	
	// Two-phased constructor
	static CBookmarkDataProvider* NewL();
	
	// Destructor
	virtual ~CBookmarkDataProvider();

protected:

	virtual void DoOnFrameworkEvent( TSmlFrameworkEvent, TInt aParam1, TInt aParam2 );
	
	virtual TBool DoSupportsOperation( TUid aOpId ) const;
	
	virtual const CSmlDataStoreFormat& DoStoreFormatL();
	
	virtual CDesCArray* DoListStoresLC();
	
	virtual const TDesC& DoDefaultStoreL();
	
	virtual CSmlDataStore* DoNewStoreInstanceLC();

	virtual const RPointerArray<CSyncMLFilter>& DoSupportedServerFiltersL();
	
	virtual void DoCheckSupportedServerFiltersL( const CSmlDataStoreFormat& aServerDataStoreFormat,
	    RPointerArray<CSyncMLFilter>& aFilters, TSyncMLFilterChangeInfo& aChangeInfo );
	
	virtual void DoCheckServerFiltersL( RPointerArray<CSyncMLFilter>& aFilters,
	    TSyncMLFilterChangeInfo& aChangeInfo );
	
	virtual HBufC* DoGenerateRecordFilterQueryLC( const RPointerArray<CSyncMLFilter>& aFilters,
	    TSyncMLFilterMatchType aMatch, TDes& aFilterMimeType,
	    TSyncMLFilterType& aFilterType, TDesC& aStoreName );
	
	virtual void DoGenerateFieldFilterQueryL( const RPointerArray<CSyncMLFilter>& aFilters,
	    TDes& aFilterMimeType, RPointerArray<CSmlDataProperty>& aProperties, TDesC& aStoreName );

private:

	CSmlDataStoreFormat* DoOwnStoreFormatL();

private:
	
	CSmlDataStoreFormat*            iOwnStoreFormat;
	RStringPool                     iStringPool;
	RPointerArray<CSyncMLFilter>    iFilters;
	RFs	                            iRFs;
	
	};

#endif // __BOOKMARKDATAPROVIDER_H__
