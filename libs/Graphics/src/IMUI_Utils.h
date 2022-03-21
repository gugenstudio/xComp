//==================================================================
/// IMUI_Utils.h
///
/// Created by Davide Pasca - 2019/12/24
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#ifndef IMUI_UTILS_H
#define IMUI_UTILS_H

#include "DBase.h"
#include "DMathBase.h"
#include "ColorF.h"
#include "StringUtils.h"

//==================================================================
class IMUI_StyledTextMaker
{
    DStr    mStr;

public:
    auto &AddText( const DStr &str ) { mStr += str; return *this; }

    auto &AddSpace()                 { mStr += ' '; return *this; }

    auto &SetColor( const ColorF &col )
    {
        mStr += SSPrintFS( "{%02x%02x%02x}",
                    DClamp( (int)(col[0]*255), 0, 255 ),
                    DClamp( (int)(col[1]*255), 0, 255 ),
                    DClamp( (int)(col[2]*255), 0, 255 ) );
        return *this;
    }

    auto &ResetColor() { mStr += "{}"; return *this; }

    DStr GetEncoded() const { return mStr; }
};

#if defined(ENABLE_IMGUI)

#include "imgui.h"

//==================================================================

void IMUI_SetContentScale( float sca );
float IMUI_GetContentScale();

void IMUI_SetIsRetinaDisplay( bool isRetina );
bool IMUI_GetIsRetinaDisplay();

bool IMUI_IsLightMode();

inline ImVec4 IMUI_MkCol( const ColorF &srcCol )
{
    return ImVec4( srcCol[0], srcCol[1], srcCol[2], srcCol[3] );
}

ImVec4 IMUI_MkTextCol( const ImVec4 &col );
ColorF IMUI_MkTextCol( const ColorF &col );

ColorF IMUI_MakeColForBG( const ColorF &col, float alpha );

void IMUI_SetNextWindowSize( const ImVec2& size, ImGuiCond cond = 0 );

void IMUI_SetNextItemWidth( float w );
void IMUI_PushItemWidth( float w );

void IMUI_SetColumnWidth( int colIdx, float w );

void IMUI_Indent( float w );
void IMUI_Unindent( float w );

// version with no formatting
void IMUI_Text( const char *pTxt );
void IMUI_Text( const DStr &txt );
void IMUI_TextRightAligned( const DStr &text );
void IMUI_TextWrappedDisabled( const DStr &text );

void IMUI_DrawHeader( const DStr &str, bool topPadding=true );

//==================================================================
void IMUI_MakeTooltip( const DStr &str );
void IMUI_HelpMarker( const DStr &str );

void IMUI_PushButtonColors( const ColorF &col );
void IMUI_PopButtonColors();

bool IMUI_TextURL(
            const char* name_,
            bool sameLineBefore_=false,
            bool sameLineAfter_=false );

void IMUI_DrawLink(
            const DStr &fullURL,
            const DStr &dispURL,
            bool useSmall=false,
            const ColorF &col={0,0,0,0} );

void IMUI_DrawLinkOpenPopup(
            const DStr &fullURL,
            const DStr &dispURL,
            bool useSmall=false,
            const ColorF &col={0,0,0,0} );

void IMUI_DrawLinkPopup( const DStr &fullURL );

void IMUI_DrawLinkPopup(
        const DStr &nameIDStr,
        const DFun<DStr ()> &getFullURL1Fn,
        const DFun<DStr ()> &getFullURL2Fn={},
        const DStr &desc1Str={},
        const DStr &desc2Str={} );

void IMUI_SameLine( float offX=0, float spacingW=-1 );

inline void IMUI_ShortSameLine() { IMUI_SameLine( 0, 4 ); }
inline void IMUI_LongSameLine()  { IMUI_SameLine( 0, 16 ); }
void IMUI_SameLineN( size_t n );

void IMUI_Spacing( float offX, float offY );

void IMUI_ShortNewLine( float scale=0.5f );

void IMUI_PushDisabled();
void IMUI_PopDisabled();

void IMUI_PushDisabledStyle();
void IMUI_PopDisabledStyle();

void IMUI_PushFlashing( const ImVec4 &col );
inline void IMUI_PushFlashing( const ColorF &col ) { IMUI_PushFlashing( IMUI_MkCol( col ) ); }
void IMUI_PopFlashing();

void IMUI_PushStyleColor( ImGuiCol colIdx, const ImVec4 &col );
void IMUI_PopStyleColor( int count=1 );
inline void IMUI_PushStyleColor( ImGuiCol colIdx, const ColorF &col )
{
    IMUI_PushStyleColor( colIdx, IMUI_MkCol( col ) );
}

void IMUI_PushTextCol( const ImVec4 &col );
inline void IMUI_PushTextCol( const ColorF &col ) { IMUI_PushTextCol( IMUI_MkCol( col ) ); }
void IMUI_PopTextColor();

ImVec2 IMUI_GetWindowInnerClipRect();
float IMUI_GetWindowContentRegionWidth();
float IMUI_GetWindowContentRegionHeight();

float IMUI_CalcItemsHeight( size_t textsN, size_t widgetsN, size_t sepsN );

void IMUI_DrawArrow( ImGuiDir dir, float scale=1.0f );
void IMUI_DrawPriceArrow( double curVal, double prevVal );

bool IMUI_BeginChildCornerOverlay( int corner = 0 );
bool IMUI_BeginChildMouseOverlay();

float IMUI_CalcHeightForChildWithBottom( size_t textsN, size_t widgetsN, size_t sepsN );

bool IMUI_BeginChildWithBottom(
                    const char *pName,
                    size_t textsN,
                    size_t widgetsN,
                    size_t sepsN );

//==================================================================
inline void IMUI_TextColored( const ColorF &col, const char *pTxt )
{
    IMUI_PushTextCol( col );
    IMUI_Text( pTxt );
    IMUI_PopTextColor();
}

inline void IMUI_TextColored( const ColorF &col, const DStr &txt )
{
    IMUI_PushTextCol( col );
    IMUI_Text( txt );
    IMUI_PopTextColor();
}

//==================================================================
inline bool IMUI_NegCheckbox( const char *pLabel, bool *pNegatedVal )
{
    auto tmp = !*pNegatedVal;   // negate to go in
    c_auto hasChanged = ImGui::Checkbox( pLabel, &tmp );
    *pNegatedVal = !tmp;        // negate back the output

    return hasChanged;
}

//==================================================================
bool IMUI_ButtonEnabled(
        const char *pLabel,
        bool doEnable,
        const ImVec2 &siz={0,0},
        const ColorF &col={0,0,0,0} );

//
inline bool IMUI_ButtonEnabled(
        const DStr &label,
        bool doEnable,
        const ImVec2 &siz={0,0},
        const ColorF &col={0,0,0,0} )
{
    return IMUI_ButtonEnabled( label.c_str(), doEnable, siz, col );
}

//
bool IMUI_SmallButtonEnabled( const char *pLabel, bool doEnable, const ImVec2 siz={0,0} );

float IMUI_CalcMainStatusBarHeight();
bool IMUI_BeginMainStatusBar();
void IMUI_EndMainStatusBar();

//==================================================================
bool IMUI_InputDoubleDispPct(
                        const char *pID,
                        double *pVal,
                        double major,
                        const char *pFmt,
                        double minPct=0,
                        double maxPct=100 );

//==================================================================
void IMUI_TextWithColors( const DStr &str );
inline void IMUI_TextWithColors( IMUI_StyledTextMaker &mak )
{
    IMUI_TextWithColors( mak.GetEncoded() );
}

//==================================================================
class IMUI_ModalDialog
{
    DStr    mDialogStyText;
    bool    mDoOpen {};

public:
    void OpenDialog( const DStr &txt );
    void DrawDialog( const DStr &title );
};

//
bool IMUI_ComboText(
            const DStr &comboName,
            DStr &io_val,
            const DVec<const char *> &pVals,
            const DVec<const char *> &pValsDisp={},
            bool makeReadOnly={},
            const char *pDefVal={} );

bool IMUI_EditableCombo(
        const DStr &title,
        DStr &io_val,
        const DVec<const char *> &pVals,
        bool makeReadOnly=false );

bool IMUI_BeginTabItem(const char* label, bool* p_open, ImGuiTabItemFlags flags);

#include "IMUI_InputTextNumFields.h"

#endif

#endif

