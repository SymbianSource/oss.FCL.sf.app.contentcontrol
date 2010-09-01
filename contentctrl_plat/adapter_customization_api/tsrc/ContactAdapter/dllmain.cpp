//  EXTERNAL INCLUDES
#include <CEUnitTestSuite.h>
#include "CNSmlDummyDataProvider_Test.h"
#include "CNSmlDummyDataStore_Test.h"
#include "CNSmlDummyModsFetcher_Test.h"

/**
* UT_nsmldscontactsdataprovider.dll test suite factory function.
*/
EXPORT_C MEUnitTest* CreateTestSuiteL()
    {
    // In this example, a root suite is needed for holding the two suites
    //with test cases. If there was only one suite with tests, that suite
    //instance could be returned directly. (see the other examples)
    CEUnitTestSuite* rootSuite = CEUnitTestSuite::NewLC(_L("Unit test for Contact adapter"));

    // Unit tests for dataprovider class (CNSmlDummyDataProvider)
    MEUnitTest* providerSuite = CNSmlDummyDataProvider_Test::NewL();
    CleanupStack::PushL( providerSuite );
    rootSuite->AddL( providerSuite );
    CleanupStack::Pop( providerSuite );

    // Unit tests for datastore class (CNSmlDummyDataStore)
    MEUnitTest* storeSuite = CNSmlDummyDataStore_Test::NewL();
    CleanupStack::PushL( storeSuite );
    rootSuite->AddL( storeSuite );
    CleanupStack::Pop( storeSuite );

    // Unit tests for ModsFetcher class (CNSmlDummyModsFetcher)
    MEUnitTest* modsFetcherSuite = CNSmlDummyModsFetcher_Test::NewL();
    CleanupStack::PushL( modsFetcherSuite );
    rootSuite->AddL( modsFetcherSuite );
    CleanupStack::Pop( modsFetcherSuite );

    CleanupStack::Pop( rootSuite );

    return rootSuite;
    }
