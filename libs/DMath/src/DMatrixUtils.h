//==================================================================
/// DMatrixUtils.h
///
/// Created by Davide Pasca - 2015/11/7
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#ifndef DMATRIXUTILS_H
#define DMATRIXUTILS_H

#include <math.h>
#include "DMatrix33.h"
#include "DMatrix44.h"

//==================================================================
namespace DMU
{

//==================================================================
#if 1 // TODO: verify that the new version didn't break anything
//==================================================================
DMT_FINLINE Matrix33 CreateRotMtxFromNormal(
                const Float3 &nor_z,
                Float3 up_y={0,1,0} )
{
    // warn if the input vectors are not normalized
    DMT_ASSERT( DIsUnitVec( nor_z ) && DIsUnitVec( up_y ) );

    float val = fabsf( nor_z[1] );

    if ( val >= 0.99f && val <= 1.01f )
    {
        auto tmp = up_y[0];
        up_y[0] = up_y[1];
        up_y[1] = up_y[2];
        up_y[2] = tmp;
    }

    auto tan_x = DNormalize( DCross( up_y  , nor_z ) );
    auto btn_y = DNormalize( DCross( nor_z , tan_x ) );

    return Matrix33( tan_x, btn_y, nor_z );
}
#else
//==================================================================
DMT_FINLINE Matrix33 CreateRotMtxFromNormal(
                const Float3 &nor,
                Float3 other={0,1,0} )
{
    // warn if the input vectors are not normalized
    DMT_ASSERT( DIsUnitVec( nor ) && DIsUnitVec( other ) );

    float val = fabsf( nor[1] );

    if ( val >= 0.99f && val <= 1.01f )
    {
        auto tmp = other[0];
        other[0] = other[1];
        other[1] = other[2];
        other[2] = tmp;
    }

    auto tan = nor.GetCross( other ).GetNormalized();
    auto btn = nor.GetCross( tan ).GetNormalized();

    return Matrix33( tan, btn, nor );
}
#endif

//==================================================================
/* http://gamedev.stackexchange.com/questions/50963/
      how-to-extract-euler-angles-from-transformation-matrix
*/
template <typename MTX_TYPE>
static Float3 CalcAnglesFromMtx( const MTX_TYPE &mtx )
{
    auto ax = atan2( mtx.mij(1,2), mtx.mij(2,2) );

    auto c2 = sqrt( mtx.mij(0,0) * mtx.mij(0,0) +
                    mtx.mij(0,1) * mtx.mij(0,1) );

    auto ay = atan2( -mtx.mij(0,2), c2 );

    auto s1 = sin( ax );
    auto c1 = cos( ax );

    auto az = atan2( s1 * mtx.mij(2,0) - c1 * mtx.mij(1,0),
                     c1 * mtx.mij(1,1) - s1 * mtx.mij(2,1) );

    return {ax, ay, az};
}

//==================================================================
};

#endif

