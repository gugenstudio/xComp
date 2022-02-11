//==================================================================
/// Image_DLS_LSS.cpp
///
/// Created by Davide Pasca - 2012/5/31
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#include "stdafx.h"
#include "DLZ2.h"
#include "DSPPack.h"
#include "Image_DLS.h"
#include "Image_DLS_LSS.h"

//#define DLOG printf
#define DLOG(FMT,...)

#define USE_YUV

//==================================================================
namespace ImgDLS
{

//==================================================================
static const u_int LSS_BLOCK_DIM = 16;

//==================================================================
template <bool DO_ALPHA>
void blockRGBAToYUVA(
				U16	*pDesYUV_A,
				const U8 *pData,
				size_t pitchRow,
                size_t wd,
				size_t he )
{
	static const size_t	SAMP_N = DO_ALPHA ? 4 : 3;

	size_t	dataRowMod = pitchRow - wd * SAMP_N;
	size_t	desRowMod = (LSS_BLOCK_DIM - wd) * SAMP_N;

	for (size_t y=0; y < he; ++y)
	{
		for (size_t x=0; x != wd; ++x)
		{
			int	r = pData[0];
			int	g = pData[1];
			int	b = pData[2];

			int yg = g;
			int cr = r - g + 255;
			int cb = b - g + 255;

			pDesYUV_A[0] = (U16)yg;
			pDesYUV_A[1] = (U16)cr;
			pDesYUV_A[2] = (U16)cb;

			if ( DO_ALPHA )
				pDesYUV_A[3] = pData[3];

			pData += SAMP_N;
			pDesYUV_A += SAMP_N;
		}

		pData += dataRowMod;
    	pDesYUV_A += desRowMod;
	}
}

//==================================================================
template <bool DO_ALPHA>
static void blockYUVAToRGBA(
				U8	*pDesRGB_A,
				const U16 *pData,
				size_t pitchRow,
                size_t wd,
				size_t he )
{
	static const size_t	SAMP_N = DO_ALPHA ? 4 : 3;

	size_t	dataMod = (LSS_BLOCK_DIM - wd) * SAMP_N;
	size_t	desMod = pitchRow - wd * SAMP_N;

	for (size_t y=0; y < he; ++y)
	{
		for (size_t x=0; x != wd; ++x)
		{
			int	yg = (int)pData[0];
			int	cr = (int)pData[1] - 255;
			int	cb = (int)pData[2] - 255;

			int g = yg;

			U8 r = cr + g;
			U8 b = cb + g;

			pDesRGB_A[0] = (U8)r;
			pDesRGB_A[1] = (U8)g;
			pDesRGB_A[2] = (U8)b;

			if ( DO_ALPHA )
				pDesRGB_A[3] = (U8)pData[3];

			pData += SAMP_N;
			pDesRGB_A += SAMP_N;
		}

        pData += dataMod;
		pDesRGB_A += desMod;
	}
}

//==================================================================
void LSS_LoadDLS( image &img, DUT::MemReader &mr, const ImgDLS_Head1 &head1 )
{
    DUT::MemReader mrFlags[MAX_CHANS];
    DUT::MemReader mrBase[MAX_CHANS];
    DUT::MemReader mrBlock[MAX_CHANS];

    DUT::MemWriterDynamic mwTmp;
    for (size_t i=0; i != head1.chans; ++i)
    {
        DLZ2_ExpandReadSizes( mwTmp, mr ); mrFlags[i].InitOwnMemWriterDyn( mwTmp );
        DLZ2_ExpandReadSizes( mwTmp, mr );  mrBase[i].InitOwnMemWriterDyn( mwTmp );
        DLZ2_ExpandReadSizes( mwTmp, mr ); mrBlock[i].InitOwnMemWriterDyn( mwTmp );
    }

#if defined(USE_YUV)
	U16	tmpBlk[4 * LSS_BLOCK_DIM * LSS_BLOCK_DIM];
#endif

	u_int	bpp = img.mBytesPerPixel;

	for (size_t y=0; y < img.mH; y += LSS_BLOCK_DIM)
	{
        DLOG("y = %d", (int )y);

		size_t he = DMin( (size_t)LSS_BLOCK_DIM, (size_t)(img.mH - y) );

		for (size_t x=0; x < img.mW; x += LSS_BLOCK_DIM)
		{
            size_t wd = DMin( (size_t)LSS_BLOCK_DIM, (size_t)(img.mW - x) );

			U8 *pData = img.GetPixelPtr( (u_int)x, (u_int)y );

			if ( bpp == head1.chans )
			{
			#if defined(USE_YUV)
				if ( head1.chans == 3 || head1.chans == 4 )
				{
					for (size_t i=0; i < head1.chans; ++i)
                    {
                        DSPPack::DecodeBlockMinMax(
                                mrFlags[i],
                                mrBase[i],
                                mrBlock[i],
                                tmpBlk + i,
                                head1.chans,
                                LSS_BLOCK_DIM*head1.chans,
                                wd,
                                he );
                    }

					if ( head1.chans == 3 )
						blockYUVAToRGBA<false>( pData, tmpBlk, img.mBytesPerRow, wd, he );
					else
						blockYUVAToRGBA<true>( pData, tmpBlk, img.mBytesPerRow, wd, he );

				}
				else
			#endif
				for (size_t i=0; i < head1.chans; ++i)
				{
                    DSPPack::DecodeBlockMinMax(
                            mrFlags[i],
                            mrBase[i],
                            mrBlock[i],
							pData + i,
                            head1.chans,
							img.mBytesPerRow,
                            wd,
                            he );
				}
			}
			else
			if ( bpp == 2 )
			{
				for (size_t i=0; i < head1.chans; ++i)
				{
                    DSPPack::DecodeBlockMinMax(
                            mrFlags[i],
                            mrBase[i],
                            mrBlock[i],
							(u_short *)pData + i,
                            head1.chans,
							img.mBytesPerRow / sizeof(u_short),
                            wd,
                            he );
				}
			}
			else
			{
				DEX_RUNTIME_ERROR( "Unsupported image format bpp:%u head1.chans:%u", bpp, head1.chans );
			}
		}
	}
    DLOG("left for-loop");
}

//==================================================================
void LSS_SaveDLS( const image &img, DUT::MemWriterDynamic &mw )
{
#if defined(USE_YUV)
	U16	tmpBlk[4 * LSS_BLOCK_DIM * LSS_BLOCK_DIM];
#endif

	u_int	bpp = img.mBytesPerPixel;
	u_int	chans = img.mChans;

    DUT::MemWriterDynamic mwFlags[MAX_CHANS];
    DUT::MemWriterDynamic mwBase[MAX_CHANS];
    DUT::MemWriterDynamic mwBlock[MAX_CHANS];

	for (size_t y=0; y < img.mH; y += LSS_BLOCK_DIM)
	{
		size_t he = DMin( (size_t)LSS_BLOCK_DIM, (size_t)(img.mH - y) );

		for (size_t x=0; x < img.mW; x += LSS_BLOCK_DIM)
		{
            size_t wd = DMin( (size_t)LSS_BLOCK_DIM, (size_t)(img.mW - x) );

			const U8 *pData = img.GetPixelPtr( (u_int)x, (u_int)y );

			if ( bpp == chans )
			{
			#if defined(USE_YUV)
				if ( chans == 3 || chans == 4 )
				{
					if ( chans == 3 )
						blockRGBAToYUVA<false>( tmpBlk, pData, img.mBytesPerRow, wd, he );
					else
						blockRGBAToYUVA<true>( tmpBlk, pData, img.mBytesPerRow, wd, he );

					for (size_t i=0; i < chans; ++i)
						DSPPack::EncodeBlockMinMax(
                            mwFlags[i],
                            mwBase[i],
                            mwBlock[i],
                            tmpBlk + i,
                            chans,
                            LSS_BLOCK_DIM*chans,
                            wd,
                            he );
				}
				else
			#endif
				for (size_t i=0; i != chans; ++i)
				{
                    DSPPack::EncodeBlockMinMax(
                        mwFlags[i],
                        mwBase[i],
                        mwBlock[i],
                        pData + i,
                        chans,
                        img.mBytesPerRow,
                        wd,
                        he );
				}
			}
			else
			if ( bpp == 2 )
			{
				for (size_t i=0; i != chans; ++i)
				{
                    DSPPack::EncodeBlockMinMax(
                        mwFlags[i],
                        mwBase[i],
                        mwBlock[i],
                        (const u_short *)pData + i,
                        chans,
                        img.mBytesPerRow / sizeof(u_short),
                        wd,
                        he );
				}
			}
			else
			{
				DEX_RUNTIME_ERROR( "Unsupported image format bpp:%u chans:%u", bpp, chans );
			}
		}
	}

    for (size_t i=0; i != chans; ++i)
    {
        DLZ2_CompressWriteSizes( mw, mwFlags[i].GetDataBegin(), mwFlags[i].GetCurSize() );
        DLZ2_CompressWriteSizes( mw, mwBase[i] .GetDataBegin(), mwBase[i] .GetCurSize() );
        DLZ2_CompressWriteSizes( mw, mwBlock[i].GetDataBegin(), mwBlock[i].GetCurSize() );
    }
}

#if 0
	size_t	rowLenBytes = (size_t)img.mW * img.mBytesPerPixel;

	for (size_t y=0; y < img.mH; ++y)
	{
		U8	*pData = img.GetPixelPtr( 0, y );
		mr.ReadArray( pData, rowLenBytes );
	}

	size_t	rowLenBytes = (size_t)img.mW * img.mBytesPerPixel;

	for (size_t y=0; y < img.mH; ++y)
	{
		const U8	*pData = img.GetPixelPtr( 0, y );
		mw.WriteArray( pData, rowLenBytes );
	}
#endif

//==================================================================
}

