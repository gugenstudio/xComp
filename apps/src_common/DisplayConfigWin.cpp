//==================================================================
/// DisplayConfigWin.cpp
///
/// Created by Davide Pasca - 2020/11/08
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#ifdef ENABLE_IMGUI

#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_stdlib.h"

#include "DBase.h"
#include "DLogOut.h"
#include "DisplayBase.h"
#include "GraphicsApp.h"
#include "IMUI_Utils.h"
#include "DisplayConfigWin.h"

//==================================================================
DisplayConfigWin::DisplayConfigWin( AppBaseConfig &appBaseConfig )
    : mAppBaseConfig(appBaseConfig)
{
}

//
DisplayConfigWin::~DisplayConfigWin() = default;

//==================================================================
static void copyConfig( AppBaseConfig &a, const AppBaseConfig &b )
{
    a.mData.color_scheme            = b.mData.color_scheme              ;
    a.mData.save_win_layout         = b.mData.save_win_layout           ;
    a.mData.content_scale           = b.mData.content_scale             ;
    a.mData.enable_multi_viewports  = b.mData.enable_multi_viewports    ;
}

//==================================================================
static bool isConfigChanged( const AppBaseConfig &a, const AppBaseConfig &b )
{
    return
           a.mData.color_scheme             != b.mData.color_scheme
        || a.mData.save_win_layout          != b.mData.save_win_layout
        || a.mData.content_scale            != b.mData.content_scale
        || a.mData.enable_multi_viewports   != b.mData.enable_multi_viewports
        ;
}

//==================================================================
void DisplayConfigWin::ActivateDisplayConfigWin( bool onOff, Tab nextOpenTab )
{
    if ( mActivate == onOff )
        return;

    if ( onOff )
        copyConfig( mEditConfig, mAppBaseConfig );

    mActivate = onOff;
}

//==================================================================
static auto makeInputPct = [](
                    c_auto *pLabel,
                    auto *pVal,
                    auto defVal,
                    auto mi,
                    auto ma )
{
    IMUI_SetNextItemWidth( 150 );

    auto tmpPct = *pVal * 100;
    if ( ImGui::InputDouble( pLabel, &tmpPct, 1.0, 5.0, "%.0f%%", 0) )
        *pVal = DClamp( tmpPct / 100, mi, ma );

    if ( *pVal != defVal )
    {
        ImGui::SameLine();

        ImGui::PushID( pLabel );
        if ( ImGui::Button( "Reset" ) )
            *pVal = defVal;

        ImGui::PopID();
    }
};

//==================================================================
void DisplayConfigWin::DrawDisplayConfigWin( GraphicsApp *pApp )
{
#ifdef ENABLE_IMGUI
    if NOT( mActivate )
        return;

    //
    IMUI_SetNextWindowSize( ImVec2(450,350), ImGuiCond_FirstUseEver );

    if NOT( ImGui::Begin( "Display Configuration", &mActivate,
                      ImGuiWindowFlags_NoCollapse
                    | ImGuiWindowFlags_NoDocking ) )
    {
        ImGui::End();
        return;
    }

#if 0
    ImGui::Text( "Configs" );
#endif

    // shorthand
    //auto &pars = mLocalLTPars;
    ImGui::NewLine();

    if ( IMUI_ComboText( "Color Scheme", mEditConfig.mData.color_scheme,
                    {"dark", "classic", "light"},
                    {"Dark", "Classic", "Light"},
                    false,
                    "dark" ) )
    {
        pApp->ChangeColorScheme( mEditConfig.mData.color_scheme );
    }

    ImGui::NewLine();

    IMUI_PushDisabledStyle();
    IMUI_Text( "Options below will have effect at restart." );
    IMUI_PopDisabledStyle();

    makeInputPct(
        "Content Scale",
        &mEditConfig.mData.content_scale,
        ABConfigData::DEF_CONT_SCALE,
        0.50,
        2.00 );

    ImGui::Checkbox( "Save Panels Layout",     &mEditConfig.mData.save_win_layout );
#ifdef DEBUG
    ImGui::Checkbox( "Enable Multi-Viewports", &mEditConfig.mData.enable_multi_viewports );
#endif

    ImGui::NewLine();

    ImGui::BeginGroup();

    ImGui::Separator();

    IMUI_ShortNewLine();

    c_auto indent = ImGui::GetWindowWidth() * 0.05f;

    ImGui::Indent( indent );

        //
        c_auto needsToSave = isConfigChanged( mEditConfig, mAppBaseConfig );

        if NOT( needsToSave )
            IMUI_PushDisabled();

        if ( ImGui::Button( "Save" ) )
        {
            copyConfig( mAppBaseConfig, mEditConfig );
            mAppBaseConfig.SaveAppBaseConfig();

            mActivate = false;
        }

        if NOT( needsToSave )
            IMUI_PopDisabled();

        ImGui::SameLine();

        if ( ImGui::Button("Cancel") )
        {
            pApp->ChangeColorScheme( mAppBaseConfig.mData.color_scheme );
            mActivate = false;
        }

    ImGui::Unindent( indent );

    ImGui::EndGroup();

    ImGui::End();
#endif
}

#endif

