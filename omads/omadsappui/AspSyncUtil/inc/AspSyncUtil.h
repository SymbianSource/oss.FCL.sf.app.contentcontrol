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
* Description: Utility methods 
*
*/


#ifndef ASPSYNCUTIL_H
#define ASPSYNCUTIL_H


// INCLUDES
#include <e32base.h>
#include "AspSyncHandler.h"
#include "AspSettingDialog.h"



// CLASS DECLARATION


/**
* CAspSyncUtil class
*
* CAspSyncUtil implements functionality that is exported in CSyncUtilApi. 
*/
NONSHARABLE_CLASS(CAspSyncUtil) : public CBase 
	{

    public:
        /**
        * Two-phased constructor.
        */
	    static CAspSyncUtil* NewL();

	    /**
        * Destructor.
        */
	    virtual ~CAspSyncUtil();

    private:
	    /**
        * By default Symbian 2nd phase constructor is private.
        */
	    void ConstructL();

	    /**
        * C++ default constructor.
        */
	    CAspSyncUtil();

	
	public:
		
        /**
        * Performs automatic (scheduled) data synchronization. Does not create 
        * any UI.Gets synchronization data from central repository.
        * @param aScheduleTaskFile File that contains sync scheduling task. 
	    * @return None.
        */
        void SynchronizeL(RFile& aScheduleTaskFile);        
        
        /**
        * Performs synchronization.
		* @param aApplicationId Id of calling application.
		* @param aInfo1.
		* @param aInfo2.
        * @return None
        */
		void DoSynchronizeL(TInt aApplicationId, TInt aInfo1, const TDesC& aInfo2);
		
        /**
        * Performs synchronization. Creates necessary UI.
        * @param aApplicationId Calling application id from AiwGenericParam.hrh. 
        * @param aJobId Sync task id, KErrNotFound if not used. 
        * @param aInfo1 Extra information, KErrNotFound if not used. 
        * @param aInfo2 Extra information, KNullDesC if not used. 
	    * @return None.
        */
		void SynchronizeL(TInt aApplicationId, TInt aJobId, TInt aInfo1, TDes& aInfo2);

        /**
        * Performs synchronization. Creates necessary UI.
        * @param aApplicationId Calling application id from AiwGenericParam.hrh. 
        * @param aJobId Sync task id, KErrNotFound if not used. 
        * @param aInfo1 Extra information, KErrNotFound if not used. 
        * @param aInfo2 Extra information, KNullDesC if not used. 
	    * @return None.
        */
		void DoSynchronizeL(TInt aApplicationId, TInt aJobId, TInt aInfo1, TDes& aInfo2);

        /**
        * Shows settings dialog for changing sync settings.
		* @param aApplicationId Calling application id.
		* @param aProfileId Profile id.
		* @param aDialogMode Dialog mode.
        * @return None.
        */
		void ShowSettingsDialogL(TInt aApplicationId, TInt aProfileId, TInt aDialogMode);
		
        /**
        * Shows settings dialog for changing sync settings.
		* @param aApplicationId Calling application id.
        * @return None.
        */
		void ShowProfileDialogL(TInt aApplicationId);
		
        /**
        * Shows UI for editing synchronization settings.
        * @param aApplicationId Calling application id from AiwGenericParam.hrh.
        * @param aInfo1 Extra information, KErrNotFound if not used. 
        * @param aInfo2 Extra information, KNullDesC if not used.
	    * @return None.
        */
		void ShowSettingsL(TInt aApplicationId, TInt aInfo1, TDes& aInfo2);
 		
        /**
        * Shows UI for editing synchronization profile.
        * @param aApplicationId Calling application id from AiwGenericParam.hrh.
  	    * @return None.
        */
		void EditProfileL(TInt aApplicationId);
		
        /**
        * Shows UI for editing synchronization profile.
        * @param aApplicationId Calling application id from AiwGenericParam.hrh.
        * @param aProfileId.
  	    * @return None.
        */
		void DoEditProfileL(TInt aAplicationId, TInt aProfileId, TInt aDialogMode);

        /**
        * Shows UI for editing synchronization profile.
        * @param aApplicationId Calling application id from AiwGenericParam.hrh.
        * @param aProfileId.
  	    * @return None.
        */
		void DoEditReadOnlyProfileL(TInt aAplicationId, TInt aProfileId, 
		                            TInt aDialogMode);

    private:

		/**
        * Checks profile mandatory fields.
        * @param aProfileId.
    	* @return ETrue if mandatory fields are ok, EFalse otherwise.
        */
		TBool CheckMandatoryL(TInt aProfileId);
		
		/**
        * Checks already selected profile for mandatory content and correct protocol version
        * @param aProfileId 
        * @param aLocalDatabase SyncML mail details name for Email app ,KNullDesC otherwise
  	    * @return ETrue if task exists and correct protocol selected, EFalse otherwise.
        */
		
		TBool CheckTaskAndProtocolL(TInt aProfileId ,const TDesC& aLocalDatabase);

		/**
        * Find out whether sync data provider exists.
        * @param aApplicationId.
    	* @return ETrue if provider exists, EFalse otherwise.
        */
		TBool DataProviderExist(TInt aApplicationId);
		
		/**
        * Checks that app id is valid and data provider exists.
        * @param aApplicationId.
    	* @return None.
        */
		void CheckAppAndProviderL(TInt aApplicationId);
		
		/**
        * Opens session with sync server.
        * @param None.
    	* @return None.
        */
		void OpenSyncSessionL();

		/**
        * Closes session with sync server.
        * @param None.
    	* @return None.
        */
		void CloseSyncSession();
		
		/**
        * Creates contentlist.
        * @param None.
    	* @return None.
        */
		void CreateContentListL();
		
		/**
        * Selects profile.
        * @param aProfileId.
        * @param aTaskId.
    	* @param aResourceId.
    	* @param aLocalDatabase
    	* @param aCheckId 
    	* @return None.
        */
        TBool SelectProfileL(TInt& aProfileId, TInt& aTaskId, 
                             TInt aResourceId, const TDesC& aLocalDatabase,TBool aCheckId);

    private:
		// id of the calling application
		TInt iApplicationId;

   		// sync session
		RSyncMLSession iSyncSession;
		
		// is sync session open
		TBool iSyncSessionOpen;
		
		// performs sync operation, uses CNSmlAppEngine
		CAspSyncHandler* iSyncHandler;

		// sync data provider list
		CAspContentList* iContentList;
		
		// settings dialog
		CAspSettingDialog* iSettingDialog;
		
    	// is setting database in read only state
		TBool iSettingEnforcement;
		
    	// resource handler
		CAspResHandler* iResHandler;
    };



#endif  // ASPSYNCUTIL_H


// End of file