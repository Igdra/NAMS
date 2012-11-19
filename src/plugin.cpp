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
 * @file plugin.cpp
 * @brief All non-trivial member functions of the Plugin class.
 */
#include "h/includes.h"
#include "h/class.h"

#include "h/plugin.h"

/** @name Core */ /**@{*/
/**@}*/

/** @name Query */ /**@{*/
/**@}*/

/** @name Manipulate */ /**@{*/
/**@}*/

/** @name Internal */ /**@{*/
Plugin::Plugin( const string& name, const uint_t& type )
{
    m_bool[CFG_PLG_MAX_ARR] = { false };
    m_name = name;
    m_string[CFG_PLG_MAX_ARR] = { string() };
    m_type = type;
    m_uint_t[CFG_PLG_MAX_ARR] = { uintmin_t };

    return;
}

Plugin::~Plugin()
{
    return;
}
/**@}*/