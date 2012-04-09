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
#include "h/includes.h"
#include "h/class.h"

#include "h/socketclient.h"

// Core
const void SocketClient::Disconnect( const string& msg )
{
    if ( !msg.empty() )
    {
        Send( msg );
        Send();
    }

    m_server->sSocketClose( m_server->gSocketClose() + 1 );
    socket_client_list.remove( this );
    delete this;

    return;
}

bool SocketClient::ProcessCommand()
{
    UFLAGS_DE( flags );

    if ( !Valid() )
    {
        LOGSTR( flags, "SocketClient::ProcessCommand()-> called with invalid socket" );
        return false;
    }

    if ( m_command_queue.empty() )
    {
        LOGSTR( flags, "SocketClient::ProcessCommand()-> called with empty command queue" );
        return false;
    }

    return true;
}

bool SocketClient::ProcessInput()
{
    UFLAGS_DE( flags );
    vector<string> commands;
    string command;
    ITER( vector, string, vi );

    if ( !Valid() )
    {
        LOGSTR( flags, "SocketClient::ProcessInput()-> called with invalid socket" );
        return false;
    }

    // Nothing new to process; move along
    if ( m_input.empty() )
        return true;

    commands = Utils::StrNewlines( m_input );
    for ( vi = commands.begin(); vi != commands.end(); vi++ )
    {
        command = *vi;
        if ( !QueueCommand( command ) )
        {
            LOGFMT( flags, "SocketClient::ProcessInput()->SocketClient::QueueCommand()-> returned false for cmd: %s", CSTR( command ) );
            return false;
        }
    }

    m_input.clear();

    return true;
}

bool SocketClient::QueueCommand( const string& command )
{
    UFLAGS_DE( flags );

    if ( !Valid() )
    {
        LOGSTR( flags, "SocketClient::QueueCommand()-> called with invalid socket" );
        return false;
    }

    if ( command.empty() )
    {
        LOGSTR( flags, "SocketClient::QueueCommand()-> called with empty command" );
        return false;
    }

    m_command_queue.push_back( command );

    return true;
}

bool SocketClient::Recv()
{
    UFLAGS_DE( flags );
    ssize_t amount = 0;
    char buf[CFG_STR_MAX_BUFLEN] = {'\0'};

    if ( !Valid() )
    {
        LOGSTR( flags, "SocketClient::Recv()-> called with invalid socket" );
        return false;
    }

    if ( ( m_input.length() + CFG_STR_MAX_BUFLEN ) >= m_input.max_size() )
    {
        LOGFMT( flags, "SocketClient::Recv()->recv()-> called with m_input overflow: %lu + %d", m_input.length(), CFG_STR_MAX_BUFLEN );
        return false;
    }

    if ( ( amount = ::recv( m_descriptor, buf, CFG_STR_MAX_BUFLEN - 1, 0 ) ) < 1 )
    {
        if ( amount == 0 )
        {
            LOGFMT( flags, "SocketClient::Recv()->recv()-> broken pipe encountered on recv from: %s", CSTR( m_hostname ) );
            return false;
        }
        else if ( errno != EAGAIN && errno != EWOULDBLOCK )
        {
            LOGERRNO( flags, "SocketClient::Recv()->recv()->" );
            return false;
        }
    }

    m_server->gSocket()->sBytesRecvd( m_server->gSocket()->gBytesRecvd() + amount );
    sBytesRecvd( m_bytes_recvd + amount );
    m_input += buf;

    return true;
}

const void SocketClient::ResolveHostname()
{
    UFLAGS_DE( flags );
    pthread_t res_thread;
    pthread_attr_t res_attr;

    if ( !Valid() )
    {
        LOGSTR( flags, "SocketClient::ResolveHostname()-> called with invalid socket" );
        return;
    }

    if ( pthread_attr_init( &res_attr ) != 0 )
    {
        LOGERRNO( flags, "SocketClient::ResolveHostname()->pthread_attr_init()->" );
        return;
    }

    if ( pthread_attr_setdetachstate( &res_attr, PTHREAD_CREATE_DETACHED ) != 0 )
    {
        LOGERRNO( flags, "SocketClient::ResolveHostname()->pthread_attr_setdetachstate()->" );
        return;
    }

    if ( pthread_create( &res_thread, &res_attr, &SocketClient::tResolveHostname, this ) != 0 )
    {
        LOGERRNO( flags, "SocketClient::ResolveHostname()->pthread_create()->" );
        return;
    }

    if ( pthread_attr_destroy( &res_attr ) != 0 )
    {
        LOGERRNO( flags, "SocketClient::ResolveHostname()->pthread_attr_destroy()->" );
        return;
    }

    return;
}

bool SocketClient::Send( const string& msg )
{
    UFLAGS_DE( flags );

    if ( !Valid() )
    {
        LOGSTR( flags, "SocketClient::Send()-> called with invalid socket" );
        return false;
    }

    // Prepend a CRLF to ensure output lands on a newline.
    // Make this client configurable in the future.
    if ( m_output.empty() && m_state > SOC_STATE_LOGIN_SCREEN )
    {
        m_output += CRLF;
        m_output += msg;
    }
    else
        m_output += msg;

    return true;
}

bool SocketClient::Send()
{
    UFLAGS_DE( flags );
    ssize_t amount = 0;

    if ( !Valid() )
    {
        LOGSTR( flags, "SocketClient::Send()-> called with invalid socket" );
        return false;
    }

    if ( m_output.empty() )
    {
        LOGSTR( flags, "SocketClient::Send()->send()-> called with empty output buffer" );
        return false;
    }

    if ( ( amount = ::send( m_descriptor, CSTR( m_output ), m_output.length(), 0 ) ) < 1 )
    {
        if ( amount == 0 )
        {
            LOGFMT( flags, "SocketClient::Send()->send()-> broken pipe encountered on send to: %s", CSTR( m_hostname ) );
            return false;
        }
        else if ( errno != EAGAIN && errno != EWOULDBLOCK )
        {
            LOGERRNO( flags, "SocketClient::Send()->send()->" );
            return false;
        }
    }

    m_server->gSocket()->sBytesSent( m_server->gSocket()->gBytesSent() + amount );
    sBytesSent( m_bytes_sent + amount );
    m_output.clear();

    return true;
}

// Query

// Manipulate
bool SocketClient::sIdle( const uint_t& idle )
{
    UFLAGS_DE( flags );

    if ( idle < 0 || idle > CFG_SOC_MAX_IDLE )
    {
        LOGFMT( flags, "SocketClient::sIdle()-> called with invalid idle: %ld", idle );
        return false;
    }

    m_idle = idle;

    return true;
}

void* SocketClient::tResolveHostname( void* data )
{
    UFLAGS_DE( flags );
    SocketClient* socket_client = reinterpret_cast<SocketClient*>( data );
    static sockaddr_in sa_zero;
    sockaddr_in sa = sa_zero;
    sint_t error = 0;
    char hostname[CFG_STR_MAX_BUFLEN];

    sa.sin_family = AF_INET;

    if ( ( error = inet_pton( AF_INET, CSTR( socket_client->gHostname() ), &sa.sin_addr ) ) != 1 )
    {
        LOGFMT( flags, "SocketClient::tResolveHostname()->inet_pton()-> returned errno %d: %s", error, gai_strerror( error ) );
        pthread_exit( reinterpret_cast<void*>( EXIT_FAILURE ) );
    }

    if ( ( error = getnameinfo( reinterpret_cast<sockaddr*>( &sa ), sizeof( sa ), hostname, sizeof( hostname ), NULL, 0, 0 ) ) != 0 )
    {
        LOGFMT( flags, "SocketClient::tResolveHostname()->getnameinfo()-> returned errno %d: %s", error, gai_strerror( error ) );
        pthread_exit( reinterpret_cast<void*>( EXIT_FAILURE ) );
    }

    socket_client->sHostname( hostname );
    LOGFMT( 0, "SocketClient::ResolveHostname()-> %s", CSTR( socket_client->gHostname() ) );

    pthread_exit( reinterpret_cast<void*>( EXIT_SUCCESS ) );
}

bool SocketClient::sServer( Server* server )
{
    UFLAGS_DE( flags );

    if ( !server )
    {
        LOGSTR( flags, "SocketClient::sServer()-> called with NULL server" );
        return false;
    }

    if ( !server->Running() )
    {
        LOGSTR( flags, "SocketClient::sServer()-> called with invalid server" );
        return false;
    }

    m_server = server;

    return true;
}

bool SocketClient::sState( const uint_t& state )
{
    UFLAGS_DE( flags );

    if ( state < SOC_STATE_DISCONNECTED || state >= MAX_SOC_STATE )
    {
        LOGFMT( flags, "SocketClient::sState()-> called with invalid state: %lu", state );
        return false;
    }

    m_state = state;

    return true;
}

SocketClient::SocketClient( Server* server, sint_t& descriptor ) : Socket( descriptor )
{
    m_command_queue.clear();
    m_idle = 0;
    m_input.clear();
    m_output.clear();
    m_state = SOC_STATE_DISCONNECTED;

    sServer( server );
    m_server->sSocketOpen( m_server->gSocketOpen() + 1 );

    socket_client_list.push_back( this );

    return;
}

SocketClient::~SocketClient()
{
    return;
}
