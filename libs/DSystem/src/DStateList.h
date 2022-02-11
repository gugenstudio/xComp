//==================================================================
/// DStateList.h
///
/// Created by Davide Pasca - 2018/3/16
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#ifndef DSTATELIST_H
#define DSTATELIST_H

#include <math.h>
#include "DContainers.h"
#include "TimeUtils.h"

//==================================================================
class DState
{
    friend class DStateList;

    static constexpr u_int FLG_IS_ACTIVE = 1;
    static constexpr u_int FLG_IS_PAUSED = 2;
    static constexpr u_int FLG_DONE_SIGNAL = 4;

    u_int   mFlags = 0;
    TimeUS  mTime = 0;
    u_int   mFrameCnt = 0;
    TimeUS  mEndTime = 0;

    DFun<void ()> mOnDoneFn;
    DFun<void ()> mOnIdleFn;
    DFun<void ()> mOnDestroyFn;

public:
    DState() { Reset(); }

    ~DState()
    {
        if ( mOnDestroyFn )
            mOnDestroyFn();
    }

    void SetupTime( TimeUS endTime ); // time only

    void Reset()
    {
        mFlags    = 0;
        mTime     = 0;
        mFrameCnt = 0;
    }

    void Pause();
    void Unpause();
    bool IsPaused() const { return !!(mFlags & FLG_IS_PAUSED); }

    void Activate(
            const DFun<void ()> &onDoneFn={},
            const DFun<void ()> &onIdleFn={},
            const DFun<void ()> &onDestroyFn={} );

    void ActivateNC(
            const DFun<void ()> &onDoneFn={},
            const DFun<void ()> &onIdleFn={},
            const DFun<void ()> &onDestroyFn={} )
    {
        if NOT( IsActive() )
            Activate( onDoneFn, onIdleFn, onDestroyFn );
    }

    void Deactivate();
    void DeactivateNC();

    void Reactivate(
            const DFun<void ()> &onDoneFn={},
            const DFun<void ()> &onIdleFn={},
            const DFun<void ()> &onDestroyFn={} )
    {
        DeactivateNC();
        Activate( onDoneFn, onIdleFn, onDestroyFn );
    }

    bool IsActive() const { return !!(mFlags & FLG_IS_ACTIVE); }

    TimeUS GetCurTimeUS() const { DASSERT( mTime != 0 ); return mTime; }
    TimeUS GetEndTime() const { return mEndTime; }

    u_int GetFrameCnt() const { return mFrameCnt; }

    bool GetDoneState();
    void SetDoneState();

    void ForwardToEnd();
    void RewindToStart();

    float GetTNorm() const
    {
        DASSERT( mTime != 0 );
        DASSERT( mEndTime > 0 );
        return (float)(mTime.CalcTimeS() / mEndTime.CalcTimeS());
    }

    bool IsSpent() const
    {
        DASSERT( mTime != 0 );
        return !IsActive() && mEndTime && mTime == mEndTime;
    }
};

//==================================================================
class DStateList
{
    //DVec<DState> mStates;
    const size_t    mStatesN;
    uptr<DState []> mStates;

public:
    DStateList( size_t statesN );

    void IdleStatesCurTImeUS( TimeUS curTimeUS );

    const DState &operator[](size_t i) const {DASSERT(i<mStatesN); return mStates[i];}
          DState &operator[](size_t i)       {DASSERT(i<mStatesN); return mStates[i];}
};

#endif

