/*
* Copyright (c) 2007 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:  This is the default plugin which will be called before starting
*                the snchronization.
*
*/


#ifndef ASPPRESYNCDEFAULTPLUGIN_H
#define ASPPRESYNCDEFAULTPLUGIN_H

// INCLUDES
#include <e32cmn.h>
#include <s32mem.h>
#include <e32hashtab.h>
#include <e32property.h>
#include <centralrepository.h>
#include <ConeResLoader.h> 
#include <bautils.h>
#include <eikenv.h>
#include "CPreSyncPlugin.h"
#include <AknQueryDialog.h>

//  INCLUDES
#include <e32base.h>

// CLASS DECLARATION

class CAknGlobalMsgQuery;

class CRoamingCheck : public CBase
	{
	public:
    /*!
    @function NewL
    */
    static CRoamingCheck* NewL();
    
      /*!
      @function NewLC
      */
    static CRoamingCheck* NewLC();
    
      /*!
      @function ConstructL
      
      @discussion Perform the second phase construction of a CRoamingCheck object
      this needs to be public 
      */
     void ConstructL();

      /*!
      @function CRoamingCheck
      
      @discussion Perform the first phase of two phase construction.
      This needs to be public 
      */
     CRoamingCheck(CEikonEnv* aEikEnv);
     /*!
      @function ~CRoamingCheck
      @discussion Destroy the object and release all memory objects
      */
    ~CRoamingCheck();
    void ShowGlobalNoteL(TInt aSoftKeyId, 
              const HBufC* aMessage, TInt& status);
	private:
    CAknGlobalMsgQuery* iGlobalNote;
    CEikonEnv* iEnv;
    RConeResourceLoader iResLoader;
    };


// FORWARD DECLARATION

// CLASS DECLARATION
class CPreSyncDefaultPlugin : public CPreSyncPlugin
{
	public: 
	
		/**
		* Two phase construction
		*/
		static CPreSyncDefaultPlugin* NewL();

		/**
		* Destructor
		*/
		virtual ~CPreSyncDefaultPlugin();


     public:

         /**
          * Function:    IsSupported
          *
          */
         virtual TBool IsSupported();
         
         /**
          * Function:    CanSync
          *
          */
         virtual TBool CanSyncL();
         
         /**
          * Function:    SetProfile  
          *
          */
         virtual TBool SetProfile(const TSmlProfileId& aProfileId);
         
         /**
          * Function:    SyncNow  
          *
          */   
         virtual TInt SyncNow(MSyncExtObserver* aObserver);

		
	private:
	    void  ReadRepositoryL(TInt aKey, TInt& aValue);
	    
	    
		/**
		* Constructor
		*/
		CPreSyncDefaultPlugin();

		/**
		* 2nd phase construtor
		*/
		void ConstructL();
		
	private:
	    TSmlProfileId iSyncProfileId;
};
	

#endif // ASPPRESYNCDEFAULTPLUGIN_H
