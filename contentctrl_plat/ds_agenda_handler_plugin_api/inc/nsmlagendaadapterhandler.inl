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

// INCLUDE FILES
#include <eCom.h>
#include "nsmlagendaadapterhandler.h"

// ============================ MEMBER FUNCTIONS ===============================

// ----------------------------------------------------------------------------
// CNSmlAgendaAdapterHandler::NewL
// Two phase construction
// ----------------------------------------------------------------------------
inline CNSmlAgendaAdapterHandler* CNSmlAgendaAdapterHandler::NewL( TUid aImpUid)
    {
    TAny* implementation = REComSession::CreateImplementationL ( aImpUid,
    _FOFF ( CNSmlAgendaAdapterHandler, iDtor_ID_Key ));
    CNSmlAgendaAdapterHandler* self = REINTERPRET_CAST( CNSmlAgendaAdapterHandler*, implementation );
    return self;
    }

// ----------------------------------------------------------------------------
// CNSmlAgendaAdapterHandler::~CNSmlAgendaAdapterHandler
// Destructor
// ----------------------------------------------------------------------------
inline CNSmlAgendaAdapterHandler::~CNSmlAgendaAdapterHandler()
    {
    REComSession::DestroyedImplementation( iDtor_ID_Key );
    delete iOpaqueData;
    }
