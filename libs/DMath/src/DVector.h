//==================================================================
/// DVector.h
///
/// Created by Davide Pasca - 2009/5/2
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#ifndef DVECTOR_H
#define DVECTOR_H

#include "DMathBase.h"
#include "DMT_VecN.h"

//==================================================================
/// Vec2
//==================================================================
template <typename T>
class Vec2
{
public:
    static constexpr size_t N = 2;

    VecN<T,N>   mV;

    typedef T value_type;

    //using VT = Vec2;
    #define VT Vec2

public:
    constexpr VT() = default;

    template <typename T2> constexpr Vec2( const Vec2<T2> &v )
    {
        mV[0] = (T)v[0];
        mV[1] = (T)v[1];
    }

    template <typename T2> constexpr Vec2( const VecN<T2,N> &v )
    {
        mV = v;
    }

    constexpr VT( const T& a ) { mV = { a }; }

    constexpr VT( const T& x, const T& y )
        : mV( std::array<T,N>{ x, y } )
    {}

    static constexpr size_t size() { return N; }

    constexpr void LoadVals( const T (&p)[N] ) { mV.LoadFromMemory( p ); }

    constexpr void SetZero()      { mV.SetZero(); }
    constexpr bool IsZero() const { return mV.IsZero(); }
    constexpr bool IsValid() const { return mV.IsValid(); }

    constexpr VT operator + (const T &rval) const { return mV + rval; }
    constexpr VT operator - (const T &rval) const { return mV - rval; }
    constexpr VT operator * (const T &rval) const { return mV * rval; }
    constexpr VT operator / (const T &rval) const { return mV / rval; }
    constexpr VT operator + (const VT &rval) const { return mV + rval.mV; }
    constexpr VT operator - (const VT &rval) const { return mV - rval.mV; }
    constexpr VT operator * (const VT &rval) const { return mV * rval.mV; }
    constexpr VT operator / (const VT &rval) const { return mV / rval.mV; }

    constexpr VT operator -() const { return -mV; }

    constexpr VT operator +=(const VT &rval) { *this = *this + rval; return *this; }
    constexpr VT operator -=(const VT &rval) { *this = *this - rval; return *this; }
    constexpr VT operator *=(const VT &rval) { *this = *this * rval; return *this; }

              friend bool operator ==(const VT &l, const VT &r){return l.mV == r.mV;}
              friend bool operator !=(const VT &l, const VT &r){return l.mV != r.mV;}
              friend bool operator  <(const VT &l, const VT &r){return l.mV  < r.mV;}

    constexpr bool IsSimilar( const VT &r, const T &eps ) const
    {
        return mV.IsSimilar( r.mV, eps );
    }

    constexpr friend VT DPow( const VT &a, const VT &b ) { return DPow( a.mV, b.mV ); }
    constexpr friend VT DSign( const VT &a )             { return DSign( a.mV ); }
    constexpr friend VT DAbs( const VT &a )              { return DAbs( a.mV ); }
    constexpr friend VT DMin( const VT &a, const VT &b ) { return DMin( a.mV, b.mV ); }
    constexpr friend VT DMax( const VT &a, const VT &b ) { return DMax( a.mV, b.mV ); }
    constexpr friend T  DDot( const VT &a, const VT &b ) { return DDot( a.mV, b.mV ); }

    constexpr friend bool DAreVecParallel( const VT &a, const VT &b, const T &eps )
    {
        auto cosa_abs = DAbs( DDot( a, b ) );
        return cosa_abs >= (1-eps) && cosa_abs <= (1+eps);
    }

    constexpr T GetDot( const VT &r ) const { return GetDot( r.mV ); }

    constexpr VT GetNormalized() const    { return mV.GetNormalized(); }
    constexpr T GetLengthSqr() const      { return mV.GetLengthSqr(); }
    constexpr T GetLength() const         { return mV.GetLength(); }

    constexpr const T &x() const { return mV[0];  }
    constexpr const T &y() const { return mV[1];  }
    constexpr       T &x()       { return mV[0];  }
    constexpr       T &y()       { return mV[1];  }

    constexpr const T &operator [] (size_t i) const   { return mV[i]; }
    constexpr       T &operator [] (size_t i)         { return mV[i]; }
};
#undef VT

//==================================================================
/// Vec3
//==================================================================
template <typename T>
class Vec3
{
public:
    static constexpr size_t N = 3;

    VecN<T,N>   mV;

    typedef T value_type;

    //using VT = Vec3;
    #define VT Vec3

public:
    constexpr VT() = default;

    constexpr explicit VT( const Vec2<T> &v, const T &t )
    {
        mV[0] = v[0];
        mV[1] = v[1];
        mV[2] = t;
    }

    template <typename T2> constexpr Vec3( const Vec3<T2> &v )
    {
        mV[0] = (T)v[0];
        mV[1] = (T)v[1];
        mV[2] = (T)v[2];
    }

    template <typename T2> constexpr Vec3( const VecN<T2,N> &v )
    {
        mV = v;
    }

    constexpr VT( const T& a ) { mV = { a }; }

    constexpr VT( const T& x, const T& y, const T& z )
        : mV({x,y,z})
    {}

    static constexpr size_t size() { return N; }

    constexpr void LoadVals( const T (&p)[N] ) { mV.LoadFromMemory( p ); }

    constexpr void SetZero()      { mV.SetZero(); }
    constexpr bool IsZero() const { return mV.IsZero(); }
    constexpr bool IsValid() const { return mV.IsValid(); }

    constexpr Vec2<T> GetAsV2() const { return { mV[0], mV[1] }; }

    constexpr VT operator + (const T &rval) const { return mV + rval; }
    constexpr VT operator - (const T &rval) const { return mV - rval; }
    constexpr VT operator * (const T &rval) const { return mV * rval; }
    constexpr VT operator / (const T &rval) const { return mV / rval; }
    constexpr VT operator + (const VT &rval) const { return mV + rval.mV; }
    constexpr VT operator - (const VT &rval) const { return mV - rval.mV; }
    constexpr VT operator * (const VT &rval) const { return mV * rval.mV; }
    constexpr VT operator / (const VT &rval) const { return mV / rval.mV; }

    constexpr VT operator -() const { return -mV; }

    constexpr VT operator +=(const VT &rval) { *this = *this + rval; return *this; }
    constexpr VT operator -=(const VT &rval) { *this = *this - rval; return *this; }
    constexpr VT operator *=(const VT &rval) { *this = *this * rval; return *this; }

              friend bool operator ==(const VT &l, const VT &r){return l.mV == r.mV;}
              friend bool operator !=(const VT &l, const VT &r){return l.mV != r.mV;}
              friend bool operator  <(const VT &l, const VT &r){return l.mV  < r.mV;}

    constexpr bool IsSimilar( const VT &r, const T &eps ) const
    {
        return mV.IsSimilar( r.mV, eps );
    }

    constexpr friend VT DPow( const VT &a, const VT &b ) { return DPow( a.mV, b.mV ); }
    constexpr friend VT DSign( const VT &a )             { return DSign( a.mV ); }
    constexpr friend VT DAbs( const VT &a )              { return DAbs( a.mV ); }
    constexpr friend VT DMin( const VT &a, const VT &b ) { return DMin( a.mV, b.mV ); }
    constexpr friend VT DMax( const VT &a, const VT &b ) { return DMax( a.mV, b.mV ); }
    constexpr friend T  DDot( const VT &a, const VT &b ) { return DDot( a.mV, b.mV ); }

    constexpr friend bool DAreVecParallel( const VT &a, const VT &b, const T &eps )
    {
        auto cosa_abs = DAbs( DDot( a, b ) );
        return cosa_abs >= (1-eps) && cosa_abs <= (1+eps);
    }

    constexpr T GetDot( const VT &r ) const { return GetDot( r.mV ); }

    constexpr VT GetNormalized() const    { return mV.GetNormalized(); }
    constexpr T GetLengthSqr() const      { return mV.GetLengthSqr(); }
    constexpr T GetLength() const         { return mV.GetLength(); }

    constexpr VT GetCross( const VT &r ) const
    {
        return VT(
            mV[1] * r[2] - mV[2] * r[1],
            mV[2] * r[0] - mV[0] * r[2],
            mV[0] * r[1] - mV[1] * r[0]
        );
    }

    constexpr const T &x() const { return mV[0];  }
    constexpr const T &y() const { return mV[1];  }
    constexpr const T &z() const { return mV[2];  }
    constexpr       T &x()       { return mV[0];  }
    constexpr       T &y()       { return mV[1];  }
    constexpr       T &z()       { return mV[2];  }

    constexpr const T &operator [] (size_t i) const   { return mV[i]; }
    constexpr       T &operator [] (size_t i)         { return mV[i]; }
};
#undef VT

//==================================================================
/// Vec4
//==================================================================
template <typename T>
class Vec4
{
public:
    static constexpr size_t N = 4;

    VecN<T,N>   mV;

    typedef T value_type;

    //using VT = Vec4;
    #define VT Vec4

public:
    constexpr VT() = default;

    constexpr explicit VT( const Vec3<T> &v, const T &t )
    {
        mV[0] = v[0];
        mV[1] = v[1];
        mV[2] = v[2];
        mV[3] = t;
    }

    template <typename T2> constexpr Vec4( const Vec4<T2> &v )
    {
        mV[0] = (T)v[0];
        mV[1] = (T)v[1];
        mV[2] = (T)v[2];
        mV[3] = (T)v[3];
    }

    template <typename T2> constexpr Vec4( const VecN<T2,N> &v )
    {
        mV = v;
    }

    constexpr VT( const T& a ) { mV = { a }; }

    constexpr VT( const T& x, const T& y, const T& z, const T& w )
        : mV({x, y, z, w})
    {}

    static constexpr size_t size() { return N; }

    constexpr void LoadVals( const T (&p)[N] ) { mV.LoadFromMemory( p ); }

    constexpr void SetZero()      { mV.SetZero(); }
    constexpr bool IsZero() const { return mV.IsZero(); }
    constexpr bool IsValid() const { return mV.IsValid(); }

    constexpr Vec2<T> GetAsV2() const { return { mV[0], mV[1] }; }
    constexpr Vec3<T> GetAsV3() const { return { mV[0], mV[1], mV[2] }; }

    constexpr VT operator + (const T &rval) const { return mV + rval; }
    constexpr VT operator - (const T &rval) const { return mV - rval; }
    constexpr VT operator * (const T &rval) const { return mV * rval; }
    constexpr VT operator / (const T &rval) const { return mV / rval; }
    constexpr VT operator + (const VT &rval) const { return mV + rval.mV; }
    constexpr VT operator - (const VT &rval) const { return mV - rval.mV; }
    constexpr VT operator * (const VT &rval) const { return mV * rval.mV; }
    constexpr VT operator / (const VT &rval) const { return mV / rval.mV; }

    constexpr VT operator -() const { return -mV; }

    constexpr VT operator +=(const VT &rval) { *this = *this + rval; return *this; }
    constexpr VT operator -=(const VT &rval) { *this = *this - rval; return *this; }
    constexpr VT operator *=(const VT &rval) { *this = *this * rval; return *this; }

              friend bool operator ==(const VT &l, const VT &r){return l.mV == r.mV;}
              friend bool operator !=(const VT &l, const VT &r){return l.mV != r.mV;}
              friend bool operator  <(const VT &l, const VT &r){return l.mV  < r.mV;}

    constexpr bool IsSimilar( const VT &r, const T &eps ) const
    {
        return mV.IsSimilar( r.mV, eps );
    }

    constexpr friend VT DPow( const VT &a, const VT &b ) { return DPow( a.mV, b.mV ); }
    constexpr friend VT DSign( const VT &a )             { return DSign( a.mV ); }
    constexpr friend VT DAbs( const VT &a )              { return DAbs( a.mV ); }
    constexpr friend VT DMin( const VT &a, const VT &b ) { return DMin( a.mV, b.mV ); }
    constexpr friend VT DMax( const VT &a, const VT &b ) { return DMax( a.mV, b.mV ); }
    constexpr friend T  DDot( const VT &a, const VT &b ) { return DDot( a.mV, b.mV ); }

    constexpr friend bool DAreVecParallel( const VT &a, const VT &b, const T &eps )
    {
        auto cosa_abs = DAbs( DDot( a, b ) );
        return cosa_abs >= (1-eps) && cosa_abs <= (1+eps);
    }

    constexpr T GetDot( const VT &r ) const { return GetDot( r.mV ); }

    constexpr VT GetNormalized() const    { return mV.GetNormalized(); }
    constexpr T GetLengthSqr() const      { return mV.GetLengthSqr(); }
    constexpr T GetLength() const         { return mV.GetLength(); }

    constexpr const T &x() const { return mV[0];  }
    constexpr const T &y() const { return mV[1];  }
    constexpr const T &z() const { return mV[2];  }
    constexpr const T &w() const { return mV[3];  }
    constexpr       T &x()       { return mV[0];  }
    constexpr       T &y()       { return mV[1];  }
    constexpr       T &z()       { return mV[2];  }
    constexpr       T &w()       { return mV[3];  }

    constexpr const T &operator [] (size_t i) const   { return mV[i]; }
    constexpr       T &operator [] (size_t i)         { return mV[i]; }
};
#undef VT

//==================================================================
template<typename T> Vec2<T> operator +(const T &l, const Vec2<T> &r) {return   r + l; }
template<typename T> Vec2<T> operator -(const T &l, const Vec2<T> &r) {return -(r - l);}
template<typename T> Vec2<T> operator *(const T &l, const Vec2<T> &r) {return   r * l; }

template<typename T> Vec3<T> operator +(const T &l, const Vec3<T> &r) {return   r + l; }
template<typename T> Vec3<T> operator -(const T &l, const Vec3<T> &r) {return -(r - l);}
template<typename T> Vec3<T> operator *(const T &l, const Vec3<T> &r) {return   r * l; }

template<typename T> Vec4<T> operator +(const T &l, const Vec4<T> &r) {return   r + l; }
template<typename T> Vec4<T> operator -(const T &l, const Vec4<T> &r) {return -(r - l);}
template<typename T> Vec4<T> operator *(const T &l, const Vec4<T> &r) {return   r * l; }

template<class _TS>
VecNMask CmpMaskEQ( const Vec3<_TS> &lval, const Vec3<_TS> &rval )
{
	return	  CmpMaskEQ( lval[0], rval[0] )
			& CmpMaskEQ( lval[1], rval[1] )
			& CmpMaskEQ( lval[2], rval[2] );
}

template<class _TS>
VecNMask CmpMaskNE( const Vec3<_TS> &lval, const Vec3<_TS> &rval )
{
	return	  CmpMaskNE( lval[0], rval[0] )
			| CmpMaskNE( lval[1], rval[1] )
			| CmpMaskNE( lval[2], rval[2] );
}

template<class _TS>
VecNMask CmpMaskEQ( const Vec4<_TS> &lval, const Vec4<_TS> &rval )
{
	return	  CmpMaskEQ( lval[0], rval[0] )
			& CmpMaskEQ( lval[1], rval[1] )
			& CmpMaskEQ( lval[2], rval[2] )
			& CmpMaskEQ( lval[3], rval[3] );
}

template<class _TS>
VecNMask CmpMaskNE( const Vec4<_TS> &lval, const Vec4<_TS> &rval )
{
	return	  CmpMaskNE( lval[0], rval[0] )
			| CmpMaskNE( lval[1], rval[1] )
			| CmpMaskNE( lval[2], rval[2] )
			| CmpMaskNE( lval[3], rval[3] );
}

//===============================================================
#define _DTPL template <typename T> DMT_FINLINE

_DTPL Vec2<T> DSqrt( const Vec2<T> &a ) { return DSqrt( a.mV ); }
_DTPL Vec3<T> DSqrt( const Vec3<T> &a ) { return DSqrt( a.mV ); }
_DTPL Vec4<T> DSqrt( const Vec4<T> &a ) { return DSqrt( a.mV ); }

_DTPL T DLengthSqr( const Vec2<T>& a ) { return DDot( a, a ); }
_DTPL T DLengthSqr( const Vec3<T>& a ) { return DDot( a, a ); }
_DTPL T DLengthSqr( const Vec4<T>& a ) { return DDot( a, a ); }

_DTPL T DLength( const Vec2<T>& a ) { return DSqrt( DLengthSqr( a ) ); }
_DTPL T DLength( const Vec3<T>& a ) { return DSqrt( DLengthSqr( a ) ); }
_DTPL T DLength( const Vec4<T>& a ) { return DSqrt( DLengthSqr( a ) ); }

_DTPL Vec2<T> DNormalize( const Vec2<T>& v ) { return v * DRSqrt( DLengthSqr( v ) ); }
_DTPL Vec3<T> DNormalize( const Vec3<T>& v ) { return v * DRSqrt( DLengthSqr( v ) ); }
_DTPL Vec4<T> DNormalize( const Vec4<T>& v ) { return v * DRSqrt( DLengthSqr( v ) ); }

_DTPL bool DNormalizeCheck( Vec3<T>& io_v )
{
    constexpr auto EPS = (T)0.00001;

    auto magSqr = DLengthSqr( io_v );
    if ( magSqr <= (EPS*EPS) )
        return false;

    io_v = io_v * DRSqrt( magSqr );
    return true;
}

_DTPL Vec3<T> DNormalizeSafeZero( const Vec3<T> &v )
{
    if (auto magSqr = DLengthSqr( v ))
        return v * DRSqrt( magSqr );

    return {0,0,0};
}

_DTPL Vec3<T> DCross( const Vec3<T>& a, const Vec3<T>& b )
{
    return Vec3<T>(
            a[1] * b[2] - a[2] * b[1],
            a[2] * b[0] - a[0] * b[2],
            a[0] * b[1] - a[1] * b[0] );
}

_DTPL Vec2<T> DFloor( const Vec2<T>& v ) { return DFloor( v.mV ); }
_DTPL Vec3<T> DFloor( const Vec3<T>& v ) { return DFloor( v.mV ); }
_DTPL Vec4<T> DFloor( const Vec4<T>& v ) { return DFloor( v.mV ); }

_DTPL Vec2<T> DCeil( const Vec2<T>& v ) { return DCeil( v.mV ); }
_DTPL Vec3<T> DCeil( const Vec3<T>& v ) { return DCeil( v.mV ); }
_DTPL Vec4<T> DCeil( const Vec4<T>& v ) { return DCeil( v.mV ); }

_DTPL Vec2<T> DClamp(const Vec2<T>& v, const Vec2<T>& a, const Vec2<T>& b)
{
    return DClamp( v.mV, a.mV, b.mV );
}
_DTPL Vec3<T> DClamp(const Vec3<T>& v, const Vec3<T>& a, const Vec3<T>& b)
{
    return DClamp( v.mV, a.mV, b.mV );
}
_DTPL Vec4<T> DClamp(const Vec4<T>& v, const Vec4<T>& a, const Vec4<T>& b)
{
    return DClamp( v.mV, a.mV, b.mV );
}

#undef _DTPL


//==================================================================
/// float implementations
//==================================================================
//==================================================================
using Float2  = Vec2<float>;
using Float3  = Vec3<float>;
using Float4  = Vec4<float>;

using Double2 = Vec2<double>;
using Double3 = Vec3<double>;
using Double4 = Vec4<double>;

using Int2    = Vec2<int>;
using Int3    = Vec3<int>;
using Int4    = Vec4<int>;

template <typename _TV>
inline bool DIsUnitVec( const _TV &v )
{
    using sca_type = typename std::remove_reference<decltype(v)>::type::value_type;

    auto sum = (sca_type)0;
    for (int i=0; i != (int)v.size(); ++i)
        sum += v[i] * v[i];

    constexpr auto EPS = (sca_type)0.001;

    return sum >= (1-EPS) &&
           sum <= (1+EPS);
}

#endif

