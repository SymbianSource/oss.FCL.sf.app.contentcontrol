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
* Description: A sync ui application class 
*
*/



#ifndef NSMLDSSYNC_APP_H
#define NSMLDSSYNC_APP_H

// INCLUDES
#include <aknapp.h>
#include <eikapp.h>

// CONSTANTS

// UID of the application
const TUid KUidSmlSyncApp = { 0x101F6DE4 };


// CLASS DECLARATION


/**
* Class CNSmlDSSyncApp.
* 
*/
class CNSmlDSSyncApp : public CAknApplication
    {
    
    private:

        /**
        * From CApaApplication, creates application document object.
        * @param None.
        * @return A pointer to the created document object.
        */
        CApaDocument* CreateDocumentL();
        
        /**
        * From CApaApplication, returns application's UID.
        * @param None.
        * @return App uid.
        */
        TUid AppDllUid() const;
    };


#endif  // NSMLDSSYNC_APP_H


// End of File

