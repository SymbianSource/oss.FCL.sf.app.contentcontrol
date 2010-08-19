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
#include "AspResHandler.h"
#include "AspUtil.h"
#include "AspState.h"

#include <akntabgrp.h>      // for AknTextUtils
#include <StringLoader.h>
#include <data_caging_path_literals.hrh>  // for resource and bitmap directories




// ============================ MEMBER FUNCTIONS ===============================



// -----------------------------------------------------------------------------
// CAspResHandler::NewL
//
// -----------------------------------------------------------------------------
//
CAspResHandler* CAspResHandler::NewL()
    {
    CAspResHandler* self = new (ELeave) CAspResHandler();
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
CAspResHandler::~CAspResHandler()
    {
	if (iResId)
		{
        iEikEnv->DeleteResourceFile(iResId);
		}

	delete iProgressTextFormat;
    }


// -----------------------------------------------------------------------------
// CAspResHandler::ConstructL
//
// -----------------------------------------------------------------------------
//
void CAspResHandler::ConstructL(void)
    {
	iEikEnv = CEikonEnv::Static();
	
	// add resource file
	TFileName buf;
	GetResFileName(buf);
	iResId = TUtil::AddResFileL(buf);

	iProgressTextFormatId = KErrNotFound;
    }


// -----------------------------------------------------------------------------
// CAspResHandler::CAspResHandler
//
// -----------------------------------------------------------------------------
//
CAspResHandler::CAspResHandler()
	{
	}


// -----------------------------------------------------------------------------
// CAspResHandler::ReadL
//
// -----------------------------------------------------------------------------
//
void CAspResHandler::ReadL(TDes& aText, TInt aResourceId)
	{
	StringLoader::Load(aText, aResourceId);
	}


// -----------------------------------------------------------------------------
// CAspResHandler::ReadL
//
// -----------------------------------------------------------------------------
//
void CAspResHandler::ReadL(TDes& aText, TInt aResourceId, const TDesC& aSubString)
	{
    HBufC* hBuf = StringLoader::LoadLC(aResourceId, aSubString);
    TUtil::StrCopy(aText, *hBuf);
    CleanupStack::PopAndDestroy(hBuf);
	}


// -----------------------------------------------------------------------------
// CAspResHandler::ReadLC
//
// -----------------------------------------------------------------------------
//
HBufC* CAspResHandler::ReadLC(TInt aResourceId)
	{
    return StringLoader::LoadLC(aResourceId);
	}


// -----------------------------------------------------------------------------
// CAspResHandler::ReadL
//
// -----------------------------------------------------------------------------
//
HBufC* CAspResHandler::ReadL(TInt aResourceId)
	{
    return StringLoader::LoadL(aResourceId);
	}


// -----------------------------------------------------------------------------
// CAspResHandler::ReadDesArrayLC
//
// -----------------------------------------------------------------------------
//
CDesCArrayFlat* CAspResHandler::ReadDesArrayLC(TInt aResourceId)
	{
	CDesCArrayFlat* arr = iEikEnv->ReadDesCArrayResourceL(aResourceId);
	CleanupStack::PushL(arr);
    return arr;
	}


// -----------------------------------------------------------------------------
// CAspResHandler::ReadDesArrayL
//
// -----------------------------------------------------------------------------
//
CDesCArrayFlat* CAspResHandler::ReadDesArrayL(TInt aResourceId)
	{
	CDesCArrayFlat* arr = iEikEnv->ReadDesCArrayResourceL(aResourceId);
    return arr;
	}


// -----------------------------------------------------------------------------
// CAspResHandler::ReadDesArrayStaticLC
//
// -----------------------------------------------------------------------------
//
CDesCArrayFlat* CAspResHandler::ReadDesArrayStaticLC(TInt aResourceId)
	{
	CDesCArrayFlat* arr = CEikonEnv::Static()->ReadDesCArrayResourceL(aResourceId);
	CleanupStack::PushL(arr);
    return arr;
	}


// -----------------------------------------------------------------------------
// CAspResHandler::ReadLC
//
// -----------------------------------------------------------------------------
//
HBufC* CAspResHandler::ReadLC(TInt aResourceId, const TDesC& aSubString)
	{
	HBufC* hBuf = StringLoader::LoadL(aResourceId, aSubString);
	CleanupStack::PushL(hBuf);
	return hBuf;
	}


// -----------------------------------------------------------------------------
// CAspResHandler::ReadLC
//
// -----------------------------------------------------------------------------
//
HBufC* CAspResHandler::ReadLC(TInt aResourceId, TInt aNumber)
	{
	HBufC* hBuf = StringLoader::LoadL(aResourceId, aNumber);
	CleanupStack::PushL(hBuf);
	return hBuf;
	}


// -----------------------------------------------------------------------------
// CAspResHandler::GetResFileName
//
// -----------------------------------------------------------------------------
//
void CAspResHandler::GetResFileName(TDes& aText)
	{
	TParse parse;
	parse.Set(KResFileName, &KDC_RESOURCE_FILES_DIR, NULL);
	TUtil::StrCopy(aText, parse.FullName());
	}


// -----------------------------------------------------------------------------
// CAspResHandler::GetBitmapFileName
//
// -----------------------------------------------------------------------------
//
void CAspResHandler::GetBitmapFileName(TDes& aText)
	{
	TParse parse;
	parse.Set(KBitmapFileName, &KDC_APP_BITMAP_DIR, NULL);
	TUtil::StrCopy(aText, parse.FullName());
	}

/* Implementation of CMCC PIM v3 begins*/
// -----------------------------------------------------------------------------
// CAspResHandler::ReadProgressTextLC
//
// -----------------------------------------------------------------------------
//
HBufC* CAspResHandler::ReadProgressTextLC(const TDesC& aContent, TInt aCurrent, TInt aFinal, TInt aPhase)
    {
    HBufC* hBuf = NULL;
    
    TBool isCalendar = EFalse;
    HBufC* content = ReadLC(R_ASP_CONTENT_NAME_CALENDAR);
    if ( !content->Compare(aContent) )
        {
        isCalendar = ETrue;
        }
    CleanupStack::PopAndDestroy(content); //content
    
    if (aPhase == CAspState::EPhaseSending)
        {
        if (isCalendar)
            {
            hBuf = ReadProgressTextLC(R_QTN_SML_SYNC_SENDING_XOFY_CAL, aCurrent, aFinal, aContent);
            }
        else
            {
            hBuf = ReadProgressTextLC(R_QTN_SML_SYNC_SENDING_XOFY, aCurrent, aFinal, aContent);
            }
        }
    else if (aPhase == CAspState::EPhaseReceiving)
        {
        if (isCalendar)
            {
            hBuf = ReadProgressTextLC(R_QTN_SML_SYNC_RECEIVING_XOFY_CAL, aCurrent, aFinal, aContent);
            }
        else
            {
            hBuf = ReadProgressTextLC(R_QTN_SML_SYNC_RECEIVING_XOFY, aCurrent, aFinal, aContent);
            }
        }
    else
        {
        hBuf = HBufC::NewLC(0);  // empty string
        }

    return hBuf;
    }
/* Implementation of CMCC PIM v3 ends*/

// -----------------------------------------------------------------------------
// CAspResHandler::ReadProgressTextL
//
// -----------------------------------------------------------------------------
//
HBufC* CAspResHandler::ReadProgressTextLC(const TDesC& aContent, TInt aPhase)
	{
	HBufC* hBuf = NULL;
	
	if (aPhase == CAspState::EPhaseSending)
		{
	    hBuf = ReadProgressTextLC(R_QTN_SML_SYNC_SENDING, aContent);
		}
	else if (aPhase == CAspState::EPhaseReceiving)
		{
		hBuf = ReadProgressTextLC(R_QTN_SML_SYNC_RECEIVING, aContent);
		}
    else
    	{
    	hBuf = HBufC::NewLC(0);  // empty string
    	}
	
	return hBuf;
	}


// -----------------------------------------------------------------------------
// CAspResHandler::ReadProgressTextL
//
// Function constructs progress dialog text that is shown with progress bar.
// Text format is one of the following:
// "Sending %U"
// "Receiving %U"
// -----------------------------------------------------------------------------
//
HBufC* CAspResHandler::ReadProgressTextLC(TInt aResourceId, const TDesC& aContent)
	{
    
    if (aResourceId != iProgressTextFormatId)
    	{
    	delete iProgressTextFormat;
    	iProgressTextFormat = NULL;
    	iProgressTextFormat = StringLoader::LoadL(aResourceId, iEikEnv);
    	iProgressTextFormatId = aResourceId;
    	}
    	
    TInt len = iProgressTextFormat->Length() + aContent.Length();
    
    HBufC* hBuf = HBufC::NewLC(len);
    TPtr ptr = hBuf->Des();

    
    // replace  %0U with content name (eg "Contacts")
    StringLoader::Format(ptr, iProgressTextFormat->Des(), 0, aContent);
    
    //HBufC* hBuf = StringLoader::LoadLC(aResourceId, aContent);
   
    return hBuf;
	}

/* Implementation of CMCC PIM v3 begins*/
// -----------------------------------------------------------------------------
// CAspResHandler::ReadProgressTextL
//
// Function constructs progress dialog text that is shown with progress bar.
// Text format is one of the following:
// "Sending %0N of %1N %U"
// "Receiving %0U: %0N"
// -----------------------------------------------------------------------------
//
HBufC* CAspResHandler::ReadProgressTextLC(TInt aResourceId, TInt aCurrent, TInt aFinal, const TDesC& aContent)
    {
    
    if (aResourceId != iProgressTextFormatId)
        {
        delete iProgressTextFormat;
        iProgressTextFormat = NULL;
        iProgressTextFormat = StringLoader::LoadL(aResourceId, iEikEnv);
        iProgressTextFormatId = aResourceId;
        }
    
    TInt len = iProgressTextFormat->Length() + aContent.Length();// + 8;
    
    HBufC* hBuf = HBufC::NewLC(len);
    TPtr ptr = hBuf->Des();

    HBufC* temp = HBufC::NewLC(len);
    TPtr tempPtr = temp->Des();
    
    if (iProgressTextFormatId == R_QTN_SML_SYNC_RECEIVING_XOFY || iProgressTextFormatId == R_QTN_SML_SYNC_RECEIVING_XOFY_CAL)
        {
        // replace %0N with current item
        StringLoader::Format(tempPtr, iProgressTextFormat->Des(), 0, aCurrent);
        }
    else
        {
        // replace %0N with current item
        StringLoader::Format(ptr, iProgressTextFormat->Des(), 0, aCurrent);
        // replace %1N with total items
        StringLoader::Format(tempPtr, ptr, 1, aFinal);
        }
    if (iProgressTextFormatId == R_QTN_SML_SYNC_RECEIVING_XOFY_CAL || iProgressTextFormatId == R_QTN_SML_SYNC_SENDING_XOFY_CAL)
        {
        ptr.Copy(tempPtr);
        }
    else
        {
        // replace  %0U with content name (eg "Contacts")
        StringLoader::Format(ptr, tempPtr, 0, aContent);
        }
    CleanupStack::PopAndDestroy(temp); //temp
    
    return hBuf;
    }
/* Implementation of CMCC PIM v3 ends*/

// -----------------------------------------------------------------------------
// CAspResHandler::ReadProfileInfoTextLC
//
// Text format: 
// "Profile: %0U, connection: %1U"
// -----------------------------------------------------------------------------
//
HBufC* CAspResHandler::ReadProfileInfoTextLC(const TDesC& aProfile, const TDesC& aBearer)
	{
    HBufC* hBuf   = HBufC::NewLC(KBufSize255);
    HBufC* temp   = HBufC::NewLC(KBufSize255);
    HBufC* format = ReadLC(R_ASP_POPUP_PROFILE_INFO);
    	
    TPtr ptr = hBuf->Des();
    TPtr tempPtr = temp->Des();
    
    // replace  %0U with profile name
    StringLoader::Format(tempPtr, format->Des(), 0, aProfile);
    
    // replace %1U with bearer name
    StringLoader::Format(ptr, tempPtr, 1, aBearer);
    
    AknTextUtils::DisplayTextLanguageSpecificNumberConversion(ptr);
    
    
    CleanupStack::PopAndDestroy(format);
    CleanupStack::PopAndDestroy(temp);
    
    return hBuf;
	}


// -----------------------------------------------------------------------------
// CAspResHandler::ReadProfileInfoTextLC
//
// Text format: 
// "Profile: %0U, connection: %1U. Next sync in %0N hours %1N minutes"
// -----------------------------------------------------------------------------
//
HBufC* CAspResHandler::ReadProfileInfoTextLC(const TDesC& aProfile, const TDesC& aBearer,
		                                    TInt aHours, TInt aMinutes)
	{
    HBufC* hBuf   = HBufC::NewLC(KBufSize255);
    HBufC* temp   = HBufC::NewLC(KBufSize255);
    HBufC* format = ReadLC(R_ASP_POPUP_AUTO_SYNC_PROFILE);
    	
    TPtr ptr = hBuf->Des();
    TPtr tempPtr = temp->Des();
    
    // replace  %0U with profile name
    StringLoader::Format(tempPtr, format->Des(), 0, aProfile);
    
    // replace %1U with bearer name
    StringLoader::Format(ptr, tempPtr, 1, aBearer);
    
    // replace %0N with hours
    StringLoader::Format(tempPtr, ptr, 0, aHours);

    // replace %1N with minutes
    StringLoader::Format(ptr, tempPtr, 1, aMinutes);

    AknTextUtils::DisplayTextLanguageSpecificNumberConversion(ptr);
    
    
    CleanupStack::PopAndDestroy(format);
    CleanupStack::PopAndDestroy(temp);
    
    return hBuf;
	}
		                                    		

// -----------------------------------------------------------------------------
// CAspResHandler::ReadAutoSyncInfoTextLC
//
// Text format: 
// "Next sync: %0U, profile: %1U"
// -----------------------------------------------------------------------------
//
HBufC* CAspResHandler::ReadAutoSyncInfoTextLC(const TDesC& aProfile, TTime aTime)
	{
    HBufC* hBuf   = HBufC::NewLC(KBufSize255);
    HBufC* temp   = HBufC::NewLC(KBufSize255);
    HBufC* format = ReadLC(R_ASP_MAIN_NOTE_AUTO_ON);
    	
    TPtr ptr = hBuf->Des();
    TPtr tempPtr = temp->Des();

	TTime currentTime;
	currentTime.HomeTime();
	TDateTime today = currentTime.DateTime();
	TDateTime syncDay = aTime.DateTime();

	TBuf<64> aTimeString;
	if(today.Day() == syncDay.Day() && today.Month() == syncDay.Month())
		{
		//show next sync time
		TBuf<KBufSize> timeFormat;
		CAspResHandler::ReadL(timeFormat, R_QTN_TIME_USUAL_WITH_ZERO);
		aTime.FormatL(aTimeString, timeFormat);

		}
	else 
		{
		//show sync day
		TBuf<KBufSize> dateFormat;
		CAspResHandler::ReadL(dateFormat, R_QTN_DATE_USUAL_WITH_ZERO);
		aTime.FormatL(aTimeString, dateFormat);

		}
	
	
    // replace  %0U with Next Sync
    StringLoader::Format(tempPtr, format->Des(), 0, aTimeString);
        
    // replace %1U with profile name
    StringLoader::Format(ptr, tempPtr, 1, aProfile);
    
    AknTextUtils::DisplayTextLanguageSpecificNumberConversion(ptr);
    
    
    CleanupStack::PopAndDestroy(format);
    CleanupStack::PopAndDestroy(temp);
    
    return hBuf;
	}
		                

// -----------------------------------------------------------------------------
// CAspResHandler::GetNewProfileNameLC
//
// -----------------------------------------------------------------------------
//
HBufC* CAspResHandler::GetNewProfileNameLC(TInt aIndex, TInt aApplicationId)
	{
	HBufC* hBuf = NULL;
	
	switch (aApplicationId)
		{
		case EApplicationIdContact:
			hBuf =  StringLoader::LoadLC(R_ASP_PROFILENAME_PHONEBOOK, aIndex);
			break;
		case EApplicationIdCalendar:
			hBuf =  StringLoader::LoadLC(R_ASP_PROFILENAME_CALENDAR, aIndex);
			break;
		case EApplicationIdEmail:
			hBuf =  StringLoader::LoadLC(R_ASP_PROFILENAME_EMAIL, aIndex);
			break;
		case EApplicationIdNote:
			hBuf =  StringLoader::LoadLC(R_ASP_PROFILENAME_NOTE, aIndex);
			break;
		default:
			hBuf =  StringLoader::LoadLC(R_QTN_APPS_NEWPROFILE, aIndex);
			break;
		}
		
	return hBuf;
	}


// -----------------------------------------------------------------------------
// CAspResHandler::GetContentNameLC
// 
// -----------------------------------------------------------------------------
//
HBufC* CAspResHandler::GetContentNameLC(TInt aDataProviderId, const TDesC& aDefaultText)
	{
    if (aDataProviderId == KUidNSmlAdapterContact.iUid)
		{
		return CAspResHandler::ReadLC(R_ASP_CONTENT_NAME_PHONEBOOK);
		}
	else if (aDataProviderId == KUidNSmlAdapterCalendar.iUid)
		{
		return CAspResHandler::ReadLC(R_ASP_CONTENT_NAME_CALENDAR);
		}
	else if (aDataProviderId == KUidNSmlAdapterEMail.iUid)
		{
		return CAspResHandler::ReadLC(R_ASP_CONTENT_NAME_EMAIL);
		}
	else if (aDataProviderId == KUidNSmlAdapterNote.iUid)
		{
		return CAspResHandler::ReadLC(R_ASP_CONTENT_NAME_NOTE);
		}
	else if (aDataProviderId == KUidNSmlAdapterSms.iUid)
		{
		return CAspResHandler::ReadLC(R_ASP_CONTENT_NAME_SMS);
		}
	else if (aDataProviderId == KUidNSmlAdapterMMS.iUid)
		{
		return CAspResHandler::ReadLC(R_ASP_CONTENT_NAME_MMS);
		}
	else if (aDataProviderId == KUidNSmlAdapterBookmarks.iUid)
		{
		return CAspResHandler::ReadLC(R_ASP_CONTENT_NAME_BKM);
		}
	else
		{
		return aDefaultText.AllocLC();
		}
	}


// -----------------------------------------------------------------------------
// CAspResHandler::GetContentTitleLC
// 
// -----------------------------------------------------------------------------
//
HBufC* CAspResHandler::GetContentTitleLC(TInt aDataProviderId, const TDesC& aDefaultText)
	{
	if (aDataProviderId == KUidNSmlAdapterContact.iUid)
		{
		return CAspResHandler::ReadLC(R_ASP_CONTENT_TITLE_PHONEBOOK);
		}
	if (aDataProviderId == KUidNSmlAdapterCalendar.iUid)
		{
		return CAspResHandler::ReadLC(R_ASP_CONTENT_TITLE_CALENDAR);
		}
	else if (aDataProviderId == KUidNSmlAdapterEMail.iUid)
		{
		return CAspResHandler::ReadLC(R_ASP_CONTENT_TITLE_EMAIL);
		}
	else if (aDataProviderId == KUidNSmlAdapterNote.iUid)
		{
    	return CAspResHandler::ReadLC(R_ASP_CONTENT_TITLE_NOTE);
		}
	else if (aDataProviderId == KUidNSmlAdapterSms.iUid)
		{
    	return CAspResHandler::ReadLC(R_ASP_CONTENT_TITLE_SMS);
		}
	else if (aDataProviderId == KUidNSmlAdapterMMS.iUid)
		{
    	return CAspResHandler::ReadLC(R_ASP_CONTENT_TITLE_MMS);
		}
	else if (aDataProviderId == KUidNSmlAdapterBookmarks.iUid)
		{
    	return CAspResHandler::ReadLC(R_ASP_CONTENT_TITLE_BKM);
		}
	else
		{
		return aDefaultText.AllocLC();
		}
	}


// -----------------------------------------------------------------------------
// CAspResHandler::GetContentSettingLC
// 
// -----------------------------------------------------------------------------
//
HBufC* CAspResHandler::GetContentSettingLC(TInt aDataProviderId, const TDesC& aDefaultText)
	{
	if (aDataProviderId == KUidNSmlAdapterContact.iUid)
		{
		return CAspResHandler::ReadLC(R_ASP_CONTENT_SETTING_PHONEBOOK);
		}
	if (aDataProviderId == KUidNSmlAdapterCalendar.iUid)
		{
		return CAspResHandler::ReadLC(R_ASP_CONTENT_SETTING_CALENDAR);
		}
	else if (aDataProviderId == KUidNSmlAdapterEMail.iUid)
		{
		return CAspResHandler::ReadLC(R_ASP_CONTENT_SETTING_EMAIL);
		}
	else if (aDataProviderId == KUidNSmlAdapterNote.iUid)
		{
    	return CAspResHandler::ReadLC(R_ASP_CONTENT_SETTING_NOTES);
		}
	else if (aDataProviderId == KUidNSmlAdapterSms.iUid)
		{
    	return CAspResHandler::ReadLC(R_ASP_CONTENT_SETTING_SMS);
		}
	else if (aDataProviderId == KUidNSmlAdapterMMS.iUid)
		{
    	return CAspResHandler::ReadLC(R_ASP_CONTENT_SETTING_MMS);
		}
	else if (aDataProviderId == KUidNSmlAdapterBookmarks.iUid)
		{
    	return CAspResHandler::ReadLC(R_ASP_CONTENT_SETTING_BKM);
		}
	else
		{
		return aDefaultText.AllocLC();
		}
	}


//------------------------------------------------------------------------------
// CAspResHandler::RemoteDatabaseHeaderId
//
//------------------------------------------------------------------------------
//
TInt CAspResHandler::RemoteDatabaseHeaderId(TInt aDataProviderId)
	{
	TInt ret = R_ASP_REMOTE_DATABASE;
	
	if (aDataProviderId == KUidNSmlAdapterContact.iUid)
		{
		ret = R_ASP_REMOTE_DATABASE_CNT;
		}
	else if (aDataProviderId == KUidNSmlAdapterCalendar.iUid)
		{
		ret = R_ASP_REMOTE_DATABASE_AGN;
		}
	else if (aDataProviderId == KUidNSmlAdapterEMail.iUid)
		{
		ret = R_ASP_REMOTE_DATABASE_EML;
		}
	else if (aDataProviderId == KUidNSmlAdapterNote.iUid)
		{
		ret = R_ASP_REMOTE_DATABASE_NPD;
		}
	else if (aDataProviderId == KUidNSmlAdapterSms.iUid)
		{
		ret = R_ASP_REMOTE_DATABASE_SMS;
		}
	else if (aDataProviderId == KUidNSmlAdapterMMS.iUid)
		{
		ret = R_ASP_REMOTE_DATABASE_MMS;
		}
	else if (aDataProviderId == KUidNSmlAdapterBookmarks.iUid)
		{
		ret = R_ASP_REMOTE_DATABASE_BKM;
		}
		
	return ret;
	}

// End of file
