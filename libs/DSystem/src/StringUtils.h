//==================================================================
/// StringUtils.h
///
/// Created by Davide Pasca - 2018/02/10
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#ifndef STRINGUTILS_H
#define STRINGUTILS_H

#include <stdarg.h>
#include <string>
#include <map>
#include "DBase.h"
#include "DContainers.h"

void StrToUpper( DStr &str );
void StrToUpper( char *pStr );
void StrToLower( DStr &str );
void StrToLower( char *pStr );
inline DStr StrMakeLower( DStr str ) { StrToLower( str ); return str; }
inline DStr StrMakeUpper( DStr str ) { StrToUpper( str ); return str; }

const char *StrStrI_( const char *pStr, const char *pSearch );

inline bool StrContains( const char *pStr, const char *pSearch ) {
    return !!StrStrI_( pStr, pSearch );
}
inline bool StrContains( const DStr &str, const char *pSearch ) {
    return !!StrStrI_( str.c_str(), pSearch );
}
inline bool StrContains( const DStr &str, const DStr &search ) {
    return !!StrStrI_( str.c_str(), search.c_str() );
}

bool StrStartsWithI( const char *pStr, const char *pSearch );
bool StrEndsWithI( const char *pStr, const char *pSearch );
bool StrEndsWith( const char *pStr, const char *pSearch );

inline bool StrStartsWithI( const DStr &str, const DStr &search )
{
    return StrStartsWithI( str.c_str(), search.c_str() );
}

inline bool StrEndsWithI( const DStr &str, const DStr &search )
{
    return StrEndsWithI( str.c_str(), search.c_str() );
}

inline bool StrEndsWith( const DStr &str, const DStr &search )
{
    return StrEndsWith( str.c_str(), search.c_str() );
}

inline int StrCaseCmd( const DStr &l, const DStr &r )
{
    return strcasecmp( l.c_str(), r.c_str() );
}

DVec<DStr> StrSplitLines( const DStr &src, bool includeIncompleteEnd=true );

DStr StrReplaceAll( DStr str, const DStr &from, const DStr &to );

void StrReplaceMacros( const std::map<DStr,DStr> &macros, DStr &io_txt );

inline void StrReplaceMacros( const std::map<DStr,DStr> &macros, DVec<DStr> &io_txts )
{
    for (auto &x : io_txts)
        StrReplaceMacros( macros, x );
}

template <size_t N>
inline void StrReplaceMacros( const std::map<DStr,DStr> &macros, DStr (&io_txts)[N] )
{
    for (size_t i=0; i != N; ++i)
        StrReplaceMacros( macros, io_txts[i] );
}

template <size_t N>
inline void StrReplaceMacros( const std::map<DStr,DStr> &macros, std::array<DStr,N> &io_txts )
{
    for (size_t i=0; i != N; ++i)
        StrReplaceMacros( macros, io_txts[i] );
}

DStr &VSSPrintFS( DStr &useStr, const char *pFmt, va_list vl );
DStr &SSPrintFS( DStr &useStr, const char *pFmt, ... );
DStr SSPrintFS( const char *pFmt, ... );

inline size_t StrToSizeT( const DStr &s ) { return std::stoull( s, nullptr, 10 ); }
inline int    StrToInt( const DStr &s )   { return std::stoi( s ); }
inline u_int  StrToUInt( const DStr &s )  { return (u_int)std::stoull( s, nullptr, 10 ); }
inline uint16_t StrToUInt16(const DStr &s) { return (uint16_t)std::stoull( s, nullptr, 10 ); }
inline int64_t StrToInt64( const DStr &s ) { return (int64_t)std::stoull( s, nullptr, 10 ); }
#if defined(__clang__)
inline uint64_t StrToUInt64( const DStr &s ) { return (uint64_t)std::stoull( s, nullptr, 10 ); }
#endif
inline bool   StrToBool( const DStr &s )  { return (bool)std::stoi( s ); }
inline double StrToDouble( const DStr &s ){ return std::stod( s ); }
inline float  StrToFloat( const DStr &s ) { return std::stof( s ); }

inline void StrToNumber( size_t &out_n, const DStr &s ) { out_n = StrToSizeT( s ); }
inline void StrToNumber( int    &out_n, const DStr &s ) { out_n = StrToInt( s ); }
inline void StrToNumber( u_int  &out_n, const DStr &s ) { out_n = StrToUInt( s ); }
inline void StrToNumber( uint16_t &out_n,const DStr &s) { out_n = StrToUInt( s ); }
inline void StrToNumber( int64_t &out_n,const DStr &s ) { out_n = StrToInt64( s ); }
#if defined(__clang__)
inline void StrToNumber( uint64_t &out_n,const DStr &s ) { out_n = StrToUInt64( s ); }
#endif
inline void StrToNumber( bool   &out_n, const DStr &s ) { out_n = (bool)StrToInt( s ); }
inline void StrToNumber( double &out_n, const DStr &s ) { out_n = StrToDouble( s ); }
inline void StrToNumber( float  &out_n, const DStr &s ) { out_n = StrToFloat( s ); }

DStr StrTrimWhiteLR( DStr s );

inline DStr DToString( const DStr    &v ) { return                v;  }
inline DStr DToString( const size_t  &v ) { return std::to_string(v); }
inline DStr DToString( const int     &v ) { return std::to_string(v); }
inline DStr DToString( const u_int   &v ) { return std::to_string(v); }
inline DStr DToString( const int64_t &v ) { return std::to_string(v); }
#if defined(__clang__)
inline DStr DToString( const uint64_t &v ) { return std::to_string(v); }
#endif
inline DStr DToString( const bool    &v ) { return std::to_string(v); }
inline DStr DToString( const double  &v ) { return std::to_string(v); }
inline DStr DToString( const void    *v ) { return SSPrintFS( "0x%p", v ); }

DStr DToString( const DVec<DStr> &v );

#define DMAKE_VAR_STR( _F_ )         (DStr(#_F_) + ":" + DToString(_F_) + ", ")
#define DMAKE_VA2_STR( _F_, _VAL_ )  (DStr(#_F_) + ":" + DToString(_VAL_) + ", ")
#define DMAKE_FLD_STR( _OBJ_, _F_ )  (DStr(#_F_) + ":" + DToString(_OBJ_._F_) + ", ")

//==================================================================
inline DStr StrFromRealCompact( double val )
{
    auto str = std::to_string( val );
    str.erase( str.find_last_not_of('0') + 1, DStr::npos );
    str.erase( str.find_last_not_of('.') + 1, DStr::npos );

    return str;
}

inline DStr StrStripTralingZeros( DStr str )
{
    c_auto pos = str.find_first_of('.');
    if ( pos == DStr::npos )
        return str;

    // do nothing if it has too few decimal numbers
    if ( (str.size() - pos) <= (3+1) )
        return str;

    str.erase( str.find_last_not_of('0') + 1, DStr::npos );
    str.erase( str.find_last_not_of('.') + 1, DStr::npos );
    return str;
}

inline char *StrStripTralingZeros( char *pStr )
{
    if (c_auto n = (ptrdiff_t)strlen( pStr ))
    {
        ptrdiff_t startI = 0;
        for (; startI < n; ++startI)
        {
            if ( pStr[startI] == '.' )
            {
                auto i = (ptrdiff_t)n - 1;

                while ( i > startI && pStr[i] == '0' )
                    pStr[i--] = 0;

                // remove the dot if there's nothing after
                if ( i == startI )
                    pStr[i] = 0;
            }
        }
    }

    return pStr;
}

inline DStr StrFromRealFixLen( double val, size_t fixN )
{
    auto str = std::to_string( val );
    c_auto n = str.size();
    if ( n < fixN )
    {
        // appen zeroes if too short
        str.append( DStr( fixN - n, '0' ) );
    }
    else
    if ( n > fixN )
    {
        // make a compact version
        str = StrFromRealCompact( val );
        if ( str.size() < fixN )
        {
            // ...if too short, then append .0 to fill
            if ( str.find_first_of('.') == DStr::npos )
                str += '.' ;

            str.append( DStr( fixN - str.size(), '0' ) );
        }
    }

    return str;
}

//
DStr Str1000CommaFromStr( const DStr &src );

//===============================================================
// see: https://stackoverflow.com/a/56833374/1507238
inline DStr StrFromU8Str(const DStr &s)
{
	return s;
}
inline DStr StrFromU8Str(DStr &&s)
{
	return std::move(s);
}

#if defined(__cpp_lib_char8_t)
inline DStr StrFromU8Str(const std::u8string &s)
{
	return DStr(s.begin(), s.end());
}
#endif

#endif

