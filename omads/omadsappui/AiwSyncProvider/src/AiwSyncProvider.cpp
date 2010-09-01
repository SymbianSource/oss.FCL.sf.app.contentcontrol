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



#include <AiwMenu.h>
#include <AiwGenericParam.hrh>
#include <AiwCommon.hrh>

#include <implementationproxy.h>

#include <data_caging_path_literals.hrh>  // for resource and bitmap directories

#include <AiwSyncProvider.rsg>
#include "AiwSyncProvider.h"
#include "AiwSyncProviderUids.hrh"
#include "AiwSyncProvider.rh"
#include "AiwSyncProviderDebug.h"

#include <aknnotewrappers.h>    // CAknInformationNote




_LIT( KResFileName, "z:AiwSyncProvider.rsc" );



// -----------------------------------------------------------------------------
// CAspSyncProvider::NewL
//
// -----------------------------------------------------------------------------
//
CAspSyncProvider* CAspSyncProvider::NewL()
	{
    FLOG( _L("CAspSyncProvider::NewL START") );

	CAspSyncProvider* self = new (ELeave) CAspSyncProvider();
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    
    FLOG( _L("CAspSyncProvider::NewL END") );
	return self;
	}


// -----------------------------------------------------------------------------
// CAspSyncProvider::Constructor
//
// -----------------------------------------------------------------------------
//
CAspSyncProvider::CAspSyncProvider()
	{
	}


// -----------------------------------------------------------------------------
// CAspSyncProvider::Destructor
//
// -----------------------------------------------------------------------------
//
CAspSyncProvider::~CAspSyncProvider()
	{
	FLOG( _L("~CAspSyncProvider START") );
	
	delete iSyncUtilApi;
	
	FLOG( _L("~CAspSyncProvider END") );
	}


// -----------------------------------------------------------------------------
// CAspSyncProvider::ConstructL
//
// -----------------------------------------------------------------------------
//
void CAspSyncProvider::ConstructL()
    {
    FLOG( _L("CAspSyncProvider::ConstructL START") );
 
	iSyncUtilApi = CSyncUtilApi::NewL();

    FLOG( _L("CAspSyncProvider::ConstructL END") );
    } 


 
// -----------------------------------------------------------------------------
// CAspSyncProvider::InitialiseL (from CAiwServiceIfBase)
//
// -----------------------------------------------------------------------------
//
void CAspSyncProvider::InitialiseL(MAiwNotifyCallback& /*aFrameworkCallback*/,
	    						   const RCriteriaArray& /*aInterest*/)
	{
	// Not implemented
	}


// -----------------------------------------------------------------------------
// CAspSyncProvider::HandleServiceCmdL (from CAiwServiceIfBase)
//
// -----------------------------------------------------------------------------
//
void CAspSyncProvider::HandleServiceCmdL(const TInt& aCmdId,
                                         const CAiwGenericParamList& aInParamList,
                                         CAiwGenericParamList& /*aOutParamList*/,
                                         TUint /*aCmdOptions*/,
                                         const MAiwNotifyCallback* /*aCallback*/)
	{
	TInt jobId = KErrNotFound;
	TInt info1 = KErrNotFound;
	TBuf<KBufSize> info2(KNullDesC);
	TInt applicationId = GetApplicationId(aInParamList);

	if (aCmdId != KAiwCmdSynchronize)
		{
		return;
		}
		
   	if (applicationId == EGenericParamMessageItemEMail)
		{
	    GetFileName(aInParamList, info2);
	    if (info2.Length() == 0)
	    	{
	    	FLOG( _L("### mailbox name not found ###") );
	    	return; // email sync requires mailbox name
	    	}
	    info1 = EGenericParamMessageItemEMail;
		}
		
	iSyncUtilApi->SynchronizeL(applicationId, jobId, info1, info2); 
		
    
        
    // R&D solution: Remove constness.
    //MAiwNotifyCallback* callback = 
    //    const_cast< MAiwNotifyCallback* >(aCallback);

	//CAiwGenericParamList* dummyParamList = NULL;
	
	//callback->HandleNotifyL(0, 0, *dummyParamList, *dummyParamList);
	}



 
// -----------------------------------------------------------------------------
// CAspSyncProvider::InitializeMenuPaneL (from CAiwServiceIfMenu)
//
// -----------------------------------------------------------------------------
//
void CAspSyncProvider::InitializeMenuPaneL(CAiwMenuPane& aMenuPane,
											  TInt aIndex,
											  TInt /*aCascadeId*/,
											  const CAiwGenericParamList& aInParamList)
	{
	FLOG( _L("CAspSyncProvider::InitializeMenuPaneL START") );
	
	
	TFileName resFile;
	GetResFileName(resFile);
	
	FTRACE( RDebug::Print(_L("resfile= '%S'  index= %d"), &resFile, aIndex) );

    TInt serviceCommand = GetServiceCommandId(aInParamList);
    
    if (serviceCommand == KErrNotFound)
    	{
    	FLOG( _L("### service command not found ###") );
    	FLOG( _L("CAspSyncProvider::InitializeMenuPaneL END") );
    	return;
    	}
    
    if (serviceCommand == KAiwCmdSynchronizeSelected)
    	{
        TRAPD(ignoreError, aMenuPane.AddMenuItemsL(resFile, R_ASPSYNCPROVIDER_MENU_4, KAiwCmdSynchronizeSelected, aIndex));
        if (ignoreError != KErrNone)
        	{
        	FTRACE( RDebug::Print(_L("### CAiwMenuPane::AddMenuItemsL failed (%d) ###"), ignoreError) );
        	}
       	}
       	
    if (serviceCommand == KAiwCmdSynchronize)
    	{
    	TRAPD(ignoreError, aMenuPane.AddMenuItemsL(resFile, R_ASPSYNCPROVIDER_SUBMENU, KAiwCmdSynchronize, aIndex));
        if (ignoreError != KErrNone)
        	{
        	FTRACE( RDebug::Print(_L("### CAiwMenuPane::AddMenuItemsL failed (%d) ###"), ignoreError) );
        	}
    	}
    	
    FLOG( _L("CAspSyncProvider::InitializeMenuPaneL END") );
	}


// -----------------------------------------------------------------------------
// CAspSyncProvider::HandleMenuCmdL (from CAiwServiceIfMenu)
//
// -----------------------------------------------------------------------------
//
void CAspSyncProvider::HandleMenuCmdL(TInt aMenuCmdId, 
                                      const CAiwGenericParamList& aInParamList,
                                      CAiwGenericParamList& /*aOutParamList*/,
                                      TUint /*aCmdOptions*/,
                                      const MAiwNotifyCallback* /*aCallback*/)
	{
	FLOG( _L("CAspSyncProvider::HandleMenuCmdL START") );
    
	TInt jobId = KErrNotFound;
	TInt info1 = KErrNotFound;
	TBuf<KBufSize> info2(KNullDesC);
	TInt applicationId = GetApplicationId(aInParamList);
	
	if (applicationId == KErrNotFound)
		{
    	FLOG(_L("### application id not found ###"));
		return; // must have consumer app id
		}
	
	switch (aMenuCmdId)
		{
		case EAspSyncProviderCmdStart:
			if (applicationId == EGenericParamMessageItemEMail)
				{
			    GetFileName(aInParamList, info2);
			    if (info2.Length() == 0)
			    	{
			    	FLOG( _L("### mailbox name not found ###") );
			    	return; // email sync requires mailbox name
			    	}
				}
		
			iSyncUtilApi->SynchronizeL(applicationId, jobId, info1, info2); 
            break;

		case EAspSyncProviderCmdSettings:
		    iSyncUtilApi->ShowSettingsL(applicationId, info1, info2);
            break;
            
		case EAspSyncProviderCmdRetrieve:
		    applicationId = EGenericParamMessageItemEMail;
		    info1 = KAiwCmdSynchronizeSelected;
		    GetFileName(aInParamList, info2);
		    if (info2.Length() == 0)
		    	{
		    	FLOG( _L("### mailbox name not found ###") );
		    	return; // email sync requires mailbox name
		    	}
		    
		    iSyncUtilApi->SynchronizeL(applicationId, jobId, info1, info2);
            break; 
            
		default:
		    FTRACE( RDebug::Print(_L("### unknown menu command (%d) ###"), aMenuCmdId) );
			break;
		}
		
	FLOG( _L("CAspSyncProvider::HandleMenuCmdL END") );
	}


// -----------------------------------------------------------------------------
// CAspSyncProvider::GetApplicationIdL
//
// -----------------------------------------------------------------------------
//
TInt CAspSyncProvider::GetApplicationId(const CAiwGenericParamList& aInParamList)
	{
	_LIT(KSyncApplicationName, "sync");
	
	TInt index = 0;
	
	const TAiwGenericParam* param = aInParamList.FindFirst(index, EGenericParamContactItem);
	if (index >= 0)
		{
    	return EGenericParamContactItem;
		}
		
	index = 0;
	param = aInParamList.FindFirst(index, EGenericParamCalendarItem);
	if (index >= 0)
		{
    	return EGenericParamCalendarItem;
		}

    index = 0;
	param = aInParamList.FindFirst(index, EGenericParamMessageItemEMail);
	if (index >= 0)
		{
    	return EGenericParamMessageItemEMail;
		}
		
	index = 0;
	param = aInParamList.FindFirst(index, EGenericParamNoteItem);
	if (index >= 0)
		{
    	return EGenericParamNoteItem;
		}
   
    index = 0;
	param = aInParamList.FindFirst(index, EGenericParamMessageItemMMS);
	if (index >= 0)
		{
    	return EGenericParamMessageItemMMS;
		}

    index = 0;
	param = aInParamList.FindFirst(index, EGenericParamEmailAddress);
	if (index >= 0)
		{
    	return EGenericParamEmailAddress;
		}

	// this is for starting Sync application - not used at the moment
	index = 0;
	param = aInParamList.FindFirst(index, EGenericParamApplication);
	if (index >= 0)
		{
   	    TPtrC ptr;
    	const TAiwVariant& val = param->Value();
	    if (val.Get(ptr))
		    {
		    if (ptr.Compare(KSyncApplicationName) == 0)
		    	{
		    	return KSyncApplicationId;
		    	}
		    }
		}

	return KErrNotFound;
	}


// -----------------------------------------------------------------------------
// CAspSyncProvider::GetFileName
//
// -----------------------------------------------------------------------------
//
void CAspSyncProvider::GetFileName(const CAiwGenericParamList& aInParamList, TDes& aText)
	{
	aText = KNullDesC;
	TInt index = 0;
	const TAiwGenericParam* param = aInParamList.FindFirst(index, EGenericParamMessageItemMbox);
	if (index < 0)
		{
		return;
		}

    TPtrC ptr;
	const TAiwVariant& val = param->Value();
	if (!val.Get(ptr))
		{
		FLOG(_L("### CAspSyncProvider::GetFileName: param value not found ###"));
		return;
		}
	
	StrCopy(aText, ptr);
	}


// -----------------------------------------------------------------------------
// CAspSyncProvider::GetServiceCommandId
//
// -----------------------------------------------------------------------------
//
TInt CAspSyncProvider::GetServiceCommandId(const CAiwGenericParamList& aInParamList)
	{
	TInt index = 0;
	const TAiwGenericParam* param = aInParamList.FindFirst(index, EGenericParamServiceCommand);
	if (index < 0)
		{
    	return KErrNotFound;
		}

    TInt32 num = 0;
	const TAiwVariant& val = param->Value();
	if (!val.Get(num))
		{
		FLOG(_L("CAspSyncProvider::GetServiceCommandId: param value not found"));
		return KErrNotFound;
		}
	return num;
	}


// -----------------------------------------------------------------------------
// CAspSyncProvider::GetResFileName
//
// -----------------------------------------------------------------------------
//
void CAspSyncProvider::GetResFileName(TDes& aText)
	{
	TParse parse;
	parse.Set(KResFileName, &KDC_RESOURCE_FILES_DIR, NULL);
	StrCopy(aText, parse.FullName());
	}


// -------------------------------------------------------------------------------
// CAspSyncProvider::StrCopy
//
// String copy with lenght check.
// -------------------------------------------------------------------------------
//
void CAspSyncProvider::StrCopy(TDes& aTarget, const TDesC& aSource)
    {
	TInt len = aTarget.MaxLength();
    if(len < aSource.Length()) 
	    {
		aTarget.Copy(aSource.Left(len));
		return;
	    }
	aTarget.Copy(aSource);
    }


//------------------------------------------------------------------------------
// TDialogUtil::ShowInformationNoteL
//
//------------------------------------------------------------------------------
//
void CAspSyncProvider::ShowInformationNoteL(const TDesC& aText)
	{
	CAknInformationNote* queryDialog = new (ELeave) CAknInformationNote(ETrue);
	queryDialog->ExecuteLD(aText);
	}





//
// Rest of the file is for ECom initialization. 
//




// -----------------------------------------------------------------------------
// Map the interface UIDs to implementation factory functions
//
// -----------------------------------------------------------------------------
//
const TImplementationProxy ImplementationTable[] =
    {
    IMPLEMENTATION_PROXY_ENTRY(KAspSyncProviderImplUid,	CAspSyncProvider::NewL)
    };


// -----------------------------------------------------------------------------
// Exported proxy for instantiation method resolution
//
// -----------------------------------------------------------------------------
//
EXPORT_C const TImplementationProxy* ImplementationGroupProxy(TInt& aTableCount)
{
    aTableCount = sizeof(ImplementationTable) / sizeof(TImplementationProxy);

    return ImplementationTable;
}






// End of file
