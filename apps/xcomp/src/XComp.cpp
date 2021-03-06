//==================================================================
/// XComp.cpp
///
/// Created by Davide Pasca - 2020/02/27
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#include <sstream>
#include <iostream>
#include <iomanip>
#include <inttypes.h>

#ifdef WIN32
# include <Windows.h>
#else
# include <sys/types.h>
# include <sys/wait.h>
# include <unistd.h>
# include <signal.h>
# include <poll.h>
#endif

#include "GTVersions.h"

#include "DExceptions.h"
#include "FileUtils.h"
#include "TimeUtils.h"
#include "GraphicsApp.h"
#include "IntervalCall.h"
#include "DLogOut.h"
#include "DThreads.h"
#include "XCConfig.h"
#include "BC_Utils.h"
#include "DUT_Str.h"
#include "UILogger.h"
#include "ConfigWin.h"
#include "XCompUI.h"
#include "XComp.h"

# include "IMUI_Utils.h"
# include "XCompUI.h"
# include <GLFW/glfw3.h>

#include "ImageSystem.h"

using namespace std::string_literals;

static constexpr double ANIM_INTERVAL_S = 1.0 / 30;

static constexpr int    BASE_START_SECS_N       = 8;
static constexpr int    PRELAUNCH_SECS_N        = 5;
static constexpr int    FAILED_LAUNCH_SECS_N    = 10;

//==================================================================
static void saveConfig( const DStr &fname, const XCConfig &conf )
{
    try {
        SerializeToFile( fname, conf );
    } catch (...) {
        LogOut( LOG_ERR, "Could not write to %s", fname.c_str() );
    }
}

//==================================================================
XComp::XComp( const XCompParams &par )
    : mPar(par)
{
    auto makeDir = [&]( const DStr &name )
    {
        if ( !name.empty() && !FU_DirectoryExists( name ) )
        {
            LogOut( LOG_STD, "Creating '%s'", name.c_str() );

            if NOT( FU_CreateDirectories( name.c_str() ) )
                DEX_RUNTIME_ERROR( "Could not find or create '%s'", name.c_str() );
        }
    };

    makeDir( mPar.mAppCacheDir );   // create the cache dir if necessary
    makeDir( mPar.mAppUserProfDir );// create the user profile dir if necessary

    makeDir( FU_JPath( mPar.mAppUserProfDir, "persist_data" ) );

    //
    c_auto dispConfigDir = FU_JPath( mPar.mAppUserProfDir, "display_configs" );
    makeDir( dispConfigDir  );

    //
    mAppBaseConfig = {
        FU_JPath( dispConfigDir, "app_base_ui_layout.ini" ),
        FU_JPath( dispConfigDir, "app_base_config.json"   ) };

    //
    if ( mPar.mConfigPathFName.empty() )
    {
        LogOut( LOG_ERR, "No processes database specified." );
        //return;
    }

    //
    moXCompUI = std::make_unique<XCompUI>( *this );

    //
    try {
        // deserialize
        DeserializeFromFile( mPar.mConfigPathFName, GetConfigXC() );
    }
    catch ( const std::exception &ex )
    {
        LogOut( LOG_ERR, "Error while parsing %s (except:%s)",
            mPar.mConfigPathFName.c_str(), ex.what() );
    }
    catch (...)
    {
        LogOut( LOG_ERR, "Error while parsing %s",
            mPar.mConfigPathFName.c_str() );
    }

    //
    moIMSys = std::make_unique<ImageSystem>( GetConfigXC().cfg_imsConfig );
}

//==================================================================
XComp::~XComp()
{
    // save the pending config as we go out
    if ( mLazySaveConfigTimeUS )
        saveConfig( mPar.mConfigPathFName, GetConfigXC() );
}

//==================================================================
XCConfig &XComp::GetConfigXC()
{
    return moXCompUI->moConfigWin->GetConfigCW();
}

const XCConfig &XComp::GetConfigXC() const
{
    return moXCompUI->moConfigWin->GetConfigCW();
}

//==================================================================
void XComp::reqLazySaveConfig()
{
    mLazySaveConfigTimeUS = GetEpochTimeUS() + TimeUS::ONE_SECOND() * 3;
}

//==================================================================
void XComp::checkLazySaveConfig( TimeUS curTimeUS )
{
    // handle saving of the config lazily
    if ( mLazySaveConfigTimeUS && curTimeUS >= mLazySaveConfigTimeUS )
    {
        mLazySaveConfigTimeUS = {};

        saveConfig( mPar.mConfigPathFName, GetConfigXC() );
    }
}

//==================================================================
void XComp::animateApp( TimeUS curTimeUS )
{
    moXCompUI->OnAnimateXCUI();

    // periodically check the proceses and update the groups
    if ( mCheckFilesTE.CheckTimedEvent( curTimeUS ) )
    {
        // have a scan directory ?
        if (c_auto &dir = GetConfigXC().cfg_scanDir; !dir.empty() )
        {
            // try a new scan (file may have changed)
            if ( moIMSys->OnNewScanDir( dir, mNextSelPathFName ) )
            {
                // since the dir is probably good, we add it to the history
                GetConfigXC().AddScanDirToHistory();
            }

            // clear after use
            mNextSelPathFName = {};
        }
    }

    moIMSys->AnimateIMS();

    checkLazySaveConfig( curTimeUS );
}

//==================================================================
void XComp::SaveCompositeXC()
{
    if NOT( moIMSys->moComposite )
        return;

    c_auto &img = *moIMSys->moComposite;

    c_auto &conf = GetConfigXC();
    c_auto outDir = conf.cfg_saveDir.empty()
                            ? conf.cfg_scanDir
                            : conf.cfg_saveDir;

    if NOT( FU_DirectoryExists( outDir ) )
    {
        LogOut( 0, "Creating directories for %s", outDir.c_str() );

        if NOT( FU_CreateDirectories( outDir ) )
        {
            LogOut( LOG_ERR, "Error while creating %s", outDir.c_str() );
            return;
        }
    }

    moIMSys->SaveComposite( outDir );
}

//==================================================================
static void addMenuFolderEntry(
                GraphicsAppParams   &par,
                const DStr          &menu,
                const DStr          &title,
                DFun<DStr ()>       getDir1Fn,
                DFun<DStr ()>       getDir2Fn={},
                const DStr          desc1Str={},
                const DStr          desc2Str={} )
{
    DStr dialogID = title;
    for (auto &x : dialogID)
        x = (x == ' ' || x == '.' ? '_' : x);

    auto getAbsDir1Fn = [=]()
    {
        return FU_MakeCanonicalPath( getDir1Fn() );
    };

    auto getAbsDir2Fn = [=]()
    {
        return getDir2Fn ? FU_MakeCanonicalPath( getDir2Fn() ) : DStr();
    };

    c_auto popupTitle = menu + title;

    par.mMenuItems.push_back({
              .mi_menuName   = menu
            , .mi_itemName   = title
            , .mi_pCheckOnOff = nullptr
            , .mi_onItemSelectFn = [=]() { ImGui::OpenPopup( popupTitle.c_str() ); }
            , .mi_drawFn = [=]()
            {
                IMUI_DrawLinkPopup( popupTitle, getAbsDir1Fn, getAbsDir2Fn, desc1Str, desc2Str );
            }
    });
}

//==================================================================
inline auto makeWindowTitle = []( c_auto appName, c_auto suiteVer, c_auto subtitle )
{
    return DStr(appName) + " " + suiteVer + " -- " + subtitle;
};

//==================================================================
bool XComp::onDrop( int count, const char **ppPathFnames )
{
    if ( count <= 0 )
        return false;

    if ( count > 1 )
        LogOut( 0, "%i files dropped. Using only the first one.", count );

    c_auto pathFName = DStr( ppPathFnames[0] );

    namespace fs = std::filesystem;

    c_auto st = fs::status( pathFName );

    if NOT( fs::exists( st ) )
    {
        LogOut( LOG_ERR, "%s doesn't seem to exist", pathFName.c_str() );
        return false;
    }

    if ( fs::is_directory( st ) )
    {
        moXCompUI->moConfigWin->UpdateConfig( [&]( auto &io_conf )
        {
            io_conf.cfg_scanDir = pathFName;

            LogOut( 0, "Changed the scan directory to " + io_conf.cfg_scanDir );
        });
    }
    else
    {
        moXCompUI->moConfigWin->UpdateConfig( [&]( auto &io_conf )
        {
            // is it a config file for OpenColorIO ?
            if ( StrEndsWithI( pathFName, ".ocio" ) )
            {
                auto &cfg = io_conf.cfg_imsConfig;
                cfg.SetOCIOXForm();
                cfg.SetOCIOFName( pathFName );

                LogOut( 0, "New OpenColorIO config file: " + cfg.imsc_ccorOCIOCfgFName );
            }
            else
            // ...else, assume it's an image file
            {
                c_auto path = fs::path( pathFName );
                io_conf.cfg_scanDir = StrFromU8Str( path.parent_path().u8string() );

                mNextSelPathFName = pathFName;

                LogOut( 0, "New scan directory: " + io_conf.cfg_scanDir );
            }
        });
    }

    return true;
}

//==================================================================
void XComp::EnterMainLoop( const DFun<void()> &onCreationFn )
{
    GraphicsAppParams par;

    par.mIsNoUIMode = mPar.mIsNoUIMode;

    par.mNonInteracIntervalUS = mPar.mIsNoFrameSkipUI ? TimeUS() : TimeUS::ONE_SECOND() / 2;

    par.mTitle = makeWindowTitle( GTV_XCOMP_NAME, GTV_SUITE_VERSION, GTV_XCOMP_SUBTITLE );

    if (c_auto pos = mAppBaseConfig.GetWinPosABC())
    {
        par.mInitX = (*pos)[0];
        par.mInitY = (*pos)[1];
    }

    par.mInitW = 700;
    par.mInitH = 580;
    if (c_auto siz = mAppBaseConfig.GetWinSizABC(); siz && (*siz)[0] > 0 && (*siz)[1] > 0)
    {
        par.mInitW = DClamp( (*siz)[0], 100, 32767 );
        par.mInitH = DClamp( (*siz)[1], 100, 32767 );
    }

    par.mSaveWinLayoutPathFName = mAppBaseConfig.GetLayoutSavePathFName();
    par.mSaveWinLayout          = mAppBaseConfig.mData.save_win_layout;
    par.mEnableMultiViewports   = mAppBaseConfig.mData.enable_multi_viewports;
    par.mColorScheme            = mAppBaseConfig.mData.color_scheme;

    par.mExtraContentScale  = DClamp( mAppBaseConfig.mData.content_scale, 0.25, 2.00 );

    par.mFontFixExtraSca = { 0.90, 0.90 };
    par.mCustomFontFName    = BCUT_FindFont( "Roboto-Medium.ttf" );
    par.mCustomFontSize     = 14.f;
    par.mCustomFixFontFName = BCUT_FindFont( "Inconsolata.otf" );
    par.mCustomFixFontSize  = 13.f;

    par.mAppIconFNames = {
        BCUT_FindIcon("xcomp_icon_16.png"),
        BCUT_FindIcon("xcomp_icon_32.png"),
        BCUT_FindIcon("xcomp_icon_48.png"),
    };

    //
    moXCompUI->SetupGraphicsAppParams( par );

    //
    addMenuFolderEntry(
            par, "File", "Open Profile Folder"   , [&](){return mPar.mAppUserProfDir;} );

    moXCompUI->SetupGraphicsAppParamsDispConfigMenu( par );

    //
    par.mOnCreationFn = [&]( GraphicsApp *pApp )
    {
        // register the graphics app
        mpGfxApp = pApp;

        if ( onCreationFn )
            onCreationFn();
    };

    par.mOnAnimateFn = [&]()
    {
        c_auto curTimeUS = GetEpochTimeUS();

        animateApp( curTimeUS );

        SleepSecs( 1.0 / 60 ); // low CPU/GPU usage

        return true;
    };

    //
    par.mOnKeyFn = [this](int key, int scancode, int action, int mods)
    {
        if NOT( moXCompUI->mRenderHasFocus || moXCompUI->mDisplayHasFocus )
            return false;

#ifdef GLFW_PRESS
        const auto isDown = (action == GLFW_PRESS);

        switch ( key )
        {
        case GLFW_KEY_PAGE_UP:
        case GLFW_KEY_LEFT:
            if ( isDown )
                moIMSys->SetLastCurSel();

            return true;

        case GLFW_KEY_PAGE_DOWN:
        case GLFW_KEY_RIGHT:
            if ( isDown )
                moIMSys->SetFirstCurSel();

            return true;

        case GLFW_KEY_UP:
            if ( isDown )
                moIMSys->IncCurSel(  1 );

            return true;

        case GLFW_KEY_DOWN:
            if ( isDown )
                moIMSys->IncCurSel( -1 );

            return true;

        default: break;
        }
#endif
        return false;
    };

    //
    par.mOnDropFn = [this]( int count, const char **ppPathFnames )
    {
        return onDrop( count, ppPathFnames );
    };

    //
    GraphicsApp app( par );

    //
    // get out
}

