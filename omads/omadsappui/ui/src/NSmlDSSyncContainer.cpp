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
#include "NSmlDSSyncContainer.h"

#include <AknUtils.h>



// ============================ MEMBER FUNCTIONS ===============================



// -----------------------------------------------------------------------------
// CNSmlDSSyncContainer::ConstructL
//
// -----------------------------------------------------------------------------
//
void CNSmlDSSyncContainer::ConstructL(const TRect& aRect)
    {
    CreateWindowL();
    SetRect(aRect);
    ActivateL();
    }


// -----------------------------------------------------------------------------
// CNSmlDSSyncContainer::~CNSmlDSSyncContainer
//
// -----------------------------------------------------------------------------
//
CNSmlDSSyncContainer::~CNSmlDSSyncContainer()
    {
    }


// -----------------------------------------------------------------------------
// CNSmlDSSyncContainer::SizeChanged
//
// Called by framework when the view size is changed.
// -----------------------------------------------------------------------------
//
void CNSmlDSSyncContainer::SizeChanged()
    {
    }


// -----------------------------------------------------------------------------
// CNSmlDSSyncContainer::CountComponentControls
//
// Returns number of controls inside this container.
// -----------------------------------------------------------------------------
//
TInt CNSmlDSSyncContainer::CountComponentControls() const
    {
    return 0;
    }


// -----------------------------------------------------------------------------
// CNSmlDSSyncContainer::ComponentControl
//
// -----------------------------------------------------------------------------
//
CCoeControl* CNSmlDSSyncContainer::ComponentControl(TInt /*aIndex*/) const
    {
    return NULL;
    }


// -----------------------------------------------------------------------------
// CNSmlDSSyncContainer::Draw
//
// -----------------------------------------------------------------------------
//
void CNSmlDSSyncContainer::Draw(const TRect& aRect) const
    {
    CWindowGc& gc = SystemGc();
    gc.SetPenStyle(CGraphicsContext::ENullPen);
    gc.SetBrushColor(KRgbGray);
    gc.SetBrushStyle(CGraphicsContext::ESolidBrush);
    gc.DrawRect(aRect);
    }


// -----------------------------------------------------------------------------
// CNSmlDSSyncContainer::HandleControlEventL
//
// -----------------------------------------------------------------------------
//
void CNSmlDSSyncContainer::HandleControlEventL(CCoeControl* /*aControl*/,TCoeEvent /*aEventType*/)
    {
    }


// -----------------------------------------------------------------------------
// CNSmlDSSyncContainer::HandleResourceChange
//
// -----------------------------------------------------------------------------
//
void CNSmlDSSyncContainer::HandleResourceChange(TInt aType)
    {
    if (aType == KEikDynamicLayoutVariantSwitch) //Handle change in layout orientation
        {
        TRect mainPaneRect;
        AknLayoutUtils::LayoutMetricsRect(AknLayoutUtils::EMainPane, mainPaneRect);
        SetRect(mainPaneRect);
		DrawNow();
		return;
		}

    CCoeControl::HandleResourceChange(aType);
    }


// End of File  
