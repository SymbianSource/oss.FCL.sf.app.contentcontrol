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


// 1.2 Changes: nsmlxmlparser module added

// ------------------------------------------------------------------------------------------------
// Includes
// ------------------------------------------------------------------------------------------------
#include <s32buf.h>
#include <s32file.h>
#include "nsmlxmlparser.h"


// ------------------------------------------------------------------------------------------------
// 
// CNSmlXmlParser methods
// 
// ------------------------------------------------------------------------------------------------

// ------------------------------------------------------------------------------------------------
// Constructor
// ------------------------------------------------------------------------------------------------
EXPORT_C CNSmlXmlParser::CNSmlXmlParser()
	{
	}


// ------------------------------------------------------------------------------------------------
// Destructor.
// ------------------------------------------------------------------------------------------------
EXPORT_C CNSmlXmlParser::~CNSmlXmlParser()
	{
	if( iBuffer ) delete iBuffer;
	if ( iCompleteBuffer ) delete iCompleteBuffer;
	}


// ------------------------------------------------------------------------------------------------
// Parses the given string for cdata areas and entitys. If a cdata area is 
// found, the data in it is skipped over. The entities outside cdata are converted 
// into characters they represent (but only if cdata is found).
// Note: This method is intended for processing the CDATA used right after <Data>. 
// I.e. this method removes the cdata-elements used for wrapping the whole xml data 
// and processes the string so that the inner cdatas used in the xml become valid.
// THIS METHOD SHOULD BE CALLED RIGHT IN THE BEGINNING OF PARSING IF <Data>-BLOCK 
// IS WRAPPED WITHIN CDATA.
// ------------------------------------------------------------------------------------------------
void CNSmlXmlParser::PreProcessL( HBufC8* aXml ) const
	{
	// take a modifiable pointer
	TPtr8 xml = aXml->Des();

	// the string used for searching and moving in the string
	TPtrC8 searchString(*aXml);

	// the current position in the original string
	TInt searchStartPos = 0;

	// find the first cdata start
	TInt cdataStartPos = searchString.Find(KCDataStart);
	
	// If CDATA is not found from beginning then data is not inside CDATA and then
	// preprocessing is not needed
	if ( cdataStartPos != 0 )
	    {
	    return;
	    }

	TInt cdataEndPos = KErrNotFound;

	// while cdata is found
	while ( cdataStartPos != KErrNotFound )
		{
		cdataStartPos += searchStartPos;

		// find an end of cdata before entities are converted
		cdataEndPos = searchString.Find(KCDataEnd);

		// convert entities between search start and cdata start
		TInt entityChange = EntitiesToCharactersL(aXml, searchStartPos, cdataStartPos);
		xml.Set(aXml->Des());
		cdataStartPos += entityChange;

		if ( cdataEndPos != KErrNotFound )
			{
			cdataEndPos += entityChange;
			cdataEndPos += searchStartPos;

			// if the end is before start -> error
			if (cdataEndPos < cdataStartPos)
				User::Leave( EInvalidCDataStructure );
				
			// remove cdata end
			xml.Delete(cdataEndPos, KCDataEnd().Length());
			// remove cdata start
			xml.Delete(cdataStartPos, KCDataStart().Length());

			searchStartPos = cdataEndPos - KCDataStart().Length();
			searchString.Set( xml.Right(xml.Length() - searchStartPos) );
			}
		else 
			{
			// the end of cdata was not found or cdata end was before start -> error
			User::Leave( EInvalidCDataStructure );
			}

		// find a new cdata start that is after the found end
		cdataStartPos = searchString.Find(KCDataStart);

		}

	}


// ------------------------------------------------------------------------------------------------
// Return the TNSmlBoolean-value's string-representation.
// ------------------------------------------------------------------------------------------------
TPtrC8 CNSmlXmlParser::BooleanToString( const TNSmlBoolean aValue ) const
	{
	TPtrC8 str;
	switch(aValue)
		{
		case EBooleanTrue:
			{
			str.Set(KStringTrue);
			break;
			}
		case EBooleanFalse:
			{
			str.Set(KStringFalse);
			break;
			}
		default:
			{
			str.Set(KStringEmpty);
			}
		}

	return str;
	}


// ------------------------------------------------------------------------------------------------
// Returns the TTime-value's string-representation.
// ------------------------------------------------------------------------------------------------
TBuf8<KDateTimeLength> CNSmlXmlParser::DateTimeToStringL( const TTime& aValue ) const
	{
	TBuf<KDateTimeLength> str;
	
	// aValue is not changed but Z character is added to the end of string.
	// Messaging uses UTC times and that is reason why time is not changed.
	aValue.FormatL( str, _L("%F%Y%M%DT%H%T%SZ") );

	// from 16-bit to 8-bit
	TBuf8<KDateTimeLength> dt;
	dt.Copy(str);

	return dt;
	}


// ------------------------------------------------------------------------------------------------
// Returns the integer value's string-representation.
// ------------------------------------------------------------------------------------------------
TBuf8<KIntegerMaxLength> CNSmlXmlParser::IntegerToString( const TInt aValue ) const
	{
	TBuf8<KIntegerMaxLength> str;
	str.AppendNum(aValue);
	return str;
	}


// ------------------------------------------------------------------------------------------------
// Returns the boolean representation of the string or leaves if error (EInvalidBooleanValue).
// ------------------------------------------------------------------------------------------------
TNSmlBoolean CNSmlXmlParser::StringToBooleanL( const TPtrC8& aValue ) const
	{
	if( aValue == KStringTrue )
		return EBooleanTrue;
	else if( aValue == KStringFalse || aValue == KStringEmpty )
		return EBooleanFalse;
	else
		User::Leave(EInvalidBooleanValue);

	return EBooleanMissing;
	}

	
// ------------------------------------------------------------------------------------------------
// Returns the integer representation of the string or leaves if error (EInvalidIntegerValue).
// ------------------------------------------------------------------------------------------------
TInt CNSmlXmlParser::StringToIntegerL( const TPtrC8& aValue ) const
	{
	if (aValue.Length() <= 0)
		return 0;

	// convert the data to an integer
	TLex8 lex(aValue);
	TUint uValue = 0;
	TBool isNegative = EFalse;

	TChar c = lex.Peek();

	// check for a minus or plus sign
	if ( c == '-' )
		{
		isNegative = ETrue;
		lex.Inc();
		}
	else if ( c == '+' )
		{
		lex.Inc();
		}

	TRadix radix = EDecimal;
	c = lex.Peek();

	if (c == '0') // octal or hex
		{
		lex.Get();
		c = lex.Get();
		if ( c == 'x' || c == 'X' )
			{
			radix = EHex;
			}
		else
			{
			radix = EOctal;
			lex.UnGet(); // back up
			}
		}

	TInt err = lex.Val(uValue, radix);
	if ( err != KErrNone )
		User::Leave(EInvalidIntegerValue);

	TInt value = uValue;

	return isNegative ? value*(-1) : value;
	}


// ------------------------------------------------------------------------------------------------
// Returns the TTime representation of the string or leaves if error (EInvalidDatetimeValue).
// UTC times are not supported, i.e. datetimes that have Z-ending are treated as 
// local times.
// ------------------------------------------------------------------------------------------------
TTime CNSmlXmlParser::StringToTTimeL( TPtrC8& aValue ) const
	{
	// check that there is data
	if (aValue.Length() <= 0)
		User::Leave( EInvalidDatetimeValue );

	// format the data into a TTime

	if (aValue[aValue.Length()-1] == 'Z')
		{
		// The datetime is in UTC, which is not supported
		// no correction done, treat as local time
		aValue.Set( aValue.Left( aValue.Length()-1 ) );
		}

	TDateTime datetime(0,(TMonth)0,0,0,0,0,0);

	// read datetime and check errors

	TInt error = KErrNone;

	// read year
	TPtrC8 str = aValue.Left(4);
	TLex8 lex(str);
	TInt value;
	error = lex.Val(value);
	CheckDatetimeErrorL( error );
	error  = datetime.SetYear(value);
	CheckDatetimeErrorL( error );

	// read month
	str.Set(aValue.Mid(4, 2));
	lex.Assign(str);
	error = lex.Val(value);
	CheckDatetimeErrorL( error );
	--value;
	error = datetime.SetMonth((TMonth)value);
	CheckDatetimeErrorL( error );

	// read day
	str.Set(aValue.Mid(6, 2));
	lex.Assign(str);
	error = lex.Val(value);
	CheckDatetimeErrorL( error );
	--value;
	error = datetime.SetDay(value);
	CheckDatetimeErrorL( error );

	// Skip character 'T' and read hour
	str.Set(aValue.Mid(9, 2));
	lex.Assign(str);
	error = lex.Val(value);
	CheckDatetimeErrorL( error );
	error = datetime.SetHour(value);
	CheckDatetimeErrorL( error );

	// minutes
	str.Set(aValue.Mid(11, 2));
	lex.Assign(str);
	error = lex.Val(value);
	CheckDatetimeErrorL( error );
	error = datetime.SetMinute(value);
	CheckDatetimeErrorL( error );

	// seconds
	str.Set(aValue.Mid(13, 2));
	lex.Assign(str);
	error = lex.Val(value);
	CheckDatetimeErrorL( error );
	error = datetime.SetSecond(value);
	CheckDatetimeErrorL( error );

	return TTime(datetime);
	}


// ------------------------------------------------------------------------------------------------
// Adds start element, the value and end element to aPtr.
// ------------------------------------------------------------------------------------------------
void CNSmlXmlParser::AppendElement( TPtr8& aPtr, const TDesC8& aElementName, const TDesC8& aValue ) const
	{
	// start element
	aPtr.Append(KElementStart);
	aPtr.Append(aElementName);
	aPtr.Append(KElementEnd);

	// value
	aPtr.Append(aValue);

	// end element
	aPtr.Append(KElementStart);
	aPtr.Append(KCharacterSlash);
	aPtr.Append(aElementName);
	aPtr.Append(KElementEnd);
	}


// ------------------------------------------------------------------------------------------------
// Forms an element using the given element name and appends it to the given string.
// ------------------------------------------------------------------------------------------------
void CNSmlXmlParser::AppendElement( TPtr8& aPtr, const TDesC8& aElementName ) const
	{
	aPtr.Append(KElementStart);
	aPtr.Append(aElementName);
	aPtr.Append(KElementEnd);
	}


// ------------------------------------------------------------------------------------------------
// Forms an end element using the given element name and appends it to the given string.
// ------------------------------------------------------------------------------------------------
void CNSmlXmlParser::AppendEndElement( TPtr8& aPtr, const TDesC8& aElementName ) const
	{
	aPtr.Append(KElementStart);
	aPtr.Append(KCharacterSlash);
	aPtr.Append(aElementName);
	aPtr.Append(KElementEnd);
	}


// ------------------------------------------------------------------------------------------------
// Returns the total length of start and end tag.
// ------------------------------------------------------------------------------------------------
TInt CNSmlXmlParser::SizeOfElements( const TDesC8& aElementName ) const
	{
	TInt size = 0;

	// start element plus end element ( 1 = length of '/' char )
	size += 2*aElementName.Length() + 1;
	size += 2*KElementStartEndWidth; // '<' and '>'

	return size;
	}


// ------------------------------------------------------------------------------------------------
// Returns the length of the given boolean element and it's data if it was a string.
// ------------------------------------------------------------------------------------------------
TInt CNSmlXmlParser::SizeOfBoolean( const TNSmlBoolean aValue, const TDesC8& aElementName ) const
	{
	TInt size = 0;
	size += SizeOfElements( aElementName );
	switch ( aValue )
		{
		case EBooleanTrue:
			return size+4; // "true"
		case EBooleanFalse:
			return size+5; // "false"
		default:
			return 0;
		}
	}


// ------------------------------------------------------------------------------------------------
// Returns the length of the given datetime element and it's data if it was a string.
// ------------------------------------------------------------------------------------------------
TInt CNSmlXmlParser::SizeOfDatetime( const TDesC8& aElementName ) const
	{
	TInt size = SizeOfElements( aElementName ) + KDateTimeLength;
	return size;
	}


// ------------------------------------------------------------------------------------------------
// Returns the maximum length of the given integer element and it's data if it was a string.
// ------------------------------------------------------------------------------------------------
TInt CNSmlXmlParser::SizeOfInteger( const TDesC8& aElementName ) const
	{
	TInt size = SizeOfElements( aElementName ) + KIntegerMaxLength;
	return size;
	}


// ------------------------------------------------------------------------------------------------
// Returns the length of the given string element and it's data if it was a string.
// ------------------------------------------------------------------------------------------------
TInt CNSmlXmlParser::SizeOfString( const HBufC8* aValue, const TDesC8& aElementName ) const
	{
	TInt size = SizeOfElements( aElementName ) + aValue->Length();
	return size;
	}


// ------------------------------------------------------------------------------------------------
// Finds entities and replaces them with the characters they represent. Returns 
// an integer indicating the size change in aXml.
// ------------------------------------------------------------------------------------------------
TInt CNSmlXmlParser::EntitiesToCharactersL( HBufC8*& aXml, TInt aStartPos, TInt aEndPos ) const
	{
	TInt change = 0;
	TInt changeSum = 0;

	change = ReplaceL(aXml, KEntityLT, KLessThan, aStartPos, aEndPos);
	changeSum += change;
	aEndPos += change;

	change = ReplaceL(aXml, KEntityGT, KGreaterThan, aStartPos, aEndPos);
	changeSum += change;
	aEndPos += change;

	change = ReplaceL(aXml, KEntityAMP, KAmpersand, aStartPos, aEndPos);
	changeSum += change;
	aEndPos += change;

	change = ReplaceL(aXml, KEntityAPOS, KApostrophe, aStartPos, aEndPos);
	changeSum += change;
	aEndPos += change;

	change = ReplaceL(aXml, KEntityQUOT, KQuotation, aStartPos, aEndPos);
	changeSum += change;

	return changeSum;
	}


// ------------------------------------------------------------------------------------------------
// Finds special characters and replaces them with corresponding entities. Returns 
// an integer indicating the size change in aXml.
// ------------------------------------------------------------------------------------------------
TInt CNSmlXmlParser::CharactersToEntitiesL( HBufC8*& aXml, TInt aStartPos, TInt aEndPos ) const
	{
	TInt change = 0;
	TInt changeSum = 0;

	// Note: this replace has to be the first one, since it changes 
	// &-characters to &amp;s and all the other replaces generate 
	// &-characters as they are entities.
	change = ReplaceL(aXml, KAmpersand, KEntityAMP, aStartPos, aEndPos);
	changeSum += change;
	aEndPos += change;

	change = ReplaceL(aXml, KLessThan, KEntityLT, aStartPos, aEndPos);
	changeSum += change;
	aEndPos += change;

	change = ReplaceL(aXml, KGreaterThan, KEntityGT, aStartPos, aEndPos);
	changeSum += change;
	aEndPos += change;

	change = ReplaceL(aXml, KApostrophe, KEntityAPOS, aStartPos, aEndPos);
	changeSum += change;
	aEndPos += change;

	change = ReplaceL(aXml, KQuotation, KEntityQUOT, aStartPos, aEndPos);
	changeSum += change;

	return changeSum;
	}


// ------------------------------------------------------------------------------------------------
// Returns ETrue if all the characters in the given text are whitespace 
// characters, else EFalse.
// ------------------------------------------------------------------------------------------------
TBool CNSmlXmlParser::IsWhitespace( const TDesC8& aText ) const
	{
	// loop the string character by character
	TText c;
	for ( TInt i=0; i < aText.Length(); ++i )
		{
		c = aText[i];
		switch( c )
			{
			case KWhitespaceEmpty:
				break;
			case KWhitespaceLineFeed:
				break;
			case KWhitespaceNewLine:
				break;
			case KWhitespaceTabular:
				break;
			case KWhitespaceLineFeedNewLine:
				break;
			default:
				return EFalse;
			}
		}

	return ETrue;
	}


// ------------------------------------------------------------------------------------------------
// Leaves with EInvalidXmlError if the given string is not whitespace.
// ------------------------------------------------------------------------------------------------
void CNSmlXmlParser::LeaveIfNotWhiteSpaceL( const TDesC8& aText ) const
	{
	if ( !IsWhitespace( aText ) )
		{
		User::Leave( EInvalidXmlError );
		}
	}


// ------------------------------------------------------------------------------------------------
// Maps the given value to TNSmlParserGeneratorError.
// ------------------------------------------------------------------------------------------------
TNSmlParserGeneratorError CNSmlXmlParser::CheckError( const TInt error ) const
	{
	if ( error == KErrNoMemory )
		{
		return EOutOfMemory;
		}
	else if ( error < KErrNone )
		{
		// some system wide error, should not occur
		return EUnknownError;
		}
	else if ( error != EErrorNone )
		{
		return (TNSmlParserGeneratorError)error;
		}

	return EErrorNone;
	}


// ------------------------------------------------------------------------------------------------
// Parses the given xml and calls NextDataL and NextElementL methods when finds 
// data or element. 
// ------------------------------------------------------------------------------------------------
void CNSmlXmlParser::ParseL( TPtrC8& aXml )
	{

	ResetBufferL(iBuffer);
	ResetBufferL(iCompleteBuffer);

	// boolean indicating if reading element name (true) or insides of 
	// an element (false)
	TBool readingElementName = EFalse;
	TText c;

	for( TInt i=0; i < aXml.Length(); ++i )
		{
		c = aXml[i];
		switch(c)
			{
			case KElementStart:
				// if currently reading element, error
				if( readingElementName )
					{
					User::Leave(EInvalidXmlError);
					}

				if( aXml.Length()-i >= KCDataStart().Length() &&
					!aXml.Mid(i, KCDataStart().Length()).Compare(KCDataStart()) )
					{ // cdata
					aXml.Set( aXml.Right( aXml.Length() - i ) );
					TInt endPos = aXml.Find(KCDataEnd);

					TPtrC8 cdata = _L8("");
					if ( endPos == KErrNotFound )
						{
						User::Leave(EInvalidCDataStructure);
						}

					cdata.Set( aXml.Mid( KCDataStart().Length(), endPos - KCDataStart().Length() ) );
					aXml.Set( aXml.Right( aXml.Length() - endPos - KCDataEnd().Length() ) );

					i = -1;

					// add current buffer to complete buffer
					EntitiesToCharactersL( iBuffer, 0, iBuffer->Length() );
					AddToCompleteL(*iBuffer);
					ResetBufferL(iBuffer);
					AddToCompleteL(cdata);
					}
#ifndef __NO_XML_COMMENTS_
				else if( aXml.Length()-i >= KCommentStart().Length() &&
					!aXml.Mid(i, KCommentStart().Length()).Compare(KCommentStart()) )
					{ // comment
					aXml.Set( aXml.Right( aXml.Length() - i ) );
					TInt endPos = aXml.Find(KCommentEnd);

					if ( endPos != KErrNotFound )
						{
						aXml.Set( aXml.Right( aXml.Length() - endPos - KCommentEnd().Length() ) );
						}
					else
						{
						User::Leave(EInvalidXmlError);
						}

					i = -1;
					}
#endif
				else 
					{
					// send the buffer
					EntitiesToCharactersL( iBuffer, 0, iBuffer->Length() );
					AddToCompleteL(*iBuffer);
					NextDataL(*iCompleteBuffer);
					ResetBufferL(iBuffer);
					ResetBufferL(iCompleteBuffer);
					readingElementName = ETrue;
					}

				break;
			case KElementEnd:
				// stop reading element name
				if( !readingElementName )
					{
					User::Leave(EInvalidXmlError);
					}
				else 
					{
					NextElementL(*iBuffer);
					ResetBufferL(iBuffer);
					readingElementName = EFalse;
					}
				break;
			default:
				// add char to buffer
				AddToBufferL(c, iBuffer);
				break;
			}
		}

	EntitiesToCharactersL( iBuffer, 0, iBuffer->Length() );
	AddToCompleteL(*iBuffer);
	NextDataL(*iCompleteBuffer);
	}


// ------------------------------------------------------------------------------------------------
// Adds the given string to iCompleteBuffer
// ------------------------------------------------------------------------------------------------
void CNSmlXmlParser::AddToCompleteL( const TPtrC8 aStr )
	{
	if(!iCompleteBuffer)
		{
		iCompleteBuffer = HBufC8::NewL(aStr.Length());
		}

	TPtr8 ptr = iCompleteBuffer->Des();
	if( ptr.MaxLength() < iCompleteBuffer->Length()+aStr.Length() )
		{
		iCompleteBuffer = iCompleteBuffer->ReAllocL(iCompleteBuffer->Length()+aStr.Length());
		ptr.Set(iCompleteBuffer->Des());
		}

	ptr.Append(aStr);
	}


// ------------------------------------------------------------------------------------------------
// Deletes the given buffer and initializes it again to length 10
// ------------------------------------------------------------------------------------------------
void CNSmlXmlParser::ResetBufferL( HBufC8*& aBuf ) const
	{
	if( aBuf )
		{
		delete aBuf;
		aBuf = NULL;
		}

	aBuf = HBufC8::NewL(10);
	}


// ------------------------------------------------------------------------------------------------
// Adds the given char to given buffer
// ------------------------------------------------------------------------------------------------
void CNSmlXmlParser::AddToBufferL( const TText c, HBufC8*& aBuf ) const
	{
	TPtr8 ptr = aBuf->Des();
	if( ptr.MaxLength() == aBuf->Length()+1 )
		{
		aBuf = aBuf->ReAllocL(aBuf->Length()+10);
		ptr.Set(aBuf->Des());
		}

	TChar str = c;
	ptr.Append(str);
	}



// ------------------------------------------------------------------------------------------------
// Replaces all occurances of aTarget in aText with aItem. Returns an integer indicating 
// the size change in aText. aStartPos and aEndPos indicate the start and end positions 
// of aText to be parsed (if whole string should be parsed, use 0 and aText.Length()).
// ------------------------------------------------------------------------------------------------
TInt CNSmlXmlParser::ReplaceL( HBufC8*& aText, const TDesC8& aTarget, const TDesC8& aItem, TInt aStartPos, TInt aEndPos ) const
	{
	TInt change = 0;
	TInt searchPos = aStartPos;
	TPtrC8 text = aText->Mid(aStartPos, aEndPos-aStartPos);
	TInt pos = text.Find(aTarget);

	while ( pos != KErrNotFound )
		{
		pos += searchPos;

		TInt currentChange = aItem.Length() - aTarget.Length();
		change += currentChange;
		aEndPos += currentChange;
		searchPos = pos;

		if ( currentChange > 0 )
			{
			searchPos += currentChange;

			// check that aText is large enough
			if ( aText->Des().MaxLength() < aText->Length()+currentChange )
				{
				aText = aText->ReAllocL(aText->Length()+currentChange);
				}
			}

		// the actual replace
		aText->Des().Replace(pos, aTarget.Length(), aItem);

		text.Set( aText->Mid(searchPos, aEndPos-searchPos) );
		pos = text.Find(aTarget);
		}

	return change;
	}


// ------------------------------------------------------------------------------------------------
// 
// ------------------------------------------------------------------------------------------------
void CNSmlXmlParser::CheckDatetimeErrorL( const TInt error ) const
	{
	if ( error != KErrNone )
		{
		User::Leave( EInvalidDatetimeValue );
		}
	}

//End of File

