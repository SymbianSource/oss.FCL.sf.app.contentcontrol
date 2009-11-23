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
* Description: Debug constants definition 
*
*/



#ifndef NSMLDSSYNC__DEBUG_H
#define NSMLDSSYNC__DEBUG_H


#ifdef _DEBUG

#include <e32svr.h>
#include <e32std.h>
#include <f32file.h>
#include <flogger.h>

_LIT(KLogFile,"smlsync.txt");
_LIT(KLogDirFullName,"c:\\logs\\");
_LIT(KLogDir,"smlsync");

// Declare the FPrint function
inline void FPrint(TRefByValue<const TDesC> aFmt, ...)
    {
    VA_LIST list;
    VA_START(list,aFmt);
    RFileLogger::WriteFormat(KLogDir, KLogFile, EFileLoggingModeAppend, TPtrC(aFmt), list);
    }


//#ifdef __WINS__     // File logging for WINS

//#ifndef __GNUC__          
//    #define FLOG            RDebug::Print
//#else   // __GNUC__       
//    #define FLOG(arg...)    RDebug::Print(arg);
//#endif // __GNUC__        


#ifndef __GNUC__          
    #define FLOG            FPrint
#else   // __GNUC__       
    #define FLOG(arg...)    FPrint(arg);
#endif // __GNUC__        

//#define FLOG   FPrint
//#define FLOG(a)   { FPrint(a); }
//#define FMTLOG(arg...)   { FPrint(arg); }
//#define FMTLOG(arg...)    RDebug::Print(arg);

//#define FLOG(a) { RDebug::Print(a);  }
#define FTRACE(a) { a; }

//#else               // RDebug logging for target HW

//#define FLOG(a) { RDebug::Print(a);  }
//#define FTRACE(a) { a; }

//#endif //__WINS__


#else // No loggings --> Reduced binary size

#define FLOG(a)
#define FTRACE(a)

#endif // _DEBUG


#endif // NSMLDSSYNC__DEBUG_H

// End of File
