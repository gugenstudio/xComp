//==================================================================
/// DExceptions.h
///
/// Created by Davide Pasca - 2018/02/09
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#ifndef DEXCEPTIONS_H
#define DEXCEPTIONS_H

#include <new>
#include <stdexcept>
#include <memory>
#include <string>
#include <string.h>
#include <functional>

#include <stdio.h>
#include <stdarg.h>
#include <sstream>

//==================================================================
inline const char *DEX_SeekFilename( const char *pStr )
{
    return
        (strrchr(pStr,'/') ? strrchr(pStr,'/')+1 : \
            (strrchr(pStr,'\\') ? strrchr(pStr,'\\')+1 : pStr) );
}

//==================================================================
#define __DSHORT_FILE__ DEX_SeekFilename( __FILE__ )

//==================================================================
inline std::string DEX_MakeString( const char *pFmt, ... )
{
    constexpr size_t BUFF_LEN = 2048;

    char buff[ BUFF_LEN ];
    va_list args;
    va_start( args, pFmt );
    vsnprintf( buff, sizeof(buff), pFmt, args );
    va_end(args);

    buff[ BUFF_LEN-1 ] = 0;

    return buff;
}

//==================================================================
class DEX_ConnectionError : public std::exception
{
    const std::string mWhatStr;

public:
    DEX_ConnectionError( const std::string &whatStr )
        : std::exception()
        , mWhatStr(whatStr)
    {
    }

    virtual const char* what() const noexcept { return mWhatStr.c_str(); }
};

//==================================================================
# define DEX_RUNTIME_ERROR(_FMT_,...) \
    throw std::runtime_error( \
        DEX_MakeString( "[%s:%i] " _FMT_, __DSHORT_FILE__, __LINE__, ##__VA_ARGS__ ) )

# define DEX_OUT_OF_RANGE(_FMT_,...) \
    throw std::out_of_range( \
        DEX_MakeString( "[%s:%i] " _FMT_, __DSHORT_FILE__, __LINE__, ##__VA_ARGS__ ) )

# define DEX_BAD_ALLOC(_FMT_,...) { \
    printf( "%s\n", \
        DEX_MakeString( \
            "[%s:%i] " _FMT_, __DSHORT_FILE__, __LINE__, ##__VA_ARGS__ ).c_str() ); \
    throw std::bad_alloc(); }

# define DEX_CONNECTION_ERROR(_FMT_,...) \
    throw DEX_ConnectionError( \
        DEX_MakeString( "[%s:%i] " _FMT_, __DSHORT_FILE__, __LINE__, ##__VA_ARGS__ ) )

//==================================================================
std::tuple<std::string,int,std::string> DEX_ParseExceptString( const std::string &src );

std::string DEX_CatchExceptionString( std::function<void ()> fn );

#endif

