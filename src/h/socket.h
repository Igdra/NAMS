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
#ifndef DEC_SOCKET_H
#define DEC_SOCKET_H

#include "utils.h"

class Socket : public Utils {
    public:
        Socket();
        ~Socket();

        // Core
        bool Valid() const { return m_descriptor > 0; }

        // Query
        uint_t gBytesRecvd() const { return m_bytes_recvd; }
        uint_t gBytesSent() const { return m_bytes_sent; }
        sint_t gDescriptor() const { return m_descriptor; }
        string gHost() const { return m_host; }
        uint_t gPort() const { return m_port; }

        // Manipulate
        bool sBytesRecvd( const uint_t amount );
        bool sBytesSent( const uint_t amount );
        bool sDescriptor( const sint_t descriptor );
        bool sHost( const string host );
        bool sPort( const uint_t port );

    protected:
        uint_t  m_bytes_recvd;
        uint_t  m_bytes_sent;
        sint_t  m_descriptor;
        string  m_host;

    private:
        uint_t  m_port;
};

#endif
