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


#include <schinfo.h>
#include <schtask.h>
#include <s32file.h>
#include <e32math.h>
#include <e32cons.h>

#include "AspSyncUtilApi.h"
#include "AspDebug.h"



// -----------------------------------------------------------------------------
// SynchronizeL
//
// -----------------------------------------------------------------------------
//
LOCAL_D void SynchronizeL(RFile& aTaskFile)
	{
	CSyncUtilApi* syncUtilApi = CSyncUtilApi::NewL();
	CleanupStack::PushL(syncUtilApi);
	
	syncUtilApi->SynchronizeL(aTaskFile);
	
	CleanupStack::PopAndDestroy(syncUtilApi);
	}


// -----------------------------------------------------------------------------
// ExecuteL
//
// -----------------------------------------------------------------------------
//
LOCAL_D TInt ExecuteL()
	{
	TInt err = KErrNoMemory;
		
	RFile file;
		
	// Adopt the task file from the Task Scheduler
	err = file.AdoptFromCreator(TScheduledTaskFile::FsHandleIndex(),
									TScheduledTaskFile::FileHandleIndex());
	User::LeaveIfError(err);
	
	TRAP(err, SynchronizeL(file));
	
	file.Close();
	
	User::LeaveIfError(err);		
	return err;
	}


// -----------------------------------------------------------------------------
// Execute
//
// -----------------------------------------------------------------------------
//
LOCAL_D TInt Execute()
	{
	TInt err = KErrNoMemory;
	
	// construct and install active scheduler
	CActiveScheduler* scheduler = new CActiveScheduler;
	if (!scheduler)
		{
		return err;
		}
	CActiveScheduler::Install(scheduler);

	CTrapCleanup* cleanupStack = CTrapCleanup::New();
	
	if (cleanupStack)
		{
        TRAP(err, ExecuteL());
		delete cleanupStack;
		}

    delete scheduler;
    
	return err;
	}


// -----------------------------------------------------------------------------
// E32Main
//
// -----------------------------------------------------------------------------
//
GLDEF_C TInt E32Main()
	{
	return Execute();
	}



