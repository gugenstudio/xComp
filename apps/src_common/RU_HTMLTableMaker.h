//==================================================================
/// RU_HTMLTableMaker.h
///
/// Created by Davide Pasca - 2020/09/21
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#ifndef RU_HTMLTABLEMAKER_H
#define RU_HTMLTABLEMAKER_H

#include "DisplayBase.h"
#include "RU_TableMakerBase.h"

//==================================================================
class RU_HTMLTableMaker : public RU_TableMakerBase
{
    static constexpr ColorF HEAD_COL  = Display::ORANGE;
    static constexpr ColorF GREEN_COL = Display::GREEN;
    static constexpr ColorF RED_COL   = Display::RED;

    DStr    mOutStr;

public:
    RU_HTMLTableMaker(
                const RU_TableMakerBase::Params &par,
                const DStr &tableClass,
                const DStr &tableStyle )
        : RU_TableMakerBase( par )
    {
        mOutStr += "<table";

        if NOT( tableClass.empty() )
            mOutStr += " class=\"" + tableClass + "\"";

        if ( !tableStyle.empty() || mIsTableMono )
            mOutStr += " style=\""
                + tableStyle
                + (mIsTableMono ? " font-family: monospace;" : "")
                + "\"";

        mOutStr += ">";
    }

    bool IsHTMLTable() const override { return true; }

    void BeginHead() override { SetColor( HEAD_COL ); SetBold( true ); }

    void EndHead() override   { ResetColor(); SetBold( false ); }

    void NewCell( int alignX=-1, int colSpan=1 ) override
    {
        if ( mColCnt == mColsN )
        {
            mColCnt = 0;
            mOutStr += "</td>";
            mOutStr += "</tr>";
        }

        if ( mColCnt == 0 )
            mOutStr += "<tr>";
        else
            mOutStr += "</td>";

        mColCnt += colSpan;

        mOutStr += "<td";
        if ( colSpan > 1 )
            mOutStr += SSPrintFS( " colspan=\"%i\"", colSpan );
        mOutStr += makeTableTDAlign( alignX );
        mOutStr += ">";
    }

    void AddSpace( size_t cnt=1 ) override
    {
        for (size_t i=0; i < cnt; ++i)
            mOutStr += "&nbsp;";
    }

    void AddNewline() override
    {
        mOutStr += "<br/>";
    }

    void AddTextNoSp( const DStr &text ) override
    {
        mOutStr += makeStyleStr( StrReplaceAll( text, "\n", "<br/>" ) );
    }

    void AddSeparator() override
    {
        NewCell( -1, (int)mColCnt );
        mOutStr += "<hr/>";
    }

    void AddPriceUpArrow() override
    {
        c_auto saveCol = mCurCol;
        SetColor( GREEN_COL );
        AddTextNoSp( "&#8679;" );
        SetColor( saveCol );
    }

    void AddPriceDownArrow() override
    {
        c_auto saveCol = mCurCol;
        SetColor( RED_COL );
        AddTextNoSp( "&#8681;" );
        SetColor( saveCol );
    }

    const DStr &CloseAndGet()
    {
        if ( mColCnt == mColsN )
        {
            mOutStr += "</td>";
            mOutStr += "</tr>";
        }

        mOutStr += "</table>";
        return mOutStr;
    }

private:
    inline static DStr makeCSSColor( const ColorF &col )
    {
        return SSPrintFS( "#%02x%02x%02x",
                    (int)DClamp( col[0] * 255, 0.f, 255.f ),
                    (int)DClamp( col[1] * 255, 0.f, 255.f ),
                    (int)DClamp( col[2] * 255, 0.f, 255.f ) );
    }

    DStr makeStyleStr( const DStr &str )
    {
        DStr out;
        out += "<span style=\"color:" + makeCSSColor( mCurCol ) + ";";
        out += (mIsBold   ? " font-weight: bold;" : "");

        if ( (mIsMono || mIsGlobMono) && !mIsTableMono )
            out += " font-family: monospace;";

        out += " " + mExtraStyle + "\" " + ">" + str + "</span>";
        return out;
    }

    inline static const DStr &makeTableTDAlign( int alignX )
    {
        static DStr al( " align=\"left\"" );
        static DStr ar( " align=\"right\"" );
        static DStr ac( " align=\"center\"" );

        if ( alignX < 0 ) return al; else
        if ( alignX > 0 ) return ar;

        return ac;
    }
};

#endif

