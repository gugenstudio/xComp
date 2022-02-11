//==================================================================
/// DisplayConfigWin.h
///
/// Created by Davide Pasca - 2020/11/08
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#ifndef DISPLAYCONFIGWIN_H
#define DISPLAYCONFIGWIN_H

#include "AppBaseConfig.h"

class GraphicsApp;

//==================================================================
class DisplayConfigWin
{
public:
    enum Tab
    {
         TAB_NONE
    };
private:
    AppBaseConfig   &mAppBaseConfig;

    bool            mIsFirstEntrySinceStart = true;

    AppBaseConfig   mEditConfig;

    bool            mActivate = false;

public:
    DisplayConfigWin( AppBaseConfig &appBaseConfig );
    ~DisplayConfigWin();

    void DrawDisplayConfigWin( GraphicsApp *pApp );

    void ActivateDisplayConfigWin( bool onOff, Tab nextOpenTab=TAB_NONE );
};

#endif

