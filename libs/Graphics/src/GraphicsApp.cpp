//==================================================================
/// GraphicsApp.cpp
///
/// Created by Davide Pasca - 2018/02/11
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#ifdef ENABLE_OPENGL
# include "GL/glew.h"
# include <GLFW/glfw3.h>
#endif

#ifdef ENABLE_IMGUI
# define IMGUI_DEFINE_MATH_OPERATORS

# include "imgui.h"
# include "imgui_impl_glfw.h"
# include "imgui_impl_opengl3.h"
# include "imgui_impl_opengl2.h"
# include "imgui_internal.h"
# include "implot.h"
# include "IMUI_Utils.h"
# include "UILogger.h"

# ifdef ENABLE_IMGUITEXINSPECT
#  include "imgui_tex_inspect.h"
#  include "tex_inspect_opengl.h"
# endif
#endif

#include "DThreads.h"
#include "TimeUtils.h"
#include "Image_PNG.h"
#include "GraphicsApp.h"
#include "DLogOut.h"
#include "DGLUtils.h"

#include "font_system_png.h"

//#define DRAW_TEST_BG
//#define DRAW_STATUSBAR_IN_MENU

//#define DEBUG_EVENTS

static bool _sDidInitGLEW;

//========================================================================
static void errorCB(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

#ifdef ENABLE_IMGUI
//==================================================================
class DIMGUI_WindowEvents
{
    Float2             mLastMousePos {};
    std::array<bool,3> mLastMouseButtDown {};
    Float2             mLastWinSize {1,1};
    bool               mLastIsHovered {};
    int                mMouseCaptureCnt = 0;

public:
    struct Params
    {
        DFun<void (bool)>                           onMouseHoverChangeFn;
        DFun<void (const Float2 &, const Float2&)>  onMousePosFn;
        DFun<void (int, bool)>                      onMouseButtonFn;
        DFun<void (float)>                          onMouseWheelFn;
        DFun<void (const Float2 &)>                 onWinSizeFn;
    };
private:

    const Params mPar;

public:
    DIMGUI_WindowEvents( const Params &par )
        : mPar(par)
    {
    }

    void CheckEvents( const Float4 &rc, bool isHovered )
    {
        if ( mLastIsHovered != isHovered )
        {
            if ( mPar.onMouseHoverChangeFn )
                mPar.onMouseHoverChangeFn( isHovered );

            mLastIsHovered = isHovered;
        }

        if ( !isHovered && !mMouseCaptureCnt )
            return;

        auto makef2 = []( const ImVec2 &v ) { return Float2( v[0], v[1] ); };

        c_auto cursorPos =
                    makef2( ImGui::GetCursorScreenPos() ) +
                    Float2( 0*rc[0], 0*rc[1] );

        if (c_auto pos = makef2(ImGui::GetMousePos()) - cursorPos; pos != mLastMousePos)
        {
            mLastMousePos = pos;

            if ( mPar.onMousePosFn )
                mPar.onMousePosFn( pos, mLastWinSize );
        }

        for (size_t i=0; i < mLastMouseButtDown.size(); ++i)
        {
            if (c_auto val = ImGui::IsMouseDown( (ImGuiMouseButton)i );
                       val != mLastMouseButtDown[i])
            {
                if ( mPar.onMouseButtonFn && (isHovered || mMouseCaptureCnt) )
                    mPar.onMouseButtonFn( (int)i, val );

                mMouseCaptureCnt += (val ? 1 : -1);
                mLastMouseButtDown[i] = val;
            }
        }

        if ( mPar.onMouseWheelFn )
        {
            if (c_auto &io = ImGui::GetIO(); io.MouseWheel)
                mPar.onMouseWheelFn( io.MouseWheel );
        }

        if (c_auto val = Float2(rc[2], rc[3]); val != mLastWinSize)
        {
            mLastWinSize = val;

            if ( mPar.onWinSizeFn )
                mPar.onWinSizeFn( val );
        }
    }
};

//==================================================================
void GraphicsApp::draw_masterImguiWin( GraphicsAppWindowState &ws )
{
#if defined(ENABLE_OPENGL) && defined(ENABLE_IMGUI)
    if ( mPar.mOnWinHeadFn )
        mPar.mOnWinHeadFn();

    auto &rend = ws.ws_Render;

    {
        const auto* window = ImGui::GetCurrentWindowRead();

        c_auto rc = window->InnerClipRect;
        c_auto pos = ImGui::GetCursorScreenPos();

        rend.mMainWinRC[0] = pos[0];
        rend.mMainWinRC[1] = pos[1];
        rend.mMainWinRC[2] = rc.Max[0] - rend.mMainWinRC[0];
        rend.mMainWinRC[3] = rc.Max[1] - rend.mMainWinRC[1];
    }

    if NOT( mWinStatusMsg.empty() )
    {
        if ( IMUI_BeginChildMouseOverlay() )
        {
            for (size_t i=0; i < mWinStatusMsg.size(); ++i)
            {
                if ( i > 0 )
                    ImGui::Separator();

                IMUI_Text( mWinStatusMsg[i] );
            }

            ImGui::End();
        }
    }

    //if ( ImGui::IsWindowFocused() || ImGui::IsWindowHovered() )
    moIMGUIWinEvents->CheckEvents( rend.mMainWinRC, ImGui::IsWindowHovered() );

    c_auto winW = (int)rend.mMainWinRC[2];
    c_auto winH = (int)rend.mMainWinRC[3];

    // special case.. it happens when the window is minimized
    if ( winW <= 0 || winH <= 0 )
    {
        rend.moMainWinRT = {};
        //ImGui::End();
        //ImGui::PopStyleVar();
        return;
    }

    //
    {
        GLint savedVP[4] {};
        glGetIntegerv( GL_VIEWPORT, savedVP );

        glViewport(0, 0, winW, winH);

        if (  !rend.moMainWinRT
            || rend.moMainWinRT->GetSize(0) != winW
            || rend.moMainWinRT->GetSize(1) != winH )
        {
            rend.moMainWinRT = {};
            RenderTarget::Params par;
            par.width       = winW;
            par.height      = winH;
            par.colorDepth  = 32;
            par.colorChansN = 4;
            par.useColTex   = true;
            par.useLinearFilter = false;
            par.samplesN    = 16;

            rend.moMainWinRT = std::make_unique<RenderTarget>( par );
        }

        //
        c_auto recentEvent = (GetSteadyTimeS() - mLastEventTimeS) < 0.2;
        if ( mShouldRedisplayCustomScene || recentEvent )
        {
            mShouldRedisplayCustomScene = false;

            glBindFramebuffer( GL_FRAMEBUFFER, rend.moMainWinRT->GetFBID() );

            draw_customScene( winW, winH );

            rend.moMainWinRT->ResolveMSFBO();

            glBindFramebuffer( GL_FRAMEBUFFER, 0 );
        }

        // Restore previous viewport
        glViewport( savedVP[0], savedVP[1], savedVP[2], savedVP[3] );
    }

    // Get the current cursor position (where your window is)
    c_auto pos = ImGui::GetCursorScreenPos();

    c_auto maxPos = ImVec2(
                        rend.mMainWinRC[0] + rend.mMainWinRC[2],
                        rend.mMainWinRC[1] + rend.mMainWinRC[3] );

    // Get the texture associated to the FBO
    auto tex = rend.moMainWinRT->GetColTex();

    // Ask ImGui to draw it as an image:
    // Under OpenGL the ImGUI image type is GLuint
    // So make sure to use "(void *)tex" but not "&tex"
    ImGui::GetWindowDrawList()->AddImage(
            (void *)(ptrdiff_t)tex,
            pos,
            maxPos,
            ImVec2(0, 1),
            ImVec2(1, 0));
#endif
}

//==================================================================
void GraphicsApp::setupDockerSpace()
{
    static bool opt_fullscreen_persistant = true;
    bool opt_fullscreen = opt_fullscreen_persistant;

    static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

    // We are using the ImGuiWindowFlags_NoDocking flag to make the parent
    // window not dockable into, because it would be confusing to have two
    // docking targets within each others.
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar
                                  | ImGuiWindowFlags_NoDocking;
    if (opt_fullscreen)
    {
        auto *viewport = ImGui::GetMainViewport();

        c_auto vpSize = viewport->Size +
                            (mPar.mpUILogger
                                ? ImVec2( 0, -IMUI_CalcMainStatusBarHeight() )
                                : ImVec2( 0, 0 ));

        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize( vpSize );
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

        window_flags |= ImGuiWindowFlags_NoTitleBar
                      | ImGuiWindowFlags_NoCollapse
                      | ImGuiWindowFlags_NoResize
                      | ImGuiWindowFlags_NoMove;

        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus
                      | ImGuiWindowFlags_NoNavFocus;
    }

    // When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will
    // render our background and handle the pass-thru hole, so we ask Begin()
    // to not render a background.
    if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
        window_flags |= ImGuiWindowFlags_NoBackground;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("DockSpace", nullptr, window_flags);
    ImGui::PopStyleVar();

    if (opt_fullscreen)
        ImGui::PopStyleVar(2);

    // DockSpace
    const ImGuiID dockspaceID = ImGui::GetID("MasterDockspace");

    if NOT( ImGui::DockBuilderGetNode( dockspaceID ) )
    {
        auto *viewport = ImGui::GetMainViewport();

        ImGui::DockBuilderRemoveNode( dockspaceID ); // Clear out existing layout
        ImGui::DockBuilderAddNode( dockspaceID  );

        c_auto vpSize = viewport->Size +
                            (mPar.mpUILogger
                                ? ImVec2( 0, -IMUI_CalcMainStatusBarHeight() )
                                : ImVec2( 0, 0 ));

        ImGui::DockBuilderSetNodeSize( dockspaceID, vpSize );

        ImGuiID dock_main_id =  dockspaceID;

        //
        auto makeSplitStack = [&]( const DStr &dirName, c_auto imdir )
        {
            int splitCnt = 0;
            ImGuiID thisID = 0;
            for (c_auto &def : mPar.mWinDefs)
            {
                if ( def.wd_dir != dirName )
                    continue;

                if NOT( splitCnt++ )
                {
                    ImGui::DockBuilderDockWindow(
                        def.wd_name.c_str(),
                        ImGui::DockBuilderSplitNode(
                            dock_main_id,
                            imdir,
                            def.wd_ratio ? def.wd_ratio : 0.5f,
                            &thisID,
                            &dock_main_id ) );
                }
                else
                {
                    ImGui::DockBuilderDockWindow( def.wd_name.c_str(), thisID );
                }
            }
        };

        //
        makeSplitStack( "down", ImGuiDir_Down );
        makeSplitStack( "left", ImGuiDir_Left );
#if 1
        makeSplitStack( "right", ImGuiDir_Right );
#else
        //
        auto splitAndDock = [&]( c_auto &name, c_auto dir, c_auto r )
        {
            if NOT( name.empty() )
                ImGui::DockBuilderDockWindow(
                    name.c_str(),
                    ImGui::DockBuilderSplitNode(
                        dock_main_id, dir, r, NULL, &dock_main_id ) );
        };

        // main window is "right"
        for (c_auto &def : mPar.mWinDefs)
        {
            if ( def.wd_dir == "right" )
            {
                splitAndDock( def.wd_name, ImGuiDir_Right, 0.45f );
                break;
            }
        }
#endif

        //
        ImGui::DockBuilderFinish( dockspaceID );
    }

    ImGui::DockSpace( dockspaceID, ImVec2(0.0f, 0.0f), dockspace_flags);

#if 0
    dockspace_flags ^= ImGuiDockNodeFlags_NoSplit;
    dockspace_flags ^= ImGuiDockNodeFlags_NoResize;
    dockspace_flags ^= ImGuiDockNodeFlags_NoDockingInCentralNode;
    dockspace_flags ^= ImGuiDockNodeFlags_PassthruCentralNode;
    dockspace_flags ^= ImGuiDockNodeFlags_AutoHideTabBar;
#endif

    ImGui::End();
}
#endif

//==================================================================
GraphicsApp::GraphicsApp( const GraphicsAppParams &par )
    : mPar(par)
{
    localLog( LOG_STD, "Running platform: "
#if __APPLE__
        "Apple"
#elif __linux__
        "Linux"
#else
        "Windows"
#endif
    );

    localLog( LOG_STD,
        "Hardware concurrency: " +
            std::to_string( (size_t)std::thread::hardware_concurrency() ) );

    localLog( LOG_STD, "Title: " + mPar.mTitle );

#ifdef ENABLE_OPENGL
    if NOT( mPar.mIsNoUIMode )
    {
        // initialize glfw here so that we have a list of monitors
        glfwSetErrorCallback( errorCB );

        if NOT( glfwInit() )
        {
            DEX_RUNTIME_ERROR(
                "Failed to initialize glfw. Problem with graphics' drivers ?" );
        }

        //
        auto forEachMon = [&]( auto fn )
        {
            int monsN = 0;
            auto **ppMons = glfwGetMonitors( &monsN );
            for (int i=0; i < monsN; ++i)
                fn( ppMons[i] );
        };


        // determine the scale to use
        if ( mPar.mForcedContentScale )
        {
            mBaseContentScale = mPar.mForcedContentScale;
        }
        else
        {
            float maxScale = 0;

            forEachMon( [&]( auto *pMon )
            {
                float x {};
                float y {};
                glfwGetMonitorContentScale( pMon, &x, &y );

                // see the minimum between x and y, for unlikely crazy situations
                maxScale = std::max( maxScale, std::min( x, y ) );
            });

            if ( maxScale > 0 )
                mBaseContentScale = maxScale;
        }

        mBaseContentScale *= mPar.mExtraContentScale;

        // collect the monitors' work areas
        forEachMon( [&]( auto *pMon )
        {
            int x {};
            int y {};
            int w {};
            int h {};
            glfwGetMonitorWorkarea( pMon, &x, &y, &w, &h );
            mMonitorsWorkArea.emplace_back( x, y, x+w, y+h );
        });

        // do the rest of glfw initialization
        mainWindowInit();
    }
#endif

#ifndef ENABLE_OPENGL
    int mUsingGLVersion_Major = 3;
#endif
    //
    moGfx = std::make_unique<Graphics>( mUsingGLVersion_Major, mIsSWRendering );

    //
#ifdef ENABLE_OPENGL
    if NOT( mPar.mIsNoUIMode )
    {
        image	fontImg;
        Image_PNGLoad( fontImg, 0, _gsFontSystemPNG, sizeof(_gsFontSystemPNG), false );
        FontFix::Params par;
        par.pImg = &fontImg;
        par.forceBilinear = true;
        par.descender = 1;
        moSysFont = std::make_unique<FontFix>( par );
    }
#endif

    //
    if ( mPar.mOnCreationFn )
        mPar.mOnCreationFn( this );

#ifdef ENABLE_OPENGL
    if NOT( mPar.mIsNoUIMode )
    {
        int width, height;
        glfwGetFramebufferSize(mpWin, &width, &height);
        framebuffer_sizeCB(width, height);
    }
#endif

#ifdef ENABLE_IMGUI
    for (c_auto &wd : mPar.mWinDefs)
    {
        auto &ws = mWinStates.emplace_back();
        ws.ws_pWinDef = &wd;
        ws.ws_isOpen = wd.wd_startOpen;
    }
#endif

    while NOT( mShouldQuit )
    {
        if NOT( mainLoop() )
            break;
    }
}

//==================================================================
GraphicsApp::~GraphicsApp()
{
    if NOT( mPar.mIsNoUIMode )
        mainWindowDestroy();
}

//==================================================================
bool GraphicsApp::mainLoop()
{
#if defined(ENABLE_OPENGL) && !defined(ENABLE_IMGUI)
    if ( !mPar.mIsNoUIMode && glfwWindowShouldClose( mpWin ) )
        mShouldQuit = true;
#endif

    if ( mPar.mOnAnimateFn )
    {
        if NOT( mPar.mOnAnimateFn() )
            return false;
    }

#ifdef ENABLE_OPENGL
    if ( mPar.mIsNoUIMode )
        return true;

    glfwPollEvents();

    c_auto isWinVisible = glfwGetWindowAttrib( mpWin, GLFW_VISIBLE );
    if NOT( isWinVisible )
        return true;

    c_auto isLFState = [&]()
    {
        c_auto pos = GetSysMousePos();
        c_auto siz = GetSysWinSize();

        return
            (mIsSWRendering ||
                (pos[0] <  (double)0      ||
                 pos[1] <  (double)0      ||
                 pos[0] >= (double)siz[0] ||
                 pos[1] >= (double)siz[1])
            ) &&
            (GetSteadyTimeS() - mLastEventTimeS) > 10.0;
    }();

    c_auto curTimeUS = GetEpochTimeUS();

    c_auto useNonInteractIntervalUS =
                mPar.mNonInteracIntervalUS * (mIsSWRendering ? 4 : 1);

    c_auto isFirstPaint = !mLastPaintTimeUS;

    if ( !mIsWindowIconified &&
         (!isLFState || (curTimeUS - mLastPaintTimeUS) > useNonInteractIntervalUS) )
    {
        mLastPaintTimeUS = curTimeUS;

        if ( mHasPendingWindowPos )
        {
            mHasPendingWindowPos = false;
            if ( !mIsWindowMaximized && mPar.mOnWindowPosFn )
                mPar.mOnWindowPosFn( mPendingWindowPos[0], mPendingWindowPos[1] );
        }

        if ( mHasPendingWindowSiz )
        {
            mHasPendingWindowSiz = false;
            if ( !mIsWindowMaximized && mPar.mOnWindowSizFn )
                mPar.mOnWindowSizFn( mPendingWindowSiz[0], mPendingWindowSiz[1] );
        }

#ifdef ENABLE_IMGUI
        // Start the Dear ImGui frame
        if ( mUsingGLVersion_Major >= 3 )
            ImGui_ImplOpenGL3_NewFrame();
        else
            ImGui_ImplOpenGL2_NewFrame();

        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        int dispW, dispH;
        glfwGetFramebufferSize( mpWin, &dispW, &dispH );

        //
        setupDockerSpace();

        //
        draw_menuBar();

#ifndef DRAW_STATUSBAR_IN_MENU
        if ( mPar.mpUILogger && IMUI_BeginMainStatusBar() )
        {
            draw_statusBar();
            IMUI_EndMainStatusBar();
        }
#endif

        if ( glfwWindowShouldClose( mpWin ) )
            ImGui::OpenPopup( "Really Quit?" );

        //

        for (auto &ws : mWinStates)
        {
            if NOT( ws.ws_isOpen )
                continue;

            c_auto &def = *ws.ws_pWinDef;

            // some size, so that it's not 0,0
            IMUI_SetNextWindowSize( ImVec2(320,240), ImGuiCond_FirstUseEver );

            if ( def.wd_winUpdateFn )
                def.wd_winUpdateFn();

            if ( def.wd_winDrawFn )
            {
                // set the focus... NOTE: doesn't work on first paint
                if ( mNextUIWindowFocus == def.wd_name && !isFirstPaint )
                {
                    ImGui::SetNextWindowFocus();
                    mNextUIWindowFocus = {};
                }

                ws.ws_isShowing = false;
                if ( ImGui::Begin(
                            def.wd_name.c_str(),
                            def.wd_closeCross ? &ws.ws_isOpen : 0 ) )
                {
                    ws.ws_isShowing = true;
                    def.wd_winDrawFn();
                }

                ImGui::End();
            }
            else
            if ( def.wd_winUpdateCustomFn )
            {
                ws.ws_isShowing = true; // TODO: proper check
                def.wd_winUpdateCustomFn(
                        def.wd_name,
                        def.wd_closeCross ? &ws.ws_isOpen : 0 );
            }
            else
            {
                c_auto needs0Pad = false;//(def.wd_dir == "right");

                auto padOff = [&]()
                {
                    if ( needs0Pad )
                        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0,0));
                };

                auto padOn = [&]()
                {
                    if ( needs0Pad )
                        ImGui::PopStyleVar();
                };

                padOff();

                ws.ws_isShowing = false;
                if ( ImGui::Begin( def.wd_name.c_str() ) )
                {
                    ws.ws_isShowing = true;
                    padOn();

                    if ( def.wd_dir == "right" )
                        draw_masterImguiWin( ws );

                    padOff();
                }

                ImGui::End();

                padOn();
            }
        }

        // Rendering
        ImGui::Render();

        glDisable(GL_DEPTH_TEST);
        glViewport(0, 0, dispW, dispH);
        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //
        if ( mUsingGLVersion_Major >= 3 )
        {
            ImGui_ImplOpenGL3_RenderDrawData( ImGui::GetDrawData() );
        }
        else
        {
            //GLint last_program;
            //glGetIntegerv(GL_CURRENT_PROGRAM, &last_program);
            //glUseProgram(0);
            ImGui_ImplOpenGL2_RenderDrawData( ImGui::GetDrawData() );
            //glUseProgram(last_program);
        }

        if ( ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable )
        {
            auto *pOldCtx = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent( pOldCtx );
        }

        glfwSwapBuffers( mpWin );
#else
        c_auto recentEvent = (GetSteadyTimeS() - mLastEventTimeS) < 0.2;

        if ( mShouldRedisplayCustomScene || recentEvent )
        {
            draw_customScene();

            glfwSwapBuffers( mpWin );

            mShouldRedisplayCustomScene = false;
        }
#endif
    }
#endif

    return true;
}

//==================================================================
void GraphicsApp::draw_menuBar()
{
#ifdef ENABLE_IMGUI
    if NOT( ImGui::BeginMainMenuBar() )
        return;

    DFun<void ()> menuFn;

    if (ImGui::BeginMenu("File"))
    {
        for (c_auto &mi : mPar.mMenuItems)
        {
            if ( mi.mi_menuName != "File" )
                continue;

            if ( ImGui::MenuItem( mi.mi_itemName.c_str(), nullptr, mi.mi_pCheckOnOff ) )
                menuFn = mi.mi_onItemSelectFn;
        }

        ImGui::Separator();

        if ( ImGui::MenuItem( "Quit..." ) )
            menuFn = [this]()
            {
                ImGui::OpenPopup( "Really Quit?" );
            };

        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Windows"))
    {
        for (auto &ws : mWinStates)
        {
            ImGui::MenuItem( ws.ws_pWinDef->wd_name.c_str(), nullptr, &ws.ws_isOpen );
        }
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Help"))
    {
        for (c_auto &mi : mPar.mMenuItems)
        {
            if ( mi.mi_menuName != "Help" )
                continue;

            if ( ImGui::MenuItem( mi.mi_itemName.c_str() ) )
                menuFn = mi.mi_onItemSelectFn;
        }
        ImGui::EndMenu();
    }

    //
#ifdef DRAW_STATUSBAR_IN_MENU
    if ( mPar.mpUILogger )
    {
        ImGui::SameLine();
        draw_statusBar();
    }
#endif

    //
    ImGui::EndMainMenuBar();

    //
    for (c_auto &mi : mPar.mMenuItems)
        if ( mi.mi_drawFn )
            mi.mi_drawFn();

    //
    if ( menuFn )
        menuFn();

    //
    if ( ImGui::BeginPopupModal(
                "Really Quit?",
                NULL,
                ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text( "Do you really want to quit?" );
        ImGui::Separator();

        if (ImGui::Button("Yes", ImVec2(120, 0)))
        {
            ImGui::CloseCurrentPopup();
            mShouldQuit = true;
        }

        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        if (ImGui::Button("No", ImVec2(120, 0)))
        {
            ImGui::CloseCurrentPopup();

            // reset the "should close"
            glfwSetWindowShouldClose( mpWin, 0 );
        }
        ImGui::EndPopup();
    }
#endif
}

//==================================================================
bool GraphicsApp::DrawSBarButton( const ColorF &col, const DStr &str, const Float2 &siz )
{
#ifdef ENABLE_IMGUI
    IMUI_PushButtonColors( col );

    c_auto didClick = ImGui::Button( str.c_str(), ImVec2( siz[0], siz[1] ) );

    IMUI_PopButtonColors();

    return didClick;
#else
    return false;
#endif
}

//
bool GraphicsApp::DrawSBarLogErrButton(
            const DStr &str, bool bright, const Float2 &siz )
{
    return DrawSBarButton(
            ColorF(220,10,10).ScaleRGB( bright ? 1.f : 0.4f ),
            str,
            siz );
}

//
bool GraphicsApp::DrawSBarLogWrnButton(
            const DStr &str, bool bright, const Float2 &siz )
{
    return DrawSBarButton(
            ColorF(220,110,10).ScaleRGB( bright ? 1.f : 0.4f ),
            str,
            siz );
}

//==================================================================
void GraphicsApp::draw_statusBar()
{
#ifdef ENABLE_IMGUI
    auto vsep = []()
    {
        //ImGui::SameLine();
        //IMUI_Text( "|" );
        ImGui::SameLine();
    };

    //
    for (auto &ws : mWinStates)
    {
        if ( ws.ws_pWinDef->wd_name != "Log" )
            continue;

        auto &logger = *mPar.mpUILogger;

#if 0
        if ( ImGui::Checkbox( "Show Log", &ws.ws_isOpen ) )
        {
            logger.SetFilterUIL( {} );
        }
#else
        if ( ImGui::Button( ws.ws_isOpen ? "Hide Log" : "Show Log" ) )
        {
            ws.ws_isOpen = !ws.ws_isOpen;
            logger.SetFilterUIL( {} );
        }
#endif

        //
        vsep();

        if (c_auto n = logger.GetPendingErrLinesN();
                DrawSBarLogErrButton( "Errors:" + std::to_string(n), !!n ) )
        {
            if ( logger.GetFilterUIL() == "[ERR]" )
            {
                ws.ws_isOpen = !ws.ws_isOpen;
            }
            else
            {
                logger.SetFilterUIL( "[ERR]" );
                ws.ws_isOpen = true;
            }
        }

        vsep();

        if (c_auto n = logger.GetPendingWrnLinesN();
                DrawSBarLogWrnButton( "Warnings:" + std::to_string(n), !!n ) )
        {
            if ( logger.GetFilterUIL() == "[WRN]" )
            {
                ws.ws_isOpen = !ws.ws_isOpen;
            }
            else
            {
                logger.SetFilterUIL( "[WRN]" );
                ws.ws_isOpen = true;
            }
        }

        //
        break;
    }
#endif
}

//==================================================================
static bool checkIfRetinaDisplay()
{
#ifdef ENABLE_IMGUI

#ifdef __APPLE__
    auto *pWin = glfwCreateWindow( 512, 512, "RetinaTestWindow", NULL, NULL );

    int ws[2] {};
    glfwGetWindowSize( pWin, &ws[0], &ws[1] );
    int fs[2] {};
    glfwGetFramebufferSize( pWin, &fs[0], &fs[1] );

    glfwDestroyWindow( pWin );

    if ( fs[0] > 0 && ws[0] > 0 )
    {
        // NOTE: fs[0] is 2 x ws[0]
        c_auto isRetina = fs[0] > ws[0];

        LogOut( LOG_DBG,
            SSPrintFS( "GAPP: Apple Retina display %s", isRetina ? "true" : "false"));

        return isRetina;
    }
#endif

#endif

    return false;
}

//==================================================================
void GraphicsApp::mainWindowInit()
{
#ifdef ENABLE_OPENGL
#if __APPLE__
    // GL 3.2 + GLSL 150
    int reqMajorGL = 3;
    int reqMinorGL = 2;
#elif __linux__
    int reqMajorGL = 2;
    int reqMinorGL = 0;
#else
    int reqMajorGL = 3;
    int reqMinorGL = 0;
#endif

    localLog( LOG_DBG, SSPrintFS("OpenGL: Requesting Version %i.%i", reqMajorGL, reqMinorGL) );
    glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, reqMajorGL );
    glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, reqMinorGL );

#if __APPLE__
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#endif

    mWinTitleBase = mPar.mTitle;

#ifndef ENABLE_IMGUI
    glfwWindowHint( GLFW_SAMPLES, 16 );
#endif

    // hidden at creation
    glfwWindowHint( GLFW_VISIBLE, GLFW_FALSE );

    //
    if ( mPar.mInitW <= 16 || mPar.mInitH <= 16 )
    {
        localLog(
            LOG_ERR,
            SSPrintFS(
                "Window size too small (%i, %i), resetting to default.",
                mPar.mInitW, mPar.mInitH ) );

        mPar.mInitW = GraphicsAppParams::DEF_WIN_WIDTH;
        mPar.mInitH = GraphicsAppParams::DEF_WIN_HEIGHT;
    }

    // Retina Display
    c_auto isRetina = checkIfRetinaDisplay();

    mWinContentScale = isRetina ? 0.5 : 1.0;

    // set retina to IMUI
    IMUI_SetIsRetinaDisplay( isRetina );

    // communicate to IMUI once we have the scale for the window as well
    IMUI_SetContentScale( (float)getFinalContentScale() );

    c_auto reqW = (int)(mPar.mInitW * getFinalContentScale());
    c_auto reqH = (int)(mPar.mInitH * getFinalContentScale());

    mpWin = glfwCreateWindow( reqW, reqH, mWinTitleBase.c_str(), NULL, NULL );
    if NOT( mpWin )
    {
        localLog( LOG_ERR, SSPrintFS(
            "Failed to create the window w:%i, h:%i, title:%s",
                reqW, reqH, mWinTitleBase.c_str() ) );

        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    // setup the window icons, if any
    localLog( LOG_DBG, "Loading app icons" );
    if (c_auto n = mPar.mAppIconFNames.size())
    {
        DVec<GLFWimage> icons( n );
        DVec<image>     images( n );

        for (size_t i=0; i < n; ++i)
        {
            Image_PNGLoad( images[i], 0, mPar.mAppIconFNames[i].c_str() );

            icons[i].width  = images[i].mW;
            icons[i].height = images[i].mH;
            icons[i].pixels = images[i].GetPixelPtr(0,0);
        }

        glfwSetWindowIcon( mpWin, (int)n, icons.data() );
    }

    // position
    if ( mPar.mInitX != -599999 && mPar.mInitY != -599999 )
    {
        if NOT( mMonitorsWorkArea.empty() )
        {
            bool isInsideAMonitor = false;
            for (c_auto &wa : mMonitorsWorkArea)
            {
                if ( (mPar.mInitX + mPar.mInitW/2) >= wa[0] &&
                     (mPar.mInitY + 0            ) >= wa[1] &&
                     (mPar.mInitX + mPar.mInitW/2) <= wa[2] &&
                     (mPar.mInitY + mPar.mInitH/2) <= wa[3] )
                {
                    isInsideAMonitor = true;
                    glfwSetWindowPos( mpWin, mPar.mInitX, mPar.mInitY );
                    break;
                }
            }

            if NOT( isInsideAMonitor )
                localLog( LOG_ERR, SSPrintFS( "Ignoring bad window position (%i, %i).",
                                                        mPar.mInitX, mPar.mInitY ) );
        }
        else
        {
            // no monitor -> no negatives
            if ( mPar.mInitX >= -10 && mPar.mInitY >= -10 )
                glfwSetWindowPos( mpWin, mPar.mInitX, mPar.mInitY );
        }
    }

    // now can show
    if ( mPar.mInitShowWindow )
    {
        localLog( LOG_DBG, "Showing window" );
        glfwShowWindow( mpWin );
    }

    //
    glfwMakeContextCurrent(mpWin);
    //gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    glfwSwapInterval(1);

    //
    if NOT( _sDidInitGLEW )
    {
        localLog( LOG_DBG, "Initializing GLEW" );
        glewInit();
        _sDidInitGLEW = true;
    }

    localLog( LOG_DBG, "Setting window user pointer" );
    glfwSetWindowUserPointer( mpWin, (void *)this );

    //
    if ( DGLUT::SetupErrorIntercept() )
        localLog( LOG_DBG, "OpenGL: Enabled callback error management" );
    else
        localLog( LOG_DBG, "OpenGL: No callback error management" );

    //
    {
        CHECKGLERR;
        glGetIntegerv( GL_MAJOR_VERSION, &mUsingGLVersion_Major );
        glGetIntegerv( GL_MINOR_VERSION, &mUsingGLVersion_Minor );

        c_auto err = DGLUT::CheckGLErr("",0,false);

        // if this failed, then assumes it's 2.0
        if ( mUsingGLVersion_Major == 0 || err )
        {
            mUsingGLVersion_Major = 2;
            mUsingGLVersion_Minor = 0;
        }
    }

    localLog( LOG_DBG, SSPrintFS( "OpenGL: Running version %i.%i",
                                        mUsingGLVersion_Major,
                                        mUsingGLVersion_Minor ) );

    {
        const char *pVersion = (const char * )glGetString( GL_VERSION );
        localLog( LOG_DBG, DStr("OpenGL: Version: ") + (pVersion ? pVersion : "Unknown") );

        const char *pRenderer = (const char * )glGetString( GL_RENDERER );
        localLog( LOG_DBG, DStr("OpenGL: Renderer: ") + (pRenderer ? pRenderer : "Unknown") );

        if ( pRenderer && strstr( pRenderer, "llvmpipe" ) )
        {
            localLog( LOG_DBG, "Platform is using software rendering." );
            mIsSWRendering = true;
        }
    }

#ifdef ENABLE_IMGUI
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImPlot::CreateContext();

    {
        auto &style = ImPlot::GetStyle();
        if NOT( mIsSWRendering )
            style.AntiAliasedLines = true;

        style.UseISO8601 = true;
        style.Use24HourClock = true;
        style.PlotPadding = {3, 3};
    }

    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;       // Enable Docking

    if ( mPar.mEnableMultiViewports )
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    if ( mPar.mSaveWinLayout )
    {
        if NOT( mPar.mSaveWinLayoutPathFName.empty() )
            io.IniFilename = mPar.mSaveWinLayoutPathFName.c_str();
    }
    else
    {
        io.IniFilename = NULL; // Disable imgui.ini
    }

    //
    {
        static ImVector<ImWchar> ranges;
        ImFontGlyphRangesBuilder builder;
        builder.AddRanges( io.Fonts->GetGlyphRangesDefault() );

        // https://stackoverflow.com/a/40272820/1507238
        static const ImWchar sGreeks[] = { 0x0391, 0x03C9, 0 };
        builder.AddRanges( sGreeks );

        builder.BuildRanges( &ranges );

        auto loadFont = [&]( c_auto &fname, float siz ) -> ImFont*
        {
            if ( fname.empty() )
                return nullptr;

            auto *p = io.Fonts->AddFontFromFileTTF(
                        fname.c_str(),
                        (siz ? siz : 15.f) * (float)getFinalContentScale(),
                        nullptr,
                        ranges.Data );

            io.Fonts->Build();

            return p;
        };

        mpCustomFont    = loadFont( mPar.mCustomFontFName,    mPar.mCustomFontSize );
        mpCustomFixFont = loadFont( mPar.mCustomFixFontFName, mPar.mCustomFixFontSize );
    }

    // Setup Dear ImGui style
    ChangeColorScheme( mPar.mColorScheme, true );

    {
        auto  &style = ImGui::GetStyle();
        style.ScaleAllSizes( (float)getFinalContentScale() );

        style.FrameRounding = 3;
    }

    //
    if ( mUsingGLVersion_Major >= 3 )
    {
        const char *pGLSLVer = mUsingGLVersion_Major > 3 || mUsingGLVersion_Minor >= 2
                                    ? "#version 150"
                                    : "#version 130";
        ImGui_ImplOpenGL3_Init( pGLSLVer );
#ifdef ENABLE_IMGUITEXINSPECT
        ImGuiTexInspect::ImplOpenGL3_Init( pGLSLVer );
        ImGuiTexInspect::Init();
        ImGuiTexInspect::CreateContext();
#endif
    }
    else
    {
        ImGui_ImplOpenGL2_Init();
    }

    {
        DIMGUI_WindowEvents::Params par;

        par.onMouseHoverChangeFn = [this]( c_auto onOff )
        {
            onInputEvent();

            if ( mPar.mOnMouseHoveredChangeFn )
                mPar.mOnMouseHoveredChangeFn( onOff );
        };

        par.onMousePosFn = [this]( c_auto &pos, c_auto &curSiz )
        {
            cursor_positionCB( pos[0], pos[1], curSiz[0], curSiz[1] );
        };

        par.onMouseButtonFn = [this](int idx, bool down){ mouse_buttonCB( idx, down, 0 ); };

        par.onMouseWheelFn = [this]( float wheel )
        {
            if ( mPar.mOnMouseWheelFn )
                mPar.mOnMouseWheelFn( wheel );
        };

        par.onWinSizeFn = [this]( c_auto &sizf ){ mShouldRedisplayCustomScene = true; };

        moIMGUIWinEvents = std::make_unique<DIMGUI_WindowEvents>( par );
    }

    //
    static auto getGApp = [](auto *w){ return (GraphicsApp *)glfwGetWindowUserPointer(w); };

    glfwSetWindowPosCallback(mpWin, [](GLFWwindow* w, int x, int y)
    {
        getGApp(w)->window_posCB( x, y );
    });

    glfwSetFramebufferSizeCallback(mpWin, [](GLFWwindow* w, int width, int height)
    {
        getGApp(w)->framebuffer_sizeCB( width, height);
    });

    glfwSetCursorPosCallback(mpWin, [](GLFWwindow* w, double x, double y)
    {
        getGApp(w)->onInputEvent();
    });

    glfwSetKeyCallback(mpWin, [](GLFWwindow* w, int key, int scancode, int action, int mods)
    {
        getGApp(w)->keyCB( key, scancode, action, mods);
    });

    glfwSetScrollCallback(mpWin, [](GLFWwindow* w, double x, double y)
    {
        getGApp(w)->onInputEvent();
    });
#endif

    glfwSetWindowMaximizeCallback(mpWin, [](GLFWwindow* w, int maximized)
    {
        auto *thiss = getGApp(w);
#ifdef DEBUG_EVENTS
        thiss->localLog( LOG_DBG, SSPrintFS( "Event, win maximize %i", maximized ) );
#endif
        thiss->mIsWindowMaximized = (bool)maximized;
    });

    glfwSetWindowIconifyCallback(mpWin, [](GLFWwindow* w, int iconified)
    {
        auto *thiss = getGApp(w);
#ifdef DEBUG_EVENTS
        thiss->localLog( LOG_DBG, SSPrintFS( "Event, win iconify %i", iconified ) );
#endif
        thiss->mIsWindowIconified = (bool)iconified;
    });

    if ( mPar.mOnDropFn )
    {
        glfwSetDropCallback(mpWin, [](GLFWwindow* w, int count, const char** paths)
        {
            getGApp(w)->mPar.mOnDropFn( count, paths );
        });
    }
#endif

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL( mpWin, true );
}

//========================================================================
void GraphicsApp::mainWindowDestroy()
{
#ifdef ENABLE_IMGUI
    if ( mUsingGLVersion_Major >= 3 )
    {
# ifdef ENABLE_IMGUITEXINSPECT
        ImGuiTexInspect::DestroyContext( nullptr );
        ImGuiTexInspect::ImplOpenGl3_Shutdown();
# endif
        ImGui_ImplOpenGL3_Shutdown();
    }
    else
    {
        ImGui_ImplOpenGL2_Shutdown();
    }

    ImGui_ImplGlfw_Shutdown();

    ImPlot::DestroyContext();

    ImGui::DestroyContext();
#endif

#ifdef ENABLE_OPENGL
    if ( mpWin )
        glfwDestroyWindow( mpWin );

    glfwTerminate();
#endif
}

//==================================================================
void GraphicsApp::ChangeColorScheme( DStr scheme, bool forceSet )
{
#ifdef ENABLE_IMGUI
    scheme = StrMakeLower( scheme );

    if ( mPar.mColorScheme != scheme || forceSet )
    {
        mPar.mColorScheme = scheme;

        if ( scheme == "light" )
            ImGui::StyleColorsLight();
        else
        if ( scheme == "classic" )
            ImGui::StyleColorsClassic();
        else
        //if ( scheme == "dark" )
            ImGui::StyleColorsDark();
    }
#endif
}

//========================================================================
void GraphicsApp::ShowWindowGA( bool onOff )
{
#ifdef ENABLE_OPENGL
    if NOT( mpWin )
        return;

    if ( onOff )
        glfwShowWindow( mpWin );
    else
        glfwHideWindow( mpWin );
#endif
}

//========================================================================
bool GraphicsApp::IsWindowVisibleGA() const
{
#ifdef ENABLE_OPENGL
    return (bool)glfwGetWindowAttrib( mpWin, GLFW_VISIBLE );
#else
    return false;
#endif
}

//========================================================================
Double2 GraphicsApp::GetSysMousePos() const
{
    double px {};
    double py {};
#ifdef ENABLE_OPENGL
    glfwGetCursorPos( mpWin, &px, &py );
#endif
    return { px, py };
}

//========================================================================
Int2 GraphicsApp::GetSysWinSize() const
{
    Int2 siz {0,0};
#ifdef ENABLE_OPENGL
    glfwGetWindowSize( mpWin, &siz[0], &siz[1] );
#endif
    return siz;
}

//========================================================================
void GraphicsApp::SetWindowPos( int x, int y )
{
#ifdef ENABLE_OPENGL
    glfwSetWindowPos( mpWin, x, y );
#endif
}

//========================================================================
Int2 GraphicsApp::GetWindowPos() const
{
#ifdef ENABLE_OPENGL
    int xpos {};
    int ypos {};
    glfwGetWindowPos( mpWin, &xpos, &ypos );

    return {xpos, ypos};
#else
    return {0, 0};
#endif
}

//========================================================================
void GraphicsApp::SetUIWindowOpen( const DStr &winName, bool onOff )
{
    for (auto &ws : mWinStates)
    {
        if ( ws.ws_pWinDef->wd_name == winName )
        {
            ws.ws_isOpen  = onOff;
            break;
        }
    }
}

//========================================================================
bool GraphicsApp::GetIsUIWindowOpen( const DStr &winName ) const
{
    for (auto &ws : mWinStates)
        if ( ws.ws_pWinDef->wd_name == winName )
            return ws.ws_isOpen;

    return false;
}

//========================================================================
bool GraphicsApp::GetIsUIWindowShowing( const DStr &winName ) const
{
    for (auto &ws : mWinStates)
        if ( ws.ws_pWinDef->wd_name == winName )
            return ws.ws_isShowing;

    return false;
}

//========================================================================
bool GraphicsApp::GetIsUILightMode() const
{
#ifdef ENABLE_IMGUI
    return IMUI_IsLightMode();
#else
    return false;
#endif
}

//========================================================================
void GraphicsApp::SetStatusMessage( const DVec<DStr> &strs )
{
    mWinStatusMsg = strs;

    if ( mPar.mIsNoUIMode )
        return;

#ifdef ENABLE_IMGUI
#else
# ifdef ENABLE_OPENGL
    c_auto disp = mWinStatusMsg.empty() ? DStr() : mWinStatusMsg.first();
    glfwSetWindowTitle( mpWin, (mWinTitleBase + " -- " + mWinStatusMsg).c_str() );
# endif
#endif
}

//==================================================================
void GraphicsApp::localLog( u_int flags, const DStr &str ) const
{
    LogOut( flags, "GAPP: " + str );
}

//========================================================================
void GraphicsApp::draw_customScene( int winW, int winH )
{
#ifdef ENABLE_OPENGL
    // reset the OpenGL states
    moGfx->ResetStates();

    // Switch on the z-buffer
    glDisable(GL_DEPTH_TEST);
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glViewport( 0, 0, winW, winH );

    moSysFont->ChangeBaseScale( Double2( 1.f, 1.35f ) * mPar.mFontFixExtraSca );

    moGfx->SetConentScale( (float)getFinalContentScale() );
    moGfx->SetVPSize( (float)winW, (float)winH );

    // obsolete, just set to Identity
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    // projection matrix goes here just normalizing to 0..1 space with 0 at top
    {
    glMatrixMode(GL_MODELVIEW);
    // X: 0 (left  ) .. 1 (right)
    // Y: 0 (bottom) .. 1 (top  )
    const auto mtxPrj = Matrix44::OrthoRH( 0, 1, 0, 1, 0, 1 );
    glLoadMatrixf( &mtxPrj.mij(0,0) );
    }

#ifdef DRAW_TEST_BG
    auto drawRC = [this]( auto b, auto col, float oy )
    {
        const auto mtx =
            Matrix44D::Scale( (1 - b*2), (1 - b*2), 1 ) *
            Matrix44D::Translate( b, b+oy, 0 );

        moGfx->SetXForm( mtx );

        moGfx->DrawRectFill( 0, 0, 1, 1, col );
    };
    drawRC( 0.02f, ColorF(0.3f,0.3f,0.3f), 0     );
    drawRC( 0.10f, ColorF(0.7f,0.0f,0.0f), 0     );
    drawRC( 0.20f, ColorF(0.0f,0.0f,0.7f), 0.08f );
#endif

    if ( mPar.mOnPaintFn )
        mPar.mOnPaintFn( *moGfx );

    // flush the primitives
    moGfx->FlushPrims();

    // reset the OpenGL states again, just in case
    moGfx->ResetStates();
#endif
}

//
#ifdef ENABLE_IMGUI

//========================================================================
void GraphicsApp::onInputEvent()
{
    mLastEventTimeS = GetSteadyTimeS();
    mShouldRedisplayCustomScene = true;
}

//========================================================================
bool GraphicsApp::keyCB(int key, int scancode, int action, int mods)
{
    onInputEvent();

    if ( mPar.mOnKeyFn )
        if ( mPar.mOnKeyFn( key, scancode, action, mods ) )
            return true;

    if (action != GLFW_PRESS)
        return false;

    switch (key)
    {
        case GLFW_KEY_ESCAPE:
            //glfwSetWindowShouldClose(mpWin, GLFW_TRUE);
            break;
        case GLFW_KEY_SPACE: break;
        case GLFW_KEY_LEFT: break;
        case GLFW_KEY_RIGHT: break;
        case GLFW_KEY_UP: break;
        case GLFW_KEY_DOWN: break;
        case GLFW_KEY_PAGE_UP: break;
        case GLFW_KEY_PAGE_DOWN: break;

        default: break;
    }

    return false;
}

//========================================================================
void GraphicsApp::mouse_buttonCB(int button, int action, int mods)
{
    onInputEvent();

    c_auto idx = [&]()
    {
        switch ( button )
        {
        case GLFW_MOUSE_BUTTON_LEFT:  return 0;
        case GLFW_MOUSE_BUTTON_RIGHT: return 1;
        }
        return -1;
    }();

    if ( idx == -1 )
        return;

    const auto isDown = (action == GLFW_PRESS);

    const auto justPressed = (isDown && !mMainWin_IsMouseDown[idx]);
    const auto justReleased = (!isDown && mMainWin_IsMouseDown[idx]);

    mMainWin_IsMouseDown[idx] = isDown;

    if ( justPressed )
    {
        mMainWin_MouseClickPos = mMainWin_MousePos;

        if ( mPar.mOnMouseButtonFn &&
             mPar.mOnMouseButtonFn( idx, true, mMainWin_MouseClickPos ) )
            return;

        mDraggingState = DRAG_WAITFIRSTMOVE;
        mDragButtIdx = mDragButtIdx == -1 ? idx : mDragButtIdx;
    }


    if ( justReleased )
    {
        if (mDraggingState == DRAG_MOVING )
        {
            if ( mPar.mOnDragDoneFn )
                mPar.mOnDragDoneFn( idx, mMainWin_MouseClickPos, mMainWin_MousePos );

            mDraggingState = DRAG_NONE;
            mDragButtIdx = -1;
            return;
        }
        mDraggingState = DRAG_NONE;
        mDragButtIdx = -1;

        if ( mPar.mOnMouseButtonFn &&
             mPar.mOnMouseButtonFn( idx, false, mMainWin_MouseClickPos ) )
            return;
    }
}

//========================================================================
void GraphicsApp::cursor_positionCB( double x, double y, double winW, double winH )
{
    onInputEvent();

    //if (glfwGetInputMode(mpWin, GLFW_CURSOR) == GLFW_CURSOR_DISABLED)

    mMainWin_MousePos = { x / (double)winW, 1.0 - y / (double)winH };

    if ( mPar.mOnMouseMoveFn && mPar.mOnMouseMoveFn( mMainWin_MousePos ) )
        return;

    if ( mDraggingState == DRAG_WAITFIRSTMOVE )
    {
        const auto dragLen = DLength( mMainWin_MouseClickPos - mMainWin_MousePos );
        if ( dragLen > std::min( 1.0/winW, 1.0/winH ) )
            mDraggingState = DRAG_MOVING;
    }

    if ( mDraggingState == DRAG_MOVING && mPar.mOnDragMoveFn )
        mPar.mOnDragMoveFn( mDragButtIdx, mMainWin_MouseClickPos, mMainWin_MousePos );
}

//========================================================================
void GraphicsApp::scrollCB(double x, double y)
{
    onInputEvent();

}

//========================================================================
void GraphicsApp::window_posCB( int x, int y )
{
    if ( !mIsWindowMaximized && !mIsWindowIconified )
    {
        mPendingWindowPos = {x, y};
        mHasPendingWindowPos = true;
    }
}

//========================================================================
void GraphicsApp::framebuffer_sizeCB( int width, int height )
{
#ifdef DEBUG_EVENTS
    localLog( LOG_DBG, SSPrintFS( "On framebuffer size %i %i", width, height ) );
#endif
    if ( !mIsWindowMaximized && !mIsWindowIconified )
    {
        mPendingWindowSiz = {width, height};
        mHasPendingWindowSiz = true;
    }
}

#endif // ENABLE_IMGUI

