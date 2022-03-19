//==================================================================
/// XComp.h
///
/// Created by Davide Pasca - 2020/02/27
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#ifndef XCOMP_H
#define XCOMP_H

#include <array>
#include "DBase.h"
#include "XCConfig.h"
#include "AppBaseConfig.h"
#include "TimeUtils.h"

class GraphicsApp;
class Graphics;

class XCompUI;
class MarketStatsDB;

//==================================================================
struct XCompParams
{
    DStr    mAppExeDir;
    DStr    mAppCacheDir;
    DStr    mAppUserProfDir;
    DStr    mAppUserProfDisplay;
    bool    mIsNoUIMode = false;
    bool    mIsNoFrameSkipUI = false;
    DStr    mConfigPathFName;
};

class ImageSystem;

//==================================================================
class XComp
{
    friend class XCompUI;

    GraphicsApp         *mpGfxApp {};
    AppBaseConfig       mAppBaseConfig;
    uptr<XCompUI>       moXCompUI;

    TimedEvent          mCheckFilesTE { TimeUS::ONE_SECOND() * 5 };

    DStr                mNextSelPathFName;

    uptr<ImageSystem>   moIMSys;

private:
    const XCompParams mPar;

public:
    XComp( const XCompParams &par );
    ~XComp();

    c_auto &GetParamsXC() const { return mPar; }

          XCConfig &GetConfigXC();
    const XCConfig &GetConfigXC() const;

    const DStr &GetCacheDir() const { return mPar.mAppCacheDir; }
    const DStr &GetUserPofDir() const { return mPar.mAppUserProfDir; }
    const DStr &GetUserProfDisplay() const { return mPar.mAppUserProfDisplay; }

    void SaveCompositeXC();

    void EnterMainLoop( const DFun<void()> &onCreationFn={} );

          GraphicsApp *GetGfxApp()       { return mpGfxApp; }
    const GraphicsApp *GetGfxApp() const { return mpGfxApp; }

private:
    TimeUS mLazySaveConfigTimeUS;
    void reqLazySaveConfig();
    void checkLazySaveConfig( TimeUS curTimeUS );

    void animateApp( TimeUS curTimeUS );
};

#endif

