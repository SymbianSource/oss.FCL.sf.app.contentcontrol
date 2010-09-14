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
* Description:  Nsml DS settings Ecom plugin Impln.
*
*/


//System Includes
#include <e32std.h>
#include <implementationproxy.h>
#include <StringLoader.h>
#include <bautils.h>
#include <AknsUtils.h>
#include <gulicon.h>
#include <akntitle.h>

// Required only for embedded app
#include <AknNullService.h>
#include <apgtask.h>

// User includes
#include "OmaDsAppUiPluginInterface.h"
#include "gsprivatepluginproviderids.h"
#include <nsmldssync.mbg>
#include "nsmldsgspluginresource.rsg"


_LIT(KNsmlDsIconFileName, "Z:\\resource\\apps\\nsmldssync.mif");
_LIT(KNsmlDsResFileName,  "Z:\\resource\\NsmlDSGSPluginResource.rsc" );

// ---------------------------------------------------------------------------------------------
// COmaDsAppUiPluginInterface::NewLC
// returns an object of COmaDsAppUiPluginInterface
// ---------------------------------------------------------------------------------------------
//
CGSPluginInterface* COmaDsAppUiPluginInterface::NewL()
    {
	COmaDsAppUiPluginInterface* self = new(ELeave) COmaDsAppUiPluginInterface();
    CleanupStack::PushL(self);
    self->ConstructL();
	CleanupStack::Pop(self);
	return (CGSPluginInterface*) self;
	}

// ---------------------------------------------------------------------------------------------
// COmaDsAppUiPluginInterface::Id()
// returns the view id
// ---------------------------------------------------------------------------------------------
//
TUid COmaDsAppUiPluginInterface::Id() const
	{
	return TUid::Uid( KUidNsmlDSGSPluginDll );	
	}

// ---------------------------------------------------------------------------------------------
// COmaDsAppUiPluginInterface::ConstructL
// ---------------------------------------------------------------------------------------------
//	
void COmaDsAppUiPluginInterface::ConstructL()
	{
	BaseConstructL(); 
	
    env = CEikonEnv::Static();

	TFileName fileName(KNsmlDsResFileName);
	
	BaflUtils::NearestLanguageFile(env->FsSession(), fileName);

	iResId = env->AddResourceFileL(fileName);
	}


// ---------------------------------------------------------------------------------------------
// COmaDsAppUiPluginInterface::COmaDsAppUiPluginInterface
// ---------------------------------------------------------------------------------------------
//    
COmaDsAppUiPluginInterface::COmaDsAppUiPluginInterface() : iNullService( NULL )
	{
	// Nothing
	}

// ---------------------------------------------------------------------------------------------
// COmaDsAppUiPluginInterface::~COmaDsAppUiPluginInterface
// ---------------------------------------------------------------------------------------------
//
COmaDsAppUiPluginInterface::~COmaDsAppUiPluginInterface()
	{
	if (iResId)
		{
    	env->DeleteResourceFile(iResId);
		}	
	if ( iNullService )
        {
            delete iNullService;
            iNullService = NULL;
        }
	}

// ---------------------------------------------------------------------------------------------
// COmaDsAppUiPluginInterface::GetCaptionL
// returns the caption to be shown in GS view
// ---------------------------------------------------------------------------------------------
//
void COmaDsAppUiPluginInterface::GetCaptionL( TDes& aCaption ) const
    {
    StringLoader::Load( aCaption, R_DS_GS_TITLE);
    }

// ---------------------------------------------------------------------------------------------
// COmaDsAppUiPluginInterface::CreateIconL
// returns the icon to be shown in GS view
// ---------------------------------------------------------------------------------------------
//
CGulIcon* COmaDsAppUiPluginInterface::CreateIconL( const TUid /*aIconType*/ )
	{
	CFbsBitmap* bitmap = NULL;
    CFbsBitmap* mask = NULL;                                               
 
    MAknsSkinInstance* instance = AknsUtils::SkinInstance();    
    AknsUtils::CreateIconL(instance, KAknsIIDQgnPropCpConnDatasync, bitmap, mask,
    					KNsmlDsIconFileName,
    					EMbmNsmldssyncQgn_prop_cp_conn_datasync,
    					EMbmNsmldssyncQgn_prop_cp_conn_datasync_mask);

    CGulIcon* icon = CGulIcon::NewL( bitmap, mask );
    icon->SetBitmapsOwnedExternally( EFalse );
    bitmap = NULL;
    mask = NULL;      
    return icon;    
	}
	
// -----------------------------------------------------------------------------
// COmaDsAppUiPluginInterface::HandleSelection()
// -----------------------------------------------------------------------------
//
void COmaDsAppUiPluginInterface::HandleSelection(
    	const TGSSelectionTypes /*aSelectionType*/ )
	{
		TRAP_IGNORE(LaunchDsAppL());
	}
    
// ---------------------------------------------------------------------------------------------
// COmaDsAppUiPluginInterface::LaunchDsAppL
// ---------------------------------------------------------------------------------------------
//
void COmaDsAppUiPluginInterface::LaunchDsAppL()
	{
	TUid KAppUid( TUid::Uid( KDsAppUid ) );

	RWsSession ws;
    User::LeaveIfError( ws.Connect() );
    CleanupClosePushL( ws );
    
	TApaTaskList taskList( ws );
    TApaTask task = taskList.FindApp( KAppUid );

    if ( task.Exists() )
   		{
    	task.BringToForeground();
    	}
    else
    	{
        // Launch application as embedded.
        if ( iNullService )
        	{
            delete iNullService;
            iNullService = NULL;
        	}
        iNullService = CAknNullService::NewL( KAppUid, this );
    	}
    
	CleanupStack::PopAndDestroy();
  	}
	
// ---------------------------------------------------------------------------------------------
// COmaDsAppUiPluginInterface::DoActivateL
// ---------------------------------------------------------------------------------------------
//
void COmaDsAppUiPluginInterface::DoActivateL(const TVwsViewId& /*aPrevViewId*/,
											 TUid /*aCustomMessageId*/,
											 const TDesC8& /*aCustomMessage*/)
	{
	// Nothing
	}


// -----------------------------------------------------------------------------
// COmaDsAppUiPluginInterface::PluginProviderCategory
// -----------------------------------------------------------------------------
//
TInt COmaDsAppUiPluginInterface::PluginProviderCategory() const
	{
    return KGSPluginProviderInternal;
	}

// -----------------------------------------------------------------------------
// COmaDsAppUiPluginInterface::ItemType()
// -----------------------------------------------------------------------------
//
TGSListboxItemTypes COmaDsAppUiPluginInterface::ItemType()
	{
    return EGSItemTypeSettingDialog;
	}

// ---------------------------------------------------------------------------------------------
// COmaDsAppUiPluginInterface::DoDeactivate
// ---------------------------------------------------------------------------------------------
//	
void COmaDsAppUiPluginInterface::DoDeactivate()
	{
	// Nothing
	}

// End of File
