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
* Description:  Email XML parser
*
*/


// 1.2 Changes: nsmlxmlparser module added


#ifndef __NSMLXMLPARSER_H__
#define __NSMLXMLPARSER_H__

// ------------------------------------------------------------------------------------------------
// Includes
// ------------------------------------------------------------------------------------------------
#include <e32base.h>
#include <s32strm.h>
#include "nsmlEmailmtmdefs.h"


// ------------------------------------------------------------------------------------------------
// Defines for binary size optimization
// ------------------------------------------------------------------------------------------------
#define __NO_XML_COMMENTS_
//#define __NO_EMPTY_ELEMENTS_


// ------------------------------------------------------------------------------------------------
// Class forward declarations
// ------------------------------------------------------------------------------------------------
class CNSmlXmlParser;
class CNSmlExtData;


// ------------------------------------------------------------------------------------------------
// Typedefs
// ------------------------------------------------------------------------------------------------
// the array used for xval-values
typedef CArrayPtrFlat<HBufC8> CNSmlXValArray;
// the array used for extension data
typedef CArrayPtrFlat<CNSmlExtData> CNSmlExtDataArray;
// the array used for truncate info attachments data
typedef RPointerArray<CNSmlAttachmentTruncateInfo> RNSmlAttachArray;
// the array used for truncate info body data
typedef RPointerArray<CNSmlBodyTruncateInfo> RNSmlBodyArray;


// ------------------------------------------------------------------------------------------------
// constants
// ------------------------------------------------------------------------------------------------

// the maximum length of an integer in characters
const TInt KIntegerMaxLength = 12;

// the length of a datetime in characters
const TInt KDateTimeLength = 16;

// the length of element start (<) and end (>)
const TInt KElementStartEndWidth = 2;

// constants used with files
const TInt KReadDataLength = 512;
const TUint KFileReadAccess	= EFileRead|EFileShareAny;
const TUint KFileWriteAccess = EFileWrite|EFileShareExclusive;
const TInt KMaxEmailItemFileNameLength = 256;
_LIT(KEmailItemFile, "c:\\system\\temp\\emailitem.txt");
_LIT(KEmailItemPath, "c:\\system\\temp\\");

const TInt KNSmlEmailItemDrive = EDriveC;

// Whitespace characters
const TText KWhitespaceEmpty = ' ';
const TText KWhitespaceLineFeed = '\r';
const TText KWhitespaceNewLine = '\n';
const TText KWhitespaceTabular = '\t';
const TText KWhitespaceLineFeedNewLine = '\r\n';

const TText KCharacterSlash = '/';

// CData
_LIT8(KCDataStart,		"<![CDATA[");
_LIT8(KCDataEnd,		"]]>");
_LIT8(KCDataInnerEnd,	"]]]]>&gt;<![CDATA[");

#ifndef __NO_XML_COMMENTS_
// Xml comments
_LIT8(KCommentStart,	"<!--");
_LIT8(KCommentEnd,		"-->");
#endif

const TText KElementStart = '<';
const TText KElementEnd = '>';

// Xml entities
_LIT8(KEntityLT,	"&lt;");
_LIT8(KEntityGT,	"&gt;");
_LIT8(KEntityAMP,	"&amp;");
_LIT8(KEntityAPOS,	"&apos;");
_LIT8(KEntityQUOT,	"&quot;");

// special characters
_LIT8(KLessThan,	"<");
_LIT8(KGreaterThan,	">");
_LIT8(KAmpersand,	"&");
_LIT8(KApostrophe,	"\'");
_LIT8(KQuotation,	"\"");

// boolean string representations
_LIT8(KStringTrue,	"true");
_LIT8(KStringFalse,	"false");

// empty string
_LIT8(KStringEmpty,	"");

// email spesific elements
_LIT8(KEmailElement,			"Email");
_LIT8(KEmailReadElement,		"read");
_LIT8(KEmailForwardedElement,	"forwarded");
_LIT8(KEmailRepliedElement,		"replied");
_LIT8(KEmailReceivedElement,	"received");
_LIT8(KEmailCreatedElement,		"created");
_LIT8(KEmailModifiedElement,	"modified");
_LIT8(KEmailDeletedElement,		"deleted");
_LIT8(KEmailFlaggedElement,		"flagged");
_LIT8(KEmailItemElement,		"emailitem");

// truncated info spesific elements
_LIT8(KTruncatedElement,		"truncated");
_LIT8(KTruncatedBodyElement,	"body");
_LIT8(KTruncatedAttachElement,	"attach");
_LIT8(KTruncatedSizeElement,	"size");
_LIT8(KTruncatedTypeElement,	"type");
_LIT8(KTruncatedNameElement,	"name");

// extension spesific elements
_LIT8(KExtElement,		"Ext");
_LIT8(KExtXNamElement,	"XNam");
_LIT8(KExtXValElement,	"XVal");

// folder spesific elements
_LIT8(KFolderElement,			"Folder");
_LIT8(KFolderNameElement,		"name");
_LIT8(KFolderCreatedElement,	"created");
_LIT8(KFolderModifiedElement,	"modified");
_LIT8(KFolderAccessedElement,	"accessed");
_LIT8(KFolderAttributesElement,	"attributes");
_LIT8(KFolderRoleElement,		"role");

// folder attributes
_LIT8(KAttributeHiddenElement,		"h");
_LIT8(KAttributeSystemElement,		"s");
_LIT8(KAttributeArchivedElement,	"a");
_LIT8(KAttributeDeleteElement,		"d");
_LIT8(KAttributeWritableElement,	"w");
_LIT8(KAttributeReadableElement,	"r");
_LIT8(KAttributeExecutableElement,	"x");


// The error values returned from parser and generator functions
enum TNSmlParserGeneratorError
	{
	EErrorNone,				// no error
	EOutOfMemory,			// out of memory
	EMandatoryFieldNotFound,// XNam inside Ext missing
	EInvalidBooleanValue,	// Invalid value inside boolean elements
	EInvalidIntegerValue,	// Invalid value inside integer elements
	EInvalidDatetimeValue,	// Invalid value inside datetime elements
	EInvalidCDataStructure,	// CDATA error
	EInvalidXmlError, 		// Xml includes invalid data, i.e. xml generally against dtd
	EInvalidFilename,		// file was not found or the filename was otherwise erroneous
	EUnknownError			// Unknown error
	};


// enumeration for boolean values
enum TNSmlBoolean
	{
	EBooleanMissing = 0, // not included in the xml
	EBooleanTrue,
	EBooleanFalse
	};


// extension data (found in both email and folder)
class CNSmlExtData : public CBase
	{
public:
	IMPORT_C static CNSmlExtData* NewL();
	IMPORT_C static CNSmlExtData* NewLC();
	IMPORT_C virtual ~CNSmlExtData();

	IMPORT_C void AddXValL( HBufC8* aXVal );

	IMPORT_C void GenerateXmlL( TPtr8& aXml, const CNSmlXmlParser* aParser ) const;
	IMPORT_C TInt CountXmlSize( const CNSmlXmlParser* aParser ) const;
	IMPORT_C void ConvertIntoEntitiesL( const CNSmlXmlParser* aParser );

private:
	CNSmlExtData();
	void ConstructL();

public:
	HBufC8* iXNam;
	CNSmlXValArray* iXVals;
	};


// folder attributes
struct TNSmlFolderAttributeData
	{
	IMPORT_C TNSmlFolderAttributeData();

	/*
	* Returns the count of attributes that are not EBooleanMissing.
	*/
	IMPORT_C TInt AttributeCount() const;

	IMPORT_C void GenerateXml( TPtr8& aXml, const CNSmlXmlParser* aParser ) const;
	IMPORT_C TInt CountXmlSize( const CNSmlXmlParser* aParser ) const;

	TNSmlBoolean iHidden;
	TNSmlBoolean iSystem;
	TNSmlBoolean iArchived;
	TNSmlBoolean iDelete;
	TNSmlBoolean iWritable;
	TNSmlBoolean iReadable;
	TNSmlBoolean iExecutable;
	};


// truncate info data
class CNSmlTruncatedData : public CBase
	{
public:
	IMPORT_C static CNSmlTruncatedData* NewL();
	IMPORT_C static CNSmlTruncatedData* NewLC();
	IMPORT_C virtual ~CNSmlTruncatedData();

	IMPORT_C void AddAttachmentL(HBufC8* aName, TInt aSize, HBufC8* aType);
	IMPORT_C void AddBodyL(TInt aSize, HBufC8* aType);

	IMPORT_C void GenerateXmlL( TPtr8& aXml, const CNSmlXmlParser* aParser ) const;
	IMPORT_C TInt CountXmlSizeL( const CNSmlXmlParser* aParser ) const;
	IMPORT_C void ConvertIntoEntitiesL( const CNSmlXmlParser* aParser );

private:
	CNSmlTruncatedData();
	void ConstructL();

public:
	RNSmlBodyArray iBodys;
	RNSmlAttachArray iAttachments;
	};


// Base class for actual parsergenerators. 
// Implements basic utilities for parsing and generating the xml.
class CNSmlXmlParser : public CBase  
	{
public:
	/*
	* The constructor.
	*/
	IMPORT_C CNSmlXmlParser();

	/*
	* The destructor.
	*/
	IMPORT_C virtual ~CNSmlXmlParser();

	/*
	* Parses through the given xml and places the data it contains to 
	* member variables.	Removes all the comments from the original string.
	* @param aXml The xml to be parsed.
	* @return 
	*/
	virtual TNSmlParserGeneratorError ParseXml( HBufC8* aXml ) =0;

	/*
	* Generates xml using the data in member variables of the child class. 
	* aXml contains the generated xml when method returns successfully. 
	* Caller should not instantiate 
	* the buffer, since this method counts the size of the xml and 
	* instantiates the buffer using the size as its maximum size.
	* Caller gets the control of the buffer when method returns.
	* @param aXml A pointer to a buffer, which is instantiated in this 
	* method and contains the xml when method returns succesfully.
	* @return 
	*/
	virtual TNSmlParserGeneratorError GenerateXml( HBufC8*& aXml ) =0;

	/*
	* Counts the size of the generated xml, when the xml would be generated 
	* using the data in child class' member variables.
	*/
	virtual TInt CountXmlSizeL() =0;

	/* utility functions */

	void PreProcessL( HBufC8* aXml ) const;

	TPtrC8 BooleanToString( const TNSmlBoolean aValue ) const;
	TBuf8<KDateTimeLength> DateTimeToStringL( const TTime& aValue ) const;
	TBuf8<KIntegerMaxLength> IntegerToString( const TInt aValue ) const;

	TNSmlBoolean StringToBooleanL( const TPtrC8& aValue ) const;
	TInt StringToIntegerL( const TPtrC8& aValue ) const;
	// Note: Ignores UTC-times (Z-ending) and treats them as local time!
	TTime StringToTTimeL( TPtrC8& aValue ) const;

	void AppendElement( TPtr8& aPtr, const TDesC8& aElementName, const TDesC8& aValue ) const;
	void AppendElement( TPtr8& aPtr, const TDesC8& aElementName ) const;
	void AppendEndElement( TPtr8& aPtr, const TDesC8& aElementName ) const;

	TInt SizeOfElements( const TDesC8& aElementName ) const;
	TInt SizeOfBoolean( const TNSmlBoolean aValue, const TDesC8& aElementName ) const;
	TInt SizeOfDatetime( const TDesC8& aElementName ) const;
	TInt SizeOfInteger( const TDesC8& aElementName ) const;
	TInt SizeOfString( const HBufC8* aValue, const TDesC8& aElementName ) const;

	TInt EntitiesToCharactersL( HBufC8*& aXml, TInt aStartPos, TInt aEndPos ) const;
	TInt CharactersToEntitiesL( HBufC8*& aXml, TInt aStartPos, TInt aEndPos ) const;

	TBool IsWhitespace( const TDesC8& aText ) const;
	void LeaveIfNotWhiteSpaceL( const TDesC8& aText ) const;

	TNSmlParserGeneratorError CheckError( const TInt error ) const;

protected:
	// parsing methods
	void ParseL( TPtrC8& aXml );
	void AddToCompleteL( const TPtrC8 aStr );
	void ResetBufferL( HBufC8*& aBuf ) const;
	void AddToBufferL( const TText c, HBufC8*& aBuf ) const;

	virtual void NextElementL( TPtrC8 aElement ) =0;
	virtual void NextDataL( TPtrC8 aData ) =0;

private:
	TInt ReplaceL( HBufC8*& aText, const TDesC8& aTarget, const TDesC8& aItem, TInt aStartPos, TInt aEndPos ) const;
	void CheckDatetimeErrorL( const TInt error ) const;

protected:
	HBufC8* iBuffer;
	HBufC8* iCompleteBuffer;
	};



// Email parser. Implements the parsing and generating of Email xml.
class CNSmlEmailParser : public CNSmlXmlParser
	{
public:
	IMPORT_C static CNSmlEmailParser* NewL();
	IMPORT_C static CNSmlEmailParser* NewLC();
	IMPORT_C virtual ~CNSmlEmailParser();

	IMPORT_C virtual TNSmlParserGeneratorError ParseXml( HBufC8* aXml );
	IMPORT_C virtual TNSmlParserGeneratorError GenerateXml( HBufC8*& aXml );
	IMPORT_C virtual TInt CountXmlSizeL();

	IMPORT_C void AddExtL( CNSmlExtData* aExt );

protected:
	virtual void NextElementL( TPtrC8 aElement );
	virtual void NextDataL( TPtrC8 aData );

	virtual void HandleNextEmailItemDataL( TPtrC8 aData );
	virtual void HandleNextEmailItemStartL() {}; // nothing to do
	virtual void HandleNextEmailItemEndL() {}; // nothing to do

	void ConvertIntoEntitiesL();

	CNSmlEmailParser();
	void ConstructL();

protected:
	
	// the email elements (states) in the order they appear in DTD
	enum TNSmlCurrentEmailElement
		{
		ENone = 0,
		EEmail,
		ERead, 
		EForwarded,
		EReplied,
		EReceived,
		ECreated,
		EModified,
		EDeleted,
		EFlagged,
		EEmailItem,
		ETrunc,
		ETruncBody,
		ETruncBodySize,
		ETruncBodyType,
		ETruncAttach,
		ETruncAttachName,
		ETruncAttachSize,
		ETruncAttachType, 
		EExt,
		EExtXNam,
		EExtXVal
		};
	
	// struct used in email parsing to know which members are already set
	struct TNSmlSetEmailValues
		{
		TNSmlSetEmailValues();
		void Reset();
		
		TBool iEmail;
		TBool iRead;
		TBool iForwarded;
		TBool iReplied;
		TBool iDeleted;
		TBool iFlagged;
		TBool iReceived;
		TBool iCreated;
		TBool iModified;
		TBool iEmailItem;
		TBool iTrunc;
		TBool iTruncBodySize;
		TBool iTruncBodyType;
		TBool iTruncAttachSize;
		TBool iTruncAttachType;
		TBool iTruncAttachName;
		TBool iXNam;
		};
	
private:
	void StartElementStateChangeL( TNSmlCurrentEmailElement aCurrentState, TNSmlCurrentEmailElement aNextState, TBool aIsSet = EFalse );
	void EndElementStateChangeL( TNSmlCurrentEmailElement aCurrentState, TNSmlCurrentEmailElement aNextState );
	
	void GenerateEmailXmlL( HBufC8*& aXml );

public:

	/*
	* The values of the data below is treated as follows:
	* When constructing this data from an xml, boolean values 
	* are EBooleanTrue, when <field>true</field>, EBooleanFalse 
	* when <field>false</field> OR <field/>, and EBooleanMissing 
	* when the field is not present in the xml. Datetime values 
	* are equal to Time::NullTTime when the field is missing (they should 
	* never be empty). Text values (HBufC8*) are NULL when the field is 
	* missing, an empty string when <field></field> OR <field/> and the 
	* actual value between elements if there is data between those 
	* elements. iExt is never NULL, but when iExt->Count() 
	* equals to zero, there are no extensions present in the xml.
	*
	* These same rules are used when generating xml from 
	* this data object.
	*/

	TNSmlBoolean iRead;
	TNSmlBoolean iForwarded;
	TNSmlBoolean iReplied;
	TTime iReceived;
	TTime iCreated;
	TTime iModified;
	HBufC8* iEmailItem;
	TNSmlBoolean iDeleted;
	TNSmlBoolean iFlagged;
	CNSmlTruncatedData* iTruncated;
	CNSmlExtDataArray* iExt;

protected:
	TNSmlSetEmailValues iSetValues;
	TNSmlCurrentEmailElement iCurrentState;
	TNSmlCurrentEmailElement iLastState;

private:

	TInt iTruncSize;
	HBufC8* iTruncType;
	HBufC8* iTruncName;
	CNSmlExtData* iExtData;
	};



// ------------------------------------------------------------------------------------------------
// Parses and generates the xml using files/streams. If emailitem is present, it is 
// stored in a file.
// ------------------------------------------------------------------------------------------------
class CNSmlEmailFileParser : public CNSmlEmailParser
	{
public:
	IMPORT_C static CNSmlEmailFileParser* NewL();
	IMPORT_C static CNSmlEmailFileParser* NewLC();
	IMPORT_C virtual ~CNSmlEmailFileParser();

	IMPORT_C virtual TNSmlParserGeneratorError ParseXml( HBufC8* aXml );
	IMPORT_C virtual TNSmlParserGeneratorError ParseXml( RReadStream& aRs );
	IMPORT_C virtual TNSmlParserGeneratorError GenerateXml( HBufC8*& aXml );
	IMPORT_C virtual TNSmlParserGeneratorError GenerateXml( RWriteStream& aWs );
	IMPORT_C virtual TInt CountXmlSizeL();

protected:
	virtual void HandleNextEmailItemDataL( TPtrC8 aData );
	virtual void HandleNextEmailItemStartL();
	virtual void HandleNextEmailItemEndL();

private:
	void ParseFromStreamL( RReadStream& rs );
	void ParsePieceL( TPtrC8& aXml, TBool aLastPiece = EFalse );
	TUint RemoveIncompleteL( HBufC8* aPiece, RReadStream& aStream );
	TUint CheckPiece( const TPtrC8 aEnd ) const;
	void PreProcessPieceL( HBufC8* aXml );

	void GenerateEmailXmlL( RWriteStream& aWs );

	void ResetParserL();
	void ResetPreProcessor();

	HBufC* Buf8ToBuf16( const HBufC8* buf8 ) const;

	CNSmlEmailFileParser();
	void ConstructL();

public:

	TBool iUseEmailItem; // whether or not emailitem should be used in generation

private:

	// xml parsing state
	TBool iReadingElementName;
	TBool iReadingCData;
#ifndef __NO_XML_COMMENTS_
	TBool iReadingComment;
#endif
	TBool iReadingEmailItemData;

	// pre-processing state
	TInt iCdataStartPos;
	TInt iCdataEndPos;

	// file handling
	RFs iRfs;
	RFileWriteStream iWriteStream;
	
	HBufC* iEmailItemFile;
	TBool iFirstPiece;
	TBool iCdataFoundFromFirstPiece;
	};




// Folder parser. Implements parsing and generating of Folder xml.
class CNSmlFolderParser : public CNSmlXmlParser
	{
public:
	IMPORT_C static CNSmlFolderParser* NewL();
	IMPORT_C static CNSmlFolderParser* NewLC();
	IMPORT_C virtual ~CNSmlFolderParser();

	IMPORT_C virtual TNSmlParserGeneratorError ParseXml( HBufC8* aXml );
	IMPORT_C virtual TNSmlParserGeneratorError GenerateXml( HBufC8*& aXml );
	IMPORT_C virtual TInt CountXmlSizeL() ;

	IMPORT_C void AddExtL( CNSmlExtData* aExt );

protected:
	virtual void NextElementL( TPtrC8 aElement );
	virtual void NextDataL( TPtrC8 aData );

protected:
	
	// the folder elements (states) in the order they appear in DTD
	enum TNSmlCurrentFolderElement
		{
		EFolderNone = 0,
		EFolder,
		EFolderName, 
		EFolderCreated,
		EFolderModified,
		EFolderAccessed,
		EAttributes,
		EAttributesH,
		EAttributesS,
		EAttributesA,
		EAttributesD,
		EAttributesW,
		EAttributesR,
		EAttributesX,
		EFolderRole,
		EFolderExt,
		EFolderExtXNam,
		EFolderExtXVal
		};
	
	// struct used in folder parsing (which members are already set)
	struct TNSmlSetFolderValues
		{
		TNSmlSetFolderValues();
		void Reset();
		
		TBool iFolder;
		TBool iName;
		TBool iCreated;
		TBool iModified;
		TBool iAccessed;
		TBool iAttributes;
		TBool iAttributesH;
		TBool iAttributesS;
		TBool iAttributesA;
		TBool iAttributesD;
		TBool iAttributesW;
		TBool iAttributesR;
		TBool iAttributesX;
		TBool iRole;
		TBool iXNam;
		};
	
private:
	void StartElementStateChangeL( TNSmlCurrentFolderElement aCurrentState, TNSmlCurrentFolderElement aNextState, TBool aIsSet = EFalse );
	void EndElementStateChangeL( TNSmlCurrentFolderElement aCurrentState, TNSmlCurrentFolderElement aNextState );

	void GenerateFolderXmlL( HBufC8*& aXml );
	void ConvertIntoEntitiesL();

	CNSmlFolderParser();
	void ConstructL();

public:
	HBufC8* iName;
	TTime iCreated;
	TTime iModified;
	TTime iAccessed;
	TNSmlFolderAttributeData iAttributes;
	HBufC8* iRole;
	CNSmlExtDataArray* iExt;

private:
	TNSmlCurrentFolderElement iCurrentState;
	TNSmlCurrentFolderElement iLastState;
	TNSmlSetFolderValues iSetValues;

	CNSmlExtData* iExtData;
	};



#endif // __NSMLXMLPARSER_H__
