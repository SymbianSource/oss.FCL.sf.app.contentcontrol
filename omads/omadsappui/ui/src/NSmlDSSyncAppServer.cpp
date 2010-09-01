/*
* Copyright (c) 2002-2005 Nokia Corporation and/or its subsidiary(-ies). 
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



// INCLUDE FILES
#include    "NSmlDSSyncAppServer.h"
#include    "aspdebug.h"

#include    <SyncServiceSession.h>
#include    <syncserviceconst.h>



// ============================ MEMBER FUNCTIONS ===============================



// ------------------------------------------------------------------------------
// CNSmlDMSyncAppServer::CNSmlDMSyncAppServer
//
// ------------------------------------------------------------------------------
//
CNSmlDSSyncAppServer::CNSmlDSSyncAppServer()   
	{
	FLOG( _L("CNSmlDSSyncAppServer::CNSmlDSSyncAppServer") );	
	}


// ------------------------------------------------------------------------------
// CNSmlDMSyncAppServer::~CNSmlDMSyncAppServer
//
// ------------------------------------------------------------------------------
//        
CNSmlDSSyncAppServer::~CNSmlDSSyncAppServer()
	{
    FLOG( _L("CNSmlDSSyncAppServer::~CNSmlDSSyncAppServer") );
	}


// ------------------------------------------------------------------------------
// CNSmlDMSyncAppServer::CreateServiceL
//
// ------------------------------------------------------------------------------
//
CApaAppServiceBase* CNSmlDSSyncAppServer::CreateServiceL( TUid aServiceType ) const
	{	
	FLOG( _L("CNSmlDSSyncAppServer::CreateServiceL") );
	
	if (aServiceType == TUid::Uid(KSyncServiceUid))   
		{
		return new (ELeave) CSyncServiceSession;
		}
	else
		{
		return CAknAppServer::CreateServiceL(aServiceType);
		}
	}


// ------------------------------------------------------------------------------
// CNSmlDMSyncAppServer::CreateServiceL
//
// ------------------------------------------------------------------------------
//
void CNSmlDSSyncAppServer::HandleAllClientsClosed()
    {
   	FLOG( _L("CNSmlDMSyncAppServer::HandleAllClientsClosed") );
        
    // NOTE: HandleAllClientsClosed needs overriding to avoid RunAppShutter
    }




// End of File
