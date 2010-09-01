/*
* Copyright (c) 2007 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:  Implements the default plugin
*
*/


// INCLUDES
#include "AspPreSyncDefaultPlugin.h"

#include <ConeResLoader.h> 
#include <bautils.h>
#include <eikenv.h>
#include <AknGlobalNote.h>
#include <StringLoader.h>
#include <avkon.rsg>
#include <avkon.hrh>
#include <AknGlobalMsgQuery.h>
#include <rconnmon.h>


#include <AknQueryDialog.h>
#include <AspSyncUtil.rsg>
#include <StringLoader.h> 
#include <aspdefines.h> //For enum TRoamingSettings
#include <centralrepository.h> //For central Repository

// ============================== MEMBER FUNCTIONS ============================

_LIT(KDC_RESOURCE_FILES_DIR,"\\resource\\");
_LIT(KResourceFile,        "z:AspSyncUtil.rsc");


// -------------------------------------------------------------------------------
// CRoamingCheck::NewL
//
// 
// -------------------------------------------------------------------------------
//
CRoamingCheck* CRoamingCheck::NewL()
    {
    CRoamingCheck* self = CRoamingCheck::NewLC();
    CleanupStack::Pop(self);
    return self;
    }

// -------------------------------------------------------------------------------
// CRoamingCheck::NewLC
//
// 
// -------------------------------------------------------------------------------
//
CRoamingCheck* CRoamingCheck::NewLC()
    {
    CRoamingCheck* self = new (ELeave) CRoamingCheck(CEikonEnv::Static());
    CleanupStack::PushL(self);
    self->ConstructL();
    return self;
    }

// -------------------------------------------------------------------------------
// CRoamingCheck::CRoamingCheck
//
// 
// -------------------------------------------------------------------------------
//
CRoamingCheck::CRoamingCheck(CEikonEnv* aEikEnv) 
        : iEnv(aEikEnv),iResLoader (*aEikEnv)
	{
	}

// -------------------------------------------------------------------------------
// CRoamingCheck::~CRoamingCheck
//
// 
// -------------------------------------------------------------------------------
//
CRoamingCheck::~CRoamingCheck()
	{    
    iResLoader.Close();
	delete iGlobalNote;
	
	}

// -------------------------------------------------------------------------------
// CRoamingCheck::ConstructL
//
// 
// -------------------------------------------------------------------------------
//
void CRoamingCheck::ConstructL()
    {    
    iGlobalNote = CAknGlobalMsgQuery::NewL();
    
    // Resolve resource file path
    TFileName fileName(0);
    TParse parse;
    parse.Set(KResourceFile,&KDC_RESOURCE_FILES_DIR,NULL);
    fileName.Append( parse.FullName() );
    // Get language of resource file
    BaflUtils::NearestLanguageFile(iEnv->FsSession(), fileName );
    // Open resource file
    TInt err = iResLoader.Open( fileName );            
  	}
    
// -------------------------------------------------------------------------------
// CRoamingCheck::ShowGlobalNoteL
//
// 
// -------------------------------------------------------------------------------
//
void CRoamingCheck::ShowGlobalNoteL(TInt aSoftKeyId, 
                  const HBufC* aMessage, TInt& status)
	{
    iGlobalNote->CancelMsgQuery();
	TRequestStatus GlobalNotestatus = KErrNone;
	iGlobalNote->ShowMsgQueryL( GlobalNotestatus, *aMessage, aSoftKeyId, KNullDesC , KNullDesC ); 
	User::WaitForRequest(GlobalNotestatus);
	iGlobalNote->CancelMsgQuery();
	status=GlobalNotestatus.Int();
	}


// ----------------------------------------------------------------------------
// CPreSyncDefaultPlugin::NewL
// Two Phase Construction
// ----------------------------------------------------------------------------
CPreSyncDefaultPlugin* CPreSyncDefaultPlugin::NewL()
{
	CPreSyncDefaultPlugin* self = new ( ELeave ) CPreSyncDefaultPlugin();
	CleanupStack::PushL( self );
	self->ConstructL();
	CleanupStack::Pop( self );

	return self;
}

// ----------------------------------------------------------------------------
// CPreSyncDefaultPlugin::CPreSyncDefaultPlugin
// Two Phase Construction
// ----------------------------------------------------------------------------
CPreSyncDefaultPlugin::CPreSyncDefaultPlugin()
{		
}

// ----------------------------------------------------------------------------
// CPreSyncDefaultPlugin::ConstructL
// Two Phase Construction
// ----------------------------------------------------------------------------
void CPreSyncDefaultPlugin::ConstructL()
{
} 
	
	
// ----------------------------------------------------------------------------
// CPreSyncDefaultPlugin::~CPreSyncDefaultPlugin
// Destructor
// ----------------------------------------------------------------------------
CPreSyncDefaultPlugin::~CPreSyncDefaultPlugin()
{
}


// -------------------------------------------------------------------------------
// CPreSyncDefaultPlugin::IsSupported
//
// 
// -------------------------------------------------------------------------------
//
TBool CPreSyncDefaultPlugin::IsSupported()
    {
    return ETrue;
    }

// -------------------------------------------------------------------------------
// CPreSyncDefaultPlugin::CanSyncL
//
// 
// -------------------------------------------------------------------------------
//
TBool CPreSyncDefaultPlugin::CanSyncL()
    {
    return ETrue;
    /*
    TBool ret = ETrue;
    TInt value(0);
    
    ReadRepositoryL(KNSmlRoamingSettingKey, value);    
    CRoamingCheck* roamingCheck = CRoamingCheck::NewL();
    CleanupStack::PushL(roamingCheck);
    
	HBufC* hBuf = NULL;
	CAknQueryDialog* queryDialog = NULL;
    
    switch(value)
        {
        case ERoamingSettingAlwaysAsk:
			hBuf = StringLoader::LoadLC(R_ASP_NOTE_NETWORK_CONFIRMATION);
			queryDialog = new (ELeave) CAknQueryDialog;
			ret = queryDialog->ExecuteLD(R_ASP_ROAMING_CONFIRMATION_QUERY, hBuf->Des());
			if(ret)
				{
				ret = ETrue;
				}
			else
				{
				ret = EFalse;
				}
			CleanupStack::PopAndDestroy(hBuf);
            break;
        case ERoamingSettingAlwaysAllow:
            break;
        case ERoamingSettingNeverAllow:
     		hBuf = StringLoader::LoadLC(R_ASP_NOTE_NETWORK_NOT_ALLOWED);
			queryDialog = new (ELeave) CAknQueryDialog;
			ret = queryDialog->ExecuteLD(R_ASP_INFORMATION_QUERY, hBuf->Des());
			CleanupStack::PopAndDestroy(hBuf);

            ret = EFalse;
            break;
        case ERoamingSettingBlock:
			hBuf = StringLoader::LoadLC(R_ASP_NOTE_NOT_HOME_NETWORK);
			queryDialog = new (ELeave) CAknQueryDialog;
			ret = queryDialog->ExecuteLD(R_ASP_INFORMATION_QUERY, hBuf->Des());
			CleanupStack::PopAndDestroy(hBuf);

            ret = EFalse;
            break;
        default:
            break;
        }
    
    CleanupStack::PopAndDestroy(roamingCheck);
    return ret;
    */
    }

// -------------------------------------------------------------------------------
// CPreSyncDefaultPlugin::ReadRepositoryL
//
// 
// -------------------------------------------------------------------------------
//
void CPreSyncDefaultPlugin::ReadRepositoryL(TInt aKey, TInt& aValue)
    {
    CRepository* rep = CRepository::NewLC(KCRUidNSmlDSApp);
    TInt err = rep->Get(aKey, aValue);
    User::LeaveIfError(err);
    
    CleanupStack::PopAndDestroy(rep);
    }


// -------------------------------------------------------------------------------
// CPreSyncDefaultPlugin::SetProfile
//
// 
// -------------------------------------------------------------------------------
//
TBool CPreSyncDefaultPlugin::SetProfile(const TSmlProfileId& aProfileId)
    {
    iSyncProfileId = aProfileId;
    return ETrue;
    }

// -------------------------------------------------------------------------------
// CPreSyncDefaultPlugin::SyncNow
//
// 
// -------------------------------------------------------------------------------
//
TInt CPreSyncDefaultPlugin::SyncNow(MSyncExtObserver* aObserver)
    {
    TInt retValue = 1;
    return retValue;
    }


