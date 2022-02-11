//==================================================================
/// BC_Utils.cpp
///
/// Created by Davide Pasca - 2020/05/04
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#if defined(__GNUC__)
# include <stdio.h>
# include <unistd.h>
# include <execinfo.h>
#endif
#include <filesystem>
#include <string>
#include <array>
#include "FileUtils.h"
#include "StringUtils.h"
#include "BC_Utils.h"

//==================================================================
DStr BCUT_FindAssetsDir()
{
    constexpr const char *PRIMARY   = "assets";
    constexpr const char *SECONDARY = "../deploy_base/assets";

    if ( FU_DirectoryExists( PRIMARY ) )
        return PRIMARY;

    if ( FU_DirectoryExists( SECONDARY ) )
        return SECONDARY;

    DEX_RUNTIME_ERROR( "ERROR: Could not find the \"assets\" directory." );
}

//==================================================================
DStr BCUT_FindFont(const DStr &name )
{
    static const std::string dirs[] = {
        "share/fonts/", "../share/fonts/", "../deploy_base/fonts/"
#if defined(__linux__)
        , "/usr/local/share/fonts/"
#endif
    };
    for(const auto& f : dirs) {
        if ( FU_FileExists( f + name ) )
            return ( f + name );
    }

    DEX_RUNTIME_ERROR( "ERROR: Could not find the font \"%s\".", name.c_str() );
}

//==================================================================
DStr BCUT_FindIcon(const DStr &name )
{
    static const std::string dirs[] = {
        "share/icons/", "../share/icons/", "../deploy_base/icons/"
#if defined(__linux__)
        , "/usr/local/share/icons/"
#endif
    };
    for(const auto& f : dirs) {
        if ( FU_FileExists( f + name ) )
            return ( f + name );
    }

    DEX_RUNTIME_ERROR( "ERROR: Could not find the icon \"%s\".", name.c_str() );
}

//==================================================================
DStr BCUT_MakeAppBaseFName( const char *pArgv0 )
{
    // remove _hl*, because it's just a variant
    auto name = DStr( pArgv0 );
    if (c_auto pos = name.rfind( DStr("_hl.exe") ); pos != DStr::npos)
        name = name.substr( 0, pos ) + ".exe";
    else
    if (c_auto pos = name.rfind( DStr("_hl") ); pos != DStr::npos)
        name = name.substr( 0, pos );

    return StrFromU8Str( std::filesystem::path( name )
                                    .filename()
                                    .replace_extension()
                                    .u8string() );
}

//==================================================================
DStr BCUT_MakeAppBaseWorkDir( const DStr &profileBaseDir, const char *pArgv0 )
{
    c_auto dirName = !profileBaseDir.empty() ? profileBaseDir :
#ifdef WIN32
                            FU_GetAppDataDir();
#elif defined(__APPLE__)
                            FU_JPath( FU_GetHomeDir(), "Library/Preferences/xcomp" );
#else
                            DStr("_var");
#endif

    c_auto appBaseFName = BCUT_MakeAppBaseFName( pArgv0 );

    return FU_JPath( dirName, appBaseFName );
}

//==================================================================
DStr BCUT_MakeFNameCompatibleString( DStr str )
{
    for (auto &ch : str)
    {
        if ( ch >= 'A' && ch <= 'Z' )
        {
            ch = (char)('a' + (ch - 'A'));
        }
        else
        if NOT( (ch >= 'a' && ch <= 'z') ||
                (ch >= '0' && ch <= '9') ||
                 ch == '-' ||
                 ch == '_' )
        {
            ch = '_';
        }
    }

    return str;
}

//==================================================================
DStr BCUT_MakeProfileNameFromArgs( int argc, const char *argv[], bool forDisplay )
{
    {
        DStr profName;

        // see if we have a specific profile name requested (last overrides previous)
        for (int i=1; i < argc; ++i)
        {
            if ( !strcasecmp( argv[i], "--profile" ) )
            {
                if ( (i+1) >= argc )
                    DEX_RUNTIME_ERROR( "Missing profile name after --profile." );

                profName = argv[i+1];

                if ( forDisplay )
                    continue;

                for (c_auto &ch : profName)
                {
                    if NOT( (ch >= '0' && ch <= '9') ||
                            (ch >= 'a' && ch <= 'z') ||
                             ch == '-' ||
                             ch == '_' )
                    {
                        DEX_RUNTIME_ERROR(
                            "Bad character '%c' in profile name. "
                            "Must be lower-case alphanumeric or '-' or '_'", ch );
                    }
                }
            }
        }

        if NOT( profName.empty() )
            return profName;
    }


    // make a random profile name
    DStr paramsStr;
    DStr configPathFName;
    size_t usedParamsCnt = 0;

    // see if we have relevant params that would define a new profile
    for (int i=1; i < argc; ++i)
    {
        if ( StrEndsWithI( argv[i], ".json" ) )
        {
            usedParamsCnt += 1;
            paramsStr += DStr(" ") + argv[i];
            configPathFName = argv[i];
        }
        else
        if (   !strcasecmp( argv[i], "livetrade" )      // is livetrade
            || !strcasecmp( argv[i], "sigbinance" )     // is sigbinance
            || !strcasecmp( argv[i], "backtest" )       // is backtest
            || !strcasecmp( argv[i], "btbinance" )      // is btbinance
            || StrStartsWithI( argv[i], "-D" )          // is a macro definition
           )
        {
            usedParamsCnt += 1;
            paramsStr += DStr(" ") + argv[i];

            if ( (   !strcasecmp( argv[i], "sigbinance" )
                  || !strcasecmp( argv[i], "btbinance"  )) && (i+2) < argc )
            {
                usedParamsCnt += 2;
                paramsStr += DStr(" ") + argv[++i]; // c0
                paramsStr += DStr(" ") + argv[++i]; // c1
            }
        }
        else
        if ( !strcasecmp( argv[i], "--year" ) && (i+1) < argc )
        {
            usedParamsCnt += 1;
            paramsStr += DStr(" ") + argv[i] + argv[i+1];
        }
        else
        if ( !strcasecmp( argv[i], "--time-range" ) && (i+2) < argc )
        {
            usedParamsCnt += 1;
            paramsStr += DStr(" ") + argv[i] + argv[i+1] + argv[i+2];
        }
    }

    c_auto configFNameNoExt = StrFromU8Str( std::filesystem::path( configPathFName )
                                                .filename()
                                                .replace_extension()
                                                .u8string() );

    if ( paramsStr.empty() )
        return "default"; // no relevant params
    else
    if ( usedParamsCnt == 1 && !configFNameNoExt.empty() )
    {
        // ensure that it's a filename only and with no strange chars
        bool isBadFName = false;
        for (c_auto &c : configFNameNoExt)
        {
            if ( (isBadFName = !(
                    (c >= 'a' && c <= 'z') ||
                    (c >= 'A' && c <= 'Z') ||
                    (c >= '0' && c <= '9') ||
                     c == '-'              ||
                     c == '_'              ||
                     c == '+'              ||
                     c == '.'              )) )
            {
                break;
            }
        }

        // if it's not a "bad" file name, then proceed
        if NOT( isBadFName )
            return BCUT_MakeFNameCompatibleString( configFNameNoExt );
    }

    // if we have params, then let's make an original profile name
    c_auto hashLong = std::hash<std::string>{}( paramsStr );

    c_auto hashFolded = (uint32_t)(hashLong | (hashLong >> 32));

    return SSPrintFS( forDisplay ? "Profile:%08x" : "%08x", hashFolded );
}

//==================================================================
DStr BCUT_MakeProfileFolderNameFromArgs( int argc, const char *argv[] )
{
    return "profile_" + BCUT_MakeProfileNameFromArgs( argc, argv );
}

//==================================================================
DStr BCUT_MakeAppUserProfileDir( int argc, const char *argv[] )
{
    DStr profileBaseDir;
    for (int i=1; i < argc; ++i)
    {
        if ( !strcasecmp( argv[i], "--profile_base_dir" ) )
        {
            if ( (i+1) >= argc )
                DEX_RUNTIME_ERROR( "Missing value after --profile_base_dir." );

            profileBaseDir = argv[i+1];
        }
    }

    return FU_JPath(
            BCUT_MakeAppBaseWorkDir( profileBaseDir, argv[0] ),
            BCUT_MakeProfileFolderNameFromArgs( argc, argv ) );
}

//==================================================================
DStr BCUT_MakeAppUserProfileDisplay( int argc, const char *argv[] )
{
    return BCUT_MakeProfileNameFromArgs( argc, argv, true );
}

//==================================================================
// https://stackoverflow.com/a/77336/1507238
void BCUT_BacktraceHandler( int sig )
{
#if defined(__GNUC__)
    void *arr[ 128 ] {};

    // get void*'s for all entries on the stack
    c_auto size = backtrace( arr, std::size( arr ) );

    // print out all the frames to stderr
    fprintf( stderr, "Error: signal %d:\n", sig );

    backtrace_symbols_fd( arr, size, STDERR_FILENO );

    exit( 1 );
#endif
}
