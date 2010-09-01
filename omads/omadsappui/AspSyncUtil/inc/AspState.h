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
* Description: Utility class to handle sync phases during progress 
*
*/


#ifndef ASPSTATE_H
#define ASPSTATE_H


//  INCLUDES
#include <e32base.h>


// CONSTANTS

const TInt KTotalProgressUnknown = -1;
const TInt KTotalProgress = 100;


/**
* CAspState
*
* CAspState contains sync state information.
*/
NONSHARABLE_CLASS (CAspState) : public CBase
    {
    public:
		enum TSyncPhase
			{
			EPhaseConnecting = 0,
			EPhaseInitialising,
			EPhaseSending,
			EPhaseReceiving,
			EPhaseSynchronizing,
			EPhaseDisconnecting,
			EPhaseCanceling,
			EPhaseNone
			};


    public:
        /**
        * Two-phased constructor.
        */
        static CAspState* NewL();
        
        /**
        * Destructor.
        */
        ~CAspState();
        
    private:
        /**
        * C++ default constructor.
        */
        CAspState();
        
        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL();
        
    public:
        /**
        * Reset state variables.
        * @param None.
        * @return None.
        */
        void Reset();
        
    public:
        /**
        * Set synchronization phase.
        * @param aPhase synchronization phase.
        * @return None.
        */
        void SetSyncPhase(TInt aPhase);
        
        /**
        * Get synchronization phase.
        * @param None.
        * @return Synchronization phase.
        */
        TInt SyncPhase();

        /**
        * Set total sync progress.
        * @param aCount Progress count.
        * @return None.
        */
		void SetTotalProgress(TInt aCount);

        /**
        * Set sync progress.
        * @param aCount Progress count.
        * @return None.
        */
		//void SetProgress(TInt aCount);

        /**
        * Get total sync progress.
        * @param None. 
        * @return Progress count.
        */
		TInt TotalProgress();

        /**
        * Increase sync progress.
        * @param None.
        * @return None.
        */
        void IncreaseProgress();
        
        /**
        * Increase sync progress.
        * @param aCount.
        * @return None.
        */
        void IncreaseProgress(TInt aCount);
        
        /**
        * Get sync progress.
        * @param None. 
        * @return Progress count.
        */
		TInt Progress();

        /**
        * Has IncreaseProgress been called once.
        * @param None. 
        * @return Boolean.
        */
		TBool FirstProgress();

        /**
        * Set sync content name.
        * @param aText Content name.
        * @return None.
        */
        void SetContentL(const TDesC& aText);

        /**
        * Get sync content name.
        * @param None. 
        * @return Content name.
        */
		const TDesC& Content();

        /**
        * Resets progress state.
        * @param None. 
        * @return None.
        */
		void ResetProgress();
		
        /**
        * Finds out whether total progress count is known.
        * @param None. 
        * @return ETrue if total progress count is known, EFalse otherwise.
        */
		TBool ProgressKnown();
        
    private:
        // synchronization phase
		TInt iSyncPhase;

		// total number of sent/received sync items
		TInt iTotalProgressCount;

		// number of sent/received sync items
		TInt iProgressCount;
		
		// number of increase progress calls, can different than iProgressCount
		TInt iProgressCount2;

		// name of the content being synced
		HBufC* iSyncContent;
    };

#endif  // ASPSTATE_H

// End of File
