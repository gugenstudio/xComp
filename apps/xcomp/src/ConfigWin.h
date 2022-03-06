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
#include "XCConfig.h"

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

    Tab             mNextOpenTab = TAB_NONE;
    bool            mActivate = false;

    bool            mHasChangedConfig {};

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
    void writeChanges();

    void drawTabs();
    void drawGeneral();
    void drawColorCorr();
};

#endif

