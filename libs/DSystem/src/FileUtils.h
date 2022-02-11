//==================================================================
/// FileUtils.h
///
/// Created by Davide Pasca - 2018/02/09
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#ifndef FILEUTILS_H
#define FILEUTILS_H

#include <stdio.h>
#include "DBase.h"
#include "DExceptions.h"
#include "DContainers.h"

#include <filesystem>
#include <optional>

namespace fs = std::filesystem;

//==================================================================
inline int64_t FU_ftell( FILE *pFile )
{
#ifdef WIN32
    return _ftelli64( pFile );
#else
    return ftell( pFile );
#endif
}

//==================================================================
inline int FU_fseek( FILE *pFile, int64_t off, int fsset )
{
#ifdef WIN32
    return _fseeki64( pFile, off, fsset );
#else
    return fseek( pFile, off, fsset );
#endif
}

//==================================================================
inline std::optional<size_t> FU_GetFSizeSeekStart( FILE *pFile )
{
    FU_fseek( pFile, 0, SEEK_END );
    const auto siz = FU_ftell( pFile );
    FU_fseek( pFile, 0, SEEK_SET );

    if ( siz < 0 )
        return {};
    else
        return {(size_t)siz};
}

//==================================================================
struct FileObj
{
    FILE    *mpFile {};

public:
    FileObj(
        const DStr &fname,
        const char *pMode,
        bool lockRead = false,
        bool lockWrite = false );

    ~FileObj();

    FILE *GetFilePtr() { return mpFile; }

    bool FWriteFO( const void *pData, size_t size );
    bool FReadFO( void *pData, size_t size );

    bool FSeekFO( int64_t off, int type ) { return 0 == FU_fseek( mpFile, off, type ); }

    int64_t FTellFO() { return FU_ftell( mpFile ); }

    template <typename T>
    bool FWriteFO( const T &obj )
    {
        return FWriteFO( (const void *)&obj, sizeof(obj) );
    }
};

//==================================================================
FILE *FU_FOpen( const DStr &fname, const char *pMode );
bool FU_FileExists( const DStr &fname );

//==================================================================
template <typename T>
inline void FU_WriteDataToFile( const DStr &fname, const T *pData, size_t siz )
{
    auto *pFile = FU_FOpen( fname, "wb" );
    if NOT( pFile )
        DEX_RUNTIME_ERROR( "Couldn't open file '%s' for writing", fname.c_str() );

    const auto bytesN = siz * sizeof(T);

    if ( (int)bytesN != fwrite( (const void *)pData, 1, (int)bytesN, pFile ) )
    {
        fclose( pFile );
        DEX_RUNTIME_ERROR( "Couldn't write to file '%s'", fname.c_str() );
    }

    fclose( pFile );
}

//==================================================================
template <typename T>
inline void FU_WriteVectorToFile( const DStr &fname, const DVec<T> &vec )
{
    FU_WriteDataToFile( fname, vec.data(), vec.size() );
}

//==================================================================
template <typename T>
inline DVec<T> FU_ReadVectorFromFile( const DStr &fname )
{
    DVec<T> vec;

    auto *pFile = FU_FOpen( fname, "rb" );
    if NOT( pFile )
        DEX_RUNTIME_ERROR( "Couldn't open file '%s' for reading", fname.c_str() );

    //
    const auto siz = FU_GetFSizeSeekStart( pFile );
    if NOT( siz )
        DEX_RUNTIME_ERROR( "Failed to get size of file '%s'", fname.c_str() );

    const auto bytesN = *siz;

    vec.resize( (bytesN + sizeof(T)-1) / sizeof(T) );

    const auto readSize = fread( (char *)vec.data(), 1, (size_t)bytesN, pFile );

    if ( bytesN != readSize )
    {
        fclose( pFile );
        DEX_RUNTIME_ERROR( "Couldn't read from file '%s'", fname.c_str() );
    }

    fclose( pFile );

    return vec;
}

//==================================================================
inline void FU_WriteStringToFile( const DStr &fname, const DStr &str )
{
    FU_WriteDataToFile( fname, str.data(), str.size() );
}

//==================================================================
inline DStr FU_ReadStringFromFile( const DStr &fname )
{
    auto vec = FU_ReadVectorFromFile<char>( fname );

    return {vec.data(), vec.data() + vec.size()};
}

//==================================================================
DStr FU_JPath( const DStr &path1, const DStr &path2 );
DStr FU_JPath( const char *pPath1, const char *pPath2 );

inline DStr FU_ConvertToSysSlash( const DStr &srcPath )
{
    DStr desPath = srcPath;
    for (auto &ch : desPath)
#ifdef WIN32
        ch = (ch == '/' ? '\\' : ch);
#else
        ch = (ch == '\\' ? '/' : ch);
#endif
    return desPath;
}

int FU_ChDir( const char *pDir );
DStr FU_GetHomeDir();
DStr FU_GetAppDataDir();
DStr FU_GetAppDataLocalDir();
DStr FU_GetDirFromPathFName( const DStr &pathFName );
DStr FU_ReplaceSubPaths( const DStr &src, const DStr &from, const DStr &to );
DStr FU_ExpandPath( const DStr &srcPath );
DStr FU_MakeCanonicalPath( const DStr &url );
bool FU_DirectoryExists( const DStr &dir );
bool FU_CreateDirectories( const DStr &dir );
bool FU_RemoveAll( const DStr &dir );

bool FU_CanOpenURL();
void FU_OpenURL( const DStr &url );

void FU_ShowSysConsole( bool onOff );

#endif

