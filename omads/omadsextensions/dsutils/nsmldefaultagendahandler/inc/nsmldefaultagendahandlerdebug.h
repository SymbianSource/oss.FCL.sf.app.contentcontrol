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
* Description: Debug macros definition and constant declaration 
*
*/



#ifndef NSML_DEFAULTAGENDAHANDLER_DEBUG_H
#define NSML_DEFAULTAGENDAHANDLER_DEBUG_H

#include <e32svr.h>
#include <e32std.h>
#include <f32file.h>
#include <flogger.h>

_LIT(KLogFile,"DefaultAgendaHandler.txt");
_LIT(KLogDirFullName,"c:\\logs\\");
_LIT(KLogDir,"MultiCalDB");

#ifdef _DEBUG

// Declare the FPrint function
inline void FPrint(const TRefByValue<const TDesC> aFmt, ...)
    {
    VA_LIST list;
    VA_START(list,aFmt);
    RFileLogger::WriteFormat(KLogDir, KLogFile, EFileLoggingModeAppend, aFmt, list);
    }

// ===========================================================================
#ifdef __WINS__     // File logging for WINS
// ===========================================================================
#define FLOG(arg...)   { FPrint(arg); }
//#define FLOG(a) { RDebug::Print(a);  }
#define FTRACE(a) { a; }
// ===========================================================================
#else               // RDebug logging for target HW
// ===========================================================================
//#define FLOG(arg...) { RDebug::Print(arg);  }
#define FLOG(arg...)   { FPrint(arg); }
#define FTRACE(a) { a; }
#endif //__WINS__

// ===========================================================================
#else // // No loggings --> Reduced binary size
// ===========================================================================
#define FLOG(arg...)
#define FTRACE(a)

#endif // _DEBUG

#endif // NSML_DEFAULTAGENDAHANDLER_DEBUG_H

// End of File
