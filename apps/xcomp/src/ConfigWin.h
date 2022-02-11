//==================================================================
/// ConfigWin.h
///
/// Created by Davide Pasca - 2021/09/01
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#ifndef CONFIGWIN_H
#define CONFIGWIN_H

#ifdef ENABLE_IMGUI

#include <future>
#include <optional>
#include <unordered_map>
#include "XCConfig.h"

class XComp;

//==================================================================
class ConfigWin
{
    XComp        &mXComp;

    bool            mIsFirstEntrySinceStart = true;

    XCConfig        mLocalVars;
    XCConfig        mStoredVars;

    bool            mActivate = false;

    bool            mHasChangedConfig {};

public:
    ConfigWin( XComp &bh );
    ~ConfigWin();

    void DrawConfigWin();

    void ActivateConfigWin( bool onOff );

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
};

#endif

#endif

