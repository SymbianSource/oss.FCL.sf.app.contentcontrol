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
* Description: server derivation
*
*/


#ifndef NSMLDSSYNCAPPSERVER_H
#define NSMLDSSYNCAPPSERVER_H

// INCLUDES

#include <akndoc.h>
#include <aknapp.h>
#include <AknServerApp.h>


// CLASS DECLARATION

/**
* Data synchronization Aplication Server
*
*/
class CNSmlDSSyncAppServer :  public CAknAppServer
	{
	public:

        /**
        * C++ default constructor.
        */
		CNSmlDSSyncAppServer();
        
        /**
        * Destructor.
        */		
		virtual ~CNSmlDSSyncAppServer();
		
    public: //from CAknServerApp
    
        /**
        * CreateServiceL.
        * @param aServiceType
        * @return CApaAppServiceBase.
        */
        CApaAppServiceBase* CreateServiceL( TUid aServiceType ) const;
        
        /**
        * From the base class.
        * @param None.
        * @return None.
        */
        void HandleAllClientsClosed();
	};

#endif  // NSMLDSSYNCAPPSERVER_H


// End of File
