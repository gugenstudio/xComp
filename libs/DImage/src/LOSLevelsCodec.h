//==================================================================
/// LOSLevelsCodec.h
///
/// Created by Davide Pasca - 2017/4/13
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#ifndef LOSLEVELSCODEC_H
#define LOSLEVELSCODEC_H

#include "DLZ2.h"
#include "Image_DLS.h"

//==================================================================
namespace ImgDLS
{

static const u_int LOS_BLOCK_SIZE = 8;
static const u_int LOS_BLOCK_SIZE2 = 8*8;
static const size_t	ENTROPY_BLOCK_SIZE = 32;

//==================================================================
//==================================================================
#ifdef USE_EXTRA_BASE_COMPR
//==================================================================
DFORCEINLINE u_int countZero( const U8 *pSrc, size_t srcStride, const U8 *pSrcEnd )
{
	u_int	cnt = 0;

	for (const U8 *pSrc2 = pSrc; pSrc2 < pSrcEnd; pSrc2 += srcStride, ++cnt)
		if ( *pSrc2 != 0 )
			break;

	return cnt;
}
//==================================================================
DFORCEINLINE u_int countNonZero( const U8 *pSrc, size_t srcStride, const U8 *pSrcEnd )
{
	u_int	cnt = 0;

	for (const U8 *pSrc2 = pSrc; pSrc2 < pSrcEnd; pSrc2 += srcStride, ++cnt)
		if ( *pSrc2 == 0 )
			break;

	return cnt;
}
//==================================================================
DFORCEINLINE void emitZeroRun( DUT::MemWriterDynamic &outFile, size_t cnt )
{
	// split by 0x80-1 counts !
	// (up to 127 elements, as 128 is the zero flag !)
	for (size_t i=0; i < cnt;)
	{
		size_t sub_cnt = cnt - i;
		sub_cnt = DMin( sub_cnt, (size_t)0x80-1 );

		outFile.WriteValue<U8>( (U8)sub_cnt | 0x80 );

		i += sub_cnt;
	}
}
//==================================================================
DFORCEINLINE void emitNonZeroRun( DUT::MemWriterDynamic &outFile, const U8 *pSrc, size_t srcStride, size_t cnt )
{
	// split by 0x80-1 counts !
	// (up to 127 elements, as 128 is the zero flag !)
	for (size_t i=0; i < cnt;)
	{
		size_t sub_cnt = cnt - i;
		sub_cnt = DMin( sub_cnt, (size_t)0x80-1 );

		outFile.WriteValue<U8>( (U8)sub_cnt | 0x00 );

		//outFile.WriteData( &pSrc[ from_idx + i ], sub_cnt );
		for (size_t vi=0; vi < sub_cnt; ++vi, pSrc += srcStride)
			outFile.WriteValue( *pSrc );

		i += sub_cnt;
	}
}

//=============================================================================
static void zeroEncoder( DUT::MemWriterDynamic &outFile, const U8 *pSrc, size_t srcStride, size_t srcSize )
{
	const U8	*pSrcEnd = pSrc + srcSize * srcStride;

	while ( pSrc < pSrcEnd )
	{
		u_int	cnt;

		if ( *pSrc == 0 )
		{
			cnt = countZero( pSrc, srcStride, pSrcEnd );

			emitZeroRun( outFile, cnt );
		}
		else
		{
			cnt = countNonZero( pSrc, srcStride, pSrcEnd );

			emitNonZeroRun( outFile, pSrc, srcStride, cnt );
		}

		pSrc += cnt * srcStride;
	}
}

//=============================================================================
#define USE_ZERODEST_SPEEDUP
static void zeroDecoder( U8 *pDes, size_t desStride, size_t desSize, DUT::MemReader &mr )
{
	size_t i=0;

	for (; i < desSize;)
	{
		U8		flag_and_cnt = mr.ReadValue<const U8>();
		size_t	cnt = flag_and_cnt & ~0x80;

		if ( flag_and_cnt & 0x80 )
		{
#ifdef USE_ZERODEST_SPEEDUP
			pDes += desStride * cnt;
#else
			for (size_t j=0; j < cnt; ++j, pDes += desStride)
				*pDes = 0;
#endif
		}
		else
		{
			for (size_t vi=0; vi < cnt; ++vi, pDes += desStride)
				*pDes = mr.ReadValue<const U8>();
		}

		i += cnt;
	}

	DASSERT( i == desSize );
}

//==================================================================
DFORCEINLINE u_int log2ceil( u_int val )
{
    for (u_int i=0; i < 32; ++i)
    {
        if ( ((u_int)1<<i) >= val )
            return i;
    }

    return (u_int)DNPOS;
}

//==================================================================
static void packBits(
				DUT::MemWriterDynamic &mw,
				const U8 *pData,
				size_t stride,
				size_t siz )
{
	// calc base (min) value
	u_int	minVal = 1000000;
	u_int	maxVal = 0;

	const U8 *pData2 = pData;
	for (size_t x=0; x < siz; ++x, pData2 += stride)
	{
		u_int val = (u_int)pData2[0];
		minVal = DMin( minVal, val );
		maxVal = DMax( maxVal, val );
	}
	
	// write the base value
	mw.WriteValue( (U8)minVal );

	// maximum difference from the base value
	u_int maxDiff = maxVal - minVal;
	u_int maxBits = log2ceil( maxDiff+1 );

	//mw.WriteValue( (U8)maxBits );
	mw.WriteBits( maxBits, 4 );

	pData2 = pData;
	for (size_t x=0; x < siz; ++x, pData2 += stride)
	{
		u_int diff = pData2[0] - minVal;

		mw.WriteBits( diff, maxBits );
	}

	mw.WriteBitsEnd();
}

//==================================================================
static void unpackBits(
					U8 *pData,
					size_t stride,
					size_t siz,
					DUT::MemReader &mr )
{
	U8		minVal = mr.ReadValue<U8>();
	u_int	maxBits = mr.ReadBits( 4 );

	for (size_t x=0; x < siz; ++x, pData += stride)
	{
		u_int diff = mr.ReadBits( maxBits );
		u_int val = diff + minVal;

		pData[0] = (U8)val;
	}

	mr.ReadBitsEnd();
}
#endif // USE_EXTRA_BASE_COMPR

//==================================================================
//==================================================================
template <typename QT>
class LOSLevelsBase
{
public:
    const static size_t QUANT_BYTES = sizeof(QT);

	DVec<U8>	mLevels3B [MAX_CHANS][LOS_BLOCK_SIZE2];
	U8			mMaxBytesN[MAX_CHANS][LOS_BLOCK_SIZE2] {};

    size_t      mPixelsPerLevN = 0;

public:
    LOSLevelsBase( u_int srcChansN, u_int chansMask, size_t pixelsPerLevN )
        : mPixelsPerLevN(pixelsPerLevN)
    {
        for (u_int ch=0; ch != srcChansN; ++ch)
        {
            // skip the channels that are not needed
            if NOT( chansMask & (1 << ch) )
                continue;

            // allocated the "levels" data and initialize to 0
            for (size_t i=0; i != LOS_BLOCK_SIZE2; ++i)
            {
                mLevels3B[ch][i].resize( mPixelsPerLevN * QUANT_BYTES );
                memset( &mLevels3B[ch][i][0], 0, mPixelsPerLevN * QUANT_BYTES );
            }
        }
    }
};

//==================================================================
template <typename QT> class LOSLeveleDecode : public LOSLevelsBase<QT>
{
public:
    LOSLeveleDecode(
            u_int srcChansN,
            u_int decodeChansMask,
            size_t pixelsPerLevN,
            DUT::MemReader &mr )
        : LOSLevelsBase<QT>( srcChansN, decodeChansMask, pixelsPerLevN )
    {
        for (u_int ch=0; ch != srcChansN; ++ch)
        {
            if ( (decodeChansMask & (1 << ch)) )
                decodeLevelsChan( mr, ch );
            else
                skipLevelsChan( mr );
        }
    }

    //
    DFORCEINLINE const U8 *GetBlockData( u_int ch, size_t coeIdx, size_t blockIdx ) const
    {
        return &this->mLevels3B[ ch ][ coeIdx ][ blockIdx * this->QUANT_BYTES ];
    }

private:
    void skipLevelsChan( DUT::MemReader &mr )
    {
        DLZ2_SkipReadSizes( mr ); // skip flags data compressed unit
        DLZ2_SkipReadSizes( mr ); // skip block data compressed unit
    }

    void decodeLevelsChan( DUT::MemReader &mr, u_int ch )
    {
        DUT::MemReader mrFlags;
        DUT::MemReader mrBlock;
        DLZ2_ExpandReadSizes( mrFlags, mr );
        DLZ2_ExpandReadSizes( mrBlock, mr );

        for (size_t i=0; i < LOS_BLOCK_SIZE2; ++i)
        {
            decodeLevelsChanCoe( mrFlags, mrBlock, ch, i );
        }
    }

    //
    void decodeLevelsChanCoe(
            DUT::MemReader &mrFlags,
            DUT::MemReader &mrBlock,
            u_int ch,
            size_t i )
    {
        this->mMaxBytesN[ch][i] = mrFlags.ReadValue<U8>();

        if NOT( this->mMaxBytesN[ch][i] )
            return;

        U8 *pLevel = &this->mLevels3B[ch][i][0];
#if defined(DEBUG) || defined(_DEBUG)
        U8 *pLevelEnd = &this->mLevels3B[ch][i].back() + 1;
#endif
        for (int j=(int)this->mMaxBytesN[ch][i]-1; j >= 0; --j)
        {
            for (size_t k=0; k < this->mPixelsPerLevN; k += ENTROPY_BLOCK_SIZE)
            {
                U8 *pLevel2 = pLevel + k * this->QUANT_BYTES + j;

                U8 compr = mrFlags.ReadValue<U8>();

                switch ( compr )
                {
                case 0:
                    break;

                case 1:
                    // n: 1   -> 0x00
                    // n: 2   -> 0x01 0x00
                    // n: 3   -> 0x01 0x01
                    // n: 257 -> 0x01 0xFF
                    {
                    size_t n = mrFlags.ReadValue<U8>();
                    size_t skipN = ENTROPY_BLOCK_SIZE * (n + 2);
                    k += skipN - ENTROPY_BLOCK_SIZE;
                    DASSERT( (k + ENTROPY_BLOCK_SIZE) <= this->mPixelsPerLevN );
                    }
                    break;

                case 2:
                    DASSERT( (pLevel2+(ENTROPY_BLOCK_SIZE-1)*this->QUANT_BYTES) <= pLevelEnd );
                    for (size_t l=0; l < ENTROPY_BLOCK_SIZE; ++l)
                        pLevel2[l*this->QUANT_BYTES] = mrBlock.ReadValue<U8>();
                    break;

#ifdef USE_EXTRA_BASE_COMPR
                case 3:
                    unpackBits( pLevel2, this->QUANT_BYTES, ENTROPY_BLOCK_SIZE, mrBlock );
                    break;

                case 4:
                    zeroDecoder( pLevel2, this->QUANT_BYTES, ENTROPY_BLOCK_SIZE, mrBlock );
                    break;
#endif
                default:
                    DASSERT( 0 );
                    break;
                }
            }
        }
    }
};

//==================================================================
//==================================================================
template <typename QT> class LOSLevelEncode : public LOSLevelsBase<QT>
{
    u_int   mChans = 0;

public:
    LOSLevelEncode(
            u_int chans,
            u_int encodeChansMask,
            size_t pixelsPerLevN )
        : LOSLevelsBase<QT>( chans, encodeChansMask, pixelsPerLevN )
        , mChans(chans)
    {
    }

    //
    void EncodeLevels( DUT::MemWriterDynamic &mw )
    {
#ifdef USE_EXTRA_BASE_COMPR
        DUT::MemWriterDynamic	packBitsMW;
        packBitsMW.Reserve( this->QUANT_BYTES * pixelsPerLevN );
        DUT::MemWriterDynamic	packZRLEMW;
        packZRLEMW.Reserve( this->QUANT_BYTES * pixelsPerLevN );
#endif

        size_t allZeroCnt = 0;

        DUT::MemWriterDynamic mwFlags;
        DUT::MemWriterDynamic mwBlock;

        for (u_int ch=0; ch != mChans; ++ch)
        {
            mwFlags.Resize( 0 );
            mwBlock.Resize( 0 );

            for (size_t i=0; i < LOS_BLOCK_SIZE2; ++i)
            {
                encodeLevelChanCoe( mwFlags, mwBlock, ch, i, allZeroCnt );
            }

            DLZ2_CompressWriteSizes( mw, mwFlags.GetDataBegin(), mwFlags.GetCurSize() );
            DLZ2_CompressWriteSizes( mw, mwBlock.GetDataBegin(), mwBlock.GetCurSize() );
        }
    }

private:
    //==================================================================
    static bool isAllZero( const U8 *pData, size_t stride, size_t siz )
    {
        size_t siz2 = siz * stride;
        for (size_t i=0; i < siz2; i += stride)
            if ( pData[i] )
                return false;

        return true;
    }

    //==================================================================
    static void flushAllZero( DUT::MemWriterDynamic &mw, size_t &io_allZeroCnt )
    {
        while ( io_allZeroCnt )
        {
            size_t nz = DMin( io_allZeroCnt, (size_t)257 );
            // nz: 1   -> 0x00
            // nz: 2   -> 0x01 0x00
            // nz: 3   -> 0x01 0x01
            // nz: 257 -> 0x01 0xFF
            if ( nz == 1 )
                mw.WriteValue<U8>( 0x00 );
            else
            {
                mw.WriteValue<U8>( 0x01 );
                mw.WriteValue<U8>( (U8)(nz-2) );
            }

            io_allZeroCnt -= nz;
        }
    }

    //==================================================================
    void encodeLevelChanCoe(
            DUT::MemWriterDynamic &mwFlags,
            DUT::MemWriterDynamic &mwBlock,
            u_int ch,
            size_t i,
            size_t &allZeroCnt )
    {
        mwFlags.WriteValue<U8>( this->mMaxBytesN[ch][i] );

        if NOT( this->mMaxBytesN[ch][i] )
            return;

        const U8 *pLevel = &this->mLevels3B[ch][i][0];
        for (int j=(int)this->mMaxBytesN[ch][i]-1; j >= 0; --j)
        {
            for (size_t k=0; k < this->mPixelsPerLevN; k += ENTROPY_BLOCK_SIZE)
            {
                const U8 *pLevel2 = pLevel + k * this->QUANT_BYTES + j;

                if ( isAllZero( pLevel2, this->QUANT_BYTES, ENTROPY_BLOCK_SIZE ) )
                {
                    allZeroCnt += 1;
                    continue;
                }

                flushAllZero( mwFlags, allZeroCnt );

#ifdef USE_EXTRA_BASE_COMPR
                packBitsMW.Resize( 0 );
                packBits( packBitsMW, pLevel2, this->QUANT_BYTES, ENTROPY_BLOCK_SIZE );

                packZRLEMW.Resize( 0 );
                zeroEncoder( packZRLEMW, pLevel2, this->QUANT_BYTES, ENTROPY_BLOCK_SIZE );

                if ( ENTROPY_BLOCK_SIZE < packBitsMW.GetCurSize() &&
                     ENTROPY_BLOCK_SIZE < packZRLEMW.GetCurSize() )
#endif
                {
                    mwFlags.WriteValue<U8>( 0x02 );
                    for (size_t l=0; l < ENTROPY_BLOCK_SIZE; ++l)
                        mwBlock.WriteValue<U8>( pLevel2[l*this->QUANT_BYTES] );
                }
#ifdef USE_EXTRA_BASE_COMPR
                else
                if ( packBitsMW.GetCurSize() < packZRLEMW.GetCurSize() )
                {
                    mwFlags.WriteValue<U8>( 0x03 );
                    mwBlock.WriteArray<U8>(
                            packBitsMW.GetDataBegin(),
                            packBitsMW.GetCurSize() );
                }
                else
                {
                    mwFlags.WriteValue<U8>( 0x04 );
                    mwBlock.WriteArray<U8>(
                            packZRLEMW.GetDataBegin(),
                            packZRLEMW.GetCurSize() );
                }
#endif
            }
            flushAllZero( mwFlags, allZeroCnt );
        }
    }

};

//==================================================================
}

#endif

