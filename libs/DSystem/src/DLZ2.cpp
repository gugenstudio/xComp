//==================================================================
/// DLZ2.cpp
///
/// Created by Davide Pasca - 2013/11/13
/// From Mark R. Nelson in Dr. Dobb's Journal, April 1989
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#include "DUT_Files.h"

#include "zlib.h"
#include "bzlib.h"

#include "DLZ2.h"

//#define TEST_FORCE_AS_IS

//#define DLOG printf
#define DLOG(FMT,...)

//==================================================================
#define CZZ3_HEAD_MARKER (((U32)'C' << 0) | ((U32)'Z' << 8) | ((U32)'Z' << 16) | ((U32)'3' << 24))

enum class DLZ2_ID : U8 { ZIP=0, BZ2=1, AS_IS=2 };

// zlib can be way too slow with large data
static const size_t MAX_SIZE_FOR_ZLIB_TRY = 1 * 1024 * 1024;

//==================================================================
size_t DLZ2_Compress(
        DUT::MemWriterDynamic &mw,
        const U8 *pSrc,
        size_t srcSize,
        DLZ2_PackOptions opt )
{
    //DFUNCTION();

    if NOT( srcSize )
        return 0;

#ifdef TEST_FORCE_AS_IS
    mw.WriteValue( DLZ2_ID::AS_IS );
    mw.WriteValue( (U32)srcSize );
    mw.WriteArray( pSrc, srcSize );
    return srcSize;
#endif

    auto algoBest = DLZ2_ID::BZ2;
    auto dataBest = DVec<U8>();
    auto dataTemp = DVec<U8>();

    // start off with ZLIB
    {
        auto ioLen = compressBound( (uLong)srcSize );
        dataTemp.resize( (size_t)ioLen );

        int err = compress2(
                (Bytef *)&dataTemp[0],
                &ioLen,
                (const Bytef *)pSrc,
                (uLong)srcSize,
                8 ); // 9 can be way too slow compared to 8

        if ( err )
            DEX_RUNTIME_ERROR( "compress2() failed !" );

        dataTemp.resize( (size_t)ioLen );
        if ( dataTemp.size() < dataBest.size() || dataBest.empty() )
        {
            dataBest = dataTemp;
            algoBest = DLZ2_ID::ZIP;
        }
    }

    // try BZ2, if we must
    if ( opt == DLZ2_PACKOPT_EXTREME )
    {
        auto ioLen = (u_int)srcSize + 1024;
        dataTemp.resize( (size_t)ioLen );

        int err = BZ2_bzBuffToBuffCompress(
                (char *)&dataTemp[0],
                &ioLen,
                (char *)pSrc,
                (u_int)srcSize,
                8,      // compression 1-9
                0,      // verbosity 0-4
                50 );   // work factor 0-250

        if ( err )
            DEX_RUNTIME_ERROR( "compress2() failed !" );

        dataTemp.resize( (size_t)ioLen );
        if ( dataTemp.size() < dataBest.size() || dataBest.empty() )
        {
            dataBest = dataTemp;
            algoBest = DLZ2_ID::BZ2;
        }
    }

    mw.WriteValue( algoBest );
    mw.WriteArray( &dataBest[0], dataBest.size() );

    return dataBest.size();
}

//==================================================================
//  This is the expansion routine.  It takes an LZW format buffer, and expands
//  it to an output buffer.  Returns the number of bytes in the output.
size_t DLZ2_Expand(
        DUT::MemWriterDynamic &mw,
        size_t desSize,
        const U8 *pSrc,
        size_t srcSize )
{
    if NOT( srcSize )
        return 0;

    auto id = ((const DLZ2_ID *)pSrc)[0];
    pSrc += sizeof(id);

    auto *pDes = mw.Grow( desSize );

    if ( id == DLZ2_ID::ZIP )
    {
        auto expandedSize = (uLongf)desSize;
        int err = uncompress(
                    (Bytef *)pDes,
                    &expandedSize,
                    (const Bytef *)pSrc,
                    (uLong)srcSize );

        if ( err != 0 || (size_t)expandedSize != desSize )
            DEX_RUNTIME_ERROR(
                "Error decoding ZIP packet (err %i, siz expand %zu, siz dest %zu ",
                err, (size_t)expandedSize, desSize );

        return (size_t)expandedSize;
    }
    else
    if ( id == DLZ2_ID::BZ2 )
    {
        auto expandedSize = (u_int)desSize;
        int err = BZ2_bzBuffToBuffDecompress(
                    (char *)pDes,
                    &expandedSize,
                    (char *)pSrc,
                    (u_int)srcSize,
                    0,
                    0 );

        if ( err != 0 || (size_t)expandedSize != desSize )
            DEX_RUNTIME_ERROR(
                "Error decoding BZ2 packet (err %i, siz expand %zu, siz dest %zu ",
                err, (size_t)expandedSize, desSize );

        return (size_t)expandedSize;
    }
    else
    if ( id == DLZ2_ID::AS_IS )
    {
        DUT::MemReader mr( pSrc, srcSize );

        auto expandedSize = (size_t)mr.ReadValue<U32>();

        DASSERT( expandedSize == desSize );

        mr.ReadArray( pDes, expandedSize );

        return expandedSize;
    }

    DEX_RUNTIME_ERROR( "Unknown DLZ2 type %zu", (size_t)id );

    return 0;
}

//==================================================================
void DLZ2_CompressWriteSizes(
        DUT::MemWriterDynamic &des,
        const U8 *pSrc,
        size_t srcSize,
        DLZ2_PackOptions opt )
{
    des.WriteValue( (U32)srcSize );

    size_t comprSizePos = des.GetCurSize();
    des.WriteValue( (U32)0 );

    DLZ2_Compress( des, pSrc, srcSize, opt );

    size_t comprSize = des.GetCurSize() - comprSizePos - sizeof(U32);

    *(U32 *)(des.GetDataBegin() + comprSizePos) = (U32)comprSize;
}

//==================================================================
void DLZ2_ExpandReadSizes(
        DUT::MemWriterDynamic &des,
        DUT::MemReader &src )
{
    auto expSize = src.ReadValue<U32>();
    auto cmpSize = src.ReadValue<U32>();

    DLZ2_Expand( des, expSize, src.GetDataPtr(cmpSize), cmpSize );
}

//==================================================================
void DLZ2_SkipReadSizes( DUT::MemReader &src )
{
    auto expSize = src.ReadValue<U32>();
    auto cmpSize = src.ReadValue<U32>();

    src.SkipBytes( cmpSize );
}

//==================================================================
void DLZ2_CompressCC(
        DUT::MemWriterDynamic &mw,
        const U8 *pSrc,
        size_t srcSize,
        DLZ2_PackOptions opt )
{
	mw.WriteValue( CZZ3_HEAD_MARKER );

    DLZ2_CompressWriteSizes( mw, pSrc, srcSize, opt );
}

//==================================================================
DLZ2_CCType DLZ2_ExpandCC(
        DUT::MemWriterDynamic &mw,
        const U8 *pSrc,
        size_t srcSize )
{
    static const size_t HEAD_SIZE = sizeof(U32);

    if NOT( srcSize >= HEAD_SIZE )
        return DLZ2_CC_NONE;

    U32 czzHeadMarker = ((U32 *)pSrc)[0];

    if ( czzHeadMarker == CZZ3_HEAD_MARKER )
    {
        DUT::MemReader mr( pSrc + HEAD_SIZE, srcSize - HEAD_SIZE );
        DLZ2_ExpandReadSizes( mw, mr );

        return DLZ2_CC_Z1;
    }
    else
    {
        return DLZ2_CC_NONE;
    }
}

//==================================================================
DLZ2_CCType DLZ2_ReadFileCC(
					const char *pFileName,
					DUT::MemFile &out_mf,
					bool &out_success,
					bool useFileSystem )
{
	DUT::MemWriterDynamic	mw;

	out_success = true;

	DVec<U8>	data;
	if NOT( DUT::GrabFile( pFileName, data ) )
	{
        DLOG("GrabFile(%s) failed", pFileName);
		out_success = false;
		return DLZ2_CC_NONE;
	}

	if NOT( data.size() )
		return DLZ2_CC_NONE;

	DLZ2_CCType type = DLZ2_ExpandCC( mw, &data[0], data.size() );

	if ( type == DLZ2_CC_NONE )
	{
		// as is
		out_mf.InitExclusiveOwenership( data );
	}
	else
	if ( type == DLZ2_CC_Z1 )
	{
		out_mf.InitExclusiveOwenership( mw );
	}

	return type;
}

//==================================================================
bool DLZ2_LoadCZZFile( const char *pFName, DUT::MemReader &mr, bool prefs )
{
	DFileData data;
	if NOT( DUT::GrabFile( pFName, data ) )
		return false; // no cache file

	DUT::MemWriterDynamic mw;

	if ( DLZ2_CC_NONE == DLZ2_ExpandCC( mw, &data[0], data.size() ) )
		return false; // bad format

	mr.InitOwnMemWriterDyn( mw );

	return true;
}

//==================================================================
bool DLZ2_LoadCZZFile( const char *pFName, DStr &out_str, bool mustBeCompressed )
{
	out_str.clear();

	DFileData data;
	if NOT( DUT::GrabFile( pFName, data ) )
		return false; // no cache file

	DUT::MemWriterDynamic mw;

	DLZ2_CCType type = DLZ2_ExpandCC( mw, &data[0], data.size() );

	if ( type == DLZ2_CC_NONE )
	{
		if ( mustBeCompressed )
			return false;

		out_str = DStr( (const char *)&data[0], data.size() );
	}
	else
		out_str = DStr( (const char *)mw.GetDataBegin(), mw.GetCurSize() );

	return true;
}

//==================================================================
void DLZ2_SaveCZZFile( const char *pFName, const DUT::MemWriterDynamic &mw, bool prefs )
{
	DUT::MemWriterDynamic mwzz;

	if ( mw.GetCurSize() )
		DLZ2_CompressCC( mwzz, mw.GetDataBegin(), mw.GetCurSize() );

	DUT::SaveFile( pFName, mwzz );
}

//==================================================================
void DLZ2_SaveCZZFile( const char *pFName, const uint8_t *pData, size_t siz )
{
    DUT::MemWriterDynamic mwzz;

    if ( siz )
        DLZ2_CompressCC( mwzz, (const U8 *)pData, siz );

    DUT::SaveFile( pFName, mwzz );
}

//==================================================================
void DLZ2_SaveCZZFile( const char *pFName, const DStr &str )
{
    DLZ2_SaveCZZFile( pFName, (const uint8_t *)str.data(), str.size() );
}

