/*
* Copyright (c) 2005-2010 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  CSConAppInstaller implementation
*
*/


// INCLUDE FILES
#include <s32mem.h> // For RBufWriteStream
#include <utf.h>  // for CnvUtfConverter
#include <usif/usifcommon.h>
#include <usif/scr/scr.h>
#include <driveinfo.h>

using namespace Usif;

#include "debug.h"
#include "sconinstaller.h"
#include "sconpcdconsts.h"

const TInt KSConSeConUidValue = 0x101f99f6;
const TUid KSConSeConUid = {KSConSeConUidValue};

_LIT8(KValSep, "=");
_LIT8(KComma, ",");
_LIT8(KLineEnd, "\r\n");

// ============================= MEMBER FUNCTIONS ===============================

CSConAppInstaller* CSConAppInstaller::NewL( CSConInstallerQueue* aQueue, RFs& aFs )
    {
    CSConAppInstaller* self = new (ELeave) CSConAppInstaller( aQueue, aFs );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }
// -----------------------------------------------------------------------------
// CSConAppInstaller::CSConAppInstaller( CSConInstallerQueue* aQueue )
// Constructor
// -----------------------------------------------------------------------------
//
CSConAppInstaller::CSConAppInstaller( CSConInstallerQueue* aQueue, RFs& aFs ) :
    CActive( EPriorityStandard ), iQueue( aQueue ), iFs( aFs )
    {
    CActiveScheduler::Add( this );
    }

void CSConAppInstaller::ConstructL()
    {
    iSifOptions = COpaqueNamedParams::NewL();
    iSifResults = COpaqueNamedParams::NewL();
    }

// -----------------------------------------------------------------------------
// CSConAppInstaller::~CSConAppInstaller()
// Destructor
// -----------------------------------------------------------------------------
//
CSConAppInstaller::~CSConAppInstaller()
    {
    TRACE_FUNC;
    Cancel();
    iSwInstaller.Close();
    delete iSifOptions;
    delete iSifResults;
    }

// -----------------------------------------------------------------------------
// CSConAppInstaller::StartInstaller( TInt& aTaskId )
// Starts the installer task
// -----------------------------------------------------------------------------
//
void CSConAppInstaller::StartInstaller( TInt& aTaskId )
    {
    TRACE_FUNC_ENTRY;
    CSConTask* task = NULL;
    TRequestStatus* status = NULL;
    TInt err( KErrNone );
    
    TInt ret = iQueue->GetTask( aTaskId, task );
    
    if( aTaskId > 0 && ret != KErrNotFound )
        {
        if ( IsActive() )
            {
            LOGGER_WRITE("WARNING! SConAppInstaller was not on idle state!");
            iQueue->CompleteTask( aTaskId, KErrInUse );
            TRACE_FUNC_EXIT;
            return;
            }
        
        
        iCurrentTask = aTaskId;
        iQueue->SetTaskProgress( aTaskId, KSConCodeProcessingStarted );
        
        switch( task->GetServiceId() )
            {
            case EInstall :
                iQueue->ChangeQueueProcessStatus();
                err = iSwInstaller.Connect();
                if( err == KErrNone )
                    {
                    TRAP(err, ProcessInstallL( *task->iInstallParams ));
                    if( err != KErrNone )
                        {
                        LOGGER_WRITE_1( "StartInstaller ProcessInstallL err: %d", err );
                        iStatus = KRequestPending;
                        SetActive();
                        status = &iStatus;
                        User::RequestComplete( status, err );
                        }
                    }
                
                break;
            case EUninstall :
                iQueue->ChangeQueueProcessStatus();
                err = iSwInstaller.Connect();
                if( err == KErrNone )
                    {
                    LOGGER_WRITE( "Begin to uninstall.. " );
                    
                    TRAP( err, ProcessUninstallL( *task->iUninstallParams ) );
                    if( err != KErrNone )
                        {
                        LOGGER_WRITE_1( "StartInstaller ProcessUninstallL err: %d", err );
                        iStatus = KRequestPending;
                        SetActive();
                        status = &iStatus;
                        User::RequestComplete( status, err );
                        }
                    }
                
                break;
            case EListInstalledApps :
                iQueue->ChangeQueueProcessStatus();
                iStatus = KRequestPending;
                SetActive();
                TRAP( err, ProcessListInstalledAppsL() );
                status = &iStatus;
                User::RequestComplete( status, err );
                break;
            default :
                break;
            }
        }
    TRACE_FUNC_EXIT;
    }

// -----------------------------------------------------------------------------
// CSConAppInstaller::StopInstaller( TInt& aTaskId )
// Stops the installer task
// -----------------------------------------------------------------------------
//
void CSConAppInstaller::StopInstaller( TInt& aTaskId )
    {
    TRACE_FUNC_ENTRY;
    //If the task is the current task, cancel it first
    if( iCurrentTask == aTaskId )
        {
        Cancel();
        iSwInstaller.Close();
        }
    TRACE_FUNC_EXIT;
    }

// -----------------------------------------------------------------------------
// TBool CSConAppInstaller::InstallerActive()
// returns installer activity status
// -----------------------------------------------------------------------------
//
TBool CSConAppInstaller::InstallerActive() const
    {
    return IsActive();
    }

// -----------------------------------------------------------------------------
// CSConAppInstaller::DoCancel()
// Implementation of CActive::DoCancel()
// -----------------------------------------------------------------------------
//
void CSConAppInstaller::DoCancel()
    {
    TRACE_FUNC_ENTRY;
    LOGGER_WRITE("Cancel iSwInstaller");
    iSwInstaller.CancelOperation();
    TRACE_FUNC_EXIT;
    }

// -----------------------------------------------------------------------------
// CSConAppInstaller::RunL()
// Implementation of CActive::RunL()
// -----------------------------------------------------------------------------
//
void CSConAppInstaller::RunL()
    {
    TRACE_FUNC_ENTRY;
    iSwInstaller.Close();
    iQueue->ChangeQueueProcessStatus();
    TInt err( iStatus.Int() );
    LOGGER_WRITE_1( "CSConAppInstaller::RunL() iStatus.Int() : returned %d", err );
    
    CSConTask* task = NULL;
    TInt taskErr = iQueue->GetTask( iCurrentTask, task );
    if( taskErr == KErrNone )
        {
        if ( task->GetServiceId() == EInstall || task->GetServiceId() == EUninstall )
            {
            TRAPD(dataErr, WriteTaskDataL( *task ));
            if (dataErr)
                {
                LOGGER_WRITE_1("WriteTaskDataL err: %d", dataErr);
                }
            }
        
        if( task->GetServiceId() == EInstall && err == KErrNone )
            {
            LOGGER_WRITE( "CSConAppInstaller::RunL() : before DeleteFile" );
            //delete sis after succesfull install
            iFs.Delete( task->iInstallParams->iPath );
            }
        }
    
    iQueue->CompleteTask( iCurrentTask, err );
    TRACE_FUNC_EXIT;
    }

// -----------------------------------------------------------------------------
// CSConAppInstaller::WriteTaskDataL()
// Writes data to task
// -----------------------------------------------------------------------------
//
void CSConAppInstaller::WriteTaskDataL( CSConTask& aTask )
    {
    TRACE_FUNC_ENTRY;
    CBufFlat* buffer = CBufFlat::NewL(200);
    CleanupStack::PushL(buffer);
    RBufWriteStream stream( *buffer );
    CleanupClosePushL( stream );
    
    ExternalizeResultArrayIntValL( KSifOutParam_ComponentId , stream);
    ExternalizeResultIntValL( KSifOutParam_ErrCategory , stream);
    ExternalizeResultIntValL( KSifOutParam_ErrCode , stream);
    //ExternalizeResultIntValL( KSifOutParam_ExtendedErrCode , stream);
    ExternalizeResultStringValL( KSifOutParam_ErrMessage , stream);
    ExternalizeResultStringValL( KSifOutParam_ErrMessageDetails , stream);
    
    stream.CommitL();
    
    buffer->Compress();
    
    HBufC8* data = HBufC8::NewL( buffer->Size() );
    TPtr8 dataPtr = data->Des();
    buffer->Read( 0, dataPtr, buffer->Size() );
    
    if ( aTask.GetServiceId() == EInstall )
        {
        if ( aTask.iInstallParams->iData )
            {
            delete aTask.iInstallParams->iData;
            aTask.iInstallParams->iData = NULL;
            }
        aTask.iInstallParams->iData = data;
        data = NULL;
        }
    else if ( aTask.GetServiceId() == EUninstall )
        {
        if ( aTask.iUninstallParams->iData )
            {
            delete aTask.iUninstallParams->iData;
            aTask.iUninstallParams->iData = NULL;
            }
        aTask.iUninstallParams->iData = data;
        data = NULL;
        }
    else
        {
        delete data;
        data = NULL;
        }
    
    CleanupStack::PopAndDestroy( &stream );
    CleanupStack::PopAndDestroy( buffer );
    TRACE_FUNC_EXIT;
    }

void CSConAppInstaller::ExternalizeResultArrayIntValL( const TDesC& aName, RWriteStream& aStream )
    {
    TRACE_FUNC_ENTRY;
    RArray<TInt> valueArray;
    TRAPD(err, valueArray = iSifResults->IntArrayByNameL(aName));
    if ( !err && valueArray.Count() > 0 )
        {
        LOGGER_WRITE_1("count: %d", valueArray.Count());
        TBuf8<100> nameBuf;
        err = CnvUtfConverter::ConvertFromUnicodeToUtf8( nameBuf, aName );
        if (!err)
            {
            LOGGER_WRITE("2");
            aStream.WriteL( nameBuf, nameBuf.Length() );
            aStream.WriteL( KValSep, 1 );
            aStream.WriteInt32L( valueArray[0] );
            for (TInt i=1; i<valueArray.Count(); i++)
                {
                aStream.WriteL( KComma, 1 );
                aStream.WriteInt32L( valueArray[i] );
                }
            aStream.WriteL( KLineEnd, 2 );
            }
        }
    TRACE_FUNC_EXIT;
    }
// -----------------------------------------------------------------------------
// CSConAppInstaller::ExternalizeResultIntValL()
// Read integer value and write it to stream
// -----------------------------------------------------------------------------
//
void CSConAppInstaller::ExternalizeResultIntValL( const TDesC& aName, RWriteStream& aStream )
    {
    TRACE_FUNC_ENTRY;
    TInt value;
    TBool found(EFalse);
    found = iSifResults->GetIntByNameL(aName, value);

    if (found)
        {
        TBuf8<100> nameBuf;
        TInt err = CnvUtfConverter::ConvertFromUnicodeToUtf8( nameBuf, aName );
        if (!err)
            {
            aStream.WriteL( nameBuf, nameBuf.Length() );
            aStream.WriteL( KValSep, 1 );
            aStream.WriteInt32L( value );
            aStream.WriteL( KLineEnd, 2 );
            }
        }
    TRACE_FUNC_EXIT;
    }

// -----------------------------------------------------------------------------
// CSConAppInstaller::ExternalizeResultStringValL()
// Read string value and write it to stream
// -----------------------------------------------------------------------------
//
void CSConAppInstaller::ExternalizeResultStringValL( const TDesC& aName, RWriteStream& aStream )
    {
    TRACE_FUNC_ENTRY;
    const TDesC& strValue = iSifResults->StringByNameL( aName );
    if (strValue.Length() > 0)
        {
        TBuf8<100> nameBuf;
        TInt err = CnvUtfConverter::ConvertFromUnicodeToUtf8( nameBuf, aName );
        if (!err)
            {
            HBufC8* nameVal = CnvUtfConverter::ConvertFromUnicodeToUtf8L( strValue );
            CleanupStack::PushL( nameVal );
            aStream.WriteL( nameBuf, nameBuf.Length() );
            aStream.WriteL( KValSep, 1 );
            aStream.WriteL( nameVal->Des(), nameVal->Length() );
            aStream.WriteL( KLineEnd, 2 );
            CleanupStack::PopAndDestroy( nameVal );
            }
        }
    TRACE_FUNC_EXIT;
    }

// -----------------------------------------------------------------------------
// CSConAppInstaller::ProcessInstallL()
// Executes Install task
// -----------------------------------------------------------------------------
//
void CSConAppInstaller::ProcessInstallL( const CSConInstall& aInstallParams )
    {
    TRACE_FUNC_ENTRY;
    iSifOptions->Cleanup();
    iSifResults->Cleanup();
    
    if ( aInstallParams.iMode == ESilentInstall )
        {
        LOGGER_WRITE( "Begin silent installation.. " );
        
        iSifOptions->AddIntL( Usif::KSifInParam_InstallSilently, ETrue );
        iSifOptions->AddIntL( Usif::KSifInParam_PerformOCSP, Usif::ENotAllowed );   
        // Note if upgrade is allowed, see NeedsInstallingL function.
        iSifOptions->AddIntL( Usif::KSifInParam_AllowUpgrade, Usif::EAllowed );
        iSifOptions->AddIntL( Usif::KSifInParam_AllowUntrusted, Usif::ENotAllowed );
        iSifOptions->AddIntL( Usif::KSifInParam_GrantCapabilities, Usif::ENotAllowed ); 
        // Defined for the install.
        iSifOptions->AddIntL( Usif::KSifInParam_InstallOptionalItems, Usif::EAllowed );          
        iSifOptions->AddIntL( Usif::KSifInParam_IgnoreOCSPWarnings, Usif::EAllowed );            
        iSifOptions->AddIntL( Usif::KSifInParam_AllowAppShutdown, Usif::EAllowed );
        iSifOptions->AddIntL( Usif::KSifInParam_AllowDownload, Usif::EAllowed );
        iSifOptions->AddIntL( Usif::KSifInParam_AllowOverwrite, Usif::EAllowed );

        iSwInstaller.Install( aInstallParams.iPath, *iSifOptions,
                *iSifResults, iStatus, ETrue );
        }
    else
        {
        LOGGER_WRITE( "Begin to install.. " );
        
        iSwInstaller.Install( aInstallParams.iPath, *iSifOptions,
                *iSifResults, iStatus, ETrue );
        }
    SetActive();
    TRACE_FUNC_EXIT;
    }

// -----------------------------------------------------------------------------
// CSConAppInstaller::ProcessUninstallL( const CSConUninstall& aUninstallParams )
// Executes UnInstall task
// -----------------------------------------------------------------------------
//
void CSConAppInstaller::ProcessUninstallL( const CSConUninstall& aUninstallParams )
    {
    TRACE_FUNC_ENTRY;
    LOGGER_WRITE_1( "aUid: 0x%08x", aUninstallParams.iUid.iUid );
    LOGGER_WRITE_1( "aName: %S", &aUninstallParams.iName );
    LOGGER_WRITE_1( "aVendor: %S", &aUninstallParams.iVendor );
    LOGGER_WRITE_1( "aType: %d", aUninstallParams.iType );
    LOGGER_WRITE_1( "aMode: %d", aUninstallParams.iMode );
    
    if ( aUninstallParams.iUid == KSConSeConUid )
	    {
	    LOGGER_WRITE("Cannot uninstall itself, leave");
	    // cannot uninstall itself
	    User::Leave( KErrInUse ); //SwiUI::KSWInstErrFileInUse );
	    }
    TComponentId componentId = aUninstallParams.iUid.iUid;
    iSifOptions->Cleanup();
    iSifResults->Cleanup();
    
    if ( aUninstallParams.iMode == ESilentInstall)
        {
        iSifOptions->AddIntL( Usif::KSifInParam_InstallSilently, ETrue );
        iSwInstaller.Uninstall( componentId, *iSifOptions, *iSifResults, iStatus, ETrue );
        }
    else
        {
        iSwInstaller.Uninstall( componentId, *iSifOptions, *iSifResults, iStatus, ETrue );
        }
    SetActive();
    TRACE_FUNC_EXIT;
    }
    
//--------------------------------------------------------------------------------
//void CSConAppInstaller::ProcessListInstalledAppsL()
//--------------------------------------------------------------------------------
//
void CSConAppInstaller::ProcessListInstalledAppsL()
    {
    TRACE_FUNC_ENTRY;
    
    CSConTask* task = NULL;
    User::LeaveIfError( iQueue->GetTask( iCurrentTask, task ) );
    
    CSConListInstApps& listInstApps = *task->iListAppsParams;
    Usif::RSoftwareComponentRegistry scr;
    User::LeaveIfError( scr.Connect() );
    CleanupClosePushL( scr );
    
    RArray<Usif::TComponentId> componentList;
    CleanupClosePushL( componentList );
    Usif::CComponentFilter* filter = Usif::CComponentFilter::NewLC();
    
    if ( listInstApps.iAllApps )
        {
        TDriveList driveList;
        // Get all drives that are visible to the user.
        TInt driveCount;
        User::LeaveIfError( DriveInfo::GetUserVisibleDrives( iFs, driveList, driveCount ) );
        filter->SetInstalledDrivesL( driveList );
        }
    else
        {
        filter->SetInstalledDrivesL( listInstApps.iDriveList );
        }
            
    scr.GetComponentIdsL( componentList, filter);
    
    CleanupStack::PopAndDestroy( filter );
    LOGGER_WRITE_1("components found: %d", componentList.Count());
    for (int i=0; i<componentList.Count(); i++)
        {
        Usif::CComponentEntry* entry = Usif::CComponentEntry::NewLC();
        TBool found = scr.GetComponentL(componentList[i], *entry );
        if (!found)
            User::Leave(KErrNotFound);
        
        CSConInstApp* app = new (ELeave) CSConInstApp();
        CleanupStack::PushL( app );
        
        StrCopyL( entry->Name(), app->iName );
        StrCopyL( entry->Vendor(), app->iVendor );
        app->iSize = entry->ComponentSize();
        StrCopyL( entry->Version(), app->iVersion );
        app->iWidgetBundleId = entry->GlobalId().AllocL();
        
        app->iUid.iUid = componentList[i];
        
        if ( entry->SoftwareType().Match( Usif::KSoftwareTypeNative ) == 0 )
            {
            app->iType = ESisApplication;
            }
        else if ( entry->SoftwareType().Match( Usif::KSoftwareTypeJava ) == 0 )
            {
            app->iType = EJavaApplication;
            }
        else if ( entry->SoftwareType().Match( Usif::KSoftwareTypeWidget ) == 0 )
            {
            app->iType = EWidgetApplication;
            }
        
        User::LeaveIfError( listInstApps.iApps.Append( app ) );
        CleanupStack::Pop( app );
            
        CleanupStack::PopAndDestroy( entry );
        }
    CleanupStack::PopAndDestroy( &componentList );
    CleanupStack::PopAndDestroy( &scr );
    
    TRACE_FUNC_EXIT;
    }

void CSConAppInstaller::StrCopyL( const TDesC& aSrc, TDes& aDest )
    {
    if (aSrc.Size() > aDest.MaxSize())
        User::Leave(KErrTooBig);
    aDest.Copy( aSrc );
    }
// End of file
