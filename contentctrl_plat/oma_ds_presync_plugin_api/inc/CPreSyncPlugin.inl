/*
* Copyright (c) 2008-2009 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:  ECom search interface definition
*
*/

// Interface's (abstract base class's) destructor

inline CPreSyncPlugin::~CPreSyncPlugin()
    {
 
       REComSession::DestroyedImplementation(iDtor_ID_Key);
    }

inline CPreSyncPlugin*
CPreSyncPlugin::NewL(TUid aImplementationUid)
    {
    TAny* ptr = REComSession::CreateImplementationL(
            aImplementationUid,_FOFF(CPreSyncPlugin,iDtor_ID_Key));
    // The type of TAny* ptr should be CPreSyncPlugin.
    return REINTERPRET_CAST(CPreSyncPlugin*, ptr);
    }

inline CPreSyncPlugin* CPreSyncPlugin::NewL(const TDesC8& aParam)
    {
    // Define options, how the default resolver will find appropriate
    // implementation.
    TEComResolverParams resolverParams;
    resolverParams.SetDataType(aParam);
    resolverParams.SetWildcardMatch(ETrue);     // Allow wildcard matching

    TAny* ptr = REComSession::CreateImplementationL(
            KPreSyncPluginInterfaceUid,
        _FOFF(CPreSyncPlugin,iDtor_ID_Key),resolverParams);

    // The type of TAny* ptr should be CTmoPhonebookSyncInterfaceDefinitionDlg.
    return REINTERPRET_CAST(CPreSyncPlugin*, ptr);
    }
