//==================================================================
/// main.cpp
///
/// Created by Davide Pasca - 2020/02/27
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#include <sstream>
#include <iostream>
#include <iomanip>
#include <stdarg.h>
#include <filesystem>
#if defined(__GNUC__)
# include <signal.h>
#endif

#include "DExceptions.h"
#include "StringUtils.h"
#include "FileUtils.h"
#include "DUT_Str.h"
#include "CmdLineUtils.h"
#include "DLogOut.h"
#include "BC_Utils.h"
#include "XComp.h"

static bool _gsForceConsole;

//==================================================================
static void printUsage( const char *pArgv0, const char *pMsg, ... )
{
    if ( pMsg )
    {
        va_list vl;
        va_start( vl, pMsg );
        vprintf( pMsg, vl );
        va_end( vl );
    }

    printf(
R"RAW(

Usage:
 %s <config_file.json> [options]
 %s <obsolete_process_list.csv> [options]

Options:
 --help                         : This help
 --noui                         : No graphical UI
 --noframeskipui                : Disable the automatic slow refresh of the display
 --forceconsole                 : Keep the console window open
 --low_memory_mode              : Force low-memory mode (only for plain clients)
 --profile <name>               : Specify a profile name to use as a reference for storage
 --cd <dir>                     : Change the current directory at the start
 --autosizemode [linear|vector] : Set the position sizing mode

Examples:
 %s compar_config.json
)RAW",
    pArgv0,
    pArgv0,
    pArgv0 );
}

//==================================================================
static XCompParams makeBLParams(int argc, const char* argv[])
{
    auto nextParam = [&]( auto &io_idx )
    {
        if ( ++io_idx >= argc )
        {
            printUsage( argv[0], "Missing parameters ?" );
            exit( -1 );
        }

        return argv[io_idx];
    };

    //
    auto par = XCompParams();

    //
    int i = 1;
    DStr procListFNameJSN;
    if ( argc > 1 && argv[1][0] != '-' && argv[1][1] != '-' )
    {
        if ( StrEndsWithI( argv[1], ".json" ) )
            procListFNameJSN = argv[1];

        i += 1;
    }

    c_auto profileDir = BCUT_MakeAppUserProfileDir( argc, argv );

    bool isDefaultJSNFName {};
    if ( procListFNameJSN.empty() )
    {
        isDefaultJSNFName = true;
        procListFNameJSN = FU_JPath( profileDir, "xcomp_config.json" );
    }

    if NOT( FU_FileExists( procListFNameJSN ) )
    {
        // if it's the default name, then we create it
        if ( isDefaultJSNFName )
        {
            // if it's a generated/default name, then we make a default file

            XCConfig blsys;

            blsys.cfg_profileBaseDir = profileDir;

            SerializeToFile( procListFNameJSN, blsys );
        }
        else
        {
            LogOut( LOG_ERR, "Could not find %s", procListFNameJSN.c_str() );
            exit( -1 );
        }
    }

    //
	for (; i < argc; ++i)
	{
        auto isparam = [&]( c_auto &src ) { return !strcasecmp( argv[i], src ); };

		if (isparam("--cd" ))
		{
            c_auto newDir = nextParam( i );
            if ( FU_ChDir( newDir ) == -1 )
                DEX_RUNTIME_ERROR( "Could not change directory to '%s'", newDir );
		}
		else if (isparam("--noui" ))            { par.mIsNoUIMode = true;   }
		else if (isparam("--noframeskipui" ))   { par.mIsNoFrameSkipUI = true;}
		else if (isparam("--forceconsole" ))    { _gsForceConsole = true;   }
		else if (isparam("--low_memory_mode" )) { par.mIsLowMemMode = true; }
		else if (isparam("--pro_mode" ))        { par.blp_isProMode = true; }
	}

    par.mIsNoUIMode = true;

    par.mAppExeDir = FU_GetDirFromPathFName( argv[0] );
#ifdef __linux__
    if ( par.mAppExeDir.empty() )
    {
        LogOut( 0, "Execution dir is empty, assuming it's /usr/local/bin" );
        par.mAppExeDir = "/usr/local/bin";
    }
#endif
    par.mConfigPathFName = procListFNameJSN;
    par.mAppUserProfDir = BCUT_MakeAppUserProfileDir( argc, argv );
    par.mAppUserProfDisplay = BCUT_MakeAppUserProfileDisplay( argc, argv );

    return par;
}

//==================================================================
static bool hasArg( int argc, const char* argv[], const char *pSearchArg )
{
	for (int i=1; i < argc; ++i)
		if ( 0 == strcasecmp( argv[i], pSearchArg ) )
            return true;

    return false;
}

//==================================================================
int main( int argc, const char* argv[] )
{
#if defined(__GNUC__)
    signal( SIGSEGV, BCUT_BacktraceHandler );
#endif

    auto localShowConsole = [&]( bool onOff )
    {
        FU_ShowSysConsole( _gsForceConsole || onOff );
    };

    localShowConsole( true );

    if ( hasArg( argc, argv, "--help" ) )
    {
        printUsage( argv[0], nullptr );
        exit( 0 );
    }

    LogCreate( BCUT_MakeAppBaseFName( argv[0] ),
               FU_JPath( BCUT_MakeAppUserProfileDir( argc, argv ), "logs" ) );

    uptr<XComp> oApp;

    try
    {
        c_auto par = makeBLParams( argc, argv );

        oApp = std::make_unique<XComp>( par );

        if NOT( oApp->GetParamsBL().mIsNoUIMode )
            localShowConsole( false );

        oApp->EnterMainLoop();

        localShowConsole( true );
    }
    catch (const std::exception& ex)
    {
        localShowConsole( true );
        LogOut( LOG_ERR, "Uncaught Exception: %s", ex.what() );

        oApp = {};
        return -1;
    }
    catch (...)
    {
        localShowConsole( true );
        LogOut( LOG_ERR, "Unknown Uncaught Exception" );

        oApp = {};
        return -1;
    }

    oApp = {};

	return 0;
}

