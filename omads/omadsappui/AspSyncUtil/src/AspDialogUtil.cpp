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
* Description:  
*
*/


// INCLUDE FILES

#include "AspDialogUtil.h"
#include "AspUtil.h"
#include "AspSchedule.h"
#include <aspsyncutil.mbg>  // for bitmap enumerations

#include <aknnavilabel.h>
#include <akntitle.h>           // CAknTitlePane
#include <akncontext.h>         // CAknTitlePane
#include <aknnotewrappers.h>    // CAknInformationNote
#include <eikmenup.h>           // CEikMenuPane
#include <aknlists.h>           // CAknSingleHeadingPopupMenuStyleListBox

#include <aknmfnesettingpage.h>         // CAknDateSettingPage
#include <aknradiobuttonsettingpage.h>  // CAknRadioButtonSettingPage
#include <akntextsettingpage.h>         // CAknIntegerSettingPage
#include <aknmessagequerydialog.h>      // CAknMessageQueryDialog
#include <AknIndicatorContainer.h>

#include <AknIconArray.h>   // for GulArray
#include <gulicon.h>

#include "AspSettingDialog.h"
// ============================ MEMBER FUNCTIONS ===============================




/*******************************************************************************
 * class TDialogUtil
 *******************************************************************************/


//------------------------------------------------------------------------------
// TDialogUtil::ShowTextQueryL
//
//------------------------------------------------------------------------------
//
void TDialogUtil::ShowTextQueryL(const TDesC& aTitle, TDes& aText, TInt aMaxLength)
	{
	CAknTextQueryDialog* query = CAknTextQueryDialog::NewL(aText);
	CleanupStack::PushL(query);
	query->SetPromptL(aTitle);
	if (aMaxLength > 0)
		{
		query->SetMaxLength(aMaxLength);
		}
    CleanupStack::Pop(query);		

	query->ExecuteLD(R_ASP_TEXT_QUERY);
	}


//------------------------------------------------------------------------------
// TDialogUtil::ShowMessageQueryL
//
//------------------------------------------------------------------------------
//
void TDialogUtil::ShowMessageQueryL(const TDesC& aTitle, const TDesC& aText)
	{
	TPtrC ptr = aText;
	CAknMessageQueryDialog* query = CAknMessageQueryDialog::NewL(ptr);
	query->PrepareLC(R_ASP_MESSAGE_QUERY);
	_LIT(KEmpty, " ");
	if (aTitle.Compare(KEmpty) != 0 )
		{
		CAknPopupHeadingPane* headingPane = query->Heading();
		headingPane->SetTextL(aTitle);
		}
	query->RunLD();
	}

//------------------------------------------------------------------------------
// TDialogUtil::ShowAutoSyncMessageQueryL
//
//------------------------------------------------------------------------------
//
void TDialogUtil::ShowAutoSyncMessageQueryL(const TDesC& aText)
	{
	TPtrC ptr = aText;
	HBufC* hBuf = CAspResHandler::ReadLC(R_ASP_MAIN_AUTO_ERROR_LOG_LINK);
	CAknMessageQueryDialog* query = CAknMessageQueryDialog::NewL(ptr);
	CleanupStack::PushL(query);

	TCallBack LogView(TDialogUtil::ShowAutoSyncLogL);
	query->SetLink(LogView);
    query->SetLinkTextL(hBuf->Des());
	
	//CAknPopupHeadingPane* headingPane = query->Heading();
	//headingPane->SetTextL(aTitle);

	CleanupStack::Pop(query);
	query->ExecuteLD(R_ASP_MESSAGE_QUERY);

	CleanupStack::PopAndDestroy(hBuf);
	}

//------------------------------------------------------------------------------
// TDialogUtil::ShowMessageQueryL
//
//------------------------------------------------------------------------------
//
void TDialogUtil::ShowMessageQueryL(const TDesC& aTitle, TInt aResource)
	{
	HBufC* hBuf = CAspResHandler::ReadLC(aResource);
	ShowMessageQueryL(aTitle, hBuf->Des());
	CleanupStack::PopAndDestroy(hBuf);
	}

	
//------------------------------------------------------------------------------
// TDialogUtil::ShowConfirmationQueryL
//
//------------------------------------------------------------------------------
//
TBool TDialogUtil::ShowConfirmationQueryL(const TDesC& aText)
	{
	CAknQueryDialog* queryDialog = new (ELeave) CAknQueryDialog;
	TBool ret = queryDialog->ExecuteLD(R_ASP_CONFIRMATION_QUERY, aText);
	return ret;
	}

//------------------------------------------------------------------------------
// TDialogUtil::ShowRoamingConfirmationQueryL
//
//------------------------------------------------------------------------------
//
TBool TDialogUtil::ShowRoamingConfirmationQueryL(const TDesC& aText)
    {
    CAknQueryDialog* queryDialog = new (ELeave) CAknQueryDialog;
    TBool ret = queryDialog->ExecuteLD(R_ASP_ROAMING_CONFIRMATION_QUERY, aText);
    return ret;
    }

//------------------------------------------------------------------------------
// TDialogUtil::ShowRoamingConfirmationQueryL
//
//------------------------------------------------------------------------------
//
TBool TDialogUtil::ShowRoamingConfirmationQueryL(TInt aResource)
    {
    HBufC* hBuf = CAspResHandler::ReadLC(aResource);
    TBool ret = ShowRoamingConfirmationQueryL(hBuf->Des());
    CleanupStack::PopAndDestroy(hBuf);
    return ret;
    }


//------------------------------------------------------------------------------
// TDialogUtil::ShowConfirmationQueryL
//
//------------------------------------------------------------------------------
//
TBool TDialogUtil::ShowConfirmationQueryL(TInt aResource)
	{
	HBufC* hBuf = CAspResHandler::ReadLC(aResource);
	TBool ret = ShowConfirmationQueryL(hBuf->Des());
	CleanupStack::PopAndDestroy(hBuf);
	return ret;
	}


//------------------------------------------------------------------------------
// TDialogUtil::ShowInformationQueryL
//
//------------------------------------------------------------------------------
//
void TDialogUtil::ShowInformationQueryL(const TDesC& aText)
	{
	CAknQueryDialog* queryDialog = new (ELeave) CAknQueryDialog;
	queryDialog->ExecuteLD(R_ASP_INFORMATION_QUERY, aText);
	}


//------------------------------------------------------------------------------
// TDialogUtil::ShowInformationQueryL
//
//------------------------------------------------------------------------------
//
void TDialogUtil::ShowInformationQueryL(TInt aResource)
	{
	HBufC* hBuf = CAspResHandler::ReadLC(aResource);
	ShowInformationQueryL(hBuf->Des());
	CleanupStack::PopAndDestroy(hBuf);
	}


//------------------------------------------------------------------------------
// TDialogUtil::ShowInformationNoteL
//
//------------------------------------------------------------------------------
//
void TDialogUtil::ShowInformationNoteL(TInt aResource)
	{
	HBufC* hBuf = CAspResHandler::ReadLC(aResource);
    ShowInformationNoteL(hBuf->Des());
	CleanupStack::PopAndDestroy(hBuf);
	}


//------------------------------------------------------------------------------
// TDialogUtil::ShowInformationNoteL
//
//------------------------------------------------------------------------------
//
void TDialogUtil::ShowInformationNoteL(const TDesC& aText)
	{
	CAknInformationNote* queryDialog = new (ELeave) CAknInformationNote(ETrue);
	queryDialog->ExecuteLD(aText);
	}


//------------------------------------------------------------------------------
// TDialogUtil::ShowErrorNoteL
//
//------------------------------------------------------------------------------
//
void TDialogUtil::ShowErrorNoteL(TInt aResource)
	{
	HBufC* hBuf = CAspResHandler::ReadLC(aResource);
	ShowErrorNoteL(hBuf->Des());
	CleanupStack::PopAndDestroy(hBuf);
	}


//------------------------------------------------------------------------------
// TDialogUtil::ShowErrorNoteL
//
//------------------------------------------------------------------------------
//
void TDialogUtil::ShowErrorNoteL(const TDesC& aText)
	{
	CAknErrorNote* errorNote = new (ELeave) CAknErrorNote(ETrue);
	errorNote->ExecuteLD(aText);
	}


//------------------------------------------------------------------------------
// TDialogUtil::ShowErrorQueryL
//
//------------------------------------------------------------------------------
//
void TDialogUtil::ShowErrorQueryL(TInt aResource)
	{
	HBufC* hBuf = CAspResHandler::ReadLC(aResource);
	ShowErrorQueryL(hBuf->Des());
	CleanupStack::PopAndDestroy(hBuf);
	}


//------------------------------------------------------------------------------
// TDialogUtil::ShowErrorQueryL
//
//------------------------------------------------------------------------------
//
void TDialogUtil::ShowErrorQueryL(const TDesC& aText)
	{
	CAknQueryDialog* queryDialog = new (ELeave) CAknQueryDialog;
	queryDialog->ExecuteLD(R_ASP_ERROR_QUERY, aText);
	}


// -----------------------------------------------------------------------------
// TDialogUtil::ShowListQueryL
// 
// -----------------------------------------------------------------------------
TBool TDialogUtil::ShowListQueryL(const TDesC& aTitle, CDesCArray* aList, TInt* aSelectedIndex)
	{
	CAknListQueryDialog* query = new (ELeave) CAknListQueryDialog(aSelectedIndex);
	CleanupStack::PushL(query);

	query->PrepareLC(R_ASP_LIST_QUERY);
	query->SetItemTextArray(aList);
	query->SetOwnershipType(ELbmDoesNotOwnItemArray);
	CAknPopupHeadingPane* headingPane = query->QueryHeading();
	headingPane->SetTextL(aTitle);
	
	CleanupStack::Pop(query);

	return query->RunLD();
	}

//------------------------------------------------------------------------------
// TDialogUtil::ShowListQueryIconL
//
//------------------------------------------------------------------------------
TBool TDialogUtil::ShowListQueryIconL(const TDesC& aTitle, CDesCArray* aList, TInt* aSelectedIndex)
	{
	  
	 CAknListQueryDialog* query = new (ELeave) CAknListQueryDialog(aSelectedIndex);
	 CleanupStack::PushL(query);
	 query->PrepareLC(R_ASP_LIST_APP_QUERY);
	 
	 
	 query->SetItemTextArray(aList);
	 query->SetOwnershipType(ELbmOwnsItemArray);
	 
	 CArrayPtr<CGulIcon>* iconList = new (ELeave) CAknIconArray(1);
	 CleanupStack::PushL(iconList);
	 
 	 TFileName bitmapName;
 	 CAspResHandler::GetBitmapFileName(bitmapName);
 	 //renders Qgn_Prop_Sml_New icon
	 iconList->AppendL(CreateIconL(KAknsIIDQgnPropSmlNew, bitmapName, 
	                     EMbmAspsyncutilQgn_prop_sml_new,
	                     EMbmAspsyncutilQgn_prop_sml_new_mask) );
	 query->SetIconArrayL(iconList);
	 CAknPopupHeadingPane* headingPane = query->QueryHeading();
 	 headingPane->SetTextL(aTitle);
 	  
	 CleanupStack::Pop(iconList);
	 CleanupStack::Pop(query);
	 return query->RunLD();
	}

// -----------------------------------------------------------------------------
// TDialogUtil::ShowListQueryL
// 
// -----------------------------------------------------------------------------
TBool TDialogUtil::ShowListQueryL(TInt aTitle, CDesCArray* aList, TInt* aSelectedIndex)
	{
	HBufC* hBuf = CAspResHandler::ReadLC(aTitle);

    TBool ret = ShowListQueryL(hBuf->Des(), aList, aSelectedIndex);
   	CleanupStack::PopAndDestroy(hBuf);
   	
   	return ret;
	}

//------------------------------------------------------------------------------
//TDialogUtil::ShowListQueryIconL
//
//------------------------------------------------------------------------------
TBool TDialogUtil::ShowListQueryIconL(TInt aTitle, CDesCArray* aList, TInt* aSelectedIndex)
	{
	HBufC* hBuf = CAspResHandler::ReadLC(aTitle);

    TBool ret = ShowListQueryIconL(hBuf->Des(), aList, aSelectedIndex);
   	CleanupStack::PopAndDestroy(hBuf);
   	
   	return ret;
	}
// -----------------------------------------------------------------------------
// TDialogUtil::ShowProfileQueryL
// 
// -----------------------------------------------------------------------------
TBool TDialogUtil::ShowProfileQueryL(CAspProfileList* aList, TInt& aId, TInt aResource)
	{
	aId = KErrNotFound;
	
	CDesCArray* arr = new (ELeave) CDesCArrayFlat(KDefaultArraySize);
	CleanupStack::PushL(arr);

	TInt count = aList->Count();
	for (TInt i=0; i<count; i++)
		{
		TAspProfileItem& profile = aList->Item(i);
		arr->AppendL(profile.iProfileName);
		}

    TInt index=KErrNotFound;
	TBool ret = ShowListQueryL(aResource, arr, &index);
	
	if (ret)
		{
	    TAspProfileItem& profile = aList->Item(index);
	    aId = profile.iProfileId;
		}

	CleanupStack::PopAndDestroy(arr);
	return ret;
	}
//------------------------------------------------------------------------------
//TDialogUtil::ShowProfileQueryAppL
//
//-----------------------------------------------------------------------------

TBool TDialogUtil::ShowProfileQueryAppL(CAspProfileList* aList, TInt& aId, TInt aResource,TInt &index1)
{
	aId = KErrNotFound;
	
	CDesCArray* arr = new (ELeave) CDesCArrayFlat(KDefaultArraySize);
	CleanupStack::PushL(arr);
      
    
	HBufC* hBuf = CAspResHandler::ReadLC(R_ASP_SELECT_PROFILE_CREATE_NEW);
    TPtr ptr = hBuf->Des();
  
    //"%d\t%S" for formatting the icon and text data in the query dialog 
   	 _LIT(KListItemFormat,"%d\t%S");
	 TBuf<KBufSize> item;
     item.Format(KListItemFormat,0 , &ptr);
	 arr->AppendL(item);
   	 CleanupStack::PopAndDestroy(hBuf);
       
    
	 TInt count = aList->Count();
	 for (TInt i=0; i< count; i++)
		{
		TAspProfileItem& profile = aList->Item(i);
		//"\t%S" no need of keeping icon for profile list
		_LIT(KListItemFormat,"\t%S");
		HBufC* hBuf1=TUtil::StrConversion(profile.iProfileName);
		TPtr ptr1=hBuf1->Des();
		item.Format(KListItemFormat,&ptr1);
		CleanupStack::PopAndDestroy(hBuf1);
	    arr->AppendL(item);
		}
		
	
	TInt index=KErrNotFound;
	TBool ret = ShowListQueryIconL(aResource, arr, &index);

	if (ret)
		{
		
		  if(index==0) //to check if R_ASP_SELECT_PROFILE_CREATE_NEW is selected
				{
	   				 ret=EFalse;
	   				 index1=1;
	    			 CleanupStack::Pop(arr);
					 return ret;
				}
		
	    TAspProfileItem& profile = aList->Item(index-1);
	    aId = profile.iProfileId;
	    
		}

	CleanupStack::Pop(arr);
	return ret;
}


// -----------------------------------------------------------------------------
// TDialogUtil::DeleteMenuItem
//
// -----------------------------------------------------------------------------
//
void TDialogUtil::DeleteMenuItem(CEikMenuPane* aMenuPane, TInt aCommandId)
	{
	TInt pos=KErrNotFound;
    if (aMenuPane->MenuItemExists(aCommandId, pos))
		{
		aMenuPane->DeleteMenuItem(aCommandId);
		}
	}


// -----------------------------------------------------------------------------
// TDialogUtil::CreateIconL
//
// Constructs an independent CGulIcon object with fallback support.
// If no matching item (aId) is found in the currently active skin,
// attempts to construct the item using the given file (last 3 parameters).
// -----------------------------------------------------------------------------
//
CGulIcon* TDialogUtil::CreateIconL(TAknsItemID aId, const TDesC& aFileName, TInt aFileBitmapId, TInt aFileMaskId)
	{
    CGulIcon* icon = AknsUtils::CreateGulIconL(AknsUtils::SkinInstance(), aId, aFileName, aFileBitmapId, aFileMaskId);
	return icon;
	}


// -----------------------------------------------------------------------------
// TDialogUtil::CreateImageL
//
// -----------------------------------------------------------------------------
//
CEikImage* TDialogUtil::CreateImageLC(TAknsItemID aId, const TDesC& aFileName, TInt aFileBitmapId, TInt aFileMaskId)
	{
	CFbsBitmap* bitmap1 = NULL;
	CFbsBitmap* bitmap2 = NULL;
	
	CEikImage* image = new (ELeave) CEikImage;
	CleanupStack::PushL(image);

	AknsUtils::CreateIconLC(AknsUtils::SkinInstance(), aId, bitmap1, bitmap2, aFileName, aFileBitmapId, aFileMaskId);
	image->SetPicture(bitmap1, bitmap2);
	CleanupStack::Pop(2); // bitmap1 bitmap2

	return image;
	}


// -----------------------------------------------------------------------------
// TDialogUtil::ConstructListBoxItemL
//
// -----------------------------------------------------------------------------
//
HBufC* TDialogUtil::ConstructListBoxItemLC(const TDesC& aFirstLine, const TDesC& aSecondLine, TInt /*aIndex*/)
	{
    _LIT(KFormat, "%d\t%S\t%S");
	
	TInt len = aFirstLine.Length() + aSecondLine.Length() + KBufSize16;
	
	HBufC* hBuf = HBufC::NewLC(len);
	TPtr ptr = hBuf->Des();
	
	ptr.Format(KFormat, 1, &aFirstLine, &aSecondLine);
	
	
	return hBuf;
	}


// -----------------------------------------------------------------------------
// TDialogUtil::ShowTimeEditorL
//
// -----------------------------------------------------------------------------
//
TBool TDialogUtil::ShowTimeEditorL(TTime& aTime, const TDesC& aTitle)
	{
    CAknTimeSettingPage* dlg = new (ELeave) CAknTimeSettingPage(R_ASP_SETTING_TIME, aTime);
    
   	CleanupStack::PushL(dlg);
	dlg->ConstructL();
	dlg->SetSettingTextL(aTitle);
	CleanupStack::Pop(dlg);

    
    if (dlg->ExecuteLD(CAknSettingPage::EUpdateWhenAccepted))
        {
        return ETrue;
        }
        
    return EFalse;
	}


// -----------------------------------------------------------------------------
// TDialogUtil::ShowDateEditorL
//
// -----------------------------------------------------------------------------
//
TBool TDialogUtil::ShowDateEditorL(TTime& aTime, const TDesC& aTitle)
	{
    CAknDateSettingPage* dlg = new (ELeave) CAknDateSettingPage(R_ASP_SETTING_DATE, aTime);
    
   	CleanupStack::PushL(dlg);
	dlg->ConstructL();
	dlg->SetSettingTextL(aTitle);
	CleanupStack::Pop(dlg);

    if (dlg->ExecuteLD(CAknSettingPage::EUpdateWhenAccepted))
        {
        return ETrue;
        }
        
    return EFalse;
	}


//------------------------------------------------------------------------------
// TDialogUtil::ShowListEditorL
//
//------------------------------------------------------------------------------
//
TBool TDialogUtil::ShowListEditorL(CDesCArray* aList, const TDesC& aTitle, TInt& aCurrent)
	{

	CAknRadioButtonSettingPage* dlg = new (ELeave) CAknRadioButtonSettingPage(
	                                       R_ASP_SETTING_RADIO, aCurrent, aList);
	CleanupStack::PushL(dlg);
	dlg->SetSettingTextL(aTitle); 
	CleanupStack::Pop(dlg);

	return dlg->ExecuteLD();
	}

//------------------------------------------------------------------------------
// TDialogUtil::ShowListEditorL
//
//------------------------------------------------------------------------------
//
TBool TDialogUtil::ShowListEditorL(TInt aResourceId, CDesCArray* aList, const TDesC& aTitle, TInt& aCurrent)
	{
	CAspRadioButtonSettingPageEditor* dlg = new (ELeave) CAspRadioButtonSettingPageEditor(
	                                       aResourceId, aCurrent, aList);
	CleanupStack::PushL(dlg);
	dlg->SetSettingTextL(aTitle); 
	CleanupStack::Pop(dlg);

	return dlg->ExecuteLD();
	}


//------------------------------------------------------------------------------
// TDialogUtil::ShowTextEditorL
//
//------------------------------------------------------------------------------
//
TBool TDialogUtil::ShowTextEditorL(TInt aResourceId, TDes& aText, const TDesC& aTitle, TBool aMandatory, TBool aLatinInput, TInt aMaxLength)
	{
	TInt flags(CAknTextSettingPage::EZeroLengthAllowed);

	if (aMandatory)
		{
		flags = CAknTextSettingPage::EZeroLengthNotOffered;
		}

	CAspTextSettingPageEditor * dlg = new (ELeave) CAspTextSettingPageEditor(aResourceId, aText, flags);
	CleanupStack::PushL(dlg);

	dlg->SetSettingTextL(aTitle); 
	dlg->ConstructL();
	dlg->TextControl()->SetTextLimit(aMaxLength);

	if (aLatinInput)
		{		
		dlg->TextControl()->SetAvkonWrap(ETrue);
		dlg->TextControl()->SetAknEditorCase(EAknEditorLowerCase);
		dlg->TextControl()->SetAknEditorFlags(EAknEditorFlagNoT9 | EAknEditorFlagLatinInputModesOnly);
		}
	
	CleanupStack::Pop(dlg);

	// CAknTextSettingPage::ExecuteLD only return boolean. See CAknSettingPage::ProcessCommandL
	// what softkey values map to ETrue and EFalse.
	TInt ret = dlg->ExecuteLD(CAknSettingPage::EUpdateWhenChanged);
	if (ret)
		{
    	return ETrue;
		}

	return EFalse;
	}


//------------------------------------------------------------------------------
// TDialogUtil::ShowTextEditorL
//
//------------------------------------------------------------------------------
//
TBool TDialogUtil::ShowTextEditorL(TDes& aText, const TDesC& aTitle, TBool aMandatory, TBool aLatinInput, TInt aMaxLength)
	{
	return ShowTextEditorL(R_ASP_SETTING_TEXT, aText, aTitle, aMandatory, aLatinInput, aMaxLength);
	}


//------------------------------------------------------------------------------
// TDialogUtil::ShowIntegerEditorL
//
//------------------------------------------------------------------------------
//
TBool TDialogUtil::ShowIntegerEditorL(TInt& aNumber, const TDesC& aTitle, TInt aMin, TInt aMax, TBool aLatinInput)
	{
	CAspIntegerSettingPageEditor* dlg = new (ELeave) CAspIntegerSettingPageEditor(
	                              R_ASP_SETTING_NUMBER, aNumber,
	                              CAknIntegerSettingPage::EInvalidValueNotOffered);
	CleanupStack::PushL(dlg);
	dlg->ConstructL();
	dlg->SetSettingTextL(aTitle);
	CleanupStack::Pop(dlg);
	
	CAknIntegerEdwin* editor = dlg->IntegerEditorControl();
	if (aMin != KErrNotFound)
		{
		editor->SetMinimumIntegerValue(aMin);
		}
	if (aMax != KErrNotFound)
		{
		editor->SetMaximumIntegerValue(aMax);
		}

	editor->SetAknEditorNumericKeymap(EAknEditorPlainNumberModeKeymap);
	
	if (aLatinInput)
		{
		editor->SetAknEditorFlags(EAknEditorFlagLatinInputModesOnly);
		editor->SetValueL(aNumber);
		}

	if (dlg->ExecuteLD(CAknIntegerSettingPage::EUpdateWhenChanged))
		{
		return ETrue;
		}

	return EFalse;
	}


// -----------------------------------------------------------------------------
// TDialogUtil::ShowPopupDialogL
// 
// -----------------------------------------------------------------------------
void TDialogUtil::ShowPopupDialogL(const TDesC& aText, CDesCArray* aArray)
	{
	CAknSingleHeadingPopupMenuStyleListBox* listBox = new (ELeave) CAknSingleHeadingPopupMenuStyleListBox;
	CleanupStack::PushL(listBox);


	CAknPopupList* popupList = CAknPopupList::NewL(listBox, R_AVKON_SOFTKEYS_OK_EMPTY, AknPopupLayouts::EMenuWindow);
	CleanupStack::PushL(popupList);

	listBox->ConstructL(popupList, EAknListBoxViewerFlags);
	
	CDesCArray* array = (CDesCArray*) listBox->Model()->ItemTextArray();
	listBox->CreateScrollBarFrameL(ETrue); 
	listBox->ScrollBarFrame()->	SetScrollBarVisibilityL(CEikScrollBarFrame::EOff, CEikScrollBarFrame::EAuto);
    
	TInt count = aArray->Count();
	for (TInt i=0; i<count; i++)
		{
		TPtrC ptr = (*aArray)[i];
		array->AppendL(ptr);
		}
	
	popupList->SetTitleL(aText);

	popupList->ExecuteLD();

	CleanupStack::Pop(popupList);
	CleanupStack::PopAndDestroy(listBox);
	}


// -----------------------------------------------------------------------------
// TDialogUtil::ShowPopupDialog2L
// 
// -----------------------------------------------------------------------------
void TDialogUtil::ShowPopupDialog2L(const TDesC& aText, CDesCArray* aArray)
	{
    CAknDoublePopupMenuStyleListBox* listBox = new(ELeave) CAknDoublePopupMenuStyleListBox;
    
    CleanupStack::PushL(listBox);

    // Avkon popup list with heading
    CAknPopupList* popupList = CAknPopupList::NewL(listBox, R_AVKON_SOFTKEYS_OK_EMPTY, AknPopupLayouts::EMenuDoubleWindow);
    CleanupStack::PushL(popupList);
    
    // Set listbox properties and create item array
    listBox->ConstructL(popupList, EAknListBoxViewerFlags);
    listBox->CreateScrollBarFrameL(ETrue);
    listBox->ScrollBarFrame()->SetScrollBarVisibilityL(CEikScrollBarFrame::EOff, CEikScrollBarFrame::EAuto);
    
   	CDesCArray* array = (CDesCArray*) listBox->Model()->ItemTextArray();
	TInt count = aArray->Count();
	for (TInt i=0; i<count; i++)
		{
		TPtrC ptr = (*aArray)[i];
		array->AppendL(ptr);
		}
    popupList->SetTitleL(aText);
    	
    popupList->ExecuteLD();

    CleanupStack::Pop(popupList);
    CleanupStack::PopAndDestroy(listBox);
	}

// -----------------------------------------------------------------------------
// TDialogUtil::ShowAutoSyncLogL
// 
// -----------------------------------------------------------------------------
TInt TDialogUtil::ShowAutoSyncLogL(TAny* /*aAny*/)
	{

	CAspSchedule* schedule = CAspSchedule::NewLC();
	TRAPD (err,schedule->ShowAutoSyncLogL());
	CleanupStack::PopAndDestroy(schedule);

	if (err == KErrNone)
		{
		return ETrue;
		}
	return EFalse;
	}

//-----------------------------------------------------------------------------
//
//Does Descriptor contain string "PC Suite"
//Its NOT case sensitive
//
//-----------------------------------------------------------------------------
TBool TDialogUtil::IsPCSuiteDesC(const TDesC& des)
{
	_LIT(KPCSuite, "pc suite");
	TPtrC pcSuitePtr(KPCSuite);
	
	TBuf<KBufSize64> buf;
	TUtil::StrCopy(buf,des);
	buf.LowerCase();
	if (buf.Match(pcSuitePtr) == 0)
	{
		return ETrue;
	}
		
	return EFalse;
}
	    		

/*******************************************************************************
 * class CAspIndicatorHandler
 *******************************************************************************/


// -----------------------------------------------------------------------------
// Destructor
//
// -----------------------------------------------------------------------------
//
CAspIndicatorHandler::~CAspIndicatorHandler()
    {
    if (iIndicator && iIndicatorContainer && iIndicatorModified)
    	{
        if (iPreviousIndicatorType == EAknNaviPaneEditorIndicatorMessageLength)
        	{
        	TRAP_IGNORE( iIndicatorContainer->SetIndicatorValueL(
                      TUid::Uid(EAknNaviPaneEditorIndicatorMessageLength), KNullDesC) );
            }

        iIndicatorContainer->SetIndicatorState(TUid::Uid(iIndicatorType), EAknIndicatorStateOff);
        }
    }


// -----------------------------------------------------------------------------
// CAspIndicatorHandler::CAspIndicatorHandler
//
// -----------------------------------------------------------------------------
//
CAspIndicatorHandler::CAspIndicatorHandler(MAknEditingStateIndicator* aIndicator) : iIndicator(aIndicator)
	{
	iIndicatorContainer = iIndicator->IndicatorContainer();
    iIndicatorModified = EFalse;
    iPreviousIndicatorType = KErrNotFound;
	}


// ----------------------------------------------------------------------------
// CAspIndicatorHandler::SetIndicatorStateL
// 
// ----------------------------------------------------------------------------
//
void CAspIndicatorHandler::SetIndicatorStateL(TInt aIndicatorType, const TDesC& aText)
	{
    if (iIndicator && iIndicatorContainer)
    	{
   	
        if (iPreviousIndicatorType == EAknNaviPaneEditorIndicatorMessageLength)
        	{
        	iIndicatorContainer->SetIndicatorValueL(
                      TUid::Uid(EAknNaviPaneEditorIndicatorMessageLength), KNullDesC);
            }

    	iIndicatorContainer->SetIndicatorState(TUid::Uid(aIndicatorType),
                                               EAknIndicatorStateOn);
                                               
        iIndicatorModified = ETrue;
        iPreviousIndicatorType = aIndicatorType;
        
                                               
        if (aIndicatorType == EAknNaviPaneEditorIndicatorMessageLength)
        	{
        	iIndicatorContainer->SetIndicatorValueL(
                                 TUid::Uid(EAknNaviPaneEditorIndicatorMessageLength), aText);
            iPreviousIndicatorType = aIndicatorType;
            }
    	}
	
	}



/*******************************************************************************
 * class CAspNaviPaneHandler
 *******************************************************************************/


// -----------------------------------------------------------------------------
// Destructor
//
// -----------------------------------------------------------------------------
//
CAspNaviPaneHandler::~CAspNaviPaneHandler()
    {
   	if (iNaviDecorator)
		{
		if (iNaviPane && iNavidecoratorPushed)
			{
			iNaviPane->Pop(iNaviDecorator);
			}
	
		delete iNaviDecorator;
		}
		
	if (iNaviPane && iNavipanePushed)
		{
		iNaviPane->Pop(); // restore previous navi pane
		}
    }


// -----------------------------------------------------------------------------
// CAspNaviPaneHandler::CAspNaviPaneHandler
//
// -----------------------------------------------------------------------------
//
CAspNaviPaneHandler::CAspNaviPaneHandler(CEikStatusPane* aStatusPane) : iStatusPane(aStatusPane)
	{
	iNavidecoratorPushed = EFalse;
	iNavipanePushed = EFalse;
	
	if (iStatusPane)
		{
		TRAP_IGNORE( iNaviPane = (CAknNavigationControlContainer*)iStatusPane->ControlL(TUid::Uid(EEikStatusPaneUidNavi)) );
		}
	}


// ----------------------------------------------------------------------------
// CAspNaviPaneHandler::SetNaviPaneTitleL
// 
// ----------------------------------------------------------------------------
//
void CAspNaviPaneHandler::SetNaviPaneTitleL(const TDesC& aTitle)
	{
	if (!iStatusPane || !iNaviPane)
		{
		return;
		}

    if (iNaviDecorator)
        {
        CAknNaviLabel* naviLabel = static_cast<CAknNaviLabel*>(iNaviDecorator->DecoratedControl());
        if (naviLabel)
        	{
        	naviLabel->SetTextL(aTitle);
        	iNaviDecorator->DrawDeferred();
        	}
        }
     else
    	{
    	iNaviDecorator = iNaviPane->CreateNavigationLabelL(aTitle);
    	iNaviPane->PushL(*iNaviDecorator); // activate navi label in navi pane
    	iNavidecoratorPushed = ETrue;
    	}
    	
    //CFbsBitmap* bitmap = NULL;
    //CFbsBitmap* bitmapMask = NULL;
    //AknsUtils::CreateIconL(AknsUtils::SkinInstance(), KAknsIIDQgnPropSmlHttp,
    //                       bitmap, bitmapMask, bitmapName,
    //                       EMbmAspsyncutilQgn_prop_sml_http, EMbmAspsyncutilQgn_prop_sml_http_mask);
    //iNaviDecorator = naviPane->CreateNavigationImageL(bitmap, bitmapMask);
    //naviPane->PushL(*iNaviDecorator);
    	
	}


// ----------------------------------------------------------------------------
// CAspNaviPaneHandler::StoreNavipaneL
// 
// ----------------------------------------------------------------------------
//
void CAspNaviPaneHandler::StoreNavipaneL()
	{
	if (!iNaviPane || !iStatusPane || iNavipanePushed)
		{
		return;
		}

	if (iStatusPane->PaneCapabilities(TUid::Uid(EEikStatusPaneUidTitle)).IsPresent())
		{
		iNaviPane->PushDefaultL();
		iNavipanePushed = ETrue;
		}
	}

#ifdef RD_DSUI_TIMEDSYNC

/*******************************************************************************
 * class CAspTabbedNaviPaneHandler
 *******************************************************************************/


// -----------------------------------------------------------------------------
// Destructor
//
// -----------------------------------------------------------------------------
//
CAspTabbedNaviPaneHandler::~CAspTabbedNaviPaneHandler()
    {
   	if (iNaviDecorator)
		{
		if (iNaviPane && iNavidecoratorPushed)
			{
			iNaviPane->Pop(iNaviDecorator);
			}
	
		delete iNaviDecorator;
		}
		
	if (iNaviPane && iNavipanePushed)
		{
		iNaviPane->Pop(); // restore previous navi pane
		}
    }


// -----------------------------------------------------------------------------
// CAspTabbedNaviPaneHandler::CAspTabbedNaviPaneHandler
//
// -----------------------------------------------------------------------------
//
CAspTabbedNaviPaneHandler::CAspTabbedNaviPaneHandler(CEikStatusPane* aStatusPane ,CAspSettingDialog *aDialog) 
						  : iStatusPane(aStatusPane)
	{
	iNavidecoratorPushed = EFalse;
	iNavipanePushed = EFalse;
	
	iDialog = aDialog;
	
	if (iStatusPane)
		{
		TRAP_IGNORE( iNaviPane = (CAknNavigationControlContainer*)iStatusPane->ControlL(TUid::Uid(EEikStatusPaneUidNavi)) );
		}
		
    if (iNaviPane)
        {
        TRAP_IGNORE(iNaviDecorator = iNaviPane->CreateTabGroupL(this));    
        if (iNaviDecorator)
            {
            TRAP_IGNORE(iNaviPane->PushL(*iNaviDecorator));
            iNavidecoratorPushed = ETrue;
    
            iTabGroup = static_cast<CAknTabGroup*>( iNaviDecorator->DecoratedControl() );
            }
        }
    }

// -----------------------------------------------------------------------------
// CAspTabbedNaviPaneHandler::SetTabWidthL
//
// -----------------------------------------------------------------------------
//
void CAspTabbedNaviPaneHandler::SetTabWidthL()
{
	
	if (iTabGroup)
	    {
	     if (iTabGroup->TabCount() == 1)
	         {
	         iTabGroup->SetTabFixedWidthL(KTabWidthWithOneTab);     	
	         }
	     else if (iTabGroup->TabCount() == 2)
	         {
	         iTabGroup->SetTabFixedWidthL(KTabWidthWithTwoLongTabs);     	
	         }
	     else
	         {
	         iTabGroup->SetTabFixedWidthL(KTabWidthWithThreeLongTabs);     		
	         }
	      }
}
// -----------------------------------------------------------------------------
// CAspTabbedNaviPaneHandler::SetTabTextL
//
// -----------------------------------------------------------------------------
//
void CAspTabbedNaviPaneHandler::SetTabTextL(const TDesC& aText, TInt aTabId)
	{
	if (iTabGroup)
	    {
	    iTabGroup->ReplaceTabTextL(aTabId, aText);
	    }
	}


// -----------------------------------------------------------------------------
// CAspTabbedNaviPaneHandler::AddTabL
//
// -----------------------------------------------------------------------------
//
void CAspTabbedNaviPaneHandler::AddTabL(const TDesC& aText, TInt aTabId)
	{
	if (iTabGroup)
	    {
	    iTabGroup->AddTabL(aTabId, aText);
	    }
	}


// -----------------------------------------------------------------------------
// CAspTabbedNaviPaneHandler::DeleteTabsL
//
// -----------------------------------------------------------------------------
//
void CAspTabbedNaviPaneHandler::DeleteTabsL()
	{
	if (iTabGroup)
	    {
	    TInt count = iTabGroup->TabCount();
	    for (TInt i=0; i<count; i++)
	        {
	        TInt id = iTabGroup->TabIdFromIndex(0);
	        iTabGroup->DeleteTabL(id);
	        }
	    }
	}


// -----------------------------------------------------------------------------
// CAspTabbedNaviPaneHandler::SetActiveTabL
//
// -----------------------------------------------------------------------------
//
void CAspTabbedNaviPaneHandler::SetActiveTabL(TInt aTabId)
	{
	if (iTabGroup)
	    {
	    TInt index = iTabGroup->TabIndexFromId(aTabId);
	    if (index != KErrNotFound)
	        {
	        iTabGroup->SetActiveTabById(aTabId);    
	        }
	    }
	}


// ----------------------------------------------------------------------------
// CAspTabbedNaviPaneHandler::StoreNavipaneL
// 
// ----------------------------------------------------------------------------
//
void CAspTabbedNaviPaneHandler::StoreNavipaneL()
	{
	if (!iNaviPane || !iStatusPane || iNavipanePushed)
		{
		return;
		}

	if (iStatusPane->PaneCapabilities(TUid::Uid(EEikStatusPaneUidTitle)).IsPresent())
		{
		iNaviPane->PushDefaultL();
		iNavipanePushed = ETrue;
		}
	}

// ----------------------------------------------------------------------------
// CAspTabbedNaviPaneHandler::TabIndex
// 
// ----------------------------------------------------------------------------
//
TInt CAspTabbedNaviPaneHandler::TabIndex(TInt aTabId)
	{
	return iTabGroup->TabIndexFromId(aTabId);
	}

// ----------------------------------------------------------------------------
// CAspTabbedNaviPaneHandler::TabChangedL (from MAknTabObserver)
// 
// ----------------------------------------------------------------------------
//
void CAspTabbedNaviPaneHandler::TabChangedL(TInt aIndex)
	{
	TInt profileId = iTabGroup->TabIdFromIndex(aIndex );
	iDialog->SetCurrentProfile(profileId);
	iDialog->UpdateTabsL();
	iDialog->UpdateListBoxL();
	}

#endif




/*******************************************************************************
 * class CStatusPaneHandler
 *******************************************************************************/

// -----------------------------------------------------------------------------
// CStatusPaneHandler::NewL
//
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CStatusPaneHandler* CStatusPaneHandler::NewL(CAknAppUi* aAppUi)
    {
    CStatusPaneHandler* self = new (ELeave) CStatusPaneHandler(aAppUi);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
    }


// -----------------------------------------------------------------------------
// Destructor
//
// -----------------------------------------------------------------------------
//
CStatusPaneHandler::~CStatusPaneHandler()
    {
	TRAP_IGNORE(RestoreOriginalTitleL());
	TRAP_IGNORE(RestoreOriginalImageL());
	
	delete iNaviPaneHandler;
    }


// -----------------------------------------------------------------------------
// CStatusPaneHandler::ConstructL
//
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CStatusPaneHandler::ConstructL(void)
    {
    iNaviPaneHandler = new (ELeave) CAspNaviPaneHandler(iAppUi->StatusPane());
    }


// -----------------------------------------------------------------------------
// CStatusPaneHandler::CStatusPaneHandler
//
// -----------------------------------------------------------------------------
//
CStatusPaneHandler::CStatusPaneHandler(CAknAppUi* aAppUi) : iAppUi(aAppUi)
	{
	__ASSERT_ALWAYS(iAppUi, TUtil::Panic(KErrGeneral));

	iOriginalTitleStored = EFalse;
	iOriginalTitle = KNullDesC;
	}


// ----------------------------------------------------------------------------
// CStatusPaneHandler::GetTitleL
// 
// ----------------------------------------------------------------------------
//
TBool CStatusPaneHandler::GetTitleL(CAknAppUi* aAppUi, TDes& aTitle)
	{
	aTitle = KNullDesC;

	CEikStatusPane* statusPane = aAppUi->StatusPane();
    if (statusPane && statusPane->PaneCapabilities(TUid::Uid(EEikStatusPaneUidTitle)).IsPresent())
		{
        CAknTitlePane* titlePane = static_cast<CAknTitlePane*>
            (statusPane->ControlL(TUid::Uid(EEikStatusPaneUidTitle)));
        if (titlePane->Text()) 
			{
		    TUtil::StrCopy(aTitle, *titlePane->Text());
			return ETrue;
    		}
		}

	return EFalse;
	}


// ----------------------------------------------------------------------------
// CStatusPaneHandler::SetTitleL
// 
// ----------------------------------------------------------------------------
//
TBool CStatusPaneHandler::SetTitleL(CAknAppUi* aAppUi, const TDesC& aTitle)
	{
	CEikStatusPane* statusPane = aAppUi->StatusPane();
    if (statusPane && statusPane->PaneCapabilities(TUid::Uid(EEikStatusPaneUidTitle)).IsPresent())
		{
        CAknTitlePane* titlePane = static_cast<CAknTitlePane*>
            (statusPane->ControlL(TUid::Uid(EEikStatusPaneUidTitle)));
        if (titlePane->Text()) 
			{
	   		titlePane->SetTextL(aTitle);
			return ETrue;
			}
		}
	return EFalse;
	}


// ----------------------------------------------------------------------------
// CStatusPaneHandler::SetNaviPaneTitleL
// 
// ----------------------------------------------------------------------------
//
void CStatusPaneHandler::SetNaviPaneTitleL(const TDesC& aTitle)
	{
	iNaviPaneHandler->SetNaviPaneTitleL(aTitle);
	}


// ----------------------------------------------------------------------------
// CStatusPaneHandler::SetNaviPaneTitle
// 
// ----------------------------------------------------------------------------
//
void CStatusPaneHandler::SetNaviPaneTitle(const TDesC& aTitle)
	{
	TRAP_IGNORE(iNaviPaneHandler->SetNaviPaneTitleL(aTitle));
	}


// ----------------------------------------------------------------------------
// CStatusPaneHandler::SetNaviPaneTitleL
// 
// ----------------------------------------------------------------------------
//
void CStatusPaneHandler::SetNaviPaneTitleL(TInt aResource)
	{
	HBufC* hBuf = CAspResHandler::ReadLC(aResource);
    SetNaviPaneTitleL(hBuf->Des());
	CleanupStack::PopAndDestroy(hBuf);
	}


// ----------------------------------------------------------------------------
// CStatusPaneHandler::StoreNavipaneL
// 
// ----------------------------------------------------------------------------
//
void CStatusPaneHandler::StoreNavipaneL()
	{
	iNaviPaneHandler->StoreNavipaneL();
	}


// ----------------------------------------------------------------------------
// CStatusPaneHandler::StoreOriginalTitleL
// 
// ----------------------------------------------------------------------------
//
void CStatusPaneHandler::StoreOriginalTitleL()
	{
	TBool ret = GetTitleL(iAppUi, iOriginalTitle);
	
	if (ret)
		{
		iOriginalTitleStored = ETrue;
		}
	}


// ----------------------------------------------------------------------------
// CStatusPaneHandler::RestoreOriginalTitleL
// 
// ----------------------------------------------------------------------------
//
void CStatusPaneHandler::RestoreOriginalTitleL()
	{
	if (iOriginalTitleStored)
		{
		SetTitleL(iAppUi, iOriginalTitle);
		}
	}


// ----------------------------------------------------------------------------
// CStatusPaneHandler::SetTitleL
// 
// ----------------------------------------------------------------------------
//
void CStatusPaneHandler::SetTitleL(const TDesC& aText)
	{
	if (iOriginalTitleStored)
		{
		SetTitleL(iAppUi, aText);
		}
	}


// ----------------------------------------------------------------------------
// CStatusPaneHandler::SetTitleL
// 
// ----------------------------------------------------------------------------
//
void CStatusPaneHandler::SetTitleL(TInt aResourceId)
	{
	HBufC* hBuf = CAspResHandler::ReadLC(aResourceId);
	SetTitleL(hBuf->Des());
	CleanupStack::PopAndDestroy(hBuf);
	}


// ----------------------------------------------------------------------------
// CStatusPaneHandler::SetNewImageL
// 
// ----------------------------------------------------------------------------
//
void CStatusPaneHandler::SetNewImageL(const TDesC& aFilename,TInt aMainId,TInt aMaskId)
	{
	__ASSERT_DEBUG(!iOriginalImage, TUtil::Panic(KErrGeneral)); // can set image only once

	CEikImage* image = new (ELeave) CEikImage;
	CleanupStack::PushL(image);
	
	image->CreatePictureFromFileL(aFilename, aMainId, aMaskId);

	iOriginalImage = SetImageL(iAppUi, image);

	if (iOriginalImage)
		{
        CleanupStack::Pop(image); // ownership transferred to context pane
		return;
		}

	CleanupStack::PopAndDestroy(image);
	}


// ----------------------------------------------------------------------------
// CStatusPaneHandler::RestoreOriginalImageL
// 
// ----------------------------------------------------------------------------
//
void CStatusPaneHandler::RestoreOriginalImageL()
	{
	if (!iOriginalImage)
		{
		return; // image has not been replaced
		}

	CEikImage* image = NULL;
	TRAPD(err, image = SetImageL(iAppUi, iOriginalImage));

	if (err == KErrNone && image)
		{
		delete image; // delete our own image
		}
	else
		{
		delete iOriginalImage; // delete original image (could not restore it)
		iOriginalImage = NULL;
		}
	}


// ----------------------------------------------------------------------------
// CStatusPaneHandler::SetImageL
// 
// ----------------------------------------------------------------------------
//
CEikImage* CStatusPaneHandler::SetImageL(CAknAppUi* aAppUi, CEikImage* aIcon)
	{
	CEikStatusPane* statusPane = aAppUi->StatusPane();
    if (statusPane && statusPane->PaneCapabilities(TUid::Uid(EEikStatusPaneUidContext)).IsPresent())
		{
        CAknContextPane* contextPane = static_cast<CAknContextPane*>
            (statusPane->ControlL(TUid::Uid(EEikStatusPaneUidContext)));
        if (contextPane) 
			{
	   		return contextPane->SwapPicture(aIcon);
			}
		}

	return NULL;
	}




/*******************************************************************************
 * class CAspSelectionItemList
 *******************************************************************************/


// -----------------------------------------------------------------------------
// CAspSelectionItemList::Cleanup
//
// -----------------------------------------------------------------------------
//
void CAspSelectionItemList::Cleanup(TAny* aObj)
    {
    CAspSelectionItemList* list = static_cast<CAspSelectionItemList*>(aObj);
	list->ResetAndDestroy();
	delete list;
	list = NULL;
    }


// -----------------------------------------------------------------------------
// CAspSelectionItemList::CAspSelectionItemList
//
// -----------------------------------------------------------------------------
//
CAspSelectionItemList::CAspSelectionItemList(TInt aGranularity) 
: CSelectionItemList(aGranularity)
	{
	}



/*******************************************************************************
 * class CAspListItemData
 *******************************************************************************/



//------------------------------------------------------------------------------
// CAspListItemData::NewLC
// 
//------------------------------------------------------------------------------
//
CAspListItemData* CAspListItemData::NewLC()
	{
	CAspListItemData* self = new (ELeave) CAspListItemData;
	CleanupStack::PushL( self);
	self->ConstructL();
	return self;
	}


//------------------------------------------------------------------------------
// CAspListItemData::ConstructL
//
//------------------------------------------------------------------------------
//
void CAspListItemData::ConstructL()
	{
	Init();
	
    iHeader  = HBufC::NewL(0);
	iDisplayValue = HBufC::NewL(0);
	iValue = HBufC::NewL(0);
    }


// -----------------------------------------------------------------------------
// CAspListItemData::CAspListItemData
// -----------------------------------------------------------------------------
//
CAspListItemData::CAspListItemData()
	{
	}


//------------------------------------------------------------------------------
// Destructor
//
//------------------------------------------------------------------------------
//
CAspListItemData::~CAspListItemData()
	{
	delete iHeader;
	delete iDisplayValue;
	delete iValue;
	}


//------------------------------------------------------------------------------
// CAspListItemData::Header
//
//------------------------------------------------------------------------------
//
const TDesC& CAspListItemData::Header()
	{
	__ASSERT_ALWAYS(iHeader, TUtil::Panic(KErrGeneral));
	
	return *iHeader;
    }


//------------------------------------------------------------------------------
// CAspListItemData::SetHeaderL
// 
//------------------------------------------------------------------------------
//
void CAspListItemData::SetHeaderL(const TDesC& aText)
	{
	delete iHeader;
	iHeader = NULL;
	iHeader = aText.AllocL();
	}


//------------------------------------------------------------------------------
// CAspListItemData::SetHeaderL
// 
//------------------------------------------------------------------------------
//
void CAspListItemData::SetHeaderL(TInt aResourceId)
	{
	HBufC* hBuf = CAspResHandler::ReadLC(aResourceId);
    SetHeaderL(hBuf->Des());
	CleanupStack::PopAndDestroy(hBuf);
	}


//------------------------------------------------------------------------------
// CAspListItemData::Value
// 
//------------------------------------------------------------------------------
//
const TDesC& CAspListItemData::Value()
	{
	__ASSERT_ALWAYS(iValue, TUtil::Panic(KErrGeneral));
	
	return *iValue;
	}


//------------------------------------------------------------------------------
// CAspListItemData::SetValue
// 
//------------------------------------------------------------------------------
//
void CAspListItemData::SetValueL(const TDesC& aText)
	{
    delete iValue;
    iValue = NULL;
	iValue = aText.AllocL();
	}


//------------------------------------------------------------------------------
// CAspListItemData::SetValueL
// 
//------------------------------------------------------------------------------
//
void CAspListItemData::SetValueL(TInt aNumber)
	{
	TBuf<KBufSize32> buf;
	buf.Num(aNumber);
	SetValueL(buf);
	}


//------------------------------------------------------------------------------
// CAspListItemData::SetDisplayValue
// 
//------------------------------------------------------------------------------
//
void CAspListItemData::SetDisplayValueL(const TDesC& aText)
	{
	delete iDisplayValue;
	iDisplayValue = NULL;
	iDisplayValue = aText.AllocL();
    }


//------------------------------------------------------------------------------
// CAspListItemData::SetDisplayValue
// 
//------------------------------------------------------------------------------
//
void CAspListItemData::SetDisplayValueL(TInt aResource)
	{
	delete iDisplayValue;
	iDisplayValue = NULL;
	
	iDisplayValue = StringLoader::LoadL(aResource);
	}


//------------------------------------------------------------------------------
// CAspListItemData::IsEmpty
// 
//------------------------------------------------------------------------------
//
TBool CAspListItemData::IsEmpty()
	{
	if (!iValue)
		{
		return ETrue;
		}
		
	if (TUtil::IsEmpty(iValue->Des()))
		{
		return ETrue;
		}
		
	return EFalse;
	}


//------------------------------------------------------------------------------
// CAspListItemData::ListItemTextL
//
// Constructs header and value suitable for listbox.
// List item syntax:
// "\t<header>\t\t<value>\t<mandatorychar>\t\t<readonlyicon>"
//------------------------------------------------------------------------------
//
HBufC* CAspListItemData::ListItemTextL(TBool aConvert)
	{
	_LIT(KTab, "\t");
	_LIT(KMandatoryChar, "*");
	_LIT(KReadOnlyIconIndex, "0");
	
	HBufC* hBuf = HBufC::NewL(KBufSize255);
	TPtr ptr = hBuf->Des();
	
	ptr.Append(KTab);
	ptr.Append(iHeader->Des());
	ptr.Append(KTab);
	ptr.Append(KTab);
	                                              //"PC Suite" string in Host address field and server id should be displayed as chinese
	if (IsEmpty() || iItemType == ETypeSecret || (TDialogUtil::IsPCSuiteDesC(iValue->Des()) && (iItemId == EAspHostAddress || iItemId == EAspServerId)))
		{
		ptr.Append(iDisplayValue->Des());
		}
	else
		{
		ptr.Append(iValue->Des());
		}

	ptr.Append(KTab);
	
	if (iMandatory)
		{
		ptr.Append(KMandatoryChar);
		}
	
	ptr.Append(KTab);
	ptr.Append(KTab);
	
	if (iHidden == EVisibilityReadOnly)
		{
		ptr.Append(KReadOnlyIconIndex);
		}

	if (aConvert)
		{
		AknTextUtils::DisplayTextLanguageSpecificNumberConversion(ptr);
		}

	return hBuf;
	}


//------------------------------------------------------------------------------
// CAspListItemData::Init
// 
//------------------------------------------------------------------------------
//
void CAspListItemData::Init()
	{
	iNumberData = KErrNotFound;
	iDate = 0;
	iTime = 0;
	
	iItemType = KErrNotFound;
	iHidden = EVisibilityNormal;
	iMaxLength = KBufSize;
	iResource = KErrNotFound;
	iIndex = KErrNotFound;
	iItemId = KErrNotFound;
	
	iMandatory = EFalse;
	iLatinInput = EFalse;
	
	iMinValue = KErrNotFound;
	iMaxValue = KErrNotFound;
	
	//iFilter = 0;
	iSelectKeyPressed = EFalse;
	
	iValue = NULL;
	iDisplayValue = NULL;
	iHeader = NULL;
    }


		
/*******************************************************************************
 * class CAspRadioButtonSettingPageEditor
 *******************************************************************************/


//------------------------------------------------------------------------------
// CAspRadioButtonSettingPageEditor::CAspRadioButtonSettingPage
// 
//------------------------------------------------------------------------------
//
CAspRadioButtonSettingPageEditor::CAspRadioButtonSettingPageEditor(TInt aResourceID,
                            TInt& aCurrentSelectionIndex,
                            const MDesCArray* aItemArray
                            )
                            
 : CAknRadioButtonSettingPage(aResourceID, aCurrentSelectionIndex, aItemArray)
	{
	}
	
	
// ----------------------------------------------------------------------------
// CAspRadioButtonSettingPageEditor::OfferKeyEventL
//
// ----------------------------------------------------------------------------
//
TKeyResponse CAspRadioButtonSettingPageEditor::OfferKeyEventL(const TKeyEvent& aKeyEvent, 
                                                 TEventCode aType)
    {
		    if(EKeyEnter == aKeyEvent.iCode)
		    {
		    	ProcessCommandL(EAknSoftkeyOk);
		    	return EKeyWasConsumed;
		    }
		    return CAknRadioButtonSettingPage::OfferKeyEventL(aKeyEvent, aType);
    }

//------------------------------------------------------------------------------
// Destructor
// 
//------------------------------------------------------------------------------
//
CAspRadioButtonSettingPageEditor::~CAspRadioButtonSettingPageEditor()
	{
	}

		
/*******************************************************************************
 * class CAspTextSettingPageEditor
 *******************************************************************************/


//------------------------------------------------------------------------------
// CAspTextSettingPageEditor::CAspTextSettingPageEditor
// 
//------------------------------------------------------------------------------
//
CAspTextSettingPageEditor::CAspTextSettingPageEditor(TInt aResourceID, 
                                                     TDes& aText, 
                                                     TInt aTextSettingPageFlags)                      
 : CAknTextSettingPage(aResourceID, aText, aTextSettingPageFlags)
	{
	}
	
	
// ----------------------------------------------------------------------------
// CAspTextSettingPageEditor::OfferKeyEventL
//
// ----------------------------------------------------------------------------
//
TKeyResponse CAspTextSettingPageEditor::OfferKeyEventL(const TKeyEvent& aKeyEvent, 
                                                 TEventCode aType)
    {
		    if(EKeyEnter == aKeyEvent.iCode)
		    {
		    	ProcessCommandL(EAknSoftkeyOk);
		    	return EKeyWasConsumed;
		    }
		    return CAknTextSettingPage::OfferKeyEventL(aKeyEvent, aType);
    }

//------------------------------------------------------------------------------
// Destructor
// 
//------------------------------------------------------------------------------
//
CAspTextSettingPageEditor::~CAspTextSettingPageEditor()
	{
	}

		
/*******************************************************************************
 * class CAspAlphaPasswordSettingPageEditor
 *******************************************************************************/


//------------------------------------------------------------------------------
// CAspAlphaPasswordSettingPageEditor::CAspAlphaPasswordSettingPageEditor
// 
//------------------------------------------------------------------------------
//
CAspAlphaPasswordSettingPageEditor::CAspAlphaPasswordSettingPageEditor(TInt aResourceID, 
                                                     TDes& aNewPassword, 
                                                     const TDesC& aOldPassword)                      
 : CAknAlphaPasswordSettingPage(aResourceID,aNewPassword,aOldPassword)
	{
	}
	
	
// ----------------------------------------------------------------------------
// CAspAlphaPasswordSettingPageEditor::OfferKeyEventL
//
// ----------------------------------------------------------------------------
//
TKeyResponse CAspAlphaPasswordSettingPageEditor::OfferKeyEventL(const TKeyEvent& aKeyEvent, 
                                                 TEventCode aType)
    {
		    if(EKeyEnter == aKeyEvent.iCode)
		    {
		    	ProcessCommandL(EAknSoftkeyOk);
		    	return EKeyWasConsumed;
		    }
		    return CAknAlphaPasswordSettingPage::OfferKeyEventL(aKeyEvent, aType);
    }

//------------------------------------------------------------------------------
// Destructor
// 
//------------------------------------------------------------------------------
//
CAspAlphaPasswordSettingPageEditor::~CAspAlphaPasswordSettingPageEditor()
	{
	}

/*******************************************************************************
 * class CAspIntegerSettingPageEditor
 *******************************************************************************/


//------------------------------------------------------------------------------
// CAspIntegerSettingPageEditor::CAspIntegerSettingPageEditor
// 
//------------------------------------------------------------------------------
//
CAspIntegerSettingPageEditor::CAspIntegerSettingPageEditor(TInt aResourceID, 
												TInt& aValue, 
												TInt aTextSettingPageFlags)
 : CAknIntegerSettingPage(aResourceID,aValue,aTextSettingPageFlags)
	{
	}
	
	
// ----------------------------------------------------------------------------
// CAspIntegerSettingPageEditor::OfferKeyEventL
//
// ----------------------------------------------------------------------------
//
TKeyResponse CAspIntegerSettingPageEditor::OfferKeyEventL(const TKeyEvent& aKeyEvent, 
                                                 TEventCode aType)
    {
		    if(EKeyEnter == aKeyEvent.iCode)
		    {
		    	ProcessCommandL(EAknSoftkeyOk);
		    	return EKeyWasConsumed;
		    }
		    return CAknIntegerSettingPage::OfferKeyEventL(aKeyEvent, aType);
    }

//------------------------------------------------------------------------------
// Destructor
// 
//------------------------------------------------------------------------------
//
CAspIntegerSettingPageEditor::~CAspIntegerSettingPageEditor()
	{
	}


//  End of File  
