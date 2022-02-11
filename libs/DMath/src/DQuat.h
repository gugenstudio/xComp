//==================================================================
/// DQuat.h
///
/// Created by Davide Pasca - 2012/12/18
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#ifndef DQUAT_H
#define DQUAT_H

#include "DMatrix33.h"
#include "DMatrix44.h"
#include "DVector.h"

//==================================================================
class Quat
{
    Float4  v4;
public:
    Quat( bool doInit=false )          { if ( doInit ) SetIdentity(); }
    Quat( const Quat &v_ )			   { v4[0]=v_[0]; v4[1]=v_[1]; v4[2]=v_[2]; v4[3]=v_[3]; }
    Quat( float w_, const Float3 &v_ ) { v4[0]=v_[0]; v4[1]=v_[1]; v4[2]=v_[2]; v4[3]=w_; }

    Quat( const float *p ) = delete;

    // from: http://www.euclideanspace.com/maths/geometry/rotations/conversions/matrixToQuaternion/
    Quat( const Matrix33 &m )
    {
#if 1
        auto m00 = m.mij(0,0);
        auto m11 = m.mij(1,1);
        auto m22 = m.mij(2,2);
        auto sat = [](float x) { return DMax( 0.f, x ); };
        v4[3] = sqrtf( sat( 1 + m00 + m11 + m22 ) ) * 0.5f;
        v4[0] = sqrtf( sat( 1 + m00 - m11 - m22 ) ) * 0.5f;
        v4[1] = sqrtf( sat( 1 - m00 + m11 - m22 ) ) * 0.5f;
        v4[2] = sqrtf( sat( 1 - m00 - m11 + m22 ) ) * 0.5f;
        v4[0] = (float)copysign( v4[0], m.mij(2,1) - m.mij(1,2) );
        v4[1] = (float)copysign( v4[1], m.mij(0,2) - m.mij(2,0) );
        v4[2] = (float)copysign( v4[2], m.mij(1,0) - m.mij(0,1) );
#else
        float trace = m.mij(0,0) + m.mij(1,1) + m.mij(2,2);
        if ( trace > 0 )
        {
            float s = 0.5f / sqrtf(trace+ 1.0f);
            v4[3] = 0.25f / s;
            v4[0] = ( m.mij(2,1) - m.mij(1,2) ) * s;
            v4[1] = ( m.mij(0,2) - m.mij(2,0) ) * s;
            v4[2] = ( m.mij(1,0) - m.mij(0,1) ) * s;
        }
        else
        if ( m.mij(0,0) > m.mij(1,1) && m.mij(0,0) > m.mij(2,2) )
        {
            float s = 2.0f * sqrtf( 1.0f + m.mij(0,0) - m.mij(1,1) - m.mij(2,2));
            v4[3] = (m.mij(2,1) - m.mij(1,2) ) / s;
            v4[0] = 0.25f * s;
            v4[1] = (m.mij(0,1) + m.mij(1,0) ) / s;
            v4[2] = (m.mij(0,2) + m.mij(2,0) ) / s;
        }
        else
        if (m.mij(1,1) > m.mij(2,2))
        {
            float s = 2.0f * sqrtf( 1.0f + m.mij(1,1) - m.mij(0,0) - m.mij(2,2));
            v4[3] = (m.mij(0,2) - m.mij(2,0) ) / s;
            v4[0] = (m.mij(0,1) + m.mij(1,0) ) / s;
            v4[1] = 0.25f * s;
            v4[2] = (m.mij(1,2) + m.mij(2,1) ) / s;
        }
        else
        {
            float s = 2.0f * sqrtf( 1.0f + m.mij(2,2) - m.mij(0,0) - m.mij(1,1) );
            v4[3] = (m.mij(1,0) - m.mij(0,1) ) / s;
            v4[0] = (m.mij(0,2) + m.mij(2,0) ) / s;
            v4[1] = (m.mij(1,2) + m.mij(2,1) ) / s;
            v4[2] = 0.25f * s;
        }
#endif

        v4[3] = -v4[3]; // TODO: works, but why ?!!
    }

private:
    Quat( const Float4 &v4_ ) : v4(v4_) {} // private to avoid confusion
public:
    Quat operator + (float rval) const       { return Quat( v4 + rval ); }
    Quat operator - (float rval) const       { return Quat( v4 - rval ); }
    Quat operator * (float rval) const       { return Quat( v4 * rval ); }
    Quat operator / (float rval) const       { return Quat( v4 / rval ); }
    Quat operator + (const Quat &rval) const { return Quat( v4 + rval.v4 ); }
    Quat operator - (const Quat &rval) const { return Quat( v4 - rval.v4 ); }

    // q * v
    Quat operator * (const Float3 &v) const { return *this * Quat(0,v); }

    // v * q
    friend Quat operator * (const Float3 &v, const Quat &q) { return Quat(0,v) * q; }

private:
    template <typename T>
    static inline Vec4<T> mulQuaQua( const Vec4<T> &ql, const Vec4<T> &qr )
    {
        return { ql[3]*qr[0] + ql[0]*qr[3] + ql[1]*qr[2] - ql[2]*qr[1],   // x
                 ql[3]*qr[1] + ql[1]*qr[3] + ql[2]*qr[0] - ql[0]*qr[2],   // y
                 ql[3]*qr[2] + ql[2]*qr[3] + ql[0]*qr[1] - ql[1]*qr[0],   // z
                 ql[3]*qr[3] - ql[0]*qr[0] - ql[1]*qr[1] - ql[2]*qr[2] }; // w
    }

public:
    Quat operator * (const Quat &qr) const // q * q
    {
        const auto res = mulQuaQua( this->v4, qr.v4 );

        return { res[3], {res[0], res[1], res[2]} };
    }

    friend Quat operator * (float lval, const Quat &rval) { return rval * lval; } // s * q

    Quat operator -() const	{ return Quat( -v4 ); }
    Quat operator +=(const Quat &rval) { *this = *this + rval; return *this; }
    Quat operator *=(const Quat &rval) { *this = *this * rval; return *this; }
    Quat operator *=(const float s)    { *this = *this * s; return *this; }

    friend Quat	DMin( const Quat &a, const Quat &b ) { return Quat( DMin( a.v4, b.v4 ) ); }
    friend Quat	DMax( const Quat &a, const Quat &b ) { return Quat( DMax( a.v4, b.v4 ) ); }

    const float &operator [] (size_t i) const { return v4[i]; }
          float &operator [] (size_t i)       { return v4[i]; }

    void SetIdentity() { *this = Quat(1,{0,0,0}); }

    bool IsIdentity() { return v4[0]==0 && v4[1]==0 && v4[2]==0 && v4[3]==1; }

#if 0
    static Quat FromNormal( const Float3 &n )
    {
        DMT_ASSERT( DIsUnitVec(n) );
        // determine the up vector (0,1,0) unless too close to normal
        Float3 up = (fabs(n[1]) < 0.9f ? Float3(0,1,0) : Float3(1,0,0));
        Float3 H = DNormalize( up + n );
        return Quat(
                DDot(up, H),
               {up[1]*H[2] - up[2]*H[1],
                up[2]*H[0] - up[0]*H[2],
                up[0]*H[1] - up[1]*H[0]} );
    }

    static Quat FromNormalVectors( const Float3 &v, const Float3 &w )
    {
        DMT_ASSERT( DIsUnitVec(v) && DIsUnitVec(w) );
        auto cosa = DDot( v, w );
        auto ang  = acos( DClamp( cosa, -1.f, 1.f ) );
        auto axis = DCross( v, w );
        return Quat::Rot( ang, axis );
    }

    // from http://math.stackexchange.com/a/60556
    static Quat FromNormalVectors( const Float3 &v, const Float3 &w, const Float3 &u )
    {
        DMT_ASSERT( DIsUnitVec(v) && DIsUnitVec(w) && DIsUnitVec(u) );
        auto vproj = DNormalize( v - DDot( v, u ) * u );
        auto wproj = DNormalize( w - DDot( w, u ) * u );
        return Quat::FromNormalVectors( vproj, wproj );
    }
#endif

    DMT_FINLINE static Quat Rot( float ang, const Float3 &axis )
    {
        DMT_ASSERT( DIsUnitVec(axis) );
        float hang = ang * 0.5f;
        float co = cosf( hang );
        float si = sinf( hang );
        return Quat( co, axis * si );
    }

    DMT_FINLINE static Quat Rot( float ang, float x, float y, float z ) {
        return Rot( ang, { x, y, z} );
    }

    //==================================================================
    DMT_FINLINE friend float DDot( const Quat& a, const Quat& b ) { return DDot( a.v4, b.v4 ); }
    DMT_FINLINE friend float DLengthSqr( const Quat& a )  { return DLengthSqr( a.v4 ); }
    DMT_FINLINE friend float DLength( const Quat& a )     { return DLength( a.v4 ); }
    DMT_FINLINE friend Quat  DNormalize( const Quat& v )  { return v * DRSqrt( DLengthSqr( v ) ); }

    Quat GetConjugate() const { return Quat( v4[3], { -v4[0], -v4[1], -v4[2] } ); }
    Quat GetInverse() const { return GetConjugate() / DLength( *this ); }
    Float3 GetVector() const { return Float3( v4[0], v4[1], v4[2] ); }

    Float3 RotateVec( const Float3 &v ) const
    {
        return (*this * v * this->GetConjugate()).GetVector();
    }

    Float3 RotateVecInv( const Float3 &v ) const
    {
        return (this->GetConjugate() * v * *this).GetVector();
    }

    Double3 RotateVecD( const Double3 &v ) const;

    Matrix33 ToMatrix33() const
    {
        float xs = v4[0] * 2.0f;
        float ys = v4[1] * 2.0f;
        float zs = v4[2] * 2.0f;

        float xx = v4[0] * xs;
        float wx = v4[3] * xs;

        float xy = v4[0] * ys;
        float yy = v4[1] * ys;
        float wy = v4[3] * ys;

        float xz = v4[0] * zs;	
        float yz = v4[1] * zs;
        float zz = v4[2] * zs;
        float wz = v4[3] * zs;

        return Matrix33(
            1.0f - (yy + zz), xy + wz,          xz - wy,
            xy - wz,          1.0f - (xx + zz), yz + wx,
            xz + wy,          yz - wx,          1.0f - (xx + yy) );
    }
    Matrix44 ToMatrix() const
    {
        return Matrix44( ToMatrix33() );
    }
};

//==================================================================
DMT_FINLINE Double3 Quat::RotateVecD( const Double3 &v ) const
{
    const auto v4d      = (Double4)v4;
    const auto conj_v4d = (Double4)GetConjugate().v4;

    // q * v ...
    const auto q_v = mulQuaQua( v4d, Double4( v[0], v[1], v[2], 0 ) );
    // ... * q^-1
    const auto q_v_cq = mulQuaQua( q_v, conj_v4d );

    return {q_v_cq[0], q_v_cq[1], q_v_cq[2]};
}

//==================================================================
template <typename _TB>
inline Quat QuatNLerp( const Quat &a, const Quat &b, const _TB &t )
{
    if ( DDot( a, b ) < 0 )
        return DNormalize( b + (a - b) * t );
    else
        return DNormalize( a + (b - a) * t );
}

//==================================================================
// Closest link:
// http://www.euclideanspace.com/maths/algebra/realNormedAlgebra/quaternions/slerp/
class QuatSlerp
{
    Quat    mQa;
    Quat    mQb;
    float   mAB_angle = 0.0f;
    float   mAB_oosin = 1.0f;
    bool    mNoInterp = true;

public:
    QuatSlerp( const Quat &qa, const Quat &qb )
        : mQa(qa)
        , mQb(qb)
    {
        auto cosa = DDot( mQa, mQb );
        if ( cosa < 0 )
        {
            cosa = -cosa;
            mQb = -qb;
        }

        if ( cosa >= 1.0f )
            return;

        mAB_angle = DSafeACos( cosa );
        mNoInterp = false;

        float sina = sinf( mAB_angle );
        const float EPS = 0.001f;
        if ( sina > EPS || sina < -EPS )
            mAB_oosin = 1.0f / sina;
    }

    DMT_FINLINE Quat Intpl( float t ) const
    {
        if ( mNoInterp )
            return mQa;

        auto spf = sinf( (1.0f-t) * mAB_angle ) * mAB_oosin;
        auto sqf = sinf(       t  * mAB_angle ) * mAB_oosin;

        return spf * mQa + sqf * mQb;
    }
};

#endif

