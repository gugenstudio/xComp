//==================================================================
/// Image_EXR.cpp
///
/// Created by Davide Pasca - 2022/01/29
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#if defined(ENABLE_OPENEXR)

#include <stdio.h>
#include <stdlib.h>

#include <set>

#include <ImfNamespace.h>

//#include <ImfRgbaFile.h>
#include <ImfInputFile.h>
#include <ImfHeader.h>
#include <ImfStringAttribute.h>
#include <ImfMatrixAttribute.h>
#include <ImfArray.h>
#include <ImfChannelList.h>

#include <ImathBox.h>
#include <ImfFrameBuffer.h>

#include "DLogOut.h"

#include "Image_EXR.h"

#define IEXR_MAKE_FLOAT_IMAGES

namespace IMF = OPENEXR_IMF_NAMESPACE;
using namespace IMATH_NAMESPACE;

//==================================================================
#ifdef IMAGE_EXR_KEEP_FILE_OPEN
//
struct ImageEXRFileWork
{
    uptr<IMF::InputFile>    moInputFile;

    ImageEXRFileWork( const DStr &pathFName )
        : moInputFile(std::make_unique<IMF::InputFile>( pathFName.c_str() ))
    {
    }
};

//
ImageEXR::~ImageEXR() = default;
#endif

//==================================================================
ImageEXR::ImageEXR( const DStr &pathFName )
    : ie_pathFName(pathFName)
{
#ifdef IMAGE_EXR_KEEP_FILE_OPEN
    ie_oFileWork = std::make_unique<ImageEXRFileWork>( ie_pathFName );
#endif
}

//==================================================================
void ImageEXRChan::AllocDataSrc( size_t w, size_t h )
{
    iec_dataSrc.resize( w * h * ImageEXR_GetDTypeSize( iec_dataType ) );
}

//==================================================================
static auto makeIEXRDType = []( auto imfDType )
{
    switch ( imfDType )
    {
    case IMF::PixelType::UINT : return IEXR_DTYPE_UINT ;
    case IMF::PixelType::HALF : return IEXR_DTYPE_HALF ;
    case IMF::PixelType::FLOAT: return IEXR_DTYPE_FLOAT;

    default:
        DEX_RUNTIME_ERROR( "Unknown PixelType %zu", (size_t)imfDType );
        break;
    }
};

//==================================================================
static auto makeIMFPixType = []( auto iexrDType )
{
    switch ( iexrDType )
    {
    case IEXR_DTYPE_UINT : return IMF::PixelType::UINT ;
    case IEXR_DTYPE_HALF : return IMF::PixelType::HALF ;
    case IEXR_DTYPE_FLOAT: return IMF::PixelType::FLOAT;

    default:
        DEX_RUNTIME_ERROR( "Unknown IEXR Type %zu", (size_t)iexrDType );
        break;
    }
};

//==================================================================
uptr<ImageEXR> ImageEXR_Load( const DStr &pathFName, const DStr &dummyLayerName )
{
    auto oIE = std::make_unique<ImageEXR>( pathFName );

#ifdef IMAGE_EXR_KEEP_FILE_OPEN
    auto *pFile = oIE->ie_oFileWork->moInputFile.get();
#else
    IMF::InputFile localFile( oIE->ie_pathFName.c_str() );
    auto *pFile = &localFile;
#endif

    {
        // get the image size right away
        c_auto dw = pFile->header().dataWindow();
        oIE->ie_w = dw.max.x - dw.min.x + 1;
        oIE->ie_h = dw.max.y - dw.min.y + 1;
    }

    const auto &channels = pFile->header().channels();

    std::set<std::string> layerNames;

    channels.layers( layerNames );

    if ( layerNames.empty() )
    {
        // add a dummy layer
        auto oDummyLayer = std::make_unique<ImageEXRLayer>();
        oDummyLayer->iel_name = dummyLayerName;

        for (auto j = channels.begin(); j != channels.end(); ++j)
        {
            auto &ch = oDummyLayer->iel_chans.emplace_back();
            ch.iec_dataType = makeIEXRDType( j.channel().type );
            ch.iec_chanName = j.name();
        }

        oIE->ie_layers.push_back( std::move( oDummyLayer ) );
    }
    else
    {
        // add a dummy layer in case there's also an image outside layers
        uptr<ImageEXRLayer> oDummyLayer;
        for (auto j = channels.begin(); j != channels.end(); ++j)
        {
            if ( StrStrI_( j.name(), "." ) )
                continue;

            if NOT( oDummyLayer )
            {
                oDummyLayer = std::make_unique<ImageEXRLayer>();
                oDummyLayer->iel_name = dummyLayerName;
            }

            auto &ch = oDummyLayer->iel_chans.emplace_back();
            ch.iec_dataType = makeIEXRDType( j.channel().type );
            ch.iec_chanName = j.name();
        }

        if ( oDummyLayer )
            oIE->ie_layers.push_back( std::move( oDummyLayer ) );

        // add the remaining layers
        for (auto i = layerNames.begin(); i != layerNames.end(); ++i)
        {
            IMF::ChannelList::ConstIterator imfChanBegin, imfChanEnd;

            channels.channelsInLayer( *i, imfChanBegin, imfChanEnd );

            auto oLayer = std::make_unique<ImageEXRLayer>();
            oLayer->iel_name = *i;

            for (auto j = imfChanBegin; j != imfChanEnd; ++j)
            {
                auto &ch = oLayer->iel_chans.emplace_back();
                ch.iec_dataType = makeIEXRDType( j.channel().type );
                ch.iec_chanName = j.name();
            }

            oIE->ie_layers.push_back( std::move( oLayer ) );
        }
    }

    return oIE;
}

//==================================================================
void ImageEXR_LoadLayer( ImageEXR &ie, const DStr &loadLayerName )
{
    auto *pLayer = ie.FindLayerByName( loadLayerName );
    if NOT( pLayer )
    {
        LogOut( LOG_ERR, "Could not find layer %s", loadLayerName.c_str() );
        return;
    }

    LogOut( LOG_DBG, SSPrintFS( "Loading layer %s pixel data from %s",
                loadLayerName.c_str(), ie.ie_pathFName.c_str() ) );

#ifdef IMAGE_EXR_KEEP_FILE_OPEN
    auto *pFile = ie.ie_oFileWork->moInputFile.get();
#else
    IMF::InputFile localFile( ie.ie_pathFName.c_str() );
    auto *pFile = &localFile;
#endif

    c_auto dw = pFile->header().dataWindow();

    IMF::FrameBuffer frameBuffer;

    for (size_t i = 0; i < pLayer->iel_chans.size(); ++i)
    {
        auto &ch = pLayer->iel_chans[i];

        ch.AllocDataSrc( ie.ie_w, ie.ie_h );

        c_auto typeSize = ImageEXR_GetDTypeSize( ch.iec_dataType );

        frameBuffer.insert( ch.iec_chanName,                // name
            IMF::Slice( makeIMFPixType( ch.iec_dataType ),  // type
                (char *)(ch.iec_dataSrc.data() - dw.min.x - dw.min.y * ie.ie_w), // base
                typeSize * 1,                               // xStride
                typeSize * ie.ie_w,                         // yStride
                1, 1,                                       // x/y sampling
                0.0 ) );                                    // fillValue
    }

    pFile->setFrameBuffer( frameBuffer );
    pFile->readPixels( dw.min.y, dw.max.y );
}

//==================================================================
static auto convertSrcImage = [](
            auto &desImage,
            c_auto dtype,
            c_auto useChans,
            c_auto desChI,
            c_auto &srcVec )
{
    c_auto w = (size_t)desImage.mW;
    c_auto h = (size_t)desImage.mH;

    switch ( dtype )
    {
    case IEXR_DTYPE_UINT :
    {
        for (size_t y=0; y < h; ++y)
        {
            c_auto *pSrc = (const uint32_t *)srcVec.data() + y * w;

            if ( desImage.IsFloat32() )
            {
                auto *pDes = (float *)desImage.GetPixelPtr( 0, (u_int)y );
                for (size_t x=0; x < w; ++x)
                    pDes[ x * useChans + desChI ] = (float)pSrc[x];
            }
            else
            {
                auto *pDes = (uint8_t *)desImage.GetPixelPtr( 0, (u_int)y );
                for (size_t x=0; x < w; ++x)
                    pDes[ x * useChans + desChI ] =
                        (uint8_t)DClamp( pSrc[x], (uint32_t)0, (uint32_t)255 );
            }
        }
    }
    break;

    case IEXR_DTYPE_HALF :
    {
        for (size_t y=0; y < h; ++y)
        {
            c_auto *pSrc = (const uint16_t *)srcVec.data() + y * w;
            if ( desImage.IsFloat32() )
            {
                auto *pDes = (float *)desImage.GetPixelPtr( 0, (u_int)y );
                for (size_t x=0; x < w; ++x)
                    pDes[ x * useChans + desChI ] = imath_half_to_float( pSrc[x] );
            }
            else
            {
                auto *pDes = (uint8_t *)desImage.GetPixelPtr( 0, (u_int)y );
                for (size_t x=0; x < w; ++x)
                {
                    c_auto valF = imath_half_to_float( pSrc[x] );

                    pDes[ x * useChans + desChI ] =
                        (uint8_t)DClamp( valF * 255.f, 0.f, 255.f );
                }
            }
        }
    }
    break;

    case IEXR_DTYPE_FLOAT:
    {
        for (size_t y=0; y < h; ++y)
        {
            c_auto *pSrc = (const float *)srcVec.data() + y * w;
            if ( desImage.IsFloat32() )
            {
                auto *pDes = (float *)desImage.GetPixelPtr( 0, (u_int)y );
                for (size_t x=0; x < w; ++x)
                    pDes[ x * useChans + desChI ] = pSrc[x];
            }
            else
            {
                auto *pDes = (uint8_t *)desImage.GetPixelPtr( 0, (u_int)y );
                for (size_t x=0; x < w; ++x)
                    pDes[ x * useChans + desChI ] =
                        (uint8_t)DClamp( pSrc[x] * 255.f, 0.f, 255.f );
            }
        }
    }
    break;

    default: DEX_RUNTIME_ERROR( "Impossible data type for channe;" );
    }
};

//==================================================================
uptr<image> ImageEXR_MakeImageFromLayer( ImageEXRLayer &layer, const ImageEXR &ie )
{
    if NOT( layer.IsLayerDataLoaded() )
        DEX_RUNTIME_ERROR( "Source layer data was not loaded" );

    c_auto useChans = std::min( (size_t)4, layer.iel_chans.size() );

    image::Params par;
    par.width  = (u_int)ie.ie_w;
    par.height = (u_int)ie.ie_h;
    par.chans  = (u_int)useChans;
#ifdef IEXR_MAKE_FLOAT_IMAGES
    par.depth  = (u_int)useChans * 32;
    par.flags  = image::FLG_IS_FLOAT32;
#else
    par.depth  = (u_int)useChans * 8;
#endif

    auto oImage = std::make_unique<image>( par );

    for (size_t chI=0; chI < layer.iel_chans.size(); ++chI)
    {
        c_auto &ch = layer.iel_chans[ chI ];

        c_auto desChI = useChans - chI - 1;

        convertSrcImage(
                *oImage,
                ch.iec_dataType,
                useChans,
                desChI,
                ch.iec_dataSrc );
    }

    return oImage;
}

#endif

