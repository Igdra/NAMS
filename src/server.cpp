#include "h/globals.h"
#include "h/server.h"

// Core
bool Server::InitSocket( Socket* socket )
{
    bitset<CFG_MEM_MAX_BITSET> flags;
    uint_t enable = 1;

    flags.set( UTILS_DEBUG );
    flags.set( UTILS_TYPE_ERROR );

    if ( !socket->sDescriptor( ::socket( AF_INET, SOCK_STREAM, 0 ) ) )
    {
        Utils::Logger( flags, Utils::FormatString( flags, "socket() returned errno %d: %s", errno, strerror( errno ) ) );
        return false;
    }

    if ( ::setsockopt( socket->gDescriptor(), SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>( &enable ), sizeof( enable ) ) < 0 )
    {
        Utils::Logger( flags, Utils::FormatString( flags, "setsockopt() returned errno %d: %s", errno, strerror( errno ) ) );
        return false;
    }

    m_socket = socket;

    return true;
}

const void Server::NewConnection() const
{/*
        LOOKUP_DATA *ld = new LOOKUP_DATA();
        ld->m_brain = dnew;
        ld->buf = str_dup((char *) & sock.sin_addr);
*/
    bitset<CFG_MEM_MAX_BITSET> flags;
    struct sockaddr_in sin;
    uint_t descriptor;
    pthread_t lookup_thread;
    pthread_attr_t lookup_attr;
    socklen_t size = static_cast<socklen_t>( sizeof( sin ) );
    Socket* socket;

    flags.set( UTILS_DEBUG );
    flags.set( UTILS_TYPE_ERROR );

    if ( ::getsockname( server.gSocket()->gDescriptor(), reinterpret_cast<sockaddr*>( &sin ), &size ) < 0 )
    {
        Utils::Logger( flags, Utils::FormatString( flags, "getsockname() returned errno %d: %s", errno, strerror( errno ) ) );
        return;
    }

    if ( ( descriptor = ::accept( server.gSocket()->gDescriptor(), reinterpret_cast<sockaddr*>( &sin ), &size ) ) < 0 )
    {
        Utils::Logger( flags, Utils::FormatString( flags, "accept() returned errno %d: %s", errno, strerror( errno ) ) );
        return;
    }

    if ( ::fcntl( descriptor, F_SETFL, O_NDELAY ) < 0 )
    {
        Utils::Logger( flags, Utils::FormatString( flags, "fcntl() returned errno %d: %s", errno, strerror( errno ) ) );
        return;
    }

    socket = new Socket();
    socket->sDescriptor( descriptor );

    if ( ::getpeername( socket->gDescriptor(), reinterpret_cast<sockaddr*>( &sin ), &size ) < 0 )
    {
        Utils::Logger( flags, Utils::FormatString( flags, "getpeername() returned errno %d: %s", errno, strerror( errno ) ) );
        socket->sHost( "(unknown)" );
    }
    else
    {
        socket->sHost( inet_ntoa( sin.sin_addr ) );
        socket->sPort( ntohs( sin.sin_port ) );
        Utils::Logger( 0, Utils::FormatString( 0, "Server::NewConnection() :: %s:%lu", CSTR( socket->gHost() ), socket->gPort() ) );
    }

    return;
}

bool Server::PollSockets() const
{
    bitset<CFG_MEM_MAX_BITSET> flags;
    static struct timeval null_time;
    fd_set in_set;
    fd_set out_set;
    fd_set exc_set;
    ITER( Socket*, si );
    Socket* socket;
    sint_t max_desc = 0;

    flags.set( UTILS_DEBUG );
    flags.set( UTILS_TYPE_ERROR );

    FD_ZERO( &in_set );
    FD_ZERO( &out_set );
    FD_ZERO( &exc_set );

    FD_SET( server.gSocket()->gDescriptor(), &in_set );

    for ( si = socket_list.begin(); si != socket_list.end(); si++ )
    {
        socket = *si;

        max_desc = max( server.gSocket()->gDescriptor(), socket->gDescriptor() );
        FD_SET( socket->gDescriptor(), &in_set );
        FD_SET( socket->gDescriptor(), &out_set );
        FD_SET( socket->gDescriptor(), &exc_set );
    }

    if ( ::select( max_desc + 1, &in_set, &out_set, &exc_set, &null_time ) < 0 )
    {
        Utils::Logger( flags, Utils::FormatString( flags, "select() returned errno %d: %s", errno, strerror( errno ) ) );
        return false;
    }

    if ( FD_ISSET( server.gSocket()->gDescriptor(), &in_set ) )
        NewConnection();

    return true;
}

const void Server::Shutdown( const sint_t status )
{
    for_each( socket_list.begin(), socket_list.end(), Utils::DeleteObject() );

    m_shutdown = true;
}

const void Server::Update() const
{
    bitset<CFG_MEM_MAX_BITSET> flags;

    flags.set( UTILS_DEBUG );
    flags.set( UTILS_TYPE_ERROR );

    if ( !PollSockets() )
    {
        Utils::Logger( flags, Utils::FormatString( flags, "Error while calling PollSockets()" ) );
        server.Shutdown( EXIT_FAILURE );
        return;
    }

    return;
}

// Query
string Server::gTimeBoot() const
{
    string output;

    output = ctime( &m_time_boot );
    output.resize( output.length() - 1 );

    return output;
}

string Server::gTimeCurrent() const
{
    string output;

    output = ctime( &m_time_current );
    output.resize( output.length() - 1 );

    return output;
}

// Manipulate
bool Server::sPort( const uint_t port )
{
    if ( port <= CFG_SOC_MIN_PORTNUM || port >= CFG_SOC_MAX_PORTNUM )
        return false;

    m_port = port;

    return true;
}

bool Server::sSocket( Socket* socket )
{
    bitset<CFG_MEM_MAX_BITSET> flags;

    flags.set( UTILS_DEBUG );
    flags.set( UTILS_TYPE_ERROR );

    if ( !socket )
    {
        Utils::Logger( flags, "sSocket() called with invalid input" );
        return false;
    }

    if ( !socket->isValid() )
    {
        Utils::Logger( flags, "sSocket() called with invalid socket" );
        return false;
    }

    m_socket = socket;

    return true;
}

const void Server::sTimeBoot()
{
    struct timeval now;

    gettimeofday( &now, NULL );
    m_time_boot = now.tv_sec;

    return;
}

const void Server::sTimeCurrent()
{
    struct timeval now;

    gettimeofday( &now, NULL );
    m_time_current = now.tv_sec;

    return;
}

Server::Server()
{
    m_port = 0;
    m_shutdown = false;
    m_socket = 0;
    m_time_boot = 0;
    m_time_current = 0;

    return;
}

Server::~Server()
{
    return;
}
