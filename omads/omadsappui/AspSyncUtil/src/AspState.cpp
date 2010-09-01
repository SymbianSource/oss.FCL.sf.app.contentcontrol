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

#include "AspState.h"
#include "AspUtil.h"


// ============================ MEMBER FUNCTIONS ===============================


// -----------------------------------------------------------------------------
// CAspState::NewL
//
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CAspState* CAspState::NewL()
    {
    CAspState* self = new (ELeave) CAspState();
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
    }


// -----------------------------------------------------------------------------
// Destructor
//
// -----------------------------------------------------------------------------
//
CAspState::~CAspState()
    {
    delete iSyncContent;
    }


// -----------------------------------------------------------------------------
// CAspState::CAspState
//
// -----------------------------------------------------------------------------
//
CAspState::CAspState()
    {
    }


// -----------------------------------------------------------------------------
// CAspState::ConstructL
//
// -----------------------------------------------------------------------------
//
void CAspState::ConstructL()
    {
	Reset();
	
	iSyncContent = HBufC::NewL(0);
    }


// -----------------------------------------------------------------------------
// CAspState::Reset
//
// -----------------------------------------------------------------------------
//
void CAspState::Reset()
    {
	iSyncPhase = EPhaseNone;
    }


// -----------------------------------------------------------------------------
// CAspState::SetSyncPhase
//
// -----------------------------------------------------------------------------
//
void CAspState::SetSyncPhase(TInt aPhase)
	{
	if (aPhase != iSyncPhase)
		{
		iSyncPhase = aPhase;
		}
	}


// -----------------------------------------------------------------------------
// CAspState::SyncPhase
//
// -----------------------------------------------------------------------------
//
TInt CAspState::SyncPhase()
	{
	return iSyncPhase;
	}


// -----------------------------------------------------------------------------
// CAspState::IncreaseProgress
//
// -----------------------------------------------------------------------------
//
void CAspState::IncreaseProgress()
	{
	iProgressCount++;
	iProgressCount2++;
	}


// -----------------------------------------------------------------------------
// CAspState::IncreaseProgress
//
// -----------------------------------------------------------------------------
//
void CAspState::IncreaseProgress(TInt aCount)
	{
	if (aCount > 0)
		{
		iProgressCount = iProgressCount + aCount;
		iProgressCount2++;
		}
	}


// -----------------------------------------------------------------------------
// CAspState::Progress
//
// -----------------------------------------------------------------------------
//
TInt CAspState::Progress()
	{
	return iProgressCount;
	}


// -----------------------------------------------------------------------------
// CAspState::SetProgress
//
// -----------------------------------------------------------------------------
//
/*
void CAspState::SetProgress(TInt aCount)
	{
	if (aCount >= 0 && aCount <= KTotalProgress)
		{
		iProgressCount = aCount;
		iProgressCount2++;
		}
	}
*/

// -----------------------------------------------------------------------------
// CAspState::SetTotalProgress
//
// -----------------------------------------------------------------------------
//
void CAspState::SetTotalProgress(TInt aCount)
	{
	if (aCount < 0)
		{
		iTotalProgressCount = KTotalProgressUnknown;
		return;
		}
		
	iTotalProgressCount = aCount;
	}


// -----------------------------------------------------------------------------
// CAspState::TotalProgress
//
// -----------------------------------------------------------------------------
//
TInt CAspState::TotalProgress()
	{
	return iTotalProgressCount;
	}


// -----------------------------------------------------------------------------
// CAspState::FirstProgress
//
// -----------------------------------------------------------------------------
//
TBool CAspState::FirstProgress()
	{
	if (iProgressCount2 == 1)
		{
		return ETrue;
		}
		
	return EFalse;
	}


// -----------------------------------------------------------------------------
// CAspState::SetContentL
//
// -----------------------------------------------------------------------------
//
void CAspState::SetContentL(const TDesC& aText)
	{
	delete iSyncContent;
	iSyncContent = NULL;
	iSyncContent = aText.AllocL();
	}


// -----------------------------------------------------------------------------
// CAspState::Content
//
// -----------------------------------------------------------------------------
//
const TDesC& CAspState::Content()
	{
	__ASSERT_ALWAYS(iSyncContent, TUtil::Panic(KErrGeneral));
		
	return *iSyncContent;
	}


// -----------------------------------------------------------------------------
// CAspState::ResetProgress
//
// -----------------------------------------------------------------------------
//
void CAspState::ResetProgress()
	{
	iProgressCount = 0;
	iProgressCount2 = 0;
	iTotalProgressCount = 0;
	
	TRAP_IGNORE(SetContentL(KNullDesC));
	}


// -----------------------------------------------------------------------------
// CAspState::ProgressKnown
//
// -----------------------------------------------------------------------------
//
TBool CAspState::ProgressKnown()
	{
	if (iTotalProgressCount == KTotalProgressUnknown)
		{
		return EFalse;
		}

	return ETrue;
	}


// End of file

