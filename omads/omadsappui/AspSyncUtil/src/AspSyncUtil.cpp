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



// INCLUDES
#include "AspSyncUtil.h"
#include "AspDialogUtil.h"
#include "AspProfileDialog.h"
#include "AspProfileWizardDialog.h"
#include "AspAutoSyncHandler.h"

#include "AspSyncUtil.rh"
#include "AspDebug.h"

#include <AiwCommon.hrh> // for application ids 
#include <DataSyncInternalPSKeys.h>

#ifdef RD_DSUI_TIMEDSYNC 
#include "AspSettingViewDialog.h"
#endif
// ============================ MEMBER FUNCTIONS ===============================



// -----------------------------------------------------------------------------
// CAspSyncUtil::NewL
//
// -----------------------------------------------------------------------------
//
CAspSyncUtil* CAspSyncUtil::NewL()
    {
	FLOG( _L("CAspSyncUtil::NewL START") );

    CAspSyncUtil* self = new (ELeave) CAspSyncUtil();
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);

	FLOG( _L("CAspSyncUtil::NewL END") );
    return self;
    }


// -----------------------------------------------------------------------------
// Destructor
//
// -----------------------------------------------------------------------------
//
CAspSyncUtil::~CAspSyncUtil()
    {
   	FLOG( _L("CAspSyncUtil::~CAspSyncUtil START") );
   	
	delete iResHandler;
	delete iSyncHandler;
 	delete iContentList;
	
	CloseSyncSession();
	
	FLOG( _L("CAspSyncUtil::~CAspSyncUtil END") );
    }


// -----------------------------------------------------------------------------
// CAspSyncUtil::ConstructL
//
// -----------------------------------------------------------------------------
//
void CAspSyncUtil::ConstructL(void)
    {
   	FLOG( _L("CAspSyncUtil::ConstructL START") );
 	
    iSettingEnforcement = TUtil::SettingEnforcementState();
   	    
   	TAspParam param(iApplicationId, NULL);  // param not used in CAspSyncHandler
	iSyncHandler = CAspSyncHandler::NewL(param);

	iSyncSessionOpen = EFalse;
	
   	FLOG( _L("CAspSyncUtil::ConstructL END") );
    }


// -----------------------------------------------------------------------------
// CAspSyncUtil::CAspSyncUtil
//
// -----------------------------------------------------------------------------
//
CAspSyncUtil::CAspSyncUtil()
	{
	}


// -----------------------------------------------------------------------------
// CAspSyncUtil::SynchronizeL
//
// Called by CSyncUtilApi::SynchronizeL.
// -----------------------------------------------------------------------------
//
void CAspSyncUtil::SynchronizeL(RFile& aTaskFile)
	{
	CAspAutoSyncHandler* autoSyncHandler = CAspAutoSyncHandler::NewL();
	CleanupStack::PushL(autoSyncHandler);
	
	autoSyncHandler->SynchronizeL(aTaskFile);
	
	CleanupStack::PopAndDestroy(autoSyncHandler);
	}


// -----------------------------------------------------------------------------
// CAspSyncUtil::SynchronizeL
//
// Called by CSyncUtilApi::SynchronizeL. 
// -----------------------------------------------------------------------------
//
void CAspSyncUtil::SynchronizeL(TInt aApplicationId, TInt aJobId, TInt aInfo1, TDes& aInfo2)
	{
	FLOG( _L("CAspSyncUtil::SynchronizeL START") );
	
	if (!iResHandler)
		{
		iResHandler = CAspResHandler::NewL();
		}
		
	iApplicationId = aApplicationId;
	
	OpenSyncSessionL();
	
	TRAPD(err, DoSynchronizeL(aApplicationId, aJobId, aInfo1, aInfo2));
	
	CloseSyncSession();
	
	User::LeaveIfError(err);
		
	FLOG( _L("CAspSyncUtil::SynchronizeL END") );
	}


// -----------------------------------------------------------------------------
// CAspSyncUtil::ShowSettingsL
//
// Called by CSyncUtilApi::ShowSettingsL.
// -----------------------------------------------------------------------------
//
void CAspSyncUtil::ShowSettingsL(TInt aApplicationId, TInt aInfo1, TDes& /*aInfo2*/)
	{
    FLOG( _L("CAspSyncUtil::ShowSettingsL START") );

	if (!iResHandler)
		{
		iResHandler = CAspResHandler::NewL();
		}

    iApplicationId = aApplicationId;
    
    if (aApplicationId != EApplicationIdSync)
    	{
    	// other apps only show profile dialog
    	ShowProfileDialogL(aApplicationId);
    	return;
    	}
    	
    
    TInt dialogMode = EModeDialogWait;
    if (aInfo1 == EModeDialogNoWait)
    	{
    	dialogMode = EModeDialogNoWait;
    	}
    
    OpenSyncSessionL();
        
    TRAPD(err, ShowSettingsDialogL(aApplicationId, KErrNotFound, dialogMode));
            
    if (dialogMode == EModeDialogWait)
    	{
    	iSettingDialog = NULL;
    	CloseSyncSession();
    	}
    
    User::LeaveIfError(err);
    
    FLOG( _L("CAspSyncUtil::ShowSettingsL END") );
	}


// -----------------------------------------------------------------------------
// CAspSyncUtil::ShowSettingsDialog
//
// -----------------------------------------------------------------------------
//
void CAspSyncUtil::ShowSettingsDialogL(TInt aApplicationId, 
                                       TInt aProfileId, TInt aDialogMode)
	{
	if (aApplicationId != EApplicationIdSync)
		{
		CreateContentListL();
	    CheckAppAndProviderL(aApplicationId);
		}
	
	TAspParam param(aApplicationId, &iSyncSession);
	param.iMode = aDialogMode;
	param.iProfileId = aProfileId;

    iSettingDialog = CAspSettingDialog::NewL(param);
 
    if (aDialogMode == EModeDialogWait)
    	{
    	iSettingDialog->ExecuteLD(R_ASP_SETTING_DIALOG);
    	}
    else
    	{
    	iSettingDialog->ExecuteLD(R_ASP_SETTING_DIALOG_2);
    	}
	}


// -----------------------------------------------------------------------------
// CAspSyncUtil::ShowProfileDialog
//
// -----------------------------------------------------------------------------
//
void CAspSyncUtil::ShowProfileDialogL(TInt aApplicationId)
	{
    FLOG( _L("CAspSyncUtil::ShowProfileDialogL START") );
    
	OpenSyncSessionL();
	
	TRAPD(err, EditProfileL(aApplicationId));
	
	CloseSyncSession();
		
    User::LeaveIfError(err);
		
	FLOG( _L("CAspSyncUtil::ShowProfileDialogL END") );
	}


// -----------------------------------------------------------------------------
// CAspSyncUtil::EditProfileL
//
// -----------------------------------------------------------------------------
//
void CAspSyncUtil::EditProfileL(TInt aAplicationId)
	{
	CreateContentListL();
    CheckAppAndProviderL(aAplicationId);
	
	TInt profileId = KErrNotFound;
	TInt taskId = KErrNotFound;
	TInt resourceId = R_ASP_PROFILE_QUERY_TITLE;
	
	TBool ret = SelectProfileL(profileId, taskId, resourceId, KNullDesC,ETrue);
	if (!ret || profileId == KErrNotFound)
		{
		return;  // no sync profile
		}

    if (iSettingEnforcement)
    	{
    	DoEditReadOnlyProfileL(aAplicationId, profileId, 
    	                       EDialogModeSettingEnforcement);
    	}
    else
    	{
    	DoEditProfileL(aAplicationId, profileId, EDialogModeEdit);
    	}
	}


// -----------------------------------------------------------------------------
// CAspSyncUtil::DoEditProfileL
//
// -----------------------------------------------------------------------------
//
void CAspSyncUtil::DoEditProfileL(TInt aApplicationId, TInt aProfileId,
                                                      TInt aDialogMode)
	{
	TAspParam param(EApplicationIdSync, &iSyncSession);
	CAspProfileList* profileList = CAspProfileList::NewLC(param);
	profileList->ReadAllProfilesL(CAspProfileList::ENoMandatoryCheck);
	
	
    param.iApplicationId = aApplicationId;
	CAspProfile* profile = CAspProfile::NewLC(param);
    
    TInt err = KErrNone;
    TRAP(err, profile->OpenL(aProfileId, CAspProfile::EOpenReadWrite,
                                         CAspProfile::EAllProperties));

	if (err == KErrLocked)
		{
		TDialogUtil::ShowErrorNoteL(R_ASP_LOCKED_PROFILE);
		}
	User::LeaveIfError(err);

	iContentList->SetProfile(profile);
	TRAP(err, iContentList->InitAllTasksL());
	
	if (err != KErrNone)
		{
		iContentList->SetProfile(NULL);
		}
	User::LeaveIfError(err);
		
	TAspParam param2(aApplicationId, &iSyncSession);
	param2.iProfile = profile;
	param2.iMode = aDialogMode;
    param2.iContentList = iContentList;
    param2.iProfileList = profileList;  // needed for unique server id check
 
#ifdef RD_DSUI_TIMEDSYNC 
	CAspSettingViewDialog::ShowDialogL(param2);
#else
	TRAP(err, CAspProfileDialog::ShowDialogL(param2));
#endif
	
	iContentList->SetProfile(NULL);
	User::LeaveIfError(err);

	CleanupStack::PopAndDestroy(profile);
	CleanupStack::PopAndDestroy(profileList);
	}


// -----------------------------------------------------------------------------
// CAspSyncUtil::DoEditReadOnlyProfileL
//
// -----------------------------------------------------------------------------
//
void CAspSyncUtil::DoEditReadOnlyProfileL(TInt aApplicationId, TInt aProfileId,
                                                      TInt aDialogMode)
	{
	TAspParam param(aApplicationId, &iSyncSession);
	CAspProfile* profile = CAspProfile::NewLC(param);
    
    profile->OpenL(aProfileId, CAspProfile::EOpenRead,
                                         CAspProfile::EAllProperties);

	iContentList->SetProfile(profile);
	TInt err = KErrNone;
	TRAP(err, iContentList->InitAllTasksL());
	
	if (err != KErrNone)
		{
		iContentList->SetProfile(NULL);
		}
	User::LeaveIfError(err);
		
	TAspParam param2(aApplicationId, &iSyncSession);
	param2.iProfile = profile;
	param2.iMode = aDialogMode;
    param2.iContentList = iContentList;
 
	TRAP(err, CAspProfileDialog::ShowDialogL(param2));
	
	iContentList->SetProfile(NULL);
	User::LeaveIfError(err);

	CleanupStack::PopAndDestroy(profile);
	}


// -----------------------------------------------------------------------------------------------------
// CAspSyncUtil::SelectProfileL
// aCheckId currently will be always set to ETrue
// this Check can be used in future to differentiate if its called from DoSynchronizeL() or EditProfileL()
// ------------------------------------------------------------------------------------------------------
//
TBool CAspSyncUtil::SelectProfileL(TInt& aProfileId, TInt& aTaskId, 
                                   TInt aResourceId, const TDesC& aLocalDatabase,TBool aCheckId)
	{
	aProfileId = KErrNotFound;
	aTaskId = KErrNotFound;
	TBool ret = EFalse;
	TInt index=KErrNotFound;
	
	TAspParam param(iApplicationId, &iSyncSession);
	CAspProfileList* profileList = CAspProfileList::NewLC(param);
	
	if (aLocalDatabase.Length() > 0)
		{
		profileList->ReadEMailProfilesL(aLocalDatabase, KErrNotFound);
		if(profileList->Count() == 0)
		{
		 profileList->ReadAllProfilesL(CAspProfileList::EBasePropertiesOnly);
		}
		}
	else
		{
		profileList->ReadAllProfilesL(CAspProfileList::EBasePropertiesOnly);
		}
	
	profileList->Sort();
   	
	if (profileList->Count() == 0)
		{
	    if (!iSettingEnforcement)
	    	{
	    	if (TDialogUtil::ShowConfirmationQueryL(R_ASP_QUERY_NO_PROFILES))
		        {
	    	    TAspParam param(iApplicationId, &iSyncSession);
         	    CAspProfileWizardDialog::ShowDialogL(param);
    	        }
	    	}

    	ret = EFalse;
		}
		
	else if (profileList->Count() == 1)
		{
		TAspProfileItem& item = profileList->Item(0);
		aProfileId = item.iProfileId;
		aTaskId = item.iTaskId;
       	ret = ETrue;
		}
		
	else
		{
		if(aCheckId)
		{
			
	
			if (iSettingEnforcement)
				{
					ret = TDialogUtil::ShowProfileQueryL(profileList, aProfileId, aResourceId);
						
				}
			else
				{
		  			ret = TDialogUtil::ShowProfileQueryAppL(profileList, aProfileId, aResourceId,index);	
				}
		
		
			if (ret)
				{
		   			 index = profileList->ListIndex(aProfileId);
	    	
	        		__ASSERT_DEBUG(index != KErrNotFound, TUtil::Panic(KErrGeneral));
	              
	         		TAspProfileItem& item = profileList->Item(index);
	         		
		   			aTaskId = item.iTaskId;
	       
				}
			
		 	else
	 			 {
	 		    		if(index==1)
			     		CAspProfileWizardDialog::ShowDialogL(param);
	        	 		//ret=EFalse;
		 	
		 		 }	
		}
		
		else
		{
		
		   
			ret = TDialogUtil::ShowProfileQueryL(profileList, aProfileId, aResourceId);	
		  	
	 		if (ret)
				{
					TInt index = profileList->ListIndex(aProfileId);
	    	
	        		__ASSERT_DEBUG(index != KErrNotFound, TUtil::Panic(KErrGeneral));
	                
	          		TAspProfileItem& item = profileList->Item(index);
		      		aTaskId = item.iTaskId;
	        
			
		 
		 		}
				
		
	   		}
		}
		
	CleanupStack::PopAndDestroy(profileList);

	return ret;
	}


// -----------------------------------------------------------------------------
// CAspSyncUtil::DoSynchronizeL
//
// -----------------------------------------------------------------------------
//
void CAspSyncUtil::DoSynchronizeL(TInt aApplicationId, TInt /*aJobId*/, 
                                  TInt aInfo1, TDes& aInfo2)
	{
	FLOG( _L("CAspSyncUtil::DoSynchronizeL START") );
	
	CreateContentListL();
	CheckAppAndProviderL(aApplicationId);

	//check PS key
    TInt keyVal = 0;//EDataSyncNotRunning
    TInt err = RProperty::Get( KPSUidDataSynchronizationInternalKeys,
                                 KDataSyncStatus,
                                 keyVal );
    if(err == KErrNone && keyVal)
    	{
    	TDialogUtil::ShowInformationNoteL(R_ASP_SYNC_ALREADY_RUNNING);
		return;
    	}
	
	if (CAspProfile::OtherSyncRunning(&iSyncSession))
		{
		TDialogUtil::ShowInformationNoteL(R_ASP_SYNC_ALREADY_RUNNING);
		FLOG( _L("CAspSyncUtil::DoSynchronizeL END") );
   		return;
		}
		
	if (iApplicationId == EApplicationIdEmail && TUtil::IsEmpty(aInfo2))
		{
		FLOG( _L("### mailbox name not defined ###") );
		FLOG( _L("CAspSyncUtil::DoSynchronizeL END") );
		User::Leave(KErrNotFound);
		}
	

	TInt profileId = KErrNotFound;
	TInt taskId = KErrNotFound;
	TBool ret = EFalse;
	TInt resourceId = R_ASP_PROFILE_QUERY_TITLE;

	if (aApplicationId == EApplicationIdEmail)
		{
		ret = SelectProfileL(profileId, taskId, resourceId, aInfo2,ETrue);
		}
	else
		{
		ret = SelectProfileL(profileId, taskId, resourceId, KNullDesC,ETrue);
		}
		
	if (!ret || profileId == KErrNotFound)
		{
		FLOG( _L("CAspSyncUtil::DoSynchronizeL END") );
		return; // no sync profile
		}
	
	TAspParam aspParam(iApplicationId, &iSyncSession);
	CAspProfile* profile = CAspProfile::NewLC(aspParam);
	TRAPD (err1, profile->OpenL(profileId, CAspProfile::EOpenRead, 
	                           CAspProfile::EAllProperties));
	User::LeaveIfError(err1);
	if (aApplicationId != EApplicationIdEmail)
		{
		aInfo2 = KNullDesC;
		}
	//create task for content type ,if it is not created yet
	ret = CheckTaskAndProtocolL(profileId, aInfo2);
	
	TInt dataProviderId = TUtil::ProviderIdFromAppId(aApplicationId);
	taskId = TAspTask::FindTaskIdL(profile, dataProviderId, aInfo2);
	
	CleanupStack::PopAndDestroy(profile);
	    		
	if (!ret)
		{
		    FLOG( _L("CAspSyncUtil::DoSynchronizeL END") );
			return; // some mandatory data missing
		}
	
	if (!CheckMandatoryL(profileId))
		{
	    FLOG( _L("CAspSyncUtil::DoSynchronizeL END") );
		return; // some mandatory data missing
		}

#ifdef _DEBUG
        TBuf<KBufSize> buf;
        TUtil::GetApplicationName(buf, aApplicationId);
        FTRACE(RDebug::Print(_L("statrting sync (%S)"), &buf));
#endif

	TAspSyncParam param(aApplicationId);
	param.iProfileId = profileId;
	param.iTaskId = taskId;

    if (aInfo1 == EApplicationIdEmail)
    	{
    	// email sync 3.2 needs synchronous syncing
    	param.iDialogMode = EModeDialogWait;
    	}

	iSyncHandler->SynchronizeL(param);

	FLOG( _L("CAspSyncUtil::DoSynchronizeL END") );
	}


// -----------------------------------------------------------------------------
// CAspSyncUtil::CheckMandatoryL
//
// -----------------------------------------------------------------------------
//
TBool CAspSyncUtil::CheckMandatoryL(TInt aProfileId)
	{
	TAspParam param(iApplicationId, &iSyncSession);
	CAspProfile* profile = CAspProfile::NewLC(param);
	profile->OpenL(aProfileId, CAspProfile::EOpenRead, 
	                           CAspProfile::EAllProperties);
	
	TInt contentCount = 0;
	TInt ret = CAspProfile::CheckMandatoryData(profile, contentCount);
	CleanupStack::PopAndDestroy(profile);
	
	if (ret != EMandatoryOk)
		{
        if (iSettingEnforcement)
    	    {
    	    TDialogUtil::ShowInformationNoteL(R_ASP_NOTE_TARM_MANDATORY_MISSING);
    	    }    	
    	else if (TDialogUtil::ShowConfirmationQueryL(R_ASP_QUERY_MANDATORY_MISSING))
			{
   	        DoEditProfileL(iApplicationId, aProfileId, EDialogModeEditMandatory);
 			}
		
		return EFalse;
    	}
	
	return ETrue;
	}


// -----------------------------------------------------------------------------
// CAspSyncUtil::DataProviderExist
//
// -----------------------------------------------------------------------------
//
TBool CAspSyncUtil::DataProviderExist(TInt aApplicationId)
	{
	if (iContentList == NULL)
		{
		return ETrue;
		}
	
	TInt providerId = TUtil::ProviderIdFromAppId(aApplicationId);
	TInt index = iContentList->FindProviderIndex(providerId);
	
	if (index == KErrNotFound)
		{
		return EFalse;
		}
	return ETrue;
	}


// -----------------------------------------------------------------------------
// CAspSyncUtil::CreateContentListL
//
// -----------------------------------------------------------------------------
//
void CAspSyncUtil::CreateContentListL()
	{
	if (iContentList)
		{
		return;
		}

	TAspParam param(KErrNotFound, &iSyncSession);
    param.iMode = CAspContentList::EInitDataProviders;
    iContentList = CAspContentList::NewL(param);
	}


// -----------------------------------------------------------------------------
// CAspSyncUtil::CheckAppAndProvider
//
// -----------------------------------------------------------------------------
//
void CAspSyncUtil::CheckAppAndProviderL(TInt aApplicationId)
	{
	TInt sid = RProcess().SecureId().iId;
	
	if (!TUtil::IsValidSID(sid))
		{
		FLOG( _L("### CAspSyncUtil::CheckAppAndProviderL: wrong SID (%xd) ###"), sid);
		User::Leave(KErrNotSupported);  // unknown caller app
		}

	if (!TUtil::IsValidAppId(aApplicationId))
		{
		FLOG( _L("### CAspSyncUtil::CheckAppAndProviderL: wrong app id (%d) ###"), aApplicationId);
		User::Leave(KErrNotSupported);  // unknown caller app
		}
		
	if (aApplicationId != EApplicationIdSync && !DataProviderExist(aApplicationId))
		{
		FLOG( _L("### CAspSyncUtil::CheckAppAndProviderL: no data provider (%d) ###"), aApplicationId);
		User::Leave(KErrNotFound);  // sync data provider not found
		}
	}

// -----------------------------------------------------------------------------
// CAspSyncUtil::CheckTaskAndProtocolL
//
// -----------------------------------------------------------------------------
//
TBool CAspSyncUtil::CheckTaskAndProtocolL(TInt aProfileId ,const TDesC& aLocalDatabase)
	{
	
	TAspParam param(iApplicationId, &iSyncSession);
	
	CAspProfile* profile = CAspProfile::NewLC(param);
	TRAPD (err, profile->OpenL(aProfileId, CAspProfile::EOpenReadWrite, 
	                           CAspProfile::EAllProperties));
	if (err == KErrLocked)
		{
			TDialogUtil::ShowErrorNoteL(R_ASP_LOCKED_PROFILE);
			CleanupStack::PopAndDestroy(profile);                           
			return EFalse;
		}
	
	TInt dataProviderId = TUtil::ProviderIdFromAppId(iApplicationId);
	iContentList->RemoveAllTasks();
	iContentList->SetProfile(profile);
	iContentList->InitAllTasksL();
	
	if (iApplicationId == EApplicationIdEmail)
		{
			    
	    TInt protocol = profile->ProtocolVersion();
		if (protocol == EAspProtocol_1_1)
			{

      		CleanupStack::PopAndDestroy(profile);
      		iContentList->SetProfile(NULL);
      		return EFalse;	 
			}
        
        TInt taskId = TAspTask::FindTaskIdL(profile, dataProviderId, aLocalDatabase);
        if (taskId == KErrNotFound)
			{
	  		
      	  iContentList->ModifyTaskIncludedL(dataProviderId, ETrue, aLocalDatabase);
      		
      		}
	
		}
    else
		{
		TInt taskId = TAspTask::FindTaskIdL(profile, dataProviderId, KNullDesC);
        if (taskId == KErrNotFound)
			{
	  		    		
      		iContentList->ModifyTaskIncludedL(dataProviderId, ETrue, KNullDesC);
      		
			}
		}
	
	iContentList->SetProfile(NULL);
	CleanupStack::PopAndDestroy(profile);
	return ETrue;
	}

// -----------------------------------------------------------------------------
// OpenSyncSessionL
// 
// -----------------------------------------------------------------------------
//
void CAspSyncUtil::OpenSyncSessionL()
	{
	if (!iSyncSessionOpen)
		{
        TRAPD(err, iSyncSession.OpenL());
        
        if (err != KErrNone)
        	{
        	FLOG( _L("### CAspSyncUtil::OpenSyncSessionL: RSyncMLSession::OpenL failed (%d) ###"), err);
        	
            iSyncSession.Close();  // can be done always
        	User::Leave(err);
        	}
        else
        	{
        	FLOG( _L("CAspSyncUtil::OpenSyncSessionL: RSyncMLSession::OpenL ok") );
        	}
        
		iSyncSessionOpen = ETrue;
		}
    }


// -----------------------------------------------------------------------------
// CloseSyncSession
// 
// -----------------------------------------------------------------------------
//
void CAspSyncUtil::CloseSyncSession()
	{
	if (iSyncSessionOpen)
		{
		iSyncSession.Close();
		iSyncSessionOpen = EFalse;
		}
    }


// End of file

