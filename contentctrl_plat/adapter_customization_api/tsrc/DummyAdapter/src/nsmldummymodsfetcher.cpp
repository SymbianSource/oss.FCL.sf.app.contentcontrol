/*
* Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  DS Dummy Adapter ModsFetcher
*
*/

//  CLASS HEADER
#include "nsmldummymodsfetcher.h"


//  EXTERNAL INCLUDES
#include <e32base.h>
#include <s32strm.h>
#include <e32cmn.h>
#include <e32des16.h>
#include <CVPbkContactViewDefinition.h>
#include <CVPbkContactIdConverter.h>
#include <CVPbkContactFieldIterator.h>
#include <MVPbkContactStoreProperties.h>
#include <MVPbkStoreContactProperties.h>
#include <MVPbkViewContact.h>
#include <MVPbkFieldType.h>
#include <MVPbkContactFieldTextData.h>
#include <MVPbkStoreContact.h>
#include <MVPbkStoreContact2.h>
#include <MVPbkContactLink.h>
#include <vpbkeng.rsg>

// INTERNAL INCLUDES
#include "nsmldebug.h"
#include "nsmlsnapshotitem.h"
#include "nsmlchangefinder.h"
#include "nsmldummyutility.h"


// ------------------------------------------------------------------------------------------------
// CNSmlDummyModsFetcher::CNSmlDummyModsFetcher
// ------------------------------------------------------------------------------------------------
//
CNSmlDummyModsFetcher::CNSmlDummyModsFetcher(
    TBool& aSnapshotRegistered,CVPbkContactManager& aContactManager,
    MVPbkContactStore& aStore,TKeyArrayFix& aKey,
    CNSmlChangeFinder& aChangeFinder ) :
	CActive( EPriorityLow ),
	iSnapshotRegistered( aSnapshotRegistered ),
	iKey( aKey ),
	iChangeFinder( aChangeFinder ),
	iContactManager( aContactManager ),
	iStore( aStore ),
	iContactViewBase( NULL )
	{
	_DBG_FILE("CNSmlDummyModsFetcher::CNSmlDummyModsFetcher(): begin");
	CActiveScheduler::Add( this );
	_DBG_FILE("CNSmlDummyModsFetcher::CNSmlDummyModsFetcher(): end");
	}

// ------------------------------------------------------------------------------------------------
// CNSmlDummyModsFetcher::~CNSmlDummyModsFetcher
// ------------------------------------------------------------------------------------------------
//
CNSmlDummyModsFetcher::~CNSmlDummyModsFetcher()
	{
	_DBG_FILE("CNSmlDummyModsFetcher::~CNSmlDummyModsFetcher(): begin");

	Cancel();
	delete iSnapshot;
	delete iContactViewBase;

	_DBG_FILE("CNSmlDummyModsFetcher::~CNSmlDummyModsFetcher(): end");
	}

// ----------------------------------------------------------------------------
// CNSmlDummyModsFetcher::CancelRequest
// ----------------------------------------------------------------------------
void CNSmlDummyModsFetcher::CancelRequest()
     {
     _DBG_FILE("CNSmlDummyModsFetcher::CancelRequest(): begin");

     Cancel();

     _DBG_FILE("CNSmlDummyModsFetcher::CancelRequest(): end");
     }

// ------------------------------------------------------------------------------------------------
// CNSmlDummyModsFetcher::FetchModificationsL
// ------------------------------------------------------------------------------------------------
//
void CNSmlDummyModsFetcher::FetchModificationsL( TRequestStatus& aStatus )
	{
	_DBG_FILE("CNSmlDummyModsFetcher::FetchModificationsL(): begin");

	iCallerStatus = &aStatus;
	*iCallerStatus = KRequestPending;
	iContactCount = 0;
	if( !iSnapshot )
		{
		iSnapshot = new (ELeave) CArrayFixSeg< TNSmlSnapshotItem >( KNSmlSnapshotSize );
		}

	if( iContactViewBase )
	   {
	   delete iContactViewBase;
	   iContactViewBase = NULL;
	   }

	CreateViewL();

	_DBG_FILE("CNSmlDummyModsFetcher::FetchModificationsL(): end");
	}

// ------------------------------------------------------------------------------------------------
// CNSmlDummyModsFetcher::ConstructL
// ------------------------------------------------------------------------------------------------
//
void CNSmlDummyModsFetcher::ConstructL()
	{
	_DBG_FILE("CNSmlDummyModsFetcher::ConstructL(): begin");

	iSnapshot = new ( ELeave ) CArrayFixSeg< TNSmlSnapshotItem >( KNSmlSnapshotSmallSize );

	_DBG_FILE("CNSmlDummyModsFetcher::ConstructL(): end");
	}

// ------------------------------------------------------------------------------------------------
// CNSmlDummyModsFetcher::RunL
// ------------------------------------------------------------------------------------------------
//
void CNSmlDummyModsFetcher::RunL()
	{
	_DBG_FILE("CNSmlDummyModsFetcher::RunL(): begin");

	if ( !iSnapshotRegistered )
		{
		FetchNextContactL();
		}
#ifdef __NSML_MODULETEST__
	else
		{
		CActiveScheduler::Stop();
		}
#endif

	_DBG_FILE("CNSmlDummyModsFetcher::RunL(): end");
	}

// ------------------------------------------------------------------------------------------------
// CNSmlDummyModsFetcher::FetchNextContactL
// ------------------------------------------------------------------------------------------------
//
void CNSmlDummyModsFetcher::FetchNextContactL()
	{
	_DBG_FILE("CNSmlDummyModsFetcher::FetchNextContactL(): begin");
	if( iContactViewBase->ContactCountL() == 0 )
		{
		User::RequestComplete( iCallerStatus, KErrNone );
		return;
		}
	if ( !iSnapshotRegistered )
		{
		const MVPbkViewContact& contact = iContactViewBase->ContactAtL( iContactCount );

		MVPbkContactLink* contactLink = contact.CreateLinkLC();
		iContactManager.RetrieveContactL( *contactLink, *this );
		CleanupStack::PopAndDestroy();
		}

	_DBG_FILE("CNSmlDummyModsFetcher::FetchNextContactL(): end");
	}

// ------------------------------------------------------------------------------------------------
// CNSmlDummyModsFetcher::DoCancel
// ------------------------------------------------------------------------------------------------
//
void CNSmlDummyModsFetcher::DoCancel()
	{
	_DBG_FILE("CNSmlDummyModsFetcher::DoCancel(): begin");

	User::RequestComplete( iCallerStatus, KErrCancel );

	_DBG_FILE("CNSmlDummyModsFetcher::DoCancel(): end");
	}

// ----------------------------------------------------------------------------
// CNSmlDummyModsFetcher::IsConfidentialL
// ----------------------------------------------------------------------------
//
TBool CNSmlDummyModsFetcher::IsConfidentialL( MVPbkStoreContact& aItem )
    {
    _DBG_FILE("CNSmlDummyModsFetcher::IsConfidentialL(): begin");
    TBool ret( EFalse );

    //find X-CLASS field type
	const MVPbkFieldType* syncclass =
        iContactManager.FieldTypes().Find( R_VPBK_FIELD_TYPE_SYNCCLASS );
	CVPbkBaseContactFieldTypeIterator* itr =
        CVPbkBaseContactFieldTypeIterator::NewLC( *syncclass,
            aItem.Fields() );
	while ( itr->HasNext() )
        {
        const MVPbkBaseContactField* field = itr->Next();
        const MVPbkContactFieldTextData& data =
        	MVPbkContactFieldTextData::Cast( field->FieldData() );
        TPtrC ptr = data.Text();
        //compare the sync type
        if ( ptr.Compare( KNSmlContactSyncNoSync ) == 0 )
            {
            _DBG_FILE("CNSmlDummyModsFetcher::IsConfidentialL(): \
                       find confidential");
            ret = ETrue;
            }

        }
	CleanupStack::PopAndDestroy( itr );

    _DBG_FILE("CNSmlDummyModsFetcher::IsConfidentialL(): end");
    return ret;
    }


// ----------------------------------------------------------------------------
// CNSmlDummyModsFetcher::RunError
// ----------------------------------------------------------------------------
 TInt CNSmlDummyModsFetcher::RunError( TInt aError )
	{
	DBG_FILE_CODE(aError, _S8("CNSmlDummyModsFetcher::RunError() : error code received "));
	User::RequestComplete( iCallerStatus, aError );
	return KErrNone;
	}

// -----------------------------------------------------------------------------
// CNSmlDummyModsFetcher::CreateViewL()
// Create a contact view of the store which is opened
// -----------------------------------------------------------------------------

void CNSmlDummyModsFetcher::CreateViewL()
	{
	_DBG_FILE("CNSmlDummyModsFetcher::CreateViewL(): begin");

	CVPbkContactViewDefinition* viewDef = CVPbkContactViewDefinition::NewL();
	CleanupStack::PushL( viewDef );
	viewDef->SetType( EVPbkContactsView );

	viewDef->SetUriL( iStore.StoreProperties().Uri().UriDes() );

	 iContactViewBase = iContactManager.CreateContactViewLC(
				                         *this,
				                         *viewDef,
				                         iContactManager.FieldTypes()  );
	CleanupStack::Pop();
	CleanupStack::PopAndDestroy( viewDef );

	_DBG_FILE("CNSmlDummyModsFetcher::CreateViewL(): end");
	}
// ---------------------------------------------------------------------------
//  CNSmlDummyModsFetcher::UpdateSnapshotL
//  Add the fetched contacts id and timestamp
//  to the snapshot.
// ---------------------------------------------------------------------------
//

 void CNSmlDummyModsFetcher::UpdateSnapshotL( MVPbkStoreContact* aContact )
{
    _DBG_FILE("CNSmlDummyModsFetcher::UpdateSnapshotL(): begin");

	if( !IsConfidentialL( *aContact) )
		{
		CVPbkContactIdConverter* idConverter = CVPbkContactIdConverter::NewL( iStore );
		CleanupStack::PushL( idConverter );

		TInt32 id = idConverter->LinkToIdentifier(
			*iContactViewBase->ContactAtL( iContactCount).CreateLinkLC() );
		TNSmlSnapshotItem snap( id );

		MVPbkStoreContact2* tempContact =
		reinterpret_cast<MVPbkStoreContact2*> ( aContact->StoreContactExtension ( KMVPbkStoreContactExtension2Uid ) );

		MVPbkStoreContactProperties *contactProterties
									= tempContact ->PropertiesL();
		CleanupDeletePushL( contactProterties );
		snap.SetLastChangedDate( contactProterties->LastModifiedL() );
		CleanupStack::PopAndDestroy();

		iSnapshot->InsertIsqL( snap, iKey );
		iCurrentSnapshotCounter++;
		if ( iCurrentSnapshotCounter > KNSmlSnapshotCompressSize )
			{
			iSnapshot->Compress();
			iCurrentSnapshotCounter = 0;
			}
		CleanupStack::PopAndDestroy();
		CleanupStack::PopAndDestroy( idConverter );
		}

	iContactCount++;

	if ( iContactCount == iContactViewBase->ContactCountL() )
		{
		iChangeFinder.SetNewSnapshot( iSnapshot ); // changefinder takes ownership
		iSnapshot = NULL;
		iSnapshotRegistered = ETrue;
		User::RequestComplete( iCallerStatus, KErrNone );
		}

	_DBG_FILE("CNSmlDummyModsFetcher::UpdateSnapshotL(): end");
}
// ---------------------------------------------------------------------------
// CNSmlDummyModsFetcher::ContactViewReady
// Implements the view ready function of MVPbkContactViewObserver
// ---------------------------------------------------------------------------
void CNSmlDummyModsFetcher::ContactViewReady(
                MVPbkContactViewBase& /*aView*/ )
	{
    _DBG_FILE("CNSmlDummyModsFetcher::ContactViewReady(): begin");

	SetActive();
	TRequestStatus* status = &iStatus;
	User::RequestComplete( status, KErrNone );

	_DBG_FILE("CNSmlDummyModsFetcher::ContactViewReady(): end");
	}

// ---------------------------------------------------------------------------
// CNSmlDummyModsFetcher::ContactViewUnavailable
// Implements the view unavailable function of MVPbkContactViewObserver
// ---------------------------------------------------------------------------
void CNSmlDummyModsFetcher::ContactViewUnavailable(
                MVPbkContactViewBase& /*aView*/ )
{
}

// ---------------------------------------------------------------------------
// CNSmlDummyModsFetcher::ContactAddedToView
// Implements the add contact function of MVPbkContactViewObserver
// ---------------------------------------------------------------------------
void CNSmlDummyModsFetcher::ContactAddedToView(
            MVPbkContactViewBase& /*aView*/,
            TInt /*aIndex*/,
            const MVPbkContactLink& /*aContactLink*/ )
{
}

// ---------------------------------------------------------------------------
// Implements the remove contact function of MVPbkContactViewObserver
// ---------------------------------------------------------------------------
void CNSmlDummyModsFetcher::ContactRemovedFromView(
                MVPbkContactViewBase& /*aView*/,
                TInt /*aIndex*/,
                const MVPbkContactLink& /*aContactLink*/ )
{
}

// ---------------------------------------------------------------------------
// CNSmlDummyModsFetcher::ContactViewError
// Implements the view error function of MVPbkContactViewObserver
// ---------------------------------------------------------------------------
void CNSmlDummyModsFetcher::ContactViewError(
            MVPbkContactViewBase& /*aView*/,
            TInt aError,
            TBool /*aErrorNotified*/ )
{
    _DBG_FILE("CNSmlDummyModsFetcher::ContactViewError(): begin");

    User::RequestComplete( iCallerStatus, aError );

    _DBG_FILE("CNSmlDummyModsFetcher::ContactViewError(): end");
}

// ---------------------------------------------------------------------------
//  CNSmlDummyModsFetcher::VPbkSingleContactOperationComplete
//  Called when the contact operation is completed.
// ---------------------------------------------------------------------------
//
 void CNSmlDummyModsFetcher::VPbkSingleContactOperationComplete(
		MVPbkContactOperationBase& aOperation,
		MVPbkStoreContact* aContact )
{
    _DBG_FILE("CNSmlDummyModsFetcher::VPbkSingleContactOperationComplete(): begin");

	MVPbkContactOperationBase* operation = &aOperation;
	if ( operation )
	{
		delete operation;
		operation = NULL;
	}

	TRAPD( error,UpdateSnapshotL( aContact ) );
	delete aContact;
	if( error!= KErrNone )
	   {
	   User::RequestComplete( iCallerStatus, error );
	   }
	else
	   {
	   SetActive();
	   TRequestStatus* status = &iStatus;
	   User::RequestComplete( status, KErrNone );
	   }

	_DBG_FILE("CNSmlDummyModsFetcher::VPbkSingleContactOperationComplete(): end");
}

// ---------------------------------------------------------------------------
//  CNSmlDummyModsFetcher::VPbkSingleContactOperationFailed
//  Called when the contact operation fails
// ---------------------------------------------------------------------------
//
void CNSmlDummyModsFetcher::VPbkSingleContactOperationFailed(
		MVPbkContactOperationBase& aOperation,
		 TInt /*aError*/ )
 {
    _DBG_FILE("CNSmlDummyModsFetcher::VPbkSingleContactOperationFailed(): begin");

 	MVPbkContactOperationBase* operation = &aOperation;
	if ( operation )
	{
		delete operation;
		operation = NULL;
	}

	_DBG_FILE("CNSmlDummyModsFetcher::VPbkSingleContactOperationFailed(): end");
 }
// End of File
