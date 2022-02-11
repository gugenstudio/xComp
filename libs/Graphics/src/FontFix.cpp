//==================================================================
/// FontFix.cpp
///
/// Created by Davide Pasca - 2018/3/17
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#include "ImageConv.h"
//#include "ImageUploader.h"
#include "FontFix.h"

#define USE_RGBA_TEXTURE

//==================================================================
static void blitConvertRedToAlpha( const image &srcImg,
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
        const U8 *pSrcLine = srcImg.GetPixelPtr( sx, sy + yi );
              U8 *pDesLine = desImg.GetPixelPtr( dx, dy + yi );

        for (int xi=0; xi < dw; ++xi)
        {
            pDesLine[0] = pSrcLine[0];

            pDesLine += desImg.mBytesPerPixel;
            pSrcLine += srcImg.mBytesPerPixel;
        }
    }
}

//==================================================================
static void blitConvertRedToRGBA( const image &srcImg,
                                      int sx, int sy,
                                      image &desImg,
                                      int dx, int dy,
                                      int dw, int dh )
{
    DASSERT( sx >= 0 && sy >= 0 && dx >= 0 && dy >= 0 && dw >= 0 && dh >= 0 );
    DASSERT( (sx+dw) <= (int)srcImg.mW && (sy+dh) <= (int)srcImg.mH );
    DASSERT( (dx+dw) <= (int)desImg.mW && (dy+dh) <= (int)desImg.mH );
    DASSERT( desImg.mDepth == 32 );

    for (int yi=0; yi < dh; ++yi)
    {
        const U8 *pSrcLine = srcImg.GetPixelPtr( sx, sy + yi );
              U8 *pDesLine = desImg.GetPixelPtr( dx, dy + yi );

        for (int xi=0; xi < dw; ++xi)
        {
            const auto srcRed = pSrcLine[0];
            pDesLine[0] = srcRed;
            pDesLine[1] = srcRed;
            pDesLine[2] = srcRed;
            pDesLine[3] = srcRed;

            pDesLine += desImg.mBytesPerPixel;
            pSrcLine += srcImg.mBytesPerPixel;
        }
    }
}

//==================================================================
void FontFix::ChangeBaseScale( const Double2 &baseScale )
{
    mUseBaseScale = baseScale;

    mBaseDrawChFixSiz[0] = (double)mSrcChSiz[0] * mUseBaseScale[0];
    mBaseDrawChFixSiz[1] = (double)mSrcChSiz[1] * mUseBaseScale[1];

    for (size_t i=0; i < 256; ++i)
        mBaseDrawChWd[i] = mBaseDrawChFixSiz[0];
}

//==================================================================
FontFix::FontFix( const Params &par ) :
    mPar(par)
{
    int fw = (int)mPar.pImg->mW / 32;
    int fh = (int)mPar.pImg->mH / 3;

    mSrcChSiz = Int2( fw, fh );

    ChangeBaseScale( mPar.baseScale );

    auto baseFlags = mPar.pImg->mFlags;

    baseFlags |= image::FLG_IS_ALPHA;
    baseFlags |= image::FLG_USE_MIPMAPS;

    baseFlags &= ~image::FLG_HAS_GAMMA22;

    const int paddedCharW = fw+2;
    const int paddedCharH = fh+2;

    //
    auto makeImage = [&]( auto &img, c_auto flags )
    {
        image::Params imgpar;
        imgpar.width   = (u_int)paddedCharW * 32;
        imgpar.height  = (u_int)paddedCharH * 3;
#ifdef USE_RGBA_TEXTURE
        imgpar.depth   = 32;
        imgpar.chans   = 4;
#else
        imgpar.depth   = 8;
        imgpar.chans   = 1;
#endif
        imgpar.flags   = flags;
        img.Setup( imgpar );

        for (size_t i=0; i < 256; ++i)
            mSrcCoords[i] = Int2( 0, 0 );

        size_t charIdx = 32;
        for (int row=0; row < 3; ++row)
        {
            for (int col=0; col < 32; ++col)
            {
                c_auto sx1 = col * fw;
                c_auto sy1 = row * fh;

                c_auto dx = col * paddedCharW + 1;
                c_auto dy = row * paddedCharH + 1;

#ifdef USE_RGBA_TEXTURE
                blitConvertRedToRGBA( *mPar.pImg, sx1, sy1, img, dx, dy, fw, fh );
#else
                blitConvertRedToAlpha( *mPar.pImg, sx1, sy1, img, dx, dy, fw, fh );
#endif

                mSrcCoords[charIdx][0] = dx;
                mSrcCoords[charIdx][1] = dy;
                ++charIdx;
            }
        }

        Graphics::UploadImageTexture( img );
    };

    makeImage( mImg[0], baseFlags );
    makeImage( mImg[1], baseFlags | image::FLG_USE_BILINEAR );
}

//==================================================================
void FontFix::DrawText(
                Graphics &g,
                const double shape[4],
                FontDrawParams &par ) const
{
    if ( par.alignX == 0  ) par.px = shape[0] + shape[2] * 0.5f; else
    if ( par.alignX == -1 ) par.px = shape[0]; else
    if ( par.alignX ==  1 ) par.px = shape[0] + shape[2]; else
    {
        DASSERT( 0 );
    }

    if ( par.alignY == 0  ) par.py = shape[1] + shape[3] * 0.5f; else
    if ( par.alignY == -1 ) par.py = shape[1]; else
    if ( par.alignY ==  1 ) par.py = shape[1] + shape[3]; else
    {
        DASSERT( 0 );
    }

    DrawText( g, par );
}

//==================================================================
class FontDrawTask
{
    Graphics    &mG;
    const image &mImage;

public:
    //==================================================================
    FontDrawTask( Graphics &g, const image &img, double posZ )
        : mG(g)
        , mImage(img)
    {
        mG.SetTextureFromImage( mImage );
    }

    void DrawImage(
                        const Double4 &s,
                        double dx,
                        double dy,
                        double dw,
                        double dh,
                        const ColorF &topCol,
                        const ColorF &botCol )
    {
        c_auto s0 = (float)((s[0] + s[2]*0) * mImage.mXtoS);
        c_auto t0 = (float)((s[1] + s[3]*1) * mImage.mYtoT);
        c_auto s1 = (float)((s[0] + s[2]*1) * mImage.mXtoS);
        c_auto t1 = (float)((s[1] + s[3]*0) * mImage.mYtoT);
        c_auto txcBox = Float4( s0, t0, s1, t1 );

        mG.DrawRectTex( {dx,dy}, {dw,dh}, topCol, txcBox );
    }
};

//==================================================================
void FontFix::DrawText( Graphics &g, const FontDrawParams &par ) const
{
    const char *pStr = par.pTxt;

    auto x = par.px;
    auto y = par.py;

    auto len = (int)strlen( pStr );
    if NOT( len )
    {
        if ( par.pOutShape )
        {
            par.pOutShape[0] = x;
            par.pOutShape[1] = y;
            par.pOutShape[2] = 0;
            par.pOutShape[3] = 0;
        }
        return;
    }

    c_auto strSiz = CalcSize( par );

    if ( par.alignX == 0 )  x -= strSiz[0]*.5f; else
    if ( par.alignX > 0 )   x -= strSiz[0];

    if ( par.alignY == 0 )  y -= strSiz[1]*.5f; else
    if ( par.alignY > 0 )   y -= strSiz[1];

    c_auto sw = (double)mSrcChSiz[0] - 1;
    c_auto sh = (double)mSrcChSiz[1];

    c_auto dw = par.scaleX * sw * mUseBaseScale[0];
    c_auto dh = par.scaleY * mBaseDrawChFixSiz[1];

    if ( par.pOutShape )
    {
        par.pOutShape[0] = x;
        par.pOutShape[1] = y;
        par.pOutShape[2] = strSiz[0];
        par.pOutShape[3] = strSiz[1];
    }

    auto x1 = x;
    auto y1 = y - mPar.descender * (mUseBaseScale[1] * par.scaleY);

    Double4 src( 0, 0, sw, sh );

    FontDrawTask fdtask( g, mImg[ (par.flags & FONT_DFLG_NOBILINEAR) ? 0 : 1], par.pz );

    for (int i=0; i < len; ++i)
    {
        int ch = pStr[ i ];
        if ( ch > 32 && ch <= 32+32*3 )
        {
            src[0] = (double)mSrcCoords[ch][0];
            src[1] = (double)mSrcCoords[ch][1];

            if ( par.flags & FONT_DFLG_OUTLINE )
            {
                for (int yy=-1; yy <= 1; ++yy)
                    for (int xx=-1; xx <= 1; ++xx)
                    {
                        c_auto xx1 = x1 + (double)xx * dw / mBaseDrawChFixSiz[0] * 0.55f;
                        c_auto yy1 = y1 + (double)yy * dh / mBaseDrawChFixSiz[1] * 0.55f;
                        fdtask.DrawImage( src, xx1, yy1, dw, dh, par.bgcol, par.bgcol );
                    }
            }

            fdtask.DrawImage( src, x1, y1, dw, dh, par.col1, par.col2 );
        }
        else
        if ( ch == '\n' )
        {
            y1 += dh;
            x1 = x;
            continue;
        }

        double chW = (mBaseDrawChWd[ch] + par.spacing) * par.scaleX;

        x1 += chW;
    }
}

//==================================================================
Double2 FontFix::CalcSize(
            const FontDrawParams &par,
            const char *pTxt ) const
{
    const double chH = mBaseDrawChFixSiz[1] * par.scaleY;

    if NOT( pTxt )
        pTxt = par.pTxt;

    const int len = (int)strlen( pTxt );

    double maxX = 0;

    double x = 0;
    double y = chH;

    for (int i=0; i < len; ++i)
    {
        int ch = pTxt[ i ];

        if ( ch == '\n' )
        {
            y += chH;

            maxX = DMax( maxX, x );
            x = 0;
            continue;
        }

        double chW = (mBaseDrawChWd[ch] + par.spacing) * par.scaleX;

        x += chW;
    }
    maxX = DMax( maxX, x );

    return { maxX, y };
}

//==================================================================
double FontFix::CalcStrWidth( const FontDrawParams &par, const char *pTxt ) const
{
    return CalcSize( par, pTxt )[0];
}

//==================================================================
size_t FontFix::CountCharsToMaxWidth( const FontDrawParams &par, double maxWd ) const
{
    double wd = 0;

    UTF8Parser  parse( (const U8 *)par.pTxt );

    U32 ch;
    while ( (ch = parse.GetNextCharacter()) )
    {
        DASSERT( ch < 256 );
        double chW = (mBaseDrawChWd[ch] + par.spacing) * par.scaleX;

        wd += chW;

        if ( wd > maxWd )
            return (const char *)parse.GetCurrent() - par.pTxt;
    }

    return strlen( par.pTxt );
}

