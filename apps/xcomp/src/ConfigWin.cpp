//==================================================================
/// ConfigWin.cpp
///
/// Created by Davide Pasca - 2021/09/01
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_stdlib.h"

#include "DBase.h"
#include "DLogOut.h"
#include "DThreads.h"
#include "XComp.h"
#include "DisplayBase.h"
#include "DUT_Str.h"
#include "DNETUT.h"
#include "GTVersions.h"
#include "IMUI_Utils.h"
#include "ConfigWin.h"

//==================================================================
ConfigWin::ConfigWin( XComp &bl )
    : mXComp(bl)
{
}

//
ConfigWin::~ConfigWin() = default;

//==================================================================
void ConfigWin::ActivateConfigWin( bool onOff, Tab nextOpenTab )
{
    if ( mActivate == onOff )
        return;

    if ( onOff )
    {
        // we get the local params only once in the lifetime
        //  of the application (until restart)
        if ( mIsFirstEntrySinceStart )
        {
            mStoredVars = mXComp.GetConfigXC();
            mIsFirstEntrySinceStart = false;
        }

        mLocalVars = mStoredVars;

        mNextOpenTab = nextOpenTab;
    }

    mActivate = onOff;
}

//==================================================================
void ConfigWin::UpdateConfig( const std::function<void (XCConfig&)> &fn )
{
    fn( mLocalVars );
    if NOT( mActivate )
        writeIfChanged();
}

//==================================================================
void ConfigWin::writeIfChanged()
{
    if ( XCConfig::CheckValsChange( mStoredVars, mLocalVars ) )
    {
        mStoredVars = mLocalVars;
        mHasChangedConfig = true;
    }
}

//==================================================================
void ConfigWin::drawGeneral()
{
    IMUI_DrawHeader( "Folders" );

    ImGui::InputText( "Scan Folder", &mLocalVars.cfg_scanDir );
    ImGui::InputText( "Save Folder", &mLocalVars.cfg_saveDir );

    IMUI_DrawHeader( "Controls" );

    IMUI_ComboText( "Pan Button", mLocalVars.cfg_ctrlPanButton,
                    {"left", "right"},
                    {"Left", "Right"},
                    false,
                    "left" );

    IMUI_DrawHeader( "Display" );

    ImGui::Checkbox( "Use Bilinear", &mLocalVars.cfg_dispUseBilinear );
}

//==================================================================
void ConfigWin::drawColorCorr()
{
    ImGui::NewLine();

    ImGui::Checkbox( "Apply to RGB Channels Only", &mLocalVars.cfg_ccorRGBOnly );

    ImGui::Checkbox( "sRGB Output", &mLocalVars.cfg_ccorSRGB );

    IMUI_ComboText( "Color Transform", mLocalVars.cfg_ccorXform,
                    {"none" ,
#ifdef ENABLE_OCIO
                     "ocio" ,
#endif
                     "filmic" },
                    {"None" ,
#ifdef ENABLE_OCIO
                     "OpenColorIO" ,
#endif
                     "Filmic (embedded)" },
                    false,
                    "filmic" );

#ifdef ENABLE_OCIO
    if ( mLocalVars.cfg_ccorXform == "ocio" )
    {
        //ImGui::Indent();
        if ( ImGui::InputText( "OCIO Config File", &mLocalVars.cfg_ccorOCIOCfgFName ) )
        {
        }
        //ImGui::Unindent();
    }
#endif
}

//==================================================================
void ConfigWin::drawTabs()
{
    auto makeFlags = [&]( auto matchTab )
    {
        return mNextOpenTab == matchTab
                ? ImGuiTabItemFlags_SetSelected
                : ImGuiTabItemFlags_None;
    };

    if ( ImGui::BeginTabItem( "General", 0, makeFlags(TAB_GENERAL) ) )
    {
        drawGeneral();
        ImGui::EndTabItem();
    }

    if ( ImGui::BeginTabItem( "Color Correction", 0, makeFlags(TAB_COLOR_CORR) ) )
    {
        drawColorCorr();
        ImGui::EndTabItem();
    }

    // reset the tab selection
    mNextOpenTab = TAB_NONE;

    ImGui::EndTabBar();
}

//==================================================================
void ConfigWin::DrawConfigWin()
{
    if NOT( mActivate )
        return;

    //
    IMUI_SetNextWindowSize( ImVec2(450,410), ImGuiCond_FirstUseEver );

    if NOT( ImGui::Begin( "Configuration", &mActivate,
                      ImGuiWindowFlags_NoCollapse
                    | ImGuiWindowFlags_NoDocking ) )
    {
        ImGui::End();
        return;
    }

    if ( ImGui::BeginTabBar("##tabs", ImGuiTabBarFlags_None) )
    {
        drawTabs();
    }

    //
    ImGui::NewLine();

    ImGui::BeginGroup();

    ImGui::Separator();

    IMUI_ShortNewLine();

    c_auto indent = ImGui::GetWindowWidth() * 0.05f;

    ImGui::Indent( indent );

        c_auto needsToSave = XCConfig::CheckValsChange( mStoredVars, mLocalVars );

        // have changes since we last saved ?

        c_auto disableSave = !needsToSave;

        if ( disableSave )
            IMUI_PushDisabled();

        if ( ImGui::Button( "Save" ) )
        {
            writeIfChanged();
            mActivate = false;
        }

        if ( disableSave )
            IMUI_PopDisabled();

        ImGui::SameLine();
        ImGui::SameLine();

        if ( ImGui::Button("Cancel") )
        {
            mActivate = false;
        }

    ImGui::Unindent( indent );

    ImGui::EndGroup();

    ImGui::End();
}

