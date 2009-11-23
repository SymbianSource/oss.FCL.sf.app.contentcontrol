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




// INCLUDES

#include <SyncMLTransportProperties.h>
#include <flogger.h>
#include <eikenv.h>
#include <bautils.h>  // BaflUtils
#include <AiwGenericParam.hrh>  // for application ids
#include <collate.h>   // for TCollationMethod

#include <featmgr.h>   // FeatureManager
#include <ApUtils.h>   // CApUtils
#include <avkon.rsg>
#include <hlplch.h>    // HlpLauncher

#include <MuiuMsvUiServiceUtilities.h> // for MsvUiServiceUtilities


#include "AspUtil.h"
#include "AspDialogUtil.h"
#include "AspResHandler.h"
#include "AspDebug.h"
#include <AspSyncUtil.rsg>

#include <SyncMLErr.h>  // sync error codes

//#ifdef __SAP_POLICY_MANAGEMENT
#include <SettingEnforcementInfo.h> // VSettingEnforcementInfo
//#endif





/******************************************************************************
 * class TAspParam
 ******************************************************************************/


// -------------------------------------------------------------------------------
// TAspParam::TAspParam
//
// -------------------------------------------------------------------------------
//
TAspParam::TAspParam(TInt aApplicationId)
 : iApplicationId(aApplicationId), iProfileId(KErrNotFound), iMode(KErrNotFound),
   iId(KErrNotFound), iReturnValue(KErrNotFound), iDataProviderId(KErrNotFound),
   iSyncTaskId(KErrNotFound), iProfile(NULL), 
   iSyncTask(NULL), iSyncFilter(NULL), iSyncSession(NULL),
   iProfileList(NULL), iContentList(NULL), iSchedule(NULL)
	 {
	 }


// -------------------------------------------------------------------------------
// TAspParam::TAspParam
//
// -------------------------------------------------------------------------------
//
TAspParam::TAspParam(TInt aApplicationId, RSyncMLSession* aSyncSession)
 : iApplicationId(aApplicationId), iProfileId(KErrNotFound), iMode(KErrNotFound),
   iId(KErrNotFound), iReturnValue(KErrNotFound), iDataProviderId(KErrNotFound),
   iSyncTaskId(KErrNotFound), iProfile(NULL),
   iSyncTask(NULL), iSyncFilter(NULL), iSyncSession(aSyncSession),
   iProfileList(NULL), iContentList(NULL), iSchedule(NULL)
  	 {
	 }



/******************************************************************************
 * class TAspUtil
 ******************************************************************************/


// -------------------------------------------------------------------------------
// TUtil::Panic
//
// -------------------------------------------------------------------------------
//
void TUtil::Panic(TInt aReason)
    {
	_LIT(KPanicCategory,"AspSyncUtil");

	User::Panic(KPanicCategory, aReason); 
    }


// -------------------------------------------------------------------------------
// TUtil::StrCopy
//
// String copy with lenght check.
// -------------------------------------------------------------------------------
//
void TUtil::StrCopy(TDes8& aTarget, const TDesC& aSource)
    {
	TInt len = aTarget.MaxLength();
    if(len < aSource.Length()) 
	    {
		aTarget.Copy(aSource.Left(len));
		return;
	    }
	aTarget.Copy(aSource);
    }

// -------------------------------------------------------------------------------
// TUtil::StrCopy
//
// String copy with lenght check.
// -------------------------------------------------------------------------------
//
void TUtil::StrCopy(TDes& aTarget, const TDesC8& aSource)
    {
	TInt len = aTarget.MaxLength();
    if(len < aSource.Length()) 
	    {
		aTarget.Copy(aSource.Left(len));
		return;
	    }
	aTarget.Copy(aSource);
    }


// -------------------------------------------------------------------------------
// TUtil::StrCopy
//
// String copy with lenght check.
// -------------------------------------------------------------------------------
//
void TUtil::StrCopy(TDes& aTarget, const TDesC& aSource)
    {
	TInt len = aTarget.MaxLength();
    if(len < aSource.Length()) 
	    {
		aTarget.Copy(aSource.Left(len));
		return;
	    }
	aTarget.Copy(aSource);
    }

//--------------------------------------------------------------------------------
//TUtil::StrConversion
//
//Convert TBuf to HBuf
//--------------------------------------------------------------------------------
//
HBufC* TUtil::StrConversion(const TDesC& aDefaultText)
{
	return aDefaultText.AllocLC();
    }


// -------------------------------------------------------------------------------
// TUtil::StrAppend
//
// -------------------------------------------------------------------------------
//
void TUtil::StrAppend(TDes& aTarget, const TDesC& aSource)
{
    if (aSource.Length() == 0)
    	{
    	return;
    	}
    	
	TInt free = aTarget.MaxLength() - aTarget.Length();
	if (aSource.Length() < free)
		{
		aTarget.Append(aSource);
		}
}


// -------------------------------------------------------------------------------
// TUtil::StrInsert
//
// -------------------------------------------------------------------------------
//
void TUtil::StrInsert(TDes& aTarget, const TDesC& aSource)
{
	TInt free = aTarget.MaxLength() - aTarget.Length();
	if (aSource.Length() < free)
		{
		aTarget.Insert(0, aSource);
		}
}


// -------------------------------------------------------------------------------
// TUtil::StrToInt
//
// -------------------------------------------------------------------------------
//
TInt TUtil::StrToInt(const TDesC& aText, TInt& aNum)
	{
    TLex lex(aText);

    TInt err=lex.Val(aNum); 
	return err;
	}


// -------------------------------------------------------------------------------
// TUtil::IsEmpty
//
// Function returns ETrue if string only contains white space or has no characters.
// -------------------------------------------------------------------------------
//
TBool TUtil::IsEmpty(const TDesC& aText)
	{
	TInt len=aText.Length();
	for (TInt i=0; i<len; i++)
		{
		TChar c=aText[i];
		if (!c.IsSpace())
			{
			return EFalse;
			}
		}
	return ETrue;
	}


// -------------------------------------------------------------------------------
// TUtil::Fill
//
// -------------------------------------------------------------------------------
//
void TUtil::Fill(TDes& aTarget, const TDesC& aOneCharStr, TInt aLength)
	{
	TInt len = aTarget.MaxLength();
    if(len > aLength)
    	{
    	TBuf<KBufSize16> buf(aOneCharStr);
	    aTarget.Fill(buf[0], aLength);
    	}
	}


// -----------------------------------------------------------------------------
// TUtil::AddResFileL
//
// -----------------------------------------------------------------------------
//
TInt TUtil::AddResFileL(const TDesC& aFile)
	{
	CEikonEnv* env = CEikonEnv::Static();

	TFileName fileName(aFile);

	BaflUtils::NearestLanguageFile(env->FsSession(), fileName);

    return env->AddResourceFileL(fileName);
	}


// -----------------------------------------------------------------------------
// TUtil::IsValidAppId
//
// -----------------------------------------------------------------------------
//
TBool TUtil::IsValidAppId(TInt aApplicationId)
	{
	TInt id = aApplicationId;
	
	if (id == EApplicationIdSync || id == EApplicationIdContact || 
	    id == EApplicationIdCalendar || id == EApplicationIdEmail ||
	    id == EApplicationIdNote || id == EApplicationIdMms)
		{
		return ETrue;
		}
		
	return EFalse;
	}


// -----------------------------------------------------------------------------
// TUtil::IsValidSID
//
// -----------------------------------------------------------------------------
//
TBool TUtil::IsValidSID(TInt aSid)
	{
	TInt id = aSid;
	
	if (id == KUidAppSync.iUid || id == KUidAppPhonebook.iUid || 
	    id == KUidAppCalendar.iUid || id == KUidAppMessages.iUid ||
	    id == KUidAppNotepad.iUid || id == KUidAppTest.iUid || 
	    id == KUidAppMailEditor.iUid)
		{
		return ETrue;
		}
		
	return EFalse;
	}


// -----------------------------------------------------------------------------
// TUtil::ProviderIdFromAppId
//
// -----------------------------------------------------------------------------
//
TInt TUtil::ProviderIdFromAppId(TInt aApplicationId)
	{
	TInt id=KErrNotFound;

	switch (aApplicationId)
		{
		case EApplicationIdContact:
			id = KUidNSmlAdapterContact.iUid;
			break;
		case EApplicationIdCalendar:
			id = KUidNSmlAdapterCalendar.iUid;
			break;
		case EApplicationIdEmail:
     		id = KUidNSmlAdapterEMail.iUid;
			break;
		case EApplicationIdNote:
			id = KUidNSmlAdapterNote.iUid;
			break;
		default:
            id = KErrNotFound;
			break;
		}

	return id;
	}


// -----------------------------------------------------------------------------
// TUtil::AppIdFromProviderId
//
// -----------------------------------------------------------------------------
//
TInt TUtil::AppIdFromProviderId(TInt aAdapterId)
	{
	TInt id=KErrNotFound;

	if (aAdapterId == KUidNSmlAdapterContact.iUid)
		{
		id = EApplicationIdContact;
		}
	if (aAdapterId == KUidNSmlAdapterCalendar.iUid)
		{
		id = EApplicationIdCalendar;
		}
	if (aAdapterId == KUidNSmlAdapterEMail.iUid)
		{
		id = EApplicationIdEmail;
		}
	if (aAdapterId == KUidNSmlAdapterNote.iUid)
		{
		id = EApplicationIdNote;
		}

	return id;
	}


// -----------------------------------------------------------------------------
// TUtil::IsToDay
//
// -----------------------------------------------------------------------------
//
TBool TUtil::IsToday(TTime aTime)
	{
	TTime now;
	now.HomeTime();
	
	TInt day1 = now.DayNoInYear();
	TInt day2 = aTime.DayNoInYear();
	
	TDateTime date1 = now.DateTime();
	TDateTime date2 = aTime.DateTime();
	
	
	TTimeIntervalDays daysBetween = now.DaysFrom(aTime);
	
	if (day1 != day2 || date1.Year() != date2.Year())
		{
		return EFalse;
		}
		
	if (daysBetween.Int() > 0)
		{
		return EFalse;
		}
		
	return ETrue; 
	}


// -----------------------------------------------------------------------------
// TUtil::SyncTimeLC
//
// -----------------------------------------------------------------------------
//
HBufC* TUtil::SyncTimeLC(TTime aLastSync)
	{
	TTime homeTime = ConvertUniversalToHomeTime(aLastSync);
	
	HBufC* hBuf = HBufC::NewLC(KBufSize255);
	TPtr ptr = hBuf->Des();

	if (IsToday(homeTime))
		{
		TBuf<KBufSize> timeFormat;
		CAspResHandler::ReadL(timeFormat, R_QTN_TIME_USUAL_WITH_ZERO);
		homeTime.FormatL(ptr, timeFormat);
		}
	else
		{
		TBuf<KBufSize> dateFormat;
		CAspResHandler::ReadL(dateFormat, R_QTN_DATE_USUAL_WITH_ZERO);
		homeTime.FormatL(ptr, dateFormat);
		}
   
	return hBuf;
	}
	
	
// -----------------------------------------------------------------------------
// TUtil::ConvertUniversalToHomeTime
//
// -----------------------------------------------------------------------------
//
TTime TUtil::ConvertUniversalToHomeTime(const TTime& aUniversalTime)
    {
    TTime time(aUniversalTime);   // time stores UTC time.

    TLocale locale;
    TTimeIntervalSeconds universalTimeOffset(locale.UniversalTimeOffset());
    
    // Add locale's universal time offset to universal time.
    time += universalTimeOffset;    // time stores Local Time.

    // If home daylight saving in effect, add one hour offset.
    if (locale.QueryHomeHasDaylightSavingOn())
        {
        TTimeIntervalHours daylightSaving(1);
        time += daylightSaving;
        }

    return time;
    }
	

// -----------------------------------------------------------------------------
// TUtil::GetDateTextL
//
// -----------------------------------------------------------------------------
//
void TUtil::GetDateTextL(TDes& aText, TTime aDateTime)
	{
	TTime homeTime = ConvertUniversalToHomeTime(aDateTime);
	
	HBufC* hBuf = CAspResHandler::ReadLC(R_QTN_DATE_USUAL_WITH_ZERO);
    homeTime.FormatL(aText, *hBuf);
    CleanupStack::PopAndDestroy(hBuf);
	}


// -----------------------------------------------------------------------------
// TUtil::GetTimeTextL
//
// -----------------------------------------------------------------------------
//
void TUtil::GetTimeTextL(TDes& aText, TTime aDateTime)
	{
	TTime homeTime = ConvertUniversalToHomeTime(aDateTime);
	
	HBufC* hBuf = CAspResHandler::ReadLC(R_QTN_TIME_USUAL_WITH_ZERO);
    homeTime.FormatL(aText, *hBuf);
    CleanupStack::PopAndDestroy(hBuf);
	}


// -----------------------------------------------------------------------------
// TUtil::GetDateTimeTextL
//
// -----------------------------------------------------------------------------
//
void TUtil::GetDateTimeTextL(TDes& aText, TTime aDateTime)
	{
	TDateTime dt = aDateTime.DateTime();
	aText.Format(_L("%02d.%02d.%04d %02d:%02d:%02d"), dt.Day()+1, dt.Month()+1, dt.Year(),  dt.Hour(), dt.Minute(), dt.Second());
	}


// -----------------------------------------------------------------------------
// TUtil::LaunchHelpAppL
//
// -----------------------------------------------------------------------------
//
void TUtil::LaunchHelpAppL()
	{
	if (!FeatureManager::FeatureSupported(KFeatureIdHelp))
		{
		return;
		}
		
	CEikonEnv* eikEnv = CEikonEnv::Static();
	HlpLauncher::LaunchHelpApplicationL(eikEnv->WsSession(), eikEnv->EikAppUi()->AppHelpContextL());
	}


// -----------------------------------------------------------------------------
// TUtil::LaunchHelpAppL
//
// -----------------------------------------------------------------------------
//
void TUtil::LaunchHelpAppL(CEikonEnv* aEikonkEnv)
	{
	if (!FeatureManager::FeatureSupported(KFeatureIdHelp))
		{
		return;
		}
		
	CEikonEnv* eikEnv = CEikonEnv::Static();
	HlpLauncher::LaunchHelpApplicationL(aEikonkEnv->WsSession(), 
	                                    aEikonkEnv->EikAppUi()->AppHelpContextL());
	}


// ---------------------------------------------------------
// TUtil::SettingEnforcementStateL
// 
// Checks if setting enforcement is activated.
// ---------------------------------------------------------
TBool TUtil::SettingEnforcementStateL()
    {
    
 FeatureManager::InitializeLibL();
 if(!FeatureManager::FeatureSupported(KFeatureIdSapPolicyManagement))
 {
		//#ifndef __SAP_POLICY_MANAGEMENT
		FeatureManager::UnInitializeLib();
		    return EFalse;
		//#else
}
else
{
	FeatureManager::UnInitializeLib();
	CSettingEnforcementInfo* info = CSettingEnforcementInfo::NewL();
	CleanupStack::PushL(info);

	TBool active = EFalse;
	User::LeaveIfError(info->EnforcementActive(EDataSyncEnforcement, active));
	CleanupStack::PopAndDestroy(info);

	return active;
 //#endif
}


    }


// ---------------------------------------------------------
// TUtil::SettingEnforcementState
// 
// Checks if setting enforcement is activated.
// ---------------------------------------------------------
TBool TUtil::SettingEnforcementState()
    {
    TBool ret = EFalse;
    
    TRAP_IGNORE(ret = SettingEnforcementStateL());
    
    return ret;
    }


// ---------------------------------------------------------
// TUtil::NumberConversionLC
// 
// ---------------------------------------------------------
CDesCArray* TUtil::NumberConversionLC(CDesCArray* aList)
    {
    CDesCArrayFlat* arr = new (ELeave) CDesCArrayFlat(KDefaultArraySize);
    CleanupStack::PushL(arr);
    
    TInt count = aList->Count();
    
    for (TInt i=0; i<count; i++)
    	{
    	TBuf<KBufSize> buf((*aList)[i]);
     	AknTextUtils::DisplayTextLanguageSpecificNumberConversion(buf);
     	arr->AppendL(buf);
    	}
    	
    return arr;
    }



#ifdef _DEBUG

// -------------------------------------------------------------------------------
// TUtil::Print
//
// -------------------------------------------------------------------------------
//
void TUtil::Print(const TDesC& aText)
    {
	CEikonEnv::Static()->InfoMsg(aText);
	}


// -------------------------------------------------------------------------------
// TUtil::Print
//
// -------------------------------------------------------------------------------
//
void TUtil::Print(const TDesC& aText, TInt aNum)
    {
	_LIT(KFormat, "%S (%d)");

	TBuf<KBufSize> buf;
	buf.Format(KFormat, &aText, aNum);
	CEikonEnv::Static()->InfoMsg(buf);
	}


// -------------------------------------------------------------------------------
// TUtil::GetMandatoryFieldText
//
// -------------------------------------------------------------------------------
//
void TUtil::GetMandatoryFieldText(TDes& aText, TInt aStatus)
    {
	switch (aStatus)
		{
		case EMandatoryNoProfileName:
            aText = _L("no profile name");
			break;
		case EMandatoryNoHostAddress:
			aText = _L("no host address");
			break;
		case EMandatoryNoContent:
			aText = _L("no content");
			break;
		case EMandatoryNoLocalDatabase:
			aText = _L("no local database");
			break;
		case EMandatoryNoRemoteDatabase:
			aText = _L("no remote database");
			break;
		default:
			aText = _L("profile ok");
			break;
		}
    }


// -----------------------------------------------------------------------------
// TUtil::TimeBefore
//
// Function returns TTime of the current time (needed with function TimeAfter).
// -----------------------------------------------------------------------------
//
TTime TUtil::TimeBefore()
{
    TTime now;
	now.UniversalTime();
	return now;
}


// -----------------------------------------------------------------------------
// TUtil::TimeAfter
//
// Function returns milliseconds passed since aBefore.
// -----------------------------------------------------------------------------
//
TInt TUtil::TimeAfter(TTime aBefore)
{
	TTime after;
	after.UniversalTime();
	TTimeIntervalMicroSeconds microSeconds = after.MicroSecondsFrom(aBefore); 
	TInt seconds = microSeconds.Int64()/1000;
	return seconds;
}


// -----------------------------------------------------------------------------
// TUtil::TimeAfter
//
// -----------------------------------------------------------------------------
//
void TUtil::TimeAfter(TTime aBefore, const TDesC& aText)
    {
	TTime after;
	after.UniversalTime();
	TTimeIntervalMicroSeconds microSeconds = after.MicroSecondsFrom(aBefore); 
	
	FTRACE( RDebug::Print(_L("%S %d ms"), &aText, microSeconds.Int64()/1000) );
    }


// -----------------------------------------------------------------------------
// TUtil::GetApplicationNameL
//
// -----------------------------------------------------------------------------
//
void TUtil::GetApplicationName(TDes& aText, TInt aApplicationId)
    {
	switch (aApplicationId)
		{
		case EApplicationIdSync:
			aText = _L("EApplicationIdSync");
			break;
		case EApplicationIdContact:
			aText = _L("EApplicationIdContact");
			break;
		case EApplicationIdCalendar:
			aText = _L("EApplicationIdCalendar");
			break;
		case EApplicationIdEmail:
			aText = _L("EApplicationIdEmail");
			break;
		case EApplicationIdMms:
			aText = _L("EApplicationIdMms");
			break;
		case EApplicationIdNote:
			aText = _L("EApplicationIdNote");
			break;
		default:
			aText = _L("unknown app");
			break;
		}
    }

#endif



/******************************************************************************
 * class TURIParser
 ******************************************************************************/


// ---------------------------------------------------------
// TURIParser::TURIParser
// 
// ---------------------------------------------------------
TURIParser::TURIParser(const TDesC& aStr) : iOriginalStr(aStr)
    {
    SkipHttp();
    }


// ---------------------------------------------------------
// TURIParser::GetUriWithoutPort
// 
// URI format: "http://serveraddress/documentname/"
// ---------------------------------------------------------
void TURIParser::GetUriWithoutPort(TDes& aText)
	{
	_LIT(KSlash, "/");
	
	aText = KNullDesC;
	
	TPtrC http = ParseHttp();
	TPtrC address = ParseAddress();
	TPtrC document = ParseDocument();
	
	TUtil::StrAppend(aText, http);
	TUtil::StrAppend(aText, address);
	
	if (document.Length() > 0)
		{
		TUtil::StrAppend(aText, KSlash);
		TUtil::StrAppend(aText, document);
		}
	//else if (aText.Length() > 0)
	//	{
	//	TUtil::StrAppend(aText, KSlash);
	//	}
	}
	

// ---------------------------------------------------------
// TURIParser::GetUriWithPort
// 
// URI format: "http://serveraddress:port/documentname/"
// ---------------------------------------------------------
void TURIParser::GetUriWithPort(TDes& aText, const TInt aPort)
	{
	_LIT(KColon, ":");
	_LIT(KSlash, "/");
	
	aText = KNullDesC;
	
	TPtrC http = ParseHttp();
	
	TPtrC address = ParseAddress();
	
	TPtrC document = ParseDocument();
	
	TUtil::StrAppend(aText, http);
	TUtil::StrAppend(aText, address);
	
	TBuf<KBufSize32> buf;
	buf.Num(aPort);
	TUtil::StrAppend(aText, KColon);
	TUtil::StrAppend(aText, buf);
	
	if (document.Length() > 0)
		{
		TUtil::StrAppend(aText, KSlash);
		TUtil::StrAppend(aText, document);
		}
	//else if (aText.Length() > 0)
	//	{
	//	TUtil::StrAppend(aText, KSlash);
	//	}
	}


// ---------------------------------------------------------
// TURIParser::Port
// 
// ---------------------------------------------------------
TInt TURIParser::Port()
	{
	TInt port = KErrNotFound;
	
	TInt err = ParsePort(port);
	if (err != KErrNone)
		{
		return KErrNotFound;
		}
		
	return port;
	}


// ---------------------------------------------------------
// TURIParser::DefaultPort
// 
// ---------------------------------------------------------
TInt TURIParser::DefaultPort()
	{
	_LIT(KHttpsHeader, "https://");
	
	TInt pos = iOriginalStr.FindF(KHttpsHeader);
	if (pos != KErrNotFound)
		{
		return EDefaultHttpsPort;
		}

	return EDefaultHttpPort;
	}


// ---------------------------------------------------------
// TURIParser::IsValidUri
// 
// ---------------------------------------------------------
TBool TURIParser::IsValidUri(const TDesC& aText)
	{
	_LIT(KHttpHeader, "http://");
	_LIT(KHttpsHeader, "https://");

	const TInt KHttpLength = 7;
	const TInt KHttpsLength = 8;
	
	// IsValidDomainL this does not accept comma ("http://" fails)
	//return MsvUiServiceUtilities::IsValidDomainL(aText);
	
	if (TUtil::IsEmpty(aText))
		{
		return EFalse;
		}
		
    if (aText.Compare(KHttpHeader) == 0)
    	{
    	return EFalse;
    	}
	
    if (aText.Compare(KHttpsHeader) == 0)
    	{
    	return EFalse;
    	}
    
    if (aText.Left(KHttpLength).Compare(KHttpHeader) != 0
    				&& aText.Left(KHttpsLength).Compare(KHttpsHeader) != 0)
    	{
    	return EFalse;
    	}
	
	return ETrue;
	}

// ---------------------------------------------------------
// TURIParser::IsValidBluetoothHostAddress
// 
// ---------------------------------------------------------
TBool TURIParser::IsValidBluetoothHostAddress(const TDesC& aText)
	{
	_LIT(KHttpHeader, "http://");
	_LIT(KHttpsHeader, "https://");

	const TInt KHttpLength = 7;
	const TInt KHttpsLength = 8;
	
	// IsValidDomainL this does not accept comma ("http://" fails)
	//return MsvUiServiceUtilities::IsValidDomainL(aText);
	
	if (TUtil::IsEmpty(aText))
		{
		return EFalse;
		}
		
    if (aText.Compare(KHttpHeader) == 0)
    	{
    	return EFalse;
    	}
	
    if (aText.Compare(KHttpsHeader) == 0)
    	{
    	return EFalse;
    	}
    
    if (aText.Left(KHttpLength).Compare(KHttpHeader) == 0
    				|| aText.Left(KHttpsLength).Compare(KHttpsHeader) == 0)
    	{
    	return EFalse;
    	}
	
	return ETrue;
	}
// ---------------------------------------------------------
// TURIParser::IsValidPort
// 
// ---------------------------------------------------------
TBool TURIParser::IsValidPort(const TInt aPort)
	{
	if (aPort < TURIParser::EMinURIPortNumber || 
	    aPort > TURIParser::EMaxURIPortNumber)
		{
		return EFalse;
		}
		
	return ETrue;
	}


// ---------------------------------------------------------
// TURIParser::SkipHttp
// 
// Set pointer to the end of "http://".
// ---------------------------------------------------------
void TURIParser::SkipHttp()
	{
    TInt start = KErrNotFound;
    TInt end   = KErrNotFound;
    TStr str(iOriginalStr);
    
    GetHttpPos(start, end);
    
    if (start != KErrNotFound)
    	{
    	iPtr.Set(str.Right(end+1));
    	}
    else
    	{
    	iPtr.Set(str.Right(0));
    	}
	}


// ---------------------------------------------------------
// TURIParser::ParseHttp
// 
// ---------------------------------------------------------
TPtrC TURIParser::ParseHttp()
	{
    TInt start = KErrNotFound;
    TInt end   = KErrNotFound;
    
    GetHttpPos(start, end);
    
    if (start == KErrNotFound)
    	{
    	return TPtrC();
    	}
    
   	TStr str(iOriginalStr);
    return str.SubStr(start, end);
	}


// ---------------------------------------------------------
// TURIParser::GetHttpPos
// 
// ---------------------------------------------------------
void TURIParser::GetHttpPos(TInt& aStartPos, TInt& aEndPos)
	{
	_LIT(KHttpHeader, "http://");
    _LIT(KHttpsHeader, "https://");

	aEndPos = KErrNotFound;
	
	aStartPos = iOriginalStr.FindF(KHttpHeader);
	if (aStartPos != KErrNotFound)
		{
		aEndPos = aStartPos + KHttpHeader().Length() - 1;
		return;
		}
	
	aStartPos = iOriginalStr.FindF(KHttpsHeader);
	if (aStartPos != KErrNotFound)
		{
		aEndPos = aStartPos + KHttpsHeader().Length() - 1;
		return;
		}
	}


// ---------------------------------------------------------
// TURIParser::ParseAddress
// 
// ---------------------------------------------------------
TPtrC TURIParser::ParseAddress()
	{
	const TChar KSlash('/');
	const TChar KColon(':');
	
	TStr str(iPtr);
	
    TInt firstSlash = str.Find(0, KSlash);
	TInt firstcolon = str.Find(0, KColon);
	
	TBool portFound = EFalse;
	if (firstcolon != KErrNotFound)
		{
		if (firstSlash == KErrNotFound)
			{
			portFound = ETrue;
			}
		else if (firstcolon < firstSlash)
			{
			portFound = ETrue;
			}
		}
    
	if (portFound)
		{
		// address is text before first colon
		return str.Left(firstcolon-1);
		}
	else if (firstSlash != KErrNotFound)
		{
		// address is text before first slash
		return str.Left(firstSlash-1);
		}
	else
		{
		// address is all text
		return str.Right(0);
		}
	}


// ---------------------------------------------------------
// TURIParser::ParseDocument
// 
// ---------------------------------------------------------
TPtrC TURIParser::ParseDocument()
	{
	const TChar KSlash('/');
	
	TStr str(iPtr);
    TInt firstSlash = str.Find(0, KSlash);
    
	if (firstSlash != KErrNotFound)
		{
		// document is all text after first slash
		return str.Right(firstSlash+1);
		}
		
	return TPtrC();
	
	}


// ---------------------------------------------------------
// TURIParser::ParsePort
// 
// ---------------------------------------------------------
TInt TURIParser::ParsePort(TInt& aNumber)
	{
	TPtrC port = ParsePort();
	if (port.Length() == 0)
		{
		return KErrNotFound;
		}
		
	return TUtil::StrToInt(port, aNumber);
	}


// ---------------------------------------------------------
// TURIParser::ParsePort
// 
// ---------------------------------------------------------
TPtrC TURIParser::ParsePort()
	{
	const TChar KSlash('/');
	const TChar KColon(':');
	
	TStr str(iPtr);
    TInt firstSlash = str.Find(0, KSlash);
	TInt firstColon = str.Find(0, KColon);
    
	if (firstSlash != KErrNotFound)
		{
		if (firstColon != KErrNotFound && firstColon < firstSlash)
			{
		    // port number is text between colon and slash 
		    return str.SubStrEx(firstColon, firstSlash);
			}
		}
	else 
		{
		if (firstColon != KErrNotFound)
			{
		    // port number is all text after colon 
		    return str.Right(firstColon+1);
			}
		}
		
	return TPtrC();
	}




/******************************************************************************
 * class TStr
 ******************************************************************************/


// -----------------------------------------------------------------------------
// TStr::TStr
//
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
TStr::TStr(const TDesC& aStr) : iStr(aStr)
    {
    }


// -------------------------------------------------------------------------------
// TStr::Mid
//
// -------------------------------------------------------------------------------
//
TPtrC TStr::Mid(TInt aPos)
    {
	TInt len = iStr.Length();
	if ( (aPos < 0) || (aPos >= len) )
	    {
		return TPtrC();  // return empty descriptor
	    }

	return iStr.Mid(aPos);
    }


// -------------------------------------------------------------------------------
// TStr::Mid
//
// -------------------------------------------------------------------------------
//
TPtrC TStr::Mid(TInt aPos, TInt aLen)
    {
	TInt len = iStr.Length();
	if ( (aPos < 0) || (aPos >= len) )
	    {
		return TPtrC();
	    }
	if ( (aLen <= 0) || (aPos + aLen > len) )
	    {
		return TPtrC();
	    }

	return iStr.Mid(aPos, aLen);
    }


// -------------------------------------------------------------------------------
// TStr::Right
//
// -------------------------------------------------------------------------------
//
TPtrC TStr::Right(TInt aPos)
    {
	return Mid(aPos);
    }


// -------------------------------------------------------------------------------
// TStr::Left
//
// -------------------------------------------------------------------------------
//
TPtrC TStr::Left(TInt aPos)
    {
	TInt len = iStr.Length();

	if ( (aPos < 0) || (len == 0) )
	    {
		return TPtrC();
	    }

	TInt pos = len;
	if ( (aPos + 1) < len )
	    {
		pos = aPos + 1;
	    }

	return iStr.Left(pos);
    }


// -------------------------------------------------------------------------------
// TStr::SubStr
//
// -------------------------------------------------------------------------------
//
TPtrC TStr::SubStr(TInt aStartPos, TInt aEndPos)
    {
	return Mid(aStartPos, aEndPos - aStartPos + 1);
    }


// -------------------------------------------------------------------------------
// TStr::SubStrEx
//
// -------------------------------------------------------------------------------
//
TPtrC TStr::SubStrEx(TInt aStartPos, TInt aEndPos)
    {
	return Mid(aStartPos + 1, aEndPos - aStartPos - 1);
    }


// -------------------------------------------------------------------------------
// TStr::LastPos
//
// -------------------------------------------------------------------------------
//
TInt TStr::LastPos()
    {
	return iStr.Length() - 1;
    }


// -------------------------------------------------------------------------------
// TStr::Len
//
// -------------------------------------------------------------------------------
//
TInt TStr::Len()
    {
    return iStr.Length();
    }

// -------------------------------------------------------------------------------
// TStr::Compare
//
// -------------------------------------------------------------------------------
//
TBool TStr::Compare(TInt aPos, TChar aChar)
    {
	TInt len = iStr.Length();
	if ( (aPos < 0) || (aPos >= len) )
	    {
		return EFalse;
	    }
	TChar ch = iStr[aPos];
	if (ch == aChar)
	    {
		return ETrue;
	    }
	return EFalse;
    }


// -------------------------------------------------------------------------------
// TStr::Find
//
// -------------------------------------------------------------------------------
//
TInt TStr::Find(TInt aPos, TChar aChar)
    {
	TInt len = iStr.Length();
	if ( (aPos < 0) || (aPos >= len) )
	    {
		return KErrNotFound;
	    }
	    
	TPtrC ptr(iStr.Mid(aPos)); // move to possition aPos
	return ptr.Locate(aChar);
    }


// -------------------------------------------------------------------------------
// TStr::Trim
//
// -------------------------------------------------------------------------------
//
TPtrC TStr::Trim()
    {
	TInt first = FirstNonSpace();
	TInt last = LastNonSpace();

	return Mid(first, last - first + 1);
    }


// -------------------------------------------------------------------------------
// TStr::FirstNonSpace
//
// -------------------------------------------------------------------------------
//
TInt TStr::FirstNonSpace()
    {
	TInt len = Len();
	for (TInt i = 0; i < len; i++)
	    {
		TChar c = iStr[i];
        if (!c.IsSpace())
		    {
		    return i;
		    }
	    }
	return KErrNotFound;
    }


// -------------------------------------------------------------------------------
// TStr::LastNonSpace
//
// -------------------------------------------------------------------------------
//
TInt TStr::LastNonSpace()
    {
	TInt last = LastPos();
	for (TInt i = last; i >= 0; i--)
	    {
		TChar c = iStr[i];
        if (!c.IsSpace())
		    {
		    return i;
		    }
	    }
	return KErrNotFound;
    }



/******************************************************************************
 * class TFlag
 ******************************************************************************/

// -----------------------------------------------------------------------------
// TFlag::TFlag
//
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
TFlag::TFlag(TUint32& aFlag) : iFlag(aFlag)
    {
    }

// -------------------------------------------------------------------------------
// TFlag::SetOn
//
// -------------------------------------------------------------------------------
//
void TFlag::SetOn(const TUint32 aValue)
    {
    iFlag |= aValue;
    }

// -------------------------------------------------------------------------------
// TFlag::SetOff
//
// -------------------------------------------------------------------------------
//
void TFlag::SetOff(const TUint32 aValue)
    {
    iFlag &= ~aValue;
    }

// -------------------------------------------------------------------------------
// TFlag::IsOn
//
// -------------------------------------------------------------------------------
//
TBool TFlag::IsOn(const TUint32 aValue) const
    {
    return iFlag & aValue;
    }



/*******************************************************************************
 * class CAspIdleWait
 *******************************************************************************/


// -----------------------------------------------------------------------------
// CAspIdleWait::WaitL
// 
// -----------------------------------------------------------------------------
//
void CAspIdleWait::WaitL()
	{
    CAspIdleWait* waiter = CAspIdleWait::NewL();
    CleanupStack::PushL(waiter);
    
    CIdle* idle = CIdle::NewL(CActive::EPriorityIdle);
	CleanupStack::PushL(idle);
	
	idle->Start(TCallBack(IdleCallback, waiter));
	if (!waiter->iWait.IsStarted())
		{
		waiter->iWait.Start();
		}

    CleanupStack::PopAndDestroy(idle);
    CleanupStack::PopAndDestroy(waiter);
    }


// -----------------------------------------------------------------------------
// CAspIdleWait::NewLC
//
// -----------------------------------------------------------------------------
//
CAspIdleWait* CAspIdleWait::NewL()
    {
    CAspIdleWait* self = new (ELeave) CAspIdleWait();
    return self;
    }


// -----------------------------------------------------------------------------
// CAspIdleWait::CAspIdleWait
// 
// -----------------------------------------------------------------------------
//
CAspIdleWait::CAspIdleWait() 
    {
    }


// -----------------------------------------------------------------------------
// Destructor
//
// -----------------------------------------------------------------------------
//
CAspIdleWait::~CAspIdleWait()
    {
    }


// -----------------------------------------------------------------------------
// CAspIdleWait::IdleCallback
// 
// -----------------------------------------------------------------------------
//
TInt CAspIdleWait::IdleCallback(TAny* aThisPtr)
    {
    static_cast<CAspIdleWait*>(aThisPtr)->iWait.AsyncStop();
    return KErrNone;
    }



/******************************************************************************
 * class CAspActiveCaller
 ******************************************************************************/


// -----------------------------------------------------------------------------
// CAspActiveCaller::NewL
// 
// -----------------------------------------------------------------------------
//
CAspActiveCaller* CAspActiveCaller::NewL(MAspActiveCallerObserver* aObserver)
    {
    FLOG( _L("CAspActiveCaller::NewL START") );
    
    CAspActiveCaller* self = new (ELeave) CAspActiveCaller(aObserver);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);

    FLOG( _L("CAspActiveCaller::NewL END") );
	return self;
    }


// -----------------------------------------------------------------------------
// Destructor
// 
// -----------------------------------------------------------------------------
//
CAspActiveCaller::~CAspActiveCaller()
    {
    FLOG( _L("CAspActiveCaller::~CAspActiveCaller START") );
    
	Cancel();
	iTimer.Close();
	
    FLOG( _L("CAspActiveCaller::~CAspActiveCaller END") );
    }


// -----------------------------------------------------------------------------
// CAspActiveCaller::CAspActiveCaller
// 
// -----------------------------------------------------------------------------
//
CAspActiveCaller::CAspActiveCaller(MAspActiveCallerObserver* aObserver) : CActive(CActive::EPriorityStandard)
    {
	iObserver = aObserver;
    }


// -----------------------------------------------------------------------------
// CAspActiveCaller::ConstructL
// 
// -----------------------------------------------------------------------------
//
void CAspActiveCaller::ConstructL()
    {
	User::LeaveIfError(iTimer.CreateLocal());
	CActiveScheduler::Add(this);
    }


// -----------------------------------------------------------------------------
// CAspActiveCaller::DoCancel
//
// -----------------------------------------------------------------------------
//
void CAspActiveCaller::DoCancel()
    {
    FLOG( _L("CAspActiveCaller::DoCancel START") );
    
	iTimer.Cancel();
	
    FLOG( _L( "CAspActiveCaller::DoCancel END" ) );
    }


// -----------------------------------------------------------------------------
// CAspActiveCaller::RunL
//
// -----------------------------------------------------------------------------
//
void CAspActiveCaller::RunL()
    {
    FLOG( _L("CAspActiveCaller::RunL START") );

	TRAP_IGNORE(iObserver->HandleActiveCallL(iCallId));

    FLOG( _L("CAspActiveCaller::RunL END") );
    }


// -----------------------------------------------------------------------------
// CAspActiveCaller::Request
//
// This function calls this class RunL.
// -----------------------------------------------------------------------------
//
void CAspActiveCaller::Request()
    {
    FLOG( _L("CAspActiveCaller::Request START") );
    
	Cancel();
	SetActive();
	TRequestStatus* status = &iStatus;
	User::RequestComplete(status, KErrNone);
    
    FLOG( _L("CAspActiveCaller::Request END") );
    }


// -----------------------------------------------------------------------------
// CAspActiveCaller::Start
//
// -----------------------------------------------------------------------------
//
void CAspActiveCaller::Start(TInt aCallId, TInt aMilliseconds)
    {
    FLOG( _L("CAspActiveCaller::Start START") );
    
	if (IsActive())
		{
		return;
		}
	
	iCallId = aCallId;

	if (aMilliseconds <= 0)
		{
		Request();  // no delay - complete right away
		}
	else
		{
	    iTimer.After(iStatus, aMilliseconds*1000);
	    SetActive();
		}
    
    FLOG( _L("CAspActiveCaller::Start END") );
    }




/******************************************************************************
 * class CAspBearerHandler
 ******************************************************************************/


// -----------------------------------------------------------------------------
// CAspBearerHandler::NewLC
//
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CAspBearerHandler* CAspBearerHandler::NewL(const TAspParam& aParam)
    {
    CAspBearerHandler* self = new (ELeave) CAspBearerHandler(aParam);
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
CAspBearerHandler::~CAspBearerHandler()
	{
	iList.Close();
	}


// -----------------------------------------------------------------------------
// CAspBearerHandler::CAspBearerHandler
// -----------------------------------------------------------------------------
//
CAspBearerHandler::CAspBearerHandler(const TAspParam& aParam)
	{
	__ASSERT_ALWAYS(aParam.iSyncSession, TUtil::Panic(KErrGeneral));

	iSyncSession = aParam.iSyncSession;	
	}


// -----------------------------------------------------------------------------
// CAspBearerHandler::ConstructL
//
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CAspBearerHandler::ConstructL(void)
    {
	CreateSupportedBearerListL();
    }


// -----------------------------------------------------------------------------
// CAspBearerHandler::IsSupported
//
// -----------------------------------------------------------------------------
//
TBool CAspBearerHandler::IsSupported(TInt aBearer)
	{
	TInt count = iList.Count();

    for (TInt i=0; i<count; i++)
		{
		TAspBearerItem& item = iList[i];
		if (item.iBearerType == aBearer)
			{
			return ETrue;
			}
		}
	
	return EFalse; // unknown bearer
}


// -----------------------------------------------------------------------------
// CAspBearerHandler::BearerForListIndex
//
// -----------------------------------------------------------------------------
//
TInt CAspBearerHandler::BearerForListIndex(TInt aIndex)
	{
	TInt count = iList.Count();
	if (aIndex < 0 || aIndex >= count)
	    {
		TUtil::Panic(KErrArgument);
	    }

	TAspBearerItem& item = iList[aIndex];
	return item.iBearerType;
	}


// -----------------------------------------------------------------------------
// CAspBearerHandler::ListIndexForBearer
//
// -----------------------------------------------------------------------------
//
TInt CAspBearerHandler::ListIndexForBearer(TInt aBearer)
	{
	TInt count = iList.Count();
	for (TInt i=0; i<count; i++)
		{
		TAspBearerItem& item = iList[i];
		if (item.iBearerType == aBearer)
			{
			return i;
			}
		}

	return KErrNotFound;
	}


// -----------------------------------------------------------------------------
// CAspBearerHandler::CreateSupportedBearerListL
//
// -----------------------------------------------------------------------------
//
void CAspBearerHandler::CreateSupportedBearerListL()
	{
	RSyncMLTransport transport;
	
	RArray<TSmlTransportId> arr(8);
	CleanupClosePushL(arr);
	
	//
	// ListTransportsL returns transports that can be used in current phone.
	// RSyncMLSession uses FeatureManager to construct this list.
	//
	iSyncSession->ListTransportsL(arr);
	TInt count = arr.Count();
	
	for (TInt i=0; i<count; i++)
		{
		TInt id = arr[i];
		transport.OpenL(*iSyncSession, id);
		CleanupClosePushL(transport);
		
		TAspBearerItem item;
		TInt smlBearerId = transport.Identifier();
		item.iBearerType = AspBearerId(smlBearerId);  // convert bearer type
		TUtil::StrCopy(item.iBearerName, transport.DisplayName());
		

#ifndef __SYNCML_DS_OVER_HTTP   // KFeatureIdDsOverHttp
        
   		if (item.iBearerType == EAspBearerInternet)
			{
			CleanupStack::PopAndDestroy(&transport);
			continue;
			}
#endif

   		if (item.iBearerType != EAspBearerInternet && 
   		    item.iBearerType != EAspBearerBlueTooth)
			{
			CleanupStack::PopAndDestroy(&transport);
			continue; // only internet and bluetooth are supported
			}

		if (item.iBearerType != KErrNotFound)
			{
			User::LeaveIfError(iList.Append(item));
			}
		
		CleanupStack::PopAndDestroy(&transport);
		}
		
	CleanupStack::PopAndDestroy(&arr);
	}


// -----------------------------------------------------------------------------
// CAspBearerHandler::AspBearerId
//
// -----------------------------------------------------------------------------
//
TInt CAspBearerHandler::AspBearerId(const TInt aId)
	{
	if (aId == KUidNSmlMediumTypeInternet.iUid)
		{
		return EAspBearerInternet;
		}
	else if (aId == KUidNSmlMediumTypeBluetooth.iUid)
		{
		return EAspBearerBlueTooth;
		}
	else if (aId == KUidNSmlMediumTypeUSB.iUid)
		{
		return EAspBearerUsb;
		}
	else if (aId == KUidNSmlMediumTypeIrDA.iUid)
		{
		return EAspBearerIrda;
		}
	//else if (aId == KUidNSmlMediumTypeWSP.iUid)
	//	{
	//	return EAspBearerWsp;
	//	}
	//else if (aId == KUidNSmlMediumTypeNSmlUSB.iUid)
	//	{
	//	return EAspBearerSmlUsb;
	//	}

	else
		{
		return KErrNotFound;
		}
    }


// -----------------------------------------------------------------------------
// CAspBearerHandler::SmlBearerId
//
// -----------------------------------------------------------------------------
//
TInt CAspBearerHandler::SmlBearerId(const TInt aId)
	{
	if (aId == EAspBearerInternet)
		{
		return KUidNSmlMediumTypeInternet.iUid;
		}
	else if (aId == EAspBearerBlueTooth)
		{
		return KUidNSmlMediumTypeBluetooth.iUid;
    	}
	else if (aId == EAspBearerUsb)
		{
		return KUidNSmlMediumTypeUSB.iUid;		
		}
	else if (aId == EAspBearerIrda)
		{
		return KUidNSmlMediumTypeIrDA.iUid;		
		}
	//else if (aId == EAspBearerWsp)
	//	{
	//	return KUidNSmlMediumTypeWSP.iUid;		
	//	}
	//else if (aId == EAspBearerSmlUsb)
	//	{
	//	return KUidNSmlMediumTypeNSmlUSB.iUid;		
	//	}
	else
		{
		return KErrNotFound;
		}
	}


// -----------------------------------------------------------------------------
// CAspBearerHandler::DefaultBearer
//
// -----------------------------------------------------------------------------
//
TInt CAspBearerHandler::DefaultBearer()
	{
	if (IsSupported(EAspBearerInternet))
		{
		return EAspBearerInternet;
		}
	if (IsSupported(EAspBearerBlueTooth))
		{
		return EAspBearerBlueTooth;
		}
    if (IsSupported(EAspBearerUsb))
		{
		return EAspBearerUsb;
		}
	if (IsSupported(EAspBearerIrda))
		{
		return EAspBearerIrda;
		}

	return KErrNotFound; // something is wrong
	}


// -----------------------------------------------------------------------------
// CAspBearerHandler::GetBearerName
//
// -----------------------------------------------------------------------------
//
void CAspBearerHandler::GetBearerName(TDes& aText, TInt aBearerType)
	{
    aText = KNullDesC;
	TInt count = iList.Count();

	for (TInt i=0; i<count; i++)
		{
		TAspBearerItem& item = iList[i];
		if (item.iBearerType == aBearerType)
			{
			TUtil::StrCopy(aText, item.iBearerName);
			break;
			}
		}
	}


// -----------------------------------------------------------------------------
// CAspBearerHandler::BuildBearerListLC
//
// -----------------------------------------------------------------------------
//
CDesCArray* CAspBearerHandler::BuildBearerListLC()
	{	
	CDesCArray* arr = new (ELeave) CDesCArrayFlat(1);
	CleanupStack::PushL(arr);

	TInt count = iList.Count();
	for (TInt i=0; i<count; i++)
		{
		TAspBearerItem& item = iList[i];
		arr->AppendL(item.iBearerName);
		}

	return arr;
	}


// -----------------------------------------------------------------------------
// CAspBearerHandler::SupportedBearerCount
//
// -----------------------------------------------------------------------------
//
TInt CAspBearerHandler::SupportedBearerCount()
	{
	return iList.Count();
	}





/******************************************************************************
 * class CAspAccessPointHandler
 ******************************************************************************/



// -----------------------------------------------------------------------------
// CAspAccessPointHandler::NewLC
//
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CAspAccessPointHandler* CAspAccessPointHandler::NewL(const TAspParam& aParam)
    {
    CAspAccessPointHandler* self = new(ELeave) CAspAccessPointHandler(aParam);
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
CAspAccessPointHandler::~CAspAccessPointHandler()
	{
	delete iCommDb;
	delete iApUtil;
	}


// -----------------------------------------------------------------------------
// CAspAccessPointHandler::CAspAccessPointHandler
//
// -----------------------------------------------------------------------------
//
CAspAccessPointHandler::CAspAccessPointHandler(const TAspParam& /*aParam*/)
	{
	}


// -----------------------------------------------------------------------------
// CAspAccessPointHandler::ConstructL
//
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CAspAccessPointHandler::ConstructL(void)
    {
	iCommDb = CCommsDatabase::NewL(EDatabaseTypeIAP);
	
	iApUtil = CApUtils::NewLC(*iCommDb);
	CleanupStack::Pop(iApUtil);
    }


// -----------------------------------------------------------------------------
// CAspAccessPointHandler::ShowApSelectDialogL
//
// -----------------------------------------------------------------------------
//
TInt CAspAccessPointHandler::ShowApSelectDialogL(TAspAccessPointItem& aItem)
	{
	aItem.iUid = KErrNotFound;
	aItem.iName = KNullDesC;

	TUint32 selectedAp = 0;
	TUint32 currentAp = 0;
	if (aItem.iUid2 != KAskAlways && aItem.iUid2 != KDefaultConnection)
		{
		//currentAp = aItem.iUid2;
		
		// convert internet ap to Wap ap (CApSettingsHandler requires this)
    	TRAP_IGNORE(currentAp = iApUtil->WapIdFromIapIdL(aItem.iUid2));
		}
	
	
	CApSettingsHandler* handler = NULL;
	handler = CApSettingsHandler::NewLC( ETrue,
	                                     EApSettingsSelListIsPopUp,
	                                     EApSettingsSelMenuSelectNormal,
	                                     KEApIspTypeAll,
	                                     BearerFlags(),
	    	                             KEApSortNameAscending);

	TInt ret = handler->RunSettingsL(currentAp, selectedAp);
	CleanupStack::PopAndDestroy(handler);
	
	if (ret & KApUiEventSelected)
		{
		TAspAccessPointItem item;
		item.iUid = selectedAp;

	    GetApInfo(item);
	    
	    // convert Wap ap to internet ap
	    aItem.iUid = iApUtil->IapIdFromWapIdL(selectedAp);
	    
	    //aItem.iUid = selectedAp;
	    
	    aItem.iName = item.iName;
	    
		return EAspDialogSelect;
		}
    else if (ret & KApUiEventExitRequested)
    	{
    	return EAspDialogExit;
    	}
    else
    	{
    	return EAspDialogCancel;
    	}
	}


// -----------------------------------------------------------------------------
// CAspAccessPointHandler::GetApInfoL
//
// -----------------------------------------------------------------------------
//
TInt CAspAccessPointHandler::GetApInfoL(TAspAccessPointItem& aItem)
	{
	if (aItem.iUid == KAskAlways || aItem.iUid == KErrNotFound || aItem.iUid == KDefaultConnection)
		{
		return KErrNotFound;
		}

	CApSelect* selector = CApSelect::NewLC(*iCommDb,
	                             KEApIspTypeAll,
		                         BearerFlags(),
		                         KEApSortUidAscending);
	
	TInt ret = KErrNone;
	TBool found = EFalse;
	
	// find access point with id aId
	if (selector->MoveToFirst())
		{
		if (aItem.iUid == (TInt)selector->Uid())
			{
			found = ETrue;		
			}
		while (!found && selector->MoveNext())
			{
			if (aItem.iUid == (TInt)selector->Uid())
				{
				found = ETrue;
				}
			}
		}

	if (found) 
		{
		TUtil::StrCopy(aItem.iName, selector->Name());
		}
	else
		{
		aItem.iName = KNullDesC;
		ret = KErrNotFound;
		}
		
    CleanupStack::PopAndDestroy(selector);
	return ret;
	}
	

// -----------------------------------------------------------------------------
// CAspAccessPointHandler::GetApInfo
//
// -----------------------------------------------------------------------------
//
TInt CAspAccessPointHandler::GetApInfo(TAspAccessPointItem& aItem)
	{
	aItem.iName = KNullDesC;
	
	TInt ret = KErrNone;
	TRAPD(err, ret = GetApInfoL(aItem));
	
	if (err != KErrNone)
		{
		return KErrNotFound;
		}
	return ret;
	}


// -----------------------------------------------------------------------------
// CAspAccessPointHandler::GetInternetApInfo
//
// -----------------------------------------------------------------------------
//
TInt CAspAccessPointHandler::GetInternetApInfo(TAspAccessPointItem& aItem)
	{
	if (aItem.iUid == KAskAlways || aItem.iUid == KErrNotFound || aItem.iUid == KDefaultConnection)
		{
		return KErrNotFound;
		}

	TAspAccessPointItem item;
	
	// convert internet ap to Wap ap
	TRAPD(err, item.iUid = iApUtil->WapIdFromIapIdL(aItem.iUid));
	if (err != KErrNone)
		{
		return KErrNotFound;
		}
	
	TInt ret = GetApInfo(item);
	
	TUtil::StrCopy(aItem.iName, item.iName);
	return ret;
	}


// -----------------------------------------------------------------------------
// CAspAccessPointHandler::BearerFlags
//
// -----------------------------------------------------------------------------
//
TInt CAspAccessPointHandler::BearerFlags()
	{
	TInt flags;
	if (FeatureManager::FeatureSupported(KFeatureIdAppCsdSupport))
		{
		//flags = EApBearerTypeGPRS | EApBearerTypeCDMA | EApBearerTypeWLAN | EApBearerTypeHSCSD | EApBearerTypeCSD;
		flags = EApBearerTypeAllBearers;
		}
	else
		{
		flags = EApBearerTypeGPRS | EApBearerTypeCDMA | EApBearerTypeWLAN;
		}
		
	return flags;
	}




/******************************************************************************
 * class TAspSyncError
 ******************************************************************************/




// -----------------------------------------------------------------------------
// TAspSyncError::GetSmlErrorText
//
// -----------------------------------------------------------------------------
//
HBufC* TAspSyncError::GetSmlErrorText(TInt aErrorCode, TInt aTransport)
	{
	HBufC* hBuf = NULL;
	
	TRAPD(err, hBuf = GetSmlErrorTextL(aErrorCode, aTransport));
	
	if (err == KErrNone)
		{
		return hBuf;
		}
	else
		{
		return NULL;
		}
	}


// -----------------------------------------------------------------------------
// TAspSyncError::GetSmlErrorTextL
//
// -----------------------------------------------------------------------------
//
HBufC* TAspSyncError::GetSmlErrorTextL(TInt aErrorCode, TInt /*aTransport*/)
	{
    TInt id = R_ASP_LOG_ERR_SYSTEMERROR;  // "System Error"
   
	switch (aErrorCode)
		{
		case SyncMLError::KErrTransportAuthenticationFailure:
			{
			id = R_ASP_LOG_ERR_HTTPAUTH;  // "Http authentication failed"
		    break;
			}
	
		case SyncMLError::KErrAuthenticationFailure:
			{
			id = R_ASP_LOG_ERR_CLIENTAUTH;  // "Invalid user name or password"
		    break;
			}

		case KErrServerBusy:
		    id = R_ASP_LOG_ERR_SERVERBUSY;  // "Synchronisation server was busy"
		    break;
		    
		case SyncMLError::KErrServerDatabaseNotFound:
		    id = R_ASP_LOG_ERR_REMOTEDATABASE;  // "Invalid remote database"
		    break;

		case SyncMLError::KErrClientDatabaseNotFound:
		    id = R_ASP_LOG_ERR_LOCALDATABASE;  // "Unable to open database"
		    break;

		case SyncMLError::KErrConnectionError:
		    id = R_ASP_LOG_ERR_SERVERNOTRESPONDING; // "Server is not responding"
		    break;
		    
		case SyncMLError::KErrTransportSvrNoUri:
		    id = R_ASP_LOG_ERR_URIINVALID; // "Invalid host address"
		    break;
		
		case SyncMLError::KErrPhoneOnOfflineMode:
		    id = R_ASP_LOG_ERR_OFFLINE; // "Operation not possible in Off-line mode"
		    break;
		    
		case SyncMLError::KErrTransportRequest:
		case SyncMLError::KErrTransportHeader:
		case SyncMLError::KErrTransportSvrError:
		case SyncMLError::KErrTransportDisconnected:
		case SyncMLError::KErrCannotCommunicateWithServer:
		    id = R_ASP_LOG_ERR_COMM;  // "Error in communication"
		    break;
		    
		case SyncMLError::KErrVerDTDMissing:
		case SyncMLError::KErrProtoMissing:
		case SyncMLError::KErrVersionDTDNotSupported:
		case SyncMLError::KErrProtoNotSupported:
		    id = R_ASP_LOG_ERR_VERSIONNOTSUPPORTED;  // "Unsupported SyncML software version in server"
		    break;
       
		case SyncMLError::KErrSessionIdNotValid:
		case SyncMLError::KErrMappingInfoMissing:
		case SyncMLError::KErrMappingInfoNotAcked:
		case SyncMLError::KErrStatusCommandForCommandNotSent:
		case SyncMLError::KErrStatusCommandNotTheSame:
		case SyncMLError::KErrMissingStatusCommands:
		case SyncMLError::KErrFailedToAddRecord:
		case SyncMLError::KErrFailedToReplaceRecord:
		case SyncMLError::KErrFailedToDeleteRecord:
		case SyncMLError::KErrXmlParsingError:
		case SyncMLError::KErrReferenceToolkitError:
		case SyncMLError::KErrObjectTooLargeToSendToServer:
		case SyncMLError::KErrFailedToGetRecord:
		case SyncMLError::KErrUserInteractionFailed:
		case SyncMLError::KErrStatusSizeMismatch:
		case SyncMLError::KErrFailedToCloseDatabase:
		case SyncMLError::KErrDatabaseAdapterIndexError:
	   	    id = R_ASP_LOG_ERR_SERVERERROR;  // "Error in sync server"
		    break;
	        
        default:
            id = R_ASP_LOG_ERR_SYSTEMERROR;  // "System Error"
		    break;
		}
		
	HBufC* hBuf = CAspResHandler::ReadLC(id);
	
#ifdef _DEBUG
	TBuf<KBufSize> buf2(hBuf->Des());
	TBuf<KBufSize> buf1;
	CleanupStack::PopAndDestroy(hBuf);
		
	hBuf = HBufC::NewLC(256);
	TPtr ptr = hBuf->Des();
		
	GetSyncMLErrorText(buf1, aErrorCode);
	ptr.Format(_L("%S (%S)"), &buf2, &buf1);
#endif		
		
	CleanupStack::Pop(hBuf);
	return hBuf;
	}
	

// -----------------------------------------------------------------------------
// TAspSyncError::GetSmlErrorValue
// 
// -----------------------------------------------------------------------------
TInt TAspSyncError::GetSmlErrorValue(TInt aErrorCode)
	{
    TInt id = R_ASP_LOG_ERR_SYSTEMERROR;  // "System Error"
   
	switch (aErrorCode)
		{
		case SyncMLError::KErrTransportAuthenticationFailure:
			{
			id = R_ASP_LOG_ERR_HTTPAUTH;  // "Http authentication failed"
		    break;
			}
	
		case SyncMLError::KErrAuthenticationFailure:
			{
			id = R_ASP_LOG_ERR_CLIENTAUTH;  // "Invalid user name or password"
		    break;
			}

		case KErrServerBusy:
		    id = R_ASP_LOG_ERR_SERVERBUSY;  // "Synchronisation server was busy"
		    break;
		    
		case SyncMLError::KErrServerDatabaseNotFound:
		    id = R_ASP_LOG_ERR_REMOTEDATABASE;  // "Invalid remote database"
		    break;

		case SyncMLError::KErrClientDatabaseNotFound:
		    id = R_ASP_LOG_ERR_LOCALDATABASE;  // "Unable to open database"
		    break;

		case SyncMLError::KErrConnectionError:
		    id = R_ASP_LOG_ERR_SERVERNOTRESPONDING; // "Server is not responding"
		    break;
		    
		case SyncMLError::KErrTransportSvrNoUri:
		    id = R_ASP_LOG_ERR_URIINVALID; // "Invalid host address"
		    break;
		
		case SyncMLError::KErrPhoneOnOfflineMode:
		    id = R_ASP_LOG_ERR_OFFLINE; // "Operation not possible in Off-line mode"
		    break;
		    
		case SyncMLError::KErrTransportRequest:
		case SyncMLError::KErrTransportHeader:
		case SyncMLError::KErrTransportSvrError:
		case SyncMLError::KErrTransportDisconnected:
		case SyncMLError::KErrCannotCommunicateWithServer:
		    id = R_ASP_LOG_ERR_COMM;  // "Error in communication"
		    break;
		    
		case SyncMLError::KErrVerDTDMissing:
		case SyncMLError::KErrProtoMissing:
		case SyncMLError::KErrVersionDTDNotSupported:
		case SyncMLError::KErrProtoNotSupported:
		    id = R_ASP_LOG_ERR_VERSIONNOTSUPPORTED;  // "Unsupported SyncML software version in server"
		    break;
       
		case SyncMLError::KErrSessionIdNotValid:
		case SyncMLError::KErrMappingInfoMissing:
		case SyncMLError::KErrMappingInfoNotAcked:
		case SyncMLError::KErrStatusCommandForCommandNotSent:
		case SyncMLError::KErrStatusCommandNotTheSame:
		case SyncMLError::KErrMissingStatusCommands:
		case SyncMLError::KErrFailedToAddRecord:
		case SyncMLError::KErrFailedToReplaceRecord:
		case SyncMLError::KErrFailedToDeleteRecord:
		case SyncMLError::KErrXmlParsingError:
		case SyncMLError::KErrReferenceToolkitError:
		case SyncMLError::KErrObjectTooLargeToSendToServer:
		case SyncMLError::KErrFailedToGetRecord:
		case SyncMLError::KErrUserInteractionFailed:
		case SyncMLError::KErrStatusSizeMismatch:
		case SyncMLError::KErrFailedToCloseDatabase:
		case SyncMLError::KErrDatabaseAdapterIndexError:
	   	    id = R_ASP_LOG_ERR_SERVERERROR;  // "Error in sync server"
		    break;
	        
        default:
            id = R_ASP_LOG_ERR_SYSTEMERROR;  // "System Error"
		    break;
		}
		
		return id ;
	}
	

#ifdef _DEBUG

// -----------------------------------------------------------------------------
// TAspSyncError::GetSyncMLErrorText
//
// -----------------------------------------------------------------------------
//
void TAspSyncError::GetSyncMLErrorText(TDes& aText, TInt aError)
	{
    aText = KNullDesC();
  
   
	switch (aError)
		{
		case SyncMLError::KErrTransportRequest:
		    aText = _L("KErrTransportRequest");
		    break;
		case SyncMLError::KErrTransportHeader:
		    aText = _L("KErrTransportHeader");
		    break;
		case SyncMLError::KErrTransportSvrNoUri:
		    aText = _L("KErrTransportSvrNoUri");
		    break;
		case SyncMLError::KErrTransportSvrError:
		    aText = _L("KErrTransportSvrError");
		    break;
		case SyncMLError::KErrTransportAuthenticationFailure:
		    aText = _L("KErrTransportAuthenticationFailure");
		    break;
		case SyncMLError::KErrTransportDisconnected:
		    aText = _L("KErrTransportDisconnected");
		    break;
		case SyncMLError::KErrConnectionError:
		    aText = _L("KErrConnectionError");
		    break;
		case SyncMLError::KErrCannotCommunicateWithServer:
		    aText = _L("KErrCannotCommunicateWithServer");
		    break;
		case SyncMLError::KErrDatabaseAdapterIndexError:
		    aText = _L("KErrDatabaseAdapterIndexError");
		    break;
		case SyncMLError::KErrParseCallbackError:
		    aText = _L("KErrParseCallbackError");
		    break;
		case SyncMLError::KErrCommandInvalid:
		    aText = _L("KErrCommandInvalid");
		    break;
		case SyncMLError::KErrSessionIdNotValid:
		    aText = _L("KErrSessionIdNotValid");
		    break;
		case SyncMLError::KErrVerDTDMissing:
		    aText = _L("KErrVerDTDMissing");
		    break;
		case SyncMLError::KErrProtoMissing:
		    aText = _L("KErrProtoMissing");
		    break;
		case SyncMLError::KErrSyncHdrMissing:
		    aText = _L("KErrSyncHdrMissing");
		    break;
		case SyncMLError::KErrTargetLocMissing:
		    aText = _L("KErrTargetLocMissing");
		    break;
		case SyncMLError::KErrSourceLocMissing:
		    aText = _L("KErrSourceLocMissing");
		    break;
		case SyncMLError::KErrTargetLocNameMissing:
		    aText = _L("KErrTargetLocNameMissing");
		    break;
		case SyncMLError::KErrSourceLocNameMissing:
		    aText = _L("KErrSourceLocNameMissing");
		    break;
		case SyncMLError::KErrMappingInfoMissing:
		    aText = _L("KErrMappingInfoMissing");
		    break;
		case SyncMLError::KErrMappingInfoNotAcked:
		    aText = _L("KErrMappingInfoNotAcked");
		    break;
		case SyncMLError::KErrVersionDTDNotSupported:
		    aText = _L("KErrVersionDTDNotSupported");
		    break;
		case SyncMLError::KErrProtoNotSupported:
		    aText = _L("KErrProtoNotSupported");
		    break;
		case SyncMLError::KErrStatusCommandForCommandNotSent:
		    aText = _L("KErrStatusCommandForCommandNotSent");
		    break;
		case SyncMLError::KErrStatusCommandNotTheSame:
		    aText = _L("KErrStatusCommandNotTheSame");
		    break;
		case SyncMLError::KErrMissingStatusCommands:
		    aText = _L("KErrMissingStatusCommands");
		    break;
		case SyncMLError::KErrFailedToAddRecord:
		    aText = _L("KErrFailedToAddRecord");
		    break;
		case SyncMLError::KErrFailedToReplaceRecord:
		    aText = _L("KErrFailedToReplaceRecord");
		    break;
		case SyncMLError::KErrFailedToDeleteRecord:
		    aText = _L("KErrFailedToDeleteRecord");
		    break;
		case SyncMLError::KErrXmlParsingError:
		    aText = _L("KErrXmlParsingError");
		    break;
		case SyncMLError::KErrReferenceToolkitError:
		    aText = _L("KErrReferenceToolkitError");
		    break;
		case SyncMLError::KErrClientDatabaseNotFound:
		    aText = _L("KErrClientDatabaseNotFound");
		    break;
		case SyncMLError::KErrServerDatabaseNotFound:
		    aText = _L("KErrServerDatabaseNotFound");
		    break;
		case SyncMLError::KErrDevInfFileNotFound:
		    aText = _L("KErrDevInfFileNotFound");
		    break;
		case SyncMLError::KErrObjectTooLargeToSendToServer:
		    aText = _L("KErrObjectTooLargeToSendToServer");
		    break;
		case SyncMLError::KErrSlowSync:
		    aText = _L("KErrSlowSync");
		    break;
		case SyncMLError::KErrFailedToGetRecord:
		    aText = _L("KErrFailedToGetRecord");
		    break;
		case SyncMLError::KErrUserInteractionFailed:
		    aText = _L("KErrUserInteractionFailed");
		    break;
		case SyncMLError::KErrStatusSizeMismatch:
		    aText = _L("KErrStatusSizeMismatch");
		    break;
		case SyncMLError::KErrFailedToCloseDatabase:
		    aText = _L("KErrFailedToCloseDatabase");
		    break;
	
    	case SyncMLError::KErrPhoneOnOfflineMode:
		    aText = _L("KErrPhoneOnOfflineMode");
		    break;
		
	    case SyncMLError::KErrAuthenticationFailure:
		    aText = _L("KErrAuthenticationFailure");
		    break;
		
		case SyncMLError::KErrGeneral:
		    aText = _L("KErrGeneral");
		    break;
		
		case SyncMLError::KErrMessageTooShort:
		    aText = _L("KErrMessageTooShort");
		    break;
		case SyncMLError::KErrMessageIsCorrupt:
		    aText = _L("KErrMessageIsCorrupt");
		    break;
		case SyncMLError::KErrBadTransportId:
		    aText = _L("KErrBadTransportId");
		    break;
		case SyncMLError::KErrNoNotificationBody:
		    aText = _L("KErrNoNotificationBody");
		    break;
		case SyncMLError::KErrWrongVersion:
		    aText = _L("KErrWrongVersion");
		    break;
		case SyncMLError::KErrUnsupportedMimeType:
		    aText = _L("KErrUnsupportedMimeType");
		    break;
		case SyncMLError::KErrNoSuchTask:
		    aText = _L("KErrNoSuchTask");
		    break;
		case SyncMLError::KErrUnknownSyncType:
		    aText = _L("KErrUnknownSyncType");
		    break;
		case SyncMLError::KErrUnknownProfile:
		    aText = _L("KErrUnknownProfile");
		    break;
		case SyncMLError::KErrInvalidProfile:
		    aText = _L("KErrInvalidProfile");
		    break;
		case SyncMLError::KErrInvalidCredentials:
		    aText = _L("KErrInvalidCredentials");
		    break;
		case SyncMLError::KErrNoCredentials:
		    aText = _L("KErrNoCredentials");
		    break;
		case SyncMLError::KErrIntegrityFailureHMAC:
		    aText = _L("KErrIntegrityFailureHMAC");
		    break;
		case SyncMLError::KErrMustUsageUriNotComplete:
		    aText = _L("KErrMustUsageUriNotComplete");
		    break;
		case SyncMLError::KErrDMAccUriIsCorrupt:
		    aText = _L("KErrDMAccUriIsCorrupt");
		    break;
		case SyncMLError::KErrDMConUriIsCorrupt:
		    aText = _L("KErrDMConUriIsCorrupt");
		    break;
		case SyncMLError::KErrParmNameNotInProvDoc:
		    aText = _L("KErrParmNameNotInProvDoc");
		    break;
		case SyncMLError::KErrInvalidAppIdValue:
		    aText = _L("KErrInvalidAppIdValue");
		    break;
		case SyncMLError::KErrInvalidToNapIdValue:
		    aText = _L("KErrInvalidToNapIdValue");
		    break;
		case SyncMLError::KErrInvalidProviderIdValue:
		    aText = _L("KErrInvalidProviderIdValue");
		    break;
		case SyncMLError::KErrEmptyAauthLevelValue:
		    aText = _L("KErrEmptyAauthLevelValue");
		    break;
		case SyncMLError::KErrAppSrvAAuthLevelNotFound:
		    aText = _L("KErrAppSrvAAuthLevelNotFound");
		    break;
		case SyncMLError::KErrClientAAuthLevelNotFound:
		    aText = _L("KErrClientAAuthLevelNotFound");
		    break;
		case SyncMLError::KErrInvalidAppAuth:
		    aText = _L("KErrInvalidAppAuth");
		    break;
		case SyncMLError::KErrNoApplicationCharac:
		    aText = _L("KErrNoApplicationCharac");
		    break;
		case SyncMLError::KErrNoAppAuthCharac:
		    aText = _L("KErrNoAppAuthCharac");
		    break;
		case SyncMLError::KErrMissingRequiredParmInSvr:
		    aText = _L("KErrMissingRequiredParmInSvr");
		    break;
		case SyncMLError::KErrMissingRequiredParmInClient:
		    aText = _L("KErrMissingRequiredParmInClient");
		    break;
		case SyncMLError::KErrAppAddrCharacNotFound:
		    aText = _L("KErrAppAddrCharacNotFound");
		    break;
		case SyncMLError::KErrUnsupportedVersion:
		    aText = _L("KErrUnsupportedVersion");
		    break;
		case SyncMLError::KErrWrongProvPublicDocId:
		    aText = _L("KErrWrongProvPublicDocId");
		    break;
		case SyncMLError::KErrProfileNotFound:
		    aText = _L("KErrProfileNotFound");
		    break;
		case SyncMLError::KErrConnectionNotFound:
		    aText = _L("KErrConnectionNotFound");
		    break;
		case SyncMLError::KErrTaskNotFound:
		    aText = _L("KErrTaskNotFound");
		    break;
		case SyncMLError::KErrTransportNotFound:
		    aText = _L("KErrTransportNotFound");
		    break;
		case SyncMLError::KErrDataProviderNotFound:
		    aText = _L("KErrDataProviderNotFound");
		    break;
		case SyncMLError::KErrJobNotFound:
		    aText = _L("KErrJobNotFound");
		    break;
		case SyncMLError::KErrObjectPendingDelete:
		    aText = _L("KErrObjectPendingDelete");
		    break;
		case SyncMLError::KErrObjectInUse:
		    aText = _L("KErrObjectInUse");
		    break;
		case SyncMLError::KErrProfileAccessDenied:
		    aText = _L("KErrProfileAccessDenied");
		    break;
		case SyncMLError::KErrSuspended:
		    aText = _L("KErrSuspended");
		    break;
		case SyncMLError::KErrDataDepotStreamNotFound:
		    aText = _L("KErrDataDepotStreamNotFound");
		    break;
		
		case SyncMLError::KErrServerIdNotMatching:
		    aText = _L("KErrServerIdNotMatching");
		    break;
	        
        default:
            aText.Format(_L("%d"), aError);
		    break;
		}
	}

#endif







// End of file
