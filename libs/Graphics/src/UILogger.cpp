//==================================================================
/// UILogger.cpp
///
/// Created by Davide Pasca - 2019/12/22
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#ifdef ENABLE_IMGUI
# include <mutex>

# include "imgui.h"
# include "imgui_internal.h"
#endif

#include "DLogOut.h"
#include "UILogger.h"

//==================================================================
#ifdef ENABLE_IMGUI
class UILogger::UILInternal
{
public:
    std::mutex          mMutex;

    ImGuiTextBuffer     mBuf;
    ImGuiTextFilter     mFilter;
    ImVector<int>       mLineOffsets;    // Index to lines offset.
};
#else
class UILogger::UILInternal
{
};
#endif

//==================================================================
UILogger::UILogger( bool addToSysLog )
{
#ifdef ENABLE_IMGUI
    INTE = std::make_unique<UILInternal>();
#endif
    mAutoScroll = true;
    ClearUIL();

    if ( addToSysLog )
        mLoggerID = LogAddPrinter( [this]( c_auto &str ){ AddLogStr( str ); } );
}

//==================================================================
UILogger::~UILogger()
{
    if ( mLoggerID != DNPOS )
        LogRemovePrinter( mLoggerID );
}

//==================================================================
void UILogger::clearUIL_NoLock()
{
#ifdef ENABLE_IMGUI
    INTE->mBuf.clear();
    INTE->mLineOffsets.clear();
    INTE->mLineOffsets.push_back(0);
    mLineInfos.clear();
    mPendingWrnLines = 0;
    mPendingErrLines = 0;
    mNeedScroll = true;
#endif
}

//==================================================================
void UILogger::ClearUIL()
{
#ifdef ENABLE_IMGUI
    std::lock_guard<std::mutex> lock( INTE->mMutex );

    clearUIL_NoLock();
#endif
}

//==================================================================
void UILogger::makeLineInfo( LineInfo &info, const char *pSta, const char *pEnd )
{
    c_auto *HL_ERR_STR = "ERR";
    c_auto  HL_ERR_STRLEN = 3;

    c_auto *HL_WRN_STR = "WRN";
    c_auto  HL_WRN_STRLEN = 3;

    const char *pSectionSta {};
    for (c_auto *p=pSta; p < pEnd; ++p)
    {
        if ( !pSectionSta )
        {
            if ( *p == '[' )
                pSectionSta = p + 1;
        }
        else
        if ( *p == ']' )
        {
            info.isErr = (p - pSectionSta) >= (ptrdiff_t)HL_ERR_STRLEN
                             && !memcmp( pSectionSta, HL_ERR_STR, HL_ERR_STRLEN );

            info.isWrn = (p - pSectionSta) >= (ptrdiff_t)HL_WRN_STRLEN
                             && !memcmp( pSectionSta, HL_WRN_STR, HL_WRN_STRLEN );

            if ( info.isErr || info.isWrn || (p+1) >= pEnd || p[1] != '[' )
            {
                if ( info.isErr )
                {
                    info.errSta = pSectionSta - pSta - 1;
                    info.errEnd = p - pSta + 1;
                }
                if ( info.isWrn )
                {
                    info.wrnSta = pSectionSta - pSta - 1;
                    info.wrnEnd = p - pSta + 1;
                }
                break;
            }

            pSectionSta = {};
        }
    }
}

//==================================================================
void UILogger::AddLogStr( const DStr &str )
{
#ifdef ENABLE_IMGUI
    std::lock_guard<std::mutex> lock( INTE->mMutex );

    int old_size = INTE->mBuf.size();

    c_auto *pSta = str.data();
    c_auto *pEnd = str.data() + str.size();

    INTE->mBuf.append( pSta, pEnd );

    for (int new_size = INTE->mBuf.size(); old_size < new_size; old_size++)
    {
        if (INTE->mBuf[old_size] == '\n')
        {
            INTE->mLineOffsets.push_back(old_size + 1);

            LineInfo info;

            makeLineInfo(
                info,
                INTE->mBuf.c_str() + INTE->mLineOffsets[ INTE->mLineOffsets.size()-2 ],
                INTE->mBuf.c_str() + INTE->mLineOffsets[ INTE->mLineOffsets.size()-1 ] );

            mLineInfos.push_back( info );

            mPendingWrnLines += info.isWrn ? 1 : 0;
            mPendingErrLines += info.isErr ? 1 : 0;
        }
    }

    mNeedScroll = true;
#endif
}


//==================================================================
void UILogger::SetFilterUIL( const DStr &str )
{
#ifdef ENABLE_IMGUI
    mFilterStr = str;
    ImStrncpy( INTE->mFilter.InputBuf, str.c_str(), IM_ARRAYSIZE( INTE->mFilter.InputBuf ));
    INTE->mFilter.Build();
#endif
}

//==================================================================
void UILogger::DrawUIL( void *pImFont )
{
#ifdef ENABLE_IMGUI
    std::lock_guard<std::mutex> lock( INTE->mMutex );

    const ImVec4 HL_ERR_COL( 1.0f, 0.2f, 0.2f, 1.0f );

    const ImVec4 HL_WRN_COL( 1.0f, 0.6f, 0.2f, 1.0f );

    // Options menu
    if ( ImGui::BeginPopup( "Options" ) )
    {
        if ( ImGui::RadioButton( "Show All", mFilterStr.empty() ) )
            SetFilterUIL( {} );

        if ( ImGui::RadioButton( "Show Errors", mFilterStr == "[ERR]" ) )
            SetFilterUIL( "[ERR]" );

        if ( ImGui::RadioButton( "Show Warnings", mFilterStr == "[WRN]" ) )
            SetFilterUIL( "[WRN]" );

        ImGui::Separator();

        ImGui::MenuItem( "Auto-scroll", "", &mAutoScroll );

        ImGui::EndPopup();
    }

    // Main window
    if (ImGui::Button("Options"))
        ImGui::OpenPopup("Options");
    ImGui::SameLine();
    c_auto doClear = ImGui::Button("Clear");
    ImGui::SameLine();
    c_auto doCopy = ImGui::Button("Copy");
    ImGui::SameLine();
    INTE->mFilter.Draw("Filter", -100.0f);

    ImGui::Separator();
    ImGui::BeginChild("scrolling", ImVec2(0,0), false, ImGuiWindowFlags_HorizontalScrollbar);

    if (doClear)
        clearUIL_NoLock();

    if ( INTE->mLineOffsets.Size < 2 )
    {
        ImGui::EndChild();
        return;
    }

    if ( doCopy )
        ImGui::LogToClipboard();

    //
    auto drawTextColFiltered = [&]( int line_no, c_auto *pSta, c_auto *pEnd )
    {
        auto &info = mLineInfos[ line_no ];

        if NOT( info.wasVisited )
        {
            info.wasVisited = true;
            mPendingWrnLines -= info.isWrn ? 1 : 0;
            mPendingErrLines -= info.isErr ? 1 : 0;

            mPendingWrnLines = std::max( mPendingWrnLines, 0 );
            mPendingErrLines = std::max( mPendingErrLines, 0 );
        }

        //ImGui::Text( "%i %i %i", (int)info.isWrn, (int)info.isErr, (int)info.wasVisited );
        //ImGui::SameLine();

        auto drawColored = [&]( c_auto secSta, c_auto secEnd, c_auto &col )
        {
            ImGui::TextUnformatted( pSta, pSta + secSta );
            ImGui::SameLine( 0.0f, 0.0f );
            ImGui::PushStyleColor( ImGuiCol_Text, col );
            ImGui::TextUnformatted( pSta + secSta, pSta + secEnd );
            ImGui::PopStyleColor();
            ImGui::SameLine( 0.0f, 0.0f );
            ImGui::TextUnformatted( pSta + secEnd, pEnd );
        };

        if ( info.isErr )
            drawColored( info.errSta, info.errEnd, HL_ERR_COL );
        else
        if ( info.isWrn )
            drawColored( info.wrnSta, info.wrnEnd, HL_WRN_COL );
        else
            ImGui::TextUnformatted( pSta, pEnd );
    };

    if ( pImFont )
        ImGui::PushFont( (ImFont *)pImFont );

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
    const char* buf = INTE->mBuf.begin();
    const char* buf_end = INTE->mBuf.end();
    if (INTE->mFilter.IsActive())
    {
        for (int line_no = 0; line_no < (INTE->mLineOffsets.Size-1); line_no++)
        {
            c_auto *line_start = buf + INTE->mLineOffsets[line_no];

            c_auto *line_end = (line_no + 1 < INTE->mLineOffsets.Size)
                                        ? (buf + INTE->mLineOffsets[line_no + 1] - 1)
                                        : buf_end;

            if (INTE->mFilter.PassFilter(line_start, line_end))
                drawTextColFiltered( line_no, line_start, line_end );
        }
    }
    else
    {
        ImGuiListClipper clipper;
        clipper.Begin( INTE->mLineOffsets.Size-1 );
        while (clipper.Step())
        {
            for (int line_no = clipper.DisplayStart; line_no < clipper.DisplayEnd; line_no++)
            {
                c_auto *line_start = buf + INTE->mLineOffsets[line_no];

                c_auto *line_end = (line_no + 1 < INTE->mLineOffsets.Size)
                                            ? (buf + INTE->mLineOffsets[line_no + 1] - 1)
                                            : buf_end;

                drawTextColFiltered( line_no, line_start, line_end );
            }
        }
        clipper.End();
    }
    ImGui::PopStyleVar();

    //if (mAutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
    if ( mAutoScroll && mNeedScroll && ImGui::GetScrollY() < ImGui::GetScrollMaxY() )
    {
        ImGui::SetScrollHereY(1.0f);
        mNeedScroll = false;
    }

    if ( pImFont )
        ImGui::PopFont();

    if ( doCopy )
        ImGui::LogFinish();

    ImGui::EndChild();
#endif
}

