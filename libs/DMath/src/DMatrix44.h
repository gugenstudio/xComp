//==================================================================
/// DMatrix44.h
///
/// Created by Davide Pasca - 2008/12/22
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#ifndef DMATRIX44_H
#define DMATRIX44_H

#include <string.h>
#include <stdio.h>
#include "DMathBase.h"
#include "DVector.h"
#include "DMatrix33.h"

//==================================================================
//#define DMATRIX44_ROWMTX_MODE

//==================================================================
template <typename T>
class Matrix44Packed
{
public:
    Matrix44Packed( const T *pSrc )
    {
        for (size_t i=0; i != 16; ++i)
            mData[i] = pSrc[i];
    }

    T   mData[16];
};

//==================================================================
template <typename T>
class alignas(DMT_SIMD_ALIGN_SIZE) Matrix44T
{
public:
    VecN<T,16>  v16;

    constexpr Matrix44T() = default;

    constexpr DMT_FINLINE Matrix44T( bool setToIdentity )
    {
        DMT_ASSERT( setToIdentity == true );
        Identity();
    }

    constexpr DMT_FINLINE Matrix44T( const T (&srcMtx)[16] )
    {
        CopyRowMajor( srcMtx );
    }

    constexpr DMT_FINLINE Matrix44T(
        const Vec4<T> &row0,
        const Vec4<T> &row1,
        const Vec4<T> &row2,
        const Vec4<T> &row3 )
    {
        *((Vec4<T> *)&v16[0*4]) = row0;
        *((Vec4<T> *)&v16[1*4]) = row1;
        *((Vec4<T> *)&v16[2*4]) = row2;
        *((Vec4<T> *)&v16[3*4]) = row3;
    }

    constexpr DMT_FINLINE Matrix44T(
        const T& m00_, const T& m01_, const T& m02_, const T& m03_,
        const T& m10_, const T& m11_, const T& m12_, const T& m13_,
        const T& m20_, const T& m21_, const T& m22_, const T& m23_,
        const T& m30_, const T& m31_, const T& m32_, const T& m33_ )
    {
        mij(0,0) = m00_; mij(0,1) = m01_; mij(0,2) = m02_; mij(0,3) = m03_;
        mij(1,0) = m10_; mij(1,1) = m11_; mij(1,2) = m12_; mij(1,3) = m13_;
        mij(2,0) = m20_; mij(2,1) = m21_; mij(2,2) = m22_; mij(2,3) = m23_;
        mij(3,0) = m30_; mij(3,1) = m31_; mij(3,2) = m32_; mij(3,3) = m33_;
    }

    template<typename T2>
    constexpr Matrix44T( const Matrix44T<T2> &s )
    {
        for (size_t i=0; i != 16; ++i)
            v16[i] = (T)s.v16[i];
    }

    constexpr DMT_FINLINE Matrix44T( const Matrix33T<T> &s )
    {
        mij(0,0)=s.mij(0,0); mij(0,1)=s.mij(0,1); mij(0,2)=s.mij(0,2); mij(0,3)=0;
        mij(1,0)=s.mij(1,0); mij(1,1)=s.mij(1,1); mij(1,2)=s.mij(1,2); mij(1,3)=0;
        mij(2,0)=s.mij(2,0); mij(2,1)=s.mij(2,1); mij(2,2)=s.mij(2,2); mij(2,3)=0;
        mij(3,0)=         0; mij(3,1)=         0; mij(3,2)=         0; mij(3,3)=1;
    }

    DMT_FINLINE Matrix44T( const Matrix44Packed<T> &src ) { v16 = VecN<T,16>( src.mData ); }

    constexpr const T &mij( size_t y, size_t x ) const    { return v16[ y * 4 + x ];  }
    constexpr       T &mij( size_t y, size_t x )          { return v16[ y * 4 + x ];  }

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

    constexpr DMT_FINLINE void SetZero()
    {
        v16 = VecN<T,16>( 0.f );
    }

    constexpr DMT_FINLINE void Identity()
    {
        v16 = VecN<T,16>( 0.f );

        mij(0,0) = mij(1,1) = mij(2,2) = mij(3,3) = 1.0f;
    }

    constexpr bool IsIdentity() const { return *this == Matrix44T(true); }

    constexpr bool IsValid() const { return v16.IsValid(); }

    inline Matrix44T GetTranspose() const;
    inline Matrix44T GetAs33() const;

    constexpr DMT_FINLINE Matrix33T<T> Make33() const
    {
        return { GetV3(0), GetV3(1), GetV3(2) };
    }

    inline Matrix44T GetOrthonormal() const;

    inline Matrix44Packed<T> MakePacked() const
    {
        return Matrix44Packed<T>( &v16[0] );
    }

    Matrix44T GetInverse() const;
    DMT_FINLINE Matrix44T GetAffineInverse() const;

    inline static Matrix44T Scale( const Vec3<T> &sca );
    inline static Matrix44T Scale( const T& sx, const T& sy, const T& sz );
    inline static Matrix44T Translate( const Vec3<T> &tra );
    inline static Matrix44T Translate( const T& tx, const T& ty, const T& tz );
    inline static Matrix44T Rot( const T& ang, const T& ax, const T& ay, const T& az );
    inline static Matrix44T Rot( const T& ang, const Vec3<T> &axis ) {
        return Rot( ang, axis[0], axis[1], axis[2] );
    }
    inline static Matrix44T OrthoRH(
                        T left,
                        T right,
                        T bottom,
                        T top,
                        T nearr,
                        T farr );

    inline static Matrix44T PerspectiveFrustumRH(
        T l, T r, T b, T t, T n, T f, unsigned flags );

    static Matrix44T LookAt(
            const Vec3<T> &eye,
            const Vec3<T> &at,
            const Vec3<T> &up,
            bool safeUpVec=true );

    constexpr void CopyRowMajor( const T (&srcMtx)[16] )
    {
        v16.LoadFromMemory( srcMtx );
    }

    constexpr DMT_FINLINE void SetTranslation( const Vec3<T> &tra )
    {
        mij(3,0) = tra[0];
        mij(3,1) = tra[1];
        mij(3,2) = tra[2];
    }

    constexpr DMT_FINLINE Vec3<T> GetTranslation() const
    {
        return {
            mij(3,0),
            mij(3,1),
            mij(3,2)
        };
    }

    void PrintOut() const;

    Matrix44T operator + (const Matrix44T &rval) const { Matrix44T ret; ret.v16 = v16 + rval.v16; return ret; }
    Matrix44T operator - (const Matrix44T &rval) const { Matrix44T ret; ret.v16 = v16 - rval.v16; return ret; }
//  Matrix44T operator * (const Matrix44T &rval) const { Matrix44T ret; ret.v16 = v16 * rval.v16; return ret; }
    Matrix44T operator / (const Matrix44T &rval) const { Matrix44T ret; ret.v16 = v16 / rval.v16; return ret; }
    Matrix44T operator * (const T rval) const    { Matrix44T ret; ret.v16 = v16 * rval; return ret; }
    //void   operator *= (const T rval)          { v16 *= rval; }

    friend bool operator ==( const Matrix44T &lval, const Matrix44T &rval ) { return lval.v16 == rval.v16; }
    friend bool operator !=( const Matrix44T &lval, const Matrix44T &rval ) { return lval.v16 != rval.v16; }

    bool IsSimilar( const Matrix44T &other, T eps ) const { return v16.IsSimilar( other.v16, eps ); }
};

//==================================================================
template <typename T>
DMT_FINLINE Matrix44T<T> Matrix44T<T>::GetTranspose() const
{
    Matrix44T<T>    out;

    for (int i=0; i < 4; ++i)
        for (int j=0; j < 4; ++j)
            out.mij(i,j) = mij(j,i);

    return out;
}

//==================================================================
template <typename T>
DMT_FINLINE Matrix44T<T> Matrix44T<T>::GetAs33() const
{
    Matrix44T<T>    out( true );

    for (int i=0; i < 3; ++i)
        for (int j=0; j < 3; ++j)
            out.mij(i,j) = mij(i,j);

    return out;
}

//==================================================================
template <typename T>
inline Matrix44T<T> Matrix44T<T>::GetOrthonormal() const
{
    // TODO: verify that this actually works !!
    Matrix44T<T> out( true );

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
    out.SetV3( 3, GetV3(3) );

    return out;
}

//==================================================================
template <typename T>
DMT_FINLINE Matrix44T<T> Matrix44T<T>::GetAffineInverse() const
{
    auto invRot = Make33().GetInverse();
    auto invTra = V3__V3_Mul_M33( -GetV3(3), invRot );

    auto out44 = Matrix44T<T>( invRot );
    out44.SetV3( 3, invTra );

    return out44;
}

//==================================================================
template <typename T>
DMT_FINLINE Matrix44T<T> Matrix44T<T>::Scale( const Vec3<T> &sca )
{
    return Scale( sca[0], sca[1], sca[2] );
}

//==================================================================
template <typename T>
DMT_FINLINE Matrix44T<T> Matrix44T<T>::Scale( const T& sx, const T& sy, const T& sz )
{
    Matrix44T<T> m;
    m.v16 = VecN<T,16>( (T)0 );
    m.mij(0,0) = sx;
    m.mij(1,1) = sy;
    m.mij(2,2) = sz;
    m.mij(3,3) = (T)1;

    return m;
}
//==================================================================
template <typename T>
inline Matrix44T<T> Matrix44T<T>::Translate( const Vec3<T> &tra )
{
    return Translate( tra.x(), tra.y(), tra.z() );
}
//==================================================================
template <typename T>
DMT_FINLINE Matrix44T<T> Matrix44T<T>::Translate( const T& tx, const T& ty, const T& tz )
{
    Matrix44T<T> m( true );
    m.mij(3,0) = tx;
    m.mij(3,1) = ty;
    m.mij(3,2) = tz;
    m.mij(3,3) = (T)1;

    return m;
}
//==================================================================
template <typename T>
DMT_FINLINE Matrix44T<T> Matrix44T<T>::Rot( const T& ang, const T& ax, const T& ay, const T& az )
{
    auto s = std::sin( ang );
    auto c = std::cos( ang );

    auto xx = ax * ax;  auto yy = ay * ay;  auto zz = az * az;
    auto xy = ax * ay;  auto yz = ay * az;  auto zx = az * ax;
    auto xs = ax * s;   auto ys = ay * s;   auto zs = az * s;
    auto one_c = 1 - c;

    return {
            (one_c * xx) + c,   (one_c * xy) + zs,  (one_c * zx) - ys,  0,
            (one_c * xy) - zs,  (one_c * yy) + c,   (one_c * yz) + xs,  0,
            (one_c * zx) + ys,  (one_c * yz) - xs,  (one_c * zz) + c,   0,
            0,                  0,                  0,                  1 };
}

//==================================================================
template <typename T>
inline Matrix44T<T> Matrix44T<T>::OrthoRH(
                        T left,
                        T right,
                        T bottom,
                        T top,
                        T nearr,
                        T farr )
{
    DMT_ASSERT( right != left && top != bottom && farr != nearr );

    auto rl = right - left;
    auto tb = top - bottom;
    auto fn = farr - nearr;

    auto tx = -(right + left) / rl;
    auto ty = -(top + bottom) / tb;
    auto tz = -(farr + nearr) / fn;

    return {
            2 / rl, 0,      0,          0,
            0,      2 / tb, 0,          0,
            0,      0,      -2 / fn,    0,
            tx,     ty,     tz,         1 };
}

#if 0
//==================================================================
template <typename T>
inline Matrix44T<T> Matrix44T<T>::PerspectiveFrustumRH( T l, T r, T b, T t, T n, T f )
{
    DMT_ASSERT( (r-l) != 0 && (t-b) != 0 && (f-n) != 0 );

    return {
          2 * n / (r-l),                  0,                   0,     0,
                      0,      2 * n / (t-b),                   0,     0,
          (r+l) / (r-l),      (t+b) / (t-b),      -(f+n) / (f-n),    -1,
                      0,                  0,    -(2*f*n) / (f-n),     0 };
}

//==================================================================
// See: http://www.terathon.com/gdc07_lengyel.pdf
template <typename T>
inline Matrix44T<T> Matrix44T<T>::PerspectiveFrustumRHInf( T l, T r, T b, T t, T n )
{
    DMT_ASSERT( (r-l) != 0 && (t-b) != 0 );

    static const auto EPS = (T)2.5e-7f;

    return {
          2 * n / (r-l),                  0,                0,     0,
                      0,      2 * n / (t-b),                0,     0,
          (r+l) / (r-l),      (t+b) / (t-b),     EPS - 1,         -1,
                      0,                  0,    (EPS - 2) * n,     0 };
}
#endif

//==================================================================
// see also: OVR_StereoProjection.cpp
enum : unsigned {
    DMAT44_PERSP_FLG_OPENGL_CLIP = 1,
    DMAT44_PERSP_FLG_FLIPZ       = 2,
    DMAT44_PERSP_FLG_FAR_AT_INF  = 4,
};

template <typename T>
inline Matrix44T<T> Matrix44T<T>::PerspectiveFrustumRH(
        T l, T r, T b, T t, T n, T f, unsigned flags )
{
    DMT_ASSERT( (r-l) != 0 && (t-b) != 0 && (f-n) != 0 );

    auto mtx = Matrix44T<T>(
          2 * n / (r-l),               0,   0,   0,
                      0,   2 * n / (t-b),   0,   0,
          (r+l) / (r-l),   (t+b) / (t-b),   0,  -1,
                      0,               0,   0,   0 );

    const auto flipZ = !!(flags & DMAT44_PERSP_FLG_FLIPZ);

    if ( (flags & DMAT44_PERSP_FLG_FAR_AT_INF) )
    {
        if ( (flags & DMAT44_PERSP_FLG_OPENGL_CLIP) )
        {
            // It's not clear this makes sense for OpenGL - you don't get the same precision benefits you do in D3D.
            const auto sign = flipZ ? 1.f : -1.f;
            mtx.mij(2,2) = sign * 1.f;
            mtx.mij(3,2) = sign * 2.0f * n;
        }
        else
        {
            // no option for now.. see some other time
            DMT_ASSERT( flipZ );
            mtx.mij(2,2) = 0.0f;
            mtx.mij(3,2) = n;
        }
    }
    else
    {
        // NOTE: default OpenGL definition has '-' and (f - n)
        //  this implementations simply uses (n - f)
        if ( (flags & DMAT44_PERSP_FLG_OPENGL_CLIP) )
        {
            // Clip range is [-w,+w], so 0 is at the middle of the range.
            mtx.mij(2,2) =         (flipZ ? -1.0f : 1.0f) * (n + f) / (n - f);
            mtx.mij(3,2) = 2.0f * ((flipZ ? -f    : f   ) * n)      / (n - f);
        }
        else
        {
            // Clip range is [0,+w], so 0 is at the start of the range.
            mtx.mij(2,2) =  (flipZ ? -n : f)      / (n - f);
            mtx.mij(3,2) = ((flipZ ? -f : f) * n) / (n - f);
        }
    }

    return mtx;
}

//==================================================================
template <typename T>
DMT_FINLINE Matrix44T<T> operator * (const Matrix44T<T> &m1, const Matrix44T<T> &m2)
{
    Matrix44T<T> tmp;
    for (size_t r=0; r < 4; ++r)
    {
        for (size_t c=0; c < 4; ++c)
        {
            T sum = 0;
            for (size_t i=0; i < 4; ++i)
#ifdef DMATRIX44_ROWMTX_MODE
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
template <typename T>
void Matrix44T<T>::PrintOut() const
{
	printf( "[" );
	for (size_t r=0; r < 4; ++r)
	{
		if ( r != 0 )
			printf( " |" );

		for (size_t c=0; c < 4; ++c)
		{
			printf( " %f", mij(r,c) );
		}
	}
	printf( "]\n" );
}

//==================================================================
template <typename T>
DMT_FINLINE T Matrix44_det3x3(T a1,T a2,T a3,
                              T b1,T b2,T b3,
                              T c1,T c2,T c3)
{
#define MATRIX44_DET2X2(a,b,c,d) (a * d - b * c)

    return   a1 * MATRIX44_DET2X2( b2, b3, c2, c3 )
           - b1 * MATRIX44_DET2X2( a2, a3, c2, c3 )
           + c1 * MATRIX44_DET2X2( a2, a3, b2, b3 );

#undef MATRIX44_DET2X2
}

//==================================================================
template <typename T>
DMT_FINLINE T Matrix44_det4x4(T a1, T a2, T a3, T a4,
                              T b1, T b2, T b3, T b4,
                              T c1, T c2, T c3, T c4,
                              T d1, T d2, T d3, T d4)
{
    //
    return   a1 * Matrix44_det3x3( b2, b3, b4, c2, c3, c4, d2, d3, d4)
           - b1 * Matrix44_det3x3( a2, a3, a4, c2, c3, c4, d2, d3, d4)
           + c1 * Matrix44_det3x3( a2, a3, a4, b2, b3, b4, d2, d3, d4)
           - d1 * Matrix44_det3x3( a2, a3, a4, b2, b3, b4, c2, c3, c4);
}

//==================================================================
template <typename T>
Matrix44T<T> Matrix44T<T>::GetInverse() const
{
    auto a1=mij(0,0); auto b1=mij(0,1); auto c1=mij(0,2); auto d1=mij(0,3);
    auto a2=mij(1,0); auto b2=mij(1,1); auto c2=mij(1,2); auto d2=mij(1,3);
    auto a3=mij(2,0); auto b3=mij(2,1); auto c3=mij(2,2); auto d3=mij(2,3);
    auto a4=mij(3,0); auto b4=mij(3,1); auto c4=mij(3,2); auto d4=mij(3,3);

    auto det = Matrix44_det4x4( a1, a2, a3, a4,
                                b1, b2, b3, b4,
                                c1, c2, c3, c4,
                                d1, d2, d3, d4 );

    if ( fabs(det) < (T)1e-20 )
    {
        DMT_ASSERT( 0 );
        return *this;
    }

    auto oodet = (T)1 / det;

    return {
         Matrix44_det3x3( b2, b3, b4, c2, c3, c4, d2, d3, d4) * oodet,
        -Matrix44_det3x3( b1, b3, b4, c1, c3, c4, d1, d3, d4) * oodet,
         Matrix44_det3x3( b1, b2, b4, c1, c2, c4, d1, d2, d4) * oodet,
        -Matrix44_det3x3( b1, b2, b3, c1, c2, c3, d1, d2, d3) * oodet,

        -Matrix44_det3x3( a2, a3, a4, c2, c3, c4, d2, d3, d4) * oodet,
         Matrix44_det3x3( a1, a3, a4, c1, c3, c4, d1, d3, d4) * oodet,
        -Matrix44_det3x3( a1, a2, a4, c1, c2, c4, d1, d2, d4) * oodet,
         Matrix44_det3x3( a1, a2, a3, c1, c2, c3, d1, d2, d3) * oodet,

         Matrix44_det3x3( a2, a3, a4, b2, b3, b4, d2, d3, d4) * oodet,
        -Matrix44_det3x3( a1, a3, a4, b1, b3, b4, d1, d3, d4) * oodet,
         Matrix44_det3x3( a1, a2, a4, b1, b2, b4, d1, d2, d4) * oodet,
        -Matrix44_det3x3( a1, a2, a3, b1, b2, b3, d1, d2, d3) * oodet,

        -Matrix44_det3x3( a2, a3, a4, b2, b3, b4, c2, c3, c4) * oodet,
         Matrix44_det3x3( a1, a3, a4, b1, b3, b4, c1, c3, c4) * oodet,
        -Matrix44_det3x3( a1, a2, a4, b1, b2, b4, c1, c2, c4) * oodet,
         Matrix44_det3x3( a1, a2, a3, b1, b2, b3, c1, c2, c3) * oodet,
    };
}

//==================================================================
template <typename T>
Matrix44T<T> Matrix44T<T>::LookAt(
            const Vec3<T> &eye,
            const Vec3<T> &at,
            const Vec3<T> &up,
            bool safeUpVec )
{
    // build on the LookAt() from Matrix33
    auto mtx33 = Matrix33T<T>::LookAt( eye, at, up, safeUpVec );

    auto mtx44 = Matrix44T<T>( mtx33 );
    mtx44.SetV3( 3, eye );

	return mtx44;
}

//==================================================================
#include "DMatrix44_XForms.h"

//==================================================================
using Matrix44 = Matrix44T<float>;
using Matrix44D = Matrix44T<double>;

#endif

