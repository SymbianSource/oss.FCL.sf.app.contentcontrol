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

#ifndef C_NSML_DEFAULTAGENDAHANDLER_UTIL_H
#define C_NSML_DEFAULTAGENDAHANDLER_UTIL_H

#include <e32cons.h>
#include <calcommon.h>
#include <gdi.h>

/**
 * 
 * 
 *
 * @lib NSmlDefaultAgendaHandler.lib
 */
class CNSmlDefaultAgendaHandlerUtil : public CBase
  	{
    public:
        /**
        * Two-phased constructor.
        */
        static CNSmlDefaultAgendaHandlerUtil* NewL();
        
        /**
        * Destructor.
        */
        virtual ~CNSmlDefaultAgendaHandlerUtil();
    
    public: // New Functions
    
    private:
       /**
       * C++ default constructor.
       */
        CNSmlDefaultAgendaHandlerUtil();
    
    public: // Data
        HBufC* iFileName;
        HBufC* iName;
        TBool iEnabled;    
        TBool iSyncStatus;
        TTime iCreated;
        TTime iModified;
	};

#endif // C_NSML_DEFAULTAGENDAHANDLER_UTIL_H
