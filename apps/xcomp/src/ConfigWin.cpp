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
static const auto CWIN_PREFERRED_LOOK   = DStr("Medium Contrast");

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
    // utility function
    auto isContained = []( c_auto &val, c_auto &cont )
    {
        return cont.end() != std::find( cont.begin(), cont.end(), val );
    };

    // select a default;
    auto pickDefCSpace = [&]( c_auto &vec )
    {
        return isContained( CWIN_PREFERRED_CSPACE, vec )
                //  use the preferred value if it exists
                ? CWIN_PREFERRED_CSPACE
                // otherwise get the first available color space,
                //  otherwise just use a standard one
                : (!vec.empty() ? vec.front() : OCIO::ROLE_SCENE_LINEAR);
    };

    auto pickDefLook = [&]( c_auto &vec )
    {
        return isContained( CWIN_PREFERRED_LOOK, vec )
                //  use the preferred value if it exists
                ? CWIN_PREFERRED_LOOK
                // otherwise get the first available color space,
                //  otherwise just nothing
                : (!vec.empty() ? vec.front() : DStr());
    };

    auto &locIMSC = mLocalVars.cfg_imsConfig;

    if (c_auto &fname = locIMSC.imsc_ccorOCIOCfgFName; FU_FileExists( fname ) )
    {
        // update the config
        mLocalOCIO.UpdateConfigOCIO( fname );

        // default color space, if the selected one isn't available
        if (c_auto &names = mLocalOCIO.GetColorSpaces();
                    !isContained( locIMSC.imsc_ccorOCIOCSpace, names ))
        {
            locIMSC.imsc_ccorOCIOCSpace = pickDefCSpace( names );
        }

        // default look, if the selected one isn't available
        if (c_auto &names = mLocalOCIO.GetLooks();
                    !isContained( locIMSC.imsc_ccorOCIOLook, names ))
        {
            locIMSC.imsc_ccorOCIOLook = pickDefLook( names );
        }
    }
    else
    {
        locIMSC.imsc_ccorOCIOCSpace = {};
        locIMSC.imsc_ccorOCIOLook = {};
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
    auto &locIMSC = mLocalVars.cfg_imsConfig;

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

    ImGui::Checkbox( "Use Bilinear", &locIMSC.imsc_useBilinear );
}

//==================================================================
void ConfigWin::drawColorCorr()
{
    auto &locIMSC = mLocalVars.cfg_imsConfig;

    ImGui::NewLine();

    ImGui::Checkbox( "Apply to RGB Channels Only", &locIMSC.imsc_ccorRGBOnly );

    IMUI_ComboText( "Color Transform", locIMSC.imsc_ccorXform,
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
    if ( locIMSC.imsc_ccorXform == "ocio" )
    {
        if ( ImGui::InputText( "OCIO Config File", &locIMSC.imsc_ccorOCIOCfgFName ) )
            updateOnLocalChange();

        auto makeCStrList = [&,this]( c_auto &src )
        {
            DVec<const char *> pList( src.size() );
            for (size_t i=0; i < src.size(); ++i)
                pList[i] = src[i].c_str();

            return pList;
        };

        if (c_auto pList = makeCStrList( mLocalOCIO.GetColorSpaces() ); !pList.empty() )
            IMUI_ComboText( "Color Space", locIMSC.imsc_ccorOCIOCSpace, pList );
        else
            IMUI_TextWrappedDisabled( "No available color spaces" );

        if (c_auto pList = makeCStrList( mLocalOCIO.GetLooks() ); !pList.empty() )
            IMUI_ComboText( "Looks", locIMSC.imsc_ccorOCIOLook, pList );
        else
            IMUI_TextWrappedDisabled( "No available looks" );
    }
    else
#endif
    {
        ImGui::Checkbox( "sRGB Output", &locIMSC.imsc_ccorSRGB );
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

