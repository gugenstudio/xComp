//==================================================================
/// DUT_Files.cpp
///
/// Created by Davide Pasca - 2018/3/16
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#include "DUT_Files.h"
#include "FileUtils.h"

//==================================================================
namespace DUT
{

//==================================================================
bool FileExists( const char *pFileName )
{
    if (auto *pFile = fopen( pFileName, "rb" ) )
    {
        fclose( pFile );
        return true;
    }

    return false;
}

//==================================================================
bool GrabFile( const char *pFileName, DVec<U8> &out_data, int fxSiz )
{
    //(void )pMode;
    out_data.clear();

    auto *pFile = fopen( pFileName, "rb" );
    if NOT( pFile )
    {
        return false;
    }

    if ( fxSiz != -1 )
    {
        out_data.resize( (size_t)fxSiz );
    }
    else
    {
        if (auto siz = FU_GetFSizeSeekStart( pFile ))
            out_data.resize( *siz );
        else
        {
            fclose( pFile );
            return false;
        }
    }

    if ( out_data.size() )
    {
        size_t actualReadSiz = fread( &out_data[0], 1, out_data.size(), pFile );
        // resize, mostly in the case of fixed size.. because the file may be smaller
        out_data.resize( actualReadSiz );
    }

    fclose( pFile );

    return true;
}

//==================================================================
static void onFileDataDtor( DFileData *pFData )
{
    if NOT( pFData->pData )
        return;

    if ( pFData->pData )
        delete [] pFData->pData;

    pFData->pData = nullptr;
}

//==================================================================
bool GrabFile( const char *pFileName, DFileData &out_fdata, int fxSiz )
{
    auto *pFile = fopen( pFileName, "rb" );
    if NOT( pFile )
        return false;

    if ( fxSiz != -1 )
    {
        out_fdata.dataSize = (size_t)fxSiz;
    }
    else
    {
        if (auto siz = FU_GetFSizeSeekStart( pFile ))
            out_fdata.dataSize = *siz;
        else
        {
            fclose( pFile );
            return false;
        }
    }

    out_fdata.dtorCB = onFileDataDtor;

    if ( out_fdata.dataSize )
    {
        out_fdata.pData = new U8 [ out_fdata.dataSize ];

        size_t readSize = fread( (char *)out_fdata.pData, 1, out_fdata.dataSize, pFile );

        // handle the case in which fixed size is specified.. then it's not a problem
        // if the read size is less than that. Because the file could be smaller.
        if ( fxSiz != -1 && readSize < (size_t)fxSiz )
        {
            out_fdata.dataSize = readSize;
        }

        // if the read size is different, then it failed
        if ( out_fdata.dataSize != readSize )
        {
            if ( out_fdata.pData )
            {
                delete [] out_fdata.pData;
                out_fdata.pData = nullptr;
            }

            fclose( pFile );
            return false;
        }
    }

    fclose( pFile );

	return true;
}

//==================================================================
bool SaveFile( const char *pFileName, const U8 *pInData, size_t dataSize )
{
    auto *pFile = fopen( pFileName, "wb" );
    if NOT( pFile )
    {
        return false;
    }

    if ( dataSize != fwrite( pInData, 1, dataSize, pFile ) )
    {
        fclose( pFile );
        return false;
    }

    fclose( pFile );

    return true;
}

//==================================================================
bool GrabFile( const char *pFileName, MemReader &reader, int fxSiz )
{
	DVec<U8> data;
	bool ret = GrabFile( pFileName, data, fxSiz );
	reader.InitOwnVec( data );
	return ret;
}

//==================================================================
bool SaveFile( const char *pFileName, const MemWriterDynamic &mw )
{
    return SaveFile( pFileName, mw.GetDataBegin(), mw.GetCurSize() );
}

//==================================================================
//== Path name handling, etc.
//==================================================================
const char *GetFileNameOnly( const char *pPathFileName )
{
	int	len = (int)strlen( pPathFileName );

	for (int i=len-1; i >= 0; --i)
		if ( pPathFileName[i] == '/' || pPathFileName[i] == '\\' )
			return pPathFileName + i + 1;

	return pPathFileName;
}

//==================================================================
const char *GetFileNameExt( const char *pPathFileName )
{
	int	len = (int)strlen( pPathFileName );

	for (int i=len-1; i >= 0; --i)
		if ( pPathFileName[i] == '.' )
			return pPathFileName + i + 1;

	return pPathFileName + len;
}

//==================================================================
char *GetFileNameExt( char *pPathFileName )
{
	return (char *)GetFileNameExt( (const char *)pPathFileName );
}

//==================================================================
DStr GetDirNameFromFPathName( const char *pInFPathname )
{
    const char *pInFPathnameEnd = pInFPathname + strlen( pInFPathname );

    const char *pFNamePtr = GetFileNameOnly( pInFPathname );

    if ( (pFNamePtr + strlen(pFNamePtr)) >= pInFPathnameEnd )
        return DStr();  // and empty string after all..
    else
    {
        size_t  len = pFNamePtr - pInFPathname - 1;

        return DStr( pInFPathname, len );
    }
}

//==================================================================
}

