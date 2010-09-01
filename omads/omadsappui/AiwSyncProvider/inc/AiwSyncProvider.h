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
* Description: AIW provider plugin for data sync 
*
*/


#ifndef AIWSYNCPROVIDER_H
#define AIWSYNCPROVIDER_H

//  INCLUDES
#include <AiwServiceIfMenu.h>
#include "AspSyncUtilApi.h"


// CONSTANTS
const TInt KBufSize = 128;
const TInt KSyncApplicationId = 0;



// CLASS DECLARATION


/**
* CAspSyncProvider
*  
* CAspSyncProvider implements CAiwServiceIfMenu interface to
* allow synchronize functionality to be called via AIW.
*
* @lib syncservice.lib
* @since Series 60 3.0
*/
class CAspSyncProvider : public CAiwServiceIfMenu 
	{
	public:
        /**
        * Two-phased constructor.
        */
		static CAspSyncProvider* NewL();

        /**
        * Destructor.
        */
		~CAspSyncProvider();

	public:		
        /**
        * From the base class.
		* Not implemented.
        */
		virtual void InitialiseL(MAiwNotifyCallback& aFrameworkCallback,
			                     const RCriteriaArray& aInterest);

        /**
        * From the base class.
		* Not implemented.
        */
		virtual void HandleServiceCmdL(const TInt& aCmdId,
                                       const CAiwGenericParamList& aInParamList,
                                       CAiwGenericParamList& aOutParamList,
                                       TUint aCmdOptions = 0,
                                       const MAiwNotifyCallback* aCallback = NULL);


     	 /**
         * Initialises menu pane by adding provider specific menu items.
	     * The AIW Framework gives the parameters to be used in addition.
	     * @param aMenuPane Menu pane handle
	     * @param aIndex position of item where to add menu items.
	     * @param aCascadeId ID of cascade menu item.
	     * @param aInParamList input parameter list for provider's parameters checking
	     */
		virtual void InitializeMenuPaneL(CAiwMenuPane& aMenuPane,
                                         TInt aIndex,
                                         TInt aCascadeId,
                                         const CAiwGenericParamList& aInParamList);

        /**
        * Handle a menu command invoked by the Handler.
        * @param aMenuCmdId Command ID for the menu command,
        *        defined by the provider when adding the menu commands.
        * @param aInParamList Input parameters, could be empty list
        * @param aOutParamList Output parameters, could be empty list
        * @param aCmdOptions Options for the command, see KAiwCmdOpt* constants.
        * @param aCallback callback if asynchronous command handling is wanted by consumer.
        *    The provider may or may not support this, leaves with KErrNotSupported, it not.
        */
		virtual void HandleMenuCmdL(TInt aMenuCmdId,
                                    const CAiwGenericParamList& aInParamList,
                                    CAiwGenericParamList& aOutParamList,
                                    TUint aCmdOptions = 0,
                                    const MAiwNotifyCallback* aCallback = NULL);

	private:
        /**
        * C++ default constructor.
        */
		CAspSyncProvider();

        /**
        * By default Symbian OS constructor is private.
        */
		void ConstructL();

	
	private:
        /**
		* Gets service command id from AIW parameter list.
        * @param aInParamList AIW parameter list.
		* @return Service command id.
        */
		TInt GetServiceCommandId(const CAiwGenericParamList& aInParamList);

        /**
		* Gets calling application id from AIW parameter list.
        * @param aInParamList AIW parameter list.
		* @return Application id.
        */
	    TInt GetApplicationId(const CAiwGenericParamList& aInParamList);

        /**
		* Gets sync file name from AIW parameter list.
        * @param aInParamList AIW parameter list.
        * @param aText File name.
		* @return None.
        */
	    void GetFileName(const CAiwGenericParamList& aInParamList, TDes& aText);
	    
	private:
        /**
		* Displays information note.
        * @param aText Information note text.
		* @return None.
        */
	    void ShowInformationNoteL(const TDesC& aText);

        /**
		* Gets this plugin resource file name.
        * @param aText Resource file name.
		* @return None.
        */
		void GetResFileName(TDes& aText);
		
        /**
		* Copies strings.
        * @param aTarget.
        * @param aSource.
		* @return None.
        */
    	void StrCopy(TDes& aTarget, const TDesC& aSource);

	private:
	
		// CSyncUtilApi handles synchronize operations. 
		CSyncUtilApi* iSyncUtilApi;
	};	

#endif  // AIWSYNCPROVIDER_H

// End of file

