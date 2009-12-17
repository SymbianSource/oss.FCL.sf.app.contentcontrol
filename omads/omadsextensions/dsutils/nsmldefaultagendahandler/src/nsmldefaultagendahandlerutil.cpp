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
*
*/

#include "nsmldefaultagendahandlerutil.h"
#include "nsmldefaultagendahandlerdebug.h"

// -----------------------------------------------------------------------------
// CNSmlDefaultAgendaHandlerUtil::CNSmlDefaultAgendaHandlerUtil
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CNSmlDefaultAgendaHandlerUtil::CNSmlDefaultAgendaHandlerUtil()
    {
    FLOG(_L("CNSmlDefaultAgendaHandlerUtil::CNSmlDefaultAgendaHandlerUtil(): BEGIN"));
    
    iFileName = NULL;
    iName = NULL;
    iEnabled = ETrue;
    iSyncStatus = ETrue;
    
    FLOG(_L("CNSmlDefaultAgendaHandlerUtil::CNSmlDefaultAgendaHandlerUtil(): END"));
    }

// -----------------------------------------------------------------------------
// CNSmlDefaultAgendaHandlerUtil::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CNSmlDefaultAgendaHandlerUtil* CNSmlDefaultAgendaHandlerUtil::NewL()
    {
    FLOG(_L("CNSmlDefaultAgendaHandlerUtil::NewL: BEGIN"));
    
    CNSmlDefaultAgendaHandlerUtil* self = new ( ELeave ) CNSmlDefaultAgendaHandlerUtil();
    
    FLOG(_L("CNSmlDefaultAgendaHandlerUtil::NewL: END"));
    return self;
    }

// -----------------------------------------------------------------------------
// CNSmlDefaultAgendaHandlerUtil::~CNSmlDefaultAgendaHandlerUtil
// Destructor.
// -----------------------------------------------------------------------------
//
CNSmlDefaultAgendaHandlerUtil::~CNSmlDefaultAgendaHandlerUtil()
    {
    FLOG(_L("CNSmlDefaultAgendaHandlerUtil::~CNSmlDefaultAgendaHandlerUtil(): BEGIN"));
    if(iFileName)
        {
        delete iFileName;
        iFileName = NULL;
        }
    if(iName)
        {
        delete iName;
        iName = NULL;
        }
    FLOG(_L("CNSmlDefaultAgendaHandlerUtil::~CNSmlDefaultAgendaHandlerUtil(): END"));
    }

//End of file
