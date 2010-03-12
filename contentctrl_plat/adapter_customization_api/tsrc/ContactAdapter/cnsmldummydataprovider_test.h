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
* Description:  Header file for contact adapter's dataprovider unit tests.
*
*/

#ifndef __CNSMLDUMMYDATAPROVIDER_TEST_H__
#define __CNSMLDUMMYDATAPROVIDER_TEST_H__

// INCLUDES
#include <CEUnitTestSuiteClass.h>
#include <EUnitDecorators.h>
#include "nsmldummydataprovider.h"

// FORWARD DECLARATIONS


// CLASS DEFINITION
/**
 * Generated EUnit test suite class.
 */
NONSHARABLE_CLASS( CNSmlDummyDataProvider_Test )
	: public CEUnitTestSuiteClass
    {
    public:  // Constructors and destructor

        static CNSmlDummyDataProvider_Test* NewL();
        static CNSmlDummyDataProvider_Test* NewLC();
        ~CNSmlDummyDataProvider_Test();

    private: // Constructors

        CNSmlDummyDataProvider_Test();
        void ConstructL();

    private: // New methods

    	void Empty() {};

        void SetupL();

        void Teardown();

        void T_Global_NewLL();
        
        void GreateAdapterL();

        void GetStoreFormatResourceFileLL();

        void DoStoreFormatLL();

        void TestStoreSupportForPartialSyncL();

        void TestStoreSupportMimeTypeL();

        void DoListStoresLCL();
        
        void DoOnFrameworkEvent();
        
        void DoSupportedServerFiltersL();
        
        void DoCheckSupportedServerFiltersL();
        
        void DoCheckServerFiltersL();
        
        void DoGenerateRecordFilterQueryL();
        
        void DoGenerateFieldFilterQueryL();

    private: // Data

        CNSmlDummyDataProvider* iCNSmlDummyDataProvider;

        EUNIT_DECLARE_TEST_TABLE;
    };

#endif      //  __CNSMLDUMMYDATAPROVIDER_TEST_H__
