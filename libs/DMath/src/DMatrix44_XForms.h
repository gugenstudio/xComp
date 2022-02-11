//==================================================================
/// DMatrix_XForms.h
///
/// Created by Davide Pasca - 2016/5/24
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#ifndef DMATRIX_XFORMS_H
#define DMATRIX_XFORMS_H

//==================================================================
template <typename TV, typename TM>
DMT_FINLINE Vec4<TV> V4__M44_Mul_V3W1( const Matrix44T<TM> &a, const Vec3<TV> &v )
{
    const auto &x = v[0];
    const auto &y = v[1];
    const auto &z = v[2];

    return {
        (TV)(a.mij(0,0) * x + a.mij(0,1) * y + a.mij(0,2) * z + a.mij(0,3)),
        (TV)(a.mij(1,0) * x + a.mij(1,1) * y + a.mij(1,2) * z + a.mij(1,3)),
        (TV)(a.mij(2,0) * x + a.mij(2,1) * y + a.mij(2,2) * z + a.mij(2,3)),
        (TV)(a.mij(3,0) * x + a.mij(3,1) * y + a.mij(3,2) * z + a.mij(3,3))
    };
}

//==================================================================
template <typename TV, typename TM>
DMT_FINLINE Vec3<TV> V3__M44_Mul_V3W0( const Matrix44T<TM> &a, const Vec3<TV> &v )
{
    const auto &x = v[0];
    const auto &y = v[1];
    const auto &z = v[2];

    return {
        (TV)(a.mij(0,0) * x + a.mij(0,1) * y + a.mij(0,2) * z),
        (TV)(a.mij(1,0) * x + a.mij(1,1) * y + a.mij(1,2) * z),
        (TV)(a.mij(2,0) * x + a.mij(2,1) * y + a.mij(2,2) * z)
    };
}
//==================================================================
// NOTE: this should probably be marked as V3W0
template <typename TV, typename TM>
DMT_FINLINE Vec3<TV> V3__M44_Mul_V3W1( const Matrix44T<TM> &a, const Vec3<TV> &v )
{
    const auto &x = v[0];
    const auto &y = v[1];
    const auto &z = v[2];

    return {
        (TV)(a.mij(0,0) * x + a.mij(0,1) * y + a.mij(0,2) * z + a.mij(0,3)),
        (TV)(a.mij(1,0) * x + a.mij(1,1) * y + a.mij(1,2) * z + a.mij(1,3)),
        (TV)(a.mij(2,0) * x + a.mij(2,1) * y + a.mij(2,2) * z + a.mij(2,3))
    };
}

//==================================================================
template <typename TV, typename TM>
DMT_FINLINE Vec4<TV> V4__V3W1_Mul_M44( const Vec3<TV> &v, const Matrix44T<TM> &a )
{
    const auto &x = v[0];
    const auto &y = v[1];
    const auto &z = v[2];

    return {
        (TV)(x * a.mij(0,0) + y * a.mij(1,0) + z * a.mij(2,0) + a.mij(3,0)),
        (TV)(x * a.mij(0,1) + y * a.mij(1,1) + z * a.mij(2,1) + a.mij(3,1)),
        (TV)(x * a.mij(0,2) + y * a.mij(1,2) + z * a.mij(2,2) + a.mij(3,2)),
        (TV)(x * a.mij(0,3) + y * a.mij(1,3) + z * a.mij(2,3) + a.mij(3,3))
    };
}

//==================================================================
template <typename TV, typename TM>
DMT_FINLINE Vec4<TV> V4__V4_Mul_M44( const Vec4<TV> &v, const Matrix44T<TM> &a )
{
    const auto &x = v[0];
    const auto &y = v[1];
    const auto &z = v[2];
    const auto &w = v[3];

    return {
        (TV)(x * a.mij(0,0) + y * a.mij(1,0) + z * a.mij(2,0) + w * a.mij(3,0)),
        (TV)(x * a.mij(0,1) + y * a.mij(1,1) + z * a.mij(2,1) + w * a.mij(3,1)),
        (TV)(x * a.mij(0,2) + y * a.mij(1,2) + z * a.mij(2,2) + w * a.mij(3,2)),
        (TV)(x * a.mij(0,3) + y * a.mij(1,3) + z * a.mij(2,3) + w * a.mij(3,3))
    };
}
//==================================================================
template <typename TV, typename TM>
DMT_FINLINE Vec4<TV> V4__M44_Mul_V4( const Matrix44T<TM> &a, const Vec4<TV> &v )
{
    const auto &x = v[0];
    const auto &y = v[1];
    const auto &z = v[2];
    const auto &w = v[3];

    return {
        (TV)(a.mij(0,0) * x + a.mij(0,1) * y + a.mij(0,2) * z + a.mij(0,3) * w),
        (TV)(a.mij(1,0) * x + a.mij(1,1) * y + a.mij(1,2) * z + a.mij(1,3) * w),
        (TV)(a.mij(2,0) * x + a.mij(2,1) * y + a.mij(2,2) * z + a.mij(2,3) * w),
        (TV)(a.mij(3,0) * x + a.mij(3,1) * y + a.mij(3,2) * z + a.mij(3,3) * w)
    };
}


//==================================================================
template <typename TV, typename TM>
DMT_FINLINE Vec4<TV> V4__V3W0_Mul_M44( const Vec3<TV> &v, const Matrix44T<TM> &a )
{
    const auto &x = v[0];
    const auto &y = v[1];
    const auto &z = v[2];

    return {
        (TV)(x * a.mij(0,0) + y * a.mij(1,0) + z * a.mij(2,0)),
        (TV)(x * a.mij(0,1) + y * a.mij(1,1) + z * a.mij(2,1)),
        (TV)(x * a.mij(0,2) + y * a.mij(1,2) + z * a.mij(2,2)),
        (TV)(x * a.mij(0,3) + y * a.mij(1,3) + z * a.mij(2,3))
    };
}

//==================================================================
template <typename TV, typename TM>
DMT_FINLINE Vec3<TV> V3__V3W1_Mul_M44( const Vec3<TV> &v, const Matrix44T<TM> &a )
{
    const auto &x = v[0];
    const auto &y = v[1];
    const auto &z = v[2];

    return {
        (TV)(x * a.mij(0,0) + y * a.mij(1,0) + z * a.mij(2,0) + a.mij(3,0)),
        (TV)(x * a.mij(0,1) + y * a.mij(1,1) + z * a.mij(2,1) + a.mij(3,1)),
        (TV)(x * a.mij(0,2) + y * a.mij(1,2) + z * a.mij(2,2) + a.mij(3,2))
    };
}

//==================================================================
template <typename TV, typename TM>
DMT_FINLINE Vec3<TV> V3__V3W0_Mul_M44( const Vec3<TV> &v, const Matrix44T<TM> &a )
{
    const auto &x = v[0];
    const auto &y = v[1];
    const auto &z = v[2];

    return {
        (TV)(x * a.mij(0,0) + y * a.mij(1,0) + z * a.mij(2,0)),
        (TV)(x * a.mij(0,1) + y * a.mij(1,1) + z * a.mij(2,1)),
        (TV)(x * a.mij(0,2) + y * a.mij(1,2) + z * a.mij(2,2))
    };
}

//==================================================================
template <typename TV, typename TM>
DMT_FINLINE Vec3<TV> V3__V2Z0W0_Mul_M44( const Vec2<TV> &v, const Matrix44T<TM> &a )
{
    return {
        (TV)(v[0] * a.mij(0,0) + v[1] * a.mij(1,0)),
        (TV)(v[0] * a.mij(0,1) + v[1] * a.mij(1,1)),
        (TV)(v[0] * a.mij(0,2) + v[1] * a.mij(1,2))
    };
}

//==================================================================
template <typename TV, typename TM>
DMT_FINLINE Vec2<TV> V2__V2Z0W0_Mul_M44( const Vec2<TV> &v, const Matrix44T<TM> &a )
{
    return {
        (TV)(v[0] * a.mij(0,0) + v[1] * a.mij(1,0)),
        (TV)(v[0] * a.mij(0,1) + v[1] * a.mij(1,1))
    };
}

//==================================================================
template <typename TV, typename TM>
DMT_FINLINE Vec2<TV> V2__V2Z0W1_Mul_M44( const Vec2<TV> &v, const Matrix44T<TM> &a )
{
    return {
        (TV)(v[0] * a.mij(0,0) + v[1] * a.mij(1,0) + a.mij(3,0)),
        (TV)(v[0] * a.mij(0,1) + v[1] * a.mij(1,1) + a.mij(3,1))
    };
}

//==================================================================
template <typename TV, typename TM>
DMT_FINLINE Vec3<TV> DXFormPos( const Vec3<TV> &v, const Matrix44T<TM> &m )
{
    return V3__V3W1_Mul_M44<TV,TM>( v, m );
}
template <typename TM, typename TV>
DMT_FINLINE Vec3<TV> DXFormPos( const Matrix44T<TM> &m, const Vec3<TV> &v )
{
    return V3__M44_Mul_V3W1<TM,TV>( m, v );
}

template <typename TV, typename TM>
DMT_FINLINE Vec3<TV> DXFormNor( const Vec3<TV> &v, const Matrix44T<TM> &m )
{
    return V3__V3W0_Mul_M44<TV,TM>( v, m );
}

template <typename TV, typename TM>
DMT_FINLINE Vec3<TV> DXFormNorN( const Vec3<TV> &v, const Matrix44T<TM> &m )
{
    return DNormalize( V3__V3W0_Mul_M44<TV,TM>( v, m ) );
}

#endif

