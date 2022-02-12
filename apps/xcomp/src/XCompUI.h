//==================================================================
/// XCompUI.h
///
/// Created by Davide Pasca - 2022/02/07
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#ifndef XCOMPUI_H
#define XCOMPUI_H

#include "DBase.h"
#include "UILogger.h"

class XComp;
class DisplayConfigWin;
struct GraphicsAppParams;
class ConfigWin;

//==================================================================
class XCompUI
{
    friend class XComp;

    XComp               &mXComp;

    uptr<ConfigWin>     moConfigWin;
    uptr<DisplayConfigWin> moDisplayConfigWin;

    UILogger            mUILogger { true };

    TimeUS              mAppBaseConfig_SaveTimeUS;

    bool                mShowDebugWin {};

    bool                mRenderHasFocus     {};
    bool                mDisplayHasFocus    {};

    float               mCurZoom = 1.0f;

public:
    XCompUI( XComp &bl );
    ~XCompUI();

    void SetupGraphicsAppParams( GraphicsAppParams &par );
    void SetupGraphicsAppParamsDispConfigMenu( GraphicsAppParams &par );
    void OnAnimateXCUI();

private:
    void drawImgList();
    void drawLayersList();
    void drawDisplayHead();
    void drawCompositeDisp();
};

#endif

