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
* Description: Datasync app view container
*
*/


#ifndef NSMLDSSYNC_CONTAINER_H
#define NSMLDSSYNC_CONTAINER_H

// INCLUDES
#include <coecntrl.h>


// CLASS DECLARATION


/**
*  Class CNSmlDSSyncContainer.
*  
*/
class CNSmlDSSyncContainer : public CCoeControl, MCoeControlObserver
    {
    public: // Constructors and destructor
        
        /**
        * By default Symbian OS constructor is private.
        */
        void ConstructL(const TRect& aRect);

        /**
        * Destructor.
        */
        ~CNSmlDSSyncContainer();
        
    private: // Functions from base classes

       /**
        * From CoeControl,SizeChanged.
		* @param None. 
    	* @return None.
        */
        void SizeChanged();

       /**
        * From CoeControl,CountComponentControls.
		* @param None. 
    	* @return Component count.
        */
        TInt CountComponentControls() const;

       /**
        * From CCoeControl,ComponentControl.
		* @param aIndex. 
    	* @return CCoeControl.
        */
        CCoeControl* ComponentControl(TInt aIndex) const;

       /**
        * From CCoeControl,Draw.
		* @param aEventType	The type of control event 
    	* @return None
        */
        void Draw(const TRect& aRect) const;
       	
		/**
		* From MCoeControlObserver
		* Acts upon changes in the hosted control's state. 
		* @param aControl The control changing its state
		* @param aEventType	The type of control event 
    	* @return None
		*/
        void HandleControlEventL(CCoeControl* aControl,TCoeEvent aEventType);
        
		/**
		* From base class.
		* Called when UI layout changes. 
		* @param aType.
    	* @return None.
		*/
        void HandleResourceChange(TInt aType);
    };


#endif  // NSMLDSSYNC_CONTAINER_H



// End of File
