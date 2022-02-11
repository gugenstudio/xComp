//==================================================================
/// TimeUtils.h
///
/// Created by Davide Pasca - 2018/02/12
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#ifndef TIMEUTILS_H
#define TIMEUTILS_H

#include <ctime>
#include <stdint.h>
#include <limits>
#include "DBase.h"
#include "DSerialBase.h"

class TimeUS
{
    // time in microseconds
    int64_t mV = 0;

public:
    constexpr TimeUS( int64_t timeUS=0 ) : mV(timeUS) {}

    constexpr TimeUS &FromSecsI64( int64_t timeS ){ *this = timeS * 1000000; return *this; }
    constexpr TimeUS &FromSecs( double timeS )    { *this = (int64_t)(timeS  * 1e6); return *this; }
    constexpr TimeUS &FromMSecs( int64_t timeMS ) { *this = (int64_t)(timeMS * 1e3); return *this; }

    static constexpr TimeUS EPSILON()    { return (int64_t)1; };
    static constexpr TimeUS ONE_SECOND() { return (int64_t)1000000; };
    static constexpr TimeUS ONE_MINUTE() { return (int64_t)60 * 1000000; };
    static constexpr TimeUS ONE_HOUR()   { return (int64_t)60 * 60 * 1000000; };
    static constexpr TimeUS ONE_DAY()    { return (int64_t)60 * 60 * 24 * 1000000; };

    static constexpr TimeUS MIN_TIME_US(){ return std::numeric_limits<int64_t>::min(); }
    static constexpr TimeUS MAX_TIME_US(){ return std::numeric_limits<int64_t>::max(); }

    constexpr bool IsTimeZero() const { return !mV; }

    constexpr double  CalcTimeS()  const { return (double)mV / 1e6; }
    constexpr int64_t CalcTimeMS() const { return         mV / 1000; }
    constexpr int64_t CalcTimeUS() const { return         mV; }

    constexpr TimeUS CalcModulo( const TimeUS div ) const
    {
        return mV % div.mV;
    }

    constexpr TimeUS CalcAlignTimeFloor( const TimeUS align ) const
    {
        return mV - mV % align.mV;
    }

    constexpr TimeUS CalcAlignTimeCeil( const TimeUS align ) const
    {
        return mV + (align.mV - mV % align.mV);
    }

    explicit operator bool   () const { return !!mV; }
    explicit operator int64_t() const { return   mV; }

    constexpr TimeUS  operator + (const TimeUS &rval) const { return mV + rval.mV; }
    constexpr TimeUS  operator - (const TimeUS &rval) const { return mV - rval.mV; }
    constexpr TimeUS  operator * (     int64_t  rval) const { return mV * rval   ; }
    constexpr TimeUS  operator * (     int      rval) const { return mV * rval   ; }
    constexpr TimeUS  operator * (     size_t   rval) const { return mV * rval   ; }
    constexpr TimeUS  operator * (     double   rval) const { return (int64_t)(mV * rval); }
    constexpr int64_t operator / (const TimeUS &rval) const { return mV / rval.mV; }
    constexpr TimeUS  operator / (     int64_t  rval) const { return mV / rval   ; }
    constexpr TimeUS  operator / (     size_t   rval) const { return mV / rval   ; }
    constexpr TimeUS  operator / (     int      rval) const { return mV / rval   ; }

    constexpr TimeUS operator +=(const TimeUS  &rval) { *this = *this + rval; return *this; }
    constexpr TimeUS operator -=(const TimeUS  &rval) { *this = *this - rval; return *this; }
    constexpr TimeUS operator *=(      int64_t  rval) { *this = *this * rval; return *this; }
    constexpr TimeUS operator *=(      int      rval) { *this = *this * rval; return *this; }
    constexpr TimeUS operator *=(      size_t   rval) { *this = *this * rval; return *this; }

    constexpr friend bool operator ==(const TimeUS &l, const TimeUS &r){return l.mV == r.mV;}
    constexpr friend bool operator !=(const TimeUS &l, const TimeUS &r){return l.mV != r.mV;}
    constexpr friend bool operator  <(const TimeUS &l, const TimeUS &r){return l.mV <  r.mV;}
    constexpr friend bool operator  >(const TimeUS &l, const TimeUS &r){return l.mV >  r.mV;}
    constexpr friend bool operator <=(const TimeUS &l, const TimeUS &r){return l.mV <= r.mV;}
    constexpr friend bool operator >=(const TimeUS &l, const TimeUS &r){return l.mV >= r.mV;}

    constexpr friend bool operator ==(const TimeUS &l, int64_t r){return l.mV == r;}
    constexpr friend bool operator !=(const TimeUS &l, int64_t r){return l.mV != r;}
    constexpr friend bool operator  <(const TimeUS &l, int64_t r){return l.mV <  r;}
    constexpr friend bool operator  >(const TimeUS &l, int64_t r){return l.mV >  r;}
    constexpr friend bool operator <=(const TimeUS &l, int64_t r){return l.mV <= r;}
    constexpr friend bool operator >=(const TimeUS &l, int64_t r){return l.mV >= r;}

    constexpr friend bool operator ==(const TimeUS &l, int r){return l.mV == r;}
    constexpr friend bool operator !=(const TimeUS &l, int r){return l.mV != r;}
    constexpr friend bool operator  <(const TimeUS &l, int r){return l.mV <  r;}
    constexpr friend bool operator  >(const TimeUS &l, int r){return l.mV >  r;}
    constexpr friend bool operator <=(const TimeUS &l, int r){return l.mV <= r;}
    constexpr friend bool operator >=(const TimeUS &l, int r){return l.mV >= r;}
};

inline TimeUS operator *(const int64_t &l, const TimeUS &r) {return r * l; }
inline TimeUS operator *(const int     &l, const TimeUS &r) {return r * l; }
inline TimeUS operator *(const size_t  &l, const TimeUS &r) {return r * l; }

inline void Serialize( DSerialBase &w, const TimeUS &val )
{
    Serialize( w, (int64_t)val );
}

inline void Deserialize( DDeserialBase &r, TimeUS &val )
{
    int64_t t {};
    Deserialize( r, t );
    val = { t };
}

// integral microseconds to double seconds
inline constexpr double TimeUSToSecs( TimeUS tus )
{
    return tus.CalcTimeS();
}

// integral microseconds to double seconds
inline constexpr TimeUS TimeSToUS( double s )
{
    return TimeUS().FromSecs( s );
}

//==================================================================
TimeUS GetSteadyTimeUS();
TimeUS GetEpochTimeUS();

inline double GetSteadyTimeS() { return TimeUSToSecs( GetSteadyTimeUS() ); }
inline double GetEpochTimeS()  { return TimeUSToSecs( GetEpochTimeUS() ); }

void SleepSecs( double s );
void SleepTimeUS( TimeUS deltaTimeUS );

time_t TIMEGM(std::tm const *t);
std::tm *GMTIME(std::time_t const* t, std::tm *ptm);

//==================================================================
class TimedEvent
{
    const TimeUS mSpanTimeUS;

    TimeUS mLastEventTimeUS;

public:
    TimedEvent( TimeUS spanTimeUS ) : mSpanTimeUS(spanTimeUS) {}

    bool CheckTimedEvent( TimeUS curTimeUS )
    {
        if ( (curTimeUS - mLastEventTimeUS) > mSpanTimeUS )
        {
            mLastEventTimeUS = curTimeUS;
            return true;
        }

        return false;
    }

    void WaitTimedEvent( TimeUS curTimeUS )
    {
        if (c_auto timeSpent = mLastEventTimeUS
                                ? curTimeUS - mLastEventTimeUS
                                : TimeUS{};
                   timeSpent < mSpanTimeUS )
        {
            SleepTimeUS( mSpanTimeUS - timeSpent );
        }

        mLastEventTimeUS = curTimeUS;
    }

    void MarkTimedEvent( TimeUS curTimeUS )
    {
        mLastEventTimeUS = curTimeUS;
    }

    void ResetTimedEventWait()
    {
        mLastEventTimeUS = {};
    }

    TimeUS GetLastEventTimeUS() const { return mLastEventTimeUS; }
};

//==================================================================
class QuickProf
{
    DStr    mMsg;
    TimeUS  mStart;

public:
    QuickProf( const DStr &msg )
        : mMsg(msg)
        , mStart(GetSteadyTimeUS())
    {}

    // empty version to reuse
    QuickProf() {}

    ~QuickProf()
    {
        FlushSampling();
    }

    void FlushSampling()
    {
        if NOT( mStart )
            return;

        c_auto elapsed = GetSteadyTimeUS() - mStart;
        printf( "%s: %4.2f ms\n", mMsg.c_str(), elapsed.CalcTimeS()*1000 );

        mStart = 0;
    }
};

#define DUT_STRINGIFY(_X_) #_X_
#define DUT_TOSTRING(_X_) DUT_STRINGIFY(_X_)
#define QUICKPROF \
    QuickProf _qprof_##__LINE__( \
        DStr( DStr(__DSHORT_FILE__) + (":" DUT_TOSTRING(__LINE__)) ) )

//==================================================================
template <int DISP_INTERVAL>
class StaticProf
{
    const char  *mpMsg;
    TimeUS      mStart;
    double      &mAccTimeS;
    int         &mCnt;

public:
    StaticProf( const char *pMsg, double &accTime, int &cnt )
        : mpMsg(pMsg)
        , mStart(GetSteadyTimeUS())
        , mAccTimeS(accTime)
        , mCnt(cnt)
    {}

    ~StaticProf()
    {
        c_auto elapsed = GetSteadyTimeUS() - mStart;
        mAccTimeS += elapsed.CalcTimeS();
        if ( DISP_INTERVAL == 1 || (++mCnt % DISP_INTERVAL) == 0 )
            printf( "%s: %4.2lf ms (accum %i)\n", mpMsg, mAccTimeS*1000.0, mCnt );
    }
};

// static prof
#define DUT_STATICPROF \
    static double _sprof_##__LINE__##accTime; \
    static int    _sprof_##__LINE__##cnt; \
    static DStr _sprof_##__LINE__##str( \
                    DStr(__DSHORT_FILE__) + \
                    (":" DUT_TOSTRING(__LINE__)) + \
                    (" (" __FUNCTION__ ")") ); \
    StaticProf<1> _sprof_##__LINE__( \
        _sprof_##__LINE__##str.c_str(), \
        _sprof_##__LINE__##accTime, \
        _sprof_##__LINE__##cnt )

#define DUT_STATICPROFN(_N_) \
    static double _sprof_##__LINE__##accTime; \
    static int    _sprof_##__LINE__##cnt; \
    static DStr _sprof_##__LINE__##str( \
                    DStr(__DSHORT_FILE__) + \
                    (":" DUT_TOSTRING(__LINE__)) + \
                    (" (" __FUNCTION__ ")") ); \
    StaticProf<_N_> _sprof_##__LINE__( \
        _sprof_##__LINE__##str.c_str(), \
        _sprof_##__LINE__##accTime, \
        _sprof_##__LINE__##cnt )

#endif

