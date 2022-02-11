//==================================================================
/// FileUtils.cpp
///
/// Created by Davide Pasca - 2018/02/09
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#include "StringUtils.h"

#include <filesystem>
#include <cstdlib>

namespace fs = std::filesystem;

#ifdef _MSC_VER
# include <Windows.h>
# include <Shlobj.h>
# include <Shlwapi.h>
# include <direct.h>
#else
# include <unistd.h>
#endif

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#if defined(_MSC_VER)
#include <io.h>
#elif defined(__APPLE__)
# include <sys/syslimits.h>
#else
# include <linux/limits.h>
# include <sys/file.h>
#endif

#include "DExceptions.h"
#include "FileUtils.h"

#ifndef _MSC_VER
//# define DONT_USE_FILESYSTEM
#endif

#ifdef DONT_USE_FILESYSTEM
# include <dirent.h>
#endif

static int _gsCanOpenURLs = -1;

//==================================================================
FileObj::FileObj(
    const DStr &fname,
    const char *pMode,
    bool lockRead,
    bool lockWrite )
{
#if defined(_MSC_VER)
    // https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/fsopen-wfsopen

    int shflags = _SH_DENYNO;
    if ( lockRead || lockWrite )
    {
        if ( lockRead && lockWrite )
            shflags = _SH_DENYRW; // deny R & W
        else
        if ( lockRead )
            shflags = _SH_DENYRD; // deny R
        else
            shflags = _SH_DENYWR; // deny W
    }

    mpFile = _fsopen( fname.c_str(), pMode, shflags );
#else
    mpFile = FU_FOpen( fname, pMode );

    if ( mpFile && (lockRead || lockWrite) )
    {
        if ( lockRead && lockWrite )
        {
            if ( 0 != flock( fileno( mpFile ), LOCK_EX | LOCK_NB ) )
            {
                fclose( mpFile );
                mpFile = nullptr;
            }
        }
        else
        {
            DEX_RUNTIME_ERROR( "Unsupported lock mode for %s", fname.c_str() );
        }
    }
#endif
}

//==================================================================
FileObj::~FileObj()
{
    if ( mpFile ) fclose( mpFile );
}

//==================================================================
bool FileObj::FWriteFO( const void *pData, size_t size )
{
    return size == fwrite( pData, 1, size, mpFile );
}

//==================================================================
bool FileObj::FReadFO( void *pData, size_t size )
{
    return size == fread( pData, 1, size, mpFile );
}

//==================================================================
class FU_GlobalLock
{
    DStr    mLockName;

    int     mLockFD {};

public:
    FU_GlobalLock( const DStr &lockName )
        : mLockName(lockName)
    {
    }

    ~FU_GlobalLock()
    {
        //if ( mLockFD )
        //    close( mLockFD );
    }

    bool TryLockGL()
    {
        const int perm = 0666;
#ifdef _MSC_VER
        //mLockFD = _open( mLockName.c_str(), _O_CREAT | _O_EXCL | _O_WRONLY, perm );
#else
        //mLockFD = open( mLockName.c_str(), O_CREAT | O_EXCL | O_WRONLY, perm );
#endif

        return true;
    }
};

//==================================================================
FILE *FU_FOpen( const DStr &fname, const char *pMode )
{
    FILE *pFile {};
#if defined(_MSC_VER)
    fopen_s( &pFile, fname.c_str(), pMode );
#else
    pFile = fopen( fname.c_str(), pMode );
#endif

    return pFile;
}

//==================================================================
bool FU_FileExists( const DStr &fname )
{
    auto *pFile = FU_FOpen( fname, "rb" );
    if ( pFile )
        fclose( pFile );

    return !!pFile;
}

//==================================================================
DStr FU_JPath( const DStr &path1, const DStr &path2 )
{
	if NOT( path2.size() ) return path1;
	if NOT( path1.size() ) return path2;

	bool slash1 = (path1[path1.size()-1] == '\\' || path1[path1.size()-1] == '/');
	bool slash2 = (path2[0] == '\\' || path2[0] == '/');

	if ( slash1 && slash2 )
		return path1 + (path2.c_str()+1);
	else
	if ( slash1 || slash2 )
		return path1 + path2;
	else
    {
        char divider = '/';
#if defined(WIN32)
        // in case of Windows, we pick the backslash only if it's
        // alaready in the path.. otherwise we prefer the Unix-style one
        if ( DStr::npos != path1.find_first_of('\\') ||
             DStr::npos != path2.find_first_of('\\') )
        {
            divider = '\\';
        }
#endif
		return path1 + divider + path2;
    }
}

//==================================================================
DStr FU_JPath( const char *pPath1, const char *pPath2 )
{
	return FU_JPath( DStr( pPath1 ), DStr( pPath2 ) );
}

//==================================================================
#if defined(WIN32)
static DStr getWinSysDir( u_int flags )
{
    char szPath[ MAX_PATH + 64 ] {};

    auto res = SHGetFolderPath( nullptr, flags, NULL, 0, szPath );

    if ( res != S_OK )
        DEX_RUNTIME_ERROR( "Could not find the Home folder !" );

    return szPath;
}
#endif

//==================================================================
int FU_ChDir( const char *pDir )
{
#if defined(WIN32)
    return _chdir( pDir );
#else
    return chdir( pDir );
#endif
}

//==================================================================
DStr FU_GetHomeDir()
{
#if defined(WIN32)
    return getWinSysDir( CSIDL_PROFILE | CSIDL_FLAG_CREATE );
#else
    return getenv("HOME");
#endif
}

//==================================================================
DStr FU_GetAppDataDir()
{
#if defined(WIN32)
    return getWinSysDir( CSIDL_APPDATA  | CSIDL_FLAG_CREATE );
#else
    return getenv("HOME");
#endif
}

//==================================================================
DStr FU_GetAppDataLocalDir()
{
#if defined(WIN32)
    return getWinSysDir( CSIDL_LOCAL_APPDATA  | CSIDL_FLAG_CREATE );
#else
    return getenv("HOME");
#endif
}

//==================================================================
DStr FU_GetDirFromPathFName( const DStr &pathFName )
{
    auto lastPos  = pathFName.find_last_of( '/' );

#ifdef WIN32
    c_auto lastBS = pathFName.find_last_of( '\\' );
    if ( lastBS != DStr::npos )
        lastPos = std::min( lastPos, lastBS );
#endif

    return lastPos != DStr::npos ? pathFName.substr( 0, lastPos ) : DStr{};
}

//===============================================================
// see: https://stackoverflow.com/a/24315631/1507238
DStr FU_ReplaceSubPaths( const DStr &src, const DStr &from, const DStr &to )
{
    c_auto srcP  = fs::path( src );
    c_auto fromP = fs::path( from );
    c_auto toP   = fs::path( to );

    fs::path dstP;

    for (c_auto &subP : srcP)
        dstP /= (subP == fromP ? toP : subP);

    return StrFromU8Str( dstP.u8string() );
}

//==================================================================
DStr FU_ExpandPath( const DStr &srcPath )
{
    if ( srcPath.find( '~' ) != DStr::npos )
        return FU_ReplaceSubPaths( srcPath, "~", FU_GetHomeDir() );

    return srcPath;
}

//==================================================================
DStr FU_MakeCanonicalPath( const DStr &url )
{
    if ( url.empty() )
        return  {};

    try {
        return StrFromU8Str( fs::canonical( fs::path( url ) ).u8string() );
    }
    catch ( const std::exception& )
    {
        //DEX_RUNTIME_ERROR( "Failed to make canonical path for '%s'", url.c_str() );
        return url;
    }
}

//==================================================================
bool FU_DirectoryExists( const DStr &dir )
{
#ifdef DONT_USE_FILESYSTEM
    auto *pDir = opendir( dir.c_str() );
    printf( "DIR %s %p\n", dir.c_str(), pDir );
    if ( pDir )
        closedir( pDir );

    return !!pDir;
#else
    return fs::exists( dir );
#endif
}

//==================================================================
bool FU_CreateDirectories( const DStr &dir )
{
#ifdef DONT_USE_FILESYSTEM
    // https://stackoverflow.com/a/2336245
    char tmp[PATH_MAX];
    char *p = nullptr;

    snprintf(tmp, sizeof(tmp),"%s",dir.c_str());
    const auto len = strlen(tmp);
    if (tmp[len - 1] == '/')
        tmp[len - 1] = 0;

    for(p = tmp + 1; *p; ++p)
    {
        if(*p == '/')
        {
            *p = 0;
            if ( 0 != mkdir(tmp, S_IRWXU) )
                return false;
            *p = '/';
        }
    }

    if ( 0 != mkdir(tmp, S_IRWXU) )
        return false;

    return true;
#else
    std::error_code ec;
    return fs::create_directories( dir, ec );
#endif
}

//==================================================================
bool FU_RemoveAll( const DStr &dir )
{
    std::error_code ec;
    return fs::remove_all( dir, ec );
}

//==================================================================
bool FU_CanOpenURL()
{
#ifdef WIN32
    return true;
#else
    if ( _gsCanOpenURLs == -1 )
        _gsCanOpenURLs = std::system( "which xdg-open > /dev/null 2>&1" ) ? 0 : 1;

    return !!_gsCanOpenURLs;
#endif
}

//==================================================================
void FU_OpenURL( const DStr &url )
{
    if NOT( FU_CanOpenURL() )
        return;

#ifdef WIN32
    ShellExecute(
            NULL,
            "open",
            url.c_str(),
            NULL,
            NULL,
            SW_SHOWNORMAL );
#else
    std::system( ("xdg-open " + url).c_str() );
#endif
}

//==================================================================
void FU_ShowSysConsole( bool onOff )
{
#ifdef WIN32
    ShowWindow( GetConsoleWindow(), onOff ? SW_SHOW : SW_HIDE );
#endif
}

