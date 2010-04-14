/*
* Copyright (c) 2007 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description: Roaming warning plugin interface implementation 
*
*/

#ifndef ASPPRESYNCPLUGININTERFACE_H
#define ASPPRESYNCPLUGININTERFACE_H

// INCLUDE FILES
#include <s32mem.h>
#include <coemain.h>
#include <ecom/ecom.h>
#include <ecom/implementationinformation.h>
#include <SyncMLDef.h>

class CPreSyncPlugin;

/**
* Class is used to instantiate the required plugins when the search has been
* asked. Acts as interface to all Plugins.
*/

class CPreSyncPluginInterface: public CBase 
{
	public:

		/** 
		* Symbian OS 2 phased constructor.
		* @return	A pointer to the created instance of CPreSyncPluginInterface.
		*/
		static CPreSyncPluginInterface* NewL();

		/** 
		* Symbian OS 2 phased constructor.
		* @return	A pointer to the created instance of CPreSyncPluginInterface.
		*/
		static CPreSyncPluginInterface* NewLC();

		/**
		* Destructor.
		*/      
		virtual ~CPreSyncPluginInterface();

		/**
		* Instantiates the required search plug-ins known by the ecom framework.
		* basically this function first calls ListAllImplementationsL()
		* and then checks them one by one to match the algorithm name
		*/
		void InstantiatePluginL();
		
		CPreSyncPlugin* InstantiateRoamingPluginLC( TSmlProfileId aProfileId );

		/**
		* Unloads all plugins
		* This should internally call  REComSession::DestroyedImplementation( iDtor_ID_Key )
		* and then REComSession::FinalClose()
		*/
		void UnloadPlugIns(); 

		/**
		* Lists all implementations which satisfy this ecom interface
		*
		* @param aImplInfoArray On return, contains the list of available implementations
		* 
		*/   
		void ListAllImplementationsL( RImplInfoPtrArray& aImplInfoArray );
		

		/**
		* Forwards the request to check if language is supported
		* to the algorithm instance
		*/
		TBool IsDefault();   
		              
		/**
		* Forwards the request to recover data fields order for a URI
		* to the algorithm instance
		*/
		TBool IsProfileYours(TSmlProfileId aProfileId);

		/**
		* Forwards the request to recover sort order for a URI
		* to the algorithm instance
		*/
		void Handle(TSmlProfileId aProfileId); 

	private: 

		/** 
		* Performs the first phase of two phase construction.
		*/
		CPreSyncPluginInterface();  

		/** 
		* Symbian OS 2 phased constructor.
		*/
		void ConstructL();

		/**
		* Instantiates a PCS plug-in, knowing the implementation uid.
		* @param aImpUid imp uID
		*/
		CPreSyncPlugin* InstantiatePlugInFromImpUidL( const TUid& aImpUid );


	private: 

	    /** iDtor_ID_Key Instance identifier key. When instance of an
	         *               implementation is created by ECOM framework, the
	         *               framework will assign UID for it. The UID is used in
	         *               destructor to notify framework that this instance is
	         *               being destroyed and resources can be released.
	         */
	    TUid iDtor_ID_Key;

		// List of plugins that this interface will interact with
		RPointerArray<CPreSyncPlugin>  iPluginInstances; 

		// Uid of the algorithm in use
		TUid iPluginInUse;
};

#endif // ASPPRESYNCPLUGININTERFACE_H

//End of File
