//==================================================================
/// DisplayBase.cpp
///
/// Created by Davide Pasca - 2021/04/19
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#include "DisplayBase.h"

//==================================================================
namespace Display
{

static constexpr auto LIGHT_YELLOW       = ColorF( 210, 210,  70 );
static constexpr auto LIGHT_ORANGE       = ColorF( 240, 140,  75 );
static constexpr auto LIGHT_CYAN         = ColorF(  70, 250, 250 );

//==================================================================
const ColorF &GetMarketCol( size_t idx )
{
#if 0
    static constexpr auto PURE_R = ColorF( 255, 0,   0   );
    static constexpr auto PURE_G = ColorF( 255, 255, 0   );
    static constexpr auto PURE_B = ColorF( 0,   0,   255 );

    static const auto OFF = 0.20f;

    static const ColorF sMarketCols[] =
    {
        DLerp( PURE_R, PURE_G, 0.00f ) + OFF,
        DLerp( PURE_G, PURE_B, 0.00f ) + OFF,
        DLerp( PURE_B, PURE_R, 0.00f ) + OFF,

        DLerp( PURE_R, PURE_G, 0.33f ) + OFF,
        DLerp( PURE_G, PURE_B, 0.33f ) + OFF,
        DLerp( PURE_B, PURE_R, 0.33f ) + OFF,

        DLerp( PURE_R, PURE_G, 0.66f ) + OFF,
        DLerp( PURE_G, PURE_B, 0.66f ) + OFF,
        DLerp( PURE_B, PURE_R, 0.66f ) + OFF,

        DLerp( PURE_R, PURE_G, 1.00f ) + OFF,
        DLerp( PURE_G, PURE_B, 1.00f ) + OFF,
        DLerp( PURE_B, PURE_R, 1.00f ) + OFF,
    };
#else
    static const ColorF sMarketCols[] =
    {
        Display::LIGHT_RED    ,
        Display::ORANGE       ,
        Display::YELLOW       ,
        Display::LIGHT_GREEN  ,
        Display::CYAN         ,
        Display::LIGHT_BLUE   ,
        Display::BLUE         ,
        Display::MAGENTA      ,
        Display::LIGHT_MAGENTA,
        Display::LIGHT_GRAY   ,
        Display::GRAY         ,
        (Display::GRAY  + Display::LIGHT_RED) * 0.5f,
    };
#endif

    return sMarketCols[ idx % std::size(sMarketCols) ];
}

//==================================================================
ColorF GetMarketCol( const DStr &mktStr )
{
#if 0
    int sum = 0;
    size_t n = mktStr.size();
    if ( n > 3 )
    {
        sum = (mktStr[1  ] - 'A') * 1 +
              (mktStr[0  ] - 'A') * 3 +
              (mktStr[n-1] - 'A') * 0;
    }

    return GetMarketCol( (size_t)sum );
#else
    c_auto n = mktStr.size();
    if ( n < 3 )
    {
        return Display::MAGENTA;
    }
    else
    {
        constexpr auto len = (float)('Z' - 'A' + 1);
        c_auto c0 = (float)std::max( 0, (int)mktStr[0] - 'A' ) / len;
        c_auto c1 = (float)std::max( 0, (int)mktStr[1] - 'A' ) / len;
        c_auto c2 = (float)std::max( 0, (int)mktStr[2] - 'A' ) / len;

        c_auto y  = (c0 + c1 + c2) / 3;
        c_auto cr = c0 - c1 + 0.5f;
        c_auto cb = c0 - c2 + 0.5f;

        return ColorF(
                DClamp( y + cr - 0.5f, 0.f, 1.f ),
                DClamp( y            , 0.f, 1.f ),
                DClamp( y + cb - 0.5f, 0.f, 1.f ) );
    }
#endif
}

//==================================================================
}
