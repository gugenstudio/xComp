//==================================================================
/// DCRC32.cpp
///
/// Created by Davide Pasca - 2011/2/24
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#include "DCRC32.h"

//==================================================================
bool    DCRC32::mTableBuilt = false;
U32     DCRC32::mTable[0x100];

//==================================================================
void DCRC32::BuildTable()
{
    for(U32 i = 0; i < 0x100; ++i)
    {
        mTable[i] = Reflect(i,8) << 24;

        for(size_t j = 0; j < 8; ++j)
            mTable[i] = (mTable[i] << 1) ^ ( (mTable[i] & (1<<31))  ? POLYNOMIAL : 0);

        mTable[i] = Reflect(mTable[i],32);
    }
    mTableBuilt = true;
}

//==================================================================
U32 DCRC32::Reflect(U32 v, int bits)
{
    U32 ret = 0;

    --bits;
    for(int i = 0; i <= bits; ++i)
    {
        if(v & (1<<i))
            ret |= 1 << (bits-i);
    }

    return ret;
}

//==================================================================
DCRC32 &DCRC32::HashCStr( const char *pStr )
{
    return Hash( (const U8 *)pStr, strlen(pStr) );
}

//==================================================================
DCRC32 &DCRC32::HashStr( const DStr &str )
{
    return Hash( (const U8 *)str.c_str(), str.size() );
}
