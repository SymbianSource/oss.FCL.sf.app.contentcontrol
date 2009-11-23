/*
* Copyright (c) 2004 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:  Sources
*
*/



// INCLUDE FILES
#include <s32buf.h>
#include <s32file.h>

#include "nsmlxmlparser.h"


// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CNSmlEmailParser::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
EXPORT_C CNSmlEmailParser* CNSmlEmailParser::NewL()
	{
	CNSmlEmailParser* self = CNSmlEmailParser::NewLC();
	CleanupStack::Pop();

	return self;
	}

// -----------------------------------------------------------------------------
// CNSmlEmailParser::NewLC
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
EXPORT_C CNSmlEmailParser* CNSmlEmailParser::NewLC()
	{
	CNSmlEmailParser* self = new (ELeave) CNSmlEmailParser();
	CleanupStack::PushL(self);
	self->ConstructL();

	return self;
	}

// -----------------------------------------------------------------------------
// CNSmlEmailParser::~CNSmlEmailParser
// Destructor
// -----------------------------------------------------------------------------
//
EXPORT_C CNSmlEmailParser::~CNSmlEmailParser()
	{
	if( iTruncated )
		delete iTruncated;

	if( iExt ) 
		{
		iExt->ResetAndDestroy();
		delete iExt;
		}

	if( iEmailItem ) delete iEmailItem;

	if( iTruncType ) delete iTruncType;
	if( iTruncName ) delete iTruncName;
	if( iExtData ) delete iExtData;

	}

// -----------------------------------------------------------------------------
// CNSmlEmailParser::ParseXml
// Parses through the given XML and puts the data it contains 
// to member variables. 
// Note: The string-values in CNSmlEmailParser are NOT copies, but 
// references to aXml. The method does not check, if member variables 
// already contain data, but just writes it (if some of them are already 
// instantiated dynamically, they should be deleted before this method is called).
// -----------------------------------------------------------------------------
//
EXPORT_C TNSmlParserGeneratorError CNSmlEmailParser::ParseXml( HBufC8* aXml )
	{

	// pre-process the data (remove cdata over <Data>)
	TRAPD( cdataErr, PreProcessL(aXml));
	if (cdataErr != EErrorNone)
		return (TNSmlParserGeneratorError)cdataErr;

	// reset state
	iSetValues.Reset();
	iCurrentState = ENone;
	iLastState = ENone;
	TPtrC8 ptr(*aXml);

	// parse
	TRAPD(err, ParseL( ptr ));

	// return error, if in wrong state
	if( err == EErrorNone && iCurrentState != ENone )
		return EInvalidXmlError;

	return CheckError(err);
	}

// -----------------------------------------------------------------------------
// CNSmlEmailParser::GenerateXml
// Generates Email-XML using the data in member variables. aXml contains the 
// generated xml when the function returns successfully. Note: caller should not 
// instantiate the HBufC8, since this method first counts the size of the xml to
// be generated and then instatiates the HBufC8 with a correct maximum size. If
// the given buffer is not NULL, it is destroyed and a new one is instantiated.
// However, the caller gets the control of the HBufC8 when the function returns.
// -----------------------------------------------------------------------------
//
EXPORT_C TNSmlParserGeneratorError CNSmlEmailParser::GenerateXml( HBufC8*& aXml ) 
	{

	TRAPD( err, GenerateEmailXmlL( aXml ) );

	return CheckError(err);
	}

// -----------------------------------------------------------------------------
// CNSmlEmailParser::GenerateEmailXmlL
// Private method that does the actual email xml generation
// -----------------------------------------------------------------------------
//
void CNSmlEmailParser::GenerateEmailXmlL( HBufC8*& aXml )
	{
	ConvertIntoEntitiesL();

	// count the size of the xml
	TInt size = CountXmlSizeL();

	// create buffer for the xml

	if ( aXml )
		{
		delete aXml;
		aXml = NULL;
		}

	aXml = HBufC8::NewLC(size);
	TPtr8 ptr = aXml->Des();

	// append data to buffer
	AppendElement(ptr, KEmailElement());

	if ( iRead )
		{
		AppendElement(ptr, KEmailReadElement(), BooleanToString( iRead ));
		}

	if ( iForwarded )
		{
		AppendElement(ptr, KEmailForwardedElement(), BooleanToString( iForwarded ));
		}

	if ( iReplied )
		{
		AppendElement(ptr, KEmailRepliedElement(), BooleanToString( iReplied ));
		}

	if (iReceived != Time::NullTTime())
		{
		AppendElement(ptr, KEmailReceivedElement(), DateTimeToStringL(iReceived));
		}

	if (iCreated != Time::NullTTime())
		{
		AppendElement(ptr, KEmailCreatedElement(), DateTimeToStringL(iCreated));
		}

	if (iModified != Time::NullTTime())
		{
		AppendElement(ptr, KEmailModifiedElement(), DateTimeToStringL(iModified));
		}

	if ( iDeleted )
		{
		AppendElement(ptr, KEmailDeletedElement(), BooleanToString( iDeleted ));
		}

	if ( iFlagged )
		{
		AppendElement(ptr, KEmailFlaggedElement(), BooleanToString( iFlagged ));
		}

	if ( iEmailItem )
		{
		AppendElement(ptr, KEmailItemElement());
		ptr.Append(KCDataStart);
		ptr.Append( *iEmailItem );
		ptr.Append(KCDataEnd);
		AppendEndElement(ptr, KEmailItemElement());
		}

	if ( iTruncated )
		{
		iTruncated->GenerateXmlL(ptr, this);
		}

	if ( iExt )
		{
		for (TInt i=0; i < iExt->Count(); ++i)
			{
			iExt->At(i)->GenerateXmlL(ptr, this);
			}

		}


	AppendEndElement(ptr, KEmailElement());

	CleanupStack::Pop(); // aXml

	}

// -----------------------------------------------------------------------------
// CNSmlEmailParser::CountXmlSizeL
// Counts how long the xml string will be if it was generated using 
// the values currently in member variables.
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CNSmlEmailParser::CountXmlSizeL()
	{
	TInt size = 0;

	size += KCDataStart().Length() + KCDataEnd().Length();
	size += SizeOfElements( KEmailElement() );

	if ( iRead )
		{
		size += SizeOfBoolean( iRead, KEmailReadElement() );
		}

	if ( iForwarded )
		{
		size += SizeOfBoolean( iForwarded, KEmailForwardedElement() );
		}

	if ( iReplied )
		{
		size += SizeOfBoolean( iReplied, KEmailRepliedElement() );
		}

	if (iReceived != Time::NullTTime())
		{
		size += SizeOfDatetime( KEmailReceivedElement() );
		}

	if (iCreated != Time::NullTTime())
		{
		size += SizeOfDatetime( KEmailCreatedElement() );
		}

	if (iModified != Time::NullTTime())
		{
		size += SizeOfDatetime( KEmailModifiedElement() );
		}

	if ( iDeleted )
		{
		size += SizeOfBoolean( iDeleted, KEmailDeletedElement() );
		}

	if ( iFlagged )
		{
		size += SizeOfBoolean( iFlagged, KEmailFlaggedElement() );
		}

	if ( iEmailItem )
		{
		size += SizeOfString( iEmailItem, KEmailItemElement() );
		size += KCDataStart().Length() + KCDataInnerEnd().Length();
		}

	if ( iTruncated )
		{
		size += iTruncated->CountXmlSizeL( this );
		}

	if ( iExt )
		{
		for (TInt i=0; i < iExt->Count(); ++i)
			{
			size += iExt->At(i)->CountXmlSize( this );
			}
		}

	return size;
	}

// -----------------------------------------------------------------------------
// CNSmlEmailParser::AddExtL
// Adds the given extension to iExt
// -----------------------------------------------------------------------------
//
EXPORT_C void CNSmlEmailParser::AddExtL( CNSmlExtData* aExt )
	{
	iExt->AppendL(aExt);
	}

// -----------------------------------------------------------------------------
// CNSmlEmailParser::NextElementL
// Base class' ParseL method calls this method, when a new element is found. aElement 
// is the name of that element (string inside '<' and '>' characters).
// -----------------------------------------------------------------------------
//
void CNSmlEmailParser::NextElementL( TPtrC8 aElement )
	{
	if (aElement.Length() == 0)
		User::Leave(EInvalidXmlError);

	TBool startElement = ETrue;

	if(aElement[0] == KCharacterSlash)
		{
		// take off the '/' character from the beginning
		aElement.Set(aElement.Right(aElement.Length()-1));
		// end element
		startElement = EFalse;
		}
#ifndef __NO_EMPTY_ELEMENTS_
	else if (aElement[aElement.Length()-1] == KCharacterSlash)
		{
		// take off the '/' character from the end
		aElement.Set(aElement.Left(aElement.Length()-1));

		// empty element == start element, empty data, end element
		NextElementL( aElement ); // simulates start element
		NextDataL( _L8("") ); // simulates empty data

		// create the end element
		HBufC8* end = HBufC8::NewLC(aElement.Length()+1);
		TPtr8 ptr = end->Des();
		ptr.Append( _L8("/") );
		ptr.Append( aElement );

		NextElementL( ptr ); // simulates end element

		CleanupStack::PopAndDestroy(); // end

		return;
		}
#endif

	// variables used for state, init to <Email> and not set
	TNSmlCurrentEmailElement inner = EEmail;
	TNSmlCurrentEmailElement outer = EEmail;
	TBool isSet = EFalse;

	// check which element we've got
	if( aElement == KEmailElement )
		{
		outer = ENone;
		isSet = iSetValues.iEmail;
		if ( startElement ) iSetValues.iEmail = ETrue;
		}
	else if( aElement == KEmailReadElement )
		{
		inner = ERead;
		isSet = iSetValues.iRead;
		}
	else if( aElement == KEmailForwardedElement )
		{
		inner = EForwarded;
		isSet = iSetValues.iForwarded;
		}
	else if( aElement == KEmailRepliedElement )
		{
		inner = EReplied;
		isSet = iSetValues.iReplied;
		}
	else if( aElement == KEmailReceivedElement )
		{
		inner = EReceived;
		isSet = iSetValues.iReceived;
		}
	else if( aElement == KEmailCreatedElement )
		{
		inner = ECreated;
		isSet = iSetValues.iCreated;
		}
	else if( aElement == KEmailModifiedElement )
		{
		inner = EModified;
		isSet = iSetValues.iModified;
		}
	else if( aElement == KEmailDeletedElement )
		{
		inner = EDeleted;
		isSet = iSetValues.iDeleted;
		}
	else if( aElement == KEmailFlaggedElement )
		{
		inner = EFlagged;
		isSet = iSetValues.iFlagged;
		}
	else if( aElement == KEmailItemElement )
		{
		inner = EEmailItem;
		isSet = iSetValues.iEmailItem;
		if ( startElement ) 
			HandleNextEmailItemStartL();
		else
			HandleNextEmailItemEndL();
		}
	else if( aElement == KTruncatedElement )
		{
		inner = ETrunc;
		isSet = iSetValues.iTrunc;

		if ( startElement )
			{
			if ( !iTruncated )
				iTruncated = CNSmlTruncatedData::NewL();
			iSetValues.iTrunc = ETrue;
			}
		}
	else if( aElement == KTruncatedBodyElement )
		{
		inner = ETruncBody;
		outer = ETrunc;
		if ( startElement )
			{
			if( iTruncType )
				{
				delete iTruncType;
				iTruncType = NULL;
				}
			iTruncSize = KErrNotFound;

			iSetValues.iTruncBodySize = EFalse;
			iSetValues.iTruncBodyType = EFalse;
			}
		else
			{
			iTruncated->AddBodyL(iTruncSize, iTruncType);
			}
		}
	else if( aElement == KTruncatedAttachElement )
		{
		inner = ETruncAttach;
		outer = ETrunc;
		if ( startElement )
			{
			if( iTruncType )
				{
				delete iTruncType;
				iTruncType = NULL;
				}
			if( iTruncName )
				{
				delete iTruncName;
				iTruncName = NULL;
				}
			iTruncSize = KErrNotFound;

			iSetValues.iTruncAttachName = EFalse;
			iSetValues.iTruncAttachType = EFalse;
			iSetValues.iTruncAttachSize = EFalse;
			}
		else
			{
			iTruncated->AddAttachmentL(iTruncName, iTruncSize, iTruncType);
			}
		}
	else if( aElement == KTruncatedSizeElement )
		{
		if ( (startElement && iCurrentState == ETruncBody) 
				|| (!startElement && iCurrentState == ETruncBodySize) )
			{
			inner = ETruncBodySize;
			outer = ETruncBody;
			isSet = iSetValues.iTruncBodySize;
			}
		else if ( (startElement && iCurrentState == ETruncAttach) 
				|| (!startElement && iCurrentState == ETruncAttachSize) )
			{
			inner = ETruncAttachSize;
			outer = ETruncAttach;
			isSet = iSetValues.iTruncAttachSize;
			}
		else
			{
			User::Leave(EInvalidXmlError); // wrong state
			}
		}
	else if( aElement == KTruncatedTypeElement )
		{
		if ( (startElement && iCurrentState == ETruncBody) 
			|| (!startElement && iCurrentState == ETruncBodyType) )
			{
			inner = ETruncBodyType;
			outer = ETruncBody;
			isSet = iSetValues.iTruncBodyType;
			}
		else if ( (startElement && iCurrentState == ETruncAttach)
				|| (!startElement && iCurrentState == ETruncAttachType) )
			{
			inner = ETruncAttachType;
			outer = ETruncAttach;
			isSet = iSetValues.iTruncAttachType;
			}
		else
			{
			User::Leave(EInvalidXmlError); // wrong state
			}
		}
	else if( aElement == KTruncatedNameElement )
		{
		inner = ETruncAttachName;
		outer = ETruncAttach;
		isSet = iSetValues.iTruncAttachName;
		}
	else if( aElement == KExtElement )
		{
		inner = EExt;
		if ( startElement )
			{
			if( iExtData )
				{
				delete iExtData;
				iExtData = NULL;
				}
			iExtData = CNSmlExtData::NewL();
			iSetValues.iXNam = EFalse;
			}
		else
			{
			if( !iSetValues.iXNam ) User::Leave(EMandatoryFieldNotFound); // xnam not set
			iExt->AppendL(iExtData);
			iExtData = NULL;
			}
		}
	else if( aElement == KExtXNamElement )
		{
		inner = EExtXNam;
		outer = EExt;
		isSet = iSetValues.iXNam;
		}
	else if( aElement == KExtXValElement )
		{
		inner = EExtXVal;
		outer = EExt;
		}
	else
		{ // unknown element
		User::Leave(EInvalidXmlError);
		}


	// finally, change state depending on was the element start or end element
	if ( startElement )
		{
		StartElementStateChangeL( outer, inner, isSet );
		}
	else
		{
		EndElementStateChangeL( inner, outer );
		}


	}

// -----------------------------------------------------------------------------
// CNSmlEmailParser::NextDataL
// Base class' ParseL method calls this method, when a data is read from xml, but that 
// data is not element name (data inside or between elements).
// -----------------------------------------------------------------------------
//
void CNSmlEmailParser::NextDataL( TPtrC8 aData )
	{
	switch(iCurrentState)
		{
		case ENone:
		case EEmail:
		case ETrunc:
		case ETruncBody:
		case ETruncAttach:
		case EExt:
			LeaveIfNotWhiteSpaceL( aData );
			break;
		case ERead:
			iRead = StringToBooleanL(aData);
			iSetValues.iRead = ETrue;
			break;
		case EForwarded:
			iForwarded = StringToBooleanL(aData);
			iSetValues.iForwarded = ETrue;
			break;
		case EReplied:
			iReplied = StringToBooleanL(aData);
			iSetValues.iReplied = ETrue;
			break;
		case EDeleted:
			iDeleted = StringToBooleanL(aData);
			iSetValues.iDeleted = ETrue;
			break;
		case EFlagged:
			iFlagged = StringToBooleanL(aData);
			iSetValues.iFlagged = ETrue;
			break;
		case EReceived:
			iReceived = StringToTTimeL(aData);
			iSetValues.iReceived = ETrue;
			break;
		case ECreated:
			iCreated = StringToTTimeL(aData);
			iSetValues.iCreated = ETrue;
			break;
		case EModified:
			iModified = StringToTTimeL(aData);
			iSetValues.iModified = ETrue;
			break;
		case EEmailItem:
			HandleNextEmailItemDataL( aData ); // implementation dependent behavior
			iSetValues.iEmailItem = ETrue;
			break;
		case ETruncBodySize:
			iTruncSize = StringToIntegerL(aData);
			iSetValues.iTruncBodySize = ETrue;
			break;
		case ETruncBodyType:
			iTruncType = aData.AllocL();
			iSetValues.iTruncBodyType = ETrue;
			break;
		case ETruncAttachSize:
			iTruncSize = StringToIntegerL(aData);
			iSetValues.iTruncAttachSize = ETrue;
			break;
		case ETruncAttachName:
			iTruncName = aData.AllocL();
			iSetValues.iTruncAttachName = ETrue;
			break;
		case ETruncAttachType:
			iTruncType = aData.AllocL();
			iSetValues.iTruncAttachType = ETrue;
			break;
		case EExtXNam:
			iExtData->iXNam = aData.AllocL();
			iSetValues.iXNam = ETrue;
			break;
		case EExtXVal:
			iExtData->AddXValL(aData.AllocL());
			break;
		default:
			User::Leave(EUnknownError); // should never happen
			break;
		}
	}

// -----------------------------------------------------------------------------
// CNSmlEmailParser::HandleNextEmailItemDataL
// Next emailitem data
// -----------------------------------------------------------------------------
//
void CNSmlEmailParser::HandleNextEmailItemDataL( TPtrC8 aData )
	{
	iEmailItem = aData.AllocL();
	}

// -----------------------------------------------------------------------------
// CNSmlEmailParser::StartElementStateChangeL
// Checks that the state change (new start element) is legal (right order of 
// elements and element not already set) and changes the state.
// -----------------------------------------------------------------------------
//
void CNSmlEmailParser::StartElementStateChangeL(TNSmlCurrentEmailElement aCurrentState, 
											    TNSmlCurrentEmailElement aNextState, 
											    TBool aIsSet)
	{
	if( aIsSet || iCurrentState != aCurrentState || aNextState < iLastState )
		{
		User::Leave(EInvalidXmlError);
		}

	iLastState = iCurrentState;
	iCurrentState = aNextState;
	}

// -----------------------------------------------------------------------------
// CNSmlEmailParser::EndElementStateChangeL
// Checks that the state change (new end element) is legal (right order of 
// elements and element not already set) and changes the state.
// -----------------------------------------------------------------------------
//
void CNSmlEmailParser::EndElementStateChangeL(TNSmlCurrentEmailElement aCurrentState, 
											  TNSmlCurrentEmailElement aNextState )
	{
	if( iCurrentState != aCurrentState )
		{
		User::Leave(EInvalidXmlError);
		}

	iLastState = iCurrentState;
	iCurrentState = aNextState;
	}

// -----------------------------------------------------------------------------
// CNSmlEmailParser::ConvertIntoEntitiesL
// Converts special characters of this email-dataobject to corresponding 
// characters. Emailitem needs not to be changed, since it is always 
// placed inside cdata (when generating xml).
// -----------------------------------------------------------------------------
//
void CNSmlEmailParser::ConvertIntoEntitiesL()
	{

	// truncate info
	if ( iTruncated )
		{
		iTruncated->ConvertIntoEntitiesL(this);
		}

	// extensions
	if ( iExt )
		{
		for (TInt i=0; i < iExt->Count(); ++i)
			{
			iExt->At(i)->ConvertIntoEntitiesL(this);
			}
		}

	}

// -----------------------------------------------------------------------------
// CNSmlEmailParser::CNSmlEmailParser
// Private constructor
// -----------------------------------------------------------------------------
//
CNSmlEmailParser::CNSmlEmailParser()
	: iReceived(Time::NullTTime()), iCreated(Time::NullTTime()), iModified(Time::NullTTime()),
	iTruncSize(KErrNotFound)
	{
	}

// -----------------------------------------------------------------------------
// CNSmlEmailParser::ConstructL
// Second phase construction
// -----------------------------------------------------------------------------
//
void CNSmlEmailParser::ConstructL()
	{
	iExt = new (ELeave) CNSmlExtDataArray(3);
	}

// -----------------------------------------------------------------------------
// TNSmlSetEmailValues::TNSmlSetEmailValues
// -----------------------------------------------------------------------------
//
CNSmlEmailParser::TNSmlSetEmailValues::TNSmlSetEmailValues()
	{
	Reset();
	}


// -----------------------------------------------------------------------------
// TNSmlSetEmailValues::Reset
// -----------------------------------------------------------------------------
//
void CNSmlEmailParser::TNSmlSetEmailValues::Reset()
	{
	iEmail = EFalse;
	iRead = EFalse;
	iForwarded = EFalse;
	iReplied = EFalse;
	iDeleted = EFalse;
	iFlagged = EFalse;
	iReceived = EFalse;
	iCreated = EFalse;
	iModified = EFalse;
	iEmailItem = EFalse;
	iTrunc = EFalse;
	iTruncBodySize = EFalse;
	iTruncBodyType = EFalse;
	iXNam = EFalse;
	iTruncAttachSize = EFalse;
	iTruncAttachType = EFalse;
	iTruncAttachName = EFalse;
	}

//  End of File
