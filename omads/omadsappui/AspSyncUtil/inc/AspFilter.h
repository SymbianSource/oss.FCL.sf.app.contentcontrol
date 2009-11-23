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
* Description:  Contains general data definitions for AspSyncUtil.
*
*/



#ifndef ASPFILTER_H
#define ASPFILTER_H


// INCLUDES
#include <e32base.h>
#include <ecom.h>

#include <SyncMLClient.h>    // RSyncMLSession
#include <SyncMLClientDS.h>  // RSyncMLDataSyncProfile

#include <syncmldatafilter.h>
//#include <syncmldatafilter.hrh>


#include <AknCheckBoxSettingPage.h>  // CSelectionItemList


#include "aspdefines.h"
#include "asputil.h"





// CONSTANTS

const TInt KFilterBaseId = 1000;
const TInt KFilterItemBaseId = 2000;


// FORWARD DECLARATIONS
class CAspFilterItem;


// CLASS DECLARATION



/**
* CAspFilterList
*
* CAspFilterList is used for sync filter operations.
*/
NONSHARABLE_CLASS (CAspFilterList) : public CBase
    {

    public:

        /**
        * Two-phased constructor.
        */
        static CAspFilterList* NewL(const TAspParam& aParam, RPointerArray<CSyncMLFilter>& aArray);

	    /**
        * Destructor.
        */
        ~CAspFilterList();

    private:
        /**
        * C++ default constructor.
        */
	    CAspFilterList(const TAspParam& aParam, RPointerArray<CSyncMLFilter>& aArray);

        /**
        * By default Symbian 2nd phase constructor is private.
        */
	    void ConstructL();
	    
    public:
        //void InitAllFiltersL();
        void InitFiltersL();
        TInt Count();
        void SetFiltersEnabledL(TBool aEnable);
        TBool FiltersEnabled();
        CAspFilter* Item(TInt aIndex);
        CAspFilter* ItemForId(TInt aId);
        void SaveL();
    
    private:
		// application id
		TInt iApplicationId;
		
		// sync task
		//CAspTask* iAspTask;
		
		// list of filters
		RPointerArray<CAspFilter> iFilterList;
		
		// is filter feature enabled
		TBool iFiltersEnabled;
		
		RPointerArray<CSyncMLFilter> iArray;
     };




/**
* CAspFilter
*
* CAspTask is used for sync filter operations.
*/
NONSHARABLE_CLASS (CAspFilter) : public CBase
    {

    public:
        /**
        * Two-phased constructor.
        */
        static CAspFilter* NewLC(CSyncMLFilter* aSyncFilter, TInt aId);


	    /**
        * Destructor.
        */
        ~CAspFilter();

    private:
        /**
        * C++ default constructor.
        */
	    CAspFilter(CSyncMLFilter* aSyncFilter, TInt aId);

        /**
        * By default Symbian 2nd phase constructor is private.
        */
	    void ConstructL();
	    
    public:
        void SaveL();
        const TDesC& DisplayName();
        TInt Id();
        void GetDisplayValueL(TDes& aText);
        const TDesC& DisplayValueL();
        TInt ItemCount();
        CAspFilterItem* Item(TInt aIndex);
        CAspFilterItem* ItemForId(TInt aId);
        TBool Enabled();
        void SetEnabledL(TBool aEnable);
        
    private:
        void InitFilterItemsL();
        
   
    private:
		// application id
		TInt iApplicationId;
		
		// temporary filter id
		TInt iId;
		
		// sync filter
		CSyncMLFilter* iSyncFilter;
		
		// list of filters
		RPointerArray<CAspFilterItem> iFilterItemList;
    };




/**
* CAspFilterItem
*
* CAspFilterItem is used for sync filter operations.
*/
NONSHARABLE_CLASS (CAspFilterItem) : public CBase
    {
    public:
	enum TListType
		{
		ETypeQuery,
	    ETypeSingleSelection,
	    ETypeSingleSelectionUserDefined,
	    ETypeMultipleSelection,
	    ETypeMultipleSelectionUserDefined
		};
    

    public:
        /**
        * Two-phased constructor.
        */
        static CAspFilterItem* NewLC(CSyncMLFilterProperty* aProperty, TInt aId);

        /**
        * Two-phased constructor.
        */
        static CAspFilterItem* NewL(CSyncMLFilterProperty* aProperty, TInt aId);

	    /**
        * Destructor.
        */
        ~CAspFilterItem();

    private:
        /**
        * C++ default constructor.
        */
	    CAspFilterItem(CSyncMLFilterProperty* aProperty, TInt aId);

        /**
        * By default Symbian 2nd phase constructor is private.
        */
	    void ConstructL();
	    
    public:
    
        TInt ListType();
        TInt DataType();
        const TDesC& DisplayName();
        const TDesC& DisplayValueL();
        void GetDisplayValueL(TDes& aText);
        void GetSelectionL(CSelectionItemList* aList);
        void SetSelectionL(CSelectionItemList* aList);
        void CheckEmptySelectionL();
        TInt Id();
        TBool IsMandatory();
        TInt MaxLength();
        
    public:
        void SetBoolL(TBool aBool);
        TBool BoolL();
        void SetTextL(const TDesC& aText);
        void GetTextL(TDes& aText);
        void GetText(TDes& aText);
        TTime DateL();
        void SetDateL(TTime aTime);
        TTime TimeL();
        void SetTimeL(TTime aTime);
        TInt IntL();
        void SetIntL(TInt aInt);


    private:
        void InitListTypeL();
        void InitDataTypeL();

        TInt SelectionAndQueryCount();
        TInt SelectionIndex();
        TBool QueryValueSelected();
        
        void GetQueryValueTextL(TDes& aText);
        void SetQueryValueTextL(const TDesC& aText);
        void GetListValueTextL(TDes& aText);
        void GetListValueTextL(TDes& aText, TInt aIndex);

        TBool IsMultipleSelection();
        void UpdateQueryValueL(CSelectionItemList* aList);
        void AddQueryValueL(CSelectionItemList* aList);
        TInt SelectionCount();

        
        
    private:
		// application id
		TInt iApplicationId;
		
		// filter item id
		TInt iId;
		
		// editor type
		TInt iListType;
		
		// data type
		TInt iDataType;
		
		// temp buffer
		TBuf<KBufSize> iBuf;

		// temp buffer
		TBuf8<KBufSize> iBuf8;
		
		// temp buffer
		TBuf<KBufSize> iDisplayValueBuf;
		
		// sync filter propperty
		CSyncMLFilterProperty* iProperty;
     };







#endif  // ASPFILTER_H


// End of file