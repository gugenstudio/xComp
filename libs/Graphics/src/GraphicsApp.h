//==================================================================
/// GraphicsApp.h
///
/// Created by Davide Pasca - 2018/02/11
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#ifndef GRAPHICSAPP_H
#define GRAPHICSAPP_H

#if defined(_MSC_VER)
 // Make MS math.h define M_PI
 #define _USE_MATH_DEFINES
#endif

#include "DBase.h"
#include "DContainers.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <functional>
#include "FontFix.h"
#include "Graphics.h"
#include "TimeUtils.h"
#include "RenderTarget.h"

struct GLFWwindow;
class RenderTarget;
class DIMGUI_WindowEvents;

class GraphicsApp;

class UILogger;

//==================================================================
struct GraphicsAppParams
{
    bool        mIsNoUIMode = false;
    bool        mInitShowWindow = true;
    DStr        mTitle;

    int         mInitX = -599999;
    int         mInitY = -599999;

    static constexpr int DEF_WIN_WIDTH = 1024;
    static constexpr int DEF_WIN_HEIGHT = 768;

    int         mInitW = DEF_WIN_WIDTH;
    int         mInitH = DEF_WIN_HEIGHT;

    DStr        mSaveWinLayoutPathFName;
    bool        mSaveWinLayout = false;
    bool        mEnableMultiViewports   {};
    DStr        mColorScheme {"dark"};

    DStr        mCustomFontFName;
    float       mCustomFontSize {};

    DStr        mCustomFixFontFName;
    float       mCustomFixFontSize {};

    Double2     mFontFixExtraSca {1,1};
    double      mForcedContentScale = 0;
    double      mExtraContentScale = 1.0;

    TimeUS      mNonInteracIntervalUS;

    DVec<DStr>  mAppIconFNames;

    struct WindowDef
    {
        DStr            wd_name;
        DStr            wd_dir;
        float           wd_ratio = 0;
        bool            wd_startOpen = true;
        bool            wd_closeCross = false;
        DFun<void ()>   wd_winDrawFn;
        DFun<void ()>   wd_winUpdateFn;
        DFun<void (const DStr&, bool*)> wd_winUpdateCustomFn;
    };
    DVec<WindowDef> mWinDefs;

    UILogger        *mpUILogger {};

    DFun<void ()>       mOnWinHeadFn;

    struct MenuItem
    {
        DStr          mi_menuName;
        DStr          mi_itemName;
        bool          *mi_pCheckOnOff {};
        DFun<void ()> mi_onItemSelectFn;
        DFun<void ()> mi_drawFn;
    };
    DVec<MenuItem> mMenuItems;

    DFun<void (GraphicsApp *)>    mOnCreationFn;
    DFun<bool ()>                 mOnAnimateFn;
    DFun<void (Graphics &)>       mOnPaintFn;
    DFun<bool (int,int,int,int)>  mOnKeyFn;

    DFun<bool (int,int)>          mOnWindowPosFn;
    DFun<bool (int,int)>          mOnWindowSizFn;

    DFun<bool (bool)>              mOnMouseHoveredChangeFn;
    DFun<bool (Double2)>           mOnMouseMoveFn;
    DFun<bool (int,bool,Double2)>  mOnMouseButtonFn;
    DFun<bool (float)>             mOnMouseWheelFn;
    DFun<bool (int,Double2,Double2)> mOnDragMoveFn;
    DFun<bool (int,Double2,Double2)> mOnDragDoneFn;
};

//==================================================================
struct GraphicsAppWindowState
{
    bool                               ws_isOpen = true;
    bool                               ws_isShowing = false;
    const GraphicsAppParams::WindowDef *ws_pWinDef {};

    struct Render
    {
        Float4              mMainWinRC {0,0,0,0};
#ifdef ENABLE_IMGUI
        uptr<RenderTarget>  moMainWinRT;
#endif
    } ws_Render;
};

//==================================================================
class GraphicsApp
{
    GLFWwindow  *mpWin {};

    DStr        mWinTitleBase;
    DVec<DStr>  mWinStatusMsg;

    bool        mMainWin_IsMouseDown[2] {};
    Double2     mMainWin_MouseClickPos {0,0};
    Double2     mMainWin_MousePos {0,0};

    enum : int
    {
        DRAG_NONE = -1,
        DRAG_WAITFIRSTMOVE,
        DRAG_MOVING,
    };
    int         mDraggingState = DRAG_NONE;
    int         mDragButtIdx = -1;

    bool        mHasPendingWindowPos = false;
    Int2        mPendingWindowPos {0,0};
    bool        mHasPendingWindowSiz = false;
    Int2        mPendingWindowSiz {0,0};

    TimeUS      mLastPaintTimeUS;

    bool        mIsWindowMaximized = false;
    bool        mIsWindowIconified = false;

    bool        mShouldRedisplayCustomScene = true;
    bool        mShouldQuit = false;
    double      mLastEventTimeS = 0;

    DStr        mNextUIWindowFocus;

    double      mBaseContentScale = 1.0;
    double      mWinContentScale = 1.0;

    uptr<Graphics>  moGfx;
    uptr<FontFix>   moSysFont;

    void        *mpCustomFont {};
    void        *mpCustomFixFont {};

#ifdef ENABLE_IMGUI
    uptr<DIMGUI_WindowEvents>   moIMGUIWinEvents;

    int             mUsingGLVersion_Major = 0;
    int             mUsingGLVersion_Minor = 0;

    void            *mpImui_CursorPosCB {};
    void            *mpImui_KeyCB {};
    void            *mpImui_ScrollCB {};
#endif

    bool        mIsSWRendering {};

    DVec<Int4>  mMonitorsWorkArea;

private:
    DVec<GraphicsAppWindowState>  mWinStates;

    GraphicsAppParams  mPar;

public:
    GraphicsApp( const GraphicsAppParams &par );
    ~GraphicsApp();

    void PostCustomRedisplay() { mShouldRedisplayCustomScene = true; }

    void ChangeColorScheme( DStr scheme, bool forceSet=false );

    void ShowWindowGA( bool onOff );
    bool IsWindowVisibleGA() const;

    Double2 GetMousePos() const { return mMainWin_MousePos; }

    Double2 GetSysMousePos() const;
    Int2 GetSysWinSize() const;

    void SetWindowPos( int x, int y );
    Int2 GetWindowPos() const;

    void SetUIWindowFocus( const DStr &winName ) { mNextUIWindowFocus = winName; }
    void SetUIWindowOpen( const DStr &winName, bool onOff );
    bool GetIsUIWindowOpen( const DStr &winName ) const;
    bool GetIsUIWindowShowing( const DStr &winName ) const;
    bool GetIsUILightMode() const;

    void SetStatusMessage( const DVec<DStr> &strs );

      auto &GetGraphics()       { return *moGfx; }
    c_auto &GetGraphics() const { return *moGfx; }

    const FontFix &GetSysFont() const { return *moSysFont; }

    void *GetIMUIFixFont() { return mpCustomFixFont; }

    static bool DrawSBarButton( const ColorF &col, const DStr &str, const Float2 &siz={0,0} );
    static bool DrawSBarLogErrButton( const DStr &str, bool bright, const Float2 &siz={0,0} );
    static bool DrawSBarLogWrnButton( const DStr &str, bool bright, const Float2 &siz={0,0} );

private:
    double getFinalContentScale() const
    {
        return mBaseContentScale * mWinContentScale;
    }

    bool mainLoop();
    void mainWindowInit();
    void mainWindowDestroy();

    void setupDockerSpace();

    void draw_menuBar();
    void draw_statusBar();
    void draw_masterImguiWin( GraphicsAppWindowState &ws );
    void draw_customScene( int winW, int winH );

    void onInputEvent();

    bool keyCB(int key, int scancode, int action, int mods);
    void mouse_buttonCB(int button, int action, int mods);
    void cursor_positionCB( double x, double y, double winW, double winH );
    void scrollCB(double x, double y);
    void framebuffer_sizeCB(int width, int height);
    void window_posCB( int x, int y );

    void localLog( u_int flags, const DStr &str ) const;
};

#endif

