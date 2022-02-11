//==================================================================
/// ImageConv.cpp
///
/// Created by Davide Pasca - 2011/1/10
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#include "stdafx.h"
#include "DHalf.h"
#include "DUT_Str.h"
#include "ImageConv.h"

//==================================================================
namespace ImageConv
{

//==================================================================
bool IsNormalMapFName( const DStr &fname )
{
    const char *pFNameNoExt;

    DStr fnameNoExt; // buffer

    // skip possible path name.. solves ambiguity in case
    // that the path name includes a dot (!)
    size_t fnamePos = 0;
    {
        auto pos1 = fname.find_last_of( '/' );
        if ( pos1 != DStr::npos )
            fnamePos = pos1;

        auto pos2 = fname.find_last_of( '\\' );
        if ( pos2 != DStr::npos )
            fnamePos = std::max( fnamePos, pos2 );

        fnamePos += 1;
    }

    // strip the extensions (starting from the first one)
    auto dotPos = fname.find_first_of( '.', fnamePos );
    if ( dotPos != DStr::npos )
    {
        fnameNoExt = fname.substr( fnamePos, dotPos - fnamePos );
        pFNameNoExt = fnameNoExt.c_str();
    }
    else
    {
        pFNameNoExt = fname.c_str();
    }

    return
        DUT::StrEndsWithI( pFNameNoExt, "_n" ) ||
        DUT::StrEndsWithI( pFNameNoExt, "_nor" ) ||
        DUT::StrEndsWithI( pFNameNoExt, "_norm" ) ||
        DUT::StrEndsWithI( pFNameNoExt, "_normal" ) ||
        DUT::StrEndsWithI( pFNameNoExt, "_normals" );
}

//==================================================================
DFORCEINLINE bool checkBlitBounds(
				const image &srcImg,
				int sx, int sy,
				image &desImg,
				int dx, int dy,
				int dw, int dh )
{
	return
            ( sx >= 0 && sy >= 0 &&
              dx >= 0 && dy >= 0 &&
              dw >= 0 && dh >= 0 )
        && ( (sx+dw) <= (int)srcImg.mW && (sy+dh) <= (int)srcImg.mH )
        && ( (dx+dw) <= (int)desImg.mW && (dy+dh) <= (int)desImg.mH );
}

//==================================================================
void Blit(
				const image &srcImg,
				int sx, int sy,
				image &desImg,
				int dx, int dy,
				int dw, int dh )
{
    bool flipX = (dw < 0);
    bool flipY = (dh < 0);
    if ( dw < 0 ) dw = -dw;
    if ( dh < 0 ) dh = -dh;

    DASSERT( checkBlitBounds( srcImg, sx, sy, desImg, dx, dy, dw, dh ) );
    DASSERT( desImg.mDepth <= srcImg.mDepth );

    const auto srcRowInc = (ptrdiff_t)srcImg.mBytesPerRow;
          auto desRowInc = (ptrdiff_t)desImg.mBytesPerRow;

    int dx2 = dx;
    if ( flipX )
        dx2 += dw - 1;

    int dy2 = dy;
    if ( flipY )
    {
        desRowInc = -desRowInc;
        dy2 += dh - 1;
    }

    const U8 *pSrcLine = srcImg.GetPixelPtr( sx, sy );
          U8 *pDesLine = desImg.GetPixelPtr( dx2, dy2 );

    const int sbpp = srcImg.mBytesPerPixel;
    const int dbpp = desImg.mBytesPerPixel;
    const size_t useBpr = (size_t)(desImg.mBytesPerPixel * dw);

    for (int yi=0; yi < dh; ++yi)
    {
        if ( flipX )
        {
            const U8 *pS = pSrcLine;
                  U8 *pD = pDesLine;

            for (int xi=0; xi < dw; ++xi)
            {
                for (int i=0; i < dbpp; ++i)
                    pD[i] = pS[i];

                pS += sbpp;
                pD -= dbpp;
            }
        }
        else
        {
            if ( sbpp != dbpp )
            {
                const U8 *pS = pSrcLine;
                      U8 *pD = pDesLine;

                for (int xi=0; xi < dw; ++xi)
                {
                    for (int i=0; i < dbpp; ++i)
                        pD[i] = pS[i];

                    pS += sbpp;
                    pD += dbpp;
                }
            }
            else
                memcpy( pDesLine, pSrcLine, useBpr );
        }

        pSrcLine += srcRowInc;
        pDesLine += desRowInc;
    }
}

//==================================================================
void BlitConvertFormat(
        const image &srcImg,
        int sx, int sy,
        image &desImg,
        int dx, int dy,
        int dw, int dh,
        std::function<void (u_int col[])> fn )
{
    if (srcImg.mDepth == desImg.mDepth &&
        srcImg.mChans == desImg.mChans &&
        fn == nullptr )
    {
        Blit( srcImg, sx, sy, desImg, dx, dy, dw, dh );
        return;
    }

    // making it really explicit that it's constant stuff..
    // hoping that the compiler will move conditionals outside
    // the loop
    const u_int sDepth = srcImg.mDepth;
    const u_int dDepth = desImg.mDepth;
    const u_int sChans = srcImg.mChans;
    const u_int dChans = desImg.mChans;
    const size_t sBPRow = srcImg.mBytesPerRow;
    const size_t dBPRow = desImg.mBytesPerRow;
    const u_int  sBPPix = srcImg.mBytesPerPixel;
    const u_int  dBPPix = desImg.mBytesPerPixel;

    const U8 *pSrcLine = srcImg.GetPixelPtr( sx, sy );
          U8 *pDesLine = desImg.GetPixelPtr( dx, dy );

	for (int yi=0; yi < dh; ++yi)
	{
        auto *pSrc = pSrcLine;
        auto *pDes = pDesLine;
        auto *pSrcEnd = pSrc + sBPRow;
        while ( pSrc < pSrcEnd )
        {
            u_int tmp[image::MAX_CHANS];
            image::GetPixelUnpacked_s( pSrc, sDepth, sChans, sBPPix, srcImg.mFlags, tmp );
            fn( tmp );
            image::SetPixelUnpacked_s( pDes, dDepth, dChans, dBPPix, desImg.mFlags, tmp );
            pSrc += sBPPix;
            pDes += dBPPix;
        }
        pSrcLine += sBPRow;
        pDesLine += dBPRow;
    }
}

//==================================================================
void BlitQuarter(
				const image &srcImg,
				int sx, int sy,
				image &desImg,
				int dx, int dy,
				int dw, int dh )
{
    DASSERT( checkBlitBounds( srcImg, sx, sy, desImg, dx, dy, dw, dh ) );
	DASSERT( desImg.mDepth == srcImg.mDepth );

	DASSERT(
			desImg.mDepth == 8 ||
			desImg.mDepth == 16 ||
			desImg.mDepth == 24 ||
			desImg.mDepth == 32);

	const	U8	*pSrcLine = srcImg.GetPixelPtr( sx, sy );
			U8	*pDesLine = desImg.GetPixelPtr( dx, dy );

	bool isHardTransp = !!(srcImg.mFlags & image::FLG_HARD_TRANSP);

	for (int yi=0; yi < dh; ++yi)
	{
		const	U8	*pSrcLine1 = pSrcLine;
		const	U8	*pSrcLine2 = pSrcLine + srcImg.mBytesPerRow;

		if ( desImg.mDepth == 8 )
		{
			for (int xi=0; xi < dw; ++xi)
			{
				const	U8	*pSrc00 = &pSrcLine1[ xi*2+0 ];
				const	U8	*pSrc01 = &pSrcLine1[ xi*2+1 ];
				const	U8	*pSrc10 = &pSrcLine2[ xi*2+0 ];
				const	U8	*pSrc11 = &pSrcLine2[ xi*2+1 ];

				pDesLine[xi] = (U8)(((int)pSrc00[0] + (int)pSrc01[0] + (int)pSrc10[0] + (int)pSrc11[0]) >> 2);
			}
		}
		else
		if ( desImg.mDepth == 16 )
		{
			for (int xi=0; xi < dw; ++xi)
			{
                U32 chans[4][image::MAX_CHANS];
                srcImg.GetPixelUnpacked( &pSrcLine1[ xi*4+0 ], chans[0] );
                srcImg.GetPixelUnpacked( &pSrcLine1[ xi*4+2 ], chans[1] );
                srcImg.GetPixelUnpacked( &pSrcLine2[ xi*4+0 ], chans[2] );
                srcImg.GetPixelUnpacked( &pSrcLine2[ xi*4+2 ], chans[3] );

                for (size_t i=0; i != srcImg.mChans; ++i)
                    chans[0][i] = (chans[0][i] + chans[1][i] + chans[2][i] + chans[3][i]) >> 2;

                desImg.SetPixelUnpacked( &pDesLine[ xi*2 ], chans[0] );
			}
		}
		else
		if ( desImg.mDepth == 24 )
		{
			for (int xi=0; xi < dw; ++xi)
			{
				const	U8	*pSrc00 = &pSrcLine1[(xi*2+0)*3];
				const	U8	*pSrc01 = &pSrcLine1[(xi*2+1)*3];
				const	U8	*pSrc10 = &pSrcLine2[(xi*2+0)*3];
				const	U8	*pSrc11 = &pSrcLine2[(xi*2+1)*3];

				pDesLine[xi*3+0] = (U8)(((int)pSrc00[0] + (int)pSrc01[0] + (int)pSrc10[0] + (int)pSrc11[0]) >> 2);
				pDesLine[xi*3+1] = (U8)(((int)pSrc00[1] + (int)pSrc01[1] + (int)pSrc10[1] + (int)pSrc11[1]) >> 2);
				pDesLine[xi*3+2] = (U8)(((int)pSrc00[2] + (int)pSrc01[2] + (int)pSrc10[2] + (int)pSrc11[2]) >> 2);
			}
		}
		else
		if ( desImg.mDepth == 32 )
		{
			for (int xi=0; xi < dw; ++xi)
			{
				const	U8	*pSrc00 = &pSrcLine1[(xi*2+0)*4];
				const	U8	*pSrc01 = &pSrcLine1[(xi*2+1)*4];
				const	U8	*pSrc10 = &pSrcLine2[(xi*2+0)*4];
				const	U8	*pSrc11 = &pSrcLine2[(xi*2+1)*4];

				if ( isHardTransp )
				{
					int alphaSum = (int)pSrc00[3] + (int)pSrc01[3] + (int)pSrc10[3] + (int)pSrc11[3];

					if ( alphaSum == 0 )
					{
						// all transparent
						pDesLine[xi*4+3] = 0;
					}
					else
					if ( alphaSum == 255*4 )
					{
						// all opaque
						pDesLine[xi*4+3] = 255;
					}
					else
					{
						// selective case (ignore what's at alpha 0)
						int rSum = 0;
						int gSum = 0;
						int bSum = 0;
						int opaCnt = 0;

						if ( pSrc00[3] )
						{
							rSum += (int)pSrc00[0];
							gSum += (int)pSrc00[1];
							bSum += (int)pSrc00[2];
							opaCnt += 1;
						}
						if ( pSrc01[3] )
						{
							rSum += (int)pSrc01[0];
							gSum += (int)pSrc01[1];
							bSum += (int)pSrc01[2];
							opaCnt += 1;
						}
						if ( pSrc10[3] )
						{
							rSum += (int)pSrc10[0];
							gSum += (int)pSrc10[1];
							bSum += (int)pSrc10[2];
							opaCnt += 1;
						}
						if ( pSrc11[3] )
						{
							rSum += (int)pSrc11[0];
							gSum += (int)pSrc11[1];
							bSum += (int)pSrc11[2];
							opaCnt += 1;
						}

						// opaCnt shouldn't be 0
						switch ( opaCnt )
						{
						case 1: break;
						case 2: rSum >>= 1; gSum >>= 1; bSum >>= 1; break;
						case 3: rSum /= 3; gSum /= 3; bSum /= 3; break;
						case 4: rSum >>= 2; gSum >>= 2; bSum >>= 2; break;
						default:
							DASSERT( 0 );
							break;
						}

						pDesLine[xi*4+0] = (U8)rSum;
						pDesLine[xi*4+1] = (U8)gSum;
						pDesLine[xi*4+2] = (U8)bSum;

						pDesLine[xi*4+3] = (U8)(alphaSum >> 2);

						// don't fall through the plain color avg calculation
						continue;
					}
				}
				else
				{
					pDesLine[xi*4+3] = (U8)(((int)pSrc00[3] + (int)pSrc01[3] + (int)pSrc10[3] + (int)pSrc11[3]) >> 2);
				}

				pDesLine[xi*4+0] = (U8)(((int)pSrc00[0] + (int)pSrc01[0] + (int)pSrc10[0] + (int)pSrc11[0]) >> 2);
				pDesLine[xi*4+1] = (U8)(((int)pSrc00[1] + (int)pSrc01[1] + (int)pSrc10[1] + (int)pSrc11[1]) >> 2);
				pDesLine[xi*4+2] = (U8)(((int)pSrc00[2] + (int)pSrc01[2] + (int)pSrc10[2] + (int)pSrc11[2]) >> 2);
			}
		}

		pSrcLine += srcImg.mBytesPerRow * 2;
		pDesLine += desImg.mBytesPerRow;
	}
}

//==================================================================
DFORCEINLINE void blitStretchBase(
        const image &srcImg,
        int sx, int sy,
        int sw, int sh,
        image &desImg,
        int dx, int dy,
        int dw, int dh,
        DFun<void (int sxi0, int syi0, int sxi1, int syi1, U8 *pDesPix)> subSampleRectFn )
{
    DASSERT( srcImg.mChans == desImg.mChans );

    bool flipX = false;
    bool flipY = false;
    if ( dw < 0 )
    {
        dw = -dw;
        flipX = true;
    }
    if ( dh < 0 )
    {
        dh = -dh;
        flipY = true;
    }

    float d_x = (float)sw / dw;
    float d_y = (float)sh / dh;

    int dx_end = dx + dw;
    int dy_end = dy + dh;

    for (int dyi=dy; dyi != dy_end; ++dyi)
    {
        int syi0 = sy + (int)floor((float)(dyi+0) * d_y);
        int syi1 = sy + (int)ceil( (float)(dyi+1) * d_y);

        int dyi2 = (flipY ? dh-1-dyi : dyi);

        for (int dxi=dx; dxi != dx_end; ++dxi)
        {
            int dxi2 = (flipX ? dw-1-dxi : dxi);

            int sxi0 = sx + (int)floor((float)(dxi+0) * d_x);
            int sxi1 = sx + (int)ceil( (float)(dxi+1) * d_x);

            U8 *pDesPix = desImg.GetPixelPtr( dxi2, dyi2 );
            subSampleRectFn( sxi0, syi0, sxi1, syi1, pDesPix );
        }
    }
}

//==================================================================
DFORCEINLINE static void getPixelFloat(
        float des[image::MAX_CHANS],
        const image &img,
        const U8 *pSrc )
{
    if ( img.IsFloat32() )
    {
        const float *p = (const float *)pSrc;
        for (u_int i=0; i != img.mChans; ++i)
            des[i] = p[i];
    }
    else
    {
        DASSERT( img.IsFloat16() );

        const U16 *p = (const U16 *)pSrc;
        for (u_int i=0; i != img.mChans; ++i)
            des[i] = HALF::HalfToFloat( p[i] );
    }
}

//==================================================================
DFORCEINLINE static void setPixelFloat(
        const float src[image::MAX_CHANS],
        const image &img,
        U8 *pDes )
{
    if ( img.IsFloat32() )
    {
        float *p = (float *)pDes;
        for (u_int i=0; i != img.mChans; ++i)
            p[i] = src[i];
    }
    else
    {
        DASSERT( img.IsFloat16() );

        U16 *p = (U16 *)pDes;
        for (u_int i=0; i != img.mChans; ++i)
            p[i] = HALF::FloatToHalf( src[i] );
    }
}

//==================================================================
void BlitStretch(
        const image &srcImg,
        int sx, int sy,
        int sw, int sh,
        image &desImg,
        int dx, int dy,
        int dw, int dh )
{
    // allow blitting between float formats (16 and 32)
    DASSERT(
        (srcImg.IsFloat16() || srcImg.IsFloat32()) ==
        (desImg.IsFloat16() || desImg.IsFloat32()) );

    DASSERT( srcImg.mChans == desImg.mChans );

    size_t chN = srcImg.mChans;

    if ( srcImg.IsFloat16() || srcImg.IsFloat32() )
    {
        auto subSampleRectFn = [&](int sxi0, int syi0, int sxi1, int syi1, U8 *pDesPix)
        {
            float acc[image::MAX_CHANS];
            for (size_t i=0; i != chN; ++i)
                acc[i] = 0;

            for (int syif=syi0; syif < syi1; ++syif)
            {
                for (int sxif=sxi0; sxif < sxi1; ++sxif)
                {
                    float chans[image::MAX_CHANS];
                    getPixelFloat( chans, srcImg, srcImg.GetPixelPtr(sxif,syif) );

                    for (size_t i=0; i != chN; ++i)
                        acc[i] += chans[i];
                }
            }

            float weight = 1.f / ((float)(syi1 - syi0) * (float)(sxi1 - sxi0));

            for (size_t i=0; i != chN; ++i)
                acc[i] *= weight;

            setPixelFloat( acc, desImg, pDesPix );
        };

        blitStretchBase(
            srcImg, sx, sy, sw, sh,
            desImg, dx, dy, dw, dh,
            subSampleRectFn );
    }
    else
    {
        auto subSampleRectFn = [&](int sxi0, int syi0, int sxi1, int syi1, U8 *pDesPix)
        {
            U32 acc[image::MAX_CHANS];
            for (size_t i=0; i != chN; ++i)
                acc[i] = 0;

            for (int syif=syi0; syif < syi1; ++syif)
            {
                for (int sxif=sxi0; sxif < sxi1; ++sxif)
                {
                    U32 chans[image::MAX_CHANS];
                    srcImg.GetPixelUnpacked( srcImg.GetPixelPtr(sxif,syif), chans );
                    for (size_t i=0; i != chN; ++i)
                        acc[i] += chans[i];
                }
            }

            float weight = 1.f / ((float)(syi1 - syi0) * (float)(sxi1 - sxi0));

            for (size_t i=0; i != chN; ++i)
                acc[i] = (U32)((float)acc[i] * weight);

            desImg.SetPixelUnpacked( pDesPix, acc );
        };

        blitStretchBase(
            srcImg, sx, sy, sw, sh,
            desImg, dx, dy, dw, dh,
            subSampleRectFn );
    }
}

//==================================================================
void BlitConvertToAlpha(
					const image &srcImg,
					int sx, int sy,
					image &desImg,
					int dx, int dy,
					int dw, int dh )
{
	DASSERT( sx >= 0 && sy >= 0 && dx >= 0 && dy >= 0 && dw >= 0 && dh >= 0 );
	DASSERT( (sx+dw) <= (int)srcImg.mW && (sy+dh) <= (int)srcImg.mH );
	DASSERT( (dx+dw) <= (int)desImg.mW && (dy+dh) <= (int)desImg.mH );
	DASSERT( desImg.mDepth == 8 );

	for (int yi=0; yi < dh; ++yi)
	{
		const	U8	*pSrcLine = srcImg.GetPixelPtr( sx, sy + yi );
				U8	*pDesLine = desImg.GetPixelPtr( dx, dy + yi );

		for (int xi=0; xi < dw; ++xi)
		{
			pDesLine[0] = pSrcLine[0];

			pDesLine += desImg.mBytesPerPixel;
			pSrcLine += srcImg.mBytesPerPixel;
		}
	}
}

//==================================================================
template <u_int _SIZ>
void swapLine( U8 *p1, U8 *p2, u_int len)
{
    for (u_int x=0; x != len; ++x)
        for (u_int i=0; i != _SIZ; ++i)
            std::swap( p1[i], p2[i] );
}

//==================================================================
void FlipX( image &img )
{
    u_int bpp = img.mBytesPerPixel;

    auto w2 = img.mW / 2;

    for (u_int y=0; y < img.mH; ++y)
    {
        auto *p1 = img.GetPixelPtr(0, y);
        auto *p2 = img.GetPixelPtr(img.mW-1, y);

        switch ( bpp )
        {
        case  1: swapLine< 1>( p1, p2, w2 ); break;
        case  2: swapLine< 2>( p1, p2, w2 ); break;
        case  3: swapLine< 3>( p1, p2, w2 ); break;
        case  4: swapLine< 4>( p1, p2, w2 ); break;
        case 12: swapLine<12>( p1, p2, w2 ); break;
        default:
            for (u_int x=0; x < w2; ++x)
            {
                for (u_int i=0; i != bpp; ++i)
                    std::swap( p1[i], p2[i] );
            }
            break;
        }
    }
}

//==================================================================
void FlipY( image &img )
{
    size_t len = img.mW * img.mBytesPerPixel;

    DVec<U8> buff( len );

    auto h2 = img.mH / 2;

    for (u_int y=0; y < h2; ++y)
    {
        auto *p1 = img.GetPixelPtr(0, y);
        auto *p2 = img.GetPixelPtr(0, img.mH-1 - y);
        auto *pt = buff.data();

        memcpy( pt, p1, len );
        memcpy( p1, p2, len );
        memcpy( p2, pt, len );
    }
}

//==================================================================
uptr<image> Create8BitImage( const image &srcImg )
{
    uptr<image> oImg;

    int bitsPerChan = (int)srcImg.mDepth / (int)srcImg.mChans;

    image::Params par;
    par.chans  = srcImg.mChans;
    par.depth  = par.chans * 8;
    par.width  = srcImg.mW;
    par.height = srcImg.mH;
    oImg = std::make_unique<image>( par );

    // simple case
    if ( bitsPerChan == 8 )
    {
        Blit( srcImg, 0, 0, *oImg, 0, 0, (int)srcImg.mW, (int)srcImg.mH );
        return oImg;
    }

    DASSERT( bitsPerChan == 16 || bitsPerChan == 32 );

    int chansN = (int)srcImg.mChans;
    bool isFloat = srcImg.IsFloat16() || srcImg.IsFloat32();

    BlitConvertFormat(
        srcImg,
        0, 0,
        *oImg,
        0, 0,
        (int)srcImg.mW,
        (int)srcImg.mH,
        [=]( u_int col[] )
        {
            if ( bitsPerChan == 16 )
            {
                if ( isFloat )
                {
                    for (int i=0; i < chansN; ++i)
                    {
                        // convert from half-float to 8-bit unsigned
                        auto fltVal = HALF::HalfToFloat( (U16)col[i] );
                        col[i] = (u_int)DClamp( (int)(fltVal * 255.f), 0, 255 );
                    }
                }
                else
                {
                    // get the hight 8 bits
                    for (int i=0; i < chansN; ++i)
                        col[i] = col[i] >> 8;
                }
            }
            else
            if ( bitsPerChan == 32 )
            {
                if ( isFloat )
                {
                    for (int i=0; i < chansN; ++i)
                    {
                        // convert from float to 8-bit unsigned
                        auto fltVal = *(const float *)&col[i];
                        col[i] = (u_int)DClamp( (int)(fltVal * 255.f), 0, 255 );
                    }
                }
                else
                {
                    // get the hight 8 bits
                    for (int i=0; i < chansN; ++i)
                        col[i] = col[i] >> 24;
                }
            }
        });

    return oImg;
}

//==================================================================
}
