//==================================================================
/// DHalf.h
///
/// Created by Davide Pasca - 2015/1/20
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#ifndef DHALF_H
#define DHALF_H

#include "DMathBase.h"

#include <cstdint>

//==================================================================
namespace HALF
{

uint16_t FloatToHalf_convert( uint32_t i );

//==================================================================
union UIF
{
    uint32_t    ui;
    float       f;
};

//==================================================================
DMT_FINLINE bool HalfIsNaN( uint16_t y )
{
    int e = (y >> 10) & 0x0000001f;
    int m =  y        & 0x000003ff;

    return e == 31 && m != 0;
}

//==================================================================
DMT_FINLINE float HalfToFloat( uint16_t y )
{
    int s = (y >> 15) & 0x00000001;
    int e = (y >> 10) & 0x0000001f;
    int m =  y        & 0x000003ff;

    UIF out;

    if (e == 0)
    {
        if (m == 0)
        {
            // Plus or minus zero
            out.ui = s << 31;
            return out.f;
        }
        else
        {
            // Denormalized number -- renormalize it
            while (!(m & 0x00000400))
            {
                m <<= 1;
                e -=  1;
            }

            e += 1;
            m &= ~0x00000400;
        }
    }
    else
    if (e == 31)
    {
        // m == 0 -> Positive or negative infinity
        // m != 0 -> Nan -- preserve sign and significant bits
        out.ui = (s << 31) | 0x7f800000 | (m << 13);
        return out.f;
    }

    // Normalized number
    e = e + (127 - 15);
    m = m << 13;

    // Assemble s, e and m.
    out.ui = (s << 31) | (e << 23) | m;

    return out.f;
}

//==================================================================
DMT_FINLINE uint16_t FloatToHalf( float f )
{
    UIF x;
    x.f = f;

    if (f == 0)
    {
        // Common special case - zero.
        // Preserve the zero's sign bit.
        return (uint16_t)(x.ui >> 16);
    }
    else
    {
        // We extract the combined sign and exponent, e, from our
        // floating-point number, f.  Then we convert e to the sign
        // and exponent of the half number via a table lookup.
        //
        // For the most common case, where a normalized half is produced,
        // the table lookup returns a non-zero value; in this case, all
        // we have to do is round f's significand to 10 bits and combine
        // the result with e.
        //
        // For all other cases (overflow, zeroes, denormalized numbers
        // resulting from underflow, infinities and NANs), the table
        // lookup returns zero, and we call a longer, non-inline function
        // to do the float-to-half conversion.
        /*
        int e = _eLut[ (x.ui >> 23) & 0x000001ff ];

        if (e)
        {
            //
            // Simple case - round the significand, m, to 10
            // bits and combine it with the sign and exponent.
            //

            int m = x.ui & 0x007fffff;
            return e + ((m + 0x00000fff + ((m >> 13) & 1)) >> 13);
        }
        else
        */
        {
            // Difficult case - call a function.

            return FloatToHalf_convert( x.ui );
        }
    }
}

}

#endif

