#
# Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
# All rights reserved.
# This component and the accompanying materials are made available
# under the terms of "Eclipse Public License v1.0"
# which accompanies this distribution, and is available
# at the URL "http://www.eclipse.org/legal/epl-v10.html".
#
# Initial Contributors:
# Nokia Corporation - initial contribution.
#
# Contributors:
#
# Description:  Project file
#


TEMPLATE = lib
TARGET = nsmlcontactsdatastoreextension
DEPENDPATH += ../../contactsdatastoreextension ../inc ../src
INCLUDEPATH += .
INCLUDEPATH += $$APP_LAYER_SYSTEMINCLUDE



DEFINES += BUILD_CONTACTSDATASTOREEXTENSION

PRIVATE_HEADERS += ./inc/nsmlcontactsdatastoreextension_p.h

# Implementation
SOURCES += ./src/nsmlcontactsdatastoreextension.cpp \
		   ./src/nsmlcontactsdatastoreextension_p.cpp
		   
HEADERS += \
       $$PRIVATE_HEADERS

qtAddLibrary(QtContacts)
qtAddLibrary(QtVersit)
qtAddLibrary(XQUtils)
LIBS += -lestor.dll \
		-lbafl.dll  \
		-lnsmldebug.dll \
		
		
IfdefBlock = \
 "$${LITERAL_HASH}if defined(ARMCC)" \
 "DEFFILE ./def/eabicontactsdatastoreextension" \
 "$${LITERAL_HASH}elif defined(WINSCW)" \
 "DEFFILE ./def/bwinscwcontactsdatastoreextension" \
 "$${LITERAL_HASH}endif"
 
MMP_RULES += IfdefBlock

symbian { 
	TARGET.UID3 = 0x2002DC83
	TARGET.EPOCSTACKSIZE = 0x14000
	TARGET.EPOCHEAPSIZE = "0x020000 0x800000"
	TARGET.EPOCALLOWDLLDATA = 1
    TARGET.CAPABILITY = CAP_GENERAL_DLL    
}