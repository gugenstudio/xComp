//==================================================================
/// DLogOut.cpp
///
/// Created by Davide Pasca - 2018/02/16
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#include <mutex>
#include <thread>
#include <fstream>
#include <iostream>
#include "StringUtils.h"
#include "TimeUtils.h"
#include "FileUtils.h"
#include "DLogOut.h"

//==================================================================
static DStr makeTimeFNameString()
{
    c_auto timeUS = GetEpochTimeUS();

    constexpr int64_t ONE_MILLION = 1000000;

    std::tm t;
    {
        memset( &t, 0, sizeof(t) );
        const auto timeS = (time_t)((int64_t)timeUS / ONE_MILLION);
        GMTIME( &timeS, &t );
    }

    const int t_secs     = t.tm_sec  ;
    const int t_mins     = t.tm_min  ;
    const int t_hours    = t.tm_hour ;
    const int t_day      = t.tm_mday ;
    const int t_month    = t.tm_mon  + 1;
    const int t_year     = t.tm_year + 1900;

    return SSPrintFS(
            "%04d-%02d-%02dT%02d_%02d_%02d",
            t_year,  t_month, t_day,
            t_hours, t_mins,  t_secs );
}

//==================================================================
struct LogWork
{
    std::mutex      mLogMutex;

    DVec<DStr>      mPrefixes;

    std::ofstream   mOFs;

    size_t          mPrinterNextID = 0;

    std::unordered_map<size_t, DFun<void (const DStr&)>>    mPrintFns;

    std::unordered_map<std::thread::id, DStr>    mThreadNames;

    std::thread::id mMainThreadID {};

    const u_int     mDefaultFlags;
    const u_int     mLogOptFlags;

    static constexpr size_t REPEAD_BASE_CNT = 10;

    size_t          mSameStrCnt = 0;
    DStr            mLastRefConenteStr;

    LogWork(
            const DStr &baseName,
            const DStr &basePath,
            u_int defaultFlags,
            u_int logOptFlags )
        : mDefaultFlags(defaultFlags)
        , mLogOptFlags(logOptFlags)
    {
        if NOT( FU_DirectoryExists( basePath ) )
        {
            printf( "Creating '%s'\n", basePath.c_str() );
            if NOT( FU_CreateDirectories( basePath.c_str() ) )
                DEX_RUNTIME_ERROR( "Could not create '%s'", basePath.c_str() );
        }

        c_auto fullPathFName =
                FU_JPath( basePath, baseName + "_log_" ) + makeTimeFNameString() + ".txt";

        printf( "Creating '%s'\n", fullPathFName.c_str() );
        mOFs = std::ofstream( fullPathFName, std::ios::out | std::ios::app );
        if NOT( mOFs.is_open() )
            DEX_RUNTIME_ERROR( "Could not create '%s'", fullPathFName.c_str() );


        mMainThreadID = std::this_thread::get_id();
    }

    void PrintStr( const DStr &str, const DStr &refContentStr )
    {
        std::lock_guard<std::mutex> lock( mLogMutex );

        auto doWrite = [this]( c_auto &outStr )
        {
            mOFs << outStr << std::flush;

            std::cout << outStr << std::flush;

            for (c_auto &[k, fn] : mPrintFns)
                fn( outStr );
        };

        if (c_auto isSame = (refContentStr == mLastRefConenteStr);
                   isSame || mSameStrCnt )
        {
            if ( isSame )
                mSameStrCnt += 1;

            if ( (mSameStrCnt > REPEAD_BASE_CNT && !isSame) || mSameStrCnt > 10000 )
            {
                doWrite( "[WRN] Log output repeated " +
                            std::to_string( mSameStrCnt - REPEAD_BASE_CNT + 1 ) +
                                " extra times\n" );
                mSameStrCnt = 0;
            }

            if NOT( isSame )
                mSameStrCnt = 0;
        }

        if ( mSameStrCnt < REPEAD_BASE_CNT )
        {
            doWrite( str );
            mLastRefConenteStr = refContentStr;
        }
    }
};

//==================================================================
thread_local bool   _sLastWasNoNL;

static uptr<LogWork> _soLOG;
static LogWork       *_spLOG {};

//==================================================================
void LogSetInstance( void *pLog )
{
    if ( _spLOG )
        DEX_RUNTIME_ERROR( "Attempting to initalize a Log that is already initialized" );

    _spLOG = (LogWork *)pLog;
}

//==================================================================
void *LogGetInstance()
{
    return (void *)_spLOG;
}

//==================================================================
void LogCreate(
        const DStr &baseName,
        const DStr &basePath,
        u_int defaultFlags,
        u_int logOptFlags )
{
    if ( _spLOG )
        DEX_RUNTIME_ERROR( "Attempting to create a Log that is already initialized" );

    _soLOG = std::make_unique<LogWork>( baseName, basePath, defaultFlags, logOptFlags );

    _spLOG = _soLOG.get();
}

//==================================================================
size_t LogAddPrinter( const DFun<void (const DStr&)> &fn )
{
    std::lock_guard<std::mutex> lock(_spLOG->mLogMutex);

    _spLOG->mPrintFns.emplace( _spLOG->mPrinterNextID, fn );

    return _spLOG->mPrinterNextID++;
}

//==================================================================
void LogRemovePrinter( size_t id )
{
    std::lock_guard<std::mutex> lock(_spLOG->mLogMutex);

    _spLOG->mPrintFns.erase( _spLOG->mPrintFns.find( id ) );
}

//==================================================================
void LogPushPrefix( const DStr &str )
{
    std::lock_guard<std::mutex> lock(_spLOG->mLogMutex);

    _spLOG->mPrefixes.push_back( str );
}

//==================================================================
void LogPopPrefix()
{
    std::lock_guard<std::mutex> lock(_spLOG->mLogMutex);

    c_auto n = _spLOG->mPrefixes.size();

    DASSERT( n );
    if ( n )
        _spLOG->mPrefixes.resize( n - 1 );
}

//==================================================================
void LogSetThreadName( const DStr &name )
{
    LogRemoveThreadName();

    std::lock_guard<std::mutex> lock(_spLOG->mLogMutex);

    _spLOG->mThreadNames.emplace( std::this_thread::get_id(), name );
}

//==================================================================
void LogRemoveThreadName()
{
    std::lock_guard<std::mutex> lock(_spLOG->mLogMutex);

    auto it = _spLOG->mThreadNames.find( std::this_thread::get_id() );
    if ( it != _spLOG->mThreadNames.end() )
        _spLOG->mThreadNames.erase( it );
}

//==================================================================
void LogOut( u_int flags, const DStr &str )
{
    flags |= _spLOG->mDefaultFlags;

    DStr str2;

    if NOT( _sLastWasNoNL )
    {
        if NOT( flags & LOG_F_NOTS )
        {
            std::tm t;
            {
                constexpr int64_t ONE_MILLION = 1000000;

                memset( &t, 0, sizeof(t) );
                const auto timeS = (time_t)((int64_t)GetEpochTimeUS() / ONE_MILLION);
                GMTIME( &timeS, &t );
            }

            str2 = SSPrintFS(
                        (_spLOG->mLogOptFlags & LOGOPT_TS_MODE_1)
                            ? "[%02d.%02d_%02d:%02d:%02d]"
                            : "[%02d%02d_%02d%02d%02d]",
                        t.tm_mon + 1,
                        t.tm_mday,
                        t.tm_hour,
                        t.tm_min,
                        t.tm_sec );
        }

        {
        std::lock_guard<std::mutex> lock(_spLOG->mLogMutex);

        if (c_auto id  = std::this_thread::get_id();
                   id != _spLOG->mMainThreadID )
        {
            if (auto it  = _spLOG->mThreadNames.find( id );
                     it != _spLOG->mThreadNames.end())
                str2 += "[:" + it->second + "]";
            else
            {
                std::ostringstream oss;
                oss << std::this_thread::get_id();
                str2 += "[:" + oss.str() + "]";
            }
        }

        for (c_auto &p : _spLOG->mPrefixes)
            str2 += p;

        if NOT( _spLOG->mPrefixes.empty() )
            str2 += ' ';
        }

        if ( flags & LOG_STD ) str2 += "[STD]";
        if ( flags & LOG_ERR ) str2 += "[ERR]";
        if ( flags & LOG_WRN ) str2 += "[WRN]";
        if ( flags & LOG_LIV ) str2 += "[LIV]";
        if ( flags & LOG_TST ) str2 += "[TST]";
        if ( flags & LOG_TRD ) str2 += "[TRD]";
        if ( flags & LOG_CSV ) str2 += "[CSV]";
        if ( flags & LOG_DBG ) str2 += "[DBG]";
        if ( flags & LOG_ALG ) str2 += "[ALG]";
        if ( flags & LOG_NET ) str2 += "[NET]";
        //if ( flags & LOG_NUL ) str2 += "";
    }

    if ( !str2.empty() && str2.back() != ' ' )
        str2 += ' ';

    str2 += str;

    str2 += (flags & LOG_F_NONL) ? " | " : "\n";

    _sLastWasNoNL = !!(flags & LOG_F_NONL);

    _spLOG->PrintStr( str2, str );
}

