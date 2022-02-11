//==================================================================
/// IMUI_EditableListBox.cpp
///
/// Created by Davide Pasca - 2021/09/06
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#ifdef ENABLE_IMGUI

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include "imgui_stdlib.h"
#include "imgui_internal.h"

#include "IMUI_Utils.h"
#include "IMUI_EditableListBox.h"

//==================================================================
void IMUI_EditableListBox(
        const char *pTitle,
        DStr &edit, DVec<DStr> &vals,
        const DFun<bool (DStr&)> &onEditFn )
{
    ImGui::PushID( pTitle );

    size_t curSelIdx = DNPOS;
    {
        auto it = std::find( vals.begin(), vals.end(), edit );
        curSelIdx = (it != vals.end() ? (size_t)(it - vals.begin()) : DNPOS);
    }

    c_auto listH = DClamp( (int)vals.size(), 2, 8 ) * ImGui::GetTextLineHeightWithSpacing();

    if NOT( StrStartsWithI( pTitle, "##" ) )
        IMUI_Text( pTitle );

    if ( ImGui::BeginListBox( ("##" + DStr(pTitle)).c_str(), {-FLT_MIN, listH} ) )
    {
        for (size_t i=0; i < vals.size(); ++i)
        {
            if ( ImGui::Selectable( vals[i].c_str(), (i == curSelIdx) ) )
            {
                if ( curSelIdx == i )
                {
                    curSelIdx = DNPOS;
                    edit = {};
                }
                else
                {
                    curSelIdx = i;
                    edit = vals[i];
                }
            }

            // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
            if ( i == curSelIdx )
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndListBox();
    }

    {
        c_auto wmax = ImGui::CalcItemWidth();

        ImGui::SetNextItemWidth( wmax * 0.7f );
        if ( ImGui::InputText( "##Edit", &edit, 0 ) && onEditFn )
            onEditFn( edit );

        IMUI_ShortSameLine();

        ImGui::SetNextItemWidth( wmax * 0.3f );
        if ( IMUI_ButtonEnabled( "Add", curSelIdx == DNPOS && !edit.empty() ) )
        {
            if ( !onEditFn || onEditFn( edit ) )
            {
                vals.push_back( edit );
                edit = {};
                std::sort( vals.begin(), vals.end() );
            }
        }
        IMUI_ShortSameLine();

        ImGui::SetNextItemWidth( wmax * 0.3f );
        if ( IMUI_ButtonEnabled( "Remove", curSelIdx != DNPOS ) )
        {
            vals.erase( vals.begin() + curSelIdx );
            std::sort( vals.begin(), vals.end() );
            edit = {};
        }
    }

    ImGui::PopID();
}

#endif

