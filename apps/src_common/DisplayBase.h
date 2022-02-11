//==================================================================
/// DisplayBase.h
///
/// Created by Davide Pasca - 2018/05/10
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#ifndef DISPLAYBASE_H
#define DISPLAYBASE_H

#include "ColorF.h"

//==================================================================
namespace Display
{

static constexpr auto RED          = ColorF( 255,  70,  75 );
static constexpr auto GREEN        = ColorF(  60, 255,  60 );
static constexpr auto BLUE         = ColorF(  60,  60, 255 );
static constexpr auto LIGHT_RED    = ColorF( 250, 140, 140 );
static constexpr auto LIGHT_GREEN  = ColorF( 140, 250, 140 );
static constexpr auto LIGHT_BLUE   = ColorF( 150, 150, 250 );
static constexpr auto WHITE        = ColorF( 255, 255, 255 );
static constexpr auto BLACK        = ColorF(   0,   0,   0 );
static constexpr auto GRAY         = ColorF( 150, 150, 150 );
static constexpr auto LIGHT_GRAY   = ColorF( 210, 210, 210 );
static constexpr auto YELLOW       = ColorF( 210, 210,  10 );
static constexpr auto ORANGE       = ColorF( 240, 140,  15 );
static constexpr auto CYAN         = ColorF(  10, 250, 250 );
static constexpr auto MAGENTA      = ColorF( 250,  10, 250 );
static constexpr auto LIGHT_MAGENTA= ColorF( 250, 110, 250 );

static constexpr auto MARKET_COL   = ColorF( 220,  10, 220 );

static constexpr auto COL_GOOD     = GREEN;
static constexpr auto COL_WARN     = ORANGE;
static constexpr auto COL_ERR      = RED;

//
inline ColorF GetCurrCol( size_t currIdx, size_t chanIdx=0 )
{
    if ( currIdx >= 2 )
        return LIGHT_BLUE;

    constexpr ColorF CURR_COLS[2][2] =
    {
        { { 1.0f, 0.f, 0.80f }, { 0.0f, 1.0f, 0.80f } },
        { { 0.8f, 0.f, 0.65f }, { 0.0f, 0.8f, 0.65f } },
    };

    c_auto isUnkChan = (chanIdx == DNPOS);
    chanIdx = (chanIdx == DNPOS ? 0 : chanIdx);

    return CURR_COLS[ chanIdx % 2 ][ currIdx % 2 ] * (isUnkChan ? 0.7f : 1.f);
}

//
const ColorF &GetMarketCol( size_t idx );
ColorF GetMarketCol( const DStr &mktStr );

//
inline ColorF GetPredCol( size_t colIdx )
{
    constexpr ColorF COLS[3] =
    {
        { 240,  80,  80, 180 },
        {  80, 240,  80, 180 },
        {  80,  80, 240, 180 },
    };

    return COLS[ colIdx % 3 ];
}

//
inline ColorF GetPNLCol( double val )
{
    return val >= 0 ? Display::GREEN : Display::RED;
}

inline ColorF GetNAVCol( double val )
{
    return val >= 1 ? Display::GREEN : Display::RED;
}

};

#endif

