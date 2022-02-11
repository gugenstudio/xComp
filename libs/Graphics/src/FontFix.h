//==================================================================
/// FontFix.h
///
/// Created by Davide Pasca - 2018/3/17
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#ifndef FONTFIX_H
#define FONTFIX_H

#include "Image.h"
#include "Graphics.h"
#include "FontBase.h"

//==================================================================
class FontFix
{
    Int2    mSrcChSiz {0,0};
    Double2 mBaseDrawChFixSiz {0,0};

private:
    Int2    mSrcCoords[256] {};
    double  mBaseDrawChWd[256] {};
    image   mImg[2];

    Double2 mUseBaseScale {0,0};

public:
    struct Params
    {
        image   *pImg = nullptr;
        Double2 baseScale {1,1};
        bool    forceBilinear = false;
        bool    fixedWidth = true;
        double  descender = 1;
    };
private:
    Params  mPar;

public:
    FontFix( const Params &par );

    void ChangeBaseScale( const Double2 &baseScale );

    double GetCharWd() const { DASSERT( mPar.fixedWidth ); return mBaseDrawChFixSiz[0]; }
    double GetCharHe() const { return mBaseDrawChFixSiz[1]; }

    double CalcStrWidth( const FontDrawParams &par, const char *pTxt=NULL ) const;
    size_t CountCharsToMaxWidth( const FontDrawParams &par, double maxWd ) const;

    Double2 CalcSize(
            const FontDrawParams &par,
            const char *pTxt=nullptr ) const;

    //==================================================================
    void DrawText(
            Graphics &g,
            double x,
            double y,
            FontDrawParams &par ) const
    {
        par.px = x;
        par.py = y;
        DrawText( g, par );
    }

    void DrawText( Graphics &g, const FontDrawParams &par ) const;

    void DrawText(
                Graphics &g,
                const double shape[4],
                FontDrawParams &par ) const;

    void DrawText(
                Graphics &g,
                const Double4    &shape,
                FontDrawParams &par ) const
    {
        DrawText( g, &shape[0], par );
    }
};

#ifdef DrawText // evil Windows macro
# undef DrawText
#endif

#endif

