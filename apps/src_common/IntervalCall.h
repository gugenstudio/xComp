//==================================================================
/// IntervalCall.h
///
/// Created by Davide Pasca - 2019/04/04
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#ifndef INTERVALCALL_H
#define INTERVALCALL_H

#include "DContainers.h"

//==================================================================
class IntervalCall
{
    const double        mFixedIntervalS;
    const DFun<void ()> mFunc;

    double mLastTimeS = 0;
    double mAccumTimeS = 0;

public:
    IntervalCall(
            double fixedIntervalS,
            const DFun<void ()> &fn )
        : mFixedIntervalS(fixedIntervalS)
        , mFunc(fn)
    {
    }

    void AnimateIC( const double curTimeS )
    {
        if NOT( mLastTimeS )
            mLastTimeS = curTimeS;

        const auto diff = curTimeS - mLastTimeS;
        mLastTimeS = curTimeS;

        mAccumTimeS += diff;

        while ( mAccumTimeS > mFixedIntervalS )
        {
            mFunc();
            mAccumTimeS -= mFixedIntervalS;
        }
    }
};

#endif

