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
* Description: Scheduled sync implementation and setting access 
*
*/


#ifndef ASPSCHEDULE_H
#define ASPSCHEDULE_H

// INCLUDES
#include <e32base.h>
#include <SyncMLClient.h>    // for RSyncMLSession
#include <SyncMLObservers.h>

#include "AspDefines.h"
#include "AspUtil.h"


// CONSTANTS
const TInt KDefaultStartHour = 12; // 12 noon
const TInt KStartPeakHour = 8;
const TInt KStartPeakMin = 00;
const TInt KEndPeakHour = 17;
const TInt KEndPeakMin = 00;
_LIT(KAutoSyncProfileName, "AutoSync Profile");


// FORWARD DECLARATIONS
class CAspProfile;
class CAspFilterItem;


/**
* CAspSchedule
*  
*/
NONSHARABLE_CLASS (CAspSchedule) : public CBase
    {
    private:
	enum TRepositoryKey
		{
	   // ERepositoryKeyInt = 1,
	    ERepositoryKeyBin = 1
		};
		
    public:
		
    enum TWeekdayFlags
        {
	    EFlagMonday      = 0x00000001,
	    EFlagTuesday     = 0x00000002,
	    EFlagWednesday   = 0x00000004,
	    EFlagThursday    = 0x00000008,
	    EFlagFriday      = 0x00000010,
	    EFlagSaturday    = 0x00000020,
	    EFlagSunday      = 0x00000040
        };

	enum TSyncInterval
		{
		EIntervalManual = 0,
		EInterval15Mins,
		EInterval30Mins,
		EInterval1hour,
		EInterval2hours,
		EInterval4hours,
		EInterval8hours,
		EInterval12hours,
		EManyTimesPerDay,
		EInterval24hours,
		EInterval2days,
		EInterval4days,
		EInterval7days,
		EInterval14days,
		EInterval30days
		};
	
  
	public:// Constructors and destructor

        /**
        * Two-phased constructor.
        */
        static CAspSchedule* NewL();
        
        /**
        * Two-phased constructor.
        */
        static CAspSchedule* NewLC();

        /**
        * Destructor.
        */
        virtual ~CAspSchedule();
    
    private:

        /**
        * C++ default constructor.
        */	
        CAspSchedule();
	
        /**
        * By default Symbian OS constructor is private.
        */
        void ConstructL();

	public:

		/**
        *  auto sync error
        */	
	    TInt Error();
	    void SetError(TInt aError);

		/**
        *  Profile selected for auto sync
        */	
	    TInt ProfileId();
	    void SetProfileId(TInt aProfileId);

		/**
        *  week day selection 
        */	
	    TBool WeekdayEnabled(TInt aWeekday);
	    void SetWeekdayEnabled(TInt aWeekday, TBool aEnabled);
	    
	    TBool RoamingAllowed();
	    void SetRoamingAllowed(TBool aAllowed);
	    
	    void SelectedDayInfo(TInt& aDayCount, TInt& aWeekday);

		/**
        * Save all settings using a cenrep key
        */	
        void SaveL();

		/**
        * Auto sync content selection
        */	
		void SetContentEnabled(TInt aContent, TInt aEnabled);
	    TBool ContentEnabled(TInt aContent);
	    void ContentSelectionInfo(TInt& aContentCnt, TInt& aContent);

		/**
        * Auto sync interval
        */	
        void SetSyncFrequency(TInt aSchedule);
		void SetSyncPeakSchedule(TInt aSchedule);
		void SetSyncOffPeakSchedule(TInt aSchedule);
		TInt SyncFrequency();
		TInt SyncPeakSchedule();
		TInt SyncOffPeakSchedule();

		/**
        * Peak time definition
        */	
		void SetStartPeakTime(TTime aStartTime);
		void SetEndPeakTime(TTime aEndTime);
		TTime StartPeakTime();
		TTime EndPeakTime();

		/**
        * Auto sync enabled/disabled
        */	
		void SetDailySyncEnabled(TBool aSyncEnabled);
		TBool DailySyncEnabled();
		void SetPeakSyncEnabled(TBool aSyncEnabled);
		TBool PeakSyncEnabled();
		void SetOffPeakSyncEnabled(TBool aSyncEnabled);
		TBool OffPeakSyncEnabled();

		/**
        * Sync time for intervals >= a day
        */	
		void SetAutoSyncScheduleTime(TTime aSchedule);
		TTime AutoSyncScheduleTime();	

		/**
        * Schedule handle returned by task scheduler during schedule creation
        */	
		void SetPeakScheduleHandle(TInt aHandle);
		TInt PeakScheduleHandle();
		void SetOffPeakScheduleHandle(TInt aHandle);
		TInt OffPeakScheduleHandle();

		/**
        * A hidden profile used for auto sync.Contents are set as the content selected in auto sync settings
        *
        */	

		void CreateAutoSyncProfileL();
		TInt AutoSyncProfileId();
		void SetAutoSyncProfileId(TInt aProfileId);
		void CopyAutoSyncContentsL();
		void UpdateProfileSettingsL();
		
		/**
        * Check validity of shedule based on current time and user defined peak time
        *
        */					
		TInt IsValidPeakScheduleL();
		TInt IsValidOffPeakScheduleL();

		TBool IsValidNextPeakScheduleL();
		TBool IsValidNextOffPeakScheduleL();

		/**
        * Post pone schedule until next valid time
        */
		void UpdatePeakScheduleL();
		void UpdateOffPeakScheduleL();
		TIntervalType IntervalType(TInt aInterval);

		void EnableScheduleL();

		/**
        * Check if any valid mail box exists
        */
		TBool MailboxExistL();

		/**
        * Check mandatory settings for the selected profile
        */
		TInt CheckMandatoryDataL(TInt& count,TInt& firstItem);

		/**
        * Log view opened by clicking View Details link in auto sync error note
        */
		void ShowAutoSyncLogL();

		/**
        *SyncML sever version of selected profile
        */
		TInt ProtocolL();

		/**
        *Server Id modifications so that SAN selects correct profile
        */
		void UpdateServerIdL();

		/**
        *Check if atleast one valid content selected
        */
		TBool IsContentSelected();
		
		/**
        *Check if auto sync is enabled
        */
		TBool IsAutoSyncEnabled();

		/**
        *Initialize contents
        */
		void InitializeContentsL();

		
		/**
        *Check the validity of email selection.
        */
		void CheckEmailSelectionL();

				
    public:

		/**
        *Update the schedule settings with symbian task scheduler
        */
        void UpdateSyncScheduleL();
        void UpdateSyncSchedule();
        void GetStartTimeL(TTime& aTime, TBool aHomeTime);
               
      
#ifdef _DEBUG
		void LogSettings();
#endif


    public:
   	    TBool CanSynchronizeL();
		
	private:
	    void InitializeL();
	
	public:
        static TTime UniversalStartTime(TInt& aStartHour);
        static TTime LocalStartTime(TInt aStartHour);
        
	
	private:
	    void InternalizeL();
		void DoInternalizeL();
		void InternalizeL(RReadStream& aStream);
		void ExternalizeL();
		void ExternalizeL(RWriteStream& aStream);
		void WriteRepositoryL(TInt aKey, const TDesC8& aValue);
		void ReadRepositoryL(TInt aKey, TDes8& aValue);
		void ResetRepositoryL();
		
	private:
		TInt WeekdayFlag(TInt aWeekday);
		TBool IsDaySelected(TTime aTime);
	
		void OpenSyncSessionL();
		void CloseSyncSession();

		
#ifdef _DEBUG        
	public:
	    void LogScheduleL();
	    void LogSyncDays();
	    void LogSyncInterval();	


    public:
        static void PrintTimeL(TTime aTime);
        void TestL();
        void DoTestL();
        void UpdateValuesL(TInt aCount);
        CAspSchedule* CreateCopyLC();
        void CompareValues(CAspSchedule* aSchedule);
        void AssertEqual(TInt aValue1, TInt aValue2);
#endif

        	    
		
    private:
   
         // error code
        TInt iError;

		// profile id
		TInt iProfileId;
		
		// selected days
		TUint32 iWeekdayFlags;
		
		// start hour
		TInt iStartHour;

		// is roaming allowed
		TBool iRoamingAllowed;
		
		// data provider id list
		RArray<TInt> iList;
		
        // long buffer for string handling 
		TBuf<KBufSize255> iBuf;
		
		// have settings changed
		TBool iSettingChanged;

		//content selection flags
		TUint32 iContentFlags;
		
		TInt iStartPeakTimeHr;
		TInt iStartPeakTimeMin;
		TInt iEndPeakTimeHr;
        TInt iEndPeakTimeMin;		

		//peak interval
		TInt iSyncPeakSchedule;

		//off-peak interval
		TInt iSyncOffPeakSchedule;

		
		TBool iPeakSyncEnabled;
		TBool iOffPeakSyncEnabled;

		//A new RSyncMLProfileBase profile is created for auto sync
		//Selected Auto sync contents are set as the task items for this profile
		//All the settings for this profie are copied from the profile selected by user.
		
		TInt iAutoSyncProfileId;
		
        //Sync time for once a day sync
		TInt iAutoSyncScheduleTimeHr;
		TInt iAutoSyncScheduleTimeMin;

		//schedule handles.
        TInt iPeakScheduleHandle;
		TInt iOffPeakScheduleHandle;

		// sync session
		RSyncMLSession iSyncSession;

		// is sync session open
		TBool iSyncSessionOpen;

		// Daily sync setting
		TBool iDailySyncEnabled;
		TInt iSyncFrequency;

		
    };

#endif  // ASPSCHEDULE_H

// End of file