//==================================================================
/// RU_TableMakerBase.h
///
/// Created by Davide Pasca - 2020/12/11
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#ifndef RU_TABLEMAKERBASE_H
#define RU_TABLEMAKERBASE_H

#include "DBase.h"
#include "ColorF.h"

//==================================================================
class RU_TableMakerBase
{
    static constexpr ColorF  DEFAULT_COL {255,255,255};

protected:
    const size_t mColsN;

    bool    mIsBold = false;
    bool    mIsMono = false;
    bool    mIsGlobMono = false;
    bool    mIsTableMono = false;
    ColorF  mCurCol = DEFAULT_COL;
    size_t  mColCnt = 0;
    DStr    mExtraStyle;

    bool    mColumnStreOnOff = false;

public:
    struct Params
    {
        size_t  tmb_colsN       {};
        bool    tmb_isMonoTable {};
    };

    RU_TableMakerBase( const Params &par )
        : mColsN(par.tmb_colsN)
        , mIsTableMono(par.tmb_isMonoTable)
    {
    }

    virtual ~RU_TableMakerBase() {}

    virtual bool IsHTMLTable() const { return false; }

    void SetColor(const ColorF &col) { mCurCol = col; }
    void ResetColor()                { mCurCol = DEFAULT_COL; }
    bool IsDefaultColor() const      { return mCurCol == DEFAULT_COL; }
    void SetBold( bool onOff )       { mIsBold = onOff; }
    void SetMono( bool onOff )       { mIsMono = onOff; }
    void SetGlobalMono( bool onOff ) { mIsGlobMono = onOff; }
    void SetExtraStyle( const DStr &style={} ) { mExtraStyle = style; }

    void SetColumnStretch( bool onOff ) { mColumnStreOnOff = onOff; }

    void AddText( const ColorF &col, const DStr &text ) { AddTextNoSp( text, col ); }
    void AddText( const DStr &text, const ColorF &col ) { AddTextNoSp( text, col ); }
    void AddText( const DStr &text )                    { AddTextNoSp( text ); }
    virtual void AddTooltip( const DStr &/*text*/ )     { }

    void AddTextNoSp( const DStr &text, const ColorF &col )
    {
        c_auto saveCol = mCurCol;
        mCurCol = col;
        AddTextNoSp( text );
        mCurCol = saveCol;
    }

    virtual void BeginHead() {}
    virtual void EndHead() {}

    virtual void NewCell( int alignX=-1, int colSpan=1 )
    {
        DUNREFPARAM(alignX);
        DUNREFPARAM(colSpan);
    }

    virtual void AddSpace( size_t cnt=1 ) {}
    virtual void AddHalfSpace() {};
    virtual void AddNewline() {}

    virtual void AddTextNoSp( const DStr &/*text*/ ) {}

    virtual void AddSeparator() {}

    virtual void AddPriceUpArrow() {}
    virtual void AddPriceDownArrow() {}
};

#endif

