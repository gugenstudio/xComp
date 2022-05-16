//==================================================================
/// GraphicsUtils.cpp
///
/// Created by Davide Pasca - 2018/04/29
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#include "FontBase.h"
#include "GraphicsUtils.h"

//==================================================================
void GU_DrawCrossW(
                Graphics &g,
                const Double2 &c,
                const Double4 &rc,
                const double thick,
                const ColorF &col )
{
    const auto x0 = rc[0];
    const auto x1 = rc[0] + rc[2];
    const auto y0 = rc[1];
    const auto y1 = rc[1] + rc[3];

    c_auto &pixSiz = g.GetCurPixSize();
    c_auto tw = pixSiz[0] * thick;
    c_auto th = pixSiz[1] * thick;

    g.DrawRectFill(
            {x0    - tw/2, c[1] - th/2},
            {x1-x0 + tw  ,        th  },
            col );

    g.DrawRectFill(
            {c[0] - tw/2, y0    - th/2},
            {       tw  , y1-y0 + th  },
            col );
}

//==================================================================
void GU_DrawCross(
                Graphics &g,
                const Double2 &c,
                const Double4 &rc,
                const ColorF &col )
{
    c_auto useCol = col.ScaleRGB( 1.0 );
    GU_DrawCrossW( g, c, rc, 2.0, useCol );
    //GU_DrawCrossLine( g, c, rc, useCol );
}

//==================================================================
void GU_DrawCrossBG(
                Graphics &g,
                const Double2 &c,
                const Double4 &rc,
                const ColorF &col )
{
    GU_DrawCrossW( g, c, rc, 4.0, col );
}

//==================================================================
void GU_DrawDirTriangle(
                Graphics &g,
                Double2 c,
                const Double4 &rc,
                const char dir,
                const ColorF &col,
                const double outline )
{
    c_auto o = g.GetCurPixSize() * outline;

    auto x0 = rc[0]         - o[0];
    auto x1 = rc[0] + rc[2] + o[0];
    auto y0 = rc[1]         - o[1];
    auto y1 = rc[1] + rc[3] + o[1];

    auto dr = [&]( const Double2 &v0, const Double2 &v1, const Double2 &v2 )
    {
        g.DrawTri( {v0, v1, v2}, col );
    };

    switch ( dir )
    {
    default:
    case 'l': y0-=o[1]; y1+=o[1]; dr( {c[0]-o[0], c[1]}, {x1,y1}, {x1,y0} ); break;
    case 'r': y0-=o[1]; y1+=o[1]; dr( {c[0]+o[0], c[1]}, {x0,y1}, {x0,y0} ); break;
    case 'u': x0-=o[0]; x1+=o[0]; dr( {c[0], c[1]+o[1]}, {x0,y0}, {x1,y0} ); break;
    case 'd': x0-=o[0]; x1+=o[0]; dr( {c[0], c[1]-o[1]}, {x0,y1}, {x1,y1} ); break;
    }
}

//==================================================================
void GU_DrawHLineStipple(
                Graphics &g,
                const Double2 &pos,
                const double w,
                const ColorF &col )
{
    auto x0 = std::min( pos[0], pos[0] + w );
    auto x1 = std::max( pos[0], pos[0] + w );

    c_auto xf = g.GetXForm();

    c_auto p0Xfmed = V4__V4_Mul_M44<double,double>( Double4(x0, 0, 0, 1), xf );
    c_auto p1Xfmed = V4__V4_Mul_M44<double,double>( Double4(x1, 0, 0, 1), xf );

    c_auto clippedX0 = DClamp( p0Xfmed[0], -p0Xfmed[3], p0Xfmed[3] );
    c_auto clippedX1 = DClamp( p1Xfmed[0], -p1Xfmed[3], p1Xfmed[3] );

    c_auto invXf = xf.GetAffineInverse();

    x0 = V4__V4_Mul_M44<double,double>( Double4( clippedX0, 0, 0, p0Xfmed[3] ), invXf )[0];
    x1 = V4__V4_Mul_M44<double,double>( Double4( clippedX1, 0, 0, p1Xfmed[3] ), invXf )[0];

    if ( (x1 - x0) < 1e-6 )
        return;

    c_auto &pixSiz = g.GetCurPixSize();

    if ( pixSiz[0] < 1e-6 )
        return;

    c_auto tw = pixSiz[0] * 5;

    for (auto x=x0; x <= x1; x += tw * 2)
    {
        c_auto xx = std::min( x1, x + tw * 0.75 );
        g.DrawLine( x, pos[1], xx, pos[1], col );
    }
}

//==================================================================
void GU_DrawVLineStipple(
                Graphics &g,
                const Double2 &pos,
                const double h,
                const ColorF &col )
{
    auto y0 = std::min( pos[1], pos[1] + h );
    auto y1 = std::max( pos[1], pos[1] + h );

    c_auto xf = g.GetXForm();

    c_auto p0Xfmed = V4__V4_Mul_M44<double,double>( Double4(0, y0, 0, 1), xf );
    c_auto p1Xfmed = V4__V4_Mul_M44<double,double>( Double4(0, y1, 0, 1), xf );

    c_auto clippedY0 = DClamp( p0Xfmed[1], -p0Xfmed[3], p0Xfmed[3] );
    c_auto clippedY1 = DClamp( p1Xfmed[1], -p1Xfmed[3], p1Xfmed[3] );

    c_auto invXf = xf.GetAffineInverse();

    y0 = V4__V4_Mul_M44<double,double>( Double4( 0, clippedY0, 0, p0Xfmed[3] ), invXf )[1];
    y1 = V4__V4_Mul_M44<double,double>( Double4( 0, clippedY1, 0, p1Xfmed[3] ), invXf )[1];

    if ( (y1 - y0) < 1e-6 )
        return;

    c_auto &pixSiz = g.GetCurPixSize();

    if ( pixSiz[1] < 1e-6 )
        return;

    c_auto th = pixSiz[1] * 5;

    for (auto y=y0; y <= y1; y += th * 2)
    {
        c_auto yy = std::min( y1, y + th );
        g.DrawLine( pos[0], y, pos[0], yy, col );
    }
}

//==================================================================
void GU_DrawCrossStipple(
                Graphics &g,
                const Double2 &c,
                const Double4 &rc,
                const ColorF &col )
{
    GU_DrawHLineStipple( g, Double2(rc[0], c[1]), rc[2], col );
    GU_DrawVLineStipple( g, Double2(c[0], rc[1]), rc[3], col );
}

//==================================================================
void GU_SetFontScale( Graphics &g, FontDrawParams &io_par )
{
    c_auto &pixSiz = g.GetCurPixSize() * 1.15;
    io_par.scaleX = pixSiz[0];
    io_par.scaleY = pixSiz[1];
}

