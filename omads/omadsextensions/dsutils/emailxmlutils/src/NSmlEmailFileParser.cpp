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
// CNSmlEmailFileParser::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
EXPORT_C CNSmlEmailFileParser* CNSmlEmailFileParser::NewL()
	{
	CNSmlEmailFileParser* self = CNSmlEmailFileParser::NewLC();
	CleanupStack::Pop();

	return self;
	}

// -----------------------------------------------------------------------------
// CNSmlEmailFileParser::NewLC
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
EXPORT_C CNSmlEmailFileParser* CNSmlEmailFileParser::NewLC()
	{
	CNSmlEmailFileParser* self = new (ELeave) CNSmlEmailFileParser();
	CleanupStack::PushL(self);
	self->ConstructL();

	return self;
	}

// -----------------------------------------------------------------------------
// CNSmlEmailFileParser::~CNSmlEmailFileParser
// Destructor.
// -----------------------------------------------------------------------------
//
EXPORT_C CNSmlEmailFileParser::~CNSmlEmailFileParser()
	{
	// close rfs and open streams
	if ( iWriteStream.Sink() )
		iWriteStream.Close();

	iRfs.Close();
	delete iEmailItemFile;
	}

// -----------------------------------------------------------------------------
// CNSmlEmailFileParser::ParseXml
// Parses the email xml, that is located in the file whose name is given 
// as aXml (path and filename)
// -----------------------------------------------------------------------------
//
EXPORT_C TNSmlParserGeneratorError CNSmlEmailFileParser::ParseXml( HBufC8* aXml )
	{
	if ( !aXml )
		return EInvalidFilename;

	HBufC* filename = Buf8ToBuf16( aXml );
	if ( !filename )
		{
		return EOutOfMemory;
		}

	// open the stream and parse
	TNSmlParserGeneratorError err = EErrorNone;
	RFileReadStream rs;
	if ( rs.Open(iRfs, *filename, KFileReadAccess) != KErrNone )
		{
		err = EInvalidFilename;
		}
	else
		{
		err = ParseXml( rs );
		rs.Close();
		}

	delete filename;

	return err;
	}

// -----------------------------------------------------------------------------
// CNSmlEmailFileParser::ParseXml
// Parses the xml from the given stream, and writes the emailitem - if present - to a temp file
// -----------------------------------------------------------------------------
//
EXPORT_C TNSmlParserGeneratorError CNSmlEmailFileParser::ParseXml( RReadStream& aRs )
	{
	iSetValues.Reset();
	iCurrentState = ENone;
	iLastState = ENone;

	TRAPD(err, ParseFromStreamL( aRs ));

	if( err == EErrorNone && iCurrentState != ENone )
		return EInvalidXmlError;

	return CheckError(err);
	}

// -----------------------------------------------------------------------------
// CNSmlEmailFileParser::GenerateXml
// Generates the xml and writes it to the file whose name is given in aXml (path 
// and filename).
// -----------------------------------------------------------------------------
//
EXPORT_C TNSmlParserGeneratorError CNSmlEmailFileParser::GenerateXml( HBufC8*& aXml )
	{
	if ( !aXml )
		return EInvalidFilename;

	HBufC* filename = Buf8ToBuf16( aXml );
	if ( !filename )
		{
		return EOutOfMemory;
		}

	// open the stream and generate
	iWriteStream.Replace(iRfs, *filename, KFileWriteAccess);
	TNSmlParserGeneratorError err = GenerateXml( iWriteStream );
	iWriteStream.Close();

	delete filename;

	return err;
	}

// -----------------------------------------------------------------------------
// CNSmlEmailFileParser::GenerateXml
// Generates the xml and writes it to the given stream.
// -----------------------------------------------------------------------------
//
EXPORT_C TNSmlParserGeneratorError CNSmlEmailFileParser::GenerateXml( RWriteStream& aWs )
	{
	TRAPD( err, GenerateEmailXmlL( aWs ) );

	return CheckError(err);
	}

// -----------------------------------------------------------------------------
// CNSmlEmailFileParser::GenerateEmailXmlL
// -----------------------------------------------------------------------------
//
void CNSmlEmailFileParser::GenerateEmailXmlL( RWriteStream& aWs )
	{
	ConvertIntoEntitiesL();

	// count the size of the xml (does not include emailitem, since it is - if present - in a file)
	TInt size = CNSmlEmailParser::CountXmlSizeL();
	if ( iUseEmailItem ) size += 20; // extra for emailitem tags

	// create a buffer for using the size
	HBufC8* xml = HBufC8::NewLC(size);
	TPtr8 ptr = xml->Des();

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

	// if emailitem should be used in generation, write it to the stream
	if ( iUseEmailItem )
		{
		// append start tag and cdata start
		AppendElement(ptr, KEmailItemElement());
		ptr.Append(KCDataStart);

		// open a stream to emailitem
		RFileReadStream rs;
		TInt err = rs.Open( iRfs, *iEmailItemFile, KFileReadAccess );
		if ( err != KErrNone )
			User::Leave( EInvalidFilename );
		
		CleanupClosePushL(rs);
		
		// write the buffer to the stream
		aWs.WriteL(ptr);
		
		// write emailitem from stream
		aWs.WriteL(rs);
		
		// close the opened read stream
		CleanupStack::Pop(); // rs

		// empty the buffer (i.e. set the length to zero)
		ptr.SetLength(0);

		// append emailitem end tag and cdata inner end
		ptr.Append(KCDataEnd);
		AppendEndElement(ptr, KEmailItemElement());
		}

	// append the rest of the data to the buffer
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

	// write the buffer to the stream and commit
	aWs.WriteL(ptr);
	aWs.CommitL();

	CleanupStack::PopAndDestroy(); // xml

	}

// -----------------------------------------------------------------------------
// CNSmlEmailFileParser::CountXmlSizeL
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CNSmlEmailFileParser::CountXmlSizeL()
	{
	// counting the size is not essential, since xml is written to a file
	return KErrNotSupported;
	}

// -----------------------------------------------------------------------------
// CNSmlEmailFileParser::HandleNextEmailItemDataL
// Next emailitem data
// -----------------------------------------------------------------------------
//
void CNSmlEmailFileParser::HandleNextEmailItemDataL( TPtrC8 aData )
	{
	// write the given piece of data into a file (stream)
	iWriteStream.WriteL(aData);
	}


// -----------------------------------------------------------------------------
// CNSmlEmailFileParser::HandleNextEmailItemStartL
// Emailitem element starts
// -----------------------------------------------------------------------------
//
void CNSmlEmailFileParser::HandleNextEmailItemStartL()
	{
	// open the write stream
	User::LeaveIfError( iWriteStream.Replace( iRfs, *iEmailItemFile, KFileWriteAccess ) );
	iReadingEmailItemData = ETrue;
	}

// -----------------------------------------------------------------------------
// CNSmlEmailFileParser::HandleNextEmailItemEndL
// Emailitem element ends
// -----------------------------------------------------------------------------
//
void CNSmlEmailFileParser::HandleNextEmailItemEndL()
	{
	// close the write stream
	iWriteStream.Close();
	iReadingEmailItemData = EFalse;
	}

// -----------------------------------------------------------------------------
// CNSmlEmailFileParser::ParseFromStreamL
// -----------------------------------------------------------------------------
//
void CNSmlEmailFileParser::ParseFromStreamL( RReadStream& aRs )
	{

	// reset preprocess variables and parser
	ResetPreProcessor();
	ResetParserL();

	// read and process the data piece by piece
	HBufC8* piece = HBufC8::NewLC(KReadDataLength);
	TPtr8 ptr = piece->Des();
	TInt dataSize = aRs.Source()->SizeL();

	TInt pos = 0;
	iFirstPiece = ETrue;
	for( pos = KReadDataLength; pos <= dataSize ; pos += KReadDataLength )
		{
		// read piece of data from the stream
		aRs.ReadL(ptr, KReadDataLength);

		// check that there are no incomplete cdata/comment/entity at the end
		dataSize += RemoveIncompleteL( piece, aRs );

		// pre-process
		PreProcessPieceL( piece );

		// parsing
		TPtrC8 temp( *piece );
		ParsePieceL( temp );
		
		iFirstPiece = EFalse;
		}

	TInt lastPartLength = dataSize - pos + KReadDataLength;
	aRs.ReadL(ptr, lastPartLength);

	PreProcessPieceL( piece );

	// parsing
	TPtrC8 temp( *piece );
	ParsePieceL( temp, ETrue );

	CleanupStack::PopAndDestroy(); // piece
	}

// -----------------------------------------------------------------------------
// CNSmlEmailFileParser::ParsePieceL
// -----------------------------------------------------------------------------
//
void CNSmlEmailFileParser::ParsePieceL( TPtrC8& aXml, TBool aLastPiece )
	{

	// check if last cdata has not ended yet
	if ( iReadingCData )
		{
		TInt endPos = aXml.Find(KCDataEnd);
		TPtrC8 cdata = _L8("");

		if ( endPos == KErrNotFound && aLastPiece )
			{
			User::Leave(EInvalidCDataStructure);
			}
		else if ( endPos == KErrNotFound )
			{
			// cdata end was not found, read the whole piece as cdata
			NextDataL( aXml );
			return;
			}
		else 
			{
			// add data until cdata end to buffer
			cdata.Set( aXml.Left( endPos ) );
			aXml.Set( aXml.Right( aXml.Length() - endPos - KCDataEnd().Length() ) );			
			iReadingCData = EFalse;
			}

		AddToCompleteL(cdata);
		NextDataL(*iCompleteBuffer);
		ResetBufferL(iCompleteBuffer);

		}
#ifndef __NO_XML_COMMENTS_
	else if ( iReadingComment ) // check if last comment has not ended yet
		{
		TInt endPos = aXml.Find(KCommentEnd);

		if ( endPos == KErrNotFound && aLastPiece )
			{ // this is the last piece of data and end of comment has not been found -> error
			User::Leave(EInvalidXmlError);
			}
		else if ( endPos == KErrNotFound )
			{
			// end of comment was not found, no need to parse this piece of data
			return;
			}
		else 
			{
			// end of cdata was found, jump to that position
			aXml.Set( aXml.Right( aXml.Length() - endPos - KCommentEnd().Length() ) );
			iReadingComment = EFalse;
			}
		}
#endif

	// parse through the string
	TText c;

	for( TInt i=0; i < aXml.Length(); ++i )
		{
		c = aXml[i];
		switch(c)
			{
			case KElementStart:
				// if currently reading element, error
				if( iReadingElementName )
					{
					User::Leave(EInvalidXmlError);
					}

				if( aXml.Length()-i >= KCDataStart().Length() &&
					!aXml.Mid(i, KCDataStart().Length()).Compare(KCDataStart()) )
					{ // cdata
					aXml.Set( aXml.Right( aXml.Length() - i ) );
					TInt endPos = aXml.Find(KCDataEnd);

					TPtrC8 cdata = _L8("");
					if ( endPos == KErrNotFound && aLastPiece )
						{
						User::Leave(EInvalidCDataStructure);
						}
					else if ( endPos == KErrNotFound )
						{
						// cdata end was not found, read all the rest as cdata
						cdata.Set( aXml.Right( aXml.Length() - KCDataStart().Length() ) );
						i = aXml.Length(); // move to the end
						iReadingCData = ETrue;
						}
					else 
						{
						cdata.Set( aXml.Mid( KCDataStart().Length(), endPos - KCDataStart().Length() ) );
						aXml.Set( aXml.Right( aXml.Length() - endPos - KCDataEnd().Length() ) );

						i = -1;
						}

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

					if ( endPos == KErrNotFound && aLastPiece)
						{
						// this is the last piece of data, and end of comment was not found -> error
						User::Leave(EInvalidXmlError);
						}
					else if ( endPos == KErrNotFound )
						{
						// the end was not found, no need to parse the rest since it is comment
						iReadingComment = ETrue;
						return;
						}
					else
						{
						// the end of comment was found, move to the end of comment and start parsing 
						// from there on
						aXml.Set( aXml.Right( aXml.Length() - endPos - KCommentEnd().Length() ) );
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
					iReadingElementName = ETrue;
					}

				break;
			case KElementEnd:
				// stop reading element name
				if( !iReadingElementName )
					{
					User::Leave(EInvalidXmlError);
					}
				else 
					{
					NextElementL(*iBuffer);
					ResetBufferL(iBuffer);
					iReadingElementName = EFalse;
					}
				break;
			default:
				// add char to buffer
				AddToBufferL(c, iBuffer);
				break;
			}
		}

	// send the data in the buffer
	if ( ( iReadingEmailItemData || aLastPiece ) && !iReadingElementName )
		{
		EntitiesToCharactersL( iBuffer, 0, iBuffer->Length() );
		AddToCompleteL(*iBuffer);
		NextDataL(*iCompleteBuffer);
		ResetBufferL(iBuffer);
		ResetBufferL(iCompleteBuffer);
		}

	}

// -----------------------------------------------------------------------------
// CNSmlEmailFileParser::RemoveIncompleteL
// Checks if there is incomplete cdata/comment/entity at the end of the given
// piece. If one is found, it is removed from aPiece and the read mark of the
// stream is moved backwards the length of the removed item (i.e. the incomplete
// item will be read to the next piece as a whole).
// -----------------------------------------------------------------------------
//
TUint CNSmlEmailFileParser::RemoveIncompleteL( HBufC8* aPiece, RReadStream& aStream )
	{
	// we need to check only the last nine characters
	TUint removeSize = CheckPiece( aPiece->Right( 9 ) );

	if ( removeSize )
		{
		// remove the incomplete item from aPiece
		TPtr8 ptr = aPiece->Des();
		ptr.Delete( aPiece->Length() - removeSize, removeSize );

		// move backwards in reading the stream
		aStream.Source()->SeekL( MStreamBuf::ERead, aStream.Source()->TellL(MStreamBuf::ERead) - removeSize );
		}

	// no need to loop the rest of aPiece. If xml is not correctly formed,
	// an error occures later in the parser

	return removeSize;
	}

// -----------------------------------------------------------------------------
// CNSmlEmailFileParser::CheckPiece
// -----------------------------------------------------------------------------
//
TUint CNSmlEmailFileParser::CheckPiece( const TPtrC8 aEnd ) const
	{
	// check that there are no incomplete cdata/comment/entity at the end	
	// (cdata start, cdata end, cdata inner end, comment start, comment end, entities)
	// ( <![CDATA[ , ]]> , <!-- , --> , &xxxx; , ]]]]>&gt; , ]]]]&gt;> )
	// do not change the order

	// possible inner cdata end
	TInt pos = aEnd.Find( _L8("]]]") );
	if ( pos != KErrNotFound )
		{
		return aEnd.Length() - pos;
		}

	// beginning part of comment or cdata
	pos = aEnd.Find( _L8("<!") );
	if ( pos != KErrNotFound )
		{
		return aEnd.Length() - pos;
		}

	// end of cdata
	TText lastChar = aEnd[aEnd.Length()-1];
	TText secondLastChar = aEnd[aEnd.Length()-2];
	if ( lastChar == ']' && secondLastChar == ']' )
		{
		return 2;
		}

#ifndef __NO_XML_COMMENTS_
	// comment end
	if ( lastChar == '-' && secondLastChar == '-' )
		{
		return 2;
		}
#endif

	// special chars
	if ( lastChar == '<' || lastChar == ']' || lastChar == '-' )
		{
		return 1;
		}

	// possible entity
	pos = aEnd.Find( _L8("&") );
	if ( pos != KErrNotFound )
		{
		return aEnd.Length() - pos;
		}

	return 0;
	}

// -----------------------------------------------------------------------------
// CNSmlEmailFileParser::PreProcessPieceL
// -----------------------------------------------------------------------------
//
void CNSmlEmailFileParser::PreProcessPieceL( HBufC8* aXml )
	{
	// take a modifiable pointer
	TPtr8 xml = aXml->Des();

	// the string used for searching and moving in the string
	TPtrC8 searchString(*aXml);

	// the current position in the original string
	TInt searchStartPos = 0;

	// if start has been found and end has not, try to find end first
	if ( iCdataEndPos == KErrNotFound && iCdataStartPos != KErrNotFound && iCdataFoundFromFirstPiece )
		{
		// we're looking for a cdata end
		iCdataEndPos = searchString.Find(KCDataEnd);

		// if the end was found, remove it and start looking for a cdata start
		if ( iCdataEndPos != KErrNotFound )
			{
			// remove cdata end
			xml.Delete(iCdataEndPos, KCDataEnd().Length());
			searchStartPos = iCdataEndPos;
			searchString.Set( xml.Right(xml.Length() - searchStartPos) );
			}
		else
			{
			// end was not found, return
			return;
			}
		}
	
	iCdataStartPos = searchString.Find(KCDataStart);
	
	// If CDATA is not found from beginning then data is not inside CDATA and then
	// preprocessing is not needed
	if ( iCdataStartPos != 0 && iFirstPiece )
	    {
	    return;
	    }
	else if ( !iFirstPiece && !iCdataFoundFromFirstPiece )
		{
		return;
		}
	else if ( iCdataStartPos == 0 && iFirstPiece )
		{
		iCdataFoundFromFirstPiece = ETrue;	
		}
	
	
	// while cdata is found
	while ( iCdataStartPos != KErrNotFound )
		{
		iCdataStartPos += searchStartPos;
		
		// find an end of cdata before entities are converted
		iCdataEndPos = searchString.Find(KCDataEnd);
		
		// convert entities between search start and cdata start
		TInt entityChange = EntitiesToCharactersL(aXml, searchStartPos, iCdataStartPos);
		xml.Set(aXml->Des());
		iCdataStartPos += entityChange;
		
		if ( iCdataEndPos != KErrNotFound )
			{
			iCdataEndPos += entityChange;
			iCdataEndPos += searchStartPos;
			
			// if the end is before start -> error
			if ( iCdataEndPos < iCdataStartPos )
				User::Leave( EInvalidCDataStructure );
			
			// remove cdata end
			xml.Delete(iCdataEndPos, KCDataEnd().Length());
			// remove cdata start
			xml.Delete(iCdataStartPos, KCDataStart().Length());
			
			searchStartPos = iCdataEndPos - KCDataStart().Length();
			searchString.Set( xml.Right(xml.Length() - searchStartPos) );
			}
		else
			{
			// remove cdata start
			xml.Delete(iCdataStartPos, KCDataStart().Length());
			
			// since there was no end found, return
			return;
			}
		
		// find a new cdata start that is after the found end
		iCdataStartPos = searchString.Find(KCDataStart);
		
		}
	
	if ( iCdataStartPos == KErrNotFound )
		{
		EntitiesToCharactersL(aXml, searchStartPos, aXml->Length());
		}

	}

// -----------------------------------------------------------------------------
// CNSmlEmailFileParser::ResetParserL
// Resets the state of the parser, should be called before starting the parsing.
// -----------------------------------------------------------------------------
//
void CNSmlEmailFileParser::ResetParserL()
	{
	ResetBufferL(iBuffer);
	ResetBufferL(iCompleteBuffer);
	iReadingElementName = EFalse;
	iReadingCData = EFalse;
	iReadingEmailItemData = EFalse;
	iFirstPiece = EFalse;
	iCdataFoundFromFirstPiece = EFalse;
	}

// -----------------------------------------------------------------------------
// CNSmlEmailFileParser::ResetPreProcessor
// Resets the variables used in preprocessor.
// -----------------------------------------------------------------------------
//
void CNSmlEmailFileParser::ResetPreProcessor()
	{
	iCdataStartPos = KErrNotFound;
	iCdataEndPos = KErrNotFound;
	}

// -----------------------------------------------------------------------------
// CNSmlEmailFileParser::Buf8ToBuf16
// -----------------------------------------------------------------------------
//
HBufC* CNSmlEmailFileParser::Buf8ToBuf16( const HBufC8* buf8 ) const
	{
	HBufC* buf16 = HBufC::New(buf8->Length());
	if ( buf16 )
		{
		TPtr ptr = buf16->Des();
		ptr.Copy( *buf8 );
		}

	return buf16;
	}

// -----------------------------------------------------------------------------
// CNSmlEmailFileParser::CNSmlEmailFileParser
// Constructor.
// -----------------------------------------------------------------------------
//
CNSmlEmailFileParser::CNSmlEmailFileParser()
	: iUseEmailItem(ETrue)
	{
	}

// -----------------------------------------------------------------------------
// CNSmlEmailFileParser::ConstructL
// Second phase construction.
// -----------------------------------------------------------------------------
//
void CNSmlEmailFileParser::ConstructL()
	{
	CNSmlEmailParser::ConstructL();
	User::LeaveIfError( iRfs.Connect() );
	
	// create path+filename for emailitem
    iEmailItemFile = HBufC::NewL( KMaxEmailItemFileNameLength );
    TPtr file = iEmailItemFile->Des();

	file.Append( KEmailItemFile );

	}

//  End of File
