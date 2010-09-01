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
* Description:  
*
*/


// INCLUDE FILES

#include "AspProfileWizard.h"
#include "AspUtil.h"
#include "AspDebug.h"
#include "AspSyncUtil.rh"


#include <textresolver.h>
#include <applayout.cdl.h>      // Multiline_Message_writing_texts_Line_3
#include <aknlayoutscalable_apps.cdl.h>

				 



/*******************************************************************************
 * class CAspWizardItem
 *******************************************************************************/


// -----------------------------------------------------------------------------
// CAspWizardItem::NewL
//
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CAspWizardItem* CAspWizardItem::NewL()
    {
    CAspWizardItem* self = new (ELeave) CAspWizardItem();
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
CAspWizardItem::~CAspWizardItem()
    {
    if (iValue)
		{
		delete iValue;
		iValue = NULL ;
		}
    delete iContentName;
    delete iHeader;
    }


// -----------------------------------------------------------------------------
// CAspWizardItem::CAspWizardItem
//
// -----------------------------------------------------------------------------
//
CAspWizardItem::CAspWizardItem()
    {
    }


// -----------------------------------------------------------------------------
// CAspWizardItem::ConstructL
//
// -----------------------------------------------------------------------------
//
void CAspWizardItem::ConstructL()
    {
	InitL();
    }


// -----------------------------------------------------------------------------
// CAspWizardItem::SetValueL
//
// -----------------------------------------------------------------------------
//
void CAspWizardItem::SetValueL(const TDesC& aText)
	{
	if (iValue)
		{
		delete iValue;
		iValue = NULL;
		}
	iValue = aText.AllocL();
	}


// -----------------------------------------------------------------------------
// CAspWizardItem::Value
//
// -----------------------------------------------------------------------------
//
const TDesC& CAspWizardItem::Value()
	{
	__ASSERT_ALWAYS(iValue, TUtil::Panic(KErrGeneral));
		
	return *iValue;
	}


// -----------------------------------------------------------------------------
// CAspWizardItem::SetContentNameL
//
// -----------------------------------------------------------------------------
//
void CAspWizardItem::SetContentNameL(const TDesC& aText)
	{
	delete iContentName;
	iContentName = NULL;
	iContentName = aText.AllocL();
	}


// -----------------------------------------------------------------------------
// CAspWizardItem::ContentName
//
// -----------------------------------------------------------------------------
//
const TDesC& CAspWizardItem::ContentName()
	{
	__ASSERT_ALWAYS(iContentName, TUtil::Panic(KErrGeneral));
		
	return *iContentName;
	}


// -----------------------------------------------------------------------------
// CAspWizardItem::SetHeaderL
//
// -----------------------------------------------------------------------------
//
void CAspWizardItem::SetHeaderL(const TDesC& aText)
	{
	delete iHeader;
	iHeader = NULL;
	iHeader = aText.AllocL();
	}


// -----------------------------------------------------------------------------
// CAspWizardItem::SetHeaderL
//
// -----------------------------------------------------------------------------
//
void CAspWizardItem::SetHeaderL(TInt aResourceId)
	{
	delete iHeader;
	iHeader = NULL;
	iHeader = CAspResHandler::ReadL(aResourceId);
	}


// -----------------------------------------------------------------------------
// CAspWizardItem::Header
//
// -----------------------------------------------------------------------------
//
const TDesC& CAspWizardItem::Header()
	{
	__ASSERT_ALWAYS(iHeader, TUtil::Panic(KErrGeneral));
		
	return *iHeader;
	}


// -----------------------------------------------------------------------------
// CAspWizardItem::InitL
//
// -----------------------------------------------------------------------------
//
void CAspWizardItem::InitL()
	{
	iSettingType = KErrNotFound;
	iSettingId = KErrNotFound;
	iResourceId = KErrNotFound;
	iPopupNoteResourceId = KErrNotFound;
	iErrorNoteResourceId = KErrNotFound;
	iEnabled = ETrue;
	iMaxLength = 0;
	iMandatory = ETrue;
	iLatinInput = ETrue;  // accept only western chars
	iNumber = 0;
	SetValueL(KNullDesC);
	SetContentNameL(KNullDesC);
	SetHeaderL(KNullDesC);
    }


	
	

/*******************************************************************************
 * class CAspProfileWizard
 *******************************************************************************/




// -----------------------------------------------------------------------------
// CAspProfileWizard::NewL
//
// -----------------------------------------------------------------------------
CAspProfileWizard* CAspProfileWizard::NewL(TAspParam& aParam)
    {
    CAspProfileWizard* self = new (ELeave) CAspProfileWizard(aParam);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);

    return(self);
    }


// -----------------------------------------------------------------------------
// CAspProfileWizard::CAspProfileWizard
// 
// -----------------------------------------------------------------------------
//
CAspProfileWizard::CAspProfileWizard(TAspParam& aParam)
	{
	iApplicationId = aParam.iApplicationId;
	iSyncSession = aParam.iSyncSession;
	
	iDialogParam = &aParam;
	iProfileId = KErrNotFound;
	iCurrentItemIndex = 0;

	__ASSERT_ALWAYS(iSyncSession, TUtil::Panic(KErrArgument));
    }


// -----------------------------------------------------------------------------
// CAspProfileWizard::ConstructL
//
// -----------------------------------------------------------------------------
//
void CAspProfileWizard::ConstructL()
    {
    TAspParam param(iApplicationId, iSyncSession);
	param.iMode = CAspContentList::EInitDataProviders;
	iContentList = CAspContentList::NewL(param);

   	// get email syncml mailboxes
   	iLocalDataStores = new (ELeave) CDesCArrayFlat(KDefaultArraySize);
	iContentList->GetLocalDatabaseList(KUidNSmlAdapterEMail.iUid, iLocalDataStores);

    CreateSettingListL();

    SetContentEnabled(iApplicationId);

   	if (iLocalDataStores->Count() == 0)
   		{
   		SetContentEnabled(KUidNSmlAdapterEMail.iUid, EFalse);
   		}
   	TBool emailEnabled = ContentEnabled(KUidNSmlAdapterEMail.iUid);
	SetContentLocalDatabaseEnabled(KUidNSmlAdapterEMail.iUid, emailEnabled);
	
	if (iApplicationId == EApplicationIdEmail)
		{
		SetEnabled(CAspWizardItem::ETypeProtocolVersion, EFalse);
		}
    } 


// ----------------------------------------------------------------------------
// Destructor
//
// ----------------------------------------------------------------------------
//
CAspProfileWizard::~CAspProfileWizard()
    {
    delete iLocalDataStores;
    
    iSettingList.ResetAndDestroy();
    iSettingList.Close();
    
    delete iContentList;
    }


// -----------------------------------------------------------------------------
// CAspProfileWizard::CreateSettingListL
//
// -----------------------------------------------------------------------------
//
void CAspProfileWizard::CreateSettingListL()
    {
    TInt localDbCount = iLocalDataStores->Count();
    
	CAspWizardItem* item = CAspWizardItem::NewL();
	item->iSettingType = CAspWizardItem::ETypeStart;
	item->iResourceId = R_ASP_SET_WIZARD_START;
	item->SetHeaderL(item->iResourceId);
	item->iEnabled = ETrue; // start must always be enabled
	iSettingList.Append(item);

	item = CAspWizardItem::NewL();
	item->iSettingType = CAspWizardItem::ETypeProfileName;
	item->iResourceId = R_ASP_SET_DESTINATION_NAME;
	item->iPopupNoteResourceId = R_ASP_WIZARD_POPUP_PROFILE_NAME;
	item->iErrorNoteResourceId = R_ASP_NOTE_PROFILE_NAME_MISSING;
	item->SetHeaderL(item->iResourceId);
	item->iMaxLength = KAspMaxProfileNameLength;
	item->iLatinInput = EFalse; // accept also non-western chars
	iSettingList.Append(item);

	item = CAspWizardItem::NewL();
	item->iSettingType = CAspWizardItem::ETypeProtocolVersion;
	item->iResourceId = R_ASP_SET_PROTOCOL_VERSION;
	item->iPopupNoteResourceId = R_ASP_WIZARD_POPUP_PROTOCOL_VERSION;
	item->SetHeaderL(item->iResourceId);
	iSettingList.Append(item);

	item = CAspWizardItem::NewL();
	item->iSettingType = CAspWizardItem::ETypeContentSelection;
	item->iResourceId = R_ASP_SET_CONTENT_SELECTION;
	item->iPopupNoteResourceId = R_ASP_WIZARD_POPUP_CONTENT_SELECTION;
	item->SetHeaderL(item->iResourceId);
    iSettingList.Append(item);

    TInt count = iContentList->ProviderCount();
	for (TInt i=0; i<count; i++)
		{
		TAspProviderItem& p = iContentList->ProviderItem(i);
	
	    //if (p.iDataProviderId == KUidNSmlAdapterEMail.iUid && localDbCount == 0)
	    //	{
	    //	continue; // leave email out - no email SyncML mailboxes found
	    //	}
	    
	    item = CAspWizardItem::NewL();
	    item->iSettingType = CAspWizardItem::ETypeSyncContent;
	    item->iSettingId = p.iDataProviderId;
	    item->iResourceId = 
	          CAspResHandler::RemoteDatabaseHeaderId(p.iDataProviderId);
	    item->iPopupNoteResourceId = 
	          ContentPopupNoteResourceId(p.iDataProviderId);
	    
	    if (p.iDataProviderId == KUidNSmlAdapterContact.iUid)
            {
            item->iErrorNoteResourceId = R_ASP_NOTE_CONTACTS_DB_MISSING;
            }
        else if (p.iDataProviderId == KUidNSmlAdapterCalendar.iUid)
            {
            item->iErrorNoteResourceId = R_ASP_NOTE_CALENDAR_DB_MISSING;
            }
        else if (p.iDataProviderId == KUidNSmlAdapterSms.iUid)
            {
            item->iErrorNoteResourceId = R_ASP_NOTE_SMS_DATABASE_MISSING;
            }
        else if (p.iDataProviderId == KUidNSmlAdapterMMS.iUid)
            {
            item->iErrorNoteResourceId = R_ASP_NOTE_MMS_DATABASE_MISSING;
            }
        else if (p.iDataProviderId == KUidNSmlAdapterNote.iUid)
            {
            item->iErrorNoteResourceId = R_ASP_NOTE_NOTES_DB_MISSING;
            }
        else if (p.iDataProviderId == KUidNSmlAdapterBookmarks.iUid)
            {
            item->iErrorNoteResourceId = R_ASP_NOTE_BKM_DATABASE_MISSING;
            }
        else
            {
            item->iErrorNoteResourceId = R_ASP_NOTE_REMOTE_DATABASE_MISSING;
            }
	    item->SetHeaderL(item->iResourceId);
	    item->iMaxLength = KAspMaxRemoteNameLength;
	    
	    HBufC* hBuf = CAspResHandler::GetContentNameLC(p.iDataProviderId, p.iDisplayName);
	    item->SetContentNameL(hBuf->Des());
        CleanupStack::PopAndDestroy(hBuf);
	    
        iSettingList.Append(item);
        
        if (p.iDataProviderId == KUidNSmlAdapterEMail.iUid && localDbCount > 1)
        	{
	        item = CAspWizardItem::NewL();
	        item->iSettingType = CAspWizardItem::ETypeSyncContentLocalDatabase;
	        item->iSettingId = p.iDataProviderId;
	        item->iResourceId = R_ASP_LOCAL_DATABASE_EML;
	        item->iPopupNoteResourceId = R_ASP_WIZARD_POPUP_EMAIL_LOCAL_DB;
	        item->SetHeaderL(item->iResourceId);
    
            iSettingList.Append(item);
        	}
		}

	item = CAspWizardItem::NewL();
	item->iSettingType = CAspWizardItem::ETypeBearerType;
	item->iResourceId = R_ASP_SET_BEARER_TYPE;
	item->iPopupNoteResourceId = R_ASP_WIZARD_POPUP_CONNECTION_TYPE;
	item->SetHeaderL(item->iResourceId);
    iSettingList.Append(item);

	item = CAspWizardItem::NewL();
	item->iSettingType = CAspWizardItem::ETypeHostAddress;
	item->iResourceId = R_ASP_SET_HOST_ADDRESS;
	item->iPopupNoteResourceId = R_ASP_WIZARD_POPUP_HOST_ADDRESS;
	item->iErrorNoteResourceId = R_ASP_NOTE_HOST_ADDRESS_MISSING;
	item->SetHeaderL(item->iResourceId);
	item->iMaxLength = KAspMaxURILength;
    iSettingList.Append(item);

	item = CAspWizardItem::NewL();
	item->iSettingType = CAspWizardItem::ETypeServerId;
	item->iResourceId = R_ASP_SET_SERVER_ID;
	item->iPopupNoteResourceId = R_ASP_WIZARD_POPUP_SERVER_ID;
	item->iErrorNoteResourceId = KErrNotFound;
	item->SetHeaderL(item->iResourceId);
	item->iMaxLength = KAspMaxServerIdLength;
    iSettingList.Append(item);

	item = CAspWizardItem::NewL();
	item->iSettingType = CAspWizardItem::ETypeUserName;
	item->iResourceId = R_ASP_SET_USERNAME;
	item->iPopupNoteResourceId = R_ASP_WIZARD_POPUP_USERNAME;
	item->iErrorNoteResourceId = R_ASP_NOTE_USERNAME_MISSING;
	item->SetHeaderL(item->iResourceId);
	item->iMaxLength = KAspMaxUsernameLength;
    iSettingList.Append(item);

	item = CAspWizardItem::NewL();
	item->iSettingType = CAspWizardItem::ETypePassword;
	item->iResourceId = R_ASP_SET_PASSWORD;
	item->iPopupNoteResourceId = R_ASP_WIZARD_POPUP_PASSWORD;
	item->iErrorNoteResourceId = R_ASP_NOTE_PASSWORD_MISSING;
	item->SetHeaderL(item->iResourceId);
	item->iMaxLength = KAspMaxPasswordLength;
    iSettingList.Append(item);
    
	item = CAspWizardItem::NewL();
	item->iSettingType = CAspWizardItem::ETypeEnd;
	item->iResourceId = R_ASP_SET_WIZARD_END;
	item->SetHeaderL(item->iResourceId);
	item->iEnabled = ETrue; // end must always be enabled
	iSettingList.Append(item);
    } 


// -----------------------------------------------------------------------------
// CAspProfileWizard::ContentPopupNoteResourceId
// 
// -----------------------------------------------------------------------------
//
TInt CAspProfileWizard::ContentPopupNoteResourceId(TInt aDataProviderId)
	{
	TInt ret = R_ASP_WIZARD_POPUP_REMOTE_DB;
	
	if (aDataProviderId == KUidNSmlAdapterContact.iUid)
		{
		ret = R_ASP_WIZARD_POPUP_CONTACTS_DB;
		}
	if (aDataProviderId == KUidNSmlAdapterCalendar.iUid)
		{
		ret = R_ASP_WIZARD_POPUP_CALENDAR_DB;
		}
	else if (aDataProviderId == KUidNSmlAdapterEMail.iUid)
		{
		ret = R_ASP_WIZARD_POPUP_EMAIL_DB;
		}
	else if (aDataProviderId == KUidNSmlAdapterNote.iUid)
		{
    	ret = R_ASP_WIZARD_POPUP_NOTES_DB;
		}
	else if (aDataProviderId == KUidNSmlAdapterSms.iUid)
		{
    	ret = R_ASP_WIZARD_POPUP_SMS_DB;
		}
	else if (aDataProviderId == KUidNSmlAdapterMMS.iUid)
		{
    	ret = R_ASP_WIZARD_POPUP_MMS_DB;
		}
	else if (aDataProviderId == KUidNSmlAdapterBookmarks.iUid)
		{
    	ret = R_ASP_WIZARD_POPUP_BKM_DB;
		}
    

    return ret;
	}


//------------------------------------------------------------------------------
// CAspProfileWizard::GetContentSelectionL
// 
// Construct CSelectionItemList from setting list.
//------------------------------------------------------------------------------
//
void CAspProfileWizard::GetContentSelectionL(CSelectionItemList* aList)
	{
	TInt count = iSettingList.Count();
	for (TInt i=0; i<count; i++)
		{
		CAspWizardItem* item = iSettingList[i];
		if (item->iSettingType == CAspWizardItem::ETypeSyncContent)
			{
	        TBuf<KBufSize> buf(item->ContentName());
	        CSelectableItem* selectableItem = new (ELeave) CSelectableItem(
	                                          buf, item->iEnabled);
   	        CleanupStack::PushL(selectableItem);
	        selectableItem->ConstructL();
	        aList->AppendL(selectableItem);
	        CleanupStack::Pop(selectableItem);
			}
		}
	}


//------------------------------------------------------------------------------
// CAspProfileWizard::SetContentSelectionL
// 
// Update setting list from CSelectionItemList.
//------------------------------------------------------------------------------
//
void CAspProfileWizard::SetContentSelectionL(CSelectionItemList* aList)
	{
	TInt count = aList->Count();
	
	for (TInt i=0; i<count; i++)
		{
		CSelectableItem* selectableItem = (*aList)[i];
		TBool selected = selectableItem->SelectionStatus();
		SetContentEnabled(selectableItem->ItemText(), selected);
		}
		
	TBool emailEnabled = ContentEnabled(KUidNSmlAdapterEMail.iUid);
	SetContentLocalDatabaseEnabled(KUidNSmlAdapterEMail.iUid, emailEnabled);
    }


//------------------------------------------------------------------------------
// CAspProfileWizard::SetContentEnabled
// 
//------------------------------------------------------------------------------
//
void CAspProfileWizard::SetContentEnabled(const TDesC& aText, TBool aEnabled)
	{
	TInt count = iSettingList.Count();
	for (TInt i=0; i<count; i++)
		{
		CAspWizardItem* item = iSettingList[i];
		if (item->iSettingType == CAspWizardItem::ETypeSyncContent)
			{
	        TPtrC ptr = item->ContentName();
	        if (ptr.Compare(aText) == 0)
	        	{
	        	item->iEnabled = aEnabled;
	        	break;
	        	}
			}
		}
	}


//------------------------------------------------------------------------------
// CAspProfileWizard::ContentId
// 
//------------------------------------------------------------------------------
//
TInt CAspProfileWizard::ContentId(const TDesC& aText)
	{
	TInt count = iSettingList.Count();
	for (TInt i=0; i<count; i++)
		{
		CAspWizardItem* item = iSettingList[i];
		if (item->iSettingType == CAspWizardItem::ETypeSyncContent)
			{
	        TPtrC ptr = item->ContentName();
	        if (ptr.Compare(aText) == 0)
	        	{
	        	return item->iSettingId;
	        	}
			}
		}
		
	return KErrNotFound;
	}


//------------------------------------------------------------------------------
// CAspProfileWizard::SetContentEnabled
// 
//------------------------------------------------------------------------------
//
void CAspProfileWizard::SetContentEnabled(TInt aApplicationId)
	{
	TInt count = iSettingList.Count();
	for (TInt i=0; i<count; i++)
		{
		CAspWizardItem* item = iSettingList[i];
		if (item->iSettingType != CAspWizardItem::ETypeSyncContent)
			{
			continue;
			}
		
		if (aApplicationId == EApplicationIdSync)
			{
			item->iEnabled = ETrue; // sync app enables all contents
			}
		else
			{
			TInt appId = TUtil::AppIdFromProviderId(item->iSettingId);
		    if (appId == aApplicationId)
			    {
			    item->iEnabled = ETrue;
			    }
		    else
			    {
			    item->iEnabled = EFalse;
			    }
			}
		}
		
	if (aApplicationId != EApplicationIdSync)
		{
		// only sync app shows content selection page
		SetEnabled(CAspWizardItem::ETypeContentSelection, EFalse);
		}
	}


//------------------------------------------------------------------------------
// CAspProfileWizard::SetContentLocalDatabaseEnabled
// 
//------------------------------------------------------------------------------
//
void CAspProfileWizard::SetContentLocalDatabaseEnabled(TInt aDataproviderId, TBool aEnabled)
	{
	TInt count = iSettingList.Count();
	for (TInt i=0; i<count; i++)
		{
		CAspWizardItem* item = iSettingList[i];
		if (item->iSettingType == CAspWizardItem::ETypeSyncContentLocalDatabase)
			{
			if (item->iSettingId == aDataproviderId)
			    {
			    item->iEnabled = aEnabled;
			    }
			}
		}
	}


//------------------------------------------------------------------------------
// CAspProfileWizard::SetContentEnabled
// 
//------------------------------------------------------------------------------
//
void CAspProfileWizard::SetContentEnabled(TInt aDataproviderId, TBool aEnabled)
	{
	TInt count = iSettingList.Count();
	for (TInt i=0; i<count; i++)
		{
		CAspWizardItem* item = iSettingList[i];
		if (item->iSettingType == CAspWizardItem::ETypeSyncContent)
			{
			if (item->iSettingId == aDataproviderId)
			    {
			    item->iEnabled = aEnabled;
			    }
			}
		}
	}


//------------------------------------------------------------------------------
// CAspProfileWizard::ContentEnabled
// 
//------------------------------------------------------------------------------
//
TBool CAspProfileWizard::ContentEnabled(TInt aDataproviderId)
	{
	TInt count = iSettingList.Count();
	for (TInt i=0; i<count; i++)
		{
		CAspWizardItem* item = iSettingList[i];
		if (item->iSettingType == CAspWizardItem::ETypeSyncContent)
			{
			if (item->iSettingId == aDataproviderId)
			    {
			    return item->iEnabled;
			    }
			}
		}
		
	return EFalse;
	}


//------------------------------------------------------------------------------
// CAspProfileWizard::SetContentEnabled
// 
//------------------------------------------------------------------------------
//
void CAspProfileWizard::SetContentEnabled(RArray<TInt>& aDataProviderArray)
	{
	TInt count = iSettingList.Count();
	for (TInt i=0; i<count; i++)
		{
		CAspWizardItem* item = iSettingList[i];
		if (item->iSettingType == CAspWizardItem::ETypeSyncContent)
			{
			item->iEnabled = EFalse;
			if (aDataProviderArray.Find(item->iSettingId) != KErrNotFound)
				{
				item->iEnabled = ETrue;
				}
			}
		}
	}


//------------------------------------------------------------------------------
// CAspProfileWizard::GetContentEnabled
// 
//------------------------------------------------------------------------------
//
void CAspProfileWizard::GetContentEnabled(RArray<TInt>& aDataProviderArray)
	{
	aDataProviderArray.Reset();
	
	TInt count = iSettingList.Count();
	for (TInt i=0; i<count; i++)
		{
		CAspWizardItem* item = iSettingList[i];
		if (item->iSettingType == CAspWizardItem::ETypeSyncContent)
			{
			if (item->iEnabled)
				{
				aDataProviderArray.Append(item->iSettingId);
				}
			}
		}
	}


//------------------------------------------------------------------------------
// CAspProfileWizard::GetContentName
// 
//------------------------------------------------------------------------------
//
void CAspProfileWizard::GetContentName(TDes& aText)
	{
	aText = KNullDesC;
	
	TInt count = iSettingList.Count();
	for (TInt i=0; i<count; i++)
		{
		CAspWizardItem* item = iSettingList[i];
		if (item->iSettingType == CAspWizardItem::ETypeSyncContent)
			{
			if (item->iEnabled)
				{
				aText = item->ContentName();
				}
			}
		}
	}


//------------------------------------------------------------------------------
// CAspProfileWizard::SetEnabled
// 
//------------------------------------------------------------------------------
//
void CAspProfileWizard::SetEnabled(TInt aSettingType, TBool aEnabled)
	{
	TInt count = iSettingList.Count();
	for (TInt i=0; i<count; i++)
		{
		CAspWizardItem* item = iSettingList[i];
		if (item->iSettingType == aSettingType)
			{
        	item->iEnabled = aEnabled;
        	break;
			}
		}
	}


//------------------------------------------------------------------------------
// CAspProfileWizard::DeleteProfile
//
//------------------------------------------------------------------------------
//
void CAspProfileWizard::DeleteProfile()
	{
	FLOG( _L("CAspProfileWizard::DeleteProfile START") );
	
	if (iProfileId != KErrNotFound)
		{
		TRAPD(err, iSyncSession->DeleteProfileL(iProfileId));
		iProfileId = KErrNotFound;
		
		if (err != KErrNone)
			{
			FLOG( _L("### RSyncMLSession::DeleteProfileL failed (%d) ###"), err );
			}
		}
		
	FLOG( _L("CAspProfileWizard::DeleteProfile END") );
    }


//------------------------------------------------------------------------------
// CAspProfileWizard::CreateProfileL
//
//------------------------------------------------------------------------------
//
void CAspProfileWizard::CreateProfileL()
	{
	FLOG( _L("CAspProfileWizard::CreateProfileL START") );
	
	TAspParam param(iApplicationId, iSyncSession);
	CAspProfile* profile = CAspProfile::NewLC(param);
	
	iProfileId = KErrNotFound;
	profile->CreateL(CAspProfile::EAllProperties);
	iProfileId = profile->ProfileId();
	
	CAspProfile::SetDefaultValuesL(profile);
	
	GetStringValue(iBuf, CAspWizardItem::ETypeProfileName);
	profile->SetNameL(iBuf);
    
	TInt protocol = NumberValue(CAspWizardItem::ETypeProtocolVersion);
	profile->SetProtocolVersionL(protocol);

	TInt bearerType = NumberValue(CAspWizardItem::ETypeBearerType);
	profile->SetBearerTypeL(bearerType);
	
	GetStringValue(iBuf, CAspWizardItem::ETypeHostAddress);
	TURIParser parser(iBuf);
	TInt portNumber = parser.Port();
	if (!parser.IsValidPort(portNumber))
		{
		portNumber = parser.DefaultPort();
		}
    profile->SetHostAddressL(iBuf, portNumber);
    
    GetStringValue(iBuf, CAspWizardItem::ETypeServerId);
    profile->SetServerIdL(iBuf);
	
	GetStringValue(iBuf, CAspWizardItem::ETypeUserName);
	profile->SetUserNameL(iBuf);
	
	GetStringValue(iBuf, CAspWizardItem::ETypePassword);
	profile->SetPasswordL(iBuf);
	
	profile->SaveL();
	iContentList->SetProfile(profile);
	
	TInt count = iSettingList.Count();
	for (TInt i=0; i<count; i++)
		{
		CAspWizardItem* item = iSettingList[i];
		if (item->iSettingType == CAspWizardItem::ETypeSyncContent)
			{
			if (item->iEnabled)
				{
				TBuf<KBufSize> localDb;
				GetStringValue(iBuf, CAspWizardItem::ETypeSyncContent,
				               item->iSettingId);
				
				GetStringValue(localDb, CAspWizardItem::ETypeSyncContentLocalDatabase,
				               item->iSettingId);
				               
				iContentList->CreateTask(item->iSettingId,
				              localDb, iBuf, ETrue, ESyncDirectionTwoWay);
				}
			}
		}
	
	iContentList->SetProfile(NULL);
	CleanupStack::PopAndDestroy(profile);
	
	FLOG( _L("CAspProfileWizard::CreateProfileL END") );
    }


//------------------------------------------------------------------------------
// CAspProfileWizard::SetStringValueL
//
//------------------------------------------------------------------------------
//
void CAspProfileWizard::SetStringValueL(const TDesC& aValue,
                                        TInt aSettingType, TInt aSettingId)
	{
	TInt count = iSettingList.Count();
	for (TInt i=0; i<count; i++)
		{
		CAspWizardItem* item = iSettingList[i];
		if (item->iSettingType == aSettingType && item->iSettingId == aSettingId)
			{
			item->SetValueL(aValue);
			break;
			}
		}
	}


//------------------------------------------------------------------------------
// CAspProfileWizard::SetStringValueL
//
//------------------------------------------------------------------------------
//
void CAspProfileWizard::SetStringValueL(const TDesC& aValue, TInt aSettingType)
	{
	TInt count = iSettingList.Count();
	for (TInt i=0; i<count; i++)
		{
		CAspWizardItem* item = iSettingList[i];
		if (item->iSettingType == aSettingType)
			{
			item->SetValueL(aValue);
			break;
			}
		}
	}


//------------------------------------------------------------------------------
// CAspProfileWizard::GetStringValue
//
//------------------------------------------------------------------------------
//
void CAspProfileWizard::GetStringValue(TDes& aValue, TInt aSettingType)
	{
	aValue = KNullDesC;
	
	TInt count = iSettingList.Count();
	for (TInt i=0; i<count; i++)
		{
		CAspWizardItem* item = iSettingList[i];
		if (item->iSettingType == aSettingType)
			{
			TUtil::StrCopy(aValue, item->Value());
			break;
			}
		}
	}


//------------------------------------------------------------------------------
// CAspProfileWizard::GetStringValue
//
//------------------------------------------------------------------------------
//
void CAspProfileWizard::GetStringValue(TDes& aValue, TInt aSettingType, TInt aSettingId)
	{
	aValue = KNullDesC;
	
	TInt count = iSettingList.Count();
	for (TInt i=0; i<count; i++)
		{
		CAspWizardItem* item = iSettingList[i];
		if (item->iSettingType == aSettingType && item->iSettingId == aSettingId)
			{
			TUtil::StrCopy(aValue, item->Value());
			break;
			}
		}
	}


//------------------------------------------------------------------------------
// CAspProfileWizard::GetNumberValue
//
//------------------------------------------------------------------------------
//
void CAspProfileWizard::GetNumberValue(TInt& aValue, TInt aSettingType)
	{
	aValue = KErrNotFound;
	
	TInt count = iSettingList.Count();
	for (TInt i=0; i<count; i++)
		{
		CAspWizardItem* item = iSettingList[i];
		if (item->iSettingType == aSettingType)
			{
			aValue = item->iNumber;
			break;
			}
		}
	}


//------------------------------------------------------------------------------
// CAspProfileWizard::NumberValue
//
//------------------------------------------------------------------------------
//
TInt CAspProfileWizard::NumberValue(TInt aSettingType)
	{
	TInt num = KErrNotFound;
	GetNumberValue(num, aSettingType);
	return num;
	}


//------------------------------------------------------------------------------
// CAspProfileWizard::SetNumberValue
//
//------------------------------------------------------------------------------
//
void CAspProfileWizard::SetNumberValue(TInt& aValue, TInt aSettingType)
	{
	TInt count = iSettingList.Count();
	for (TInt i=0; i<count; i++)
		{
		CAspWizardItem* item = iSettingList[i];
		if (item->iSettingType == aSettingType)
			{
			item->iNumber = aValue;
			break;
			}
		}
	}
	
	
//------------------------------------------------------------------------------
// CAspProfileWizard::NumOfEnabledContentItems
//
//------------------------------------------------------------------------------
//
TInt CAspProfileWizard::NumOfEnabledContentItems()
	{
	TInt total = 0;

	TInt count = iSettingList.Count();
	for (TInt i=0; i<count; i++)
		{
		CAspWizardItem* item = iSettingList[i];
		if (item->iSettingType == CAspWizardItem::ETypeSyncContent)
			{
			if (item->iEnabled)
				{
				total++;
				}
			}
		}
		
	return total;
	}


//------------------------------------------------------------------------------
// CAspProfileWizard::NumOfContentItems
//
//------------------------------------------------------------------------------
//
TInt CAspProfileWizard::NumOfContentItems()
	{
	TInt total = 0;

	TInt count = iSettingList.Count();
	for (TInt i=0; i<count; i++)
		{
		CAspWizardItem* item = iSettingList[i];
		if (item->iSettingType == CAspWizardItem::ETypeSyncContent)
			{
			total++;
			}
		}
		
	return total;
	}


//------------------------------------------------------------------------------
// CAspProfileWizard::NumOfEnabledItems
//
//------------------------------------------------------------------------------
//
TInt CAspProfileWizard::NumOfEnabledItems()
	{
	TInt total = 0;

	TInt count = iSettingList.Count();
	for (TInt i=0; i<count; i++)
		{
		CAspWizardItem* item = iSettingList[i];
		if (item->iEnabled)
			{
			total++;
			}
		}
		
	return total;
	}


//------------------------------------------------------------------------------
// CAspProfileWizard::NumOfCurrentItem
//
//------------------------------------------------------------------------------
//
TInt CAspProfileWizard::NumOfCurrentItem(CAspWizardItem* aItem)
	{
	// find position of aItem in the list of all enabled items
	TInt current = 0;

	TInt count = iSettingList.Count();
	for (TInt i=0; i<count; i++)
		{
		CAspWizardItem* item = iSettingList[i];
		
		if (!item->iEnabled)
			{
			continue;
			}
		
		current++;
				
		if (item->iSettingType == aItem->iSettingType)
			{
			if (item->iSettingType != CAspWizardItem::ETypeSyncContent)
				{
				break;
				}
				
		    if (item->iSettingId == aItem->iSettingId)
		    	{
		    	break;
		    	}
			}
		}
		
	return current;
	}


//------------------------------------------------------------------------------
// CAspProfileWizard::CurrentItem
//
//------------------------------------------------------------------------------
//
CAspWizardItem* CAspProfileWizard::CurrentItem()
	{
	TInt count = iSettingList.Count();
	
	__ASSERT_ALWAYS(iCurrentItemIndex >= 0, TUtil::Panic(KErrGeneral));
	__ASSERT_ALWAYS(iCurrentItemIndex < count, TUtil::Panic(KErrGeneral));
	
	return iSettingList[iCurrentItemIndex];
	}
	

//------------------------------------------------------------------------------
// CAspProfileWizard::CurrentItemType
//
//------------------------------------------------------------------------------
//
TInt CAspProfileWizard::CurrentItemType()
	{
	CAspWizardItem* item = CurrentItem();
	
	return  item->iSettingType;
	}


//------------------------------------------------------------------------------
// CAspProfileWizard::CreatedProfileId
//
//------------------------------------------------------------------------------
//
TInt CAspProfileWizard::CreatedProfileId()
	{
	return iProfileId;
	}


//------------------------------------------------------------------------------
// CAspProfileWizard::MoveToNext
//
//------------------------------------------------------------------------------
//
void CAspProfileWizard::MoveToNext()
	{
	iCurrentItemIndex = IncreaseCurrentIndex(iCurrentItemIndex);
	}


//------------------------------------------------------------------------------
// CAspProfileWizard::MoveToPrevious
//
//------------------------------------------------------------------------------
//
void CAspProfileWizard::MoveToPrevious()
	{
	iCurrentItemIndex = DecreaseCurrentIndex(iCurrentItemIndex);
	}


//------------------------------------------------------------------------------
// CAspProfileWizard::MoveToStart
//
//------------------------------------------------------------------------------
//
void CAspProfileWizard::MoveToStart()
	{
	iCurrentItemIndex = 0;
	}


// -----------------------------------------------------------------------------
// CAspProfileWizard::IncreaseCurrentIndex
//
// -----------------------------------------------------------------------------
//
TInt CAspProfileWizard::IncreaseCurrentIndex(const TInt aIndex)
	{
	TInt maxIndex = iSettingList.Count() - 1;
	TInt index = aIndex;
	
	for (;;)
		{
		__ASSERT_DEBUG(index < maxIndex, TUtil::Panic(KErrGeneral));

		if (!(index < maxIndex))
			{
			return maxIndex;
			}
				
		index++;
				
	    CAspWizardItem* item = iSettingList[index];
	    if (item->iEnabled)
	    	{
	    	break;
	    	}
		}
		
	return index;
	}


// -----------------------------------------------------------------------------
// CAspProfileWizard::DecreaseCurrentIndex
//
// -----------------------------------------------------------------------------
//
TInt CAspProfileWizard::DecreaseCurrentIndex(const TInt aIndex)
	{
	TInt index = aIndex;
	
	for (;;)
		{
		__ASSERT_DEBUG(index > 0, TUtil::Panic(KErrGeneral));
		
		if (!(index > 0))
			{
			return 0;
			}
			
		index--;
		
	    CAspWizardItem* item = iSettingList[index];
	    if (item->iEnabled)
	    	{
	    	break;
	    	}
		}
		
	return index;
	}


// -----------------------------------------------------------------------------
// CAspProfileWizard::LocalDatabaseList
//
// -----------------------------------------------------------------------------
//
CDesCArray* CAspProfileWizard::LocalDatabaseList(const TInt aDataproviderId)
	{
	__ASSERT_DEBUG(aDataproviderId == KUidNSmlAdapterEMail.iUid, 
	               TUtil::Panic(KErrGeneral));
	
	return iLocalDataStores;
	}
	





//  End of File  
