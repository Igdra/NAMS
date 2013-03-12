/***************************************************************************
 * NAMS - Not Another MUD Server                                           *
 * Copyright (C) 2012 Matthew Goff (matt@goff.cc) <http://www.ackmud.net/> *
 *                                                                         *
 * This program is free software: you can redistribute it and/or modify    *
 * it under the terms of the GNU General Public License as published by    *
 * the Free Software Foundation, either version 3 of the License, or       *
 * (at your option) any later version.                                     *
 *                                                                         *
 * This program is distributed in the hope that it will be useful,         *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 * GNU General Public License for more details.                            *
 *                                                                         *
 * You should have received a copy of the GNU General Public License       *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.   *
 ***************************************************************************/
/**
 * @file handler.cpp
 * @brief All non-template member functions of the Handler namespace.
 *
 * The Handler namespace contains all non-class process control functions.
 * To pointedly avoid large complex functions such as "nanny()" from Diku
 * heritage, process controls should be implemented as individual functions
 * within this namespace.
 */
#include "h/includes.h"
#include "h/class.h"

#include "h/handler.h"
#include "h/account.h"
#include "h/command.h"
#include "h/list.h"
#include "h/server.h"

/* Core */
/**
 * @brief Main account menu for all account actions.
 * @param[in] client The SocketClient to process a menu request for.
 * @param[in] cmd The command sent by the SocketClient.
 * @param[in] args Any arguments to the command.
 * @retval void
 */
const void Handler::AccountMenu( SocketClient* client, const string& cmd, const string& args )
{
    UFLAGS_DE( flags );

    if ( client == NULL )
    {
        LOGSTR( flags, "Handler::AccountMenu()-> called with NULL client" );
        return;
    }

    if ( client->gAccount() == NULL )
    {
        LOGSTR( flags, "Handler::AccountMenu()-> called with NULL account" );
        return;
    }

    client->Send( "And we danced!" );

    switch ( client->gState() )
    {
        default:
        break;
    }

    return;
}

/**
 * @brief Handle SocketClient objects who have not fully logged into the game yet.
 * @param[in] client The SocketClient to process a login request for.
 * @param[in] cmd The command sent by the SocketClient.
 * @param[in] args Any arguments to the command.
 * @retval void
 */
const void Handler::ProcessLogin( SocketClient* client, const string& cmd, const string& args )
{
    UFLAGS_DE( flags );

    if ( client == NULL )
    {
        LOGSTR( flags, "Handler::ProcessLogin()-> called with NULL client" );
        return;
    }

    switch ( client->gState() )
    {
        case SOC_STATE_LOGIN_SCREEN:
            LoginScreen( client, cmd, args );
        break;

        case SOC_STATE_GET_OLD_PASSWORD:
            GetOldPassword( client, cmd, args );
        break;

        case SOC_STATE_GET_NEW_ACCOUNT:
            GetNewAccount( client, cmd, args );
        break;

        case SOC_STATE_GET_NEW_PASSWORD:
            GetNewPassword( client, cmd, args );
        break;

        case SOC_STATE_CREATE_ACCOUNT:
            CreateAccount( client, cmd, args );
        break;

        default:
        break;
    }

    return;
}

/* Query */
/**
 * @brief Check to see if the current account is already being created.
 * @param[in] client The SocketClient to check against active connections.
 * @param[in] name The Account name to check against active connections.
 * @retval false Returned if the account is not playing or is playing but in a state greater than or equal to #SOC_ACCOUNT_MENU.
 * @retval true Returned if the account is playing and in a state less than #SOC_ACCOUNT_MENU.
 */
const bool Handler::CheckCreating( SocketClient* client, const string& name )
{
    UFLAGS_S( flags );
    ITER( list, SocketClient*, si );
    SocketClient* socket_client;

    for ( si = socket_client_list.begin(); si != socket_client_list.end(); si = client->gServer()->gSocketClientNext() )
    {
        socket_client = *si;
        client->gServer()->sSocketClientNext( ++si );

        if ( socket_client == client )
            continue;

        if ( socket_client->gState() >= SOC_STATE_ACCOUNT_MENU )
            continue;

        if ( name.compare( socket_client->gLogin( SOC_LOGIN_NAME ) ) == 0 )
        {
            LOGFMT( flags, "Handler::CheckPlaying()-> player from site %s attempted to login as %s (in creation)", CSTR( client->gHostname() ), CSTR( name ) );
            return true;
        }
    }

    return false;
}

/**
 * @brief Checks to see if a name is on the server's prohibited list.
 * @param[in] client The SocketClient to requesting to use the name.
 * @param[in] name The Account name to check against the prohibited list.
 * @retval false Returned if the name is not prohibited.
 * @retval true Returned if the name is prohibited.
 */
const bool Handler::CheckProhibited( SocketClient* client, const string& name )
{
    ITER( forward_list, string, fi );
    forward_list<string> search;
    string comp;

    // Search for prohibited names
    search = client->gServer()->gConfig()->gAccountProhibitedNames();
    for ( fi = search.begin(); fi != search.end(); fi++ )
    {
        comp = *fi;
        if ( comp.compare( name ) == 0 )
            return true;
    }

    return false;
}

/* Manipulate */
/**
 * @brief Create a new account object with a name and password.
 * @param[in] client The SocketClient to process a login request for.
 * @param[in] cmd The command sent by the SocketClient.
 * @param[in] args Any arguments to the command.
 * @retval void
 */
const void Handler::CreateAccount( SocketClient* client, const string& cmd, const string& args )
{
    Account* account = NULL;

    account = new Account();
    if ( !account->New( client ) )
    {
        account->Delete();
        client->sLogin( SOC_LOGIN_NAME, "" );
        client->sLogin( SOC_LOGIN_PASSWORD, "" );
        client->sState( SOC_STATE_LOGIN_SCREEN );
        client->Send( CFG_STR_ACT_NEW_ERROR );
        ProcessLogin( client );

        return;
    }

    // All went well, off to the account menu
    client->sState( SOC_STATE_ACCOUNT_MENU );
    AccountMenu( client );

    return;
}

/* Internal */
/**
 * @brief Select a new account name.
 * @param[in] client The SocketClient to process a login request for.
 * @param[in] cmd The command sent by the SocketClient.
 * @param[in] args Any arguments to the command.
 * @retval void
 */
const void Handler::GetNewAccount( SocketClient* client, const string& cmd, const string& args )
{
    UFLAGS_DE( flags );

    if ( client == NULL )
    {
        LOGSTR( flags, "Handler::GetNewAccount()-> called with NULL client" );
        return;
    }

    //Initial entry or an invalid selection
    if ( cmd.empty() && !client->gLogin( SOC_LOGIN_NAME ).empty() )
    {
        client->Send( Utils::FormatString( 0, CFG_STR_ACT_NAME_CONFIRM, CSTR( client->gLogin( SOC_LOGIN_NAME ) ) ) );
        return;
    }

    if ( Utils::StrPrefix( cmd, "no", true ) )
    {
        client->sLogin( SOC_LOGIN_NAME, "" );
        client->sState( SOC_STATE_LOGIN_SCREEN );
    }
    else if ( Utils::StrPrefix( cmd, "yes", true ) )
        client->sState( SOC_STATE_GET_NEW_PASSWORD );
    else
        client->Send( CFG_STR_SEL_INVALID );

    //Generate the next input prompt
    ProcessLogin( client );

    return;
}

/**
 * @brief Create a new account.
 * @param[in] client The SocketClient to process a login request for.
 * @param[in] cmd The command sent by the SocketClient.
 * @param[in] args Any arguments to the command.
 * @retval void
 */
const void Handler::GetNewPassword( SocketClient* client, const string& cmd, const string& args )
{
    UFLAGS_DE( flags );

    if ( client == NULL )
    {
        LOGSTR( flags, "Handler::GetNewPassword()-> called with NULL client" );
        return;
    }

    //Initial entry
    if ( cmd.empty() )
    {
        client->Send( CFG_STR_ACT_NEW );
        client->Send( CFG_STR_ACT_PASSWORD_GET );

        return;
    }

    if ( cmd.length() < CFG_ACT_PASSWORD_MIN_LEN || cmd.length() > CFG_ACT_PASSWORD_MAX_LEN )
    {
        client->Send( CFG_STR_ACT_PASSWORD_INVALID );
        client->Send( CFG_STR_ACT_PASSWORD_LENGTH );
        client->Send( CFG_STR_ACT_PASSWORD_GET );

        return;
    }

    if ( client->gLogin( SOC_LOGIN_PASSWORD ).empty() )
    {
        client->sLogin( SOC_LOGIN_PASSWORD, crypt( CSTR( cmd ), CSTR( Utils::Salt( client->gLogin( SOC_LOGIN_NAME ) ) ) ) );
        client->Send( CFG_STR_ACT_PASSWORD_CONFIRM );

        return;
    }

    if ( Utils::String( crypt( CSTR( cmd ), CSTR( Utils::Salt( client->gLogin( SOC_LOGIN_NAME ) ) ) ) ).compare( client->gLogin( SOC_LOGIN_PASSWORD ) ) == 0 )
        client->sState( SOC_STATE_CREATE_ACCOUNT );
    else
    {
        client->sLogin( SOC_LOGIN_PASSWORD, "" );
        client->Send( CFG_STR_ACT_PASSWORD_MISMATCH );
        client->Send( CFG_STR_ACT_PASSWORD_GET );

        return;
    }

    //Generate the next input prompt
    ProcessLogin( client );

    return;
}

/**
 * @brief Compare a received password against an existing account.
 * @param[in] client The SocketClient to process a login request for.
 * @param[in] cmd The command sent by the SocketClient.
 * @param[in] args Any arguments to the command.
 * @retval void
 */
const void Handler::GetOldPassword( SocketClient* client, const string& cmd, const string& args )
{
    UFLAGS_DE( flags );

    if ( client == NULL )
    {
        LOGSTR( flags, "Handler::GetOldPassword()-> called with NULL client" );
        return;
    }

    //Initial hit prompting for password
    if ( cmd.empty() )
    {
        client->Send( CFG_STR_ACT_PASSWORD_GET );
        return;
    }

    //Generate the next input prompt
    ProcessLogin( client );

    return;
}

/**
 * @brief Send initial greeting and request for account name.
 * @param[in] client The SocketClient to process a login request for.
 * @param[in] cmd The command sent by the SocketClient.
 * @param[in] args Any arguments to the command.
 * @retval void
 */
const void Handler::LoginScreen( SocketClient* client, const string& cmd, const string& args )
{
    UFLAGS_DE( flags );
    stringstream account;

    if ( client == NULL )
    {
        LOGSTR( flags, "Handler::LoginScreen()-> called with NULL client" );
        return;
    }

    // Initial connection with no input yet received or previous name was invalid
    if ( cmd.empty() )
    {
        client->Send( CFG_STR_ACT_NAME_GET );
        return;
    }

    // Only allow alphanumerics for the account name itself
    if ( !Utils::iAlNum( cmd ) )
    {
        client->Send( CFG_STR_ACT_NAME_INVALID );
        client->Send( CFG_STR_ACT_NAME_ALNUM );
        ProcessLogin( client );

        return;
    }

    //Prevent prohibited names based on Server runtime configuration
    if ( Handler::CheckProhibited( client, cmd ) )
    {
        client->Send( CFG_STR_ACT_NAME_INVALID );
        client->Send( CFG_STR_ACT_NAME_PROHIBITED );
        ProcessLogin( client );

        return;
    }

    //Prevent two new accounts being created with the same name
    if ( Handler::CheckCreating( client, cmd ) )
    {
        client->Send( CFG_STR_ACT_NAME_INVALID );
        client->Send( CFG_STR_ACT_NAME_PROHIBITED );
        ProcessLogin( client );

        return;
    }

    if ( cmd.length() < CFG_ACT_NAME_MIN_LEN || cmd.length() > CFG_ACT_NAME_MAX_LEN )
    {
        client->Send( CFG_STR_ACT_NAME_INVALID );
        client->Send( CFG_STR_ACT_NAME_LENGTH );
    }

    if ( client->gLogin( SOC_LOGIN_NAME ).empty() )
    {
        client->sLogin( SOC_LOGIN_NAME, cmd );
        account << CFG_DAT_DIR_ACCOUNT << "/" << cmd;

        switch (  Utils::DirExists( account.str() ) )
        {
            case UTILS_RET_FALSE:
                client->sState( SOC_STATE_GET_NEW_ACCOUNT );
            break;

            case UTILS_RET_TRUE:
                client->sState( SOC_STATE_GET_OLD_PASSWORD );
            break;

            case UTILS_RET_ERROR:
            default:
                client->Send( CFG_STR_ACT_NAME_INVALID );
                LOGFMT( flags, "Handler::LoginScreen()->Utils::DirExists()-> returned UTILS_RET_ERROR for name: %s", CSTR( cmd ) );
            break;
        }
    }

    //Generate the next input prompt
    ProcessLogin( client );

    return;
}
