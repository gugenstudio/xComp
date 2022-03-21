//==================================================================
/// IMUI_AboutDialog.cpp
///
/// Created by Davide Pasca - 2019/12/29
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#include "IMUI_AboutDialog.h"

#ifdef ENABLE_IMGUI

#include "imgui.h"
#include "FileUtils.h"
#include "IMUI_Utils.h"

//==================================================================
static void linkLineBegin()
{
    ImGui::AlignTextToFramePadding();
}

//==================================================================
static void displayLicenses( bool showSmall )
{
    static const char *pList[][3] =
    {
         { "C++ REST SDK"       ,"Microsoft"                        ,"MIT"           }
        ,{ "Dear ImGui"         ,"Omar Cornut"                      ,"MIT"           }
        ,{ "ImPlot"             ,"Evan Pezent"                      ,"MIT"           }
#ifdef ENABLE_IMGUITEXINSPECT
        ,{ "imgui_tex_inspect"  ,"Andy Borrell"                     ,"MIT"           }
#endif
        ,{ "GLEW"               ,"multiple"                         ,"MIT"           }
        ,{ "GLFW"               ,"Marcus Geelnard, Camilla Löwy"    ,"zlib/libpng"   }
        ,{ "zlib"               ,"Jean-loup Gailly, Mark Adler"     ,"zlib"          }
        ,{ "libpng"             ,"multiple"                         ,"libpng"        }
        ,{ "OpenEXR"            ,"Academy Software Foundation"      ,"Modified BSD"  }
        ,{ "OpenColorIO"        ,"Academy Software Foundation"      ,"BSD-3-Clause"  } };

    if ( showSmall )
    {
        if NOT( ImGui::CollapsingHeader( "Open Source components" , 0 ) )
        {
            ImGui::Separator();
            return;
        }
    }
    else
    {
        IMUI_Text( "Open Source components:" );
        ImGui::Separator();
    }

    ImGui::Columns( 3 );

    static float lastW;
    if ( lastW != ImGui::GetWindowWidth() )
    {
        lastW = ImGui::GetWindowWidth();
        ImGui::SetColumnWidth( 0, lastW * 0.25f );
        ImGui::SetColumnWidth( 1, lastW * 0.50f );
        ImGui::SetColumnWidth( 2, lastW * 0.25f );
    }

    for (size_t i=0; i < std::size(pList); ++i)
    {
        IMUI_Text( pList[i][0] ); ImGui::NextColumn();
        IMUI_Text( pList[i][1] ); ImGui::NextColumn();
        IMUI_Text( pList[i][2] ); ImGui::NextColumn();
    }

    ImGui::Columns( 1 );
    ImGui::Separator();
}

//==================================================================
void IMUI_AboutDialog( const IMUI_AboutDialogParams &par )
{
    IMUI_SetNextWindowSize(
            par.showSmall
                ? ImVec2(420,330)
                : ImVec2(520,440),
            ImGuiCond_Always );

    if NOT( ImGui::Begin(
                ("About " + par.appName).c_str(),
                par.pOpen,
                  ImGuiWindowFlags_NoDocking
                | ImGuiWindowFlags_NoResize
                | ImGuiWindowFlags_NoCollapse
                //| ImGuiWindowFlags_AlwaysAutoResize
                | ImGuiWindowFlags_NoSavedSettings
                | ImGuiWindowFlags_NoNav ) )
    {
        ImGui::End();
        return;
    }

    IMUI_DrawHeader( par.appLongName + " " + par.appVersion );

    IMUI_Text( par.copyrightText );

#if 0
    ImGui::NewLine();
    ImGui::TextWrapped(
"This is a testing edition of the software.\n"
"No redistribution is allowed, unless explicity approved by NEWTYPE K.K." );
#endif

    if ( FU_CanOpenURL() )
    {
        ImGui::Text( "Go to" );
        ImGui::SameLine();

        if ( IMUI_TextURL( (par.dispURL.empty() ? par.fullURL : par.dispURL).c_str() ) )
        {
            FU_OpenURL( par.fullURL );
        }

        ImGui::SameLine();
        ImGui::Text( "for more information." );
    }

    if NOT( par.creditsLine.empty() )
        IMUI_Text( par.creditsLine );

    ImGui::NewLine();

    displayLicenses( par.showSmall );

    if NOT( par.showSmall )
        ImGui::NewLine();

    ImGui::TextDisabled( "(built on %u/%u/%u %u:%02u)",
                            par.val_Year ,
                            par.val_Month,
                            par.val_Day  ,
                            par.val_Hours,
                            par.val_Mins );

    //ImGui::SetNextItemWidth( 300 );
    if ( ImGui::Button( "OK", {-1.f,0.f} ) )
    {
        if ( par.pOpen )
            *par.pOpen = false;
    }

    ImGui::End();
}

#endif

