/*
* Copyright (c) 2005 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:  DS agenda progress view
*
*/



// INCLUDES
#include "nsmlagendadebug.h"
#include "nsmlagendadatastore.h"
#include <nsmldebug.h>

// ===================================== MEMBER FUNCTIONS ======================
// -----------------------------------------------------------------------------
// CNSmlAgendaProgressview::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
CNSmlAgendaProgressview* CNSmlAgendaProgressview::NewL()
	{
	FLOG(_L("CNSmlAgendaProgressview::NewL(): BEGIN"));
	CNSmlAgendaProgressview* self = new (ELeave) CNSmlAgendaProgressview();
	self->ConstructL();
	FLOG(_L("CNSmlAgendaProgressview::NewL(): END"));
	return self;
	}
	
// -----------------------------------------------------------------------------
// CNSmlAgendaProgressview::~CNSmlAgendaProgressview
// Destructor.
// -----------------------------------------------------------------------------
CNSmlAgendaProgressview::~CNSmlAgendaProgressview()
	{
	FLOG(_L("CNSmlAgendaProgressview::~CNSmlAgendaProgressview(): BEGIN"));
	// Nothing to do
	FLOG(_L("CNSmlAgendaProgressview::~CNSmlAgendaProgressview(): END"));
	}

// -----------------------------------------------------------------------------
// CNSmlAgendaProgressview::Progress
// This calls the observing class with the percentage complete of the current
// operation.
// -----------------------------------------------------------------------------
void CNSmlAgendaProgressview::Progress( TInt /*aPercentageCompleted*/ )
	{
	FLOG(_L("CNSmlAgendaProgressview::Progress(): BEGIN"));
	FLOG(_L("CNSmlAgendaProgressview::Progress(): END"));
	}
	
// -----------------------------------------------------------------------------
// CNSmlAgendaProgressview::Completed
// This calls the observing class when the current operation is finished.
// -----------------------------------------------------------------------------
void CNSmlAgendaProgressview::Completed( TInt aError )
	{
	FLOG(_L("CNSmlAgendaProgressview::Completed(): BEGIN"));

    iCompletedStatus = aError;
	CActiveScheduler::Stop();

    FLOG(_L("CNSmlAgendaProgressview::Completed(): END"));
	}

// -----------------------------------------------------------------------------
// CNSmlAgendaProgressview::GetCompletedStatus
// This method returns status of Completed method (see above).
// -----------------------------------------------------------------------------
TInt CNSmlAgendaProgressview::GetCompletedStatus()
    {
    return iCompletedStatus;
    }
	
// -----------------------------------------------------------------------------
// CNSmlAgendaProgressview::NotifyProgress
// Asks the observing class whether progress callbacks are required.
// -----------------------------------------------------------------------------
TBool CNSmlAgendaProgressview::NotifyProgress()
	{
	FLOG(_L("CNSmlAgendaProgressview::NotifyProgress(): BEGIN"));
	FLOG(_L("CNSmlAgendaProgressview::NotifyProgress(): END"));
	return EFalse;
	}

// -----------------------------------------------------------------------------
// CNSmlAgendaProgressview::CNSmlAgendaProgressview
// C++ default constructor.
// -----------------------------------------------------------------------------
CNSmlAgendaProgressview::CNSmlAgendaProgressview() :
	iCompletedStatus( KErrNotReady )
	{
	FLOG(_L("CNSmlAgendaProgressview::CNSmlAgendaProgressview(): BEGIN"));
	// Nothing to do
	FLOG(_L("CNSmlAgendaProgressview::CNSmlAgendaProgressview(): END"));
	}
	
// -----------------------------------------------------------------------------
// CNSmlAgendaProgressview::ConstructL
// Second phase constructor.
// -----------------------------------------------------------------------------
void CNSmlAgendaProgressview::ConstructL()
    {
    FLOG(_L("CNSmlAgendaProgressview::ConstructL(): BEGIN"));
    // Nothing to do
    FLOG(_L("CNSmlAgendaProgressview::ConstructL(): END"));
    }

//  End of File  
