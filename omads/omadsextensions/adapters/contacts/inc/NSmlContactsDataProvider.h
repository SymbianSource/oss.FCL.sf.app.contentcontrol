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
* Description:  DS contacts dataprovider.
*
*/


#ifndef __NSMLCONTACTSDATAPROVIDER_H__
#define __NSMLCONTACTSDATAPROVIDER_H__

// INCLUDES
#include <f32file.h>
#include <SmlDataProvider.h>
#include "NSmlContactsDataStore.h"

// CLASS DECLARATION

// ------------------------------------------------------------------------------------------------
//  CNSmlContactsDataProvider
//
//  @lib nsmlcontactsdataprovider.lib
// ------------------------------------------------------------------------------------------------
class CNSmlContactsDataProvider : public CSmlDataProvider 
	{
	public:
		/**
		* Two-phased constructor.
		*/
		static CNSmlContactsDataProvider* NewL();

		/**
		* Destructor.
		*/
		virtual ~CNSmlContactsDataProvider();

	private:
		/**
		* Two-phased constructor.
		*/
		static CNSmlContactsDataProvider* NewLC();

		/**
		* This method handles framework events.
		* @param					Frameworks event.
		* @param	aParam1			First parameter.
		* @param	aParam2			Second parameter.
		*/
		void DoOnFrameworkEvent(TSmlFrameworkEvent, TInt aParam1, TInt aParam2);

		/**
		* Check if operation is supported by dataprovider.
		* @param	aOpId			Operations type.
		* @return	ETrue if the operation is supported.
		*/
		TBool DoSupportsOperation(TUid aOpId) const;

		/**
		* Gets dataproviders own StoreFormat.
		*/
		const CSmlDataStoreFormat& DoStoreFormatL();

		/**
		* Lists all contacts databases on client.
		* @return List of contact databases.
		*/
		CDesCArray* DoListStoresLC();

		/**
		* Gets default storename on client.
		* @return Default storename.
		*/
		const TDesC& DoDefaultStoreL();

		/**
		* Constructs new instance of datastore.
		* @return Newly created instance.
		*/
		CSmlDataStore* DoNewStoreInstanceLC();

		/**
		This method returns the set of filters that can be used to send to the SyncML server.
		* @return New filters.
		*/
		const RPointerArray<CSyncMLFilter>& DoSupportedServerFiltersL();
		
		/**
		This method checks what filters are supported by server.
		@param	aServerDataStoreFormat	The store format of server
		@param	aFilters				The array that includes filters
		@param	aChangeInfo				The change information about changes that data provider did
		*/
		void DoCheckSupportedServerFiltersL(const CSmlDataStoreFormat& aServerDataStoreFormat, RPointerArray<CSyncMLFilter>& aFilters, TSyncMLFilterChangeInfo& aChangeInfo);
		
		/**
		This method updates dynamic filters up-to-date.
		@param	aFilters				The array that includes filters
		@param	aChangeInfo				The change information about changes that data provider did
		*/
		void DoCheckServerFiltersL(RPointerArray<CSyncMLFilter>& aFilters, TSyncMLFilterChangeInfo& aChangeInfo);
		
		/**
		This method generates a record filter query to be sent to the SyncML server for the provided filters.
		@param	aFilters			The filters to be used for the query generation
		@param	aMatch				The filter match type to be used
		@param	aFilterMimeType		The mime type of the returned filter query
		@param	TSyncMLFilterType 	The filter type of the returned filter query
		@param	aStoreName			The name of used store
		@return						The record filter query to be sent to the SyncML server - empty if no record filter involved
									for this specific filter
		*/
		HBufC* DoGenerateRecordFilterQueryLC(const RPointerArray<CSyncMLFilter>& aFilters, TSyncMLFilterMatchType aMatch, TDes& aFilterMimeType, TSyncMLFilterType& aFilterType, TDesC& aStoreName);
		
		/**
		This method generates a field filter query to be sent to the SyncML server for the provided filters.
		@param	aFilters		The filters to be used for the query generation
		@param	aFilterMimeType		The mime type of the returned filter query
		@param	aProperties		The field filter query to be sent to the SyncML server - empty if no field filter involved
								for this specific filter
		@param  aStoreName		The name of used store
		*/
		void DoGenerateFieldFilterQueryL(const RPointerArray<CSyncMLFilter>& aFilters, TDes& aFilterMimeType, RPointerArray<CSmlDataProperty>& aProperties, TDesC& aStoreName);

		/**
		* C++ constructor.
		*/
		CNSmlContactsDataProvider();

		/**
		* 2nd phase constructor.
		*/
		void ConstructL();

	private: // data
		CNSmlContactsDataStore* iContactsDataStore; 
		RPointerArray<CSyncMLFilter> iFilterArray;
		RFs iRfs;
		RStringPool iStringPool;
		CSmlDataStoreFormat* iStoreFormat;
	};
	
#endif // __NSMLCONTACTSDATAPROVIDER_H__

// End of File  
