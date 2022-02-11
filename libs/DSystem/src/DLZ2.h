//==================================================================
/// DLZ2.h
///
/// Created by Davide Pasca - 2013/11/13
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#ifndef DLZ2_H
#define DLZ2_H

#include "DUT_MemFile.h"

enum DLZ2_CCType
{
	DLZ2_CC_NONE,
	DLZ2_CC_Z1,
	DLZ2_CC_N
};

enum DLZ2_PackOptions
{
    DLZ2_PACKOPT_NORMAL,
    DLZ2_PACKOPT_EXTREME,
};

size_t DLZ2_Compress(
        DUT::MemWriterDynamic &mw,
        const U8 *pSrc,
        size_t srcSize,
        DLZ2_PackOptions opt=DLZ2_PACKOPT_NORMAL );

size_t DLZ2_Expand(
        DUT::MemWriterDynamic &mw,
        size_t desSize,
        const U8 *pSrc,
        size_t srcSize );

void DLZ2_CompressWriteSizes(
        DUT::MemWriterDynamic &des,
        const U8 *pSrc,
        size_t srcSize,
        DLZ2_PackOptions opt=DLZ2_PACKOPT_NORMAL );

void DLZ2_ExpandReadSizes(
        DUT::MemWriterDynamic &des,
        DUT::MemReader &src );

inline void DLZ2_ExpandReadSizes(
        DUT::MemReader &des,
        DUT::MemReader &src )
{
    DUT::MemWriterDynamic tmp;
    DLZ2_ExpandReadSizes( tmp, src );
    des.InitOwnMemWriterDyn( tmp );
}

void DLZ2_SkipReadSizes( DUT::MemReader &src );

void DLZ2_CompressCC(
        DUT::MemWriterDynamic &mw,
        const U8 *pSrc,
        size_t srcSize,
        DLZ2_PackOptions opt=DLZ2_PACKOPT_NORMAL );

DLZ2_CCType DLZ2_ExpandCC(
        DUT::MemWriterDynamic &mw,
        const U8 *pSrc,
        size_t srcSize );

DLZ2_CCType DLZ2_ReadFileCC(
					const char *pFileName,
					DUT::MemFile &out_mf,
					bool &out_success,
					bool useFileSystem=true );

bool DLZ2_LoadCZZFile( const char *pFName, DUT::MemReader &mr, bool prefs=false );
bool DLZ2_LoadCZZFile( const char *pFName, DStr &out_str, bool mustBeCompressed );
void DLZ2_SaveCZZFile( const char *pFName, const DUT::MemWriterDynamic &mw, bool prefs=false );
void DLZ2_SaveCZZFile( const char *pFName, const DStr &str );
void DLZ2_SaveCZZFile( const char *pFName, const uint8_t *pData, size_t siz );

#endif

