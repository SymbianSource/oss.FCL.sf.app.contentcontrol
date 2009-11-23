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
* Description:  Utility classes.
*
*/


#ifndef _AIW_SYNCPROVIDER_H
#define _AIW_SYNCPROVIDER_H

// INCLUDES
#include <e32base.h>
#include <eikenv.h>


// CONSTANTS

const TInt KBufSize = 128;



// FORWARD DECLARATIONS

// CLASS DECLARATION


/**
* TAspUtil 
* 
*/
class TUtil
	{
    public:
        static void Panic(TInt aReason);
		static void StrCopy(TDes& aTarget, const TDesC& aSource);
		static TInt StrToInt(const TDesC& aText, TInt& aNum);
		static void Print(const TDesC& aText);
		static void Print(const TDesC& aText, TInt aNum);
	};


/**
* TFileLog 
* 
*/
class TFileLog
    {
    public:
        static void Log(const TDesC& aText);
		static void Log(const TDesC& aText1, const TDesC& aText2);
		static void GetDateTime(TDes& aStr, TTime aTimeNum, TBool aDate, TBool aTime);
		static void GetMediumType(TDes& aText, TInt aType);
    };



#endif  // _AIW_SYNCPROVIDER_H