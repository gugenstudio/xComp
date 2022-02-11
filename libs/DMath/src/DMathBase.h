//==================================================================
/// DMathBase.h
///
/// Created by Davide Pasca - 2009/5/6
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#ifndef DMATHBASE_H
#define DMATHBASE_H

#if defined(__linux__) || (__APPLE__)
# include <inttypes.h>
#endif

#include <type_traits>
#define _USE_MATH_DEFINES
#include <math.h>
#include <cmath>
#include <float.h>
#include <cfloat>
#include <assert.h>

#define DMT_ASSERT assert

#if defined(_MSC_VER)
# define DMT_FINLINE __forceinline
#else
# if defined(DEBUG)
#  define DMT_FINLINE inline
# else
#  define DMT_FINLINE inline __attribute__((always_inline))
# endif
#endif


//#define DMATH_USE_M128
//#define DMATH_USE_M512

// first works on Android, second in MSVC
#if defined(__i386__) || defined(_M_IX86) // assume SSE 128 bit for x86
# define DMATH_USE_M128
#endif

// gives the number of SIMD blocks necessary for _SIZE_ elements
#define	DMT_SIMD_BLOCKS(_SIZE_)			(((unsigned)(_SIZE_) + (DMT_SIMD_FLEN-1)) / DMT_SIMD_FLEN)

// gives the block index from the global index _GLOB_IDX_
#define	DMT_SIMD_BLK_IDX(_GLOB_IDX_)	(((unsigned)(_GLOB_IDX_)) / DMT_SIMD_FLEN)

// gives the index of a SIMD block element from the global index _GLOB_IDX_
#define	DMT_SIMD_SUB_IDX(_GLOB_IDX_)	(((unsigned)(_GLOB_IDX_)) & (DMT_SIMD_FLEN-1))

// gives the size padded to the SIMD length
#define	DMT_SIMD_PADSIZE(_SIZE_)		(((unsigned)(_SIZE_) + (DMT_SIMD_FLEN-1)) & ~(DMT_SIMD_FLEN-1))


template <typename T> constexpr inline T DEG2RAD(const T &x){return x * (T)(M_PI/180);}
template <typename T> constexpr inline T RAD2DEG(const T &x){return x * (T)(180/M_PI);}

//
DMT_FINLINE float  DSqrt( const float &a )   { return sqrt( a ); }
DMT_FINLINE double DSqrt( const double &a )  { return sqrt( a ); }

DMT_FINLINE float  DRSqrt( const float &a )  { return 1 / sqrt( a ); }
DMT_FINLINE double DRSqrt( const double &a ) { return 1 / sqrt( a ); }

DMT_FINLINE float  DPow( const float &a, const float &b )  { return pow( a, b ); }
DMT_FINLINE double DPow( const double &a, const double &b ){ return pow( a, b ); }

//==================================================================
#if defined(__GNUC__)
DMT_FINLINE float   DFloor( const float &a )  { return __builtin_floorf( a ); }
DMT_FINLINE double  DFloor( const double &a ) { return __builtin_floor( a ); }
DMT_FINLINE float   DCeil( const float &a )   { return __builtin_ceilf( a ); }
DMT_FINLINE double  DCeil( const double &a )  { return __builtin_ceil( a ); }
#else
DMT_FINLINE float   DFloor( const float &a )  { return floorf( a ); }
DMT_FINLINE double  DFloor( const double &a ) { return floor( a ); }
DMT_FINLINE float   DCeil( const float &a )   { return ceilf( a ); }
DMT_FINLINE double  DCeil( const double &a )  { return ceil( a ); }
#endif

//==================================================================
template <typename T> constexpr inline T    DSign( const T &a )
{
    const auto zero = T(0);
    return a < zero ? T(-1) : (a > zero ? T(1) : zero);
}
//==================================================================
template <typename T> constexpr inline T    DAbs( const T &a )
{
    return a < T(0) ? -a : a;
}
//==================================================================
template <class T>	constexpr inline T		DMin( const T &a, const T &b )
{
	if ( a < b )	return a;	else
					return b;
}
//==================================================================
template <class T>	constexpr inline T        DMin3( const T &a, const T &b, const T &c )
{
    if ( a < b ) return a < c ? a : c; else
                 return b < c ? b : c;
}
//==================================================================
template <class T>	constexpr inline T		DMax( const T &a, const T &b )
{
	if ( a > b )	return a;	else
					return b;
}
//==================================================================
template <class T>	constexpr inline T        DMax3( const T &a, const T &b, const T &c )
{
    if ( a > b ) return a > c ? a : c; else
                 return b > c ? b : c;
}
//==================================================================
template <class T>	constexpr inline T		DClamp( const T& a, const T& x0, const T& x1 )
{
	if ( a < x0 )	return x0;	else
	if ( a > x1 )	return x1;	else
					return a;
}

//
template <typename T>
constexpr inline T DSafeSqrt( const T &a )
{
    DMT_ASSERT( a >= 0 );
    return sqrt( DMax( a, (T)0 ) );
}

template <typename T>
constexpr inline T DSafeRSqrt( const T &a )
{
    DMT_ASSERT( a != 0 );
    if ( a == 0 )
        return 1;

    return 1 / DSafeSqrt( a );
}

template <typename T>
constexpr inline T DSafePow( const T &a,  const T &b )
{
    DMT_ASSERT( a >= 0 ); // NOTE: would be ok with integral 'b' though
    return pow( DMax( a, (T)0 ), b );
}

//==================================================================
template <typename TA, typename TT>
constexpr inline auto DMix( const TA &a, const TA &b, const TT &t )
{
    return a + (b - a) * t;
}
template <typename TA, typename TT>
constexpr inline auto DLerp( const TA &a, const TA &b, const TT &t )
{
    return a + (b - a) * t;
}
template <typename TA, typename TT>
constexpr inline auto DInvLerp( const TA &left, const TA &right, const TT &x )
{
	return DClamp( (TA)((x - left) / (right - left)), TA(0), TA(1) );
}
template <typename TA, typename TT>
constexpr inline auto DNLerp( const TA &a, const TA &b,  const TT &t )
{
    return DNormalize( a + (b - a) * t );
}
template <typename TA, typename TT>
constexpr inline auto DSmoothStep( const TA &a, const TA &b, const TT &t )
{
    auto x = DInvLerp( a, b, t );
    return x*x*(TA(3) - TA(2)*x);
}
template <typename TA, typename TT>
constexpr inline auto DTimeLerp( const TA &a, const TA &b, const TT &f, const TT &dt )
{
    auto ft = DSafePow( f, dt );
    return a + (b - a) * ft;
}

#define D_ASSERT_INTEGRAL \
static_assert( !std::is_integral<_T>::value, "Not supported for integral values" )

//==================================================================
template <typename _T> constexpr inline _T DCos( const _T &a ){
    D_ASSERT_INTEGRAL;
    return cos( a );
}

//==================================================================
template <typename _T> constexpr inline _T DSin( const _T &a ){
    D_ASSERT_INTEGRAL;
    return sin( a );
}

//==================================================================
template <typename _T> constexpr inline _T DASin( const _T &a ){
    D_ASSERT_INTEGRAL;
    return asin( a );
}

template <typename _T> constexpr inline _T DSafeASin( const _T &a )
{
    D_ASSERT_INTEGRAL;
    const _T EPS = (_T)0.01;
    DMT_ASSERT( a < (1 + EPS) && a > -(1 + EPS) ); (void)EPS;
    return asin( DClamp( a, (_T)-1, (_T)1 ) );
}

template <typename _T> constexpr inline _T DSafeACos( const _T &a )
{
    D_ASSERT_INTEGRAL;
    const _T EPS = (_T)0.01;
    DMT_ASSERT( a < (1 + EPS) && a > -(1 + EPS) ); (void)EPS;
    return (_T)acos( DClamp( a, (_T)-1, (_T)1 ) );
}

#undef D_ASSERT_INTEGRAL

#define FM_E        ((float)M_E       )
#define FM_LOG2E    ((float)M_LOG2E   )
#define FM_LOG10E   ((float)M_LOG10E  )
#define FM_LN2      ((float)M_LN2     )
#define FM_LN10     ((float)M_LN10    )
#define FM_PI       ((float)M_PI      )
#define FM_2PI      ((float)(M_PI * 2))
#define FM_PI_2     ((float)M_PI_2    )
#define FM_PI_4     ((float)M_PI_4    )
#define FM_1_PI     ((float)M_1_PI    )
#define FM_2_PI     ((float)M_2_PI    )
#define FM_2_SQRTPI ((float)M_2_SQRTPI)
#define FM_SQRT2    ((float)M_SQRT2   )
#define FM_SQRT1_2  ((float)M_SQRT1_2 )

typedef unsigned char		DU8;
typedef unsigned int		DU32;

#if defined(__linux__) || (__APPLE__) || defined(ANDROID) || defined(NACL)
typedef uint64_t			DU64;
#else
typedef unsigned __int64	DU64;
#endif

#endif

