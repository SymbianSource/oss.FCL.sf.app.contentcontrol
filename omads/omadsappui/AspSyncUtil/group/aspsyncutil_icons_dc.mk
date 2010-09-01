#
# Copyright (c) 2004 Nokia Corporation and/or its subsidiary(-ies).
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
# Description: 
#

ifeq (WINS,$(findstring WINS, $(PLATFORM)))
ZDIR=\epoc32\release\$(PLATFORM)\$(CFG)\Z
else
ZDIR=\epoc32\data\z
endif

# ----------------------------------------------------------------------------
#
# ----------------------------------------------------------------------------

TARGETDIR=$(ZDIR)\resource\apps
ICONTARGETFILENAME=$(TARGETDIR)\aspsyncutil.mif

HEADERDIR=\epoc32\include
HEADERFILENAME=$(HEADERDIR)\aspsyncutil.mbg

do_nothing :
	@rem do_nothing

MAKMAKE : do_nothing

BLD : do_nothing

CLEAN : do_nothing

LIB : do_nothing

CLEANLIB : do_nothing

# ----------------------------------------------------------------------------
#
#
# NOTE 1: DO NOT DEFINE MASK FILE NAMES! They are included automatically by
# MifConv if the mask detph is defined.
#
# NOTE 2: Usually, source paths should not be included in the bitmap
# definitions. MifConv searches for the icons in all icon directories in a
# predefined order, which is currently \s60\icons, \s60\bitmaps2, \s60\bitmaps.
# The directory \s60\icons is included in the search only if the feature flag
# __SCALABLE_ICONS is defined.
# ----------------------------------------------------------------------------

RESOURCE :
	mifconv $(ICONTARGETFILENAME) /h$(HEADERFILENAME) \
	/c8,8 qgn_prop_sml_http.bmp \
	/c8,8 qgn_prop_sml_http_off.bmp \
	/c8,8 qgn_prop_sml_bt.bmp \
	/c8,8 qgn_prop_sml_bt_off.bmp \
	/c8,8 qgn_prop_sml_usb.bmp \
	/c8,8 qgn_prop_sml_usb_off.bmp \
	/c8,8 qgn_prop_sml_ir.bmp \
	/c8,8 qgn_prop_sml_ir_off.bmp \
	/c8,8 qgn_menu_sml_cxt.bmp \
	/c8,8 qgn_note_sml.bmp \
	/c8,8 qgn_note_sml_server.bmp \
	/c8,8 qgn_prop_sml_sync_server.bmp \
	/c8,8 qgn_prop_sml_sync_to_server.bmp \
	/c8,8 qgn_prop_sml_sync_from_server.bmp \
	/c8,8 qgn_prop_sml_sync_off.bmp \
	/c8,8 qgn_indi_sync_set_add.bmp \
	/c8,8 qgn_indi_sett_protected_add.bmp \
	/c8,8 qgn_prop_sml_new.bmp\

FREEZE : do_nothing

SAVESPACE : do_nothing

RELEASABLES :
	@echo $(HEADERFILENAME)&& \
	@echo $(ICONTARGETFILENAME)

FINAL : do_nothing
