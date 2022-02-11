//==================================================================
/// DLogOut.h
///
/// Created by Davide Pasca - 2018/02/16
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#ifndef DLOGOUT_H
#define DLOGOUT_H

#include "DBase.h"
#include "DSafeCrt.h"
#include "StringUtils.h"

enum LogFlags : u_int
{
    LOG_F_NONL = 1 << 0,
    LOG_F_NOTS = 1 << 1,

    LOG_STD = 1 << 6,
    LOG_ERR = 1 << 7,
    LOG_WRN = 1 << 16,
    LOG_LIV = 1 << 8,
    LOG_TST = 1 << 9,
    LOG_TRD = 1 << 10,
    LOG_CSV = 1 << 11,
    LOG_DBG = 1 << 12,
    LOG_ALG = 1 << 13,
    LOG_NET = 1 << 14,
    LOG_NUL = 1 << 15,
};

enum LogOptFlags : u_int
{
    LOGOPT_TS_MODE_0    = 0,
    LOGOPT_TS_MODE_1    = 1,
};

void LogSetInstance( void *pLog );
void *LogGetInstance();

void LogCreate(
        const DStr &baseName,
        const DStr &basePath,
        u_int defaultFlags=0,
        u_int logOptFlags=0 );

size_t LogAddPrinter( const DFun<void (const DStr&)> &fn );
void LogRemovePrinter( size_t id );
void LogPushPrefix( const DStr &str );
void LogPopPrefix();
void LogSetThreadName( const DStr &name );
void LogRemoveThreadName();

void LogOut( u_int flags, const DStr &str );
void LogOut( u_int flags, const char *pFmt, ... );

//==================================================================
inline void LogOut( u_int flags, const char *pFmt, ... )
{
	va_list	vl;
	va_start( vl, pFmt );
    auto tmpStr = DStr();
	VSSPrintFS( tmpStr, pFmt, vl );
	va_end( vl );

    LogOut( flags, tmpStr );
}

//==================================================================
class LogThreadNameScope
{
public:
    LogThreadNameScope( const DStr &name ) { LogSetThreadName( name ); }
    ~LogThreadNameScope()                  { LogRemoveThreadName(); }
};

#ifdef _MSC_VER
# define LOG_FUNCNAME  __FUNCTION__
#else
# define LOG_FUNCNAME  __FUNCTION__
#endif

#endif

