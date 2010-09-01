/*
* Copyright (c) 2002 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:  Nsml DS settings Ecom plugin Impln.
*
*/


#include <e32std.h>
#include <implementationproxy.h>
#include "OmaDsAppUiPluginInterface.h"



// Constants
const TImplementationProxy KOmaDsViewImplementationTable[] = 
	{
    IMPLEMENTATION_PROXY_ENTRY( KUidNsmlDSGSPluginImpl,	COmaDsAppUiPluginInterface::NewL ) // used by GS plugin
	};

// ---------------------------------------------------------------------------
// ImplementationGroupProxy
// Gate/factory function
// ---------------------------------------------------------------------------
//
EXPORT_C const TImplementationProxy* ImplementationGroupProxy(  TInt& aTableCount )
	{
	aTableCount = sizeof( KOmaDsViewImplementationTable ) 
                / sizeof( TImplementationProxy );
	return KOmaDsViewImplementationTable;
	}
// End of File
