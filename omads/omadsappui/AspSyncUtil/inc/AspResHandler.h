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
* Description: Utility methods to handle resources 
*
*/


#ifndef ASPRESHANDLER_H
#define ASPRESHANDLER_H


// INCLUDES
#include <e32base.h>
#include <eikenv.h>
#include <aknenv.h>
#include <StringLoader.h>

#include "AspDefines.h"
#include <AspSyncUtil.rsg>




// CLASS DECLARATION



/**
* CAspResHandler
*
* This class is used for reading aspsyncutil resources.
*/
NONSHARABLE_CLASS (CAspResHandler) : public CBase
    {
    public:
        /**
        * Two-phased constructor.
        */
        static CAspResHandler* NewL();

	    /**
        * Destructor.
        */
        ~CAspResHandler();

    private:
        /**
        * C++ default constructor.
        */
	    CAspResHandler();

        /**
        * By default Symbian 2nd phase constructor is private.
        */
	    void ConstructL();

	
    public:
	   /**
        * Finds out aspsyncutil resource file name.
        * @param aText.
		* @return None.
        */
        static void GetResFileName(TDes& aText);

	   /**
        * Finds out aspsyncutil bitmap file name.
        * @param aText.
		* @return None.
        */
        static void GetBitmapFileName(TDes& aText);
        
	   /**
        * Reads string from resource file.
        * @param aText Text read from resource file.
		* @param aResourceId Resource id.
		* @return None.
        */
		static void ReadL(TDes& aText, TInt aResourceId);

	   /**
        * Reads string from resource file.
        * @param aText Text read from resource file.
		* @param aResourceId Resource id.
		* @param aSubString Substring that is added into read resource.
		* @return None.
        */
		static void ReadL(TDes& aText, TInt aResourceId, const TDesC& aSubString);

	   /**
        * Reads string from resource file.
    	* @param aResourceId Resource id.
		* @return Text read from resource file.
        */
		static HBufC* ReadLC(TInt aResourceId);

	   /**
        * Reads string from resource file.
    	* @param aResourceId Resource id.
		* @return Text read from resource file.
        */
		static HBufC* ReadL(TInt aResourceId);

	   /**
        * Reads string from resource file.
		* @param aResourceId Resource id.
		* @param aSubString Substring that is added into read resource.
		* @return Text read from resource file.
        */
		static HBufC* ReadLC(TInt aResourceId, const TDesC& aSubString);

	   /**
        * Reads string from resource file.
		* @param aResourceId Resource id.
		* @param aNumber Number that is added into read resource.
		* @return Text read from resource file.
        */
		static HBufC* ReadLC(TInt aResourceId, TInt aNumber);
		
	   /**
        * Reads array of strings from resource file.
    	* @param aResourceId Resource id.
		* @return String array.
        */
		CDesCArrayFlat* ReadDesArrayLC(TInt aResourceId);

	   /**
        * Reads array of strings from resource file.
    	* @param aResourceId Resource id.
		* @return String array.
        */
		CDesCArrayFlat* ReadDesArrayL(TInt aResourceId);

	   /**
        * Reads array of strings from resource file.
    	* @param aResourceId Resource id.
		* @return String array.
        */
		static CDesCArrayFlat* ReadDesArrayStaticLC(TInt aResourceId);

	   /**
        * Reads progress dialog progress text from resource file.
		* @param aContent Content name.
		* @param aCurrent Currently processed sync item.
    	* @param aFinal Number of sync items.
		* @param aSend ETrue is sync phase is "sending".
		* @return Progress text.
        */
		//HBufC* ReadProgressTextLC(const TDesC& aContent, TInt aCurrent, TInt aFinal, TInt aPhase);
		
	   /**
        * Reads progress dialog progress text from resource file.
		* @param aContent.
		* @param aPhase.
		* @return Progress text.
        */
		HBufC* ReadProgressTextLC(const TDesC& aContent, TInt aPhase);
		
	   /**
        * Reads progress dialog progress text from resource file.
        * Buffers the last aResource to minimize file operations.
        * @param aResourceId Resource id.
		* @param aContent Content name.
		* @return Progress text.
        */
		HBufC* ReadProgressTextLC(TInt aResourceId, const TDesC& aContent);

	   /**
        * Reads default profile name from resource file.
		* @param aIndex Number that is added to proile name (eg. "New Profile(1)").
		* @param aApplicationId.
		* @return Profile name.
        */
		static HBufC* GetNewProfileNameLC(TInt aIndex, TInt aApplicationId);
		
	   /**
        * Reads sync content name.
		* @param aDataProviderId.
		* @param aDefaultText.
		* @return Content name.
        */
		static HBufC* GetContentNameLC(TInt aDataProviderId, const TDesC& aDefaultText);
		
	   /**
        * Reads sync content title.
		* @param aDataProviderId.
		* @param aDefaultText.
		* @return Content title.
        */
		static HBufC* GetContentTitleLC(TInt aDataProviderId, const TDesC& aDefaultText);

	   /**
        * Reads sync content setting text.
		* @param aDataProviderId.
		* @param aDefaultText.
		* @return Content title.
        */
		static HBufC* GetContentSettingLC(TInt aDataProviderId, const TDesC& aDefaultText);
		
	   /**
        * Reads remote database setting header rersource id.
		* @param aDataProviderId.
		* @return Resource id.
        */
		static TInt RemoteDatabaseHeaderId(TInt aDataProviderId);
		
	   /**
        * Reads profile info text.
		* @param aProfile.
		* @param aBearer.
		* @return Profile info text.
        */
		static HBufC* ReadProfileInfoTextLC(const TDesC& aProfile, const TDesC& aBearer);

	   /**
        * Reads profile info text.
		* @param aProfile.
		* @param aBearer.
		* @param aHours.
		* @param aMinutes.
		* @return Profile info text.
        */
		static HBufC* ReadProfileInfoTextLC(const TDesC& aProfile, const TDesC& aBearer,
		                                    TInt aHours, TInt aMinutes);

		  /**
        * Reads profile info text.
		* @param aProfile.
		* @param aTime.
			*/
		static HBufC* ReadAutoSyncInfoTextLC(const TDesC& aProfile, TTime aTIme);
		
    private:
		//  sync app resource file id
		TInt iResId;

		// pointer to CEikonEnv
		CEikonEnv* iEikEnv;

		// progress text format (from resource file)
		HBufC* iProgressTextFormat;
		
		// progress text format id
		TInt iProgressTextFormatId;
    };



#endif  // ASPRESHANDLER_H