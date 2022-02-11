//==================================================================
/// FontBase.h
///
/// Created by Davide Pasca - 2011/6/1
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#ifndef FONTBASE_H
#define FONTBASE_H

#include <ctype.h>
#include "ColorF.h"
#include "utf8parser.h"

//==================================================================
static constexpr u_int  FONT_DFLG_UNDERLINE    = 1 << 0;
static constexpr u_int  FONT_DFLG_OUTLINE      = 1 << 1;
static constexpr u_int  FONT_DFLG_OUTLINE_THIN = 1 << 2;
static constexpr u_int  FONT_DFLG_STRIKE       = 1 << 3;
static constexpr u_int  FONT_DFLG_SHADOW       = 1 << 4;
static constexpr u_int  FONT_DFLG_STIPPLED     = 1 << 5;
static constexpr u_int  FONT_DFLG_BOXED        = 1 << 6;
static constexpr u_int  FONT_DFLG_NOBILINEAR   = 1 << 7;

//==================================================================
class FontDrawParams
{
public:
    const char *pTxt = nullptr;
    double     px = 0;
    double     py = 0;
    double     pz = 0;
    int        alignX = -1;
    int        alignY = -1;
    double     scaleX = 1;
    double     scaleY = 1;
    double     spacing = 0;
    double     customOutlineOff = 0;
    ColorF     col1  = {0,0,0,0};
    ColorF     col2  = {0,0,0,0};
    ColorF     bgcol = {0,0,0,0};
    u_int      flags = 0;
    double     *pOutShape = nullptr;
};

//==================================================================
// temporary solution
#define FontBase FontBM

//==================================================================
namespace FB
{

//==================================================================
inline char *findSplitPoint( char *pStr, size_t fromLen )
{
#if defined(_DEBUG)
	size_t len = strlen( pStr );
	DASSERT( len >= fromLen );
#endif

	for (int i=(int)fromLen-1; i >= 0; --i)
	{
		if ( isspace( pStr[i] ) )
			return pStr + i;
	}

	return NULL;
}

//==================================================================
template<typename _FT, typename _DPT>
static bool textWrap_Single(
                    const char  *pText,
                    const _FT   &font,
                    _DPT        &par,
                    double      maxWidth,
                    DVec<DStr>  &out_lines )
{
    char	lineBuff[512];
    strcpy_s( lineBuff, pText );
    char *pRunTxt = lineBuff;

    while( true )
    {
        par.pTxt = pRunTxt;
        size_t fullLen = strlen(pRunTxt);
        size_t usedCharsN = font.CountCharsToMaxWidth( par, maxWidth );

        if ( usedCharsN >= fullLen || usedCharsN == 0 )
        {
            if ( out_lines.size() )
            {
                out_lines.push_back( pRunTxt );
                return true;
            }
            else
                return false;
        }

        char *pClosestBreak = findSplitPoint( pRunTxt, usedCharsN );

        if ( pClosestBreak )
        {
            pClosestBreak[0] = 0;
            out_lines.push_back( pRunTxt );
            pRunTxt = pClosestBreak + 1;
        }
        else
        {
            out_lines.push_back( DStr( pRunTxt, usedCharsN ) );
            pRunTxt += usedCharsN;
        }
    }
}

//==================================================================
template<typename _FT, typename _DPT>
inline bool WrapText(
            const char  *pText,
            const _FT   &font,
            _DPT        &par,
            double      maxWidth,
            DVec<DStr>  &out_lines )
{
    // is it multi-line ?
    UTF8Parser  parse( (const U8 *)pText );

    const char *pLastChar = (const char *)parse.GetCurrent();
    U32 ch;
    do {
        ch = parse.GetNextCharacter();

        if ( ch == '\n' )
        {
            auto lineLen = (const char *)parse.GetCurrent() - pLastChar;

            char buff[512];
            if ( lineLen <= (ptrdiff_t)_countof(buff) )
                DEX_RUNTIME_ERROR( "Out of buffer!" );

            memcpy( buff, pLastChar, (size_t)lineLen );
            buff[lineLen-1] = 0;

            bool hasSplit =
                textWrap_Single(
                    buff,
                    font,
                    par,
                    maxWidth,
                    out_lines );

            if NOT( hasSplit )
                out_lines.push_back( buff );

            pLastChar = (const char *)parse.GetCurrent();
        }
    } while ( ch != 0 );

    bool hasSplit =
        textWrap_Single(
            pLastChar,
            font,
            par,
            maxWidth,
            out_lines );

    if NOT( hasSplit )
        out_lines.push_back( pLastChar );

    return out_lines.size() != 0;
}

//==================================================================
template<typename _FT, typename _DPT>
inline bool WrapText(
            const char  *pText,
            const _FT   &font,
            _DPT        &par,
            double      maxWidth,
            DStr        &out_wrappedText )
{
    DVec<DStr>  splitLines;
    bool hasSplit = WrapText<_FT,_DPT>( pText, font, par, maxWidth, splitLines );

    if ( !hasSplit )
    {
        out_wrappedText.clear();
        return false;
    }

    out_wrappedText.reserve( strlen(pText) + splitLines.size() );
    out_wrappedText = splitLines[0];
    for (size_t i=1; i != splitLines.size(); ++i)
    {
        out_wrappedText += '\n';
        out_wrappedText += splitLines[i];
    }

    return true;
}

//==================================================================
}

#endif
