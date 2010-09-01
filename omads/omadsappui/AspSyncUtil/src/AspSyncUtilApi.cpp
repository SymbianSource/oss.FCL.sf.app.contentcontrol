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
* Description:  Contains functions that are exported from AspSyncUtil for
*                performing data synchronization.
*
*/



// INCLUDE FILES

#include "AspSyncUtilApi.h"
#include "AspSyncUtil.h"
#include "AspDebug.h"




// ============================ MEMBER FUNCTIONS ===============================



// -----------------------------------------------------------------------------
// CSyncUtilApi::NewL
//
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
EXPORT_C CSyncUtilApi* CSyncUtilApi::NewL()
    {
    FLOG( _L("CSyncUtilApi::NewL START") );
    
    CSyncUtilApi* self = new (ELeave) CSyncUtilApi();
    CleanupStack::PushL(self);
	self->ConstructL();
    CleanupStack::Pop(self);

	FLOG( _L("CSyncUtilApi::NewL END") );
	
	return self;
    }


// -----------------------------------------------------------------------------
// Destructor
//
// -----------------------------------------------------------------------------
//
CSyncUtilApi::~CSyncUtilApi()
    {
    FLOG( _L("CSyncUtilApi::~CSyncUtilApi START") );
    
	delete iSyncUtil;
	
	FLOG( _L("CSyncUtilApi::~CSyncUtilApi END") );
    }


// -----------------------------------------------------------------------------
// CSyncUtilApi::ConstructL
//
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CSyncUtilApi::ConstructL()
    {
	iSyncUtil = CAspSyncUtil::NewL();
    }


// -----------------------------------------------------------------------------
// CSyncUtilApi::CSyncUtilApi
//
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CSyncUtilApi::CSyncUtilApi()
    {
    }


// -----------------------------------------------------------------------------
// CSyncUtilApi::SynchronizeL
//
// -----------------------------------------------------------------------------
//
EXPORT_C void CSyncUtilApi::SynchronizeL(TInt aApplicationId, TInt aJobId, TInt aInfo1, TDes& aInfo2)
    {
	iSyncUtil->SynchronizeL(aApplicationId, aJobId, aInfo1, aInfo2); 
    }


// -----------------------------------------------------------------------------
// CSyncUtilApi::ShowSettingsL
//
// -----------------------------------------------------------------------------
//
EXPORT_C void CSyncUtilApi::ShowSettingsL(TInt aApplicationId, TInt aInfo1, TDes& aInfo2)
    {
    iSyncUtil->ShowSettingsL(aApplicationId, aInfo1, aInfo2);
    }


// -----------------------------------------------------------------------------
// CAspSyncUtil::SynchronizeL
//
// -----------------------------------------------------------------------------
//
EXPORT_C void CSyncUtilApi::SynchronizeL(RFile& aScheduleTaskFile)
	{
	iSyncUtil->SynchronizeL(aScheduleTaskFile);
	}



// End of file
