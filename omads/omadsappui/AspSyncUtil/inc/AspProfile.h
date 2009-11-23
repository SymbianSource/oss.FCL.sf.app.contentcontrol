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
* Description: Profile structure mapped to server side data sync profile 
*
*/


#ifndef ASPPROFILE_H
#define ASPPROFILE_H

// INCLUDES
#include <e32base.h>
#include <ecom.h>

#include <SyncMLClient.h>    // for RSyncMLSession
#include <SyncMLClientDS.h>  // for RSyncMLDataSyncProfile

#include "AspDefines.h"
#include "AspUtil.h"

//For Error EAZG-7KGAK8. Language definiton not found in elang.h 
const TInt ELangIndon_Apac=327;
const TUid KCRUidDSDefaultProfileInternalKeys  = { 0x20021338 };
const TInt KNsmlDsDefaultProfile = 0x0;

// CONSTANTS

// FORWARD DECLARATIONS
class CAspProfile;


// CLASS DECLARATION



/**
* TAspProfileItem
*
* TAspProfileItem contains sync profile data.
*/
NONSHARABLE_CLASS (TAspProfileItem)
	{
	public:
		/**
		* Get profile name.
		* @param None.
        * @return Profile name.
        */
		const TDesC& Name();

		/**
		* Get profile name.
		* @param None.
        * @return Profile name.
        */
		HBufC* NameLC();

		/**
		* Set profile name.
		* @param aName.
        * @return None.
        */
		void SetName(const TDesC& aName);
		
		/**
		* Initalize object data.
		* @param None.
        * @return None.
        */
		void Init();

	public:
		/**
		* Compare two profile items.
		* @param aFirst.
		* @param aSecond.		
        * @return Compare result.
        */
		static TInt CompareItems(const TAspProfileItem& aFirst, const TAspProfileItem& aSecond);

	public:
		// profile name
		TBuf<KBufSize> iProfileName;	
		
		// profile id 
		TInt iProfileId;
		
		// is profile synced
		TInt iSynced;
		
		// last sync time
		TTime iLastSync;
		
		// bearer type
		TInt iBearer;
		
		// is server alert sync activated
		TInt iActive;
		
		// can profile be deleted
		TBool iDeleteAllowed;
		
		// application id
		TInt iApplicationId;

		// is all mandatory data found 
		TInt iMandatoryCheck;
		
		// number of sync tasks
		TInt iTaskCount;
		
		// application specific sync task id
		TInt iTaskId;
		
		// server id check sum
		TUint16 iServerIdCheckSum;

	};



/**
* TAspTaskItem
* 
* TAspTaskItem contains sync task data.
*/
NONSHARABLE_CLASS (TAspTaskItem)
	{
	public:
	    // task id
    	TInt  iTaskId;
    	
    	// sync direction
		TInt  iSyncDirection;
		
		// is remote database defined
		TBool iRemoteDatabaseDefined;
		
		// is local database defined
		TBool iLocalDatabaseDefined;
		
		// is task included in sync
		TBool iEnabled;
		
		// id of data provider that this task uses
		TInt  iDataProviderId;
		
		// last sync time
		TTime	 iLastSync;

		// task display name
	    TBuf<KBufSize> iDisplayName;
	    TBuf<KBufSize> iClientDataSource;
    };


/**
* TAspDataProviderItem
* 
* TAspDataProviderItem contains data provider data.
*/
NONSHARABLE_CLASS (TAspProviderItem)
	{
    enum TProviderListSortOrder
	    {
	    EOrderContact = 4,
	    EOrderCalendar = 3,
	    EOrderNotes = 2,
	    EOrderEmail = 1,
	    EOrderOther = 0
    	};
	
	public:
		/**
		* Compare two profile items.
		* @param aFirst.
		* @param aSecond.		
        * @return Compare result.
        */
		static TInt CompareItems(const TAspProviderItem& aFirst, const TAspProviderItem& aSecond);
		static TInt SortOrder(TInt aDataProviderId);
	
	public:
	    // data provider id
		TInt  iDataProviderId;
		
		// can more than one database be synced 
		TBool iAllowMultipleDataStores;
		
		// is default data store defined
		TBool iHasDefaultDataStore;
		
		// sync protocol
		TInt  iSyncProtocol;

        // id of sync task that is using this data provider
		TBool iTaskId;
		
		// display name
	    TBuf<KBufSize> iDisplayName;
	    
	   	// default data store name
		TBuf<KBufSize> iDefaultDataStore;
		
		// is this provider included in sync (profile specific)
		TBool iIsIncludedInSync;
	};


/**
* TAspFilterInfo
* 
* TAspFilterInfo contains profile list filtering data.
*/
NONSHARABLE_CLASS (TAspFilterInfo)
	{
	public:
	enum TFilterType
		{
	    EIncludeDeletableProfile,
	    EIncludeRemoteProfile
		};

	public:
	    // filter type
		TInt iFilterType;
		
		// filter id (eg. application id 
		TBool iFilterId;
		
		// filter text
		TBuf<KBufSize>  iFilterText;
	};


/**
* CAspContentList 
*
* CAspContentList contains sync task list and data provider list.
*/
NONSHARABLE_CLASS (CAspContentList): public CBase
    {
    public:

	enum TListMode
		{
	    EInitDataProviders,
	    EInitTasks,
	    EInitAll
		};

    public:
        /**
        * Two-phased constructor.
        */
        static CAspContentList* NewLC(const TAspParam& aParam);

        /**
        * Two-phased constructor.
        */
        static CAspContentList* NewL(const TAspParam& aParam);

	    /**
        * Destructor.
        */
        virtual ~CAspContentList();

    private:
        /**
        * C++ default constructor.
        */
	    CAspContentList(const TAspParam& aParam);

        /**
        * By default Symbian 2nd phase constructor is private.
        */
	    void ConstructL();

	public:
		/**
		* Sets sync profile.
		* @param aProfile.
        * @return None.
        */
		void SetProfile(CAspProfile* aProfile);
		
		/**
		* Find out whether iProfile has been set.
		* @param None.
        * @return TBool.
        */
		TBool IsLocked();
	
		/**
		* Returns data provider count.
		* @param None.
        * @return Data provider count.
        */
		TInt ProviderCount();

		/**
		* Returns Data provider.
        * @param aIndex List index.
		* @return Data provider.
        */
		TAspProviderItem& ProviderItem(TInt aIndex);
		
		/**
		* Returns list index.
        * @param aId.
		* @return List index.
        */
        TInt FindProviderIndex(TInt aId);		

		/**
		* Returns list index.
        * @param aId.
		* @return List index.
        */
        TInt FindProviderIndexForTask(TInt aId);		

		/**
		* Returns task count.
        * @return Task count.
        */
		TInt TaskCount();
		
		/**
		* Returns task item.
        * @param aIndex.
		* @return Task item.
        */
		TAspTaskItem& TaskItem(TInt aIndex);
		
		/**
		* Returns list index.
        * @param aId.
		* @return List index.
        */
        TInt FindTaskIndex(TInt aId);		

		/**
		* Returns list index.
        * @param aId.
		* @return List index.
        */
        TInt FindTaskIndexForProvider(TInt aId);		

		/**
		* Returns task id.
        * @param aId.
		* @return Task id.
        */
        TInt FindTaskIdForProvider(TInt aId);		
		
		/**
		* Checks that data provider has default local database.
		* @param aDataProviderId.
		* @return ETrue if database exists, EFalse otherwise.
        */
		//TBool DefaultLocalDatabaseExist(TInt aDataProviderId);
		
		/**
		* Checks that all content items are valid.
		* @param aContentCount.
		* @return Return code.
        */
		TInt CheckMandatoryDataL(TInt& aContentCount);
        
	public:

		/**
		* Read sync data providers from RSyncMLSession.
		* @return None.
        */
		void InitDataProvidersL();
		
		/**
		* Read sync data providers from RSyncMLSession.
		* @return None.
        */
		void DoInitDataProvidersL();

		/**
		* Read all tasks from profile into task list.
		* @return None.
        */
		void InitAllTasksL();

		/**
		* Read one task from profile into task list.
		* @return None.
        */
		void InitTaskL(TInt aTaskId);
		
		/**
		* Remove task that uses dataprovider aDataProviderId.
		* @param aDataProviderId.
		* @return None.
        */
		void RemoveTask(TInt aDataProviderId);
		
		/**
		* Remove all tasks from task list.
		* @param None.
		* @return None.
        */
		void RemoveAllTasks();

		/**
		* Remove data provider with id aDataProviderId.
		* @param aDataProviderId.
		* @return None.
        */
		void RemoveDataProvider(TInt aDataProviderId);

		/**
		* Read one task item from profile.
        * @param aProfile.
        * @param aTaskId.
		* @return Task item.
        */
		static TAspTaskItem ReadTaskItemL(RSyncMLDataSyncProfile& aProfile, TInt aTaskId);
		
		
		TInt CreateTaskL(TAspProviderItem& aDataProvider);
        void CreateTaskL(TInt aDataProviderId, const TDesC& aLocalDatabase,
                         const TDesC& aRemoteDatabase, TBool aEnabled, TInt aSyncDirection);
        void CreateTask(TInt aDataProviderId, const TDesC& aLocalDatabase,
                         const TDesC& aRemoteDatabase, TBool aEnabled, TInt aSyncDirection);


        void ReadTaskL(TInt aDataProviderId, TDes& aLocalDatabase, TDes& aRemoteDatabase, TBool& aEnable, TInt& aSyncDirection);

 
		void IncludeTaskL(TAspProviderItem& aDataProvider);
		void IncludeTasks(const CArrayFix<TInt>* aIncludedProviderList);
		
		void ModifyTaskIncludedL(TInt aDataProviderId, TBool aIsIncluded, const TDesC& aLocalDatabase);
		void ModifyTaskIncludedL(TAspProviderItem& aProvider, TBool aIsIncluded, const TDesC& aLocalDatabase);
		void ModifyTaskDirectionL(TAspProviderItem& aProvider, TInt aDirection);
		void ModifyTaskDirectionsL(TInt aSyncDirection);
		TBool CheckTaskDirectionsL();
		
		void SetIncludedProviders(const CArrayFix<TInt>* aIncludedProviderList);
		void GetIncludedProviders(CArrayFix<TInt>* aIncludedProviderList);
		

		
		void GetLocalDatabaseList(TInt aDataProviderId, CDesCArray* aList);
		void GetLocalDatabaseListL(TInt aDataProviderId, CDesCArray* aList);
		void UpdateLocalDatabaseL();
		TInt SyncDirection();
		TSmlSyncType SmlSyncDirection();
		TBool TaskEnabled(TInt aDataProviderId);
    	void Sort();
    	void UpdateDataProviderL(TInt aDataProviderId);
    	void ReadDataProviderItemL(RSyncMLDataProvider& aProvider, TAspProviderItem& aItem);


		
		
    private:
    
		/**
		* Utility function.
		* @return Sync session.
        */
		RSyncMLSession& Session();

		/**
		* Returns sync profile.
		* @param None.
		* @return Sync profile.
        */
		RSyncMLDataSyncProfile* Profile();
		
		
#ifdef _DEBUG
    public:
        void LogL(const TDesC& aText);
    private:
    
        void LogTasksL();
        void LogDataProvidersL();
        void GetTaskLogText(TDes& aText, TAspTaskItem& aTask);
        void GetDataProviderIdText(TDes& aText, TInt aId);
#endif
		


    private:
		// id of the calling application
		TInt iApplicationId;
    
		// sync session
		RSyncMLSession* iSyncSession;
		
		// sync profile
		CAspProfile* iProfile;

		// sync list type
		TInt iListMode;
        
		// data provider list
		RArray<TAspTaskItem> iTaskList;

		// data provider id list
		RArray<TAspProviderItem> iProviderList;
    };



/**
* CAspProfileList 
*
* CAspProfileList is a list of sync profiles,
*/
NONSHARABLE_CLASS (CAspProfileList) : public CBase
    {
    public:

	enum TListMode
		{
	    EBasePropertiesOnly,
	    ENoMandatoryCheck,
	    EMandatoryCheck,
	    EMandatoryCheckEx,
	    EEMailRetrieve,
	    EEMailSync,
		};

    public:
        /**
        * Two-phased constructor.
        */
        static CAspProfileList* NewLC(const TAspParam& aParam);

        /**
        * Two-phased constructor.
        */
        static CAspProfileList* NewL(const TAspParam& aParam);

	    /**
        * Destructor.
        */
        virtual ~CAspProfileList();

    private:
        /**
        * C++ default constructor.
        */
	    CAspProfileList(const TAspParam& aParam);

        /**
        * By default Symbian 2nd phase constructor is private.
        */
	    void ConstructL();

	public:
		/**
		* Returns profile count.
        * @param None.
        * @return Profile count.
        */
		TInt Count();

		/**
		* Returns profile count.
        * @param aApplicationId.
        * @return Profile count.
        */
		TInt Count(TInt aApplicationId);

		/**
		* Returns profile from list.
        * @param aIndex List index.
		* @return Profile.
        */
		TAspProfileItem& Item(TInt aIndex);

		/**
		* Finds profile with name aProfileName.
		* @param aProfileName Profile name.
		* @return Profile index or KErrNotFound.
        */
		TInt FindProfileIndex(const TDesC& aProfileName);
		
		/**
		* Gets the Default Profile ID from the Cenrep
		* @param aValue ProfileID read from the Cenrep
     */
		void GetDefaultProfileIdL(TInt& aValue);
		
		/**
		* Finds last synced profile index.
		* @param aProfileName Profile name.
		* @return Profile index or KErrNotFound.
        */
		TInt FindLastSyncedProfileIndexL();
		
		/**
		* Read one profile from database.
		* @param aProfile.
		* @param aItem.
		* @return None.
        */
        void ReadProfileItemL(CAspProfile* aProfile, TAspProfileItem& aItem);

		/**
		* Reads all profiles from database.
		* @param aListMode.
		* @return None.
        */
		void ReadAllProfilesL(TInt aListMode);
		
		/**
		* Reads all profiles that contain EMail task that uses aLocalDatabase.
		* @param aLocalDatabase.
		* @param aListMode.
		* @return None.
        */
		void ReadEMailProfilesL(const TDesC& aLocalDatabase,  TInt aListMode);

		/**
		* Reads one profile from database.
		* @param aProfileId.
		* @return None.
        */
		void ReadProfileL(TInt aProfileId);

		/**
		* Sorts profile list.
		* @param None.
		* @return None.
        */
    	void Sort();

		/**
		* Removes profile from profile list.
		* @param aProfileId.
		* @return None.
        */
		void Remove(TInt aProfileId);
		
		/**
		* Adds profile into profile list.
		* @param aProfileItem.
		* @return None.
        */
		void AddL(TAspProfileItem& aProfileItem);
		
		/**
		* Constructs filtered profile list.
		* @param aFilterInfo.
		* @return Profile list.
        */
		CAspProfileList* FilteredListL(TAspFilterInfo& aFilterInfo);

		/**
		* Replaces profile item.
		* @param aProfileItem.
		* @return None.
        */
		void ReplaceProfileItemL(TAspProfileItem& aProfileItem);

		/**
		* Returns profile's list position.
		* @param aProfileId.
		* @return List index.
        */
		TInt ListIndex(TInt aProfileId);
		
		/**
		* Returns sync session.
		* @param None.
		* @return Sync session.
        */
		RSyncMLSession& Session();
		
		/**
		* Returns number of profiles that can be deleted.
		* @param None.
		* @return Profile count.
        */
		TInt DeletableProfileCount();
		
		TBool IsAutoSyncProfile(CAspProfile* aProfile);
		
	public:
                             
		/**
		* Checks if aServerId is unique in setting databse.
		* @param aServerId.
		* @param aProfileId.
		* @return Return code.
        */
        TBool IsUniqueServerId(const TDesC& aServerId, TInt aProfileId);                             

		

	private:
   		// sync session
		RSyncMLSession* iSyncSession;
        
		// list of profiles
		RArray<TAspProfileItem> iList;

		// needed for filtering unwanted profiles
		TInt iApplicationId;
		
		// long buffer for string handling 
		TBuf<KBufSize255> iBuf;
    };



/**
* CAspProfile 
*
* CAspProfile is used for getting/setting sync profile settings.
*/
NONSHARABLE_CLASS (CAspProfile) : public CBase
    {
    
    public:
	enum TOpenMode
		{
	    EOpenReadWrite,
	    EOpenRead,
	    EBaseProperties,
	    EAllProperties
		};

    public:
        /**
        * Two-phased constructor.
        */
        static CAspProfile* NewLC(const TAspParam& aParam);

        /**
        * Two-phased constructor.
        */
        static CAspProfile* NewL(const TAspParam& aParam);

	    /**
        * Destructor.
        */
        virtual ~CAspProfile();

    private:
        /**
        * C++ default constructor.
        */
	    CAspProfile(const TAspParam& aParam);

        /**
        * By default Symbian 2nd phase constructor is private.
        */
	    void ConstructL();
	    
    public:
		/**
		* Open profile.
		* @param aProfileId.
		* @param aReadWrite.
		* @param aOpenMode.
		* @return None.
        */
        void OpenL(TInt aProfileId, TInt aReadWrite, TInt aOpenMode);
        
		/**
		* Create profile.
		* @param aOpenMode.
		* @return None.
        */
        void CreateL(TInt aOpenMode);

		/**
		* Create copy profile.
		* @param aProfileId.
		* @return None.
        */
        void CreateCopyL(TInt aProfileId);
        
		/**
		* Save profile.
		* @param None.
		* @return None.
        */
        void SaveL();
        
		/**
		* Save profile.
		* @param None.
		* @return None.
        */
        void Save();

	public:
		/**
		* Set profile name.
		* @param aText.
		* @return None.
        */
        void SetNameL(const TDesC& aText);

		/**
		* Get profile name.
		* @param aText.
		* @return None.
        */
        void GetName(TDes& aText);

		/**
		* Get profile name.
		* @param None.
		* @return Profile name.
        */
        HBufC* NameL();
        
		/**
		* Set creator id.
		* @param aCreatorId.
		* @return None.
        */
        void SetCreatorId(TInt aCreatorId);
        
		/**
		* Get creator id.
		* @param None.
		* @return Creator id.
        */
        TInt CreatorId();
        
		/**
		* Get profile id.
		* @param None.
		* @return Profile id.
        */
        TInt ProfileId();
        
		/**
		* Can profile be deleted.
		* @param None.
		* @return Boolean.
        */
        TBool DeleteAllowed();

		/**
		* Has profile been synced.
		* @param None.
		* @return Boolean.
        */
		TBool IsSynced();

		/**
		* Last sync time.
		* @param None.
		* @return Sync time.
        */
		TTime LastSync();
    
		/**
		* Last sync time.
		* @param aTaskId.
		* @return Sync time.
        */
		TTime LastSync(TInt aTaskId);

    
    // settings that connection dialog displays
    public:
		/**
		* Set server id.
		* @param aText.
		* @return None.
        */
        void SetServerIdL(const TDesC& aText);

		/**
		* Get server id.
		* @param aText.
		* @return None.
        */
        void GetServerId(TDes& aText);

		/**
		* Get protocol version.
		* @param None.
		* @return aProtocolVersion.
        */
        TInt ProtocolVersion();

		/**
		* Set protocol version.
		* @param aProtocolVersion.
		* @return None.
        */
        void SetProtocolVersionL(TInt aProtocolVersion);

		/**
		* Get bearer type.
		* @param None.
		* @return Bearer type.
        */
        TInt BearerType();
        
		/**
		* Set bearer type.
		* @param aId.
		* @return None.
        */
        void SetBearerTypeL(TInt aId);

		/**
		* Get access point.
		* @param None.
		* @return Access point.
        */
        TInt AccessPointL();
        
		/**
		* Set access point.
		* @param aId.
		* @return None.
        */
        void SetAccessPointL(const TInt aId);

		/**
		* Get host address.
		* @param aText.
		* @param aPort.		
		* @return None.
        */
        void GetHostAddress(TDes& aText, TInt& aPort);

		/**
		* Set host address.
		* @param aText.
		* @param aPort.		
		* @return None.
        */
        void SetHostAddressL(const TDesC& aText, const TInt aPort);
         
		/**
		* Get user name.
		* @param aText.
		* @return None.
        */
        void GetUserName(TDes& aText);

		/**
		* Set user name.
		* @param aText.
		* @return None.
        */
        void SetUserNameL(const TDesC& aText);
        
		/**
		* Get password.
		* @param aText.
		* @return None.
        */
        void GetPassword(TDes& aText);

		/**
		* Set password.
		* @param aText.
		* @return None.
        */
        void SetPasswordL(const TDesC& aText);
        
		/**
		* Set server alert sync state.
		* @param aState.
		* @return None.
        */
        void SetSASyncStateL(TInt aState);

		/**
		* Get server alert sync state.
		* @param None.
		* @return Server alert sync state.
        */
        TInt SASyncState();

		/**
		* Set http authentication state.
		* @param aEnable.
		* @return None.
        */
        void SetHttpUsedL(TBool aEnable);

		/**
		* Get http authentication state.
		* @param aEnable.
		* @return Boolean.
        */
        TBool HttpUsedL();
         
		/**
		* Get http user name.
		* @param aText.
		* @return None.
        */
        void GetHttpUserNameL(TDes& aText);

		/**
		* Set http user name.
		* @param aText.
		* @return None.
        */
        void SetHttpUserNameL(const TDesC& aText);
 
		/**
		* Get http password.
		* @param aText.
		* @return None.
        */
        void GetHttpPasswordL(TDes& aText);
        
		/**
		* Set http password.
		* @param aText.
		* @return None.
        */
        void SetHttpPasswordL(const TDesC& aText);
 
               
    public:
  		/**
		* Open history log.
		* @param None.
		* @return None.
        */
        void OpenHistoryLog();
        
  		/**
		* Open connection.
		* @param None.
		* @return None.
        */
        void OpenConnection();

  		/**
		* Get RSyncMLDataSyncProfile.
		* @param None.
		* @return RSyncMLDataSyncProfile.
        */
        RSyncMLDataSyncProfile& Profile();
        
  		/**
		* Get RSyncMLSession.
		* @param None.
		* @return RSyncMLSession.
        */
        RSyncMLSession& Session();
        
  		/**
		* Get application id.
		* @param None.
		* @return Application id.
        */
        TInt ApplicationId();
        
    public:
  		/**
		* Get connection property name.
		* @param aText.
		* @param aPropertyPos.
		* @return None.
        */
        void GetConnectionPropertyNameL(TDes8& aText, TInt aPropertyPos);

  		/**
		* Get sync type.
		* @param aSyncTask.
		* @return Sync type.
        */
        static TInt SyncType(RSyncMLTask& aSyncTask);

  		/**
		* Set default bearer.
		* @param None.
		* @return None.
        */
        void SetDefaultBearerL();
        
  		/**
		* Get latest history job.
		* @param None.
		* @return History job.
        */
        const CSyncMLHistoryJob* LatestHistoryJob();
        
  		/**
		* Get latest history job.
		* @param aTaskId.
		* @return History job.
        */
        const CSyncMLHistoryJob* LatestHistoryJob(TInt aTaskId);

 		/**
		* Find out whether history job contains a task.
		* @param aHistoryJob.
		* @return Boolean.
        */
        TBool TaskExist(const CSyncMLHistoryJob* aHistoryJob, TInt aTaskId);
        
  		/**
		* Copy values from another profile.
		* @param aSource.
		* @return None.
        */
        void CopyValuesL(CAspProfile* aSource);
        
  		/**
		* Get default transport id.
		* @param None.
		* @return Transport id.
        */
        TInt DefaultTransportIdL();
        
  		/**
		* Delete all connections.
		* @param aId.
		* @return None.
        */
        //void DeleteConnectionsL();
        
    public:
  		/**
		* Get new profile name.
		* @param aList.
		* @param aApplicationId.
		* @return Profile name.
        */
        static HBufC* GetNewProfileNameLC(CAspProfileList* aList, TInt aApplicationId);
        
  		/**
		* Set default values.
		* @param aProfile.
		* @return None.
        */
        static void SetDefaultValuesL(CAspProfile* aProfile);
        
  		/**
		* Is other sync running.
		* @param aSyncSession.
		* @return Boolean.
        */
        static TBool OtherSyncRunning(RSyncMLSession* aSyncSession);

  		/**
		* Get current sync job.
		* @param aSyncSession.
		* @return Sync job.
        */
        static TInt CurrentJob(RSyncMLSession* aSyncSession);
        
  		/**
		* Is this profile PCSuite profile.
		* @param aProfile.
		* @return Boolean.
        */
        static TBool IsPCSuiteProfile(CAspProfile* aProfile);
        
    public:
  		/**
		* Check mandatory task data.
		* @param aProfile.
		* @param aContentCount.
		* @return Mandatory data status code.
        */
        static TInt CheckMandatoryTaskDataL(CAspProfile* aProfile, TInt& aContentCount);
 
  		/**
		* Check mandatory task data.
		* @param aProfile.
		* @param aContentCount.
		* @return Mandatory data status code.
        */
        static TInt CheckMandatoryTaskData(CAspProfile* aProfile, TInt& aContentCount);
        
 		/**
		* Check mandatory data.
		* @param aProfile.
		* @param aContentCount.
		* @return Mandatory data status code.
        */
        static TInt CheckMandatoryData(CAspProfile* aProfile, TInt& aContentCount);
 
  		/**
		* Check mandatory connection data.
		* @param aProfile.
		* @return Mandatory data status code.
        */
        static TInt CheckMandatoryConnData(CAspProfile* aProfile);
    
    	/**
		* Delete sync history for a profile
		* @param None
		* @return None
        */
 		void DeleteHistory();
 	public:
	    // -----------------------------------------------------------------------------
		// CAspProfile::GetLocalisedPCSuite
		//
		// Returns "PC Suite" string localised to specific language
		// 
		// Current requirement is to localise to 2 languages TaiwanChinese and ELangHongKongChinese
		// For all the rest it will be mapped to "PC suite" only
		// -----------------------------------------------------------------------------
		//
	    void GetLocalisedPCSuite(TDes& aText);
       
    //public:
        //HBufC* HistoryJobsLC();

	private:
        // historylog open state
        TBool iHistoryLogOpen;
        
        // connection open state
        TBool iConnectionOpen;
          
   		// needed for setting creator id
		TInt iApplicationId;
		
		// long buffer for string handling 
		TBuf<KBufSize255> iBuf;
		
		// long buffer for string handling 
		TBuf8<KBufSize255> iBuf8;

   		// sync session
		RSyncMLSession* iSyncSession;
		
		// sync profile
		RSyncMLDataSyncProfile iProfile;
		
		// history log
		RSyncMLHistoryLog iHistoryLog;
				
		// sync connection
		RSyncMLConnection iConnection;
    };


/**
* CAspTask
*
* TAspTask is used for sync task operations.
*/
NONSHARABLE_CLASS (TAspTask) : public CBase
    {
    public:
		static TInt SyncDirection(TSmlSyncType aSmlSyncDirection);
		static TSmlSyncType SmlSyncDirection(TInt aSyncDirection);
        static TInt FindTaskIdL(CAspProfile* aProfile, TInt aDataProviderId, const TDesC& aLocalDatabase);
        static TBool CheckLocalDatabaseL(CAspProfile* aProfile, TInt& aDataProviderId);
        static TBool CheckLocalDatabase(CAspProfile* aProfile, TInt& aDataProviderId);
    };







#endif  // ASPPROFILE_H


// End of file