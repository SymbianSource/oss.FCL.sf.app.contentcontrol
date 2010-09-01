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
* Description:  Contains functions that are exported from AspSyncUtil for
*                performing data synchronization.
*
*/


#ifndef ASPSYNCUTILAPI_H
#define ASPSYNCUTILAPI_H


//  INCLUDES
#include <e32base.h>
#include <s32file.h>


// FORWARD DECLARATIONS
class CAspSyncUtil;



// CLASS DECLARATION

/**
*  Public API for executing sync ui library functions. 
* 
*  @lib aspsyncutil.lib
*  @since Series60_3.0
*  
*/
class CSyncUtilApi : public CBase
    {

    public:
        /**
        * Two-phased constructor.
        */
	    IMPORT_C static CSyncUtilApi* NewL();

	    /**
        * Destructor.
        */
	    virtual ~CSyncUtilApi();

    private:
        /**
        * By default Symbian 2nd phase constructor is private.
        */
	    void ConstructL();

        /**
        * C++ default constructor.
        */
	    CSyncUtilApi();

    public:

       /**
        * Shows UI for editing data synchronization settings.
        * @since Series60_3.0
        * @param aAppId Calling application id from AiwGenericParam.hrh.
        * @param aInfo1 Extra information, KErrNotFound if not used. 
        * @param aInfo2 Extra information, KNullDesC if not used.
	    * @return None.
        */
		IMPORT_C void ShowSettingsL(TInt aAppId, TInt aInfo1, TDes& aInfo2);


        /**
        * Performs data synchronization. Creates necessary UI.
        * @since Series60_3.0
        * @param aAppId Calling application id from AiwGenericParam.hrh. 
        * @param aJobId Sync job id, KErrNotFound if not used. 
        * @param aInfo1 Extra information, KErrNotFound if not used. 
        * @param aInfo2 Extra information, KNullDesC if not used. 
	    * @return None.
        */
        IMPORT_C  void SynchronizeL(TInt aAppId, TInt aJobId, TInt aInfo1, TDes& aInfo2);

 
        /**
        * Performs automatic (scheduled) data synchronization. Does not create 
        * any UI.Gets synchronization data from central repository.
        * @since Series60_3.0
        * @param aScheduleTaskFile File that contains sync scheduling task. 
	    * @return None.
        */
		IMPORT_C void SynchronizeL(RFile& aScheduleTaskFile);

    private:
		CAspSyncUtil* iSyncUtil;
    };


#endif  // ASPSYNCUTILAPI_H


// End of file