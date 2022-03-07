//==================================================================
/// ConfigWin.h
///
/// Created by Davide Pasca - 2021/09/01
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#ifndef CONFIGWIN_H
#define CONFIGWIN_H

#include <future>
#include <optional>
#include <functional>
#include <unordered_map>
#include "TimeUtils.h"
#include "XCConfig.h"
#include "ImageSystemOCIO.h"

// determine whether we want changes to apply immediately or
//  if they should be applied only when the user clicks on "Save"
#define CONFIG_WIN_IMMEDIATE_APPLY

class XComp;

//==================================================================
class ConfigWin
{
public:
    enum Tab
    {
         TAB_NONE
        ,TAB_GENERAL
        ,TAB_COLOR_CORR
    };
private:
    XComp           &mXComp;

    bool            mIsFirstEntrySinceStart = true;

    XCConfig        mLocalVars;
    XCConfig        mStoredVars;
#ifdef ENABLE_OCIO
    ImageSystemOCIO mLocalOCIO;
    ImageSystemOCIO mStoredOCIO;
#endif

    Tab             mNextOpenTab = TAB_NONE;
    bool            mActivate = false;

    bool            mHasChangedConfig {};
#ifdef CONFIG_WIN_IMMEDIATE_APPLY
    TimedEvent      mApplyChangesTE { TimeUS::ONE_SECOND() / 10 };
#endif

public:
    ConfigWin( XComp &bh );
    ~ConfigWin();

    void DrawConfigWin();

    void ActivateConfigWin( bool onOff, Tab nextOpenTab );

    bool IsConfigWinActive() const { return mActivate; }

    void UpdateConfig( const std::function<void (XCConfig&)> &fn );

    std::optional<XCConfig> GetChangedConfig()
    {
        if NOT( mHasChangedConfig )
            return {};

        c_auto ret = mHasChangedConfig;
        mHasChangedConfig = false;
        return { mStoredVars };
    }

private:
    void storeIfChanged();
    void updateOnLocalChange();

    void drawTabs();
    void drawGeneral();
    void drawColorCorr();
#ifdef CONFIG_WIN_IMMEDIATE_APPLY
    void drawClose();
#else
    void drawSaveCancel();
#endif
};

#endif

