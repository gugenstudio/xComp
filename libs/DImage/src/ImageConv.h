//==================================================================
/// ImageConv.h
///
/// Created by Davide Pasca - 2011/1/10
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#ifndef IMAGECONV_H
#define IMAGECONV_H

#include "DMath.h"
#include "Image.h"

//==================================================================
namespace ImageConv
{

//==================================================================
bool IsNormalMapFName( const DStr &fname );

//==================================================================
void Blit(
        const image &srcImg,
        int sx, int sy,
        image &desImg,
        int dx, int dy,
        int dw, int dh );

void BlitConvertFormat(
        const image &srcImg,
        int sx, int sy,
        image &desImg,
        int dx, int dy,
        int dw, int dh,
        std::function<void (u_int col[])> fn ); // image::MAX_CHANS

void BlitQuarter(
        const image &srcImg,
        int sx, int sy,
        image &desImg,
        int dx, int dy,
        int dw, int dh );

void BlitStretch(
        const image &srcImg,
        int sx, int sy,
        int sw, int sh,
        image &desImg,
        int dx, int dy,
        int dw, int dh );

inline void BlitStretch( const image &srcImg, image &desImg )
{
    BlitStretch( srcImg, 0, 0, (int)srcImg.mW, (int)srcImg.mH,
                 desImg, 0, 0, (int)desImg.mW, (int)desImg.mH );
}

void BlitConvertToAlpha(
		const image &srcImg,
		int sx, int sy,
		image &desImg,
		int dx, int dy,
		int dw, int dh );

void FlipY( image &img );

//==================================================================
template <typename _TS, typename _TD>
inline void BlitProcess(
        const image &srcImg,
        int sx, int sy,
        image &desImg,
        int dx, int dy,
        int dw, int dh,
        std::function<void (const _TS *pSrc, _TD *pDes)> fn )
{
    const ptrdiff_t sRowInc = srcImg.mBytesPerRow;
          ptrdiff_t dRowInc = desImg.mBytesPerRow;
    const ptrdiff_t sPixInc = srcImg.mBytesPerPixel;
          ptrdiff_t dPixInc = desImg.mBytesPerPixel;

    const U8 *pSrcLine = srcImg.GetPixelPtr( sx, sy );
          U8 *pDesLine = desImg.GetPixelPtr( dx, dy );

    if ( dw < 0 )
    {
        dw = -dw;
        pDesLine += (dw-1) * dPixInc; // to rightmost pixel
        dPixInc = -dPixInc; // invert pix increment
    }

    if ( dh < 0 )
    {
        dh = -dh;
        pDesLine += (dh-1) * dRowInc; // to bottommost row
        dRowInc = -dRowInc; // invert row increment
    }

    auto sWdBytes = (ptrdiff_t)dw * sPixInc;

	for (int yi=0; yi < dh; ++yi)
	{
        auto *pSrc = pSrcLine;
        auto *pDes = pDesLine;
        auto *pSrcEnd = pSrc + sWdBytes;
        while ( pSrc < pSrcEnd )
        {
            fn( (const _TS *)pSrc, (_TD *)pDes );
            pSrc += sPixInc;
            pDes += dPixInc;
        }
        pSrcLine += sRowInc;
        pDesLine += dRowInc;
    }
}

//==================================================================
template <typename _TS, typename _TD>
DFORCEINLINE void BlitProcess(
        const image &srcImg,
        image &desImg,
        std::function<void (const _TS *pSrc, _TD *pDes)> fn )
{
    BlitProcess( srcImg, 0, 0, desImg, 0, 0, (int)desImg.mW, (int)desImg.mH, fn );
}

//==================================================================
template <typename _T>
inline void RectProcess(
        const image &img,
        std::function<void (const _T *pPix)> fn,
        u_int x=0,
        u_int y=0,
        u_int w=(u_int)-1,
        u_int h=(u_int)-1 )
{
    if ( w == (u_int)-1 ) w = img.mW;
    if ( h == (u_int)-1 ) h = img.mH;

    const size_t bprow = img.mBytesPerRow;
    const u_int  bppix = img.mBytesPerPixel;

    const U8 *pLine = img.GetPixelPtr( x, y );

    auto wdBytes = (ptrdiff_t)w * bppix;

	for (u_int yi=0; yi < h; ++yi)
	{
        auto *pPix    = pLine;
        auto *pPixEnd = pPix + wdBytes;
        while ( pPix < pPixEnd )
        {
            fn( (const _T *)pPix );
            pPix += bppix;
        }
        pLine += bprow;
    }
}

//==================================================================
template <typename _T>
inline void RectProcess(
        image &img,
        std::function<void (_T *pPix)> fn,
        u_int x=0,
        u_int y=0,
        u_int w=(u_int)-1,
        u_int h=(u_int)-1 )
{
    if ( w == (u_int)-1 ) w = img.mW;
    if ( h == (u_int)-1 ) h = img.mH;

    const size_t bprow = img.mBytesPerRow;
    const u_int  bppix = img.mBytesPerPixel;

    U8 *pLine = img.GetPixelPtr( x, y );

    auto wdBytes = (ptrdiff_t)w * bppix;

	for (u_int yi=0; yi < h; ++yi)
	{
        auto *pPix    = pLine;
        auto *pPixEnd = pPix + wdBytes;
        while ( pPix < pPixEnd )
        {
            fn( (_T *)pPix );
            pPix += bppix;
        }
        pLine += bprow;
    }
}

//==================================================================
template <typename _T>
inline void RectProcessUV(
        image &img,
        std::function<void (_T *pPix, float u, float v)> fn,
        u_int x=0,
        u_int y=0,
        u_int w=(u_int)-1,
        u_int h=(u_int)-1 )
{
    if ( w == (u_int)-1 ) w = img.mW;
    if ( h == (u_int)-1 ) h = img.mH;

    const float oow = 1.f / (float)(w ? w : 1);
    const float ooh = 1.f / (float)(h ? h : 1);

    const size_t bprow = img.mBytesPerRow;
    const u_int  bppix = img.mBytesPerPixel;

    U8 *pLine = img.GetPixelPtr( x, y );

    auto wdBytes = (ptrdiff_t)w * bppix;

    float v = (0 + 0.5f) * ooh;
	for (u_int yi=0; yi < h; ++yi)
	{
        float u = (0 + 0.5f) * oow;

        auto *pPix    = pLine;
        auto *pPixEnd = pPix + wdBytes;
        while ( pPix < pPixEnd )
        {
            fn( (_T *)pPix, u, v );
            pPix += bppix;

            u += oow;
        }
        pLine += bprow;
        v += ooh;
    }
}

//==================================================================
uptr<image> Create8BitImage( const image &srcImg );

//==================================================================
DFORCEINLINE void Conv565To888( U16 srcCol, U8 out_rgb[3] )
{
	out_rgb[0] = ((srcCol >> 11) << 3) & 0xff;
	out_rgb[1] = ((srcCol >>  5) << 2) & 0xff;
	out_rgb[2] = ((srcCol >>  0) << 3) & 0xff;
}

//==================================================================
DFORCEINLINE U16 Conv888To565( U8 r, U8 g, U8 b )
{
	const int SHIFT_L = ((sizeof(int)-1)*8-1);
	const int SHIFT_R = (sizeof(int)*8-1);

	int rr = ((int)r + 4); rr = ((rr | ((rr << SHIFT_L) >> SHIFT_R)) << 8) & 0xF800;
	int gg = ((int)g + 2); gg = ((gg | ((gg << SHIFT_L) >> SHIFT_R)) << 3) & 0x7E0;
	int bb = ((int)b + 4); bb = ((bb | ((bb << SHIFT_L) >> SHIFT_R)) >> 3) & 0x1F;

	return (U16)( rr | gg | bb );
}
//==================================================================
DFORCEINLINE U16 Conv888To565( const U8 *pRGB )
{
	return Conv888To565( pRGB[0], pRGB[1], pRGB[2] );
}

//==================================================================
DFORCEINLINE U16 Conv888To565_Fast( U8 r, U8 g, U8 b )
{
	return
		((r >> 3) << 11) |
		((g >> 2) <<  5) |
		((b >> 3) <<  0);
}
//==================================================================
DFORCEINLINE U16 Conv888To565_Fast( const U8 *pRGB )
{
	return Conv888To565_Fast( pRGB[0], pRGB[1], pRGB[2] );
}

//==================================================================
DFORCEINLINE U16 Conv8888To4444_Fast( U8 r, U8 g, U8 b, U8 a )
{
	return
        ((r >> 4) << 12) |
        ((g >> 4) <<  8) |
        ((b >> 4) <<  4) |
        ((a >> 4) <<  0);
}

//==================================================================
DFORCEINLINE U16 Conv8888To5551_Fast( U8 r, U8 g, U8 b, U8 a )
{
	return
        ((r >> 3) << 11) |
        ((g >> 3) <<  6) |
        ((b >> 3) <<  1) |
        ((a >= 128 ? 1 : 0) << 0);
}

//==================================================================
DFORCEINLINE U16 ConvF3To565( const Float3 &rgb )
{
	int r = DClamp( (int)(rgb[0] * 31), 0, 31 );
	int g = DClamp( (int)(rgb[1] * 63), 0, 63 );
	int b = DClamp( (int)(rgb[2] * 31), 0, 31 );

	return (U16)((r << 11) | (g << 5) | b);
}

//==================================================================
DFORCEINLINE Float3 Conv565ToF3( U16 srcCol )
{
	return Float3(
		(((srcCol >> 11) << 3) & 0xff) * (1.f/255),
		(((srcCol >>  5) << 2) & 0xff) * (1.f/255),
		(((srcCol >>  0) << 3) & 0xff) * (1.f/255) );
}

//==================================================================
DFORCEINLINE U8 ClampU8( int val )
{
    if ( (U32)val > 255 )
        val = ~(val >> (sizeof(int)*8-1)) & 255;

    return (U8)val;
}

//==================================================================
DFORCEINLINE void ConvF3To888( const Float3 &rgb, U8 dest[3] )
{
    dest[0] = ClampU8( (int)(rgb[0] * 255) );
    dest[1] = ClampU8( (int)(rgb[1] * 255) );
    dest[2] = ClampU8( (int)(rgb[2] * 255) );
}

//==================================================================
DFORCEINLINE Vec3<U8> Get888FromF3( const Float3 &rgb )
{
    return { ClampU8( (int)(rgb[0] * 255) ),
             ClampU8( (int)(rgb[1] * 255) ),
             ClampU8( (int)(rgb[2] * 255) ) };
}

//==================================================================
DFORCEINLINE Float3 GetF3From888( const Vec3<U8> &rgb )
{
    return { (float)rgb[0] * (1.f/255),
             (float)rgb[1] * (1.f/255),
             (float)rgb[2] * (1.f/255) };
}

//==================================================================
// from: http://chilliant.blogspot.jp/2012/08/srgb-approximations-for-hlsl.html
// NOTE: this should match the copy in FVsrgb in FatVar.h !
DFORCEINLINE Float3 LinearFromSRGB( const Float3 &srgb )
{
    auto srgb2 = srgb * srgb;
    auto srgb3 = srgb2 * srgb;

    auto lin = 0.012522878f * srgb +
               0.682171111f * srgb2 +
               0.305306011f * srgb3;

    return lin;
}
//==================================================================
DFORCEINLINE Vec3<U8> LinearFromSRGB( const Vec3<U8> &srgb )
{
    return Get888FromF3( LinearFromSRGB( GetF3From888( srgb ) ) );
}
//==================================================================
DFORCEINLINE U8 LinearFromSRGB_1( U8 x )
{
    float xf = (float)x * (1.f/255);
    auto xf2 = xf  * xf;
    auto xf3 = xf2 * xf;

    auto lin = (0.012522878f*255) * xf +
               (0.682171111f*255) * xf2 +
               (0.305306011f*255) * xf3;

    return ClampU8( (int)lin );
}

//==================================================================
DFORCEINLINE Float3 SRGBFromLinear( const Float3 &lin )
{
    Float3 S1 = DSqrt( lin );
    Float3 S2 = DSqrt( S1 );
    Float3 S3 = DSqrt( S2 );

    Float3 sRGB = 0.662002687f * S1 +
                  0.684122060f * S2 -
                  0.323583601f * S3 -
                  0.0225411470f * lin;

    return sRGB;
}
//==================================================================
DFORCEINLINE Vec3<U8> SRGBFromLinear( const Vec3<U8> &lin )
{
    return Get888FromF3( SRGBFromLinear( GetF3From888( lin ) ) );
}

//==================================================================
}

#endif
