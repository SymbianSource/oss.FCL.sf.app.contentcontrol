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



// INCLUDE FILES
#include "NSmlDSSyncAppUi.h"
#include "NSmlDSSyncContainer.h" 
#include "NSmlDSSyncDocument.h"

#include <NSmlDSSync.rsg>
#include "NSmlDSSync.hrh"
#include "AspDebug.h"

#include <eikmenup.h> 



// ============================ MEMBER FUNCTIONS ===============================

//#ifdef RD_DSUI_CP_INTEGRATION
// -----------------------------------------------------------------------------
// CNSmlDSSyncAppUi::ConstructL
//
// -----------------------------------------------------------------------------
//
void CNSmlDSSyncAppUi::ConstructL()
    {
    FLOG( _L("CNSmlDSSyncAppUi::ConstructL START") );
    
    const TInt KBufSize128 = 128;
    const TInt KModeDialogNoWait = 4;
    
    BaseConstructL(EAknEnableSkin | EAknDisableHighlightAnimation | EAknEnableMSK | EAknSingleClickCompatible);

	TBool flag = CEikonEnv::Static()->StartedAsServerApp() ;
	if (flag)
		{
		TRAP_IGNORE(iIdle = CIdle::NewL( CActive::EPriorityHigh ));
	
    	if ( !iIdle->IsActive() ) 
    		{
       		iIdle->Start( TCallBack( OpenDialog, this ) );
    		}
		}
	else
		{
		iAppContainer = new (ELeave) CNSmlDSSyncContainer;
	    iAppContainer->SetMopParent(this);
	    iAppContainer->ConstructL(ClientRect());
	    AddToStackL(iAppContainer);

	    TBuf<KBufSize128> buf;
	    CNSmlDSSyncDocument* document= STATIC_CAST(CNSmlDSSyncDocument*, Document());
	    TRAPD(err, document->Model()->ShowSettingsL(0, KModeDialogNoWait, buf));
	    if(err != KErrNone)
	        {
            iAvkonAppUi->Exit();
	        }
		}

    FLOG( _L("CNSmlDSSyncAppUi::ConstructL END") );
    }
    
// ==========================================================================
// METHOD:  OpenDialog
//
// DESIGN:  CIdle Callback function
// ==========================================================================
TInt CNSmlDSSyncAppUi::OpenDialog( TAny* /*aThis*/ )
{
    const TInt KBufSize128 = 128;
         
	// Launch the settings dialog
 	TBuf<KBufSize128> buf;
    CSyncUtilApi* utilApi = CSyncUtilApi::NewL();
	CleanupStack::PushL( utilApi );

	TRAP_IGNORE( utilApi->ShowSettingsL( 0, 0, buf ) );

	CleanupStack::PopAndDestroy( utilApi );

	// We're done; exit
	iAvkonAppUi->Exit();
	
	return KErrNone;
} 

//#else
// -----------------------------------------------------------------------------
// CNSmlDSSyncAppUi::ConstructL
//
// -----------------------------------------------------------------------------
//
/*void CNSmlDSSyncAppUi::ConstructL()
    {
    FLOG( _L("CNSmlDSSyncAppUi::ConstructL START") );
    
    const TInt KBufSize128 = 128;
    const TInt KModeDialogNoWait = 4;
    
    BaseConstructL(EAknEnableSkin | EAknDisableHighlightAnimation | EAknEnableMSK);

    iAppContainer = new (ELeave) CNSmlDSSyncContainer;
    iAppContainer->SetMopParent(this);
    iAppContainer->ConstructL(ClientRect());
    AddToStackL(iAppContainer);

    TBuf<KBufSize128> buf;
    CNSmlDSSyncDocument* document= STATIC_CAST(CNSmlDSSyncDocument*, Document());
    document->Model()->ShowSettingsL(0, KModeDialogNoWait, buf);
    
    FLOG( _L("CNSmlDSSyncAppUi::ConstructL END") );
    }*/

//#endif

// -----------------------------------------------------------------------------
// CNSmlDSSyncAppUi::~CNSmlDSSyncAppUi
//
// -----------------------------------------------------------------------------
//
CNSmlDSSyncAppUi::~CNSmlDSSyncAppUi()
    {
    FLOG( _L("CNSmlDSSyncAppUi::~CNSmlDSSyncAppUi START") );
//#ifdef RD_DSUI_CP_INTEGRATION    
	if ( iIdle )
    	{
    	delete iIdle;
     	iIdle = NULL;
    	}
//#endif    	
    if (iAppContainer)
        {
        RemoveFromStack( iAppContainer );
        delete iAppContainer;
        }
        
    FLOG( _L("CNSmlDSSyncAppUi::~CNSmlDSSyncAppUi END") );
    }


// -----------------------------------------------------------------------------
// CNSmlDSSyncAppUi::DynInitMenuPaneL
//
// -----------------------------------------------------------------------------
//
void CNSmlDSSyncAppUi::DynInitMenuPaneL(TInt /*aResourceId*/, CEikMenuPane* /*aMenuPane*/)
    {
    }


// -----------------------------------------------------------------------------
// CNSmlDSSyncAppUi::HandleKeyEventL
//
// -----------------------------------------------------------------------------
//
TKeyResponse CNSmlDSSyncAppUi::HandleKeyEventL(const TKeyEvent& /*aKeyEvent*/,TEventCode /*aType*/)
    {
    return EKeyWasNotConsumed;
    }


// -----------------------------------------------------------------------------
// CNSmlDSSyncAppUi::HandleCommandL
//
// -----------------------------------------------------------------------------
//
void CNSmlDSSyncAppUi::HandleCommandL(TInt aCommand)
    {
    FLOG( _L("CNSmlDSSyncAppUi::HandleCommandL START") );
    
    switch (aCommand)
        {
		case EAknCmdExit:
        case EEikCmdExit:
        case EAknSoftkeyBack:
			{
     		FLOG( _L("before Exit") );
     		
     		Exit();
     		
     		FLOG( _L("after Exit") );
			break;			
			}

        default:
			{
            break;      
			}
        }
        
    FLOG( _L("CNSmlDSSyncAppUi::HandleCommandL END") );
	}




// End of File  
