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
void ConfigWin::ActivateConfigWin( bool onOff )
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
    }

    mActivate = onOff;
}

//==================================================================
void ConfigWin::writeChanges()
{
    if ( XCConfig::CheckValsChange( mStoredVars, mLocalVars ) )
    {
        mStoredVars = mLocalVars;
        mHasChangedConfig = true;
    }
}

//==================================================================
void ConfigWin::DrawConfigWin()
{
    if NOT( mActivate )
        return;

    //
    IMUI_SetNextWindowSize( ImVec2(500,320), ImGuiCond_FirstUseEver );

    if NOT( ImGui::Begin( "Configuration", &mActivate,
                      ImGuiWindowFlags_NoCollapse
                    | ImGuiWindowFlags_NoDocking ) )
    {
        ImGui::End();
        return;
    }

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

    ImGui::Checkbox( "sRGB Output", &mLocalVars.cfg_dispConvToSRGB );

    IMUI_ComboText( "Tone Mapping", mLocalVars.cfg_dispToneMapping,
                    {"none"   , "filmic"},
                    {"None"   , "Filmic"},
                    false,
                    "filmic" );

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
            writeChanges();
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

