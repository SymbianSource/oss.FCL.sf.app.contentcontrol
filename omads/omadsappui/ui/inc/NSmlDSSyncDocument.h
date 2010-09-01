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
* Description: Datasync app document 
*
*/



#ifndef NSMLDSSYNC_DOCUMENT_H
#define NSMLDSSYNC_DOCUMENT_H



// INCLUDES
#include <AknDoc.h>
#include "AspSyncUtilApi.h"
#include <eikenv.h>


// FORWARD DECLARATIONS
class  CEikAppUi;


// CLASS DECLARATION


/**
* CNSmlDSSyncDocument class.
*/
class CNSmlDSSyncDocument : public CAknDocument
    {
    public:
    
        /**
        * Two-phased constructor.
        */
        static CNSmlDSSyncDocument* NewL(CEikApplication& aApp);

        /**
        * Destructor.
        */
        virtual ~CNSmlDSSyncDocument();
        
    private:

        /**
        * C++ default constructor.
        */
        CNSmlDSSyncDocument(CEikApplication& aApp);

        /**
        * By default Symbian OS constructor is private.
        */
        void ConstructL();

    public:

        /**
        * Returns application model.
        * @param None
		* @return Application model.
        */
        CSyncUtilApi* Model();

    private:

        /**
        * From CEikDocument, create CAIWTestAppAppUi "App UI" object.
        * @param None.
		* @return App ui object.
        */
        CEikAppUi* CreateAppUiL();
        
        /**
        * Panics application.
        * @param aReason.
		* @return None.
        */
        void Panic(TInt aReason);
   
    private:
    
        // CSyncUtilApi handles synchronize operations. 
		CSyncUtilApi* iSyncUtilApi;
		
		// Eikon environment
		CEikonEnv*                          iEikEnv;
		
    };

#endif  // NSMLDSSYNC_DOCUMENT_H



// End of File

