/*
* Copyright (c) 2002 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:  This class implements ECom interface for COmaDsAppUiPlugin.dll
*
*/


#ifndef _NSML_DS_GS_PLUGIN_H_
#define _NSML_DS_GS_PLUGIN_H_

// System includes
#include <gsplugininterface.h>
#include "AspSyncUtilApi.h"

#include <AknServerApp.h> 

#define KUidNsmlDSGSPluginDll   0x102833A1
#define KUidNsmlDSGSPluginImpl  0x102833A2

// Use this UID in registry file as well.
// UID belongs omadsapp.exe
#define KDsAppUid 0x101F6DE4
    
class CAknNullService;

/**
* DsGS ECom Plugin
*/
class COmaDsAppUiPluginInterface : public CGSPluginInterface, 
								   public MAknServerAppExitObserver
    {    
	public: // Constructors & destructors

	    /**
	    * Two-Phased constructor
	    * Creates new DsUi plugin.
	    * Uses Leave code KErrNotFound if implementation is not found.
	    */                                                 
	    static CGSPluginInterface* NewL();
	    
	    /**
	    * From CAknView Returns this view Id.
	    */
		TUid Id() const;
	    
	    /**
	    * returns the caption to be displayed in GS view
	    *
	    * @param aCaption - The caption to be displayed is to be copied here.
	    */
	    void GetCaptionL( TDes& aCaption ) const;
		
		/**
		* returns the icon to be displayed in GS view
		* 
		* @param aIconType - type of icon to be displayed
		*/
	    CGulIcon* CreateIconL( const TUid aIconType );
	    
	    /**
	    * Destructor
	    */
	    ~COmaDsAppUiPluginInterface();   

    	/**
        * From CGSPluginInterface.
        * @return this plugin's provider category.
        */
        TInt PluginProviderCategory() const;

        /**
        * From CGSPluginInterface.
        * @return this plugin's item type.
        */
        TGSListboxItemTypes ItemType();
         
	    /**
        * From CGSPluginInterface.
        * @param aSelectionType selection type.
        */
        void HandleSelection( const TGSSelectionTypes aSelectionType );
	
	protected:        
	    
	    /**
	    * From CAknView Activates this view.
	    */
		void DoActivateL(const TVwsViewId& aPrevViewId,
		    			 TUid aCustomMessageId, 
		    			 const TDesC8& aCustomMessage);		
		/**
	    * From CAknView Deactivates this view.
	    */
		void DoDeactivate();		

	private:
	  	void ConstructL();
		
		/**
		* Constructor
		*/
	    COmaDsAppUiPluginInterface();
	
        void LaunchDsAppL() ;

	private:
			
		CEikonEnv* env;
		TInt       iResId;
		/**
        * AVKON NULL Service.
        * Own.
        */
        CAknNullService* iNullService;
};

#endif //_NSML_DS_GS_PLUGIN_H_

//End of file
