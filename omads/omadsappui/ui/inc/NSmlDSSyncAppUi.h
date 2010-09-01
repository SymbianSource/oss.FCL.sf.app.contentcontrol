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
* Description: Datasync app UI
*
*/



#ifndef NSMLDSSYNC_APPUI_H
#define NSMLDSSYNC_APPUI_H

// INCLUDES
#include <eikapp.h>
#include <eikdoc.h>
#include <e32std.h>
#include <coeccntx.h>
#include <aknappui.h>


// FORWARD DECLARATIONS
class CNSmlDSSyncContainer;



// CLASS DECLARATION

/**
*  Class CNSmlDSSyncAppUi.
*/
class CNSmlDSSyncAppUi : public CAknAppUi
    {
    public: 

        /**
        * By default Symbian OS constructor is private.
        */
        void ConstructL();

        /**
        * Destructor.
        */      
        ~CNSmlDSSyncAppUi();
//#ifdef RD_DSUI_CP_INTEGRATION        
        static TInt OpenDialog( TAny* aThis) ;
//#endif
    private:
 
		/**
        * From the base class.
		* Called by framework before menu is shown.
        * @param aResourceId Menu resource id.
		* @param aMenuPane Pointer to the menu.
        * @return None.
        */
        void DynInitMenuPaneL(TInt aResourceId,CEikMenuPane* aMenuPane);

    private:
        /**
        * From CEikAppUi, takes care of command handling.
        * @param aCommand command to be handled
        * @return None.
        */
        void HandleCommandL(TInt aCommand);

        /**
        * From CEikAppUi, handles key events.
        * @param aKeyEvent Event to handled.
        * @param aType Type of the key event. 
        * @return Response code (EKeyWasConsumed, EKeyWasNotConsumed). 
        */
        virtual TKeyResponse HandleKeyEventL(const TKeyEvent& aKeyEvent,TEventCode aType);

    private:
        // container 
//#ifdef RD_DSUI_CP_INTEGRATION      
        CIdle *iIdle;
//#endif
        CNSmlDSSyncContainer* iAppContainer; 
    };


#endif  // NSMLDSSYNC_APPUI_H


// End of File
