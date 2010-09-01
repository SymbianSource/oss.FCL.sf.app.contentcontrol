/*
* Copyright (c) 2005-2007 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:  Logging macros used by DS plug-in adapters
*
*/


#ifndef __LOGGER_H__
#define __LOGGER_H__

#ifdef _DEBUG
    #ifdef __WINS__
        // File logging for WINS
        #define __FLOGGING__
    #else
        // Logging with RDebug for target HW
        #define __CLOGGING__
        //#define __FLOGGING__ // enable to log file on target HW
    #endif //__WINS__
    
    #include <f32file.h>
    #include <flogger.h>
    #include <e32std.h>
    #include <e32def.h>
    
    _LIT(KLogDir,"MMS");
    _LIT(KLogFile,"MMSDataproviderLog.txt");
    _LIT( KLogFormat, "[MMSDS] %S");
    
    _LIT(KLogEnterFn, "%S : Begin");
    _LIT(KLogLeaveFn, "%S : End");
    
    /**
    * Old logging macros, for public use
    */
    #ifdef __FLOGGING__
        #define LOG( AAA )      RFileLogger::Write( KLogDir,KLogFile,EFileLoggingModeAppend,AAA )
    #else
        #define LOG( AAA )
    #endif
    #define LOG2( text, par1 )       { FPrint( text, par1 ); }
    
    
    /**
    * new logging macros, for public use
    */
    #define LOGGER_ENTERFN( name )      {_LIT( temp, name ); FPrint( KLogEnterFn, &temp );}
    #define LOGGER_LEAVEFN( name )      {_LIT( temp, name ); FPrint( KLogLeaveFn, &temp );}
    
    #define LOGGER_WRITE( text )                    {_LIT( KTemp, text ); FPrint( KTemp );}
    #define LOGGER_WRITE_1( text,par1 )             {_LIT( KTemp, text ); FPrint( KTemp, par1 );}
    #define LOGGER_MSG_EC( text,par1 )	                {_LIT( KTemp, text ); FPrint( KTemp, par1 );}
    
    
    // Declare the FPrint function
    inline void FPrint( TRefByValue<const TDesC> aFmt, ...)
        {
        VA_LIST list;
        VA_START(list,aFmt);
        #if defined ( __FLOGGING__ )
            RFileLogger::WriteFormat( KLogDir, KLogFile, EFileLoggingModeAppend, aFmt, list );
        #endif
        #if defined ( __CLOGGING__ )
            const TInt KMaxLogData = 0x200;
    	    TBuf< KMaxLogData > buf;
    	    buf.FormatList( aFmt, list );
    	    
    	    RDebug::Print( KLogFormat, &buf );
        #endif
        }
#else // no _DEBUG defined
    
    #define LOG( AAA )
    
    #define LOGGER_ENTERFN( name )
    #define LOGGER_LEAVEFN( name )
    #define LOGGER_WRITE( text )
    #define LOGGER_WRITE_1( text,par1 )
    #define LOGGER_MSG_EC( text,par1 )

#endif // _DEBUG

#endif // __LOGGER_H__

