/*
* Copyright (c) 2002-2005 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:  
*
*/



// INCLUDES

#include <SyncMLTransportProperties.h>


#include "aspfilter.h"
#include "aspprofile.h"
#include "AspResHandler.h"
#include "AspDialogUtil.h"



// ============================ MEMBER FUNCTIONS ===============================



/*******************************************************************************
 * class CAspFilterList
 *******************************************************************************/



// -----------------------------------------------------------------------------
// CAspFilterList::NewL
//
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CAspFilterList* CAspFilterList::NewL(const TAspParam& aParam, RPointerArray<CSyncMLFilter>& aArray)
    {
    CAspFilterList* self = new(ELeave) CAspFilterList(aParam, aArray);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);

	return self;
    }


// -----------------------------------------------------------------------------
// CAspFilterList::~CAspFilterList
//
// -----------------------------------------------------------------------------
//
CAspFilterList::~CAspFilterList()
    {
    iFilterList.ResetAndDestroy();
    iFilterList.Close();
    }


// -----------------------------------------------------------------------------
// CAspFilterList::ConstructL
//
// -----------------------------------------------------------------------------
//
void CAspFilterList::ConstructL()
    {
    InitFiltersL();
    }


// -----------------------------------------------------------------------------
// CAspFilterList::CAspFilterList
//
// -----------------------------------------------------------------------------
//
CAspFilterList::CAspFilterList(const TAspParam& aParam, RPointerArray<CSyncMLFilter>& aArray)
	{
	iApplicationId = aParam.iApplicationId;
	//iAspTask = aParam.iSyncTask;
	iArray = aArray;
	}


// -----------------------------------------------------------------------------
// CAspFilterList::InitFiltersL
//
// -----------------------------------------------------------------------------
//
void CAspFilterList::InitFiltersL()
	{
	//TSyncMLFilterChangeInfo changeInfo = ESyncMLDefault;
	//RPointerArray<CSyncMLFilter>& arr = iAspTask->Task().SupportedServerFiltersL(changeInfo);
	
	TInt count = iArray.Count();
	for (TInt i=0; i<count; i++)
		{
		CSyncMLFilter* filter = iArray[i];
		CAspFilter* aspFilter = CAspFilter::NewLC(filter, i);
		iFilterList.AppendL(aspFilter);
		CleanupStack::Pop(aspFilter);
		}
	}
	
	
// -----------------------------------------------------------------------------
// CAspFilterList::Count
//
// -----------------------------------------------------------------------------
//
TInt CAspFilterList::Count()
	{
	return iFilterList.Count();
	}


// -----------------------------------------------------------------------------
// CAspFilterList::FiltersEnabled
//
// -----------------------------------------------------------------------------
//
TBool CAspFilterList::FiltersEnabled()
	{
    TInt count = iFilterList.Count();
    
    for (TInt i=0; i<count; i++)
    	{
    	CAspFilter* filter = iFilterList[i];
    	if (filter->Enabled())
    		{
    		return ETrue;
    		}
    	}
    	
    return EFalse;
	}


// -----------------------------------------------------------------------------
// CAspFilterList::SetFiltersEnabledL
//
// -----------------------------------------------------------------------------
//
void CAspFilterList::SetFiltersEnabledL(TBool aEnable)
	{
    TInt count = iFilterList.Count();
    
    for (TInt i=0; i<count; i++)
    	{
    	CAspFilter* filter = iFilterList[i];
    	filter->SetEnabledL(aEnable);
    	}
	}


// -----------------------------------------------------------------------------
// CAspFilterList::SaveL
//
// -----------------------------------------------------------------------------
//
void CAspFilterList::SaveL()
	{
	}


// -----------------------------------------------------------------------------
// CAspFilterList::Item
// 
// -----------------------------------------------------------------------------
//
CAspFilter* CAspFilterList::Item(TInt aIndex)
	{
	if (aIndex < 0 || aIndex >= iFilterList.Count())
	    {
		TUtil::Panic(KErrArgument);
	    }

	return iFilterList[aIndex];
	}
	

// -----------------------------------------------------------------------------
// CAspFilterList::ItemForIdL
// 
// -----------------------------------------------------------------------------
//
CAspFilter* CAspFilterList::ItemForId(TInt aId)
	{
    TInt count = iFilterList.Count();
    
    for (TInt i=0; i<count; i++)
    	{
    	CAspFilter* filter = iFilterList[i];
    	if (filter->Id() == aId)
    		{
    		return filter;
    		}
    	}
	
	return NULL;
	}





/*******************************************************************************
 * class CAspFilter
 *******************************************************************************/


// -----------------------------------------------------------------------------
// CAspFilter::NewLC
//
// -----------------------------------------------------------------------------
//
CAspFilter* CAspFilter::NewLC(CSyncMLFilter* aSyncFilter, TInt aId)
    {
    CAspFilter* self = new(ELeave) CAspFilter(aSyncFilter, aId);
	CleanupStack::PushL(self);
	self->ConstructL();

	return self;
    }


// -----------------------------------------------------------------------------
// CAspFilter::~CAspFilter
//
// -----------------------------------------------------------------------------
//
CAspFilter::~CAspFilter()
    {
    iFilterItemList.ResetAndDestroy();
    iFilterItemList.Close();
    }


// -----------------------------------------------------------------------------
// CAspFilter::ConstructL
//
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CAspFilter::ConstructL()
    {
    InitFilterItemsL();
    }


// -----------------------------------------------------------------------------
// CAspFilter::CAspFilter
//
// -----------------------------------------------------------------------------
//
CAspFilter::CAspFilter(CSyncMLFilter* aSyncFilter, TInt aId)
	{
	iId = KFilterBaseId + aId;
	iSyncFilter = aSyncFilter;
	
    __ASSERT_ALWAYS(iSyncFilter, TUtil::Panic(KErrGeneral));		
	}
	

// -----------------------------------------------------------------------------
// CAspFilter::SaveL
//
// -----------------------------------------------------------------------------
//
void CAspFilter::SaveL()
    {
    }


// -----------------------------------------------------------------------------
// CAspFilter::InitFilterItemsL
//
// -----------------------------------------------------------------------------
//
void CAspFilter::InitFilterItemsL()
	{
	RPointerArray<CSyncMLFilterProperty>& arr = iSyncFilter->FilterProperties();
	
	TInt count = arr.Count();
	for (TInt i=0; i<count; i++)
		{
		CSyncMLFilterProperty* property = arr[i];
		CAspFilterItem* item = CAspFilterItem::NewLC(property, i);
		iFilterItemList.AppendL(item);
		CleanupStack::Pop(item);
		}
	}


// -----------------------------------------------------------------------------
// CAspFilter::DisplayName
// 
// -----------------------------------------------------------------------------
//
const TDesC& CAspFilter::DisplayName()
	{
	return iSyncFilter->DisplayName();
	}


// -----------------------------------------------------------------------------
// CAspFilter::GetDisplayValueL
// 
// -----------------------------------------------------------------------------
//
void CAspFilter::GetDisplayValueL(TDes& aText)
	{
	aText = KNullDesC;
	
	if (!Enabled())
		{
		CAspResHandler::ReadL(aText, R_ASP_NOT_DEFINED);
		}
	else if (iFilterItemList.Count() == 1)
		{
	    CAspFilterItem* item = iFilterItemList[0];
	    item->GetDisplayValueL(aText);
		}
	else if (iFilterItemList.Count() > 1)
		{
		CAspResHandler::ReadL(aText, R_ASP_MULTIPLE_SELECTION);
		}
	}


// -----------------------------------------------------------------------------
// CAspFilter::Id
// 
// -----------------------------------------------------------------------------
//
TInt CAspFilter::Id()
	{
	return iId;
	}


// -----------------------------------------------------------------------------
// CAspFilter::ItemCount
// 
// -----------------------------------------------------------------------------
//
TInt CAspFilter::ItemCount()
	{
	return iFilterItemList.Count();
	}


// -----------------------------------------------------------------------------
// CAspFilter::Item
// 
// -----------------------------------------------------------------------------
//
CAspFilterItem* CAspFilter::Item(TInt aIndex)
	{
	if (aIndex < 0 || aIndex >= ItemCount())
	    {
		TUtil::Panic(KErrArgument);
	    }

	return iFilterItemList[aIndex];
	}


// -----------------------------------------------------------------------------
// CAspFilter::ItemForIdL
// 
// -----------------------------------------------------------------------------
//
CAspFilterItem* CAspFilter::ItemForId(TInt aId)
	{
    TInt count = ItemCount();
    
    for (TInt i=0; i<count; i++)
    	{
    	CAspFilterItem* item = Item(i);
    	if (item->Id() == aId)
    		{
    		return item;
    		}
    	}
	
	return NULL;
	}


// -----------------------------------------------------------------------------
// CAspFilter::Enabled
// 
// -----------------------------------------------------------------------------
//
TBool CAspFilter::Enabled()
	{
	return iSyncFilter->Enabled();
	}


// -----------------------------------------------------------------------------
// CAspFilter::SetEnabledL
// 
// -----------------------------------------------------------------------------
//
void CAspFilter::SetEnabledL(TBool aEnable)
	{
	iSyncFilter->SetEnabledL(aEnable);
	}



	

	
	
/*******************************************************************************
 * class CAspFilterItem
 *******************************************************************************/


// -----------------------------------------------------------------------------
// CAspFilterItem::NewLC
//
// -----------------------------------------------------------------------------
//
CAspFilterItem* CAspFilterItem::NewLC(CSyncMLFilterProperty* aProperty, TInt aId)
    {
    CAspFilterItem* self = new(ELeave) CAspFilterItem(aProperty, aId);
	CleanupStack::PushL(self);
	self->ConstructL();

	return self;
    }


// -----------------------------------------------------------------------------
// CAspFilterItem::NewL
//
// -----------------------------------------------------------------------------
//
CAspFilterItem* CAspFilterItem::NewL(CSyncMLFilterProperty* aProperty, TInt aId)
    {
    CAspFilterItem* self = new(ELeave) CAspFilterItem(aProperty, aId);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);

	return self;
    }


// -----------------------------------------------------------------------------
// CAspFilterItem::~CAspFilterItem
//
// -----------------------------------------------------------------------------
//
CAspFilterItem::~CAspFilterItem()
    {
    }


// -----------------------------------------------------------------------------
// CAspFilterItem::ConstructL
//
// -----------------------------------------------------------------------------
//
void CAspFilterItem::ConstructL()
    {
    InitListTypeL();
    InitDataTypeL();
    }


// -----------------------------------------------------------------------------
// CAspFilterItem::CAspFilterItem
// -----------------------------------------------------------------------------
//
CAspFilterItem::CAspFilterItem(CSyncMLFilterProperty* aProperty, TInt aId)
	{
	iId = aId + KFilterItemBaseId;
	iProperty = aProperty;
	
	__ASSERT_ALWAYS(iProperty, TUtil::Panic(KErrGeneral));
	}
	

// -----------------------------------------------------------------------------
// CAspFilterItem::ListType
//
// -----------------------------------------------------------------------------
//
TInt CAspFilterItem::ListType()
	{
	return iListType;
	}


// -----------------------------------------------------------------------------
// CAspFilterItem::DataType
//
// -----------------------------------------------------------------------------
//
TInt CAspFilterItem::DataType()
	{
	return iDataType;
	}


// -----------------------------------------------------------------------------
// CAspFilterItem::InitListType
//
// -----------------------------------------------------------------------------
//
void CAspFilterItem::InitListTypeL()
	{
	TInt defaultValueCount = iProperty->DefaultValueCount();
	TBool querySupported = iProperty->CanUseQueryValue();
	TBool multiSelectionSupported = iProperty->SupportsMultipleSelection(); 

    if (defaultValueCount == 0 && querySupported)
    	{
    	iListType = ETypeQuery;
    	}
    else if (defaultValueCount > 0 && !querySupported && !multiSelectionSupported)
    	{
    	iListType = ETypeSingleSelection;
    	}
    else if (defaultValueCount > 0 && querySupported && !multiSelectionSupported)
    	{
    	iListType = ETypeSingleSelectionUserDefined;
    	}
    else if (defaultValueCount > 0 && !querySupported && multiSelectionSupported)
    	{
    	iListType = ETypeMultipleSelection;
    	}
    else if (defaultValueCount > 0 && querySupported && multiSelectionSupported)
    	{
    	iListType = ETypeMultipleSelectionUserDefined;
    	}
    else
    	{
    	User::Leave(KErrNotSupported);
    	}
	}


// -----------------------------------------------------------------------------
// CAspFilterItem::InitDataType
//
// -----------------------------------------------------------------------------
//
void CAspFilterItem::InitDataTypeL()
	{
	if (!iProperty->CanUseQueryValue())
		{
		iDataType = CAspListItemData::ETypeList;
		return;
		}
		
	TSyncMLFilterPropertyDataType type = iProperty->QueryValueDataType();
		
    switch (type)
    	{
    	case ESyncMLDataTypeBool:
    	    iDataType = CAspListItemData::ETypeBoolean;
    	    break;
    	case ESyncMLDataTypeNumber:
    	    iDataType = CAspListItemData::ETypeNumber;
    	    break;
    	case ESyncMLDataTypeDate:
    	    iDataType = CAspListItemData::ETypeDate;
    	    break;
    	case ESyncMLDataTypeTime:
    	    iDataType = CAspListItemData::ETypeTime;
    	    break;
    	case ESyncMLDataTypeText8:
    	    iDataType = CAspListItemData::ETypeText;
    	    break;
    	case ESyncMLDataTypeText16:
    	    iDataType = CAspListItemData::ETypeText;
    	    break;

    	default:
    	    User::Leave(KErrNotSupported);
    	    break;
    	}
	}


// -----------------------------------------------------------------------------
// CAspFilterItem::DisplayName
// 
// -----------------------------------------------------------------------------
//
const TDesC& CAspFilterItem::DisplayName()
	{
	return iProperty->DisplayName();
	}


// -----------------------------------------------------------------------------
// CAspFilterItem::DisplayValueL
// 
// -----------------------------------------------------------------------------
//
const TDesC& CAspFilterItem::DisplayValueL()
	{
	GetDisplayValueL(iDisplayValueBuf);
	return iDisplayValueBuf;
	}


// -----------------------------------------------------------------------------
// CAspFilterItem::GetDisplayValueL
// 
// -----------------------------------------------------------------------------
//
void CAspFilterItem::GetDisplayValueL(TDes& aText)
	{
	aText = KNullDesC;
	
	if (ListType() == ETypeQuery)
		{
		GetQueryValueTextL(aText);
		}
	else if (SelectionAndQueryCount() > 1)
		{
		CAspResHandler::ReadL(aText, R_ASP_MULTIPLE_SELECTION);
		}
	else if (SelectionAndQueryCount() == 1)
		{
		if (QueryValueSelected())
			{
			GetQueryValueTextL(aText);
			}
		else
			{
			GetListValueTextL(aText);
			}
		}
	}


// -----------------------------------------------------------------------------
// CAspFilterItem::GetQueryValueTextL
// 
// -----------------------------------------------------------------------------
//
void CAspFilterItem::GetQueryValueTextL(TDes& aText)
	{
	aText = KNullDesC;
	
	TSyncMLFilterPropertyDataType type = iProperty->QueryValueDataType();
		
	switch (type)
		{
		case ESyncMLDataTypeText16:
	    	GetText(aText);
		    break;

		case ESyncMLDataTypeText8:
		    GetText(aText);
		    break;
		    
		case ESyncMLDataTypeNumber:
		    aText.Num(iProperty->QueryValueIntL());
    	    break;

		case ESyncMLDataTypeBool:
			{
		    CDesCArray* stringData = CAspResHandler::ReadDesArrayStaticLC(R_ASP_FILTER_ITEM_YESNO);
		    if (iProperty->QueryValueBoolL())
		    	{
		    	aText = (*stringData)[1];
		    	}
		    else
		    	{
		    	aText = (*stringData)[0];
		    	}
		    CleanupStack::PopAndDestroy(stringData);
		    break;
			}
		
		case ESyncMLDataTypeDate:
		    TUtil::GetDateTextL(aText, iProperty->QueryValueDateL());
		    break;
		    
		case ESyncMLDataTypeTime:
		    TUtil::GetTimeTextL(aText, iProperty->QueryValueTimeL());
		    break;

		default:
		    break;
	
		}
		

	if (TUtil::IsEmpty(aText))
		{
		CAspResHandler::ReadL(aText, R_ASP_SETTING_VALUE_NONE);
		}
	}


// -----------------------------------------------------------------------------
// CAspFilterItem::SetQueryValueTextL
// 
// -----------------------------------------------------------------------------
//
void CAspFilterItem::SetQueryValueTextL(const TDesC& aText)
	{
	TInt num = 0;
	
	TSyncMLFilterPropertyDataType type = iProperty->QueryValueDataType();
		
	switch (type)
		{
		case ESyncMLDataTypeText16:
	    	iProperty->SetQueryValueText16L(aText);
		    break;

		case ESyncMLDataTypeText8:
		    TUtil::StrCopy(iBuf8, aText);
	    	iProperty->SetQueryValueText8L(iBuf8);
		    break;
		    
		case ESyncMLDataTypeNumber:
    	    User::LeaveIfError(TUtil::StrToInt(aText, num));
		    iProperty->SetQueryValueIntL(num);
    	    break;

		case ESyncMLDataTypeBool:
		    User::LeaveIfError(TUtil::StrToInt(aText, num));
		    if (num)
		    	{
		    	iProperty->SetQueryValueBoolL(ETrue);
		    	}
		    else
		    	{
		    	iProperty->SetQueryValueBoolL(EFalse);
		    	}
    	    break;

		case ESyncMLDataTypeDate:
    	    User::LeaveIfError(TUtil::StrToInt(aText, num));
		    iProperty->SetQueryValueDateL(num);
    	    break;

		case ESyncMLDataTypeTime:
    	    User::LeaveIfError(TUtil::StrToInt(aText, num));
		    iProperty->SetQueryValueTimeL(num);
    	    break;

		default:
		    break;
	
		}
	}


// -----------------------------------------------------------------------------
// CAspFilterItem::GetListValueTextL
// 
// -----------------------------------------------------------------------------
//
void CAspFilterItem::GetListValueTextL(TDes& aText)
	{
	aText = KNullDesC;
	TInt index = SelectionIndex();
	if (index == KErrNotFound)
		{
		CAspResHandler::ReadL(aText, R_ASP_SETTING_VALUE_NONE);
		return; // list does not have one item selected
		}
		
	GetListValueTextL(aText, index);
	}


// -----------------------------------------------------------------------------
// CAspFilterItem::GetListValueTextL
// 
// -----------------------------------------------------------------------------
//
void CAspFilterItem::GetListValueTextL(TDes& aText, TInt aIndex)
	{
	aText = KNullDesC;
	
	if (aIndex < 0 || aIndex >= iProperty->DefaultValueCount())
		{
		TUtil::Panic(KErrArgument);
		}
		
	TSyncMLFilterPropertyDataType type = iProperty->DefaultValueDataType();
		
	switch (type)
		{
		case ESyncMLDataTypeText16:
	    	TUtil::StrCopy(aText, iProperty->DefaultValueText16L(aIndex));
		    break;

		case ESyncMLDataTypeText8:
	    	TUtil::StrCopy(aText, iProperty->DefaultValueText8L(aIndex));
		    break;
		    
		case ESyncMLDataTypeNumber:
		    aText.Num(iProperty->DefaultValueIntL(aIndex));
    	    break;

		case ESyncMLDataTypeBool:
			{
			CDesCArray* stringData = CAspResHandler::ReadDesArrayStaticLC(R_ASP_FILTER_ITEM_YESNO);
		    if (iProperty->DefaultValueBoolL(aIndex))
		    	{
		    	aText = (*stringData)[1];
		    	}
		    else
		    	{
		    	aText = (*stringData)[0];
		    	}
		    CleanupStack::PopAndDestroy(stringData);
		    break;
			}
		
		case ESyncMLDataTypeDate:
		    TUtil::GetDateTextL(aText, iProperty->DefaultValueDateL(aIndex));
		    break;
		    
		case ESyncMLDataTypeTime:
		    TUtil::GetTimeTextL(aText, iProperty->DefaultValueTimeL(aIndex));
		    break;

		default:
		    break;
	
		}
	}


// -----------------------------------------------------------------------------
// CAspFilterItem::SelectionCount
// 
// -----------------------------------------------------------------------------
//
TInt CAspFilterItem::SelectionCount()
	{
	TInt count = 0;
	TInt defaultValueCount = iProperty->DefaultValueCount();
	
	for (TInt i=0; i<defaultValueCount; i++)
		{
		if (iProperty->IsDefaultValueSelected(i))
			{
			count++;
			}
		}

   	return count;
	}


// -----------------------------------------------------------------------------
// CAspFilterItem::SelectionAndQueryCount
// 
// -----------------------------------------------------------------------------
//
TInt CAspFilterItem::SelectionAndQueryCount()
	{
	TInt count = SelectionCount();

	if (QueryValueSelected())
		{
		count++;
		}
   		
   	return count;
	}


// -----------------------------------------------------------------------------
// CAspFilterItem::SelectionIndex
// 
// -----------------------------------------------------------------------------
//
TInt CAspFilterItem::SelectionIndex()
	{
	if (SelectionCount() != 1)
		{
		return KErrNotFound;
		}
	if (QueryValueSelected())
		{
		return KErrNotFound;
		}
		
	TInt count = iProperty->DefaultValueCount();
	
	for (TInt i=0; i<count; i++)
		{
		if (iProperty->IsDefaultValueSelected(i))
			{
			return i;
			}
		}
		
	return KErrNotFound;
	}


// -----------------------------------------------------------------------------
// CAspFilterItem::QueryValueSelected
// 
// -----------------------------------------------------------------------------
//
TBool CAspFilterItem::QueryValueSelected()
	{
	if (iProperty->DefaultValueCount() > 0 && iProperty->CanUseQueryValue())
		{
    	if (iProperty->IsQueryValueSelected())
			{
			return ETrue;
        	}
		}
	
	return EFalse;
	}


// -----------------------------------------------------------------------------
// CAspFilterItem::Id
// 
// -----------------------------------------------------------------------------
//
TInt CAspFilterItem::Id()
	{
	return iId;
	}


// -----------------------------------------------------------------------------
// CAspFilterItem::IsMandatory
//
// -----------------------------------------------------------------------------
//
TBool CAspFilterItem::IsMandatory()
	{
	if (iListType == ETypeMultipleSelection || iListType == ETypeMultipleSelectionUserDefined)
		{
		if (!iProperty->SupportsEmptySelection())
			{
			return ETrue;
			}
		}
		
	return EFalse;
	}


// -----------------------------------------------------------------------------
// CAspFilterItem::MaxLength
//
// -----------------------------------------------------------------------------
//
TInt CAspFilterItem::MaxLength()
	{
	TSyncMLFilterPropertyDataType type = iProperty->QueryValueDataType();
    if (type != ESyncMLDataTypeText8 && type != ESyncMLDataTypeText16)
    	{
    	return 0;
    	}
    
   return iProperty->MaxTextLength();
	}


// -----------------------------------------------------------------------------
// CAspFilterItem::GetSelectionL
//
// -----------------------------------------------------------------------------
//
void CAspFilterItem::GetSelectionL(CSelectionItemList* aList)
	{
	__ASSERT_ALWAYS(aList, TUtil::Panic(KErrGeneral));
	
	TBuf<KBufSize> buf;
	
	TInt count = iProperty->DefaultValueCount();
	for (TInt i=0; i<count; i++)
		{
		TBool selected = iProperty->IsDefaultValueSelected(i);
		GetListValueTextL(buf, i);
     	
		CSelectableItem* item = new (ELeave) CSelectableItem(buf, selected);
   	    CleanupStack::PushL(item);
	    item->ConstructL();
	    aList->AppendL(item);
	    CleanupStack::Pop(item);
    	}
    	
    AddQueryValueL(aList);
	}


// -----------------------------------------------------------------------------
// CAspFilterItem::SetSelectionL
//
// -----------------------------------------------------------------------------
//
void CAspFilterItem::SetSelectionL(CSelectionItemList* aList)
	{
	TInt count = aList->Count();
	TInt defaultValuecount = iProperty->DefaultValueCount();
	
	__ASSERT_ALWAYS(count >= defaultValuecount, TUtil::Panic(KErrGeneral));
	
	for (TInt i=0; i<defaultValuecount; i++)
		{
		TBool selected = (*aList)[i]->SelectionStatus();
		if (selected)
			{
			iProperty->SelectDefaultValueL(i);
			}
		else
			{
			iProperty->UnselectDefaultValueL(i);
			}
     	}
    	
    UpdateQueryValueL(aList);
    
    CheckEmptySelectionL();
	}


// -----------------------------------------------------------------------------
// CAspFilterItem::CheckEmptySelectionL
//
// -----------------------------------------------------------------------------
//
void CAspFilterItem::CheckEmptySelectionL()
	{	
	if (iListType != ETypeMultipleSelection && iListType != ETypeMultipleSelectionUserDefined)
		{
		return;
		}
	
	if (SelectionAndQueryCount() == 0 && !iProperty->SupportsEmptySelection())
		{
		if (iProperty->DefaultValueCount() > 0)
			{
			iProperty->SelectDefaultValueL(0);
			}
		}
	}


// -----------------------------------------------------------------------------
// CAspFilterItem::UpdateQueryValueL
//
// -----------------------------------------------------------------------------
//
void CAspFilterItem::UpdateQueryValueL(CSelectionItemList* aList)
	{	
	if (iListType != ETypeSingleSelectionUserDefined && iListType != ETypeMultipleSelectionUserDefined)
		{
		return;
		}
	
	TSyncMLFilterPropertyDataType type = iProperty->QueryValueDataType();
	if (type != ESyncMLDataTypeNumber && type != ESyncMLDataTypeText16 && type != ESyncMLDataTypeText8)
		{
		return; // "User defined" list only supports number and text
		}
	
	
	TInt count = aList->Count();
	TInt defaultValueCount = iProperty->DefaultValueCount();
	
	__ASSERT_ALWAYS(count == defaultValueCount+1, TUtil::Panic(KErrGeneral));
	
		
	CSelectableItem* item = (*aList)[count-1];
	
	SetQueryValueTextL(item->ItemText());
		
	if (!item->SelectionStatus())
		{
		iProperty->UnselectQueryValueL();
		}
	}


// -----------------------------------------------------------------------------
// CAspFilterItem::AddQueryValueL
//
// -----------------------------------------------------------------------------
//
void CAspFilterItem::AddQueryValueL(CSelectionItemList* aList)
	{	
	if (iListType != ETypeSingleSelectionUserDefined && iListType != ETypeMultipleSelectionUserDefined)
		{
		return;
		}
		
	TSyncMLFilterPropertyDataType type = iProperty->QueryValueDataType();
	if (type != ESyncMLDataTypeNumber && type != ESyncMLDataTypeText16 && type != ESyncMLDataTypeText8)
		{
		return; // "User defined" list only supports number and text
		}


	
	TBuf<KBufSize> buf;
	TBool selected = iProperty->IsQueryValueSelected();
	GetQueryValueTextL(buf);

	CSelectableItem* item = new (ELeave) CSelectableItem(buf, selected);
   	CleanupStack::PushL(item);
	item->ConstructL();
	aList->AppendL(item);
	CleanupStack::Pop(item);
	}


// -----------------------------------------------------------------------------
// CAspFilterItem::GetTextL
//
// -----------------------------------------------------------------------------
//
void CAspFilterItem::GetTextL(TDes& aText)
	{
	TSyncMLFilterPropertyDataType type = iProperty->QueryValueDataType();
	
	__ASSERT_DEBUG(type==ESyncMLDataTypeText8 || type==ESyncMLDataTypeText16, TUtil::Panic(KErrGeneral));
	
	if (type == ESyncMLDataTypeText8)
		{
		TUtil::StrCopy(aText, iProperty->QueryValueText8L());
		}
	else
		{
	    TUtil::StrCopy(aText, iProperty->QueryValueText16L());
		}
	}


// -----------------------------------------------------------------------------
// CAspFilterItem::GetText
//
// -----------------------------------------------------------------------------
//
void CAspFilterItem::GetText(TDes& aText)
	{
	TRAPD(err, GetTextL(aText));
	
	if (err != KErrNone)
		{
		aText = KNullDesC;
		}
	}


// -----------------------------------------------------------------------------
// CAspFilterItem::SetTextL
//
// -----------------------------------------------------------------------------
//
void CAspFilterItem::SetTextL(const TDesC& aText)
	{
	TSyncMLFilterPropertyDataType type = iProperty->QueryValueDataType();
	
	__ASSERT_DEBUG(type==ESyncMLDataTypeText8 || type==ESyncMLDataTypeText16, TUtil::Panic(KErrGeneral));
	
	if (type == ESyncMLDataTypeText8)
		{
	    TUtil::StrCopy(iBuf8, aText);
    	iProperty->SetQueryValueText8L(iBuf8);
		}
	else
		{
		iProperty->SetQueryValueText16L(aText);
		}
	}


// -----------------------------------------------------------------------------
// CAspFilterItem::BoolL
//
// -----------------------------------------------------------------------------
//
TBool CAspFilterItem::BoolL()
	{
	TSyncMLFilterPropertyDataType type = iProperty->QueryValueDataType();
	
	__ASSERT_DEBUG(type==ESyncMLDataTypeBool, TUtil::Panic(KErrGeneral));
	
	return iProperty->QueryValueBoolL();
	}


// -----------------------------------------------------------------------------
// CAspFilterItem::SetBoolL
//
// -----------------------------------------------------------------------------
//
void CAspFilterItem::SetBoolL(TBool aBool)
	{
	TSyncMLFilterPropertyDataType type = iProperty->QueryValueDataType();
	
	__ASSERT_DEBUG(type==ESyncMLDataTypeBool, TUtil::Panic(KErrGeneral));
	
	iProperty->SetQueryValueBoolL(aBool);
	}


// -----------------------------------------------------------------------------
// CAspFilterItem::IntL
//
// -----------------------------------------------------------------------------
//
TInt CAspFilterItem::IntL()
	{
	TSyncMLFilterPropertyDataType type = iProperty->QueryValueDataType();
	
	__ASSERT_DEBUG(type==ESyncMLDataTypeNumber, TUtil::Panic(KErrGeneral));
	
	return iProperty->QueryValueIntL();
	}


// -----------------------------------------------------------------------------
// CAspFilterItem::SetIntL
//
// -----------------------------------------------------------------------------
//
void CAspFilterItem::SetIntL(TInt aInt)
	{
	TSyncMLFilterPropertyDataType type = iProperty->QueryValueDataType();
	
	__ASSERT_DEBUG(type==ESyncMLDataTypeNumber, TUtil::Panic(KErrGeneral));
	
	iProperty->SetQueryValueIntL(aInt);
	}


// -----------------------------------------------------------------------------
// CAspFilterItem::DateL
//
// -----------------------------------------------------------------------------
//
TTime CAspFilterItem::DateL()
	{
	TSyncMLFilterPropertyDataType type = iProperty->QueryValueDataType();
	
	__ASSERT_DEBUG(type==ESyncMLDataTypeDate, TUtil::Panic(KErrGeneral));
	
	return iProperty->QueryValueDateL();
	}


// -----------------------------------------------------------------------------
// CAspFilterItem::SetDateL
//
// -----------------------------------------------------------------------------
//
void CAspFilterItem::SetDateL(TTime aTime)
	{
	TSyncMLFilterPropertyDataType type = iProperty->QueryValueDataType();
	
	__ASSERT_DEBUG(type==ESyncMLDataTypeDate, TUtil::Panic(KErrGeneral));
	
	return iProperty->SetQueryValueDateL(aTime);
	}


// -----------------------------------------------------------------------------
// CAspFilterItem::TimeL
//
// -----------------------------------------------------------------------------
//
TTime CAspFilterItem::TimeL()
	{
	TSyncMLFilterPropertyDataType type = iProperty->QueryValueDataType();
	
	__ASSERT_DEBUG(type==ESyncMLDataTypeTime, TUtil::Panic(KErrGeneral));
	
	return iProperty->QueryValueTimeL();
	}


// -----------------------------------------------------------------------------
// CAspFilterItem::SetTimeL
//
// -----------------------------------------------------------------------------
//
void CAspFilterItem::SetTimeL(TTime aTime)
	{
	TSyncMLFilterPropertyDataType type = iProperty->QueryValueDataType();
	
	__ASSERT_DEBUG(type==ESyncMLDataTypeTime, TUtil::Panic(KErrGeneral));
	
	return iProperty->SetQueryValueTimeL(aTime);
	}





// End of file

