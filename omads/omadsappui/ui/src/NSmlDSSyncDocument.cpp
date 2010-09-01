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
#include "NSmlDSSyncDocument.h"
#include "NSmlDSSyncAppUi.h"
#include "AspDebug.h"


//const TUid KUidAppSync       = { 0x101F6DE4 };


// ============================ MEMBER FUNCTIONS ===============================


// -----------------------------------------------------------------------------
// CNSmlDSSyncDocument::Constructor
//
// -----------------------------------------------------------------------------
//
CNSmlDSSyncDocument::CNSmlDSSyncDocument(CEikApplication& aApp)
: CAknDocument(aApp)    
    {
    }


// -----------------------------------------------------------------------------
// CNSmlDSSyncDocument::~CNSmlDSSyncDocument
//
// -----------------------------------------------------------------------------
//
CNSmlDSSyncDocument::~CNSmlDSSyncDocument()
    {
    FLOG( _L("CNSmlDSSyncDocument::~CNSmlDSSyncDocument START") );
    
    delete iSyncUtilApi;
    
    FLOG( _L("CNSmlDSSyncDocument::~CNSmlDSSyncDocument END") );
    }


// -----------------------------------------------------------------------------
// CNSmlDSSyncDocument::ConstructL
//
// -----------------------------------------------------------------------------
//
void CNSmlDSSyncDocument::ConstructL()
    {
    FLOG( _L("CNSmlDSSyncDocument::ConstructL START") );
    
    iSyncUtilApi = CSyncUtilApi::NewL();
    
    iEikEnv = CEikonEnv::Static();
    
    FLOG( _L("CNSmlDSSyncDocument::ConstructL END") );
    }


// -----------------------------------------------------------------------------
// CNSmlDSSyncDocument::NewL
//
// -----------------------------------------------------------------------------
//
CNSmlDSSyncDocument* CNSmlDSSyncDocument::NewL(CEikApplication& aApp)
    {
    FLOG( _L("CNSmlDSSyncDocument::NewL START") );
    
    CNSmlDSSyncDocument* self = new (ELeave) CNSmlDSSyncDocument( aApp );
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);

    FLOG( _L("CNSmlDSSyncDocument::NewL END") );
    
    return self;
    }

    
// -----------------------------------------------------------------------------
// CNSmlDSSyncDocument::CreateAppUiL
//
// -----------------------------------------------------------------------------
//
CEikAppUi* CNSmlDSSyncDocument::CreateAppUiL()
    {
    return new (ELeave) CNSmlDSSyncAppUi;
    }


// -----------------------------------------------------------------------------
// CNSmlDSSyncDocument::Model
//
// -----------------------------------------------------------------------------
//
CSyncUtilApi* CNSmlDSSyncDocument::Model()
    {
    __ASSERT_ALWAYS(iSyncUtilApi, Panic(KErrGeneral));
    
    return iSyncUtilApi;
    }


// -------------------------------------------------------------------------------
// CNSmlDSSyncDocument::Panic
//
// -------------------------------------------------------------------------------
//
void CNSmlDSSyncDocument::Panic(TInt aReason)
    {
	_LIT(KPanicCategory,"CNSmlDSSync");

	User::Panic(KPanicCategory, aReason); 
    }




// End of File  
