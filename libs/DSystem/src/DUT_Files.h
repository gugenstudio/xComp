//==================================================================
/// DUT_Files.h
///
/// Created by Davide Pasca - 2018/3/16
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#ifndef DUT_FILES_H
#define DUT_FILES_H

#include "DContainers.h"
#include "DUT_MemFile.h"

//==================================================================
class DFileData
{
public:
    U8      *pData {};
    size_t  dataSize = 0;
    void    *pCtx {};
    void    (*dtorCB)(DFileData *) {};

    ~DFileData()
    {
        if ( dtorCB )
            dtorCB( this );
    }

    U8 *GetData() { return pData; }
    const U8 *GetData() const { return pData; }

    size_t GetSize() const { return dataSize; }
    size_t size() const { return dataSize; }

    const U8 &operator[](size_t idx) const{ DASSERT(idx < dataSize); return pData[ idx ]; }
          U8 &operator[](size_t idx)      { DASSERT(idx < dataSize); return pData[ idx ]; }
};

//==================================================================
namespace DUT
{

//==================================================================
bool FileExists( const char *pFileName );

bool GrabFile( const char *pFileName, DVec<U8> &out_data, int fxSiz=-1 );
bool GrabFile( const char *pFileName, DFileData &out_fdata, int fxSiz=-1 );

bool SaveFile( const char *pFileName, const U8 *pInData, size_t dataSize );

bool GrabFile( const char *pFileName, MemReader &reader, int fxSiz=-1 );
bool SaveFile( const char *pFileName, const MemWriterDynamic &mw );

inline DStr JoinPath( const DStr &path1, const DStr &path2 )
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

inline DStr JoinPath( const char *pPath1, const char *pPath2 )
{
	return JoinPath( DStr( pPath1 ), DStr( pPath2 ) );
}

//==================================================================
DStr GetDirNameFromFPathName( const char *pInFPathname );
DStr GetFullDirNameFromFPathName( const char *pInFPathname );

const char *GetFileNameOnly( const char *pPathFileName );

const char *GetFileNameExt( const char *pPathFileName );
char *GetFileNameExt( char *pPathFileName );

}

#endif

