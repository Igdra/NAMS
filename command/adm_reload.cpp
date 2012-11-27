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

#include "includes.h"
#include "class.h"
#include "plugin.h"

#include "command.h"

class AdmReload : public Plugin {
    public:
        virtual const void Run( SocketClient* client = NULL, const string& cmd = "", const string& arg = "" ) const;

        AdmReload( const string& name, const uint_t& type );
        ~AdmReload();
};

const void AdmReload::Run( SocketClient* client, const string& cmd, const string& arg ) const
{
    Command* newcmd = NULL;
    Command* oldcmd = NULL;
    string file;
    UFLAGS_DE( flags );

    if ( client )
    {
        if ( arg.empty() )
        {
            client->Send( "Reload -which- command?" CRLF );
            return;
        }

        if ( ( oldcmd = client->gServer()->FindCommand( arg ) ) != NULL )
        {
            if ( oldcmd->gCaller() == gCaller() )
            {
                client->Send( "Now that would be pointless, wouldn't it?" CRLF );
                return;
            }

            if ( oldcmd->Authorized( client->gSecurity() ) )
            {
                file = oldcmd->gFile();
                oldcmd->Delete();

                newcmd = new Command();
                if ( !newcmd->New( file ) )
                {
                    LOGFMT( flags, "AdmReload::Run()->Command::New()-> command %s returned false", CSTR( file ) );
                    delete newcmd;
                }

                client->Send( "Command successfully reloaded." CRLF );
            }
        }
        else
            client->Send( "That command doesn't exist." CRLF );
    }

    return;
}

AdmReload::AdmReload( const string& name = "::reload", const uint_t& type = PLG_TYPE_COMMAND ) : Plugin( name, type )
{
    Plugin::sBool( PLG_TYPE_COMMAND_BOOL_PREEMPT, true );
    Plugin::sUint( PLG_TYPE_COMMAND_UINT_SECURITY, SOC_SECURITY_ADMIN );

    return;
}

AdmReload::~AdmReload()
{
}

extern "C" {
    Plugin* New() { return new AdmReload(); }
    void Delete( Plugin* p ) { delete p; }
}