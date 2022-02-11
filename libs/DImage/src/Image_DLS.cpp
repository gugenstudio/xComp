//==================================================================
/// @author Davide Pasca - davide@oyatsukai.com
/// @author Copyright(C) 2011 oyatsukai.com. All rights reserved.
/// @since 2011/5/25
/// @file image_DLS.cpp
//==================================================================

#include "stdafx.h"
#include "DUT_Str.h"
#include "Image_DLS.h"
#include "Image_DLS_LSS.h"
#include "Image_DLS_LOS.h"
#include "ImageConv.h"

//==================================================================
namespace ImgDLS
{

//==================================================================
template <typename _T>
void fillRect( image &img, const U32 col[MAX_CHANS] )
{
    int chansN = img.mChans;
    ImageConv::RectProcess<_T>( img,
            [=](_T *pPix) {
                for (int i=0; i < chansN; ++i)
                    pPix[i] = (_T)col[i];
            });
}

//==================================================================
template <typename _T>
void avgRect( const image &img, U32 col[MAX_CHANS] )
{
    static_assert( ImgDLS::MAX_CHANS == image::MAX_CHANS,
                   "DLS and Image should have same N of chans" );

    int chansN = img.mChans;

    double sums[MAX_CHANS];
    for (auto &s : sums)
        s = 0;

    auto pixN = (size_t)img.mW * img.mH;

    if ( pixN == 0 )
        return;

    ImageConv::RectProcess<_T>( img,
            [&](const _T *pPix) {
                for (int i=0; i < chansN; ++i)
                    sums[i] += (double)pPix[i];
            });

    for (int i=0; i < chansN; ++i)
        col[i] = (U32)(sums[i] / (double)pixN);
}

//==================================================================
static void fillImageWithChanVals( image &img, const U32 vals[MAX_CHANS] )
{
    // fill up the image with the single average color
    auto bytesPerChan = img.mBytesPerPixel / img.mChans;
    switch( bytesPerChan )
    {
    case 1: fillRect<U8 >( img, vals ); break;
    case 2: fillRect<U16>( img, vals ); break;
    case 4: fillRect<U32>( img, vals ); break;
    default: DASSERT(0); break;
    }
}

//==================================================================
void QuickLoadDLS(
        image &img,
        u_int imgFlagsAdd,
        u_int imgFlagsRem,
        const ImgDLS_Head1 &head1 )
{
    static const u_int DUMMY_SIZE = 8;

    image::Params par;
    par.width  = DUMMY_SIZE;
    par.height = DUMMY_SIZE;
    par.depth = head1.depth;
    par.chans = head1.chans;
    par.flags = head1.imgFlags;
    par.flags |= imgFlagsAdd;
    par.flags &= ~imgFlagsRem;
    // remove flags not needed for the dummy
    par.flags &= ~(image::FLG_USE_MIPMAPS |
                   image::FLG_IS_CUBEMAP);

    // override
    if ( (par.flags & image::FLG_IS_YUV411) )
    {
        par.subImgWidth  = DUMMY_SIZE;
        par.subImgHeight = DUMMY_SIZE;
        par.depth = 8;
        par.chans = 1;
    }

	img.Setup( par );

    // setup the average channels (convert to YUV if necessary)
    U32 useChansAvgs[MAX_CHANS];
    for (size_t i=0; i != MAX_CHANS; ++i)
        useChansAvgs[i] = head1.chansAvgs[i];

    auto rgb2yuv = []( auto &out_chans, const auto &in_chans )
    {
        auto r = (int)in_chans[0];
        auto g = (int)in_chans[1];
        auto b = (int)in_chans[2];
        out_chans[0] = (U32)g;
        out_chans[1] = (U32)DClamp( r - g + 128, 0, 255 );
        out_chans[2] = (U32)DClamp( b - g + 128, 0, 255 );
    };

    if ( (par.flags & image::FLG_IS_YUV411) )
    {
        // TODO411: add support
        // NOTE: will only use Y for now
        rgb2yuv( useChansAvgs, head1.chansAvgs );

        fillImageWithChanVals( img, useChansAvgs );

        if ( img.moSubImage )
        {
            // fill the image with UV
            U32 uvChans[MAX_CHANS] {};
            uvChans[0] = useChansAvgs[1];
            uvChans[1] = useChansAvgs[2];
            fillImageWithChanVals( *img.moSubImage, uvChans );
        }
    }
    else
    {
        if ( (par.flags & image::FLG_IS_YUV444) )
        {
            rgb2yuv( useChansAvgs, head1.chansAvgs );
        }

        fillImageWithChanVals( img, useChansAvgs );
    }
}

//==================================================================
void LoadDLS(
        image &img,
        u_int imgFlagsAdd,
        u_int imgFlagsRem,
        u_int scalePow2Div,
        DUT::MemReader &mr,
        ImgDLS_Head1 *pOut_Head )
{
	char fmtID[5];
	fmtID[4] = 0;
	mr.ReadArray( fmtID, 4 );

	if ( 0 != strcmp( fmtID, "DLS2" ) )
    {
		DEX_RUNTIME_ERROR( "Unsupported file format" );
    }

	ImgDLS_Head1 head1;
	head1 = mr.ReadValue<ImgDLS_Head1>();

    if ( pOut_Head )
        *pOut_Head = head1;

    // base image definition
    image::Params par;
    par.depth = head1.depth;
    par.chans = head1.chans;
    par.flags = head1.imgFlags;
    par.flags |= imgFlagsAdd;
    par.flags &= ~imgFlagsRem;

    if ( head1.encodeFmt == ENC_FMT_LOSSLESS1 )
    {
        DASSERT( !(par.flags & image::FLG_IS_YUV411) &&
                 !(par.flags & image::FLG_IS_YUV444) );

        // image size, as expected
        par.width  = head1.width;
        par.height = head1.height;

        if( (par.flags & image::FLG_IS_YUV411) )
        {
            // NOTE: doing just luminance for now
            auto bpp = par.depth / par.chans;
            par.chans = 1;
            par.depth = bpp * 1;
        }

        // setup the image
        img.Setup( par );

        LSS_LoadDLS( img, mr, head1 );

        // just scale afterwards and by 1 quarter for now
        if ( scalePow2Div >= 1 )
            img.ScaleQuarter();
    }
    else
    if ( head1.encodeFmt == ENC_FMT_LOSSY1 )
    {
        // scaled image size
        par.width  = head1.width  >> scalePow2Div;
        par.height = head1.height >> scalePow2Div;

        // override
        if ( (par.flags & image::FLG_IS_YUV411) )
        {
            par.depth = 8;
            par.chans = 1;

            auto subPow2Div = std::min( scalePow2Div + 1, (u_int)MAX_SCALE_POW2_DIV );
            par.subImgWidth  = head1.width  >> subPow2Div;
            par.subImgHeight = head1.height >> subPow2Div;
        }

        // setup the image
        img.Setup( par );

        const auto loadStartPos = mr.GetCurPos();
        LOS_LoadDLS( img, mr, head1 );

        if ( (par.flags & image::FLG_IS_YUV411) )
        {
            mr.SeekFromStart( loadStartPos );
            // decode the sub-image width UV
            LOS_LoadDLS( *img.moSubImage, mr, head1 );
        }
    }
    else
    {
        DEX_RUNTIME_ERROR( "Unknown encode format" );
    }
}

//==================================================================
void SaveDLS(
        const image &img,
        DUT::MemWriterDynamic &mw,
        u_int quality,
        DStr *pOutHeadStr )
{
    // not supporting float for now...
    DASSERT( !img.IsFloat16() && !img.IsFloat32() );

	mw.WriteArray( "DLS2", 4 );
	ImgDLS_Head1 head1;
	head1.depth		= (U8)img.mDepth;
	head1.chans		= (U8)img.mChans;

	if ( quality == 100 )
		head1.encodeFmt = ENC_FMT_LOSSLESS1;
	else
		head1.encodeFmt = ENC_FMT_LOSSY1;

	head1.reserved1	= 0;

	head1.width		= (U16)img.mW;
	head1.height	= (U16)img.mH;
	head1.bprow		= img.mBytesPerRow;
	head1.imgFlags	= img.mFlags;

    // set all to 0
    for (auto &a : head1.chansAvgs)
        a = 0;

    auto bytesPerChan = img.mBytesPerPixel / img.mChans;
    switch( bytesPerChan )
    {
    case 1: avgRect<U8 >( img, head1.chansAvgs ); break;
    case 2: avgRect<U16>( img, head1.chansAvgs ); break;
    case 4: avgRect<U32>( img, head1.chansAvgs ); break;
    default: DASSERT(0); break;
    }

	mw.WriteValue( head1 );

	if ( head1.encodeFmt == ENC_FMT_LOSSLESS1 )
		LSS_SaveDLS( img, mw );
	else
		LOS_SaveDLS( img, mw, quality );

    if ( pOutHeadStr )
    {
        *pOutHeadStr = MakeStrFromHead( head1 );
    }
}

//==================================================================
DStr MakeStrFromHead( const ImgDLS_Head1 &head )
{
    char buff[128];
    sprintf_s( buff, "dlsh_%u_%u_%u_%u_",
            (u_int)head.width,
            (u_int)head.height,
            (u_int)head.depth,
            (u_int)head.chans );

    auto str = DStr( buff );
    auto bitsPerChan = head.depth / head.chans;

    for (u_int i=0; i != (u_int)head.chans; ++i)
    {
        if ( bitsPerChan == 8 )
            sprintf_s( buff, "%02x", (u_int)DMin( head.chansAvgs[i], (U32)0xff ) );
        else
        if ( bitsPerChan == 16 )
            sprintf_s( buff, "%04x", (u_int)DMin( head.chansAvgs[i], (U32)0xffff ) );
        else
        if ( bitsPerChan == 32 )
            sprintf_s( buff, "%08x", (u_int)DMin( head.chansAvgs[i], (U32)0xffffffff ) );
        else
        {
            DASSERT( 0 );
        }

        str += buff;
    }

    sprintf_s( buff, "_%x", (u_int)head.imgFlags );
    str += buff;

    return str;
}

//==================================================================
unsigned long my_stoul( const DStr &str, u_int base )
{
    u_int val = 0;
    if ( base == 10 )
    {
        sscanf( str.c_str(), "%u", &val );
    }
    else
    if ( base == 16 )
    {
        sscanf( str.c_str(), "%x", &val );
    }
    else
    {
        DASSERT( 0 );
    }

    return val;
}

//==================================================================
bool MakeHeadFromStr( ImgDLS_Head1 &out_head, const DStr &fname )
{
    // zero everything in any case
    memset( &out_head, 0, sizeof(out_head) );

    // copy the substring in buff
    char buff[128];
    {
        if ( fname.size() > sizeof(buff) )
        {
            DEX_OUT_OF_RANGE( "Name header too long ! '%s'", fname.c_str() );
        }
        //memcpy_s( buff, sizeof(buff), pHeadStart, (size_t)headLen );
        memcpy( buff, &fname[0], fname.size() );
        buff[fname.size()] = 0;
    }

    // tokenize the mofo
    auto pTokVec = DUT::VecStrTok_StrQuot( buff, "_" );

    if ( pTokVec.size() != 7 )
        DEX_RUNTIME_ERROR( "Bad name header ! '%s'", fname.c_str() );

    //
    out_head.width    = (U16)my_stoul( pTokVec[1], 10 );
    out_head.height   = (U16)my_stoul( pTokVec[2], 10 );
    out_head.depth    = (U8 )my_stoul( pTokVec[3], 10 );
    out_head.chans    = (U8 )my_stoul( pTokVec[4], 10 );

    auto bitsPerChan = out_head.depth / out_head.chans;
    if ( bitsPerChan != 8 && bitsPerChan != 16 && bitsPerChan != 32 )
        DEX_RUNTIME_ERROR( "Bad name header ! '%s'", fname.c_str() );

    auto avgColsStr = DStr( pTokVec[5] );
    // make sure that the digits match !
    if ( avgColsStr.size() != (out_head.depth/4) )
        DEX_RUNTIME_ERROR( "Bad name header ! '%s'", fname.c_str() );

    auto digitsPerChan = bitsPerChan / 4;
    for (u_int i=0; i != (u_int)out_head.chans; ++i)
    {
        auto subStr = avgColsStr.substr( i * digitsPerChan, digitsPerChan );

        out_head.chansAvgs[i] = (U32)my_stoul( subStr, 16 );
    }

    //
    out_head.imgFlags = (U32)my_stoul( pTokVec[6], 16 );

    return true;
}

//==================================================================
bool MakeHeadFromFName( ImgDLS_Head1 &out_head, const DStr &fname )
{
    // zero everything in any case
    memset( &out_head, 0, sizeof(out_head) );

    // find the head start
    auto *pHeadStart = DUT::StrStrI( fname.c_str(), ".dlsh_" );
    if NOT( pHeadStart )
        return false;

    // find the head end (at next '.' or end of string)
    auto pHeadEnd = DUT::StrFindFirstOf( pHeadStart, '.' );
    if NOT( pHeadEnd )
        pHeadEnd = pHeadStart + strlen(pHeadStart);

    auto tmpStr = DStr( pHeadStart, pHeadEnd );

    return MakeHeadFromStr( out_head, tmpStr );
}

//==================================================================
}
