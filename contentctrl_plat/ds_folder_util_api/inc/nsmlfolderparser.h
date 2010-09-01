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
* Description:  Folder XML parser
*
*/


// 1.2 Changes: nsmlxmlparser module added


#ifndef __NSMLFOLDERPARSER_H__
#define __NSMLFOLDERPARSER_H__

// ------------------------------------------------------------------------------------------------
// Includes
// ------------------------------------------------------------------------------------------------
#include <e32base.h>
#include <s32strm.h>

// ------------------------------------------------------------------------------------------------
// Class forward declarations
// ------------------------------------------------------------------------------------------------
class CNSmlXmlParser;
class CNSmlExtData;


//Constants

// the maximum length of an integer in characters
const TInt KIntegerMaxLength = 12;

// the length of a datetime in characters
const TInt KDateTimeLength = 16;

// ------------------------------------------------------------------------------------------------
// Typedefs
// ------------------------------------------------------------------------------------------------
// the array used for xval-values
typedef CArrayPtrFlat<HBufC8> CNSmlXValArray;
// the array used for extension data
typedef CArrayPtrFlat<CNSmlExtData> CNSmlExtDataArray;

// enumeration for boolean values
enum TNSmlBoolean
    {
    EBooleanMissing = 0, // not included in the xml
    EBooleanTrue,
    EBooleanFalse
    };

// The error values returned from parser and generator functions
enum TNSmlParserGeneratorError
    {
    EErrorNone,             // no error
    EOutOfMemory,           // out of memory
    EMandatoryFieldNotFound,// XNam inside Ext missing
    EInvalidBooleanValue,   // Invalid value inside boolean elements
    EInvalidIntegerValue,   // Invalid value inside integer elements
    EInvalidDatetimeValue,  // Invalid value inside datetime elements
    EInvalidCDataStructure, // CDATA error
    EInvalidXmlError,       // Xml includes invalid data, i.e. xml generally against dtd
    EInvalidFilename,       // file was not found or the filename was otherwise erroneous
    EUnknownError           // Unknown error
    };

// extension data (found in and folder)
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



#endif // __NSMLFOLDERPARSER_H__
