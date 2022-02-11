//==================================================================
/// TimeUtils.cpp
///
/// Created by Davide Pasca - 2018/02/12
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#if defined(_MSC_VER)
# include <Windows.h>
#else
# include <sys/time.h>
#endif

#include <chrono>
#include <thread>
#include <time.h>
#include "DBase.h"
#include "TimeUtils.h"

using namespace std;

//==================================================================
TimeUS GetSteadyTimeUS()
{
    return
        chrono::duration_cast<std::chrono::microseconds>(
            chrono::steady_clock::now().time_since_epoch() ).count();
}

//==================================================================
static void mytimeofday( timeval *tp )
{
#if defined(_MSC_VER)
    // http://git.postgresql.org/gitweb/?p=postgresql.git;a=blob_plain;f=src/port/gettimeofday.c

    /* FILETIME of Jan 1 1970 00:00:00. */
    static const unsigned __int64 epoch = 116444736000000000Ui64;

    FILETIME    file_time;
    SYSTEMTIME  system_time;
    ULARGE_INTEGER ularge;

    GetSystemTime(&system_time);
    SystemTimeToFileTime(&system_time, &file_time);
    ularge.LowPart = file_time.dwLowDateTime;
    ularge.HighPart = file_time.dwHighDateTime;

    tp->tv_sec = (long) ((ularge.QuadPart - epoch) / 10000000L);
    tp->tv_usec = (long) (system_time.wMilliseconds * 1000);
#else
    gettimeofday( tp, NULL );
#endif
}

//==================================================================
TimeUS GetEpochTimeUS()
{
    struct timeval tp {};
    mytimeofday( &tp );

    return { (int64_t)tp.tv_sec * 1000000 + (int64_t)tp.tv_usec };
}

//==================================================================
void SleepSecs( double s )
{
    std::this_thread::sleep_for(
        chrono::microseconds( (int64_t)(s * 1e6) ) );
}

//==================================================================
void SleepTimeUS( TimeUS deltaTimeUS )
{
    std::this_thread::sleep_for(
        chrono::microseconds( deltaTimeUS.CalcTimeUS() ) );
}

//==================================================================
time_t TIMEGM(std::tm const *t)
{
    if NOT( t )
#if defined(WIN32)
        return _mkgmtime64( (std::tm *)nullptr );
#else
        return timegm( (std::tm *)nullptr );
#endif

    // for Linux, that doesn't have a const-correct interface
    std::tm non_const_t = *t;

#if defined(WIN32)
    return _mkgmtime64( &non_const_t );
#else
    return timegm( &non_const_t );
#endif
}

//==================================================================
std::tm *GMTIME(std::time_t const* t, std::tm *ptm)
{
#if defined(WIN32)
    //return internal_gmtime( t, ptm );
    _gmtime64_s( ptm, t );
    return ptm;
#else
    return gmtime_r( t, ptm );
#endif
}

