//==================================================================
/// IMUI_Utils.cpp
///
/// Created by Davide Pasca - 2019/12/24
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#ifdef ENABLE_IMGUI

#define IMGUI_DEFINE_MATH_OPERATORS

#include "imgui.h"
#include "imgui_stdlib.h"
#include "imgui_internal.h"
#include "TimeUtils.h"
#include "FileUtils.h"
#include "IMUI_Utils.h"

static float _gsContentScale = 1.f;
static bool  _gsIsRetinaDisplay = false;

static auto IMUI_IMVEC4_TEXT_LITESCALE = ImVec4( 0.65f, 0.65f, 0.65f, 1.0f );
static auto IMUI_COLF_TEXT_LITESCALE   = ColorF( 0.65f, 0.65f, 0.65f, 1.0f );

//==================================================================
void IMUI_SetContentScale( float sca )
{
    _gsContentScale = sca;
}

float IMUI_GetContentScale()
{
    return _gsContentScale;
}

//==================================================================
void IMUI_SetIsRetinaDisplay( bool isRetina )
{
    _gsIsRetinaDisplay = isRetina;
}

bool IMUI_GetIsRetinaDisplay()
{
    return _gsIsRetinaDisplay;
}

//==================================================================
bool IMUI_IsLightMode()
{
    c_auto &c = ImGui::GetStyle().Colors[ImGuiCol_Text];

    return ((c.x + c.y + c.z) * 1.f/3) <= 0.5f;
}

//==================================================================
ColorF IMUI_MakeColForBG( const ColorF &col, float alpha )
{
    c_auto bgCol = ColorF(
        ImGui::GetStyle().Colors[ImGuiCol_WindowBg].x,
        ImGui::GetStyle().Colors[ImGuiCol_WindowBg].y,
        ImGui::GetStyle().Colors[ImGuiCol_WindowBg].z );

    return DLerp( bgCol, col, alpha );
}

//==================================================================
void IMUI_SetNextWindowSize( const ImVec2& size, ImGuiCond cond )
{
    ImGui::SetNextWindowSize( size * _gsContentScale, cond );
}

//==================================================================
static float safeScaleWidWidth( float w )
{
    return
        (w == -FLT_MIN || w == FLT_MIN || w == FLT_MAX || w == -FLT_MAX)
            ? w
            : w * _gsContentScale;
}

//==================================================================
void IMUI_SetNextItemWidth( float w )
{
    ImGui::SetNextItemWidth( safeScaleWidWidth( w ) );
}

//==================================================================
void IMUI_PushItemWidth( float w )
{
    ImGui::PushItemWidth( safeScaleWidWidth( w ) );
}

//==================================================================
void IMUI_SetColumnWidth( int colIdx, float w )
{
    ImGui::SetColumnWidth( colIdx, w * _gsContentScale );
}

//==================================================================
void IMUI_Indent( float w )
{
    ImGui::Indent( 10 * _gsContentScale );
}

void IMUI_Unindent( float w )
{
    ImGui::Unindent( 10 * _gsContentScale );
}

//==================================================================
void IMUI_Text( const char *pTxt )
{
    ImGui::TextEx( pTxt,
                   pTxt + strlen(pTxt),
                   ImGuiTextFlags_NoWidthForLargeClippedText );
}
//
void IMUI_Text( const DStr &txt )
{
    ImGui::TextEx( txt.c_str(),
                   txt.c_str() + txt.size(),
                   ImGuiTextFlags_NoWidthForLargeClippedText );
}

//==================================================================
void IMUI_TextRightAligned( const DStr &text )
{
    using namespace ImGui;

    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    if (window->SkipItems)
        return;

    c_auto *pSta = text.c_str();
    c_auto *pEnd = pSta + text.size();

    window->DC.CursorPos.x +=
        ImMax(0.0f, GetContentRegionAvail().x - CalcTextSize( pSta, pEnd ).x);

    TextEx( pSta, pEnd, ImGuiTextFlags_NoWidthForLargeClippedText );
}

//==================================================================
void IMUI_TextWrappedDisabled( const DStr &text )
{
    ImGui::PushStyleColor(
        ImGuiCol_Text, ImGui::GetStyleColorVec4( ImGuiCol_TextDisabled ) );

    ImGui::TextWrapped( "%s", text.c_str() );

    ImGui::PopStyleColor();
}

//==================================================================
void IMUI_DrawHeader( const DStr &str, bool topPadding )
{
    if ( topPadding )
        IMUI_ShortNewLine( 0.25f );

    //ImGui::SameLine( 0, 30 );
    IMUI_Indent( 10 );

    ImGui::PushStyleColor( ImGuiCol_Text, ImGui::GetColorU32( ImGuiCol_HeaderActive ) );
    IMUI_Text( str );
    ImGui::PopStyleColor();

    ImGui::Separator();
    IMUI_Unindent( 10 );
}

//==================================================================
void IMUI_MakeTooltip( const DStr &str )
{
    if ( ImGui::IsItemHovered() )
    {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 20.0f);
        ImGui::TextUnformatted( str.c_str() );
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

//==================================================================
void IMUI_HelpMarker( const DStr &str )
{
    ImGui::TextDisabled( "(?)" );
    IMUI_MakeTooltip( str );
}

//==================================================================
void IMUI_PushButtonColors( const ColorF &col )
{
    c_auto colH = col.ScaleRGB( 1.1f );
    c_auto colV = col.ScaleRGB( 1.2f );
    IMUI_PushStyleColor(ImGuiCol_Button,        IMUI_MkCol( col  ) );
    IMUI_PushStyleColor(ImGuiCol_ButtonHovered, IMUI_MkCol( colH ) );
    IMUI_PushStyleColor(ImGuiCol_ButtonActive,  IMUI_MkCol( colV ) );

    c_auto makeLiteTxt =
            IMUI_IsLightMode() &&
            (col[0] + col[1] + col[2]) < (3 * 0.5f);

    auto txtCol = ImGui::GetStyleColorVec4( ImGuiCol_Text );

    if ( makeLiteTxt )
    {
        txtCol = ImVec4(
                    1.f - txtCol.x/2,
                    1.f - txtCol.x/2,
                    1.f - txtCol.y/2,
                          txtCol.w );
    }

    ImGui::PushStyleColor( ImGuiCol_Text, txtCol );
}

//==================================================================
void IMUI_PopButtonColors()
{
    IMUI_PopStyleColor( 4 );
}

//==================================================================
// https://gist.github.com/dougbinks/ef0962ef6ebe2cadae76c4e9f0586c69#file-imguiutils-h-L228-L262
static void IMUI_AddUnderLine( ImColor col_ )
{
    ImVec2 min = ImGui::GetItemRectMin();
    ImVec2 max = ImGui::GetItemRectMax();
    min.y = max.y;
    ImGui::GetWindowDrawList()->AddLine( min, max, col_, 2.0f );
}

//==================================================================
bool IMUI_TextURL(
            const char* name,
            bool sameLineBefore,
            bool sameLineAfter )
{
    if ( sameLineBefore )
        ImGui::SameLine( 0.0f, ImGui::GetStyle().ItemInnerSpacing.x );

    ImGui::PushStyleColor(
                ImGuiCol_Text,
                ImGui::GetStyle().Colors[ImGuiCol_ButtonHovered]);

    ImGui::Text( name );
    ImGui::PopStyleColor();

    if ( ImGui::IsItemHovered() )
    {
        if( ImGui::IsMouseClicked(0) )
            return true;

        IMUI_AddUnderLine( ImGui::GetStyle().Colors[ImGuiCol_ButtonHovered] );
    }
    else
    {
        IMUI_AddUnderLine( ImGui::GetStyle().Colors[ImGuiCol_Button] );
    }

    if ( sameLineAfter )
        ImGui::SameLine( 0.0f, ImGui::GetStyle().ItemInnerSpacing.x );

    return false;
}

//==================================================================
static void drawSmartURL( const DStr &desc, const DStr &url )
{
    if ( !desc.empty() && StrStartsWithI( url, "http" ) )
    {
        ImGui::AlignTextToFramePadding();
        ImGui::Bullet();
        ImGui::SameLine();
        IMUI_Text( desc );
    }
    else
    {
        auto tmp = url;
        ImGui::InputText( (desc+"##"+url).c_str(), &tmp, ImGuiInputTextFlags_ReadOnly );
    }
}

static bool buttonSmartURL( const DStr &desc, const DStr &url )
{
    ImGui::AlignTextToFramePadding();
    IMUI_Text( "Open" );
    ImGui::SameLine();

#if 0
    return ImGui::Button( ((desc.empty() ? url : desc)+"##"+url).c_str() );
#else
    return IMUI_TextURL( (desc.empty() ? url : desc).c_str() );
#endif
}

//==================================================================
void IMUI_DrawLinkPopup( const DStr &fullURL )
{
    if ( ImGui::BeginPopup( fullURL.c_str() ) )
    {
#if 0
        drawSmartURL( {}, fullURL );

        if ( FU_CanOpenURL() )
        {
            ImGui::SameLine();

            if ( ImGui::Button( "Open..." ) )
            {
                FU_OpenURL( fullURL );
            }
        }
#else
        if ( FU_CanOpenURL() )
        {
            if ( buttonSmartURL( fullURL, fullURL ) )
            {
                FU_OpenURL( fullURL );
            }
        }
#endif
        ImGui::SameLine();

        if ( ImGui::Button( "Copy" ) )
            ImGui::SetClipboardText( fullURL.c_str() );

        ImGui::EndPopup();
    }
}

//==================================================================
void IMUI_DrawLinkPopup(
        const DStr &nameIDStr,
        const DFun<DStr ()> &getFullURL1Fn,
        const DFun<DStr ()> &getFullURL2Fn,
        const DStr &desc1Str,
        const DStr &desc2Str )
{
    if ( nameIDStr.empty() || ImGui::BeginPopup( nameIDStr.c_str() ) )
    {
        auto drawEntry = [&]( c_auto &desc, c_auto &fullURL )
        {
#if 0
            drawSmartURL( desc, fullURL );

            if ( FU_CanOpenURL() )
            {
                ImGui::SameLine();
                if ( ImGui::Button( "Open..." ) )
                {
                    FU_OpenURL( fullURL );
                }
            }
#else
            if ( FU_CanOpenURL() )
            {
                if ( buttonSmartURL( desc, fullURL ) )
                {
                    FU_OpenURL( fullURL );
                }
            }
#endif
            ImGui::SameLine();

            if ( ImGui::Button( "Copy" ) )
                ImGui::SetClipboardText( fullURL.c_str() );
        };

        bool didURL1 = false;

        if ( getFullURL1Fn )
        {
            if (c_auto &url = getFullURL1Fn(); !url.empty())
            {
                ImGui::PushID( "URL1" );
                drawEntry( desc1Str, getFullURL1Fn() );
                ImGui::PopID();

                didURL1 = true;
            }
        }

        if ( getFullURL2Fn )
        {
            if (c_auto &url = getFullURL2Fn(); !url.empty())
            {
                if ( didURL1 )
                    ImGui::Separator();

                ImGui::PushID( "URL2" );
                drawEntry( desc2Str, getFullURL2Fn() );
                ImGui::PopID();
            }
        }

        if NOT( nameIDStr.empty() )
            ImGui::EndPopup();
    }
}

//==================================================================
void IMUI_DrawLinkOpenPopup(
            const DStr &fullURL,
            const DStr &dispURL,
            bool useSmall,
            const ColorF &col )
{
    if ( col[3] != 0 )
        IMUI_PushButtonColors( col );

    c_auto didClick = (useSmall
            ?  ImGui::SmallButton( dispURL.c_str() )
                        : ImGui::Button( dispURL.c_str() ));

    if ( col[3] != 0 )
        IMUI_PopButtonColors();

    if ( didClick )
        ImGui::OpenPopup( fullURL.c_str() );
    }

//==================================================================
void IMUI_DrawLink(
            const DStr &fullURL,
            const DStr &dispURL,
            bool useSmall,
            const ColorF &col )
{
    IMUI_DrawLinkOpenPopup( fullURL, dispURL, useSmall, col );
    IMUI_DrawLinkPopup( fullURL );
}

//==================================================================
void IMUI_SameLine( float offX, float spacingW )
{
    offX *= _gsContentScale;
    spacingW = (spacingW < 0 ? spacingW : spacingW * _gsContentScale);

    ImGui::SameLine( offX, spacingW );
}

//==================================================================
void IMUI_SameLineN( size_t n )
{
    ImGui::SameLine( 0, GImGui->Style.ItemSpacing.x * n );
}

//==================================================================
void IMUI_Spacing( float offX, float offY )
{
    offX *= _gsContentScale;
    offY *= _gsContentScale;
    ImGui::Dummy( {offX, offY} );
}

//==================================================================
void IMUI_ShortNewLine( float scale )
{
    using namespace ImGui;
    {
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return;

    ImGuiContext& g = *GImGui;
    const ImGuiLayoutType backup_layout_type = window->DC.LayoutType;
    window->DC.LayoutType = ImGuiLayoutType_Vertical;
    if (window->DC.CurrLineSize.y > 0.0f)     // In the event that we are on a line with items that is smaller that FontSize high, we will preserve its height.
        ItemSize(ImVec2(0,0));
    else
        ItemSize(ImVec2(0.0f, g.FontSize * scale));
    window->DC.LayoutType = backup_layout_type;
    }
}

//==================================================================
void IMUI_PushDisabled()
{
    ImGui::PushItemFlag( ImGuiItemFlags_Disabled, true );
    ImGui::PushStyleVar( ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f );
}

//==================================================================
void IMUI_PopDisabled()
{
    ImGui::PopItemFlag();
    ImGui::PopStyleVar();
}

//==================================================================
void IMUI_PushDisabledStyle()
{
    ImGui::PushStyleVar( ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f );
}

//==================================================================
void IMUI_PopDisabledStyle()
{
    ImGui::PopStyleVar();
}

//==================================================================
inline auto flashAlpha = []( c_auto &src )
{
    auto out = src;

    c_auto t = abs( sin( M_PI * GetEpochTimeUS().CalcTimeS() ) );
    out.w = (float)DLerp( 0.2, 1.0, t );
    return out;
};

//==================================================================
void IMUI_PushFlashing( const ImVec4 &col )
{
    IMUI_PushStyleColor( ImGuiCol_Text, flashAlpha( col ) );
}

//==================================================================
void IMUI_PopFlashing()
{
    ImGui::PopStyleColor();
}

//==================================================================
ImVec4 IMUI_MkTextCol( const ImVec4 &col )
{
    return col * (IMUI_IsLightMode()
            ? IMUI_IMVEC4_TEXT_LITESCALE
            : ImVec4( 1.00f, 1.00f, 1.00f, 1.0f ) );
}
//==================================================================
ColorF IMUI_MkTextCol( const ColorF &col )
{
    return col * (IMUI_IsLightMode()
            ? IMUI_COLF_TEXT_LITESCALE
            : ColorF( 1.00f, 1.00f, 1.00f, 1.0f ) );
}

//==================================================================
void IMUI_PushStyleColor( ImGuiCol colIdx, const ImVec4 &col )
{
    if NOT( IMUI_IsLightMode() )
    {
        ImGui::PushStyleColor( colIdx, col );
        return;
    }

    c_auto useCol = col * (colIdx == ImGuiCol_Text
            ? IMUI_IMVEC4_TEXT_LITESCALE
            : ImVec4( 1.30f,1.30f,1.30f,1.0f ));

    ImGui::PushStyleColor( colIdx, useCol );
}

//==================================================================
void IMUI_PopStyleColor( int count )
{
    ImGui::PopStyleColor( count );
}

//==================================================================
void IMUI_PushTextCol( const ImVec4 &col )
{
    IMUI_PushStyleColor( ImGuiCol_Text, col );
}

//==================================================================
void IMUI_PopTextColor()
{
    IMUI_PopStyleColor();
}

//==================================================================
ImVec2 IMUI_GetWindowInnerClipRect()
{
    const ImGuiWindow* window = ImGui::GetCurrentWindowRead();

    //return window->Size;
    c_auto rc = window->InnerClipRect;
    //c_auto rc = window->InnerRect;

    return
    {
        rc.Max[0] - rc.Min[0],
        rc.Max[1] - rc.Min[1]
    };
}

//==================================================================
float IMUI_GetWindowContentRegionWidth()
{
    return GImGui->CurrentWindow->ContentRegionRect.GetWidth();
}

float IMUI_GetWindowContentRegionHeight()
{
    return GImGui->CurrentWindow->ContentRegionRect.GetHeight();
}

//==================================================================
void IMUI_DrawArrow( ImGuiDir dir, float scale )
{
    auto* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return;

    c_auto size = ImVec2( 10, 10 ) * scale;

    ImGuiContext& g = *GImGui;
    const ImRect bb(window->DC.CursorPos, window->DC.CursorPos + size);
    const float default_size = ImGui::GetFrameHeight();
    ImGui::ItemSize(size, (size.y >= default_size) ? g.Style.FramePadding.y : -1.0f);

    // Render
    const ImU32 text_col = ImGui::GetColorU32(ImGuiCol_Text);

#if 1
    ImGui::RenderArrow(
            window->DrawList,
        bb.Min + ImVec2(
                    ImMax(0.0f, (size.x - g.FontSize*1.0f) * 0.5f),
                    ImMax(0.0f, (size.y - g.FontSize*0.0f) * 0.5f) ),
            text_col,
        dir,
        scale );
#else
    ImGui::RenderArrowPointingAt(
        window->DrawList,
        (bb.Min + bb.Max) * 0.5f,
        size,
        dir,
        text_col );
#endif
}

//==================================================================
void IMUI_DrawPriceArrow( double curVal, double prevVal )
{
    static const auto RED     = ImVec4( 1.0f, 0.2f, 0.2f, 1.0f );
    static const auto GREEN   = ImVec4( 0.1f, 1.0f, 0.1f, 1.0f );

    c_auto isUp = (curVal >= prevVal);
    IMUI_PushTextCol( isUp ? GREEN : RED );
    IMUI_DrawArrow( isUp ? ImGuiDir_Up : ImGuiDir_Down );
    IMUI_PopTextColor();
}

//==================================================================
static bool beginOverlayWin()
{
    ImGui::SetNextWindowBgAlpha( 0.75f );

    return ImGui::Begin("Corner Overlay Status", nullptr,
                  ImGuiWindowFlags_NoDocking
                | ImGuiWindowFlags_NoTitleBar
                | ImGuiWindowFlags_NoResize
                | ImGuiWindowFlags_AlwaysAutoResize
                | ImGuiWindowFlags_NoSavedSettings
                | ImGuiWindowFlags_NoFocusOnAppearing
                | ImGuiWindowFlags_NoNav );
}

//==================================================================
bool IMUI_BeginChildMouseOverlay()
{
    const auto* window = ImGui::GetCurrentWindowRead();
    if NOT( window )
        return false;

    c_auto rc = window->InnerClipRect;

    c_auto winMidX = (rc.Min.x + rc.Max.x) * 0.5f;
    c_auto winMidY = (rc.Min.y + rc.Max.y) * 0.5f;

    c_auto mousePos = ImGui::GetMousePos();

    c_auto alignX = mousePos.x < winMidX ? -1 : 1;
    c_auto alignY = mousePos.y < winMidY ? -1 : 1;

    const float DISTANCE_X = 10.0f;
    const float DISTANCE_Y = 35.0f;

    c_auto piv = ImVec2( (alignX + 1) * 0.5f,
                         (alignY + 1) * 0.5f );

    c_auto winPos = ImVec2(
                        mousePos.x - alignX * DISTANCE_X,
                        mousePos.y - alignY * DISTANCE_Y );

    ImGui::SetNextWindowPos( winPos, ImGuiCond_Always, piv );

    return beginOverlayWin();
}

//==================================================================
bool IMUI_BeginChildCornerOverlay( int corner )
{
    const auto* window = ImGui::GetCurrentWindowRead();
    if NOT( window )
        return false;

    c_auto parPosX = window->InnerClipRect.Min.x;
    c_auto parPosY = window->InnerClipRect.Min.y;
    c_auto parSizX = window->InnerClipRect.Max.x - window->InnerClipRect.Min.x;
    c_auto parSizY = window->InnerClipRect.Max.y - window->InnerClipRect.Min.y;

    const float DISTANCE = 10.0f;

    c_auto window_pos =
        ImVec2(
            (corner & 1)
                ? (parPosX + parSizX - DISTANCE)
                : (parPosX + DISTANCE),
            (corner & 2)
                ? (parPosY + parSizY - DISTANCE)
                : (parPosY + DISTANCE) );

    c_auto window_pos_pivot = ImVec2( (corner & 1) ? 1.0f : 0.0f,
                                      (corner & 2) ? 1.0f : 0.0f);

    ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);

    return beginOverlayWin();
}

//==================================================================
float IMUI_CalcItemsHeight( size_t textsN, size_t widgetsN, size_t sepsN )
{
    c_auto siz = ImGui::CalcTextSize( "A", NULL, false );

    return
        GImGui->Style.FramePadding.y +
        siz[1] * ( (float)textsN   * 1.30f +
                   (float)widgetsN * 1.60f +
                   (float)sepsN    * 0.30f);
}

//==================================================================
float IMUI_CalcHeightForChildWithBottom( size_t textsN, size_t widgetsN, size_t sepsN )
{
    const float BOTTOM_SPACE = IMUI_CalcItemsHeight( textsN, widgetsN, sepsN );

    c_auto winH = ImGui::GetContentRegionAvail().y;

    return std::max( 50.f, winH - BOTTOM_SPACE );
}

//==================================================================
bool IMUI_BeginChildWithBottom(
                    const char *pName,
                    size_t textsN,
                    size_t widgetsN,
                    size_t sepsN )
{
    c_auto useH = IMUI_CalcHeightForChildWithBottom( textsN, widgetsN, sepsN );

    if NOT( ImGui::BeginChild( pName, ImVec2( 0, useH ), false, 0 ) )
    {
        ImGui::EndChild();
        return false;
    }

    return true;
}

//==================================================================
bool IMUI_ButtonEnabled(
        const char *pLabel,
        bool doEnable,
        const ImVec2 &siz,
        const ColorF &col )
{
    if ( col[3] )
        IMUI_PushButtonColors( col );

    if NOT( doEnable )
        IMUI_PushDisabled();

    c_auto didClick = ImGui::Button( pLabel, siz * _gsContentScale ) && doEnable;

    if NOT( doEnable )
        IMUI_PopDisabled();

    if ( col[3] )
        IMUI_PopButtonColors();

    return didClick;
}

//==================================================================
bool IMUI_SmallButtonEnabled( const char *pLabel, bool doEnable, const ImVec2 siz )
{
    if NOT( doEnable )
        IMUI_PushDisabled();

    ImGuiContext& g = *GImGui;
    float backup_padding_y = g.Style.FramePadding.y;
    g.Style.FramePadding.y = 0.0f;
    bool pressed = ImGui::ButtonEx(pLabel, siz, ImGuiButtonFlags_AlignTextBaseLine);
    g.Style.FramePadding.y = backup_padding_y;

    if NOT( doEnable )
        IMUI_PopDisabled();

    return pressed;
}

//==================================================================
// https://gist.github.com/kudaba/645cb5e18746290460b633230342113a
// Copy of BeginMenuBar except it's at the bottom
float IMUI_CalcMainStatusBarHeight()
{
    c_auto &g = *GImGui;

    return
        g.NextWindowData.MenuBarOffsetMinVal.y +
        g.FontSize +
        g.Style.FramePadding.y * 2;
}

//==================================================================
bool IMUI_BeginMainStatusBar()
{
    using namespace ImGui;

    ImGuiContext& g = *GImGui;
    ImGuiViewport* viewport = g.Viewports[0];

    c_auto height = IMUI_CalcMainStatusBarHeight();

    g.NextWindowData.MenuBarOffsetMinVal =
        ImVec2(       g.Style.DisplaySafeAreaPadding.x,
                ImMax(g.Style.DisplaySafeAreaPadding.y - g.Style.FramePadding.y, 0.0f));

    SetNextWindowPos(ImVec2(viewport->Pos.x, viewport->Pos.y + viewport->Size.y - height));
    SetNextWindowSize(ImVec2(viewport->Size.x, height));

    // Enforce viewport so we don't create our onw viewport when
    //  ImGuiConfigFlags_ViewportsNoMerge is set.
    SetNextWindowViewport(viewport->ID);

    PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(0,0));

    ImGuiWindowFlags window_flags =
                    ImGuiWindowFlags_NoDocking |
                    ImGuiWindowFlags_NoTitleBar |
                    ImGuiWindowFlags_NoResize |
                    ImGuiWindowFlags_NoMove |
                    ImGuiWindowFlags_NoScrollbar |
                    ImGuiWindowFlags_NoSavedSettings |
                    ImGuiWindowFlags_MenuBar;

    bool is_open = Begin("##MainStatusBar", NULL, window_flags) && BeginMenuBar();

    PopStyleVar(2);

    g.NextWindowData.MenuBarOffsetMinVal = ImVec2(0.0f, 0.0f);

    if (!is_open)
    {
        End();
        return false;
    }

    return true;
}

//==================================================================
void IMUI_EndMainStatusBar()
{
    using namespace ImGui;

    EndMainMenuBar();
}

//==================================================================
bool IMUI_InputDoubleDispPct(
                        const char *pID,
                        double *pVal,
                        double major,
                        const char *pFmt,
                        double minPct,
                        double maxPct )
{
    auto tmpPct = *pVal * 100;

    c_auto didChange =  ImGui::InputDouble( pID, &tmpPct, major/10, major, pFmt, 0 );

    if ( minPct != DBL_MAX )
        tmpPct = std::max( tmpPct, minPct );

    if ( maxPct != -DBL_MAX )
        tmpPct = std::min( tmpPct, maxPct );

    *pVal = tmpPct / 100;

    return didChange;
}

//==================================================================
static const char _gsColorMarkerStart = '{';
static const char _gsColorMarkerEnd   = '}';

//==================================================================
static bool ProcessInlineHexColor( const char* start, const char* end, ImVec4& color )
{
    const int hexCount = ( int )( end - start );
    if( hexCount == 6 || hexCount == 8 )
    {
        char hex[9];
        strncpy( hex, start, hexCount );
        hex[hexCount] = 0;

        unsigned int hexColor = 0;
        if( sscanf( hex, "%x", &hexColor ) > 0 )
        {
            color.x = static_cast< float >( ( hexColor & 0x00FF0000 ) >> 16 ) / 255.0f;
            color.y = static_cast< float >( ( hexColor & 0x0000FF00 ) >> 8  ) / 255.0f;
            color.z = static_cast< float >( ( hexColor & 0x000000FF )       ) / 255.0f;
            color.w = 1.0f;

            if( hexCount == 8 )
            {
                color.w = static_cast< float >( ( hexColor & 0xFF000000 ) >> 24 ) / 255.0f;
            }

            return true;
        }
    }

    return false;
}

//==================================================================
// https://github.com/ocornut/imgui/issues/902#issuecomment-291229555
void IMUI_TextWithColors( const DStr &str )
{
    bool pushedColorStyle = false;
    c_auto *textStart = str.c_str();
    c_auto *textCur = str.c_str();
    c_auto *textEnd = str.c_str() + str.size();
    while( textCur < textEnd && *textCur != '\0' )
    {
        if( *textCur == _gsColorMarkerStart )
        {
            // Print accumulated text
            if( textCur != textStart )
            {
                ImGui::TextUnformatted( textStart, textCur );
                ImGui::SameLine( 0.0f, 0.0f );
            }

            // Process color code
            const char* colorStart = textCur + 1;
            do
            {
                ++textCur;
            }
            while( *textCur != '\0' && *textCur != _gsColorMarkerEnd );

            // Change color
            if( pushedColorStyle )
            {
                IMUI_PopTextColor();
                pushedColorStyle = false;
            }

            ImVec4 textColor;
            if( ProcessInlineHexColor( colorStart, textCur, textColor ) )
            {
                IMUI_PushTextCol( textColor );
                pushedColorStyle = true;
            }

            textStart = textCur + 1;
        }
        else if( *textCur == '\n' )
        {
            // Print accumulated text an go to next line
            ImGui::TextUnformatted( textStart, textCur );
            textStart = textCur + 1;
        }

        ++textCur;
    }

    if( textCur != textStart )
    {
        ImGui::TextUnformatted( textStart, textCur );
    }
    else
    {
        ImGui::NewLine();
    }

    if( pushedColorStyle )
    {
        IMUI_PopTextColor();
    }
}

//==================================================================
//==================================================================
void IMUI_ModalDialog::OpenDialog( const DStr &txt )
{
    mDialogStyText = txt;
    mDoOpen = true;
}

//==================================================================
void IMUI_ModalDialog::DrawDialog( const DStr &title )
{
    if ( ImGui::BeginPopupModal(
            title.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize) )
    {
        IMUI_TextWithColors( mDialogStyText );

        ImGui::NewLine();
        ImGui::Separator();

        if ( ImGui::Button("OK", ImVec2( IMUI_GetContentScale() * 120, 0)) )
        {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    if ( mDoOpen )
    {
        mDoOpen = false;
        ImGui::OpenPopup( title.c_str() );
    }
}

//==================================================================
bool IMUI_ComboText(
            const DStr &comboName,
            DStr &io_val,
            const DVec<const char *> &pVals,
            const DVec<const char *> &pValsDisp,
            bool makeReadOnly,
            const char *pDefVal )
{
    if ( makeReadOnly )
    {
        DStr tmp = io_val;
        if ( pValsDisp.size() == pVals.size() )
        {
            for (size_t i=0; i < pValsDisp.size(); ++i)
            {
                if ( !strcasecmp( io_val.c_str(), pVals[i] ) )
                {
                    tmp = pValsDisp[i];
                    break;
                }
            }
        }

        IMUI_PushDisabledStyle();
        ImGui::InputText( comboName.c_str(), &tmp, ImGuiInputTextFlags_ReadOnly );
        IMUI_PopDisabledStyle();

        return false;
    }

    c_auto n = std::size(pVals);

    auto findRow = [&]( const char *pSearch )
    {
        for (size_t i=0; i < n; ++i)
            if ( !strcasecmp( pVals[i], pSearch ) )
                return (int)i;

        return (int)-1;
    };

    auto row = findRow( io_val.c_str() );

    // try with the default value, if any
    if ( row < 0 && pDefVal )
        row = findRow( pDefVal );

    if ( row < 0 )
    {
        ImGui::PushID( "CBInput" );
        auto ret = ImGui::InputText( comboName.c_str(), &io_val, 0 );
        ImGui::PopID();

        return ret;
    }
    else
    {
        c_auto comboRet = ImGui::Combo(
                            comboName.c_str(),
                            &row,
                            pValsDisp.empty() ? pVals.data() : pValsDisp.data(),
                            (int)n );

        io_val = pVals[ std::min( (size_t)row, n-1 ) ];

        return comboRet;
    }
}

//==================================================================
bool IMUI_EditableCombo(
        const DStr &title,
        DStr &io_val,
        const DVec<const char *> &pVals,
        bool makeReadOnly )
{
    ImGui::PushID( title.c_str() );

    auto ret = ImGui::InputText( "##ECBInput", &io_val, 0 );

    ImGui::SameLine( 0, 0 );

    if ( ImGui::BeginCombo( (title + "##ECBCombo").c_str(), "", ImGuiComboFlags_NoPreview ) )
    {
        for (size_t i=0; i < pVals.size(); ++i)
        {
            if ( ImGui::Selectable( pVals[i], !strcmp( pVals[i], io_val.c_str() ) ) )
                io_val = pVals[i];
        }
        ImGui::EndCombo();
    }

    ImGui::PopID();

    return ret;
}

//==================================================================
bool IMUI_BeginTabItem(const char* label, bool* p_open, ImGuiTabItemFlags flags)
{
    auto ret = ImGui::BeginTabItem(label, p_open, flags);

    if ( ret )
        ImGui::NewLine();

    return ret;
}
#endif


