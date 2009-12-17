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
* Description:  Folder parser constants
*
*/


#ifndef __NSMLPARSERCONSTANTS_H__
#define __NSMLPARSERCONSTANTS_H__

// ------------------------------------------------------------------------------------------------
// Defines for binary size optimization
// ------------------------------------------------------------------------------------------------
#define __NO_XML_COMMENTS_
//#define __NO_EMPTY_ELEMENTS_


// ------------------------------------------------------------------------------------------------
// constants
// ------------------------------------------------------------------------------------------------

// the length of element start (<) and end (>)
const TInt KElementStartEndWidth = 2;

// constants used with files
const TInt KReadDataLength = 512;
const TUint KFileReadAccess	= EFileRead|EFileShareAny;
const TUint KFileWriteAccess = EFileWrite|EFileShareExclusive;

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


#endif // __NSMLPARSERCONSTANTS_H__
