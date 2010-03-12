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
* Description: General utilities 
*
*/



#ifndef ASPUTIL_H
#define ASPUTIL_H

// INCLUDES
#include <e32base.h>
#include <ecom.h>
#include <eikenv.h> 


#include <ApSettingsHandlerUI.h>

#include <SyncMLClient.h>    // for RSyncMLSession
#include <SyncMLClientDS.h>  // for RSyncMLDataSyncProfile

#include <csch_cli.h> // RScheduler


#include "AspDefines.h"


// CONSTANTS



// FORWARD DECLARATIONS
class CAspProfile;
class CAspProfileList;
class CAspContentList;
class CAspTask;
class CAspFilter;
class CAspSchedule;

// CLASS DECLARATION


/**
* TAspParam
*
* This class is used as general parameter type.
*/
NONSHARABLE_CLASS (TAspParam)
	{
    public:
		// calling application id
		TInt iApplicationId;

		// profile id
		TInt iProfileId;

		// dialog mode
		TInt iMode;

		// general id number
		TInt iId;

		// return value to caller.
		TInt iReturnValue;
		
		// data provider id
		TInt  iDataProviderId;
		
		// sync task id
		TInt  iSyncTaskId;
		
		// sync profile
		CAspProfile* iProfile;
		
		// sync task
		CAspTask* iSyncTask;
		
		// sync filter
		CAspFilter* iSyncFilter;
		
		// session with sync server
		RSyncMLSession* iSyncSession;
		
		// profile list
		CAspProfileList* iProfileList;
		
		// content list
		CAspContentList* iContentList;

		//auto sync schedule setting
		CAspSchedule* iSchedule;
		

    public:
    	TAspParam(TInt aApplicationId);
		TAspParam(TInt aApplicationId, RSyncMLSession* aSyncSession);
	};





/**
* TAspUtil
* 
* This class contains general utility functions. 
*/
NONSHARABLE_CLASS (TUtil)
	{
    public:
        static void Panic(TInt aReason);
		static void StrCopy(TDes& aTarget, const TDesC& aSource);
		static void StrCopy(TDes& aTarget, const TDesC8& aSource);
		static void StrCopy(TDes8& aTarget, const TDesC& aSource);
		static HBufC* StrConversion(const TDesC& aText);
		
		static void StrAppend(TDes& aTarget, const TDesC& aSource);
		static void StrInsert(TDes& aTarget, const TDesC& aSource);
		static void Fill(TDes& aTarget, const TDesC& aOneCharStr, TInt aLength);
		static TInt StrToInt(const TDesC& aText, TInt& aNum);
		static TInt AddResFileL(const TDesC& aFile);
		static TBool IsEmpty(const TDesC& aText);
		
		static TInt ProviderIdFromAppId(TInt aApplicationId);
		static TInt ProviderIdFromAppId(TInt aApplicationId, TBool aOperatorProfile);
		static TInt OperatorProviderIdFromAppId(TInt aApplicationId);
		static TInt AppIdFromProviderId(TInt aAdapterId);
		static TInt AppIdFromOperatorSpecificProviderId(TInt aAdapterId);
		static TBool IsValidAppId(TInt aApplicationId);
		static TBool IsValidSID(TInt aApplicationId);
		
		static void GetDateTextL(TDes& aText, TTime aDateTime);
		static void GetTimeTextL(TDes& aText, TTime aDateTime);
		static HBufC* SyncTimeLC(TTime aLastSync);
		static void GetDateTimeTextL(TDes& aText, TTime aDateTime);
		static TBool IsToday(TTime aTime);
		static TTime ConvertUniversalToHomeTime(const TTime& aUniversalTime);
		static void LaunchHelpAppL();
		static void LaunchHelpAppL(CEikonEnv* aEikonkEnv);
		static TBool SettingEnforcementStateL();
		static TBool SettingEnforcementState();
		static CDesCArray* NumberConversionLC(CDesCArray* aList);
		
		


#ifdef _DEBUG		
    public:		
		static void GetMandatoryFieldText(TDes& aText, TInt aStatus);
		static void Print(const TDesC& aText);
		static void Print(const TDesC& aText, TInt aNum);
		
		static TTime TimeBefore();
		static TInt TimeAfter(TTime aBefore);
		static void TimeAfter(TTime aBefore, const TDesC& aText);
		static void GetApplicationName(TDes& aText, TInt aApplicationId);
#endif		
	};



/**
*  TStr contains descriptor operations with range checking.
*/
NONSHARABLE_CLASS (TStr)
    {
    public:
        TStr(const TDesC& aStr);
       
    public:
        TPtrC Mid(TInt aPos, TInt aLen);
        TPtrC Mid(TInt aPos);
        TPtrC Left(TInt aPos);
        TPtrC Right(TInt aPos);
        TPtrC Trim();
        TPtrC SubStr(TInt aStartPos, TInt aEndPos);
        TPtrC SubStrEx(TInt aStartPos, TInt aEndPos);
        TInt LastPos();
        TInt Len();
        TBool Compare(TInt aPos, TChar aChar);
        TInt Find(TInt aPos, TChar aChar);
        
    public:	
        TInt FirstNonSpace();
        TInt LastNonSpace();
        
    private:
        // reference to descriptor
		const TDesC& iStr;
    };


/**
*  TURIParser is used for parsing URI.
*/
NONSHARABLE_CLASS (TURIParser)
    {
    public:
    enum TDefaultPortNumber
	    {
	    EDefaultHttpPort = 80,
        EDefaultHttpsPort = 443
    	};

    enum TPortNumberRange
	    {
        EMinURIPortNumber = 1,
        EMaxURIPortNumber = 65535
    	};

    public:
        TURIParser(const TDesC& aStr);
       
    public:
        void GetUriWithoutPort(TDes& aText);
        void GetUriWithPort(TDes& aText, const TInt aPort);
        TInt Port();
        TInt DefaultPort();
        static TBool IsValidUri(const TDesC& aText);
        static TBool IsValidBluetoothHostAddress(const TDesC& aText);
        static TBool IsValidPort(const TInt aPort);
        
    private:
        TPtrC ParseHttp();
        TPtrC ParseAddress();
        TPtrC ParseDocument();
        TInt ParsePort(TInt& aPortNumber);
        TPtrC ParsePort();
        
     private:
        void GetHttpPos(TInt& aStartPos, TInt& aEndPos);
        void SkipHttp();
  
    private:
        // reference to original URI text
		const TDesC& iOriginalStr;
		
		// pointer to original URI text
		TPtrC iPtr;
		
    };


/**
*  TFlag contains integer flag operations.
*/
class TFlag
    {
    public:
        /**
        * C++ default constructor.
        */
        TFlag(TUint32& aFlag);
    public:
        /**
        * Turns given bit(s) on.
        * @since 2.5
        * @param aValue Bits to be turned on.
        * @return None.
        */
        void SetOn(const TUint32 aValue);

        /**
        * Turns given bit(s) off.
        * @since 2.5
        * @param aValue Bits to be turned off.
        * @return None.
        */
        void SetOff(const TUint32 aValue);

        /**
        * Checks if given bit(s) is/are set.
        * @since 2.5
        * @param aValue Bitmap to be checked.
        * @return None.
        */
        TBool IsOn(const TUint32 aValue) const;
        
    private:
		// reference to flag (integer)
        TUint32& iFlag;
    };


/**
* CAspIdleWait
* 
* This class is used for waiting synchronously until
* active scheduler has completed all pending events.
*/
NONSHARABLE_CLASS (CAspIdleWait) : public CBase
	{
    public:
        /**
        * Two-phased constructor.
        */
		static CAspIdleWait* NewL();

        /**
        * Destructor.
        */
		virtual ~CAspIdleWait();

    private:

        /**
        * C++ default constructor.
        */
		CAspIdleWait();

	public:
		/**
		* Waits until active schedure has nothing to do.
		* @param  None.
		* @return None.
		*/
		static void CAspIdleWait::WaitL();
		
    private:
        static TInt IdleCallback(TAny* aThisPtr);        

	private:
		CActiveSchedulerWait iWait;
	};




/**
* MAspActiveCallerObserver
*
* MAspActiveCallerObserver is for observing CAspActiveCaller.
*/
class MAspActiveCallerObserver
    {
    public:
	    virtual void HandleActiveCallL(TInt aCallId) = 0;
    };


/**
* CAspActiveCaller
*
* CAspActiveCaller is used for generating a call from active scheduler.
*/
NONSHARABLE_CLASS (CAspActiveCaller) : public CActive
    {
    public:
        /**
        * Two-phased constructor.
        */
        static CAspActiveCaller* NewL(MAspActiveCallerObserver* aObserver);

	    /**
        * Destructor.
        */
        virtual ~CAspActiveCaller();

    private:
        /**
        * C++ default constructor.
        */
	    CAspActiveCaller(MAspActiveCallerObserver* aObserver);

        /**
        * By default Symbian 2nd phase constructor is private.
        */
	    void ConstructL();

	public:
        /**
        * Calls observer using active object framework.
        * @param aCallId.
		* @param aMilliseconds Delay before observer is called.
        * @return None
        */
		void Start(TInt aCallId, TInt aMilliseconds);

        /**
        * Make active scheduler call RunL.
        * @param None
        * @return None
        */
		void Request();

    private:
        /**
        * From CActive. Called when asynchronous request completes.
        * @param None
        * @return None
        */
	    void RunL();

        /**
        * From CActive. Cancels asynchronous request.
        * @param None
        * @return None
        */
	    void DoCancel();
        
    private:
		// call that is passed to observer
		TInt iCallId;

		// timer
		RTimer iTimer;

		// observer that gets called
		MAspActiveCallerObserver* iObserver;
    };





/**
* TAspBearerItem 
*
*/
NONSHARABLE_CLASS (TAspBearerItem)
	{
	public:
		TInt iBearerType;
    	TBuf<KBufSize> iBearerName;
	};



/**
* CAspBearerHandler 
*
* CAspBearerHandler is for constructing list control for selecting
* bearer type (internet, bluetooth etc.)
*/
NONSHARABLE_CLASS (CAspBearerHandler) : public CBase
    {
    public:
        /**
        * Two-phased constructor.
        */
        static CAspBearerHandler* NewL(const TAspParam& aParam);

	    /**
        * Destructor.
        */
        virtual ~CAspBearerHandler();

    private:
        /**
        * C++ default constructor.
        */
	    CAspBearerHandler(const TAspParam& aParam);

        /**
        * By default Symbian 2nd phase constructor is private.
        */
	    void ConstructL();

	public:
	    /**
        * Finds out whether bearer type is supported.
		* @param aBearer Bearer type.
        * @return ETrue if supported, EFalse otherwise.
        */
		TBool IsSupported(TInt aBearer);

	    /**
        * Finds out bearer type for list item.
		* @param aIndex List index.
        * @return Bearer type.
        */
		TInt BearerForListIndex(TInt aIndex);

		/**
        * Finds out list position for aBearer.
		* @param aBearer.
        * @return List array index.
        */
		TInt ListIndexForBearer(TInt aBearer);

		/**
        * Creates supported bearer list.
		* @param None.
        * @return None.
        */
		void CreateSupportedBearerListL();

	    /**
        * Finds out default bearer for a device.
		* @param None.
        * @return Bearer type.
        */
		TInt DefaultBearer();

	    /**
        * Gets bearer name string.
		* @param aText.
		* @param aBearer.
        * @return None.
        */
		void GetBearerName(TDes& aText, TInt aBearerType);

	    /**
        * Builds array of bearer names for ui list control.
		* @param None.
        * @return Bearer name array.
        */
		CDesCArray* BuildBearerListLC();

	    /**
        * Finds out how many bearer types are supported.
		* @param None.
        * @return Bearer count.
        */
		TInt SupportedBearerCount();
		
	    /**
        * Convert asp bearer id to sml beareer id.
		* @param aId.
        * @return Bearer id.
        */
		static TInt SmlBearerId(const TInt aId);

	    /**
        * Convert sml bearer id to asp beareer id.
		* @param aId.
        * @return Bearer id.
        */
		static TInt AspBearerId(const TInt aId);

    private:
   		// sync session
		RSyncMLSession* iSyncSession;

		// list of bearers that can shown in ui list
		RArray<TAspBearerItem> iList;
    };


/**
* TAspAccessPointItem 
*
*/
NONSHARABLE_CLASS (TAspAccessPointItem)
	{
	public:
		TInt  iUid;
    	TBuf<KBufSize> iName;
    	
    	TInt  iUid2;
	};



/**
* CAspAccessPointHandler 
*
* CAspAccessPointHandler is for setting internet access points.
*/
NONSHARABLE_CLASS (CAspAccessPointHandler) : public CBase
    {
    public:
	enum TExitMode
		{
	    EAspDialogSelect,
	    EAspDialogCancel,
	    EAspDialogExit
		};
		
	// when ap is set to -1, ap is always asked before sync
    static const TInt KAskAlways = -1;

	static const TInt KDefaultConnection = -2;
    
    public:
        /**
        * Two-phased constructor.
        */
        static CAspAccessPointHandler* NewL(const TAspParam& aParam);

	    /**
        * Destructor.
        */
        virtual ~CAspAccessPointHandler();

    private:
        /**
        * C++ default constructor.
        */
	    CAspAccessPointHandler(const TAspParam& aParam);

        /**
        * By default Symbian 2nd phase constructor is private.
        */
	    void ConstructL();

    public:
	    /**
        * Displays dialog for selecting one access point.
		* @param aItem.
        * @return ETrue if user selected access point, EFalse otherwise.
        */
        TInt ShowApSelectDialogL(TAspAccessPointItem& aItem);
        
	    /**
        * Gets access point info.
		* @param aItem.
        * @return Error value.
        */
		TInt GetInternetApInfo(TAspAccessPointItem& aItem);

	private:
	    /**
        * Gets access point info.
		* @param aItem.
        * @return Error value.
        */
		TInt GetApInfo(TAspAccessPointItem& aItem);

	    /**
        * Gets access point info.
		* @param aItem.
        * @return Error value.
        */
		TInt GetApInfoL(TAspAccessPointItem& aItem);

        /**
        * Utility function.
		* @param None.
        * @return Bearer flags.
        */
		TInt BearerFlags();


    private:
    
   		// communications database
		CCommsDatabase* iCommDb;

    	//access point utilities
    	CApUtils* iApUtil;
    	
    	// access point selection utility
    	CApSelect* iApSelect;
    	
    	// dialog parameters
    	TAspParam* iParam;
    	
    };



/**
* TAspSyncError 
*
* TAspSyncError is used for error code handling.
*/
NONSHARABLE_CLASS (TAspSyncError)
    {
    
    public:
        static HBufC* GetSmlErrorTextL(TInt aErrorCode, TInt aTransport);
        static HBufC* GetSmlErrorText(TInt aErrorCode, TInt aTransport);
      	static TInt GetSmlErrorValue(TInt aErrorCode);
        
#ifdef _DEBUG        
    public:
        static void GetSyncMLErrorText(TDes& aText, TInt aError);
#endif
     
    };




#endif  // ASPUTIL_H


// End of file