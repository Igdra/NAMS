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
 * @file event.cpp
 * @brief All non-template member functions of the Event class.
 *
 * Event objects will execute a function after a specified delay.
 */
#include "h/includes.h"
#include "h/class.h"

#include "h/event.h"
#include "h/list.h"

/* Core */
/**
 * @brief Create a new Event. This is a special use-case to create an Event on a server, for things such as Server::ReloadCommand.
 * @param[in] args The arguments to be passed to the function.
 * @param[in] server The server to execute the function on.
 * @param[in] type The type of Event.
 * @param[in] time How long to wait before executing Event::Run().
 * @retval false Returned if the event is unable to be created.
 * @retval true Returned if the event was successfully created.
 */
const bool Event::New( const string& args, Server* server, const uint_t& type, const uint_t& time )
{
    UFLAGS_DE( flags );

    if ( args.empty() )
    {
        LOGSTR( flags, "Event::New() called with empty args" );
        return false;
    }

    if ( server == NULL )
    {
        LOGSTR( flags, "Event::New() called with NULL server" );
        return false;
    }

    if ( type != EVENT_TYPE_RELOAD )
    {
        LOGSTR( flags, "Event::New() called with invalid type" );
        return false;
    }

    if ( time < uintmin_t )
    {
        LOGSTR( flags, "Event::New() called with invalid time" );
        return false;
    }

    m_args = args;
    m_time = time;
    m_type = type;

    event_list.push_front( this );

    return true;
}

/**
 * @brief Execute the function stored within the Event object.
 * @retval void
 */
const void Event::Run()
{
    switch ( m_type )
    {
        case EVENT_TYPE_RELOAD:
            m_server->ReloadCommand( m_args );
        break;

        default:
        break;
    }

    event_list.remove( this );

    return;
}

/**
 * @brief Updates the Event by decrementing the remaining time.
 * @retval false Returned once m_time reaches #uintmin_t and the Event can no longer update.
 * @retval true Returned if m_time is greater than #uintmin_t and the Event can still update.
 */
const bool Event::Update()
{
    if ( --m_time > uintmin_t )
        return true;
    else
        return false;
}

/* Query */
/**
 * @brief  Returns the amount of time remaining.
 * @retval uint_t The amount of time remaining.
 */
const uint_t Event::gTime() const
{
    return m_time;
}

/* Manipulate */

/* Internal */
/**
 * @brief Constructor for the Event class.
 */
Event::Event()
{
    m_args.clear();
    m_server = NULL;
    m_time = uintmin_t;
    m_type = uintmin_t;

    return;
}

/**
 * @brief Destructor for the Event class.
 */
Event::~Event()
{
    return;
}