//==================================================================
/// StringUtils.cpp
///
/// Created by Davide Pasca - 2018/02/10
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#include <algorithm>
#include <cctype>
#include <locale>
#include "DSafeCrt.h"
#include "StringUtils.h"

//==================================================================
void StrToUpper( DStr &str )
{
	for (size_t i=0; i < str.size(); ++i)
		str[i] = toupper( str[i] );
}
//==================================================================
void StrToUpper( char *pStr )
{
	for (size_t i=0; pStr[i]; ++i)
		pStr[i] = toupper( pStr[i] );
}

//==================================================================
void StrToLower( DStr &str )
{
	for (size_t i=0; i < str.size(); ++i)
		str[i] = tolower( str[i] );
}
//==================================================================
void StrToLower( char *pStr )
{
    for (size_t i=0; pStr[i]; ++i)
        pStr[i] = tolower( pStr[i] );
}

//==================================================================
const char *StrStrI_( const char *pStr, const char *pSearch )
{
    // ANSI strstr behavior: it's a match if search is empty
    if NOT( pSearch[0] )
        return pStr;

    for (size_t i=0; pStr[i]; ++i)
    {
        if ( tolower(pSearch[0]) == tolower(pStr[i]) )
        {
            size_t  j = 0;
            for (; pSearch[j]; ++j)
                if ( tolower(pStr[i+j]) != tolower(pSearch[j]) )
                    break;

            // reached the end of the search string ?
            if ( pSearch[j] == 0 )
                return pStr + i;    // it's a match
        }
    }

    return NULL;
}

//==================================================================
bool StrStartsWithI( const char *pStr, const char *pSearch )
{
    return pStr == StrStrI_( pStr, pSearch );
}

//==================================================================
bool StrEndsWithI( const char *pStr, const char *pSearch )
{
    c_auto strLen = strlen( pStr );
    c_auto searchLen = strlen( pSearch );

    if ( searchLen > strLen )
        return false;

    return 0 == strcasecmp( pStr + strLen - searchLen, pSearch );
}

//==================================================================
bool StrEndsWith( const char *pStr, const char *pSearch )
{
    c_auto strLen = strlen( pStr );
    c_auto searchLen = strlen( pSearch );

    if ( searchLen > strLen )
        return false;

    return 0 == strcmp( pStr + strLen - searchLen, pSearch );
}

//===============================================================
DVec<DStr> StrSplitLines( const DStr &src, bool includeIncompleteEnd )
{
    DVec<DStr> vec;

#if 0
    std::stringstream ss( src );

    while ( std::getline( ss, vec.emplace_back(), '\n') )
    {
    }
#else
    DStr tmp;
    for (c_auto &ch : src)
    {
        if ( ch == '\n' )
        {
            vec.push_back( tmp );
            tmp.clear();
        }
        else
        {
            tmp += ch;
        }
    }

    if ( includeIncompleteEnd && !tmp.empty() )
        vec.push_back( tmp );
#endif

    return vec;
}

//===============================================================
// see: https://stackoverflow.com/a/24315631/1507238
DStr StrReplaceAll( DStr str, const DStr &from, const DStr &to )
{
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != DStr::npos)
    {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
    }

    return str;
}

//==================================================================
void StrReplaceMacros( const std::map<DStr,DStr> &macros, DStr &io_txt )
{
    for (c_auto [k, v] : macros)
    {
        // Get the first occurrence
        size_t pos = io_txt.find( k );

        // Repeat till end is reached
        while( pos != std::string::npos )
        {
            // Replace this occurrence of Sub String
            io_txt.replace( pos, k.size(), v );

            // Get the next occurrence from the current position
            pos = io_txt.find( k, pos + v.size() );
        }
    }
}

//===============================================================
DStr &VSSPrintFS( DStr &useStr, const char *pFmt, va_list vl )
{
    char buff[ 512 ];
    std::vector<char> altBuff;
    char *pOutStr = vsnprintf_exp( buff, sizeof(buff), altBuff, pFmt, vl );

    useStr.assign( pOutStr );

	return useStr;
}

//==================================================================
DStr &SSPrintFS( DStr &useStr, const char *pFmt, ... )
{
	va_list	vl;
	va_start( vl, pFmt );
	VSSPrintFS( useStr, pFmt, vl );
	va_end( vl );

	return useStr;
}

//==================================================================
DStr SSPrintFS( const char *pFmt, ... )
{
	va_list	vl;
	va_start( vl, pFmt );
    auto tmpStr = DStr();
	VSSPrintFS( tmpStr, pFmt, vl );
	va_end( vl );

	return tmpStr;
}

//==================================================================
// trim from start (in place)
static inline void ltrim(std::string &s)
{
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
        return !std::isspace(ch);
    }));
}

// trim from end (in place)
static inline void rtrim(std::string &s)
{
    s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}

// trim from both ends (in place)
static inline void trim(std::string &s)
{
    ltrim(s);
    rtrim(s);
}

//==================================================================
DStr StrTrimWhiteLR( DStr s )
{
    trim( s );
    return s;
}

//==================================================================
DStr DToString( const DVec<DStr> &v )
{
    DStr out( "{" );
    for (size_t i=0; i < v.size(); ++i)
        out += (i > 0 ? DStr(",") : DStr()) + v[i];

    return out + '}';
}

//==================================================================
DStr Str1000CommaFromStr( const DStr &src )
{
    if ( src.empty() )
        return {};

    static constexpr char SEP = '\'';

    c_auto len = src.size();

    DStr tmp;

    tmp.reserve( len + len/3 );

    size_t dotPos = 0;
    size_t i0 = 0;

    if (c_auto pos = src.find_last_of( '.' ); pos != DStr::npos)
    {
        dotPos = pos;
        i0 = src.size() - dotPos;
    }

    c_auto noNumFront = src[0] < '0' || src[0] > '9';

    c_auto n = len - (noNumFront ? 1 : 0);

    for (size_t i=i0; i < n; ++i)
    {
        tmp.push_back( src[ len-1-i ] );

        if ( (((i - i0) + 1) % 3) == 0 && (i != (n-1)) )
            tmp.push_back( SEP );
    }

    if ( noNumFront )
        tmp.push_back( src[0] );

    return DStr{ tmp.rbegin(), tmp.rend() } +
                (dotPos != 0 ? src.substr( dotPos ) : DStr{});
}

