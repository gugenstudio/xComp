//==================================================================
/// UILogger.h
///
/// Created by Davide Pasca - 2019/12/22
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#ifndef UILOGGER_H
#define UILOGGER_H

#include "DBase.h"
#include "DContainers.h"

//==================================================================
class UILogger
{
    class UILInternal;

    uptr<UILInternal>   INTE;

    struct LineInfo
    {
        bool    isErr {};
        bool    isWrn {};
        size_t  errSta {};
        size_t  errEnd {};
        size_t  wrnSta {};
        size_t  wrnEnd {};
        bool    wasVisited {};
    };

    int                 mPendingWrnLines {};
    int                 mPendingErrLines {};
    DVec<LineInfo>      mLineInfos;

    bool                mAutoScroll;     // Keep scrolling if already at the bottom
    bool                mNeedScroll = false;

    size_t              mLoggerID = DNPOS;

    DStr                mFilterStr;

public:
    UILogger( bool addToSysLog );
    ~UILogger();

    void ClearUIL();
    void AddLogStr( const DStr &str );
    void SetFilterUIL( const DStr &str );
    c_auto &GetFilterUIL() const { return mFilterStr; }
    void DrawUIL( void *pImFont );

    size_t GetPendingWrnLinesN() const { return (size_t)mPendingWrnLines; }
    size_t GetPendingErrLinesN() const { return (size_t)mPendingErrLines; }

private:
    void clearUIL_NoLock();
    static void makeLineInfo( LineInfo &info, const char *pSta, const char *pEnd );
};

#endif

