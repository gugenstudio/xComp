//==================================================================
/// DSafeCrt.h
///
/// Created by Davide Pasca - 2009/07/25
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#ifndef DSAFECRT_H
#define DSAFECRT_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>
#include <vector>
#include <string.h>

//===============================================================
#if !defined(_MSC_VER)

#define fprintf_s	fprintf
#define vfprintf_s	vfprintf
#define _strdate_s	strdate
#define _strtime_s	strtime
#define sscanf_s	sscanf

//#define vsnprintf_s(_DST_,_DSTSIZ_,_MAXCNT_,_FMT_,_ARGS_)	vsnprintf(_DST_,_MAXCNT_,_FMT_,_ARGS_)

int vsprintf_s( char *str, const char *pFmt, va_list vl );
int vsprintf_s( char *str, size_t destMaxSize, const char *pFmt, va_list vl );
int sprintf_s( char *str, const char *format, ...);
int sprintf_s( char *str, size_t destMaxSize, const char *format, ...);

#define	_getcwd		getcwd

typedef int	errno_t;

errno_t fopen_s( FILE **out_ppFile, const char *pFName, const char *pMode );

int vsnprintf_s( char *str, size_t strMaxLen, size_t size, const char *format, va_list ap );

void strtime( char *pDest, size_t maxLen=0 );

errno_t strcpy_s( char *pDest, size_t destSize, const char *pSrc );
errno_t strcat_s( char *pDest, size_t destSize, const char *pSrc );

template <size_t _SIZE>
errno_t strcpy_s( char (&dest)[_SIZE], const char *pSrc ) {
    return strcpy_s( dest, _SIZE, pSrc );
}

template <size_t _SIZE>
errno_t strcat_s( char (&dest)[_SIZE], const char *pSrc ) {
    return strcat_s( dest, _SIZE, pSrc );
}

inline void *memcpy_s(void *dest, const void *src, size_t siz)
{
    return memcpy( dest, src, siz );
}

inline void *memcpy_s(void *dest, size_t destMax, const void *src, size_t srcSiz)
{
    return memcpy( dest, src, srcSiz );
}

#else

//#define snprintf	_snprintf

#endif

void numstrdate( char *pDest, size_t maxLen=0 );

char *vsnprintf_exp(
        char *str,
        size_t strSize,
        std::vector<char> &altBuff,
        const char *format,
        va_list ap );

#endif

