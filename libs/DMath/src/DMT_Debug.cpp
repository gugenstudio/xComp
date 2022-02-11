//==================================================================
/// DMT_Debug.cpp
///
/// Created by Davide Pasca - 2015/8/9
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#include <stdio.h>
#include <string>
#include <algorithm>
#include <stdarg.h>
#include "DSafeCrt.h"
#include "DMT_Debug.h"

#if defined(ANDROID)
# include <android/log.h>
#endif

//==================================================================
namespace DMTD
{

//==================================================================
void loc_vprintf(const char *fmt, va_list vl)
{
#if defined(ANDROID)
    __android_log_vprint(ANDROID_LOG_INFO, "oyatsukai", fmt, vl);
#else
    vprintf(fmt, vl);
#endif
}

//==================================================================
void loc_printf(const char *fmt, ... )
{
	va_list	vl;
	va_start( vl, fmt );

    loc_vprintf(fmt, vl);
}

//==================================================================
template <typename T, size_t D>
static void printNiceMatrixBase( const T &m )
{
    std::string ms[D][D];
    size_t maxw = 0;
    for (size_t i=0; i != D; ++i)
    {
        for (size_t j=0; j != D; ++j)
        {
            char buff[128];
            snprintf( buff, sizeof(buff), "% 1.2f", m.mij(i,j) );
            ms[i][j] = buff;
            maxw = std::max( maxw, ms[i][j].size() );
        }
    }

    std::string str;
    for (size_t i=0; i != D; ++i)
    {
        for (size_t j=0; j != D; ++j)
        {
            size_t pad = maxw - ms[i][j].size();
            char buff[128];
            for (size_t k=0; k != pad; ++k)
                buff[k] = ' ';
            buff[pad] = 0;
            strcat( buff, ms[i][j].c_str() );
            str += buff;
            str += ' ';
        }
        str += '\n';
    }
    loc_printf( "%s", str.c_str() );
}

//==================================================================
void PrintNiceMatrix( const Matrix33 &m )
{
    printNiceMatrixBase<Matrix33,3>( m );
}

void PrintNiceMatrix( const Matrix44 &m )
{
    printNiceMatrixBase<Matrix44,4>( m );
}

//==================================================================
template <typename T>
static void printVecN(
        const T *pVals,
        size_t n,
        const char *pTitle,
        int places,
        int dec )
{
    char buff[256] = {0};

    if ( pTitle )
    {
        strcpy_s( buff, pTitle );
        strcat_s( buff, ":" );
    }

    for (size_t i=0; i != n; ++i)
    {
        char tmp[128];

        if ( i != 0 )
            strcat_s( buff, " " );

        if ( std::is_integral<T>::value )
        {
            if ( std::is_unsigned<T>::value )
                snprintf( tmp, sizeof(tmp), "%*u", places, (unsigned)pVals[i] );
            else
                snprintf( tmp, sizeof(tmp), "% *i", places, (int)pVals[i] );

        }
        else
        {
            snprintf( tmp, sizeof(tmp), "% *.*f", places, dec, (double)pVals[i] );
        }

        strcat_s( buff, tmp );
    }

    loc_printf( "%s", buff );
}

//==================================================================
void PrintNiceBool( bool val, const char *pTitle )
{
    auto valui = (unsigned int)(val ? 1 : 0);
    printVecN( &valui, 1, pTitle, 1, 0 );
}

//==================================================================
void PrintNiceNormal( const Float3 &n, const char *pTitle )
{
    printVecN( &n[0], 3, pTitle, 1, 2 );
}

//==================================================================
void PrintNiceFloat( const float n, const char *pTitle, int places, int dec )
{
    printVecN( &n, 1, pTitle, places, dec );
}

//==================================================================
void PrintNiceVec2( const Float2 &n, const char *pTitle, int places, int dec )
{
    printVecN( &n[0], 2, pTitle, places, dec );
}

//==================================================================
void PrintNiceVec3( const Float3 &n, const char *pTitle, int places, int dec )
{
    printVecN( &n[0], 3, pTitle, places, dec );
}

//==================================================================
void PrintNiceVec4( const Float4 &n, const char *pTitle, int places, int dec )
{
    printVecN( &n[0], 4, pTitle, places, dec );
}

//==================================================================
void PrintNiceQuat( const Quat &n, const char *pTitle )
{
    auto v4 = Float4( n[0], n[1], n[2], n[3] );
    printVecN( &v4[0], 4, pTitle, 5, 3 );
}

//==================================================================
void PrintSep()
{
    loc_printf( " | " );
}

//==================================================================
void PrintNL()
{
    loc_printf( "\n" );
}


//==================================================================
}

