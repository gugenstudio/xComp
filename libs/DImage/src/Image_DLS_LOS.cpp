//==================================================================
/// Image_DLS_LOS.cpp
///
/// Created by Davide Pasca - 2012/5/31
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#include "stdafx.h"
#include "DMT_DCT8.h"
#include "DSPPack.h"
#include "LOSLevelsCodec.h"
#include "Image_DLS.h"
#include "Image_DLS_LOS.h"
#include "Image_DLS_LSS.h"

//#define USE_EXTRA_BASE_COMPR
//#define SAVE_MT_PASS1

//==================================================================
namespace ImgDLS
{

//=============================================================================
const static float	DCT_COMPENSATION_SCALE = 1.0f / 8;

//=============================================================================
static const u_int gsZigZagDestTable[LOS_BLOCK_SIZE2] =
{
	 0,  1,  8, 16,  9,  2,  3, 10,
	17, 24, 32, 25, 18, 11,  4,  5,
	12, 19, 26, 33, 40, 48, 41, 34,
	27, 20, 13,  6,  7, 14, 21, 28,
	35, 42, 49, 56, 57, 50, 43, 36,
	29, 22, 15, 23, 30, 37, 44, 51,
	58, 59, 52, 45, 38, 31, 39, 46,
	53, 60, 61, 54, 47, 55, 62, 63
};

//=============================================================================
static int LinearQualityFromPercentQuality( int linearQuality )
{
	int	percentQuality = 0;

	/* Safety limit on quality factor.  Convert 0 to 1 to avoid zero divide. */
	if ( linearQuality <= 0 )	linearQuality = 1;		else
	if ( linearQuality > 100)	linearQuality = 100;

	/* The basic table is used as-is (scaling 100) for a quality of 50.
	* Qualities 50..100 are converted to scaling percentage 200 - 2*Q;
	* note that at Q=100 the scaling is 0, which will cause jpeg_add_quant_table
	* to make all the table entries 1 (hence, minimum quantization loss).
	* Qualities 1..50 are converted to scaling percentage 5000/Q.
	*/
	if ( linearQuality < 50 )
		percentQuality = 5000 / linearQuality;
	else
		percentQuality = 200 - linearQuality*2;

	return percentQuality;
}

//=============================================================================
static const int _stdLumTable[LOS_BLOCK_SIZE2] = {
	16,  11,  10,  16,  24,  40,  51,  61,
	12,  12,  14,  19,  26,  58,  60,  55,
	14,  13,  16,  24,  40,  57,  69,  56,
	14,  17,  22,  29,  51,  87,  80,  62,
	18,  22,  37,  56,  68, 109, 103,  77,
	24,  35,  55,  64,  81, 104, 113,  92,
	49,  64,  78,  87, 103, 121, 120, 101,
	72,  92,  95,  98, 112, 100, 103,  99
};

static const int _stdChrmTable[LOS_BLOCK_SIZE2] = {
    17,  18,  24,  47,  99,  99,  99,  99,
    18,  21,  26,  66,  99,  99,  99,  99,
    24,  26,  56,  99,  99,  99,  99,  99,
    47,  66,  99,  99,  99,  99,  99,  99,
    99,  99,  99,  99,  99,  99,  99,  99,
    99,  99,  99,  99,  99,  99,  99,  99,
    99,  99,  99,  99,  99,  99,  99,  99,
    99,  99,  99,  99,  99,  99,  99,  99
};

static const int *_sQuantTablesYUVA[MAX_CHANS] = {
    _stdLumTable,   // y
    _stdChrmTable,  // u
    _stdChrmTable,  // v
    _stdLumTable,   // a
};

//=============================================================================
static void makeQuantTable_Linear(
        float out_table[LOS_BLOCK_SIZE2],
        const int srcTable[LOS_BLOCK_SIZE2],
        int linearQuality )
{
    // do it in integer as JPEG does or else the quality will not match
    for (size_t i=0; i < LOS_BLOCK_SIZE2; ++i)
    {
        int deqVal = (srcTable[i] * linearQuality + 50) / 100;
        deqVal = DClamp( deqVal, 1, 65535 );
        out_table[i] = DCT_COMPENSATION_SCALE / (float)deqVal;
    }
}

//=============================================================================
static void makeDequantTable_Linear(
        float out_table[LOS_BLOCK_SIZE2],
        const int srcTable[LOS_BLOCK_SIZE2],
        int linearQuality )
{
    for (size_t i=0; i < LOS_BLOCK_SIZE2; ++i)
    {
        int deqVal = (srcTable[i] * linearQuality + 50) / 100;
        deqVal = DClamp( deqVal, 1, 65535 );
        out_table[i] = DCT_COMPENSATION_SCALE * (float)deqVal;
    }
}

//==================================================================
DFORCEINLINE void blockRGBAToYUVAf(
                        float *pDataR,
                        float *pDataG,
                        float *pDataB,
                        const int wd,
                        const int he )
{
    for (int y=0; y < he; ++y)
    {
        for (int x=0; x < wd; ++x)
        {
            auto r = pDataR[x];
            auto g = pDataG[x];
            auto b = pDataB[x];

            pDataR[x] = g;
            pDataG[x] = r - g;
            pDataB[x] = b - g;
        }
        pDataR += LOS_BLOCK_SIZE;
        pDataG += LOS_BLOCK_SIZE;
        pDataB += LOS_BLOCK_SIZE;
    }
}

//==================================================================
DFORCEINLINE void blockYUVAToRGBAf(
                        float *pDataY,
                        float *pDataU,
                        float *pDataV,
                        const int wd,
                        const int he )
{
    for (int y=0; y < he; ++y)
    {
        for (int x=0; x < wd; ++x)
        {
            auto yg = pDataY[x];
            auto cr = pDataU[x];
            auto cb = pDataV[x];

            pDataY[x] = yg + cr;
            pDataU[x] = yg;
            pDataV[x] = yg + cb;
        }
        pDataY += LOS_BLOCK_SIZE;
        pDataU += LOS_BLOCK_SIZE;
        pDataV += LOS_BLOCK_SIZE;
    }
}

//=============================================================================
template <class QTYPE>
void ZZQuant_From_DCT(	QTYPE		out_zzQuantized[LOS_BLOCK_SIZE2],
						const float	in_quantTable[LOS_BLOCK_SIZE2],
						const float	in_DCTBlock[LOS_BLOCK_SIZE2] )
{
	for (size_t i=0; i < LOS_BLOCK_SIZE2; ++i)
	{
		size_t	j = gsZigZagDestTable[i];

		int	tmpQuant = (int)(in_quantTable[j] * in_DCTBlock[j]);

		DASSERT(
			(sizeof(QTYPE) == 2 && tmpQuant >= -32768 && tmpQuant <= 32767) ||
			(sizeof(QTYPE) == 4 && tmpQuant >= -(1 << 30) && tmpQuant <= ((1 << 30)-1) )
			);

		out_zzQuantized[ i ] = (QTYPE)DSPPack::EncodeSign( tmpQuant );
	}
}

//=============================================================================
template <class QTYPE>
void DCT_From_ZZQuant(	float		out_DCTBlock[LOS_BLOCK_SIZE2],
						const float	in_dequantDCTAdjTable[LOS_BLOCK_SIZE2],
						const QTYPE	in_zzQuantized[LOS_BLOCK_SIZE2],
						u_int		levelsN )
{
	// clear the block first
	if ( levelsN < LOS_BLOCK_SIZE2 )
		memset( out_DCTBlock, 0, sizeof(float)*LOS_BLOCK_SIZE2 );

	for (size_t i=0; i < levelsN; ++i)
	{
		size_t	j = gsZigZagDestTable[i];
		int		tmpSigned = DSPPack::DecodeSign( in_zzQuantized[ i ] );
		out_DCTBlock[ j ] = (float)tmpSigned * in_dequantDCTAdjTable[ j ];
	}
}

//==================================================================
template<typename T>
void grabBlock(
        const image &img,
        u_int x,
        u_int y,
        u_int w,
        u_int h,
        float out_block[MAX_CHANS][LOS_BLOCK_SIZE2] )
{
    u_int bpp = (u_int)img.mBytesPerPixel;
    u_int bpr = (u_int)img.mBytesPerRow;

    const U8 *pSrc = img.GetPixelPtr( x, y );
    size_t   srcMod = bpr - bpp * w;

    for (u_int i=0; i != img.mChans; ++i)
    {
        float *pDes = (float *)out_block + LOS_BLOCK_SIZE2 * i;
        const U8 *pSrc2 = pSrc + sizeof(T) * i;

        for (u_int y=0; y != h; ++y)
        {
            for (u_int x=0; x != w; ++x)
            {
                *pDes++ = *(const T *)pSrc2;

                pSrc2 += bpp;
            }

            for (u_int x=w; x != LOS_BLOCK_SIZE; ++x)
                *pDes++ = 0;

            pSrc2 += srcMod;
        }

        u_int zeroH = LOS_BLOCK_SIZE - h;
        u_int zeroN = zeroH * LOS_BLOCK_SIZE;
        for (u_int j=0; j != zeroN; ++j)
            pDes[j] = 0;
    }
}

//==================================================================
template<typename T, int DO_OFF_MID>
void putBlock(
        image &img,
        U8 *pDes,
        int srcChanIdx,
        int desChanIdx,
        int w,
        int h,
        const float in_block[MAX_CHANS][LOS_BLOCK_SIZE2] )
{
    u_int bpp = (u_int)img.mBytesPerPixel;
    u_int bpr = (u_int)img.mBytesPerRow;

    size_t  desMod = bpr - bpp * w;
    size_t  srcMod = LOS_BLOCK_SIZE - w;

    auto *pSrc = (float *)in_block + LOS_BLOCK_SIZE2 * srcChanIdx;
    const auto *pDes2 = (U8 *)pDes + sizeof(T) * desChanIdx;

    constexpr int BITS_N = sizeof(T)*8;
    constexpr int MAX_VAL =              (1 << (BITS_N)) - 1;
    constexpr int OFF_VAL = DO_OFF_MID ? (1 << (BITS_N - 1)) : 0;

    for (int j=0; j != h; ++j)
    {
        for (u_int x=0; x != w; ++x)
        {
            int val = (int)*pSrc++ + OFF_VAL;

            *(T *)pDes2 = (T)DClamp( val, 0, MAX_VAL );

            pDes2 += bpp;
        }

        pDes2 += desMod;
        pSrc += srcMod;
    }
}

//==================================================================
template <u_int SCA_SHIFT>
DFORCEINLINE void downscaleBlock( float *pData )
{
    constexpr auto subSiz = 1 << SCA_SHIFT;
    constexpr auto ooSubSiz2 = 1.f / (1 << (SCA_SHIFT*2));

    for (int y=0; y < LOS_BLOCK_SIZE; y += subSiz)
    {
        for (int x=0; x < LOS_BLOCK_SIZE; x += subSiz)
        {
            float sum = 0;
            for (int yy=0; yy < subSiz; ++yy)
                for (int xx=0; xx < subSiz; ++xx)
                    sum += pData[ (x+xx) + (y+yy) * LOS_BLOCK_SIZE ];

            const auto dx = x >> SCA_SHIFT;
            const auto dy = y >> SCA_SHIFT;
            pData[ dx + dy * LOS_BLOCK_SIZE ] = sum * ooSubSiz2;
        }
    }
}

//==================================================================
// how many pixels per level ? (depends purely on the original image size)
static size_t calcPixelsPerLev( int origImgW, int origImgH )
{
    size_t pixelsN =
            ((origImgW + LOS_BLOCK_SIZE-1) & ~(LOS_BLOCK_SIZE-1)) *
            ((origImgH + LOS_BLOCK_SIZE-1) & ~(LOS_BLOCK_SIZE-1));

	auto pixelsPerLevN	= pixelsN / LOS_BLOCK_SIZE2;
    pixelsPerLevN = (pixelsPerLevN + ENTROPY_BLOCK_SIZE-1) & ~(ENTROPY_BLOCK_SIZE-1);

    return pixelsPerLevN;
}

//==================================================================
static void dec_MakeDequantTable(
                float deqTable[MAX_CHANS][LOS_BLOCK_SIZE2],
                u_int chans,
                DUT::MemReader &mr )
{
	const auto quality = (int)mr.ReadValue<U8>();
	const auto linearQ = LinearQualityFromPercentQuality( quality );

    for (u_int ch=0; ch != chans; ++ch)
        makeDequantTable_Linear( deqTable[ch], _sQuantTablesYUVA[ch], linearQ );
}

//==================================================================
template <typename QT>
struct DecodeBlocksParams
{
    image                      &mImg;
    Int2                       mOrigWH {0,0};
    float                      mDeqTable[MAX_CHANS][LOS_BLOCK_SIZE2];
    uptr<LOSLeveleDecode<QT>>  moLevs;
    u_int                      mSrcChansN = 0;
    u_int                      mDecodeChansMask = 0;

    DecodeBlocksParams( image &img, const ImgDLS_Head1 &srcHead, DUT::MemReader &mr )
        : mImg(img)
        , mOrigWH( (int)srcHead.width, (int)srcHead.height )
        , mSrcChansN( (u_int)srcHead.chans)
    {
        // which channels are actually used ?
        // special case for YUV411
        if ( mImg.IsYUV411() )
        {
            // Y or UV
            DASSERT( img.mChans == 1 || img.mChans == 2 );

            if ( img.mChans == 1 )
            {
                // decode channel 0 (Y)
                mDecodeChansMask = 1 << 0;
            }
            else
            if ( img.mChans == 2 )
            {
                // decode channel 1 and 2 (U and V)
                mDecodeChansMask = (1 << 1) | (1 << 2);
            }
        }
        else
        {
            // decode all channels
            for (u_int i=0; i != img.mChans; ++i)
                mDecodeChansMask |= 1 << i;
        }

        // create the dequantization table
        const auto srcChansN = (u_int)srcHead.chans;
        dec_MakeDequantTable( mDeqTable, srcChansN, mr );

        // create the levels (after the quantization table, because of the order
        //  of storage in the file)
        moLevs = std::make_unique<LOSLeveleDecode<QT>>(
                                srcChansN,
                                mDecodeChansMask,
                                calcPixelsPerLev( mOrigWH[0], mOrigWH[1] ),
                                mr );
    }
};

//==================================================================
template <typename T, typename QT, u_int SCA_SHIFT>
static void decodeBlocks( DecodeBlocksParams<QT> &par )
{
    auto &img = par.mImg;

    const auto yuv411 = !!(img.mFlags & image::FLG_IS_YUV411);
    const auto yuv444 = !!(img.mFlags & image::FLG_IS_YUV444);

	const auto dw        = (int)img.mW;
	const auto dh        = (int)img.mH;
    const auto desChansN = (int)img.mChans;

    const auto srcChansN = (int)par.mSrcChansN;
    const auto srcChMask = (int)par.mDecodeChansMask;

    const auto &levs = *par.moLevs;

    //
	size_t blockIdx = 0;
	for (int y=0; y < par.mOrigWH[1]; y += LOS_BLOCK_SIZE)
	{
		for (int x=0; x < par.mOrigWH[0]; x += LOS_BLOCK_SIZE, ++blockIdx)
		{
			float tmpBlocks[MAX_CHANS][LOS_BLOCK_SIZE2];

            // loop throught the source channels
            for (int ch=0; ch < srcChansN; ++ch)
            {
                if NOT( srcChMask & (1 << ch) )
                    continue;

                QT quantBlock[LOS_BLOCK_SIZE2];

                for (size_t i=0; i < LOS_BLOCK_SIZE2; ++i)
                {
                    const auto *pLevel = levs.GetBlockData( ch, i, blockIdx );

                    quantBlock[i] = 0;
                    for (u_int j=0; j != sizeof(QT); ++j)
                        quantBlock[i] |= (QT)pLevel[j] << (j*8);
                }

                DCT_From_ZZQuant(
                        tmpBlocks[ch],
                        par.mDeqTable[ch],
                        quantBlock,
                        LOS_BLOCK_SIZE2 );

                DDCT8Inv( tmpBlocks[ch] );

                if ( SCA_SHIFT )
                    downscaleBlock<SCA_SHIFT>( tmpBlocks[ch] );
            }

            const auto dx = x >> SCA_SHIFT;
            const auto dy = y >> SCA_SHIFT;

            const auto bw = (int)DMin( (int)LOS_BLOCK_SIZE, dw - dx );
            const auto bh = (int)DMin( (int)LOS_BLOCK_SIZE, dh - dy );

            auto *pDes = img.GetPixelPtr( dx, dy );

            if ( yuv411 )
            {
                if ( desChansN == 1 ) // Y
                {
                    putBlock<T,0>( img, pDes, 0, 0, bw, bh, tmpBlocks );
                }
                else // UV
                {
                    DASSERT( desChansN == 2 );
                    putBlock<T,1>( img, pDes, 1, 0, bw, bh, tmpBlocks );
                    putBlock<T,1>( img, pDes, 2, 1, bw, bh, tmpBlocks );
                }
            }
            else
            if ( yuv444 )
            {
                DASSERT( desChansN == 3 || desChansN == 4 );

                putBlock<T,0>( img, pDes, 0, 0, bw, bh, tmpBlocks );
                putBlock<T,1>( img, pDes, 1, 1, bw, bh, tmpBlocks );
                putBlock<T,1>( img, pDes, 2, 2, bw, bh, tmpBlocks );

                for (int i=3; i < desChansN; ++i)
                    putBlock<T,0>( img, pDes, i, i, bw, bh, tmpBlocks );
            }
            else
            if ( desChansN == 3 || desChansN == 4 )
            {
                blockYUVAToRGBAf(
                        tmpBlocks[0],
                        tmpBlocks[1],
                        tmpBlocks[2],
                        LOS_BLOCK_SIZE >> SCA_SHIFT,
                        LOS_BLOCK_SIZE >> SCA_SHIFT );

                for (int i=0; i < desChansN; ++i)
                    putBlock<T,0>( img, pDes, i, i, bw, bh, tmpBlocks );
            }
            else
            {
                for (int i=0; i < desChansN; ++i)
                    putBlock<T,0>( img, pDes, i, i, bw, bh, tmpBlocks );
            }
		}
	}
}

//==================================================================
template <typename T, typename QT>
void losSaveDLSBase( const image &img, DUT::MemWriterDynamic &mw, u_int quality )
{
    // multi-channel images must have 1-byte channels !
    DASSERT( img.mChans == 1 || sizeof(T) == 1 );

	u_int w = img.mW;
	u_int h = img.mH;
    u_int chans = img.mChans;

	int linearQuality = LinearQualityFromPercentQuality( (u_int)quality );

	float quantTable[MAX_CHANS][LOS_BLOCK_SIZE2];

    for (u_int ch=0; ch != chans; ++ch)
        makeQuantTable_Linear(
                quantTable[ch],
                _sQuantTablesYUVA[ch],
                linearQuality );

    size_t pixelsN =
            ((w + LOS_BLOCK_SIZE-1) & ~(LOS_BLOCK_SIZE-1)) *
            ((h + LOS_BLOCK_SIZE-1) & ~(LOS_BLOCK_SIZE-1));

	size_t	pixelsPerLevN	= pixelsN / LOS_BLOCK_SIZE2;
    pixelsPerLevN = (pixelsPerLevN + ENTROPY_BLOCK_SIZE-1) & ~(ENTROPY_BLOCK_SIZE-1);

    // all channels for now
    u_int encodeChansMask = 0;
    for (u_int ch=0; ch != chans; ++ch)
        encodeChansMask |= 1 << ch;

    LOSLevelEncode<QT> levs( chans, encodeChansMask, pixelsPerLevN );

    {
#ifdef SAVE_MT_PASS1
    DVec<std::future<void>> futures;
    futures.reserve( (h + LOS_BLOCK_SIZE - 1) / LOS_BLOCK_SIZE );
#endif

	size_t blockIdx = 0;
	for (u_int y=0; y < h; y += LOS_BLOCK_SIZE)
	{
        u_int bh = DMin( LOS_BLOCK_SIZE, h - y );

#ifdef SAVE_MT_PASS1
        futures.push_back( DUT::THREADPOOL->enqueue( [&,y,bh](){ //----
#endif

		for (u_int x=0; x < w; x += LOS_BLOCK_SIZE, ++blockIdx)
		{
            u_int bw = DMin( LOS_BLOCK_SIZE, w - x );

			float tmpBlocks[MAX_CHANS][LOS_BLOCK_SIZE2];
			grabBlock<T>( img, x, y, bw, bh, tmpBlocks );

            if ( chans == 3 || chans == 4 )
            {
                blockRGBAToYUVAf(
                        tmpBlocks[0],
                        tmpBlocks[1],
                        tmpBlocks[2],
                        bw,
                        bh );
            }

            for (u_int ch=0; ch != chans; ++ch)
            {
                float dctBlock[LOS_BLOCK_SIZE2];
                DDCT8Fwd( dctBlock, tmpBlocks[ch] );			

                QT quantBlock[LOS_BLOCK_SIZE2];

                ZZQuant_From_DCT( quantBlock, quantTable[ch], dctBlock );

                for (size_t i=0; i < LOS_BLOCK_SIZE2; ++i)
                {
                    QT val = quantBlock[i];

                    for (size_t j=0; j != sizeof(QT); ++j)
                    {
                        U8 v = (U8)(val >> (j*8));

                        if ( v )
                        {
                            levs.mMaxBytesN[ch][i] =
                                DMax( levs.mMaxBytesN[ch][i], (U8)(j+1) );
                        }

                        levs.mLevels3B[ch][i][blockIdx*sizeof(QT)+j] = v;
                    }
                }
            }
		}

#ifdef SAVE_MT_PASS1
        } ) ); //----

        while ( futures.size() >= 16 )
        {
            futures[0].get();
            futures.erase( futures.begin() );
        }
#endif
	}
#ifdef SAVE_MT_PASS1
    for (auto &f : futures)
        f.get();
#endif
    }

    //
	mw.WriteValue<U8>( (U8)quality );

    //
    levs.EncodeLevels( mw );
}

//==================================================================
void LOS_LoadDLS(
        image &img,
        DUT::MemReader &mr,
        const ImgDLS_Head1 &head1,
        int scalePow2Div )
{
    // determine automatically
    if ( scalePow2Div == -1 )
    {
        for (int i=MAX_SCALE_POW2_DIV; i >= 0; --i)
        {
            if ( ((u_int)head1.width >> i) == (u_int)img.mW &&
                 ((u_int)head1.height >> i) == (u_int)img.mH )
            {
                scalePow2Div = i;
            }
        }

        if ( scalePow2Div == -1 )
        {
            DASSERT( 0 );
            return;
        }
    }

    auto bytesPerChan = img.mBytesPerPixel / img.mChans;

    // make sure it's all even
    DASSERT( (bytesPerChan * img.mChans) == img.mBytesPerPixel );

    if ( bytesPerChan == 1 )
    {
        // decode the levels (8 bit source -> 16 bits)
        DecodeBlocksParams<U16> par( img, head1, mr );

        switch ( scalePow2Div )
        {
        case 0: decodeBlocks<U8,U16,0>( par ); break;
        case 1: decodeBlocks<U8,U16,1>( par ); break;
        case 2: decodeBlocks<U8,U16,2>( par ); break;
        default:
        case 3: decodeBlocks<U8,U16,3>( par ); break;
        }
    }
    else
    if ( bytesPerChan == 2 )
    {
        // decode the levels (16 bits source -> 32 bit (NOTE: maybe 24 would be ok ?))
        DecodeBlocksParams<U32> par( img, head1, mr );

        switch ( scalePow2Div )
        {
        case 0: decodeBlocks<U16,U32,0>( par ); break;
        case 1: decodeBlocks<U16,U32,1>( par ); break;
        case 2: decodeBlocks<U16,U32,2>( par ); break;
        default:
        case 3: decodeBlocks<U16,U32,3>( par ); break;
        }
    }
    else
    {
        DEX_RUNTIME_ERROR( "Unsupported number of bytes per channel !" );
    }
}

//==================================================================
void LOS_SaveDLS( const image &img, DUT::MemWriterDynamic &mw, u_int quality )
{
    //DUT::QuickProf prof("Saving");

    u_int bytesPerChan = img.mBytesPerPixel / img.mChans;

    // make sure it's all even
    DASSERT( (bytesPerChan * img.mChans) == img.mBytesPerPixel );

    if ( bytesPerChan == 1 )
        losSaveDLSBase<U8 ,U16>( img, mw, quality );
    else
    if ( bytesPerChan == 2 )
        losSaveDLSBase<U16,U32>( img, mw, quality );
    else
    {
        DEX_RUNTIME_ERROR( "Unsupported number of bytes per channel !" );
    }
}

//==================================================================
}

