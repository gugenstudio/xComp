//==================================================================
/// GraphicsUtils.h
///
/// Created by Davide Pasca - 2018/02/18
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#ifndef GRAPHICSUTILS_H
#define GRAPHICSUTILS_H

#include "Graphics.h"

class FontDrawParams;

inline void GU_DrawCrossLine(
                Graphics &g,
                const Double2 &c,
                const Double4 &rc,
                const ColorF &col )
{
    const auto x0 = rc[0];
    const auto x1 = rc[0] + rc[2];
    const auto y0 = rc[1];
    const auto y1 = rc[1] + rc[3];
    g.DrawLine( x0, c[1], x1, c[1], col );
    g.DrawLine( c[0], y0, c[0], y1, col );
}

void GU_DrawCross(
                Graphics &g,
                const Double2 &c,
                const Double4 &rc,
                const ColorF &col );

void GU_DrawCrossBG(
                Graphics &g,
                const Double2 &c,
                const Double4 &rc,
                const ColorF &col );

void GU_DrawCrossStipple(
                Graphics &g,
                const Double2 &c,
                const Double4 &rc,
                const ColorF &col );

void GU_DrawHLineStipple(
                Graphics &g,
                const Double2 &pos,
                const double w,
                const ColorF &col );

void GU_DrawVLineStipple(
                Graphics &g,
                const Double2 &pos,
                const double h,
                const ColorF &col );

//==================================================================
void GU_SetFontScale( Graphics &g, FontDrawParams &io_par );

//==================================================================
inline bool GU_IsPointInsideRC( const Double2 &pos, const Double4 &rc )
{
    return pos[0] >= rc[0] &&
           pos[1] >= rc[1] &&
           pos[0] < (rc[0]+rc[2]) &&
           pos[1] < (rc[1]+rc[3]);
}

#endif

