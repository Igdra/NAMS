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
 * @file utils.h
 * @brief The Utils namespace.
 *
 *  This file contains the Utils namespace and template functions.
 */
#ifndef DEC_UTILS_H
#define DEC_UTILS_H

using namespace std;

/**
 * @brief The Utils namespace contains all general purpose, multi-use, and non-class functions.
 */
namespace Utils
{
    /**
     * @brief The DeleteObject class implements only operator() to be used in easily deleting lists of pointers or other objects.
     */
    class DeleteObject
    {
        public:
            /**
             * @brief Deletes a pointer.
             * @param[in] ptr The pointer to be deleted.
             * @retval void
             */
            template <class T> inline const void operator() ( const T* ptr ) const
            {
                delete ptr;
            }
    };

    /** @name Core */ /**@{*/
    /**
     * @brief Returns a string with all whitespace characters removed.
     * @param[in] t Any type of string to remove whitespace from.
     * @retval string A string with all whitespace characters removed.
     */
    template <class T> inline const string DelSpaces( const T& t )
    {
        string output( t );

        output.erase( remove_if( output.begin(), output.end(), ::isspace ), output.end() );

        return output;
    }
    /**
     * @brief Returns a string converted to all lowercase letters.
     * @param[in] t Any type of string to convert to lowercase.
     * @retval string A string converted to all lowercase letters.
     */
    template <class T> inline const string Lower( const T& t )
    {
        string output( t );

        transform( output.begin(), output.end(), output.begin(), ::tolower );

        return output;
    }
    /**
     * @brief Generates a string from the pair of t and v.
     * @param[in] t The first element to use in the pair.
     * @param[in] v The second element to use in the pair.
     * @retval string A string containing the pair of t and v.
     */
    template <class T, class V> inline const string MakePair( const T& t, const V& v )
    {
        stringstream output;

        output << CFG_DAT_STR_CTR_A << t << CFG_DAT_STR_CTR_B << v << CFG_DAT_STR_CTR_C;

        return output.str();
    }
    /**
     * @brief Returns a string converted to all uppercase letters.
     * @param[in] t Any type of string to convert to uppercase.
     * @retval string A string converted to all uppercase letters.
     */
    template <class T> inline const string Upper( const T& t )
    {
        string output( t );

        transform( output.begin(), output.end(), output.begin(), ::toupper );

        return output;
    }
    /**
     * @brief Returns a string object from any data type: bool, int, char, etc.
     * @param[in] t Any type of data that can be represented as an alphanumeric string.
     * @retval string A string containing the converted data.
     */
    template <class T> inline const string String( const T& t )
    {
        stringstream ss;

        ss << t;

        return ss.str();
    }
    const uint_t DirExists( const string& dir );
    const string DirPath( const string& dir, const string& file, const string& ext = "" );
    const string FileExt( const string& file, const string& ext );
    #define FormatString( flags, fmt, ... ) _FormatString( PP_NARG( __VA_ARGS__ ), flags, _caller_, fmt, ##__VA_ARGS__ )
    #define Logger( flags, fmt, ... ) _Logger( PP_NARG( __VA_ARGS__ ), flags, _caller_, fmt, ##__VA_ARGS__ )
    const uint_t NumChar( const string& input, const string& item );
    const uint_t ReadIndex( const string& input );
    const pair<string,string> ReadPair( const string& input );
    const string ReadString( ifstream& input );
    const string Salt( const string& input );
    const vector<string> StrNewlines( const string& input );
    const bool StrPrefix( const string& s1, const string& s2, const bool& igncase = false );
    const string StrTime( const time_t& now = chrono::high_resolution_clock::to_time_t( chrono::high_resolution_clock::now() ) );
    const vector<string> StrTokens( const string& input, const bool& quiet = false );
    const string WriteString( const string& input );
    /**@}*/

    /** @name Query */ /**@{*/
    /**
     * @brief Checks to ensure an input is entirely alphanumeric.
     * @param[in] t The input to be checked.
     * @retval false Returned if a non-alphanumeric is found.
     * @retval true Returned if no alphanumerics are found.
     */
    template <class T> inline const bool iAlNum( const T& t )
    {
        uint_t i = uintmin_t;
        string input( t );

        while( i < input.length() )
        {
            if ( !::isalnum( input[i] ) )
                return false;

            ++i;
        }

        return true;
    }
    const bool iDirectory( const string& dir );
    const bool iFile( const string& file );
    const bool iName( const string& name, const string& input );
    const bool iNumber( const string& input );
    const bool iReadable( const string& file );
    /**@}*/

    /** @name Manipulate */ /**@{*/
    const string Argument( string& input, const string& delim = " " );
    const void CleanupTemp( uint_t& dir_close, uint_t& dir_open );
    const bool FileOpen( ofstream& ofs, const string& file );
    const bool FileOpen( ifstream& ifs, const string& file, const bool& quiet = false );
    const bool FileClose( ifstream& ifs, const bool& quiet = false );
    const bool FileClose( ofstream& ofs );
    const bool FileClose( ofstream& ofs, const string& dir, const string& file );
    /**
     * @brief Splits a string in the format of key=value. Retains any whitespace in the value.
     * @param[in] key The object to be populated with the key extracted from item.
     * @param[in] val The object to be populated with the value extracted from item.
     * @param[in] item A char or string in the form of key=value to extract data from.
     * @retval false Returned if item does not contain an equal sign.
     * @retval true Returned if data was successfully extracted from item.
     */
    template <class K, class V, class I> inline const bool KeyValue( K& key, V& val, const I& item )
    {
        size_t loc = 0;

        if ( ( loc = item.find( "=" ) ) == string::npos )
            return false;

        key = item.substr( 0, loc - 1 );
        val = item.substr( loc + 1, item.length() );
        loc = key.find_last_not_of( " " );
        key.erase( loc + 1 );
        loc = val.find_first_not_of( " " );
        val.erase( 0, loc );

        return true;
    }
    /**
     * @brief If the contents of keyd == valu, assigns loc = item.
     * @param[in] igncase If true, performs case-insensitive matching.
     * @param[in] found For loop control. If keyd == valu, set to true, otherwise false.
     * @param[in] keyd A string or number that valu must match.
     * @param[in] valu A string or number that is checked for a match against keyd.
     * @param[in] item A string to be copied to to loc if keyd == valu.
     * @param[in] loc A string to be assigned the value of item if keyd == valu.
     * @retval void
     */
    template <class K, class V> inline const void KeySet( const bool& igncase, bool& found, const K& keyd, const V& valu, const string& item, string& loc )
    {
        string key( keyd );
        string val( valu );

        // Someone else already found a value this loop
        if ( found )
            return;

        if ( igncase )
        {
            transform( key.begin(), key.end(), key.begin(), ::tolower );
            transform( val.begin(), val.end(), val.begin(), ::tolower );
        }

        if ( key == val )
        {
            loc = item;
            found = true;

            return;
        }

        return;
    }
   /**
     * @brief If the contents of keyd == valu, assigns loc = item.
     * @param[in] igncase If true, performs case-insensitive matching.
     * @param[in] found For loop control. If keyd == valu, set to true, otherwise false.
     * @param[in] keyd A string or number that valu must match.
     * @param[in] valu A string or number that is checked for a match against keyd.
     * @param[in] item Any non-string to be copied to to loc if keyd == valu.
     * @param[in] loc Any non-string to be assigned the value of item if keyd == valu.
     * @param[in] maxv The maximum value to allow loc to be assigned. If value > maxv, loc = maxv.
     * @param[in] maxb KeySet will set to true if value >= maxv, otherwise false. Allows logging from calling function.
     * @retval void
     */
    template <class K, class V, class I, class L> inline const void KeySet( const bool& igncase, bool& found, const K& keyd, const V& valu, const I& item, L& loc, const uint_t& maxv, bool& maxb )
    {
        UFLAGS_I( flags );
        string key( keyd );
        string val( valu );
        L maxt;

        // Someone else already found a value this loop
        if ( found )
            return;

        if ( igncase )
        {
            transform( key.begin(), key.end(), key.begin(), ::tolower );
            transform( val.begin(), val.end(), val.begin(), ::tolower );
        }

        if ( key == val )
        {
            string tf( item );

            // Allow bools be any of: true / 1 or false / 0
            transform( tf.begin(), tf.end(), tf.begin(), ::tolower );

            if ( tf == "true" )
                loc = true;
            else if ( tf == "false" )
                loc = false;
            else
            {
                stringstream( item ) >> maxt;

                if ( maxt > maxv )
                {
                    maxb = true;
                    maxt = maxv;
                }

                loc = maxt;
            }
            found = true;

            return;
        }

        return;
    }
    const multimap<bool,string> ListDirectory( const string& dir, const bool& recursive, const bool& path, multimap<bool,string>& output, uint_t& dir_close, uint_t& dir_open );
    /**@}*/

    /** @name Internal */ /**@{*/
    const string _FormatString( const uint_t& narg, const bitset<CFG_MEM_MAX_BITSET>& flags, const string& caller, const string& fmt, ... );
    const string __FormatString( const uint_t& narg, const bitset<CFG_MEM_MAX_BITSET>& flags, const string& caller, const string& fmt, va_list& val );
    const void _Logger( const uint_t& narg, const bitset<CFG_MEM_MAX_BITSET>& flags, const string& caller, const string& fmt, ... );
    /**@}*/
};

#endif
