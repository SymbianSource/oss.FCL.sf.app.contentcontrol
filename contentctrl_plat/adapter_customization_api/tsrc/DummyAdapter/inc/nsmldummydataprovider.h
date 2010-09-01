/*
* Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  DS Dummy dataprovider
*
*/

#ifndef CNSMLDUMMYDATAPROVIDER_H
#define CNSMLDUMMYDATAPROVIDER_H

//  EXTERNAL INCLUDES
#include <e32base.h>


//  INTERNAL INCLUDES
#include "nsmlcontactsdataprovider.h"

//  CONSTANTS
const TUint KNSmlDummyAdapterImplUid = 0x2001FDF1;



//  CLASS DEFINITION
/**
 * Dataprovider class for Dummy adapters
 * @lib nsmldummydataprovider.lib
 * @since 5.0
 */
class CNSmlDummyDataProvider : public CNSmlContactsDataProvider
    {
    
    private:    // Friend class definitions
        friend class CNSmlDummyDataProvider_Test;
    
    public:     // Constructors and destructor
        static CNSmlDummyDataProvider* NewL();
        static CNSmlDummyDataProvider* NewLC();
        ~CNSmlDummyDataProvider();

    protected:    // Constructors
        CNSmlDummyDataProvider();
        void ConstructL();

    protected:  // CSmlDataProvider

        /**
        * Gets dataproviders own StoreFormat.
        */
        const CSmlDataStoreFormat& DoStoreFormatL();

    protected: // CNSmlContactsDataProvider

        const TDesC& GetStoreFormatResourceFileL() const;

        CNSmlContactsDataStore* CreateDataStoreLC() const;

        CDesCArray* DoListStoresLC();

    };


#endif      //  CNSMLDUMMYDATAPROVIDER_H

// End of file
