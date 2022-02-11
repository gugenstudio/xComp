//==================================================================
/// DRandom.h
///
/// Created by Davide Pasca - 2009/12/6
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#ifndef DRANDOM_H
#define DRANDOM_H

#include <stdint.h>
#include "DMathBase.h"

//==================================================================
class DRandom
{
    static uint32_t xorshift32( uint32_t state )
    {
        auto x = state;
        x ^= x << 13;
        x ^= x >> 17;
        x ^= x << 5;
        return x;
    }

    uint32_t    mState = 0;

public:
    DRandom( uint32_t seed )
    {
        mState = xorshift32( fixSeed( seed ) );
    }

    DRandom( uint64_t seed )
    {
        const auto *p = (const uint32_t *)&seed;
        mState = xorshift32( fixSeed( p[0] ^ p[1] ) );
    }

    DRandom( double seed )
    {
        const auto *p = (const uint32_t *)&seed;
        mState = xorshift32( fixSeed( p[0] ^ p[1] ) );
    }
private:
    DMT_FINLINE static uint32_t fixSeed( uint32_t seed )
    {
         return seed ? seed : ~seed;
    }

public:
    uint32_t NextU32()
    {
        mState = xorshift32( mState );
        return mState;
    }

    float NextF0_1()
    {
        union
        {
            float       valF;
            uint32_t    valUI;
        };

        valUI = 0x3f800000 | (NextU32() >> 9);

        return valF - 1.0f;
    }

    int Next32Range( int rmin, int rmax )
    {
        const auto rminD = (double)rmin;
        const auto rmaxD = (double)rmax;
        const auto t = (double)NextF0_1();
        const auto val = (int)(rminD + (rmaxD+1 - rminD) * t);
        return DClamp( val, rmin, rmax );
    }
};

#endif

