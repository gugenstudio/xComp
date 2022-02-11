//==================================================================
/// RU_IMUITableMaker.h
///
/// Created by Davide Pasca - 2020/12/11
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#ifndef RU_IMUITABLEMAKER_H
#define RU_IMUITABLEMAKER_H

#ifdef ENABLE_IMGUI
#include "imgui.h"

#include "IMUI_Utils.h"
#include "DisplayBase.h"
#include "RU_TableMakerBase.h"

//==================================================================
class RU_IMUITableMaker : public RU_TableMakerBase
{
    bool    mIsTableActive {};
    bool    mIsInHeader {};
    bool    mIsRightAlign {};
    bool    mIsFirstTextInCell {};

public:
    RU_IMUITableMaker(
            const char *pName,
            size_t colsN,
            bool fitSize,
            int addImFlags=0,
            int remImFlags=0 )
        : RU_TableMakerBase( RU_TableMakerBase::Params {
                                .tmb_colsN = colsN,
                                .tmb_isMonoTable = false } )
    {
        c_auto flags =
            (
              ImGuiTableFlags_SizingFixedFit
            | ImGuiTableFlags_NoSavedSettings
            | ImGuiTableFlags_RowBg
            | ImGuiTableFlags_BordersOuter
            | ImGuiTableFlags_Borders
            | ImGuiTableFlags_Hideable
            | ImGuiTableFlags_Reorderable
            | ImGuiTableFlags_Resizable
            | ImGuiTableFlags_ContextMenuInBody
            | (fitSize ?  ImGuiTableFlags_ScrollY
                        | ImGuiTableFlags_NoHostExtendY
                       :  0)
            | addImFlags) & ~remImFlags;

        mIsTableActive = ImGui::BeginTable( pName, (int)colsN, flags, ImVec2(0,0) );
    }

    ~RU_IMUITableMaker()
    {
        if NOT( mIsTableActive )
            return;

        ImGui::EndTable();
    }

    void BeginHead() override
    {
        if NOT( mIsTableActive )
            return;

        mIsInHeader = true;
        ImGui::TableSetupScrollFreeze( 0, 1 );
    }

    void EndHead() override
    {
        if NOT( mIsTableActive )
            return;

        mIsInHeader = false;
        ImGui::TableHeadersRow();
    }

    void NewCell( int alignX=-1, int colSpan=1 ) override
    {
        DUNREFPARAM(colSpan);

        if NOT( mIsTableActive )
            return;

        if ( mIsInHeader )
            return;

        mIsRightAlign = (alignX == 1);

        ImGui::TableNextColumn();

        mIsFirstTextInCell = true;
    }

    void AddSpace( size_t cnt=1 ) override
    {
        if NOT( mIsTableActive )
            return;

        IMUI_SameLineN( cnt );
        mIsFirstTextInCell = false;
    }

    void AddHalfSpace() override
    {
        if NOT( mIsTableActive )
            return;

        IMUI_ShortSameLine();
        mIsFirstTextInCell = false;
    }

    void AddNewline() override
    {
        if NOT( mIsTableActive )
            return;

        mIsFirstTextInCell = false;
    }

    void AddTextNoSp( const DStr &text ) override
    {
        if NOT( mIsTableActive )
            return;

        if ( mIsInHeader )
        {
            ImGui::TableSetupColumn(
                        text.c_str(),
                        mColumnStreOnOff
                            ? ImGuiTableColumnFlags_WidthStretch
                            : ImGuiTableColumnFlags_WidthFixed );
        }
        else
        {
            if ( mIsFirstTextInCell )
                mIsFirstTextInCell = false;
            //else
            //    ImGui::SameLine();

            if NOT( IsDefaultColor() )
                IMUI_PushTextCol( mCurCol );

            if ( mIsRightAlign )
                IMUI_TextRightAligned( text );
            else
                IMUI_Text( text );

            if NOT( IsDefaultColor() )
                IMUI_PopTextColor();
        }
    }

    void AddTooltip( const DStr &text ) override
    {
        IMUI_MakeTooltip( text );
    }

    void AddSeparator() override
    {
        ImGui::Separator();
    }

    void AddPriceUpArrow() override
    {
        IMUI_DrawPriceArrow( 20, 10 );
    }

    void AddPriceDownArrow() override
    {
        IMUI_DrawPriceArrow( 10, 20 );
    }

    void AddToolTip( const DStr &text )
    {
        IMUI_MakeTooltip( text );
    }
};

#endif

#endif

