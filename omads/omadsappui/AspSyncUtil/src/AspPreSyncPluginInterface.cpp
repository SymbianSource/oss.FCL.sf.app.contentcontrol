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
* Description:  This implementation enumerates and returns the plugin
*
*/


// INCLUDE FILES
#include "AspPreSyncPluginInterface.h"
#include "CPreSyncPlugin.h"

_LIT(kDefault,"DEFAULT");

// ============================== MEMBER FUNCTIONS ============================

// ----------------------------------------------------------------------------
// CPreSyncPluginInterface::NewL
// Two Phase Construction
// ----------------------------------------------------------------------------
CPreSyncPluginInterface* CPreSyncPluginInterface::NewL()
{    
    CPreSyncPluginInterface* self = CPreSyncPluginInterface::NewLC();
    CleanupStack::Pop( self );
      
    return self;
}

// ----------------------------------------------------------------------------
// CPreSyncPluginInterface::NewLC
// Two Phase Construction
// ---------------------------------------------------------------------------- 
CPreSyncPluginInterface* CPreSyncPluginInterface::NewLC()
{   
    CPreSyncPluginInterface* self = new( ELeave ) CPreSyncPluginInterface();
    CleanupStack::PushL( self );
    self->ConstructL();
    
    return self;
}

// ----------------------------------------------------------------------------
// CPreSyncPluginInterface::CPreSyncPluginInterface
// Constructor
// ----------------------------------------------------------------------------    
CPreSyncPluginInterface::CPreSyncPluginInterface()
{
}

// ----------------------------------------------------------------------------
// CPreSyncPluginInterface::ConstructL
// 2nd phase constructor
// ---------------------------------------------------------------------------- 
void CPreSyncPluginInterface::ConstructL()
{
}

// ----------------------------------------------------------------------------
// CPreSyncPluginInterface::~CPreSyncPluginInterface
// Constructor
// ---------------------------------------------------------------------------- 
CPreSyncPluginInterface::~CPreSyncPluginInterface( ) 
{
    UnloadPlugIns();    
}
        
/**
* Lists all implementations which satisfy this ecom interface
*
* @param aImplInfoArray On return, contains the list of available implementations
* 
*/   
inline void CPreSyncPluginInterface::ListAllImplementationsL( RImplInfoPtrArray& aImplInfoArray )
{
    REComSession::ListImplementationsL( KPreSyncPluginInterfaceUid, aImplInfoArray );
}
        
// ----------------------------------------------------------------------------
// CPreSyncPluginInterface::InstantiateAllPlugInsL
// Instantiates all plugins
// ----------------------------------------------------------------------------     
CPreSyncPlugin* CPreSyncPluginInterface::InstantiateRoamingPluginLC( TSmlProfileId aProfileId )
{
    RImplInfoPtrArray infoArray;
    TBool bHandleSync = EFalse;
    CPreSyncPlugin* syncPlugin = NULL;
    CPreSyncPlugin* defaultSyncPlugin = NULL;

    // Get list of all implementations
    TRAPD(error, ListAllImplementationsL( infoArray ));
    
    if (error != KErrNone)
        {
        return NULL;
        }
    // Instantiate plugins for all impUIds by calling 
    // InstantiatePlugInFromImpUidLC
    for ( TInt i=0; i<infoArray.Count(); i++ )
        {
        // Get imp info
        CImplementationInformation& info( *infoArray[i] );

		TUid impUid ( info.ImplementationUid() );	        
	    

        if ( info.DisplayName().Compare(kDefault) == 0 )
            {			
	        //instantiate plugin for impUid
	        defaultSyncPlugin = InstantiatePlugInFromImpUidLC( impUid );
	        defaultSyncPlugin->SetProfile(aProfileId);
            }
		else
		    {
			syncPlugin = InstantiatePlugInFromImpUidLC( impUid );
			syncPlugin->SetProfile(aProfileId);
			if(syncPlugin->IsSupported())
			    {
				bHandleSync = ETrue;
				break;
			    }
			else
			    {
			    CleanupStack::PopAndDestroy(syncPlugin);
			    }
		    }
        }

    infoArray.ResetAndDestroy();
	if(bHandleSync)
	{		
		//delete defaultSyncPlugin;
	    if(defaultSyncPlugin != NULL)
	    {
	    CleanupStack::PopAndDestroy(defaultSyncPlugin);
	    }
		return syncPlugin;
	}
	else
	{
		return defaultSyncPlugin;
	}    
 }

// -----------------------------------------------------------------------------
// CPreSyncPluginInterface::UnloadPlugIns
// Unloads plugins
// -----------------------------------------------------------------------------
void CPreSyncPluginInterface::UnloadPlugIns()
{    
    REComSession::FinalClose();
}

// ----------------------------------------------------------------------------
// CPreSyncPluginInterface::InstantiatePlugInFromImpUidLC
// Instantiates plugin
// ---------------------------------------------------------------------------- 
CPreSyncPlugin* CPreSyncPluginInterface::InstantiatePlugInFromImpUidLC( const TUid& aImpUid )
{    
   // REComSession
    CPreSyncPlugin *preSyncPlugin = CPreSyncPlugin::NewL(aImpUid);    
    CleanupStack::PushL(preSyncPlugin);
    return preSyncPlugin;
}
    
    
