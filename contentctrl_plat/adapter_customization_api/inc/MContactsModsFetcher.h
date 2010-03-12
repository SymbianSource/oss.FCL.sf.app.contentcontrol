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
* Description:  DS contacts dataprovider.
*
*/

#ifndef MCONTACTSMODSFETCHER_H
#define MCONTACTSMODSFETCHER_H

//  EXTERNAL INCLUDES
#include <e32def.h>


//  CLASS DEFINITION
/**
 * Interface class for Contacts mods fetcher
 * @since 3.23
 */
class MContactsModsFetcher
    {
    public:     // Destructor

        virtual ~MContactsModsFetcher() {}

    public:     // Abstract methods

        /**
        * Cancels the current asynchronous request
        */
        virtual void CancelRequest( ) = 0;

        /**
        * Reads all modifications from clients contacts databse.
        * @param aStatus  On completion of the operation, contains the result code.
        */
        virtual void FetchModificationsL( TRequestStatus& aStatus ) = 0;

    };

#endif      //  MCONTACTSMODSFETCHER_H

// End of file
