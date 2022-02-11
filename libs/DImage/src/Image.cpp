//==================================================================
/// image.cpp
///
/// Created by Davide Pasca - 2009/10/5
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#include "stdafx.h"
#include <memory.h>

#include "DUT_Files.h"

#include "Image_DLS.h"
#include "Image_RGBE.h"
#include "Image_JPEG.h"
#include "Image_PNG.h"

#include "Image.h"

//#define DLOG printf
#define DLOG(FMT,...)

static constexpr u_int DIMAGE_INVALID_OGL_OBJ_ID = (u_int)-1;

//==================================================================
DFun<void (image *)> image::msOnImageDestructFn;
DFun<u_int (const image *)> image::msOnImageGetTempTexIDFn;

//==================================================================
image::image()
{
}

//==================================================================
image::image( const image &from )
{
    *this = from;
}

//==================================================================
image &image::operator=( const image &from )
{
    Params par;
    par.pSrcImg = &from;
    par.doCopySrcImgPixels = true;
	Setup( par );

	return *this;
}

//==================================================================
image &image::operator=( image &&from ) noexcept
{
    mImageTexID0    = std::move( from.mImageTexID0 );
    mTexWd          = std::move( from.mTexWd );
    mTexHe          = std::move( from.mTexHe );
    mTexS2          = std::move( from.mTexS2 );
    mTexT2          = std::move( from.mTexT2 );
    mXtoS           = std::move( from.mXtoS  );
    mYtoT           = std::move( from.mYtoT  );

    mW              = std::move( from.mW );
    mH              = std::move( from.mH );
    mDepth          = std::move( from.mDepth );
    mChans          = std::move( from.mChans );
    mBytesPerPixel  = std::move( from.mBytesPerPixel );
    mBytesPerRow    = std::move( from.mBytesPerRow );
    mFlags          = std::move( from.mFlags );

    mPixels         = std::move( from.mPixels );

    // be safe, in case the address may have changed
    if ( from.mpData == from.mPixels.data() )
        mpData = mPixels.data();
    else
        mpData = std::move( from.mpData );

    moSubImage      = std::move( from.moSubImage );

    mLoadParams = std::move( from.mLoadParams );

    return *this;
}

//==================================================================
image::image( const Params &par )
{
	Setup( par );
}

//==================================================================
image::image( const LoadParams &par, ImgDLS::ImgDLS_Head1 *pOut_Head )
    : mLoadParams(par)
{
    Setup( par, pOut_Head );
}

//==================================================================
image::image( const char *pFName, u_int flags )
{
    LoadParams par;
    par.mLP_FName   = pFName;
    par.mLP_FlagsAdd = flags;

    Setup( par, nullptr );
}

//==================================================================
void image::Setup( const Params &par )
{
    DASSERT( !isInitialized() );

	DASSERT( (par.depth & 7) == 0 );

    // either float 16 or float 32, but not both !
	DASSERT( !(par.flags & FLG_IS_FLOAT16) || !(par.flags & FLG_IS_FLOAT32) );

	if ( par.pSrcImg )
	{
	    DLOG("this %p copied from %p", this, par.pSrcImg);

		DASSERT(
			par.width == 0 &&
			par.height == 0 &&
			par.chans == 0 &&
			par.rowPitch == 0 &&
            //par.autoRowPitchAlign == 0 &&
			par.pSrcData == NULL );

        if ( par.doCopySrcImgPixels )
            CopyImageData( *par.pSrcImg );
        else
            borrowData( *par.pSrcImg );
	}
	else
	{
    	DASSERT(
            par.width != 0 &&
            par.height != 0 &&
            par.chans != 0 &&
            par.chans <= MAX_CHANS );

		mW				= par.width;
		mH				= par.height;
		mDepth			= par.depth;
		mChans			= par.chans;
		mFlags			= par.flags;
		mBytesPerPixel	= par.depth / 8;

		if ( par.rowPitch )
			mBytesPerRow = par.rowPitch;
		else
        {
			mBytesPerRow = par.width * mBytesPerPixel;

            if ( par.autoRowPitchAlign )
            {
                u_int mask = par.autoRowPitchAlign-1;
                mBytesPerRow = (mBytesPerRow + mask) & ~mask;
            }
        }

		MakeTexVars(
                mW,
                mH,
                mTexWd,
                mTexHe,
                mXtoS,
                mYtoT,
                mTexS2,
                mTexT2 );

		if ( par.pUseData )
		{
			DLOG("this %p uses pixels", this);
			mpData = par.pUseData;
		}
		else
		{
			mPixels.resize( mH * mBytesPerRow );
			mpData = &mPixels[0];

			if ( par.pSrcData )
			{
				DLOG("this %p initialize from pixels", this);
				memcpy( &mPixels[0], par.pSrcData, mPixels.size() );
			}
			else
			{
				DLOG("this %p creates clear pixels", this);
				Clear();
			}
		}
	}

    if ( (mFlags & image::FLG_IS_YUV411) && mChans == 1 )
    {
        Params subPar;
		subPar.width  = par.subImgWidth;
		subPar.height = par.subImgHeight;
		subPar.depth  = 2 * 8;
		subPar.chans  = 2;
		subPar.flags  = par.flags;

        moSubImage = std::make_unique<image>( subPar );
    }
}

//==================================================================
void image::CopyImageDefinition( const image &from )
{
	DASSERT( !isInitialized() );

    mW              = from.mW;
    mH              = from.mH;
    mDepth          = from.mDepth;
    mChans          = from.mChans;
    mBytesPerPixel  = from.mBytesPerPixel;
    mBytesPerRow    = from.mBytesPerRow;

    // do nothing for pixels and data pointer
    //mPixels
    //mpData

    mXtoS           = from.mXtoS    ;
    mYtoT           = from.mYtoT    ;
    mTexS2          = from.mTexS2   ;
    mTexT2          = from.mTexT2   ;
    mTexWd          = from.mTexWd   ;
    mTexHe          = from.mTexHe   ;
    mFlags          = from.mFlags   ;

    mLoadParams     = from.mLoadParams;

    // NOTE: maybe support for sub-image ?
}

//==================================================================
void image::CopyImageData( const image &from )
{
    DASSERT( mpData == nullptr );

    mpData = nullptr;

    if ( from.mPixels.size() )
    {
        mPixels = from.mPixels;
    }
    else
    if ( from.mpData )
    {
        mPixels.clear();
        mPixels.insert(
                mPixels.end(),
                from.mpData,
                from.mpData + mH * mBytesPerRow );
    }

    mpData = &mPixels[0];

    if ( from.moSubImage )
        GetOrMakeSubImage()->CopyImageData( *from.moSubImage );
}

//==================================================================
void image::borrowData( const image &from )
{
    if ( from.mPixels.size() )
        mpData = mPixels.data();
    else
        mpData = from.mpData;

    if ( from.moSubImage )
        GetOrMakeSubImage()->borrowData( *from.moSubImage );
}

//==================================================================
void image::moveData( image &from )
{
    // can only move if "from" is the owner
    DASSERT( !mpData || mpData == mPixels.data() );

    mPixels = std::move( from.mPixels );
    mpData = mPixels.data();

    if ( from.moSubImage )
        GetOrMakeSubImage()->moveData( *from.moSubImage );
}

//==================================================================
bool image::isInitialized() const
{
    return
           mW              != 0
        || mH              != 0
        || mDepth          != 0
        || mChans          != 0
        || mBytesPerPixel  != 0
        || mBytesPerRow    != 0
        || mImageTexID0    != (u_int)-1
        || mXtoS           != 0
        || mYtoT           != 0
        || mTexWd          != 0
        || mTexHe          != 0
        || mTexS2          != 0
        || mTexT2          != 0
        || mFlags          != 0
        || mpData          != nullptr;
}

//==================================================================
void image::MakeTexVars(
        u_int useW,
        u_int useH,
        u_int &out_texW,
        u_int &out_texH,
	    float &out_XtoS,
	    float &out_YtoT,
	    float &out_texS2,
	    float &out_texT2 ) const
{
    if ( (mFlags & FLG_IS_CUBEMAP) )
    {
        DASSERT( useW == (useH*6) );
        useW = useH;
    }

    out_texW = useW;
    out_texH = useH;
    out_XtoS = 1.0f / out_texW;
    out_YtoT = 1.0f / out_texH;
    out_texS2 = (float)useW * out_XtoS;
    out_texT2 = (float)useH * out_YtoT;
}

//==================================================================
void image::ScaleQuarter()
{
    DASSERT( mW >= 2 && mH >= 2 );

    image::Params par;
    par.width       = mW / 2;
    par.height      = mH / 2;
    par.depth       = mDepth;
    par.chans       = mChans;
    par.flags       = mFlags;
    image tmp( par );

    tmp.mLoadParams = mLoadParams;

    ImageConv::BlitQuarter( *this, 0, 0, tmp, 0, 0, (int)tmp.mW, (int)tmp.mH );

    *this = std::move( tmp );
}

//==================================================================
void image::reload_loadFileData(
        int         fxSize,
        const U8*   &out_pData,
        size_t      &out_dataSize,
        DFileData   &out_fileData ) const
{
    if ( mLoadParams.mLP_DataSrc.size() )
    {
        out_pData    = &mLoadParams.mLP_DataSrc[0];
        out_dataSize = mLoadParams.mLP_DataSrc.size();
    }
    else
    {
        const auto *pFName = mLoadParams.mLP_FName.c_str();

        if NOT( DUT::GrabFile( pFName, out_fileData, fxSize ) )
            DEX_RUNTIME_ERROR( "Cloud not load '%s'", pFName );

        out_pData    = out_fileData.pData;
        out_dataSize = out_fileData.dataSize;
    }
}

//==================================================================
void image::reload_DLS( ImgDLS::ImgDLS_Head1 *pOut_Head )
{
    const U8    *pData;
    size_t      dataSize;
    DFileData   fileData;

    if ( mLoadParams.mLP_QuickLoad )
    {
        ImgDLS::ImgDLS_Head1 head;
        if NOT( ImgDLS::MakeHeadFromFName( head, mLoadParams.mLP_FName ) )
        {
            // for dls, may want to specify a fixed size to read only the header
            reload_loadFileData( ImgDLS::MIN_QUICKLOAD_SIZE, pData, dataSize, fileData );
            memcpy( &head, pData + 4, sizeof(head) );
        }

        ImgDLS::QuickLoadDLS(
            *this,
            mLoadParams.mLP_FlagsAdd,
            mLoadParams.mLP_FlagsRem,
            head );

        if ( pOut_Head )
            *pOut_Head = head;
    }
    else
    {
        // for dls, may want to specify a fixed size to read only the header
        reload_loadFileData( -1, pData, dataSize, fileData );

        DUT::MemReader mr( pData, dataSize );
        ImgDLS::LoadDLS(
                *this,
                mLoadParams.mLP_FlagsAdd,
                mLoadParams.mLP_FlagsRem,
                mLoadParams.mLP_ScalePow2Div,
                mr,
                pOut_Head );
    }
}

//==================================================================
void image::reload_Other( const char *pExt )
{
    const U8    *pData;
    size_t      dataSize;
    DFileData   fileData;

    // for every other format, read the whole file
    reload_loadFileData( -1, pData, dataSize, fileData );

    const auto flagsAdd = mLoadParams.mLP_FlagsAdd;
    //const auto flagsRem = mLoadParams.mLP_FlagsRem;

#if !defined(D_NOPNG)
    if ( 0 == strcasecmp( pExt, "png" ) )
    {
        bool forceAsSRGB = Image_PNG_DetectSRGBFromFileName( mLoadParams.mLP_FName );

        Image_PNGLoad( *this, flagsAdd, pData, dataSize, forceAsSRGB );
    }
    else
#endif
#if !defined(D_NOJPEG)
    if ( 0 == strcasecmp( pExt, "jpg" ) || 0 == strcasecmp( pExt, "jpeg" ) )
        Image_JPEGLoad( *this, flagsAdd, pData, dataSize );
    else
#endif
    if ( 0 == strcasecmp( pExt, "hdr" ) )
        Image_RGBELoad( *this, flagsAdd, pData, dataSize );
    else
    {
        DEX_RUNTIME_ERROR( "Unknown format for '%s'", mLoadParams.mLP_FName.c_str() );
    }

    // scale down
    if ( !mLoadParams.mLP_QuickLoad && mLoadParams.mLP_ScalePow2Div >= 1 )
    {
        ScaleQuarter();
    }
}

//==================================================================
void image::reloadImage( ImgDLS::ImgDLS_Head1 *pOut_Head )
{
	const char *pExt = DUT::GetFileNameExt( mLoadParams.mLP_FName.c_str() );

    // initialize the header all to 0, so that by looking at some
    // fields, who gets it in return can guess it has no tbeen filled
    if ( pOut_Head )
        memset( pOut_Head, 0, sizeof(*pOut_Head) );

    try
    {
        DASSERT( mLoadParams.mLP_ScalePow2Div <= 3 );

        if ( 0 == strcasecmp( pExt, "dls" ) )
            reload_DLS( pOut_Head );
        else
            reload_Other( pExt );
    }
    catch ( ... )
    {
        DEX_RUNTIME_ERROR( "Cloud not load '%s'", mLoadParams.mLP_FName.c_str() );
    }
}

//==================================================================
void image::Setup( const LoadParams &par, ImgDLS::ImgDLS_Head1 *pOut_Head )
{
    // must have a file name anyway even if fictitious
    DASSERT( par.mLP_FName.size() != 0 );

    mLoadParams = par;

    reloadImage( pOut_Head );
}

//==================================================================
image::~image()
{
    if ( msOnImageDestructFn )
        msOnImageDestructFn( this );
}

//==================================================================
void image::Clear()
{
	size_t useBPR;

    // clear the whole thing if it owning the image.. to have 0s
    // also in the padding area..
    // otherwise play safe, because the discepancy between brp and
    // w * bpp may not be simply padding..
    if ( mpData == mPixels.data() )
        useBPR = mBytesPerRow;
    else
        useBPR = mW * mBytesPerPixel;

	for (u_int y=0; y != mH; ++y)
	{
		U8	*pDataRow = GetPixelPtr( 0, y );

		for (u_int i=0; i != useBPR; ++i)
			pDataRow[i] = 0;
	}
}

//==================================================================
void image::FreeImageData()
{
    // only clear if it's our own data
    if ( mpData == mPixels.data() )
        mpData = nullptr;

    // clear and deallocate anyway
    mPixels.clear();
    mPixels.shrink_to_fit();

    if ( moSubImage )
        moSubImage->FreeImageData();
}

//==================================================================
void image::CopyDefTranserData( image &from )
{
    CopyImageDefinition( from );
    moveData( from );

    if ( from.moSubImage )
        GetOrMakeSubImage()->CopyDefTranserData( *from.moSubImage );
}

//==================================================================
u_int image::GetTextureID() const
{
	if ( mImageTexID0 == DIMAGE_INVALID_OGL_OBJ_ID && msOnImageGetTempTexIDFn )
        return msOnImageGetTempTexIDFn( this );

	return mImageTexID0;
}

//==================================================================
bool image::IsTextureIDValid() const
{
    DLOG("this %p", this);
	return mImageTexID0 != (u_int)-1;
}

//==================================================================
u_int image::GetSubTextureID() const
{
    if ( moSubImage )
        return moSubImage->GetTextureID();

	return (u_int)-1;
}

//==================================================================
bool image::IsSubTextureIDValid() const
{
    if ( moSubImage )
        return moSubImage->IsTextureIDValid();

	return false;
}

