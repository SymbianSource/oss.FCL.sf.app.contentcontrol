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
#include    "NSmlDSSyncApp.h"
#include    "NSmlDSSyncDocument.h"
#include    "AspDebug.h"


// ============================ MEMBER FUNCTIONS ===============================



// -----------------------------------------------------------------------------
// CNSmlDSSyncApp::AppDllUid
//
// Returns application UID.
// -----------------------------------------------------------------------------
//
TUid CNSmlDSSyncApp::AppDllUid() const
    {
    FLOG( _L("CNSmlDSSyncApp::AppDllUid") );
    
    return KUidSmlSyncApp;
    }

   
// -----------------------------------------------------------------------------
// CNSmlDSSyncApp::CreateDocumentL
//
// Creates CAIWTestAppDocument object.
// -----------------------------------------------------------------------------
//
CApaDocument* CNSmlDSSyncApp::CreateDocumentL()
    {
    FLOG( _L("CNSmlDSSyncApp::CreateDocumentL") );
    
    return CNSmlDSSyncDocument::NewL( *this );
    }




// ======================== OTHER EXPORTED FUNCTIONS ===========================



#include <eikstart.h>

LOCAL_C CApaApplication* NewApplication()
    {
    return new CNSmlDSSyncApp;
    }

GLDEF_C TInt E32Main()
    {
    return EikStart::RunApplication(NewApplication);
    }




// End of File  

