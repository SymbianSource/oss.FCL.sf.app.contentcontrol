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
// CNSmlExtData::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
EXPORT_C CNSmlExtData* CNSmlExtData::NewL()
	{
	CNSmlExtData* self = CNSmlExtData::NewLC();
	CleanupStack::Pop();

	return self;
	}

// -----------------------------------------------------------------------------
// CNSmlExtData::NewLC
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
EXPORT_C CNSmlExtData* CNSmlExtData::NewLC()
	{
	CNSmlExtData* self = new (ELeave) CNSmlExtData();
	CleanupStack::PushL(self);
	self->ConstructL();

	return self;
	}

// -----------------------------------------------------------------------------
// CNSmlExtData::~CNSmlExtData
// Destructor.
// -----------------------------------------------------------------------------
//
EXPORT_C CNSmlExtData::~CNSmlExtData()
	{
	if (iXVals) 
		{
		iXVals->ResetAndDestroy();
		delete iXVals;
		}

	if (iXNam) delete iXNam;
	}

// -----------------------------------------------------------------------------
// CNSmlExtData::AddXValL
// Adds given buffer to iXVals.
// -----------------------------------------------------------------------------
//
EXPORT_C void CNSmlExtData::AddXValL( HBufC8* aXVal )
	{
	iXVals->AppendL(aXVal);
	}

// -----------------------------------------------------------------------------
// CNSmlExtData::GenerateXmlL
// Generates ext data xml using aParser and appends the xml to aXml.
// -----------------------------------------------------------------------------
//
EXPORT_C void CNSmlExtData::GenerateXmlL( TPtr8& aXml, const CNSmlXmlParser* aParser) const
	{
	if ( aParser == NULL )
		return;

	aParser->AppendElement(aXml, KExtElement());

	if( iXNam )
		{
		aParser->AppendElement(aXml, KExtXNamElement(), *iXNam);
		}
	else
		User::Leave(EMandatoryFieldNotFound);

	if ( iXVals )
		{
		for (TInt i=0; i < iXVals->Count(); ++i)
			{
			aParser->AppendElement(aXml, KExtXValElement(), *iXVals->At(i));
			}
		}

	aParser->AppendEndElement(aXml, KExtElement());

	}

// -----------------------------------------------------------------------------
// CNSmlExtData::CountXmlSize
// Counts the size of the ext data xml if it was generated with current 
// values in member variables.
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CNSmlExtData::CountXmlSize( const CNSmlXmlParser* aParser ) const
	{
	TInt size = 0;
	size += aParser->SizeOfElements( KExtElement() );

	if( iXNam )
		{
		size += aParser->SizeOfString( iXNam, KExtXNamElement() );
		}

	if ( iXVals )
		{
		for (TInt i=0; i < iXVals->Count(); ++i)
			{
			size += aParser->SizeOfString( iXVals->At(i), KExtXValElement() );
			}
		}

	return size;
	}

// -----------------------------------------------------------------------------
// CNSmlExtData::ConvertIntoEntitiesL
// Converts special characters in member variables into entities.
// -----------------------------------------------------------------------------
//
EXPORT_C void CNSmlExtData::ConvertIntoEntitiesL( const CNSmlXmlParser* aParser )
	{
	if( iXNam )
		aParser->CharactersToEntitiesL(iXNam, 0, iXNam->Length());

	if( iXVals )
		{
		for (TInt i=0; i < iXVals->Count(); ++i)
			{
			aParser->CharactersToEntitiesL(iXVals->At(i), 0, iXVals->At(i)->Length());
			}
		}
	}

// -----------------------------------------------------------------------------
// CNSmlExtData::CNSmlExtData
// Constructor.
// -----------------------------------------------------------------------------
//
CNSmlExtData::CNSmlExtData()
	{
	}

// -----------------------------------------------------------------------------
// CNSmlExtData::ConstructL
// Second phase construction.
// -----------------------------------------------------------------------------
//
void CNSmlExtData::ConstructL()
	{
	iXVals = new (ELeave) CNSmlXValArray(3);
	}

//  End of File
