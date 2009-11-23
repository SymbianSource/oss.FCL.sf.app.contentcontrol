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
* Description:  Contains general data definitions for AspSyncUtil.
*
*/


#ifndef ASPDEFINES_H
#define ASPDEFINES_H


//  INCLUDES
#include <e32base.h>
#include <AiwGenericParam.hrh>



// CONSTANTS

// resource files
_LIT( KResFileName, "z:aspsyncutil.rsc" );
_LIT( KBitmapFileName, "z:AspSyncUtil.mbm" );


_LIT(KHttpHeader, "http://");

// central repository uid for scheduled sync settings
const TUid KCRUidNSmlDSApp       = { 0x101F9A1D };

//unicode value for clock icon
const TUint KClockCharacter = 0xF815 ;

// consumer app uids
const TUid KUidAppSync       = { 0x101F6DE4 };
const TUid KUidAppPhonebook  = { 0x101F4CCE };
const TUid KUidAppCalendar   = { 0x10005901 };
const TUid KUidAppMessages   = { 0x100058C5 };
const TUid KUidAppNotepad    = { 0x10005907 };
const TUid KUidAppTest       = { 0x0995CDE9 };
const TUid KUidAppMailEditor = { 0x101F4CD6 };



// medium type uids
const TUid KUidNSmlMediumTypeInternet  = { 0x101F99F0 };
const TUid KUidNSmlMediumTypeBluetooth = { 0x101F99F1 };
const TUid KUidNSmlMediumTypeUSB       = { 0x101F99F2 };
const TUid KUidNSmlMediumTypeIrDA      = { 0x101F99F3 };


// medium type uids for convergence
//const TUid KUidNSmlMediumTypeInternet  = { 0x10009F76 };
//const TUid KUidNSmlMediumTypeBluetooth = { 0x10009F79 };
//const TUid KUidNSmlMediumTypeUSB       = { 0x10009FB5 };
//const TUid KUidNSmlMediumTypeIrDA      = { 0x10009F78 };
//const TUid KUidNSmlMediumTypeWSP       = { 0x10009F77 };
//const TUid KUidNSmlMediumTypeNSmlUSB   = { 0x101F9A04 };



// sync adapder uids
const TUid KUidNSmlAdapterCalendar  = { 0x101F6DDE };
const TUid KUidNSmlAdapterContact   = { 0x101F6DDD };
const TUid KUidNSmlAdapterEMail     = { 0x101F6DDF };
const TUid KUidNSmlAdapterNote      = { 0x101F8612 };
const TUid KUidNSmlAdapterSms       = { 0x10206B5C };
const TUid KUidNSmlAdapterMMS       = { 0x101FB0E9 };
const TUid KUidNSmlAdapterBookmarks = { 0x102751BA };

// convergence sync adapder uids
//const TUid KUidNSmlAdapterCalendar  = { 0x10009FD1 };
//const TUid KUidNSmlAdapterContact   = { 0x10009FD3 };


// sync application uid
const TUid KUidSmlSyncApp             = { 0x101F6DE4 };

// password editor constatnts
_LIT(KSecretEditorMask, "*");
const TInt KSecretEditorMaskLength = 4;

// string sizes
const TInt KBufSize = 128;
const TInt KBufSize255 = 255;
const TInt KBufSize16 = 16;
const TInt KBufSize64 = 64;
const TInt KBufSize32 = 32;

const TInt KDefaultArraySize = 10;


//
// text editor max sizes (from nsmlconstantdefs.h)
//
const TInt KAspMaxProfileNameLength         = 50;
const TInt KAspMaxUsernameLength            = 80;
const TInt KAspMaxPasswordLength            = 22;
const TInt KAspMaxLocalNameLength           = 255;
const TInt KAspMaxAdapterDisplayNameLength  = 50;
const TInt KAspMaxServerIdLength            = 150;
const TInt KAspMaxURILength                 = 144;
const TInt KAspMaxRemoteNameLength          = 125;

// max number of profiles
const TInt KMaxProfileCount = 50;

// setting dialog listbox update delay after db event
const TInt KListBoxUpdateDelay = 250;



// DATA TYPES

enum TAspApplicationIds
	{
	EApplicationIdSync = 0,
	EApplicationIdBookmarks = 1,//value not available in AiwGenericParam.hrh
	EApplicationIdContact = EGenericParamContactItem,    // 20
	EApplicationIdCalendar = EGenericParamCalendarItem,  // 21
	EApplicationIdEmail = EGenericParamMessageItemEMail, // 31
	EApplicationIdNote = EGenericParamNoteItem,          // 23
	EApplicationIdMms = EGenericParamMessageItemMMS,     // 32
	EApplicationIdSms = EGenericParamMessageItemSMS      // 30
	};

enum TASpSettingValue
	{
	EAspSettingDisabled = 0,
	EAspSettingEnabled = 1
	};

// NOTE: do not change these enums. They are also used in sync app
enum TAspDialogMode
	{
	EModeDialogWait = 3,
	EModeDialogNoWait = 4
	};

enum TDialogMode
	{
	EDialogModeEdit,
	EDialogModeEditMandatory,
	EDialogModeReadOnly,
	EDialogModeSettingEnforcement
	};

enum TAspMandatoryProfileData
	{
	EMandatoryOk,
	EMandatoryNoProfileName,
    EMandatoryNoHostAddress,
    EMandatoryIncorrectBearerType,
    EMandatoryIncorrectAccessPoint,

    EMandatoryNoContent,
	EMandatoryNoLocalDatabase,
	EMandatoryNoRemoteDatabase,
	EMandatoryNoSyncType
	};

enum TAspSettingItemVisibility
	{
	EVisibilityNormal,
	EVisibilityReadOnly,
	EVisibilityHidden
	};

enum TASpBearerType
	{
	EAspBearerInternet = 0,
	EAspBearerUsb = 1,
	EAspBearerBlueTooth = 2,
	EAspBearerIrda = 3,
	EAspBearerWsp = 4,
	EAspBearerSmlUsb = 5,
    EAspBearerLast = 6
	};



//
// NOTE: this enum must match to resource r_asp_connection_dialog_setting_items
//
enum TAspConnectionSettingItem
	{
	EAspProfileName = 0,
	EAspServerId = 1,
	EAspConnType = 2,
	EAspAccessPoint = 3,
	EAspHostAddress = 4,
	EAspPort = 5,
	EAspUsername = 6,
	EAspPassword = 7,
	EAspServerAlert = 8,
	EAspHttpsUsername = 9,
	EAspHttpsPassword = 10,
    EAspSyncDirection = 11,
	EAspProtocolVersion = 12,
	EAspSyncContent = 13,
	EAspSchedulingSetting = 14,
	EAspProfileSetting = 15
	};


enum TAspContentSettingItem
	{
	EAspLocalDatabase = 0,
	EAspRemoteDatabase = 1,
	EAspUseFilters = 2
	};

enum TAspAutoSyncSettingItem
	{
    EAspAutoSyncProfile = 0,
    EAspAutoSyncContents,
    EAspAutoSyncFrequency,
    EAspAutoSyncPeakSchedule,
    EAspAutoSyncOffPeakSchedule,
    EAspAutoSyncScheduleTime,
    EAspAutoSyncPeakStartTime,
    EAspAutoSyncPeakEndTime,
    EAspAutoSyncPeakDays,
    EAspAutoSyncRoamingStatus
	};


// NOTE: This enum must match with r_asp_sync_type
enum TAspSyncDirection
	{
	ESyncDirectionTwoWay,
	ESyncDirectionOneWayFromDevice,
	ESyncDirectionOneWayIntoDevice,
	ESyncDirectionRefreshFromServer
	};

// NOTE: This enum must match with r_asp_server_alert_type
enum TAspSASyncState
	{
	ESASyncStateEnable = 0,
	ESASyncStateConfirm = 1,
	ESASyncStateDisable = 2
	};



// NOTE: this enum must match with property array described in 
// SyncML_Sync_Agent_Client_API_Specification.doc.

// transport property setting ids
enum TAspTransportProperties
	{
	EPropertyIntenetAccessPoint = 0,
	EPropertyHttpUsed = 5,
	EPropertyHttpUserName = 6,
	EPropertyHttpPassword = 7
	};

/*
// transport property setting ids for convergence
enum TNSmlSyncTransportProperties
	{
	EPropertyIntenetAccessPoint = 3,
	//EPropertyHttpUsed = 0,
	EPropertyHttpUserName = 0,
	EPropertyHttpPassword = 1
	};
*/

enum TASpProtocolType
	{
	EAspProtocol_1_1 = 0,
	EAspProtocol_1_2 = 1
	};

// Roaming Settings
enum TRoamingSettings
    {
    ERoamingSettingAlwaysAsk = 0,
    ERoamingSettingAlwaysAllow = 1,
    ERoamingSettingNeverAllow = 2,
    ERoamingSettingBlock = 3
    };




enum TAdvanceSettingsItem
    {
    ERoamingSettings = 0
    };

#endif  // ASPDEFINES_H

// End of File
