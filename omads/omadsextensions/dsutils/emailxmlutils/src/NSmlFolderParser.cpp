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
// CNSmlFolderParser::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
EXPORT_C CNSmlFolderParser* CNSmlFolderParser::NewL()
	{
	CNSmlFolderParser* self = CNSmlFolderParser::NewLC();
	CleanupStack::Pop();

	return self;
	}

// -----------------------------------------------------------------------------
// CNSmlFolderParser::NewLC
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
EXPORT_C CNSmlFolderParser* CNSmlFolderParser::NewLC()
	{
	CNSmlFolderParser* self = new (ELeave) CNSmlFolderParser();
	CleanupStack::PushL(self);
	self->ConstructL();

	return self;
	}

// -----------------------------------------------------------------------------
// CNSmlFolderParser::~CNSmlFolderParser
// Destructor
// -----------------------------------------------------------------------------
//
EXPORT_C CNSmlFolderParser::~CNSmlFolderParser()
	{
	if(iExt) 
		{
		iExt->ResetAndDestroy();
		delete iExt;
		}

	if(iName) delete iName;
	if(iRole) delete iRole;

	if( iExtData ) delete iExtData;
	}

// -----------------------------------------------------------------------------
// CNSmlFolderParser::ParseXml
// Parses through the given XML and places the data to member variables.
// Note: The string-values in CNSmlEmailParser are NOT copies, but 
// references to aXml. The method does not check whether the member variables 
// already contains data or not, but just writes it (if some of them are already 
// instantiated dynamically, they should be deleted before this method is called).
// -----------------------------------------------------------------------------
//
EXPORT_C TNSmlParserGeneratorError CNSmlFolderParser::ParseXml( HBufC8* aXml )
	{

	TRAPD(cdataErr, PreProcessL(aXml));
	if (cdataErr != EErrorNone)
		return (TNSmlParserGeneratorError)cdataErr;

	iSetValues.Reset();
	iCurrentState = EFolderNone;
	iLastState = EFolderNone;
	TPtrC8 ptr(*aXml);

	TRAPD(err, ParseL( ptr ));

	if( err == EErrorNone && iCurrentState != EFolderNone )
		return EInvalidXmlError;

	return CheckError(err);
	}

// -----------------------------------------------------------------------------
// CNSmlFolderParser::GenerateXml
// Generates Folder-XML using the values in member variables. aXml contains the 
// generated xml when the function returns successfully. Note: caller should not 
// instantiate the HBufC8, since this method first counts the size of the 
// xml to be generated and then instatiates the HBufC8 with a correct maximum size.
// If the given buffer is not NULL, it is destroyed and a new one is instantiated.
// Caller gets the control of the HBufC8 when the function returns.
// -----------------------------------------------------------------------------
//
EXPORT_C TNSmlParserGeneratorError CNSmlFolderParser::GenerateXml( HBufC8*& aXml ) 
	{

	TRAPD( err, ConvertIntoEntitiesL(); // convert special characters into entities
	
			GenerateFolderXmlL( aXml ) );

	return CheckError(err);
	}

// -----------------------------------------------------------------------------
// CNSmlFolderParser::GenerateFolderXmlL
// Private methods that does the actual folder xml generation
// -----------------------------------------------------------------------------
//
void CNSmlFolderParser::GenerateFolderXmlL( HBufC8*& aXml )
	{
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

	AppendElement(ptr, KFolderElement());

	if ( iName )
		{
		AppendElement(ptr, KFolderNameElement(), *iName);
		}

	if (iCreated != Time::NullTTime())
		{
		AppendElement(ptr, KFolderCreatedElement(), DateTimeToStringL(iCreated));
		}

	if (iModified != Time::NullTTime())
		{
		AppendElement(ptr, KFolderModifiedElement(), DateTimeToStringL(iModified));
		}

	if (iAccessed != Time::NullTTime())
		{
		AppendElement(ptr, KFolderAccessedElement(), DateTimeToStringL(iAccessed));
		}

	if (iAttributes.AttributeCount() > 0)
		{
		iAttributes.GenerateXml(ptr, this);
		}

	if ( iRole )
		{
		AppendElement(ptr, KFolderRoleElement(), *iRole);
		}

	if ( iExt )
		{
		for (TInt i=0; i < iExt->Count(); ++i)
			{
			iExt->At(i)->GenerateXmlL(ptr, this);
			}

		}

	AppendEndElement(ptr, KFolderElement());

	CleanupStack::Pop(); // aXml
	}

// -----------------------------------------------------------------------------
// CNSmlFolderParser::CountXmlSizeL
// Counts how long the xml string will be if it was generated using 
// the values currently in member variables.
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CNSmlFolderParser::CountXmlSizeL()
	{
	TInt size = 0;

	size += SizeOfElements( KFolderElement() );
	size += KCDataStart().Length() + KCDataEnd().Length();

	if ( iName )
		{
		size += SizeOfString( iName, KFolderNameElement() );
		}

	if (iCreated != Time::NullTTime())
		{
		size += SizeOfDatetime( KFolderCreatedElement() );
		}

	if (iModified != Time::NullTTime())
		{
		size += SizeOfDatetime( KFolderModifiedElement() );
		}

	if (iAccessed != Time::NullTTime())
		{
		size += SizeOfDatetime( KFolderAccessedElement() );
		}

	// attributes, if any present
	if (iAttributes.AttributeCount() > 0)
		{
		size += iAttributes.CountXmlSize( this );
		}

	if ( iRole )
		{
		size += SizeOfString( iRole, KFolderRoleElement() );
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
// CNSmlFolderParser::AddExtL
// Adds the given extension to iExt
// -----------------------------------------------------------------------------
//
EXPORT_C void CNSmlFolderParser::AddExtL( CNSmlExtData* aExt )
	{
	iExt->AppendL(aExt);
	}

// -----------------------------------------------------------------------------
// CNSmlFolderParser::NextElementL
// Base class' ParseL method calls this method, when a new element is found. aElement 
// is the name of that element (string inside '<' and '>' characters).
// -----------------------------------------------------------------------------
//
void CNSmlFolderParser::NextElementL( TPtrC8 aElement )
	{
	if (aElement.Length() == 0)
		User::Leave(EInvalidXmlError);

	TBool startElement = ETrue;

	if(aElement[0] == KCharacterSlash)
		{
		// end element
		// remove the '/' character from the beginning
		aElement.Set( aElement.Right( aElement.Length() - 1 ) );
		startElement = EFalse;
		}
#ifndef __NO_EMPTY_ELEMENTS_
	else if (aElement[aElement.Length()-1] == KCharacterSlash)
		{
		// empty element
		// remove the '/' character from the end
		aElement.Set( aElement.Left( aElement.Length() - 1 ) );

		// empty element == start element, empty data, end element
		NextElementL( aElement ); // simulates start element
		NextDataL( _L8("") ); // simulates the empty data

		HBufC8* end = HBufC8::NewLC(aElement.Length()+1);
		TPtr8 ptr = end->Des();
		ptr.Append( _L8("/") );
		ptr.Append( aElement );

		NextElementL( ptr ); // simulates end element

		CleanupStack::PopAndDestroy(); // end

		return;
		}
#endif

	// variables used for state, init to <Folder> and not set
	TNSmlCurrentFolderElement inner = EFolder;
	TNSmlCurrentFolderElement outer = EFolder;
	TBool isSet = EFalse;

	if( aElement == KFolderElement )
		{
		outer = EFolderNone;
		isSet = iSetValues.iFolder;
		if ( startElement ) iSetValues.iFolder = ETrue;
		}
	else if( aElement == KFolderNameElement )
		{
		inner = EFolderName;
		isSet = iSetValues.iName;
		}
	else if( aElement == KFolderCreatedElement )
		{
		inner = EFolderCreated;
		isSet = iSetValues.iCreated;
		}
	else if( aElement == KFolderModifiedElement )
		{
		inner = EFolderModified;
		isSet = iSetValues.iModified;
		}
	else if( aElement == KFolderAccessedElement )
		{
		inner = EFolderAccessed;
		isSet = iSetValues.iAccessed;
		}
	else if( aElement == KFolderAttributesElement )
		{
		inner = EAttributes;
		isSet = iSetValues.iAttributes;
		if ( startElement) iSetValues.iAttributes = ETrue;
		}
	else if( aElement == KAttributeHiddenElement )
		{
		inner = EAttributesH;
		outer = EAttributes;
		isSet = iSetValues.iAttributesH;
		}
	else if( aElement == KAttributeSystemElement )
		{
		inner = EAttributesS;
		outer = EAttributes;
		isSet = iSetValues.iAttributesS;
		}
	else if( aElement == KAttributeArchivedElement )
		{
		inner = EAttributesA;
		outer = EAttributes;
		isSet = iSetValues.iAttributesA;
		}
	else if( aElement == KAttributeDeleteElement )
		{
		inner = EAttributesD;
		outer = EAttributes;
		isSet = iSetValues.iAttributesD;
		}
	else if( aElement == KAttributeWritableElement )
		{
		inner = EAttributesW;
		outer = EAttributes;
		isSet = iSetValues.iAttributesW;
		}
	else if( aElement == KAttributeReadableElement )
		{
		inner = EAttributesR;
		outer = EAttributes;
		isSet = iSetValues.iAttributesR;
		}
	else if( aElement == KAttributeExecutableElement )
		{
		inner = EAttributesX;
		outer = EAttributes;
		isSet = iSetValues.iAttributesX;
		}
	else if( aElement == KFolderRoleElement )
		{
		inner = EFolderRole;
		isSet = iSetValues.iRole;
		}
	else if( aElement == KExtElement )
		{
		inner = EFolderExt;

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
		inner = EFolderExtXNam;
		outer = EFolderExt;
		isSet = iSetValues.iXNam;
		}
	else if( aElement == KExtXValElement )
		{
		inner = EFolderExtXVal;
		outer = EFolderExt;
		}
	else
		{
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
// CNSmlFolderParser::NextDataL
// Base class' ParseL method calls this method, when a data is read from xml,
// but that data is not element name (data inside or between elements).
// -----------------------------------------------------------------------------
//
void CNSmlFolderParser::NextDataL( TPtrC8 aData )
	{
	switch(iCurrentState)
		{
		case EFolderNone:
		case EFolder:
		case EAttributes:
		case EFolderExt:
			LeaveIfNotWhiteSpaceL( aData );
			break;
		case EFolderName:
			iName = aData.AllocL();
			iSetValues.iName = ETrue;
			break;
		case EFolderCreated:
			iCreated = StringToTTimeL(aData);
			iSetValues.iCreated = ETrue;
			break;
		case EFolderModified:
			iModified = StringToTTimeL(aData);
			iSetValues.iModified = ETrue;
			break;
		case EFolderAccessed:
			iAccessed = StringToTTimeL(aData);
			iSetValues.iAccessed = ETrue;
			break;
		case EAttributesH:
			iAttributes.iHidden = StringToBooleanL(aData);
			iSetValues.iAttributesH = ETrue;
			break;
		case EAttributesS:
			iAttributes.iSystem = StringToBooleanL(aData);
			iSetValues.iAttributesS = ETrue;
			break;
		case EAttributesA:
			iAttributes.iArchived = StringToBooleanL(aData);
			iSetValues.iAttributesA = ETrue;
			break;
		case EAttributesD:
			iAttributes.iDelete = StringToBooleanL(aData);
			iSetValues.iAttributesD = ETrue;
			break;
		case EAttributesW:
			iAttributes.iWritable = StringToBooleanL(aData);
			iSetValues.iAttributesW = ETrue;
			break;
		case EAttributesR:
			iAttributes.iReadable = StringToBooleanL(aData);
			iSetValues.iAttributesR = ETrue;
			break;
		case EAttributesX:
			iAttributes.iExecutable = StringToBooleanL(aData);
			iSetValues.iAttributesX = ETrue;
			break;
		case EFolderRole:
			iRole = aData.AllocL();
			iSetValues.iRole = ETrue;
			break;
		case EFolderExtXNam:
			iExtData->iXNam = aData.AllocL();
			iSetValues.iXNam = ETrue;
			break;
		case EFolderExtXVal:
			iExtData->AddXValL(aData.AllocL());
			break;
		default:
			User::Leave(EUnknownError); // should never happen
			break;
		}
	}

// -----------------------------------------------------------------------------
// CNSmlFolderParser::StartElementStateChangeL
// Checks that the state change (new start element) is legal (right order of 
// elements and element not already set) and changes the state.
// -----------------------------------------------------------------------------
//
void CNSmlFolderParser::StartElementStateChangeL(TNSmlCurrentFolderElement aCurrentState, 
											     TNSmlCurrentFolderElement aNextState, 
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
// CNSmlFolderParser::EndElementStateChangeL
// Checks that the state change (new end element) is legal (right order of 
// elements and element not already set) and changes the state.
// -----------------------------------------------------------------------------
//
void CNSmlFolderParser::EndElementStateChangeL(TNSmlCurrentFolderElement aCurrentState, 
											   TNSmlCurrentFolderElement aNextState )
	{
	if( iCurrentState != aCurrentState )
		{
		User::Leave(EInvalidXmlError);
		}

	iLastState = iCurrentState;
	iCurrentState = aNextState;
	}

// -----------------------------------------------------------------------------
// CNSmlFolderParser::ConvertIntoEntitiesL
// Converts special characters of this email-dataobject to corresponding 
// characters. 
// -----------------------------------------------------------------------------
//
void CNSmlFolderParser::ConvertIntoEntitiesL()
	{
	// name
	if ( iName )
		{
		CharactersToEntitiesL(iName, 0, iName->Length());
		}

	// role
	if ( iRole )
		{
		CharactersToEntitiesL(iRole, 0, iRole->Length());
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
// CNSmlFolderParser::CNSmlFolderParser
// Constructor
// -----------------------------------------------------------------------------
//
CNSmlFolderParser::CNSmlFolderParser()
	: iCreated(Time::NullTTime()), iModified(Time::NullTTime()), 
	iAccessed(Time::NullTTime())
	{
	}

// -----------------------------------------------------------------------------
// CNSmlFolderParser::ConstructL
// Second phase construction
// -----------------------------------------------------------------------------
//
void CNSmlFolderParser::ConstructL()
	{
	iExt = new (ELeave) CNSmlExtDataArray(3);
	}

// -----------------------------------------------------------------------------
// CNSmlFolderParser::TNSmlSetFolderValues::TNSmlSetFolderValues
// -----------------------------------------------------------------------------
//
CNSmlFolderParser::TNSmlSetFolderValues::TNSmlSetFolderValues()
	{
	Reset();
	}

// -----------------------------------------------------------------------------
// CNSmlFolderParser::TNSmlSetFolderValues::Reset
// -----------------------------------------------------------------------------
//
void CNSmlFolderParser::TNSmlSetFolderValues::Reset()
	{
	iFolder = EFalse;
	iName = EFalse;
	iCreated = EFalse;
	iModified = EFalse;
	iAccessed = EFalse;
	iAttributes = EFalse;
	iAttributesH = EFalse;
	iAttributesS = EFalse;
	iAttributesA = EFalse;
	iAttributesD = EFalse;
	iAttributesW = EFalse;
	iAttributesR = EFalse;
	iAttributesX = EFalse;
	iXNam = EFalse;
	iRole = EFalse;
	}

//  End of File
