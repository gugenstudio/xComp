//==================================================================
/// DStateList.cpp
///
/// Created by Davide Pasca - 2018/3/16
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#include "DStateList.h"

//==================================================================
void DState::SetupTime( TimeUS endTime )
{
    DASSERT( endTime > 0 );
    Reset();
    mEndTime = endTime;
}

//==================================================================
void DState::Pause()
{
    mFlags |= FLG_IS_PAUSED;
}

//==================================================================
void DState::Unpause()
{
    mFlags &= ~FLG_IS_PAUSED;
}

//==================================================================
void DState::Activate(
            const DFun<void ()> &onDoneFn,
            const DFun<void ()> &onIdleFn,
            const DFun<void ()> &onDestroyFn )
{
    DASSERT( IsActive() == false );
    mFlags |= FLG_IS_ACTIVE;
    mTime = 0;
    mFrameCnt   = 0;
    mFlags &= ~FLG_DONE_SIGNAL;

    // only replace if not null
    if ( onDoneFn )
        mOnDoneFn = onDoneFn;

    if ( onIdleFn )
        mOnIdleFn = onIdleFn;
}

//==================================================================
void DState::Deactivate()
{
    DASSERT( IsActive() == true );
    mFlags &= ~FLG_IS_ACTIVE;
    mTime = 0;
    mFrameCnt   = 0;
    mFlags &= ~FLG_DONE_SIGNAL;
}

//==================================================================
void DState::DeactivateNC()
{
    if NOT( IsActive() )
        return;

    mFlags &= ~FLG_IS_ACTIVE;
    mTime = 0;
    mFrameCnt   = 0;
    mFlags &= ~FLG_DONE_SIGNAL;
}

//==================================================================
bool DState::GetDoneState()
{
    const auto ret = !!(mFlags & FLG_DONE_SIGNAL);
    mFlags &= ~FLG_DONE_SIGNAL;
    return ret;
}

//==================================================================
void DState::SetDoneState()
{
    mFlags |= FLG_DONE_SIGNAL;
    if ( mOnIdleFn ) mOnIdleFn();
    if ( mOnDoneFn ) mOnDoneFn();
}

//==================================================================
void DState::ForwardToEnd()
{
    mTime = mEndTime;
    mFlags &= ~FLG_IS_ACTIVE;
    mFlags |= FLG_DONE_SIGNAL;
    if ( mOnIdleFn ) mOnIdleFn();
    if ( mOnDoneFn ) mOnDoneFn();
}

//==================================================================
void DState::RewindToStart()
{
    mTime = 0;
    mFlags |= FLG_IS_ACTIVE;
    mFlags &= ~FLG_DONE_SIGNAL;
}

//==================================================================
DStateList::DStateList( size_t statesN )
    : mStatesN(statesN)
{
    //mStates.resize( statesN );
    mStates.reset( new DState [ mStatesN ] );
}

//==================================================================
void DStateList::IdleStatesCurTImeUS( TimeUS curTimeUS )
{
    //for (auto &s : mStates)
    //{
    for (size_t i=0; i != mStatesN; ++i)
    {
        auto &s = mStates[i];

        if ( !s.IsActive() || s.IsPaused() )
            continue;

        s.mTime = curTimeUS;
        s.mFrameCnt += 1;
        if ( s.mEndTime != 0 && curTimeUS >= s.mEndTime )
        {
            s.ForwardToEnd();
        }
        else
        {
            if ( s.mOnIdleFn )
                s.mOnIdleFn();
        }
    }
}

