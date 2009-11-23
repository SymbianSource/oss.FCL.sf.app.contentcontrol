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
// TNSmlFolderAttributeData::TNSmlFolderAttributeData
// Constructor
// -----------------------------------------------------------------------------
//
EXPORT_C TNSmlFolderAttributeData::TNSmlFolderAttributeData()
: iHidden(EBooleanMissing), iSystem(EBooleanMissing), 
iArchived(EBooleanMissing), iDelete(EBooleanMissing),
iWritable(EBooleanMissing), iReadable(EBooleanMissing),
iExecutable(EBooleanMissing)
	{
	}

// -----------------------------------------------------------------------------
// TNSmlFolderAttributeData::AttributeCount
// Returns the amount of fields that are not EBooleanMissing
// -----------------------------------------------------------------------------
//
EXPORT_C TInt TNSmlFolderAttributeData::AttributeCount() const
	{
	TInt count = 0;

	if ( iHidden ) count++;
	if ( iSystem ) count++;
	if ( iArchived ) count++;
	if ( iDelete ) count++;
	if ( iWritable ) count++;
	if ( iReadable ) count++;
	if ( iExecutable ) count++;

	return count;
	}

// -----------------------------------------------------------------------------
// TNSmlFolderAttributeData::GenerateXml
// Generates attribute xml
// -----------------------------------------------------------------------------
//
EXPORT_C void TNSmlFolderAttributeData::GenerateXml( TPtr8& aXml, const CNSmlXmlParser* aParser ) const
	{
	aParser->AppendElement(aXml, KFolderAttributesElement());

	if ( iHidden )
		{
		aParser->AppendElement(aXml, KAttributeHiddenElement(), aParser->BooleanToString( iHidden ));
		}

	if ( iSystem )
		{
		aParser->AppendElement(aXml, KAttributeSystemElement(), aParser->BooleanToString( iSystem ));
		}

	if ( iArchived )
		{
		aParser->AppendElement(aXml, KAttributeArchivedElement(), aParser->BooleanToString( iArchived ));
		}

	if ( iDelete )
		{
		aParser->AppendElement(aXml, KAttributeDeleteElement(), aParser->BooleanToString( iDelete ));
		}

	if ( iWritable )
		{
		aParser->AppendElement(aXml, KAttributeWritableElement(), aParser->BooleanToString( iWritable ));
		}

	if ( iReadable )
		{
		aParser->AppendElement(aXml, KAttributeReadableElement(), aParser->BooleanToString( iReadable ));
		}

	if ( iExecutable )
		{
		aParser->AppendElement(aXml, KAttributeExecutableElement(), aParser->BooleanToString( iExecutable ));
		}

	aParser->AppendEndElement(aXml, KFolderAttributesElement());
	}

// -----------------------------------------------------------------------------
// TNSmlFolderAttributeData::CountXmlSize
// Coounts the size of the xml if it was generated using the values currently in 
// member variables
// -----------------------------------------------------------------------------
//
EXPORT_C TInt TNSmlFolderAttributeData::CountXmlSize( const CNSmlXmlParser* aParser ) const
	{
	TInt size = 0;
	size += aParser->SizeOfElements( KFolderAttributesElement() );

	if ( iHidden )
		{
		size += aParser->SizeOfBoolean( iHidden, KAttributeHiddenElement() );
		}

	if ( iSystem )
		{
		size += aParser->SizeOfBoolean( iSystem, KAttributeSystemElement() );
		}

	if ( iArchived )
		{
		size += aParser->SizeOfBoolean( iArchived, KAttributeArchivedElement() );
		}

	if ( iDelete )
		{
		size += aParser->SizeOfBoolean( iDelete, KAttributeDeleteElement() );
		}

	if ( iWritable )
		{
		size += aParser->SizeOfBoolean( iWritable, KAttributeWritableElement() );
		}

	if ( iReadable )
		{
		size += aParser->SizeOfBoolean( iReadable, KAttributeReadableElement() );
		}

	if ( iExecutable )
		{
		size += aParser->SizeOfBoolean( iExecutable, KAttributeExecutableElement() );
		}

	return size;
	}

//  End of File
