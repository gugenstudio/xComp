//==================================================================
/// DMatrix33.h
///
/// Created by Davide Pasca - 2015/1/10
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#ifndef DMATRIX33_H
#define DMATRIX33_H

#include <math.h>
#include "DVector.h"

//#define DMATRIX33_ROWMTX_MODE

//==================================================================
template <typename T>
class Matrix33T
{
public:
    VecN<T,9>  vec;

    constexpr Matrix33T() = default;

    constexpr DMT_FINLINE Matrix33T( bool setToIdentity )
    {
        DMT_ASSERT( setToIdentity == true );
        Identity();
    }

    constexpr DMT_FINLINE Matrix33T( const T (&srcMtx)[9] )
    {
        CopyRowMajor( srcMtx );
    }

    constexpr DMT_FINLINE Matrix33T(
        const Vec3<T> &row0,
        const Vec3<T> &row1,
        const Vec3<T> &row2 )
    {
        *((Vec3<T> *)&vec[0*3]) = row0;
        *((Vec3<T> *)&vec[1*3]) = row1;
        *((Vec3<T> *)&vec[2*3]) = row2;
    }

    constexpr DMT_FINLINE Matrix33T(
        T m00_, T m01_, T m02_,
        T m10_, T m11_, T m12_,
        T m20_, T m21_, T m22_ )
    {
        mij(0,0) = m00_; mij(0,1) = m01_; mij(0,2) = m02_;
        mij(1,0) = m10_; mij(1,1) = m11_; mij(1,2) = m12_;
        mij(2,0) = m20_; mij(2,1) = m21_; mij(2,2) = m22_;
    }

    template<typename T2>
    constexpr Matrix33T( const Matrix33T<T2> &s )
    {
        for (size_t i=0; i != 9; ++i)
            vec[i] = (T)s.vec[i];
    }

    constexpr const T &mij( size_t y, size_t x ) const    { return vec[ y * 3 + x ];  }
    constexpr       T &mij( size_t y, size_t x )          { return vec[ y * 3 + x ];  }

    constexpr DMT_FINLINE const Vec3<T> &GetV3( size_t idx ) const
    {
        return *(const Vec3<T> *)&mij(idx,0);
    }

    constexpr DMT_FINLINE void SetV3( size_t idx, const Vec3<T> &v )
    {
        mij(idx,0) = v[0];
        mij(idx,1) = v[1];
        mij(idx,2) = v[2];
    }

    constexpr DMT_FINLINE void Identity()
    {
        vec = VecN<T,9>( 0.f );

        mij(0,0) = mij(1,1) = mij(2,2) = 1.0f;
    }

    constexpr bool IsIdentity() const { return *this == Matrix33T(true); }

    constexpr bool IsValid() const { return vec.IsValid(); }

    inline Matrix33T GetTranspose() const;
    inline Matrix33T GetOrthonormal() const;

    Matrix33T GetInverse() const;

    inline static Matrix33T Scale( T sx, T sy, T sz );
    inline static Matrix33T Scale( const Vec3<T> &sca ) {
        return Scale( sca[0], sca[1], sca[2] );
    }

    inline static Matrix33T Rot( T ang, T ax, T ay, T az );
    inline static Matrix33T Rot( T ang, const Vec3<T> &axis ) {
        return Rot( ang, axis[0], axis[1], axis[2] );
    }

    static Matrix33T LookAt(
            const Vec3<T> &eye,
            const Vec3<T> &at,
            const Vec3<T> &up,
            bool safeUpVec=true );

    constexpr void CopyRowMajor( const T (&srcMtx)[9] )
    {
        vec.LoadFromMemory( srcMtx );
    }

    Matrix33T operator + (const Matrix33T &rval) const { Matrix33T ret; ret.vec = vec + rval.vec; return ret; }
    Matrix33T operator - (const Matrix33T &rval) const { Matrix33T ret; ret.vec = vec - rval.vec; return ret; }
    Matrix33T operator / (const Matrix33T &rval) const { Matrix33T ret; ret.vec = vec / rval.vec; return ret; }
    Matrix33T operator * (const T rval) const     { Matrix33T ret; ret.vec = vec * rval; return ret; }

    friend bool operator ==( const Matrix33T &lval, const Matrix33T &rval ) { return lval.vec == rval.vec; }
    friend bool operator !=( const Matrix33T &lval, const Matrix33T &rval ) { return lval.vec != rval.vec; }

    bool IsSimilar( const Matrix33T &other, T eps ) const { return vec.IsSimilar( other.vec, eps ); }
};

//==================================================================
template <typename T>
DMT_FINLINE Matrix33T<T> Matrix33T<T>::GetTranspose() const
{
    Matrix33T<T>    out;

    for (int i=0; i < 3; ++i)
        for (int j=0; j < 3; ++j)
            out.mij(i,j) = mij(j,i);

    return out;
}

//==================================================================
template <typename T>
inline Matrix33T<T> Matrix33T<T>::GetOrthonormal() const
{
    // TODO: verify that this actually works !!
    Matrix33T<T> out;

    auto v0 = GetV3(0);
    auto v1 = GetV3(1);
    auto v2 = GetV3(2);

    v0 = v0.GetNormalized();
    v1 = v2.GetCross( v0 );
    v1 = v1.GetNormalized();
    v2 = v0.GetCross( v1 );

    out.SetV3( 0, v0 );
    out.SetV3( 1, v1 );
    out.SetV3( 2, v2 );

    return out;
}

//==================================================================
template <typename T>
DMT_FINLINE Matrix33T<T> Matrix33T<T>::GetInverse() const
{
    auto determinant =
           mij(0,0) * (mij(1,1) * mij(2,2) - mij(2,1) * mij(1,2))
        + -mij(0,1) * (mij(1,0) * mij(2,2) - mij(1,2) * mij(2,0))
        +  mij(0,2) * (mij(1,0) * mij(2,1) - mij(1,1) * mij(2,0));

    auto invdet = 1 / determinant;

    return {
         (mij(1,1) * mij(2,2) - mij(2,1) * mij(1,2)) * invdet,
        -(mij(0,1) * mij(2,2) - mij(0,2) * mij(2,1)) * invdet,
         (mij(0,1) * mij(1,2) - mij(0,2) * mij(1,1)) * invdet,
        -(mij(1,0) * mij(2,2) - mij(1,2) * mij(2,0)) * invdet,
         (mij(0,0) * mij(2,2) - mij(0,2) * mij(2,0)) * invdet,
        -(mij(0,0) * mij(1,2) - mij(1,0) * mij(0,2)) * invdet,
         (mij(1,0) * mij(2,1) - mij(2,0) * mij(1,1)) * invdet,
        -(mij(0,0) * mij(2,1) - mij(2,0) * mij(0,1)) * invdet,
         (mij(0,0) * mij(1,1) - mij(1,0) * mij(0,1)) * invdet };
}

//==================================================================
template <typename T>
DMT_FINLINE Matrix33T<T> Matrix33T<T>::Scale( T sx, T sy, T sz )
{
    Matrix33T<T> m;
    m.vec = VecN<T,9>( 0.f );
    m.mij(0,0) = sx;
    m.mij(1,1) = sy;
    m.mij(2,2) = sz;

    return m;
}

//==================================================================
template <typename T>
DMT_FINLINE Matrix33T<T> Matrix33T<T>::Rot( T ang, T ax, T ay, T az )
{
    T   xx, yy, zz, xy, yz, zx, xs, ys, zs;

    auto s = std::sin( ang );
    auto c = std::cos( ang );

    xx = ax * ax;   yy = ay * ay;   zz = az * az;
    xy = ax * ay;   yz = ay * az;   zx = az * ax;
    xs = ax * s;    ys = ay * s;    zs = az * s;
    auto one_c = 1 - c;

    return {(one_c * xx) + c,   (one_c * xy) + zs,  (one_c * zx) - ys,
            (one_c * xy) - zs,  (one_c * yy) + c,   (one_c * yz) + xs,
            (one_c * zx) + ys,  (one_c * yz) - xs,  (one_c * zz) + c };
}

//==================================================================
template <typename T>
DMT_FINLINE Matrix33T<T> Matrix33T<T>::LookAt(
            const Vec3<T> &eye,
            const Vec3<T> &at,
            const Vec3<T> &up,
            bool safeUpVec )
{
    const auto EPS = (T)0.01;

    auto za = DNormalize( eye - at );

    DMT_ASSERT( DLengthSqr( za ) >= (EPS*EPS) );

    auto up2 = up;
    if ( safeUpVec && DAreVecParallel( za, up, EPS ) )
        up2 = { up[1], up[2], up[0] };

    DMT_ASSERT( DLengthSqr( up2 ) >= (EPS*EPS) );
    DMT_ASSERT( DAreVecParallel( za, up2, EPS ) == false );

    auto xa = DNormalize( DCross( up2, za ) );
    auto ya = DCross( za, xa );

    return Matrix33T<T>(
             xa[0], ya[0], za[0],
             xa[1], ya[1], za[1],
             xa[2], ya[2], za[2] ).GetInverse();
}

//==================================================================
template <typename T>
DMT_FINLINE Matrix33T<T> operator * (const Matrix33T<T> &m1, const Matrix33T<T> &m2)
{
    Matrix33T<T>        tmp;
    for (size_t r=0; r < 3; ++r)
    {
        for (size_t c=0; c < 3; ++c)
        {
            T   sum = 0;
            for (size_t i=0; i < 3; ++i)
#ifdef DMATRIX33_ROWMTX_MODE
                sum += m1.mij(i,c) * m2.mij(r,i);
#else
                sum += m1.mij(r,i) * m2.mij(i,c);
#endif

            tmp.mij(r,c) = sum;
        }

    }

    return tmp;
}

//==================================================================
template <typename TV, typename TM>
DMT_FINLINE Vec3<TV> V3__V3_Mul_M33( const Vec3<TV> &v, const Matrix33T<TM> &a )
{
    const auto &x = v[0];
    const auto &y = v[1];
    const auto &z = v[2];

    return {
        x * a.mij(0,0) + y * a.mij(1,0) + z * a.mij(2,0),
        x * a.mij(0,1) + y * a.mij(1,1) + z * a.mij(2,1),
        x * a.mij(0,2) + y * a.mij(1,2) + z * a.mij(2,2)
    };
}

//==================================================================
template <typename TV, typename TM>
DMT_FINLINE Vec3<TV> V3__M33_Mul_V3( const Matrix33T<TM> &a, const Vec3<TV> &v )
{
    const auto &x = v[0];
    const auto &y = v[1];
    const auto &z = v[2];

    return {
        a.mij(0,0) * x + a.mij(0,1) * y + a.mij(0,2) * z,
        a.mij(1,0) * x + a.mij(1,1) * y + a.mij(1,2) * z,
        a.mij(2,0) * x + a.mij(2,1) * y + a.mij(2,2) * z
    };
}

//==================================================================
template <typename TV, typename TM>
DMT_FINLINE Vec3<TV> DXFormNor( const Vec3<TV> &v, const Matrix33T<TM> &m )
{
    return V3__V3_Mul_M33<TV,TM>( v, m );
}

template <typename TV, typename TM>
DMT_FINLINE Vec3<TV> DXFormNorN( const Vec3<TV> &v, const Matrix33T<TM> &m )
{
    return DNormalize( V3__V3_Mul_M33<TV,TM>( v, m ) );
}

//==================================================================
using Matrix33 = Matrix33T<float>;

#endif

