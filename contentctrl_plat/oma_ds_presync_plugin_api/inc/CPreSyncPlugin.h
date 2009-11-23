/*
* Copyright (c) 2007 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:  ECom search interface definition
*
*/


#ifndef CPRESYNC_PLUGIN_H
#define CPRESYNC_PLUGIN_H

//  INCLUDES
#include <e32base.h>
#include <s32mem.h>
#include <ecom/ecom.h>
#include <SyncMLDef.h>

/**
* CenRep key for storing the roaming setting.
* 
*
* Possible integer values:
* 
* 0 Always Ask
* 1 Always Allow
* 2 Never Allow
* 3 Block
* Default value: 0 (Always Ask)
*
*/
const TUint32 KNSmlRoamingSettingKey = 0x00000005;

/**
* CenRep key for storing the roaming setting feature.
* 
*
* Possible integer values:
* 
* 0 Feature disabled
* 1 Feature enabled
*
*/
const TUint32 KNSmlDSRoamingFeature = 0x00000006;


/**
 * Constant:    KPreSyncPluginInterfaceUid
 *
 * Description: UID of this ECOM interface. 
 */
const TUid KPreSyncPluginInterfaceUid = {0x2001955C};

enum TASpRoamingSettingFeature
    {
    EAspRoamingSettingFeatureDisabled = 0,
    EAspRoamingSettingFeatureEnabled = 1
    };

/**
 * Class:       MSyncExtObserver
 *
 * Description:  
 */

class MSyncExtObserver
{
        public: // Public pure virtual functions.

        /**
         * Function:    SyncCompleted
         *
         */
        virtual void SyncCompleted(TInt err) = 0;
 };


/**
 * Class:       CPreSyncPlugin
 *
 * Description: Custom ECOM interface definition. This interface is used by
 *              clients to handle roamong . 
 */
class CPreSyncPlugin: public CBase
{
public: 
    /**
     * Function   :   NewL(TUid aImplementationUid);
     * Description:   Implementation created based on implementation ID
     */
    static CPreSyncPlugin* NewL(TUid aImplementationUid);
    
    /**
     * Function   :   NewL(const TDesC8& aParam);
     * Description:   Implementation created based Implemetation resolution parameter
     */
    static CPreSyncPlugin* NewL(const TDesC8& aParam);


    /**
     * Function:   ~CPreSyncPlugin
     *
     * Description: Wraps ECom object destruction. Notifies the ECOM
     *              framework that specific instance is being destroyed.
     */
    virtual ~CPreSyncPlugin();


    public: // Public pure virtual functions.

    /**
     * Function:    IsSupported
     *
     */
    virtual TBool IsSupported() = 0;
    
    /**
     * Function:    CanSync
     *
     */
    virtual TBool CanSyncL() = 0;
    
    /**
     * Function:    SetProfile  
     *
     */
    virtual TBool SetProfile(const TSmlProfileId& aProfileId) = 0;
    
    /**
     * Function:    SyncNow  
     *
     */   
    virtual TInt SyncNow(MSyncExtObserver* aObserver) = 0 ;
    
public: 

        /** iDtor_ID_Key Instance identifier key. When instance of an
             *               implementation is created by ECOM framework, the
             *               framework will assign UID for it. The UID is used in
             *               destructor to notify framework that this instance is
             *               being destroyed and resources can be released.
             */
        TUid iDtor_ID_Key;

        
};

#include "CPreSyncPlugin.inl" // Own base implementations for ECOM

#endif		// CPCS_PLUGIN_H

// End of File

