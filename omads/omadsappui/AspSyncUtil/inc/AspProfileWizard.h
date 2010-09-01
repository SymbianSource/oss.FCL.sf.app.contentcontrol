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
* Description: Used to save setting entered by user in profile creation wizard 
*
*/


#ifndef ASPPROFILEWIZARD_H
#define ASPPROFILEWIZARD_H


//  INCLUDES
#include "AspDialogUtil.h"


// CONSTANTS




/**
* CAspWizardItem
* 
* CAspWizardItem contains one setting item data.
*/
NONSHARABLE_CLASS (CAspWizardItem) : public CBase
	{
    public:
	enum TSettingType
		{
	    ETypeStart,
	    ETypeProfileName,
	    ETypeProtocolVersion,
	    ETypeHostAddress,
	    ETypeServerId,
	    ETypeBearerType,
	    ETypeUserName,
		ETypePassword,
		ETypeSyncType,
		ETypeContentSelection,
		ETypeSyncContent,
		ETypeSyncContentLocalDatabase,
		ETypeEnd
		};

    public:
        /**
        * Two-phased constructor.
        */
        static CAspWizardItem* NewL();
        
        /**
        * Destructor.
        */
        ~CAspWizardItem();
        
    private:
        /**
        * C++ default constructor.
        */
        CAspWizardItem();
        
        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL();
	
	
	public:
        /**
        * Set string type value.
        * @param aTex.
        * @return None.
        */
        void SetValueL(const TDesC& aText);

        /**
        * Get string type value.
        * @param None. 
        * @return String type value.
        */
		const TDesC& Value();

        /**
        * Set content name.
        * @param aTex.
        * @return None.
        */
        void SetContentNameL(const TDesC& aText);

        /**
        * Get content name.
        * @param None. 
        * @return Content name.
        */
		const TDesC& ContentName();

        /**
        * Set header.
        * @param aTex.
        * @return None.
        */
        void SetHeaderL(const TDesC& aText);

        /**
        * Set header.
        * @param aResourceId.
        * @return None.
        */
        void SetHeaderL(TInt aResourceId);

        /**
        * Get header.
        * @param None. 
        * @return Content name.
        */
		const TDesC& Header();

        /**
        * Initalizes setting item.
        * @param None. 
        * @return None.
        */
		void InitL();
	
	public:
    	// setting type
		TInt  iSettingType;
		
		// identifies settings that have same setting type
		TInt  iSettingId;

    	// setting editor title resource id
		TInt iResourceId;
		
    	// popup note resource id
		TInt iPopupNoteResourceId;

    	// error note resource id
		TInt iErrorNoteResourceId;

		// is this setting included in sync wizard
		TBool iEnabled;

		// max text length
		TInt iMaxLength;

		// is setting mandatory
		TBool iMandatory;
		
		// are only western characters allowed
		TInt iLatinInput;
		
		// integer type value
		TInt iNumber;
		
    private:
		// string type setting value
	    HBufC* iValue;
	
   		// content display name
	    HBufC* iContentName;

   		// editor header
	    HBufC* iHeader;
    };




/**
* CAspProfileWizard
*  
* CAspProfileWizard creates new profile.
*/
NONSHARABLE_CLASS (CAspProfileWizard) : public CBase
    {

	public:// Constructors and destructor

        /**
        * Two-phased constructor.
        */
        static CAspProfileWizard* NewL(TAspParam& aParam);
        
        /**
        * Destructor.
        */
        virtual ~CAspProfileWizard();
    
    private:

        /**
        * C++ default constructor.
        */
        CAspProfileWizard(TAspParam& aParam);
	
        /**
        * By default Symbian OS constructor is private.
        */
        void ConstructL();

	private:
	    /**
        * Creates setting item list.
		* @param None.
        * @return None.
        */
	    void CreateSettingListL();

    public:
        void GetContentSelectionL(CSelectionItemList* aList);
        void SetContentSelectionL(CSelectionItemList* aList);
        void SetContentEnabled(const TDesC& aText, TBool aEnabled);
        void SetContentEnabled(TInt aApplicationId);
        void SetEnabled(TInt aSettingType, TBool aEnabled);
        void SetContentLocalDatabaseEnabled(TInt aDataproviderId, TBool aEnabled);
        void SetContentEnabled(TInt aDataproviderId, TBool aEnabled);
        TBool ContentEnabled(TInt aDataproviderId);
        
        void SetContentEnabled(RArray<TInt>& aDataProviderArray);
        void GetContentEnabled(RArray<TInt>& aDataProviderArray);
        void GetContentName(TDes& aText);
        TInt ContentId(const TDesC& aText);
    
    public:
        CAspWizardItem* CurrentItem();
        void CreateProfileL();
        void DeleteProfile();
        void MoveToNext();
        void MoveToPrevious();
        void MoveToStart();
        TInt CurrentItemType();
        TInt NumOfCurrentItem(CAspWizardItem* aItem);
        TInt NumOfEnabledItems();
        TInt NumOfEnabledContentItems();
        TInt NumOfContentItems();
        void SetStringValueL(const TDesC& aValue, TInt aSettingType, TInt aSettingId);
        void SetStringValueL(const TDesC& aValue, TInt aSettingType);
        void SetNumberValue(TInt& aValue, TInt aSettingType);
        TInt CreatedProfileId(void);
        TInt NumberValue(TInt aSettingType);
        CDesCArray* LocalDatabaseList(const TInt aDataproviderId);

        
    private:
        void GetStringValue(TDes& aValue, TInt aSettingType);
        void GetStringValue(TDes& aValue, TInt aSettingType, TInt aSettingId);
        void GetNumberValue(TInt& aValue, TInt aSettingType);
        TInt IncreaseCurrentIndex(const TInt aIndex);
        TInt DecreaseCurrentIndex(const TInt aIndex);
        TBool CheckHostAddress(const TDesC& aText);
        TInt ContentPopupNoteResourceId(TInt aDataProviderId);
        //TInt ContentHeaderResourceId(TInt aDataProviderId);
       
    private:
        //TInt ShowTextEditorL(TInt aResourceId, TDes& aText, const TDesC& aTitle, TBool aMandatory, TBool aLatinInput, TInt aMaxLength);
        
	private:
   		// dialog parameter for returning value to caller
		TAspParam* iDialogParam;

		// id of the calling application
		TInt iApplicationId;
		
		// id of the created profile
		TInt iProfileId;
	
    	// session with sync server
		RSyncMLSession* iSyncSession;
	
   		// list that contains all contents (applications)
		CAspContentList* iContentList;

        // for string handling
		TBuf<KBufSize255> iBuf;
		
		// list index of current profile setting
		TInt iCurrentItemIndex;
		
    private:
	
		// names of all available local data stores
		CDesCArray* iLocalDataStores;
		
		// list of sync contents
		RPointerArray<CAspWizardItem> iSettingList;
	};






#endif      // ASPPROFILEWIZARD_h
            
// End of File
