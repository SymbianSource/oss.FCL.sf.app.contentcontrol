/*
* Copyright (c) 2006 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:  DS contacts adapters 
*
*/


// INCLUDE FILES
#include <e32base.h>
#include <s32strm.h>
#include <cntdb.h>
#include <e32cmn.h>
#include <e32des16.h>
#include <cntitem.h>
#include <cntfield.h>
#include <cntfldst.h>

#include "nsmlcontactsdefs.h"
#include "NSmlContactsModsFetcher.h"
#include "nsmldebug.h"
#include "nsmlsnapshotitem.h"
#include "nsmlchangefinder.h"

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
#include <VPbkEng.rsg>


// ------------------------------------------------------------------------------------------------
// CNSmlContactsModsFetcher::CNSmlContactsModsFetcher
// ------------------------------------------------------------------------------------------------
//
CNSmlContactsModsFetcher::CNSmlContactsModsFetcher( TBool& aSnapshotRegistered,CVPbkContactManager& aContactManager,MVPbkContactStore& aStore,TKeyArrayFix& aKey, CNSmlChangeFinder& aChangeFinder ) :
	CActive( EPriorityLow ), 
	iSnapshotRegistered( aSnapshotRegistered ), 
	iKey( aKey ), 
	iChangeFinder( aChangeFinder ),
	iContactManager(aContactManager) ,
	iStore(aStore) ,
	iContactViewBase(NULL)
	{
	_DBG_FILE("CNSmlContactsModsFetcher::CNSmlContactsModsFetcher(): begin");
	CActiveScheduler::Add( this );
	_DBG_FILE("CNSmlContactsModsFetcher::CNSmlContactsModsFetcher(): end");
	}

// ------------------------------------------------------------------------------------------------
// CNSmlContactsModsFetcher::~CNSmlContactsModsFetcher
// ------------------------------------------------------------------------------------------------
//
CNSmlContactsModsFetcher::~CNSmlContactsModsFetcher()
	{
	_DBG_FILE("CNSmlContactsModsFetcher::~CNSmlContactsModsFetcher(): begin");
	
	Cancel();	
	delete iSnapshot;
	delete iContactViewBase;
	
	
	_DBG_FILE("CNSmlContactsModsFetcher::~CNSmlContactsModsFetcher(): end");
	}
	
// ------------------------------------------------------------------------------------------------
// CNSmlContactsModsFetcher::FetchModificationsL
// ------------------------------------------------------------------------------------------------
//
void CNSmlContactsModsFetcher::FetchModificationsL( TRequestStatus& aStatus )
	{
	_DBG_FILE("CNSmlContactsModsFetcher::FetchModificationsL(): begin");
	
	iCallerStatus = &aStatus;
	*iCallerStatus = KRequestPending;
	iContactCount = 0;
	if( !iSnapshot )
		{
		iSnapshot = new (ELeave) CArrayFixSeg<TNSmlSnapshotItem>( KNSmlSnapshotSize );
		}
		
	if(iContactViewBase)
	{
	delete iContactViewBase;
	iContactViewBase = NULL;	
	}
	
	CreateViewL();
	
	_DBG_FILE("CNSmlContactsModsFetcher::FetchModificationsL(): end");
	
	}
	
// ------------------------------------------------------------------------------------------------
// CNSmlContactsModsFetcher::ConstructL
// ------------------------------------------------------------------------------------------------
//
void CNSmlContactsModsFetcher::ConstructL()
	{
	iSnapshot = new (ELeave) CArrayFixSeg<TNSmlSnapshotItem>(KNSmlSnapshotSmallSize);
	
	}
	
// ------------------------------------------------------------------------------------------------
// CNSmlContactsModsFetcher::RunL
// ------------------------------------------------------------------------------------------------	
//
void CNSmlContactsModsFetcher::RunL()
	{
	_DBG_FILE("CNSmlContactsModsFetcher::RunL(): begin");
	
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
	
	_DBG_FILE("CNSmlContactsModsFetcher::RunL(): end");
	}

// ------------------------------------------------------------------------------------------------
// CNSmlContactsModsFetcher::FetchNextContactL
// ------------------------------------------------------------------------------------------------	
//
void CNSmlContactsModsFetcher::FetchNextContactL()
	{
	_DBG_FILE("CNSmlContactsModsFetcher::FetchNextContactL(): begin");
	if(iContactViewBase->ContactCountL() == 0)
		{
		User::RequestComplete( iCallerStatus, KErrNone );
		return;	
		}
	if ( !iSnapshotRegistered )
		{
		const MVPbkViewContact& contact = iContactViewBase->ContactAtL(iContactCount);

		MVPbkContactLink* contactLink = contact.CreateLinkLC();
		iContactManager.RetrieveContactL( *contactLink, *this );
		CleanupStack::PopAndDestroy();	
		}

	_DBG_FILE("CNSmlContactsModsFetcher::FetchNextContactL(): end");
	}

// ------------------------------------------------------------------------------------------------
// CNSmlContactsModsFetcher::DoCancel
// ------------------------------------------------------------------------------------------------	
//
void CNSmlContactsModsFetcher::DoCancel()
	{
	_DBG_FILE("CNSmlContactsModsFetcher::DoCancel(): begin");

	User::RequestComplete( iCallerStatus, KErrCancel );
	
	_DBG_FILE("CNSmlContactsModsFetcher::DoCancel(): end");
	}

// ----------------------------------------------------------------------------
// CNSmlContactsModsFetcher::IsConfidentialL
// ----------------------------------------------------------------------------
//	
TBool CNSmlContactsModsFetcher::IsConfidentialL( MVPbkStoreContact& aItem )
    {
    _DBG_FILE("CNSmlContactsModsFetcher::IsConfidentialL(): begin");
    TBool ret( EFalse );
    
    //find X-CLASS field type
	const MVPbkFieldType* syncclass = 
        iContactManager.FieldTypes().Find(R_VPBK_FIELD_TYPE_SYNCCLASS );
	CVPbkBaseContactFieldTypeIterator* itr = 
        CVPbkBaseContactFieldTypeIterator::NewLC( *syncclass, 
            aItem.Fields() );
	while ( itr->HasNext() )
        {
        const MVPbkBaseContactField* field = itr->Next();
        const MVPbkContactFieldTextData& data = 
        	MVPbkContactFieldTextData::Cast(field->FieldData());
        TPtrC ptr = data.Text();
        //compare the sync type
        if ( ptr.Compare( KNSmlContactSyncNoSync ) == 0 )
            {
            _DBG_FILE("CNSmlContactsModsFetcher::IsConfidentialL(): \
                       find confidential");
            ret = ETrue;
            }
         
        }
	CleanupStack::PopAndDestroy( itr );
    
    _DBG_FILE("CNSmlContactsModsFetcher::IsConfidentialL(): end");
    return ret;
    }


// ----------------------------------------------------------------------------
// CNSmlContactsModsFetcher::RunError
// ----------------------------------------------------------------------------
 TInt CNSmlContactsModsFetcher::RunError( TInt aError )
	{
	DBG_FILE_CODE(aError, _S8("CNSmlContactsModsFetcher::RunError() : error code received "));
	User::RequestComplete( iCallerStatus, aError );
	return KErrNone;
	}
	
// -----------------------------------------------------------------------------
// CNSmlContactsModsFetcher::CreateViewL()
// Create a contact view of the store which is opened
// -----------------------------------------------------------------------------
 
void CNSmlContactsModsFetcher::CreateViewL()
	{
	CVPbkContactViewDefinition* viewDef = CVPbkContactViewDefinition::NewL();
	CleanupStack::PushL( viewDef );
	viewDef->SetType( EVPbkContactsView );

	viewDef->SetUriL(iStore.StoreProperties().Uri().UriDes());

	 iContactViewBase = iContactManager.CreateContactViewLC( 
				                         *this, 
				                         *viewDef, 
				                         iContactManager.FieldTypes()  );
	CleanupStack::Pop();
	CleanupStack::PopAndDestroy(viewDef);

	}
// ---------------------------------------------------------------------------
//  CNSmlContactsModsFetcher::UpdateSnapshotL
//  Add the fetched contacts id and timestamp 
//  to the snapshot.
// ---------------------------------------------------------------------------
//
 
 void CNSmlContactsModsFetcher::UpdateSnapshotL(MVPbkStoreContact* aContact)
{
	if(!IsConfidentialL(*aContact))
		{
		CVPbkContactIdConverter* idConverter = CVPbkContactIdConverter::NewL(iStore );
		CleanupStack::PushL( idConverter );

		TInt32 id = idConverter->LinkToIdentifier(
			*iContactViewBase->ContactAtL(iContactCount).CreateLinkLC());
		TNSmlSnapshotItem snap( id);
		
		MVPbkStoreContact2* tempContact = 
		reinterpret_cast<MVPbkStoreContact2*> (aContact->StoreContactExtension (KMVPbkStoreContactExtension2Uid));  
              
		MVPbkStoreContactProperties *contactProterties 
									= tempContact ->PropertiesL();
		CleanupDeletePushL(contactProterties);				
		snap.SetLastChangedDate(contactProterties->LastModifiedL());
		CleanupStack::PopAndDestroy();
				
		iSnapshot->InsertIsqL( snap, iKey );
		iCurrentSnapshotCounter++;
		if ( iCurrentSnapshotCounter > 20 )
			{
			iSnapshot->Compress();
			iCurrentSnapshotCounter = 0;
			}
		CleanupStack::PopAndDestroy();
		CleanupStack::PopAndDestroy(idConverter);	
		}
	
	iContactCount++;
	
	if ( iContactCount == iContactViewBase->ContactCountL() )
		{
		iChangeFinder.SetNewSnapshot( iSnapshot ); // changefinder takes ownership
		iSnapshot = NULL;
		iSnapshotRegistered = ETrue;
		User::RequestComplete( iCallerStatus, KErrNone );
		}

}	
// ---------------------------------------------------------------------------
// CNSmlContactsModsFetcher::ContactViewReady
// Implements the view ready function of MVPbkContactViewObserver
// --------------------------------------------------------------------------- 
void CNSmlContactsModsFetcher::ContactViewReady(
                MVPbkContactViewBase& /*aView*/ ) 
	{
    _DBG_FILE("CNSmlContactsModsFetcher::ContactViewReady(): begin");
	
	SetActive();
	TRequestStatus* status = &iStatus;
	User::RequestComplete( status, KErrNone );	
	
	_DBG_FILE("CNSmlContactsModsFetcher::ContactViewReady(): end");
	}

// ---------------------------------------------------------------------------
// CNSmlContactsModsFetcher::ContactViewUnavailable
// Implements the view unavailable function of MVPbkContactViewObserver
// --------------------------------------------------------------------------- 
void CNSmlContactsModsFetcher::ContactViewUnavailable(
                MVPbkContactViewBase& /*aView*/ )  
{
   

}

// ---------------------------------------------------------------------------
// CNSmlContactsModsFetcher::ContactAddedToView
// Implements the add contact function of MVPbkContactViewObserver
// --------------------------------------------------------------------------- 
void CNSmlContactsModsFetcher::ContactAddedToView(
            MVPbkContactViewBase& /*aView*/, 
            TInt /*aIndex*/, 
            const MVPbkContactLink& /*aContactLink*/ ) 
{
}

// ---------------------------------------------------------------------------
// Implements the remove contact function of MVPbkContactViewObserver
// --------------------------------------------------------------------------- 
void CNSmlContactsModsFetcher::ContactRemovedFromView(
                MVPbkContactViewBase& /*aView*/, 
                TInt /*aIndex*/, 
                const MVPbkContactLink& /*aContactLink*/ )  
{
}

// ---------------------------------------------------------------------------
// CNSmlContactsModsFetcher::ContactViewError
// Implements the view error function of MVPbkContactViewObserver
// --------------------------------------------------------------------------- 
void CNSmlContactsModsFetcher::ContactViewError(
            MVPbkContactViewBase& /*aView*/, 
            TInt aError, 
            TBool /*aErrorNotified*/ )  
{
User::RequestComplete( iCallerStatus, aError );
}

// ---------------------------------------------------------------------------
//  CNSmlContactsModsFetcher::VPbkSingleContactOperationComplete
//  Called when the contact operation is completed.
// ---------------------------------------------------------------------------
//
 void CNSmlContactsModsFetcher::VPbkSingleContactOperationComplete(
		MVPbkContactOperationBase& aOperation,
		MVPbkStoreContact* aContact )
{
	MVPbkContactOperationBase* operation = &aOperation;
	if ( operation )
	{
		delete operation;
		operation = NULL;
	}
	
	TRAPD(error,UpdateSnapshotL(aContact));
	delete aContact;
	if(error!= KErrNone)
	{
	User::RequestComplete( iCallerStatus, error );	
	}
	else
	{
	SetActive();
	TRequestStatus* status = &iStatus;
	User::RequestComplete( status, KErrNone );		
	}
	
}

// ---------------------------------------------------------------------------
//  CNSmlContactsModsFetcher::VPbkSingleContactOperationFailed
//  Called when the contact operation fails
// ---------------------------------------------------------------------------
//
void CNSmlContactsModsFetcher::VPbkSingleContactOperationFailed(
		MVPbkContactOperationBase& aOperation,
		 TInt /*aError*/ )
 {
 	MVPbkContactOperationBase* operation = &aOperation;
	if ( operation )
	{
		delete operation;
		operation = NULL;
	}
 }
// End of File  
