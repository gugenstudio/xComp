//==================================================================
/// DMT_VecNMask.h
///
/// Created by Davide Pasca - 2018/02/14
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#ifndef DMT_VECNMASK_H
#define DMT_VECNMASK_H

#include <inttypes.h>
#include "DMathBase.h"

//==================================================================
#if defined(DMATH_USE_M128) // BEGIN DMATH_USE_M128
# include <xmmintrin.h>

static constexpr int DMT_SIMD_FLEN       = 4;
static constexpr int DMT_SIMD_ALIGN_SIZE = 16;   //  DMT_SIMD_FLEN * 4

// better reciprocal square root found on the Internet 8)
inline __m128 _mm_rsqrtnr_ps( __m128 x )
{
  __m128 t = _mm_rsqrt_ps(x);
  return _mm_mul_ps(  _mm_set_ps1( 0.5f ),
                      _mm_mul_ps(_mm_sub_ps(  _mm_set_ps1( 3.0f ),
                                              _mm_mul_ps(_mm_mul_ps(x,t),t) ),t)  );
}

//==================================================================
struct VecNMask
{
    union
    {
        uint32_t    m[4];
        __m128      v;
    } u;

    constexpr VecNMask()                      {}

    constexpr VecNMask( const __m128 &from )  {   u.v = from; }

    constexpr VecNMask( uint32_t a, uint32_t b, uint32_t c, uint32_t d )
    {
        u.m[0] = a;
        u.m[1] = b;
        u.m[2] = c;
        u.m[3] = d;
    }

    friend VecNMask CmpMaskEQ( const VecNMask &lval, const VecNMask &rval ) { return _mm_cmpeq_ps( lval.u.v, rval.u.v );  }
    friend VecNMask CmpMaskNE( const VecNMask &lval, const VecNMask &rval ) { return _mm_cmpneq_ps( lval.u.v, rval.u.v ); }

    friend bool operator ==( const VecNMask &lval, const VecNMask &rval )
    {
        __m128  tmp = _mm_xor_ps( lval.u.v, rval.u.v );

        const auto  folded =  ((const uint32_t *)&tmp)[0]
                            | ((const uint32_t *)&tmp)[1]
                            | ((const uint32_t *)&tmp)[2]
                            | ((const uint32_t *)&tmp)[3];
        return !folded;
    }

    friend bool operator !=( const VecNMask &lval, const VecNMask &rval )
    {
        return !(lval == rval);
    }

    VecNMask operator & ( const VecNMask &rval ) {  return _mm_and_ps( u.v, rval.u.v ); }
    VecNMask operator | ( const VecNMask &rval ) {  return _mm_or_ps( u.v, rval.u.v ); }
};

static constexpr VecNMask DMT_SIMD_ALLONE( 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF );
static constexpr VecNMask DMT_SIMD_ALLZERO( 0, 0, 0, 0 );

static constexpr VecNMask VecNMaskFull  = DMT_SIMD_ALLONE.u.v;
static constexpr VecNMask VecNMaskEmpty = DMT_SIMD_ALLZERO.u.v;

inline void VecNMask_Broadcast0Lane( VecNMask &val )
{
    val.u.v = _mm_shuffle_ps( val.u.v, val.u.v, _MM_SHUFFLE(0,0,0,0) );
}

#else // ELSE DMATH_USE_M128

# if defined(DMATH_USE_M512) // BEGIN DMATH_USE_M512

#  define USE_C_PROTOTYPE_PRIMITIVES 0

#  include "external/lrb/lrb_prototype_primitives.inl"

static constexpr int DMT_SIMD_FLEN       = 16;
static constexpr int DMT_SIMD_ALIGN_SIZE = 64;  //  DMT_SIMD_FLEN * 4

typedef __mmask VecNMask;   // only need 16 bits

inline void VecNMask_Broadcast0Lane( VecNMask &val )
{
    // expand the least significant bit
    val = (VecNMask)(((signed short)(val << (DMT_SIMD_FLEN-1))) >> (DMT_SIMD_FLEN-1));
}

# else // ELSE DMATH_USE_M512

static constexpr int DMT_SIMD_FLEN       = 1;
static constexpr int DMT_SIMD_ALIGN_SIZE = 16;  //  DMT_SIMD_FLEN * 4

typedef unsigned short  VecNMask;   // only need 4 bits (round to 1 byte)

inline void VecNMask_Broadcast0Lane( VecNMask &val )
{
    // expand the least significant bit
    val = (VecNMask)(((signed char)(val << (DMT_SIMD_FLEN-1))) >> (DMT_SIMD_FLEN-1));
}

# endif // END DMATH_USE_M512

static constexpr VecNMask VecNMaskFull  = (VecNMask)-1;
static constexpr VecNMask VecNMaskEmpty = (VecNMask)0;

inline VecNMask CmpMaskEQ( const VecNMask &lval, const VecNMask &rval ) { return lval == rval; }
inline VecNMask CmpMaskNE( const VecNMask &lval, const VecNMask &rval ) { return lval != rval; }

#endif // END DMATH_USE_M128

#endif

