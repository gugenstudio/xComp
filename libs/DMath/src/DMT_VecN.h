//==================================================================
/// DMT_VecN.h
///
/// Created by Davide Pasca - 2009/5/9
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#ifndef DMT_VECN_H
#define DMT_VECN_H

#include <stdint.h>
#include <cstddef>
#include <array>
#include <numeric>
#include <cmath>
#include <functional>
#include <algorithm>
#include "DMathBase.h"
#include "DMT_VecNMask.h"

//==================================================================
#define FOR_I_N	for (int i=0; i<(int)N; ++i)

template<typename T, size_t N>
class VecN
{
public:
    std::array<T,N> v {};

	//==================================================================
    constexpr VecN() {}
    constexpr VecN( const T& a_ )                 { v.fill( a_ ); }
    constexpr VecN( const std::array<T,N> &src_ ) : v(src_) {}

    constexpr VecN( const VecN& a, const std::function<T (const T&)> &fun )
    {
        FOR_I_N v[i] = fun( a[i] );
    }

    constexpr static size_t size() { return N; }

    constexpr void LoadFromMemory( const T (&src)[N] )
    {
        std::copy( std::begin(src), std::end(src), v.begin() );
    }

    constexpr void SetZero()
    {
        v.fill( T(0) );
    }

    constexpr bool IsZero() const
    {
        return std::all_of( v.begin(), v.end(), [](auto x){ return !x; } );
    }

    bool IsSimilar( const VecN &other, const T &eps ) const
    {
        FOR_I_N if ( DAbs( v[i] - other[i] ) > eps ) return false;

        return true;
    }

    constexpr bool IsValid() const
    {
        if constexpr ( std::is_floating_point<T>::value )
        {
            for (const auto &x : v)
                if ( x != x || std::isnan( x ) )
                    return false;
        }
        else
        if constexpr (    std::is_object<T>::value
                       && !std::is_same<
                                decltype( std::declval<T>().IsValid() ),
                                void>::value )
        {
            for (const auto &x : v)
                if ( !x.IsValid() )
                    return false;
        }

        return true;
    }

	VecN operator +(const T& rval) const   {VecN t; FOR_I_N t[i]=v[i]+rval; return t;}
	VecN operator -(const T& rval) const   {VecN t; FOR_I_N t[i]=v[i]-rval; return t;}
	VecN operator *(const T& rval) const   {VecN t; FOR_I_N t[i]=v[i]*rval; return t;}
	VecN operator /(const T& rval) const   {VecN t; FOR_I_N t[i]=v[i]/rval; return t;}
	VecN operator +(const VecN &rval) const{VecN t; FOR_I_N t[i]=v[i]+rval[i]; return t;}
	VecN operator -(const VecN &rval) const{VecN t; FOR_I_N t[i]=v[i]-rval[i]; return t;}
	VecN operator *(const VecN &rval) const{VecN t; FOR_I_N t[i]=v[i]*rval[i]; return t;}
	VecN operator /(const VecN &rval) const{VecN t; FOR_I_N t[i]=v[i]/rval[i]; return t;}

	VecN operator -() const	{ VecN tmp; FOR_I_N tmp.v[i] = -v[i]; return tmp; }

	VecN operator +=(const VecN &rval)	{ *this = *this + rval; return *this; }

	          friend bool operator ==( const VecN &l, const VecN &r ) { return l.v == r.v; }
	          friend bool operator !=( const VecN &l, const VecN &r ) { return l.v != r.v; }
              friend bool operator  <( const VecN &l, const VecN &r ) { return l.v  < r.v; }

    constexpr T AddReduce() const { return std::accumulate( v.begin(), v.end(), 0 ); }

    constexpr T GetDot( const VecN& rval ) const
    {
        T acc = 0; FOR_I_N acc += v[i] * rval[i]; return acc;
    }

	constexpr VecN GetNormalized() const { return *this * DRSqrt( GetDot( *this ) ); }
	constexpr T GetLengthSqr() const     { return GetDot( *this ); }
	constexpr T GetLength() const        { return DSqrt( GetDot( *this ) ); }

    constexpr const T &operator [] (size_t i) const   { return v[i]; }
    constexpr       T &operator [] (size_t i)         { return v[i]; }
};

#define _DTPL template<typename T,size_t N> DMT_FINLINE constexpr
#define _DTYP VecN<T,N>

_DTPL _DTYP DSqrt( const _DTYP &a ) {_DTYP t; FOR_I_N t[i] = DSqrt(a[i] ); return t; }
_DTPL _DTYP DRSqrt(const _DTYP &a ) {_DTYP t; FOR_I_N t[i] = DRSqrt(a[i] ); return t; }

_DTPL _DTYP DPow( const _DTYP &a, const _DTYP &b )
{
    _DTYP t;
    FOR_I_N t[i] = DPow(a[i], b[i] );
    return t;
}

_DTPL _DTYP DSign( const _DTYP &a ) {_DTYP t; FOR_I_N t[i] = DSign(a[i] ); return t; }
_DTPL _DTYP DAbs(  const _DTYP &a ) {_DTYP t; FOR_I_N t[i] = DAbs(a[i] ); return t; }

_DTPL _DTYP DMin( const _DTYP &a, const _DTYP &b )
{
    _DTYP t; FOR_I_N t[i] = DMin(a[i], b[i] ); return t;
}

_DTPL _DTYP DMax( const _DTYP &a, const _DTYP &b )
{
    _DTYP t; FOR_I_N t[i] = DMax(a[i], b[i] ); return t;
}

_DTPL _DTYP DSin(  const _DTYP &a ) {_DTYP t; FOR_I_N t[i] = DSin(a[i] ); return t; }
_DTPL _DTYP DCos(  const _DTYP &a ) {_DTYP t; FOR_I_N t[i] = DCos(a[i] ); return t; }

_DTPL _DTYP DFloor(const _DTYP &a) { return {a, [](auto x){ return DFloor( x ); }}; }
_DTPL _DTYP DCeil(const _DTYP &a) { return {a, [](auto x){ return DCeil( x ); }}; }

_DTPL auto DClamp( const _DTYP &a, const _DTYP &l, const _DTYP &r )
{
    _DTYP t; FOR_I_N t[i] = DClamp( a[i], l[i], r[i] ); return t;
}

_DTPL _DTYP operator + (const T &lval, const _DTYP &rval) { return   rval + lval; }
_DTPL _DTYP operator - (const T &lval, const _DTYP &rval) { return -(rval - lval); }
_DTPL _DTYP operator * (const T &lval, const _DTYP &rval) { return   rval * lval; }

_DTPL T DDot( const _DTYP& a, const _DTYP& b )
{
    T acc = 0;
    FOR_I_N acc += a[i] * b[i];
    return acc;
}

#undef _DTPL
#undef _DTYP

template <typename TV, size_t N, typename TP>
DMT_FINLINE VecN<TV,N> DPow(  const VecN<TV,N> &a, const TP &p )
{
    VecN<TV,N> tmp;
    for (int i=0; i < (int)N; ++i)
        tmp[i] = pow( a[i], (TV)p );

    return tmp;
}

/*
template <class T, size_t N> VecNMask CmpMaskLT( const VecN<T,N> &lval, const VecN<T,N> &rval ) { DMT_ASSERT( 0 ); return VecNMaskEmpty; }
template <class T, size_t N> VecNMask CmpMaskGT( const VecN<T,N> &lval, const VecN<T,N> &rval ) { DMT_ASSERT( 0 ); return VecNMaskEmpty; }
template <class T, size_t N> VecNMask CmpMaskEQ( const VecN<T,N> &lval, const VecN<T,N> &rval ) { DMT_ASSERT( 0 ); return VecNMaskEmpty; }
template <class T, size_t N> VecNMask CmpMaskNE( const VecN<T,N> &lval, const VecN<T,N> &rval ) { DMT_ASSERT( 0 ); return VecNMaskEmpty; }
template <class T, size_t N> VecNMask CmpMaskLE( const VecN<T,N> &lval, const VecN<T,N> &rval ) { DMT_ASSERT( 0 ); return VecNMaskEmpty; }
template <class T, size_t N> VecNMask CmpMaskGE( const VecN<T,N> &lval, const VecN<T,N> &rval ) { DMT_ASSERT( 0 ); return VecNMaskEmpty; }
*/

#undef FOR_I_N

//==================================================================
#define FOR_I_N	for (int i=0; i<DMT_SIMD_FLEN; ++i)

#if defined(DMATH_USE_M128)
template<>
class VecN<float,DMT_SIMD_FLEN>
{
//==================================================================
/// 128 bit 4-way float SIMD
//==================================================================
public:
	__m128  v {};

	//==================================================================
	constexpr VecN()						{}
	constexpr VecN( const __m128 &v_ )	{ v = v_; }
	constexpr VecN( const VecN &v_ )		{ v = v_.v; }
	constexpr VecN( const float& a_ )		{ v = _mm_set_ps1( a_ );	}

	void SetZero()				{ v = _mm_setzero_ps();		}

    size_t size() const { return DMT_SIMD_FLEN; }

	// TODO: add specialized AddReduce()
	float AddReduce() const		{ return (*this)[0] + (*this)[1] + (*this)[2] + (*this)[3]; }

	VecN operator + (const float& rval) const	{ return _mm_add_ps( v, _mm_set_ps1( rval )	); }
	VecN operator - (const float& rval) const	{ return _mm_sub_ps( v, _mm_set_ps1( rval )	); }
	VecN operator * (const float& rval) const	{ return _mm_mul_ps( v, _mm_set_ps1( rval )	); }
	VecN operator / (const float& rval) const	{ return _mm_div_ps( v, _mm_set_ps1( rval )	); }
	VecN operator + (const VecN &rval) const	{ return _mm_add_ps( v, rval.v	); }
	VecN operator - (const VecN &rval) const	{ return _mm_sub_ps( v, rval.v	); }
	VecN operator * (const VecN &rval) const	{ return _mm_mul_ps( v, rval.v	); }
	VecN operator / (const VecN &rval) const	{ return _mm_div_ps( v, rval.v	); }

	VecN operator -() const						{ return _mm_sub_ps( _mm_setzero_ps(), v ); }

	VecN operator +=(const VecN &rval)			{ *this = *this + rval; return *this; }

	friend VecNMask CmpMaskLT( const VecN &lval, const VecN &rval ) { return _mm_cmplt_ps( lval.v, rval.v );  }
	friend VecNMask CmpMaskGT( const VecN &lval, const VecN &rval ) { return _mm_cmpgt_ps( lval.v, rval.v );  }
	friend VecNMask CmpMaskEQ( const VecN &lval, const VecN &rval ) { return _mm_cmpeq_ps( lval.v, rval.v );  }
	friend VecNMask CmpMaskNE( const VecN &lval, const VecN &rval ) { return _mm_cmpneq_ps( lval.v, rval.v ); }
	friend VecNMask CmpMaskLE( const VecN &lval, const VecN &rval ) { return _mm_cmple_ps( lval.v, rval.v );  }
	friend VecNMask CmpMaskGE( const VecN &lval, const VecN &rval ) { return _mm_cmpge_ps( lval.v, rval.v );  }

	friend bool operator ==( const VecN &lval, const VecN &rval ) { return VecNMaskEmpty == CmpMaskNE( lval, rval ); }
	friend bool operator !=( const VecN &lval, const VecN &rval ) { return VecNMaskFull  != CmpMaskEQ( lval, rval ); }

//#if defined(_MSC_VER)
//	const float &operator [] (size_t i) const	{ return v.m128_f32[i]; }
//		  float &operator [] (size_t i)			{ return v.m128_f32[i]; }
//#else
	const float &operator [] (size_t i) const	{ return ((const float *)&v)[i]; }
		  float &operator [] (size_t i)			{ return ((float *)&v)[i]; }
//#endif
};

#elif defined(DMATH_USE_M512)
template<>
class VecN<float,DMT_SIMD_FLEN>
{
//==================================================================
/// 512 bit 16-way float SIMD
//==================================================================
public:
	__m512  v {};

	//==================================================================
	constexpr VecN()						{}
	constexpr VecN( const __m512 &v_ )	{ v = v_; }
	constexpr VecN( const VecN &v_ )		{ v = v_.v; }
	constexpr VecN( const float& a_ )		{ v = _mm512_set_1to16_ps( a_ );	}
	constexpr VecN( const float *p_ )		{ v = _mm512_expandd( (void *)p_, _MM_FULLUPC_NONE, _MM_HINT_NONE ); }	// unaligned

	void SetZero()				{ v = _mm512_setzero_ps();		}

	float AddReduce() const		{ return _mm512_reduce_add_ps( v ); }

	VecN operator + (const float& rval) const	{ return _mm512_add_ps( v, _mm512_set_1to16_ps( rval )	); }
	VecN operator - (const float& rval) const	{ return _mm512_sub_ps( v, _mm512_set_1to16_ps( rval )	); }
	VecN operator * (const float& rval) const	{ return _mm512_mul_ps( v, _mm512_set_1to16_ps( rval )	); }
	VecN operator / (const float& rval) const	{ return _mm512_div_ps( v, _mm512_set_1to16_ps( rval )	); }
	VecN operator + (const VecN &rval) const	{ return _mm512_add_ps( v, rval.v	); }
	VecN operator - (const VecN &rval) const	{ return _mm512_sub_ps( v, rval.v	); }
	VecN operator * (const VecN &rval) const	{ return _mm512_mul_ps( v, rval.v	); }
	VecN operator / (const VecN &rval) const	{ return _mm512_div_ps( v, rval.v	); }

	VecN operator -() const						{ return _mm512_sub_ps( _mm512_setzero_ps(), v ); }

	VecN operator +=(const VecN &rval)			{ *this = *this + rval; return *this; }

	// TODO: verify that it works !
	friend VecNMask CmpMaskLT( const VecN &lval, const VecN &rval ) { return _mm512_cmplt_ps( lval.v, rval.v );  }
	friend VecNMask CmpMaskGT( const VecN &lval, const VecN &rval ) { return ~_mm512_cmple_ps( lval.v, rval.v );  }
	friend VecNMask CmpMaskEQ( const VecN &lval, const VecN &rval ) { return _mm512_cmpeq_ps( lval.v, rval.v );  }
	friend VecNMask CmpMaskNE( const VecN &lval, const VecN &rval ) { return _mm512_cmpneq_ps( lval.v, rval.v ); }
	friend VecNMask CmpMaskLE( const VecN &lval, const VecN &rval ) { return _mm512_cmple_ps( lval.v, rval.v );  }
	friend VecNMask CmpMaskGE( const VecN &lval, const VecN &rval ) { return ~_mm512_cmplt_ps( lval.v, rval.v );  }

	friend bool operator ==( const VecN &lval, const VecN &rval ) { return VecNMaskEmpty == CmpMaskNE( lval, rval ); }
	friend bool operator !=( const VecN &lval, const VecN &rval ) { return VecNMaskFull  != CmpMaskEQ( lval, rval ); }

	const float &operator [] (size_t i) const	{ return v.v[i]; }
		  float &operator [] (size_t i)			{ return v.v[i]; }
};
#endif

// specialization of functions
#define _DTPL template<> DMT_FINLINE
#define _DTYP VecN<float,DMT_SIMD_FLEN>

#if defined(DMATH_USE_M128)
//==================================================================
//==================================================================
//==================================================================
_DTPL _DTYP	DSqrt( const _DTYP &a )	{ return _mm_sqrt_ps( a.v );	}
_DTPL _DTYP	DRSqrt( const _DTYP &a )	{ return _mm_rsqrtnr_ps( a.v );	}
//_DTPL _DTYP	DRSqrt( const _DTYP &a )	{ return _mm_rsqrt_ps( a.v );	}

// TODO: get a proper _mm_pow_ps !!!
//_DTPL _DTYP	DPow( const _DTYP &a, const _DTYP &b ){ return _mm_pow_ps( a.v, b.v );	}
_DTPL _DTYP	DPow( const _DTYP &a, const _DTYP &b )
{
	_DTYP tmp;
	for (size_t i=0; i < DMT_SIMD_FLEN; ++i)
		tmp[i] = powf( a[i], b[i] );

	return tmp;
}

//==================================================================
_DTPL _DTYP	DSign( const _DTYP &a )
{
	const __m128	zero		= _mm_setzero_ps();
	const __m128	selectPos	= _mm_cmpgt_ps( a.v, zero );	// > 0
	const __m128	selectNeg	= _mm_cmplt_ps( a.v, zero );	// < 0

	__m128	res =		  _mm_and_ps( selectPos, _mm_set_ps1(  1.0f ) );
	res = _mm_or_ps( res, _mm_and_ps( selectNeg, _mm_set_ps1( -1.0f ) ) );

	return res;
}

_DTPL _DTYP	DAbs( const _DTYP &a )
{
	static const uint32_t notSignBitMask = ~0x80000000;
	return _mm_and_ps( a.v, _mm_set_ps1( *(float *)&notSignBitMask ) );
}

_DTPL _DTYP	DMin( const _DTYP &a, const _DTYP &b )	{	return _mm_min_ps( a.v, b.v );	}
_DTPL _DTYP	DMax( const _DTYP &a, const _DTYP &b )	{	return _mm_max_ps( a.v, b.v );	}

#elif defined(DMATH_USE_M512)
//==================================================================
//==================================================================
//==================================================================
_DTPL _DTYP	DSqrt( const _DTYP &a )		{ return _mm512_sqrt_ps( a.v );	}
_DTPL _DTYP	DRSqrt( const _DTYP &a )	{ return _mm512_rsqrt_ps( a.v );	}

_DTPL _DTYP	DPow( const _DTYP &a, const _DTYP &b ){ return _mm512_pow_ps( a.v, b.v );	}

//==================================================================
_DTPL _DTYP	DSign( const _DTYP &a )
{
	const __m512	zero		= _mm512_setzero_ps();
	const __mmask	selectPos	= _mm512_cmpnle_ps( a.v, zero );	// >
	const __mmask	selectNeg	= _mm512_cmplt_ps( a.v, zero );		// <

	__m512	res = _mm512_mask_movd( zero, selectPos, _mm512_set_1to16_ps(  1.0f ) );
			res = _mm512_mask_movd( res , selectNeg, _mm512_set_1to16_ps( -1.0f ) );

	return res;
}

_DTPL _DTYP	DAbs( const _DTYP &a )
{
	return _mm512_maxabs_ps( a.v, a.v );
}

_DTPL _DTYP	DMin( const _DTYP &a, const _DTYP &b )	{	return _mm512_min_ps( a.v, b.v );	}
_DTPL _DTYP	DMax( const _DTYP &a, const _DTYP &b )	{	return _mm512_max_ps( a.v, b.v );	}
#else
//==================================================================
//==================================================================
//==================================================================
#endif

#undef _DTPL
#undef _DTYP

#undef FOR_I_N

#endif

