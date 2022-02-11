//==================================================================
/// DSafeCrt.h
///
/// Created by Davide Pasca - 2009/07/25
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#include <string.h>
#include <errno.h>
#include "DBase.h"
#include "DSafeCrt.h"

//===============================================================
#if !defined(_MSC_VER)

//==================================================================
errno_t fopen_s( FILE **out_ppFile, const char *pFName, const char *pMode )
{
	if ( (*out_ppFile = fopen( pFName, pMode )) )
		return 0;
	else
		return -1;
}

//==================================================================
void strtime( char *pDest, size_t maxLen )
{
    time_t t = time( NULL );

    strcpy_s( pDest, maxLen, ctime( &t ) );
}

//==================================================================
int vsnprintf_s( char *str, size_t strMaxLen, size_t size, const char *format, va_list ap )
{
	int len = vsnprintf( str, size, format, ap );

	// verify only later.. better than nothing !
	DASSERT( (size_t )(len+1) <= strMaxLen );

	return len;
}

//==================================================================
errno_t strcpy_s( char *pDest, size_t destSize, const char *pSrc )
{
    if ( !pSrc || !pDest )
    {
        DASSERT( 0 );
        return EINVAL;
    }

    size_t len = strlen( pSrc );
    if ( (len+1) > destSize )
    {
        pDest[0] = 0;
        DASSERT( 0 );
        return ERANGE;
    }

    strcpy( pDest, pSrc );
    return 0;
}

//==================================================================
errno_t strcat_s( char *pDest, size_t destSize, const char *pSrc )
{
    if ( !pSrc || !pDest )
    {
        DASSERT( 0 );
        return EINVAL;
    }

    size_t srcLen = strlen( pSrc );
    size_t curDestLen = strlen( pDest );

    if ( (curDestLen+srcLen+1) > destSize )
    {
        pDest[0] = 0;
        DASSERT( 0 );
        return ERANGE;
    }

    strcat( pDest, pSrc );
    return 0;
}

//==================================================================
int vsprintf_s( char *str, const char *pFmt, va_list vl )
{
	return vsprintf( str, pFmt, vl );
}

//==================================================================
int vsprintf_s( char *str, size_t destMaxSize, const char *pFmt, va_list vl )
{
	return vsprintf( str, pFmt, vl );
}

//==================================================================
int sprintf_s( char *str, const char *format, ...)
{
	int	ret;

	va_list	vl;
	va_start( vl, format );

	ret = vsprintf_s( str, format, vl );

	va_end( vl );

	return ret;
}

//==================================================================
int sprintf_s( char *str, size_t destMaxSize, const char *format, ...)
{
	int	ret;

	va_list	vl;
	va_start( vl, format );

	ret = vsprintf_s( str, format, vl );

	va_end( vl );

	return ret;
}

#endif

//==================================================================
void numstrdate( char *pDest, size_t maxLen )
{
	time_t 		t;
	struct tm	*tm;

	t 	= time( NULL );
	tm	= localtime( &t );

	sprintf( pDest, "%i/%02i/%02i",
		tm->tm_year+1900,
		tm->tm_mon+1,
		tm->tm_mday );
}

//==================================================================
char *vsnprintf_exp(
        char *str,
        size_t strSize,
        std::vector<char> &altBuff,
        const char *format,
        va_list ap )
{
    // make a copy for possible additional attempt later
    va_list ap_copy;
    va_copy( ap_copy, ap );

	int len = vsnprintf( str, strSize, format, ap );

    // special case for "special" Microsoft
#ifdef _MSC_VER
    // means buffer wasn't enough.. no idea how much
    if ( len == -1 )
    {
        // assume 128K will be enough !
        len = 128 * 1024;
    }
#endif

    DASSERT( len >= 0 );

    if ( len < (int)strSize )
        return str;

    // if it's too big, then use the alternate vector

    altBuff.resize( len+1 );

	len = vsnprintf( &altBuff[0], altBuff.size(), format, ap_copy );

    DASSERT( len >= 0 && len < (int)altBuff.size() ); (void)len;

	return &altBuff[0];
}

