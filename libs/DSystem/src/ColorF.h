//==================================================================
/// ColorF.h
///
/// Created by Davide Pasca - 2017/2/13
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#ifndef COLORF_H
#define COLORF_H

#include "DMath.h"
#include "DVector.h"

//==================================================================
class ColorF
{
    Float4  mRGBA;

    static constexpr float TOFLOAT = 1.f / 255;
public:
    constexpr ColorF() : mRGBA(1,1,1,1) {}

    constexpr ColorF( float r, float g, float b, float a=1 ) : mRGBA({r,g,b,a}) {}

    constexpr ColorF( int r, int g, int b, int a )
        : mRGBA(r * TOFLOAT, g * TOFLOAT, b * TOFLOAT, a * TOFLOAT) {}

    constexpr ColorF( int r, int g, int b )
        : mRGBA(r * TOFLOAT, g * TOFLOAT, b * TOFLOAT, 1) {}

    constexpr ColorF( const Float4 &rgba ) : mRGBA(rgba) {}
    constexpr ColorF( const Float3 &rgb ) : mRGBA(rgb[0], rgb[1], rgb[2], 1) {}

    ColorF ScaleRGB( float s ) const
    {
        return { mRGBA[0] * s,
                 mRGBA[1] * s,
                 mRGBA[2] * s,
                 mRGBA[3] };
    }

    ColorF ScaleAlpha( float s ) const
    {
        return { mRGBA[0],
                 mRGBA[1],
                 mRGBA[2],
                 mRGBA[3] * s };
    }

    friend constexpr ColorF DLerp( const ColorF &l, const ColorF &r, float t )
    {
        return { l.mRGBA[0] * (1-t) + r.mRGBA[0] * t,
                 l.mRGBA[1] * (1-t) + r.mRGBA[1] * t,
                 l.mRGBA[2] * (1-t) + r.mRGBA[2] * t,
                 l.mRGBA[3] * (1-t) + r.mRGBA[3] * t };
    }

	operator Float4 () const { return mRGBA; }

	ColorF operator + (const float& rval) const { return mRGBA + rval; }
	ColorF operator - (const float& rval) const { return mRGBA - rval; }
	ColorF operator * (const float& rval) const { return mRGBA * rval; }
	ColorF operator / (const float& rval) const { return mRGBA / rval; }
	ColorF operator + (const ColorF &rval) const { return mRGBA + rval.mRGBA; }
	ColorF operator - (const ColorF &rval) const { return mRGBA - rval.mRGBA; }
	ColorF operator * (const ColorF &rval) const { return mRGBA * rval.mRGBA; }
	ColorF operator / (const ColorF &rval) const { return mRGBA / rval.mRGBA; }

	ColorF operator -() const	{ return -mRGBA; }

	ColorF operator +=(const ColorF &rval)	{ mRGBA = mRGBA + rval; return mRGBA; }
	ColorF operator -=(const ColorF &rval)	{ mRGBA = mRGBA - rval; return mRGBA; }
	ColorF operator *=(const ColorF &rval)	{ mRGBA = mRGBA * rval; return mRGBA; }

    friend bool operator ==( const ColorF &lval, const ColorF &rval )
    {
        return lval.mRGBA == rval.mRGBA;
    }
    friend bool operator !=( const ColorF &lval, const ColorF &rval )
    {
        return lval.mRGBA != rval.mRGBA;
    }

    const float &operator [] (size_t i) const { return mRGBA[i]; }
          float &operator [] (size_t i)       { return mRGBA[i]; }
};

#endif

