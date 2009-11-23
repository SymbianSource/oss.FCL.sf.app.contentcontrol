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
// 
// -----------------------------------------------------------------------------
// CNSmlTruncatedData::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
EXPORT_C CNSmlTruncatedData* CNSmlTruncatedData::NewL()
	{
	CNSmlTruncatedData* self = CNSmlTruncatedData::NewLC();
	CleanupStack::Pop();

	return self;
	}

// -----------------------------------------------------------------------------
// CNSmlTruncatedData::NewLC
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
EXPORT_C CNSmlTruncatedData* CNSmlTruncatedData::NewLC()
	{
	CNSmlTruncatedData* self = new (ELeave) CNSmlTruncatedData();
	CleanupStack::PushL(self);
	self->ConstructL();

	return self;
	}

// -----------------------------------------------------------------------------
// CNSmlTruncatedData::~CNSmlTruncatedData
// Destructor
// -----------------------------------------------------------------------------
//
EXPORT_C CNSmlTruncatedData::~CNSmlTruncatedData()
	{
	iAttachments.ResetAndDestroy();
	iAttachments.Close();

	iBodys.ResetAndDestroy();
	iBodys.Close();
	}

// -----------------------------------------------------------------------------
// CNSmlTruncatedData::AddAttachmentL
// Adds an attachment to iAttachments. Takes copies of buffers, caller must
// destroy the original buffers.
// -----------------------------------------------------------------------------
//
EXPORT_C void CNSmlTruncatedData::AddAttachmentL(HBufC8* aName, TInt aSize, HBufC8* aType)
	{
	CNSmlAttachmentTruncateInfo* attach = new (ELeave) CNSmlAttachmentTruncateInfo();
	CleanupStack::PushL(attach);

	attach->SetMembersL(aName, aType, aSize);

	iAttachments.Append(attach);

	CleanupStack::Pop(); // attach
	}

// -----------------------------------------------------------------------------
// CNSmlTruncatedData::AddBodyL
// Adds a body. Takes copies of buffers, caller must destroy the 
// original buffers.
// -----------------------------------------------------------------------------
//
EXPORT_C void CNSmlTruncatedData::AddBodyL(TInt aSize, HBufC8* aType)
	{
	CNSmlBodyTruncateInfo* body = new (ELeave) CNSmlBodyTruncateInfo();
	CleanupStack::PushL(body);

	body->SetMembersL(aType, aSize);

	iBodys.Append(body);

	CleanupStack::Pop(); // body
	}

// -----------------------------------------------------------------------------
// CNSmlTruncatedData::GenerateXmlL
// Generates truncate data xml
// -----------------------------------------------------------------------------
//
EXPORT_C void CNSmlTruncatedData::GenerateXmlL( TPtr8& aXml, const CNSmlXmlParser* aParser ) const
	{
	aParser->AppendElement(aXml, KTruncatedElement());
	
	TInt size = 0;
	HBufC8* type = 0;
	HBufC8* name = 0;

	for ( TInt j = 0; j < iBodys.Count(); ++j )
		{
		iBodys[j]->GetMembersL(type, size);

		aParser->AppendElement(aXml, KTruncatedBodyElement());

		if ( size != KErrNotFound )
			{
			aParser->AppendElement(aXml, KTruncatedSizeElement(), aParser->IntegerToString(size));
			}

		if ( type )
			{
			aParser->AppendElement(aXml, KTruncatedTypeElement(), *type);
			delete type; type = NULL;
			}

		aParser->AppendEndElement(aXml, KTruncatedBodyElement());
		}

	for ( TInt i = 0; i < iAttachments.Count(); ++i )
		{
		size = 0;
		type = NULL;
		name = NULL;

		iAttachments[i]->GetMembersL(name, type, size);

		aParser->AppendElement(aXml, KTruncatedAttachElement());

		if ( name )
			{
			aParser->AppendElement(aXml, KTruncatedNameElement(), *name);
			delete name; name = NULL;
			}

		if ( size != KErrNotFound )
			{
			aParser->AppendElement(aXml, KTruncatedSizeElement(), aParser->IntegerToString(size));
			}

		if ( type )
			{
			aParser->AppendElement(aXml, KTruncatedTypeElement(), *type);
			delete type; type = NULL;
			}

		aParser->AppendEndElement(aXml, KTruncatedAttachElement());
		}

	aParser->AppendEndElement(aXml, KTruncatedElement());
	}

// -----------------------------------------------------------------------------
// CNSmlTruncatedData::CountXmlSizeL
// Counts the size of truncate data xml if it was generated using current values
// in member variables.
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CNSmlTruncatedData::CountXmlSizeL( const CNSmlXmlParser* aParser ) const
	{
	TInt size = 0;
	size += aParser->SizeOfElements( KTruncatedElement() );
		
	for ( TInt j = 0; j < iBodys.Count(); ++j )
		{
		TInt bodySize = 0;
		HBufC8* bodyType = 0;
		iBodys[j]->GetMembersL(bodyType, bodySize);

		size += aParser->SizeOfElements( KTruncatedBodyElement() );

		if ( bodySize != KErrNotFound )
			{
			size += aParser->SizeOfInteger( KTruncatedSizeElement() );
			}

		if ( bodyType )
			{
			size += aParser->SizeOfString( bodyType, KTruncatedTypeElement() );
			delete bodyType; bodyType = NULL;
			}

		}

	for ( TInt i = 0; i < iAttachments.Count(); ++i )
		{
		TInt attachSize = 0;
		HBufC8* attachType = 0;
		HBufC8* attachName = 0;
		iAttachments[i]->GetMembersL(attachName, attachType, attachSize);

		size += aParser->SizeOfElements( KTruncatedAttachElement() );

		if ( attachName )
			{
			size += aParser->SizeOfString( attachName, KTruncatedNameElement() );
			delete attachName; attachName = NULL;
			}

		if ( attachSize != KErrNotFound )
			{
			size += aParser->SizeOfInteger( KTruncatedSizeElement() );
			}

		if ( attachType )
			{
			size += aParser->SizeOfString( attachType, KTruncatedTypeElement() );
			delete attachType; attachType = NULL;
			}

		}

	return size;
	}

// -----------------------------------------------------------------------------
// CNSmlTruncatedData::ConvertIntoEntitiesL
// Converts special characters in strings into entities.
// -----------------------------------------------------------------------------
//
EXPORT_C void CNSmlTruncatedData::ConvertIntoEntitiesL( const CNSmlXmlParser* aParser )
	{
	TInt size = 0;
	HBufC8* type = NULL;
	HBufC8* name = NULL;

	for ( TInt j = 0; j < iBodys.Count(); ++j )
		{
		iBodys[j]->GetMembersL(type, size);
		CleanupStack::PushL(type);

		if( type )
			aParser->CharactersToEntitiesL(type, 0, type->Length());

		iBodys[j]->SetMembersL(type, size);
		CleanupStack::Pop(); // type
		delete type; type = NULL;
		}

	for ( TInt i = 0; i < iAttachments.Count(); ++i )
		{
		iAttachments[i]->GetMembersL(name, type, size);
		CleanupStack::PushL(type);
		CleanupStack::PushL(name);

		if( name )
			{
			aParser->CharactersToEntitiesL(name, 0, name->Length());
			}
		if( type )
			{
			aParser->CharactersToEntitiesL(type, 0, type->Length());
			}

		iAttachments[i]->SetMembersL(name, type, size);
		CleanupStack::Pop(2); // name, type

		delete name; name = NULL;
		delete type; type = NULL;
		}

	}

// -----------------------------------------------------------------------------
// CNSmlTruncatedData::CNSmlTruncatedData
// Private constructor
// -----------------------------------------------------------------------------
//
CNSmlTruncatedData::CNSmlTruncatedData()
	{
	}

// -----------------------------------------------------------------------------
// CNSmlTruncatedData::ConstructL
// Second phase construction
// -----------------------------------------------------------------------------
//
void CNSmlTruncatedData::ConstructL()
	{
	iAttachments.Reset();
	iBodys.Reset();
	}

//  End of File
