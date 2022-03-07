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

static const auto CWIN_PREFERRED_CSPACE = DStr("Filmic Log");

//==================================================================
ConfigWin::ConfigWin( XComp &bl )
    : mXComp(bl)
{
}

//
ConfigWin::~ConfigWin() = default;

//==================================================================
void ConfigWin::updateOnLocalChange()
{
#ifdef ENABLE_OCIO
    // select a default;
    auto pickDefCSpace = []( c_auto &vec )
    {
        return
            vec.end() != std::find( vec.begin(), vec.end(), CWIN_PREFERRED_CSPACE )
                //  use the preferred value if it exists
                ? CWIN_PREFERRED_CSPACE
                // otherwise get the first available color space,
                //  otherwise just use a standard one
                : (!vec.empty() ? vec.front() : OCIO::ROLE_SCENE_LINEAR);
    };

    if (c_auto &fname = mLocalVars.cfg_ccorOCIOCfgFName; FU_FileExists( fname ) )
    {
        mLocalOCIO.UpdateConfigOCIO( fname );

        c_auto &cspaces = mLocalOCIO.GetColorSpaces();
        if ( cspaces.end() == std::find(
                                cspaces.begin(),
                                cspaces.end(),
                                mLocalVars.cfg_ccorOCIOCSpace ) )
        {
            mLocalVars.cfg_ccorOCIOCSpace = pickDefCSpace( cspaces );
        }
    }
    else
    {
        mLocalVars.cfg_ccorOCIOCSpace = {};
    }
#endif
}

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
#ifdef ENABLE_OCIO
        mLocalOCIO = mStoredOCIO;
#endif
        updateOnLocalChange();

        mNextOpenTab = nextOpenTab;
    }

    mActivate = onOff;
}

//==================================================================
void ConfigWin::UpdateConfig( const std::function<void (XCConfig&)> &fn )
{
    fn( mLocalVars );
    updateOnLocalChange();

    // don't store it as a permament change yet if we're not
    //  working in the "immediate apply" mode
#ifndef CONFIG_WIN_IMMEDIATE_APPLY
    if NOT( mActivate )
#endif
    {
        storeIfChanged();
    }
}

//==================================================================
void ConfigWin::storeIfChanged()
{
    if ( XCConfig::CheckValsChange( mStoredVars, mLocalVars ) )
    {
        mStoredVars = mLocalVars;
#ifdef ENABLE_OCIO
        mStoredOCIO = mLocalOCIO;
#endif
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

    ImGui::Indent();
#ifdef ENABLE_OCIO
    if ( mLocalVars.cfg_ccorXform == "ocio" )
    {
        if ( ImGui::InputText( "OCIO Config File", &mLocalVars.cfg_ccorOCIOCfgFName ) )
            updateOnLocalChange();

        c_auto &cspaces = mLocalOCIO.GetColorSpaces();
        DVec<const char *> pList( cspaces.size() );
        for (size_t i=0; i < cspaces.size(); ++i)
            pList[i] = cspaces[i].c_str();

        if NOT( pList.empty() )
            IMUI_ComboText( "Color Space", mLocalVars.cfg_ccorOCIOCSpace, pList );
        else
            IMUI_TextWrappedDisabled( "No available color spaces" );
    }
    else
#endif
    {
        ImGui::Checkbox( "sRGB Output", &mLocalVars.cfg_ccorSRGB );
    }
    ImGui::Unindent();
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

#ifdef CONFIG_WIN_IMMEDIATE_APPLY
//==================================================================
void ConfigWin::drawClose()
{
    // give a bit of latency before applying the change
    if ( mApplyChangesTE.CheckTimedEvent( GetEpochTimeUS() ) )
        storeIfChanged();

    if ( ImGui::Button("Close") )
        mActivate = false;
}
#else
//==================================================================
void ConfigWin::drawSaveCancel()
{
    c_auto needsToSave = XCConfig::CheckValsChange( mStoredVars, mLocalVars );

    // have changes since we last saved ?

    c_auto disableSave = !needsToSave;

    if ( disableSave )
        IMUI_PushDisabled();

    if ( ImGui::Button( "Save" ) )
    {
        storeIfChanged();
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
}
#endif

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
    {
        ImGui::NewLine();

        ImGui::BeginGroup();

        ImGui::Separator();

        IMUI_ShortNewLine();

        c_auto indent = ImGui::GetWindowWidth() * 0.05f;
        ImGui::Indent( indent );

#ifdef CONFIG_WIN_IMMEDIATE_APPLY
        drawClose();
#else
        drawSaveCancel();
#endif
        ImGui::Unindent( indent );

        ImGui::EndGroup();
    }

    ImGui::End();
}

