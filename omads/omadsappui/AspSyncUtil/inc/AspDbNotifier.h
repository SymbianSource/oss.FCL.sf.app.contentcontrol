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
* Description: Used by UI to update settings based on DB changes 
*
*/


#ifndef ASPDBNOTIFIER_H
#define ASPDBNOTIFIER_H

#include <e32std.h>
#include <SyncMLObservers.h>

#include "AspUtil.h"


// CLASS DECLARATION


/**
* TAspDbEvent
*
* TAspDbEvent represents sync event that is sent to UI.
*/
NONSHARABLE_CLASS (TAspDbEvent)
	{
	public:
		TInt iType;	
		TInt iProfileId;
		TInt iError;

    public:
		/**
        * Constructor
        * @param aType Event type.
        * @return None. 
        */
    	TAspDbEvent(TInt aType);
	};



/**
* MAspDbEventHandler
*
* Observer interface function.
*/
NONSHARABLE_CLASS (MAspDbEventHandler)
	{
	public:
		/**
        * CAspDbNotifier calls this to inform that database has changed.
        * @param aEvent Database event.
        * @return None. 
        */
		virtual void HandleDbEventL(TAspDbEvent aEvent) = 0;
	};


/**
* CAspDbNotifier
* 
* CAspDbNotifier is used to observe sync database events. These events
* are used to update UI.
*/
NONSHARABLE_CLASS (CAspDbNotifier) : public CBase,
                                     public MAspActiveCallerObserver,
                                     public MSyncMLEventObserver
	{
    public:
	    enum TEventType
			{
			EUpdate,
			EUpdateAll,
			EDelete,
	        EClose
			};

	public:
		/**
        * Two-phased constructor
        * @param aDatabase Reference to RDbDatabase instance.
		* @param aHandler Pointer to MAspDbEventHandler instance.
        * @return None. 
        */
		static CAspDbNotifier* NewL(const TAspParam& aParam, MAspDbEventHandler* aHandler);

        /**
        * Destructor.
        */
		virtual ~CAspDbNotifier();

	private:

		/**
        * Constructor
        * @param aParam.
		* @param aHandler Pointer to event handler.
        * @return None. 
        */
		CAspDbNotifier(const TAspParam& aParam, MAspDbEventHandler* aHandler);

        /**
        * By default Symbian OS constructor is private.
        */
		void ConstructL();

	public:
		/**
		* Start requesting events from db.
        * @param None. 
        * @return None. 
        */
		void RequestL();

   
	private:   // from MAspActiveCallerObserver
	
		/**
        * From MAspActiveCallerObserver
		* Called when CAspActiveCaller completes.
        * @param aCallId.
        * @return None.
        */
		void HandleActiveCallL(TInt aCallId);

    private:    // from MSyncMLEventObserver
        
		/**
        * Called when SyncML session events oocur.
        * @param aEvent Event type.
        * @param aIdentifier Object id.
        * @param aError Error code.
        * @param aAdditionalData Additional data.
        * @return None.
        */
        void OnSyncMLSessionEvent(TEvent aEvent, TInt aIdentifier, TInt aError, TInt aAdditionalData);
        
		/**
        * Called when SyncML session events oocur.
        * @param aEvent Event type.
        * @param aIdentifier Object id.
        * @param aError Error code.
        * @param aAdditionalData Additional data.
        * @return None.
        */
        void HandleSessionEventL(TEvent aEvent, TInt aIdentifier, TInt aError, TInt aAdditionalData);

    public:
		/**
        * Calls database event observer with delay.
        * @param None.
        * @return None.
        */
		void CallObserverWithDelay();

		/**
        * Calls database event observer.
        * @param None.
        * @return None.
        */
		void CallObserver();
		
		/**
        * Disable/enable observer calling.
        * @param aDisable.
        * @return None.
        */
		void SetDisabled(TBool aDisable);

		/**
        * Finds out how many profiles have changed.
        * @param None.
        * @return Profile count.
        */
		TInt ChangedProfileCount();
		
		/**
        * Get one database event from event array.
        * @param aIndex.
        * @return Database event.
        */
        TAspDbEvent Event(TInt aIndex);
        
		/**
        * Return number of received database events.
        * @param None.
        * @return Event count.
        */
		TInt EventCount();

		/**
        * Reset event array.
        * @param None.
        * @return None.
        */
		void Reset();

		/**
        * Find out whether event array contains event that causes UI to close.
        * @param None.
        * @return Array index.
        */
		TInt FindCloseEvent();

		/**
        * Find out whether event array contains events for one profile only.
        * @param None.
        * @return Array index.
        */
		TInt FindSingleProfileEvent();
		
		/**
        * Find out whether event array contains profile event.
        * @param None.
        * @return Array index.
        */
		TInt FindProfileEvent();
		
		/**
        * Find out whether event array only contains events with same profile id.
        * @param aId Profile id.
        * @return Boolean.
        */
		TBool IsUniqueProfileId(TInt aId);
		
		/**
        * Creates one update event.
        * @param aIdentifier Profile id.
        * @param aError Error code.
        * @return None.
        */
		void CreateUpdateEventL(TInt aIdentifier, TInt aError);
		
		/**
        * Find out whether event array contains delete event.
        * @param None.
        * @return Boolean.
        */
		TBool ProfileDeleted();
		
		/**
        * Call event observer if event array contains update event.
        * @param None.
        * @return None.
        */
        void CheckUpdateEventL();
		
		/**
		* Utility function.
		* @return Sync session.
        */
		RSyncMLSession& Session();
		
		
#ifdef _DEBUG
    private:
        void GetSyncEventText(TDes& aText, MSyncMLEventObserver::TEvent aEvent);
        void LogSessionEvent(TEvent& aEvent, TInt aIdentifier, TInt aError);
#endif
		


	private:  
		// sync session
		RSyncMLSession* iSyncSession;
	
		// Pointer to event handler
		MAspDbEventHandler* iHandler;

		// for making function call via active scheduler
		CAspActiveCaller* iActiveCaller;

		// for storing ids of changed profiles
		RArray<TAspDbEvent> iList;

		// can observer be called
		TBool iDisabled;

		// is synchronizing going on
		//TBool iSyncRunning;
		
		// have sync session events been requested from RSyncMLSession
		TBool iSessionEventRequested;
		
		// is this object destroyed
		TBool* iDestroyedPtr;
	};


#endif   // ASPDBNOTIFIER_H


// End of file