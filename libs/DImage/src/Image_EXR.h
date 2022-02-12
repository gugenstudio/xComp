//==================================================================
/// Image_EXR.h
///
/// Created by Davide Pasca - 2022/01/29
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#ifndef IMAGE_EXR_H
#define IMAGE_EXR_H

#if defined(ENABLE_OPENEXR)

//#define IMAGE_EXR_KEEP_FILE_OPEN

#include "Image.h"

//
enum ImageEXRDType
{
    IEXR_DTYPE_UINT,
    IEXR_DTYPE_HALF,
    IEXR_DTYPE_FLOAT,
};

//
inline size_t ImageEXR_GetDTypeSize( ImageEXRDType type )
{
    switch ( type )
    {
    case IEXR_DTYPE_UINT : return sizeof(uint32_t);
    case IEXR_DTYPE_HALF : return sizeof(uint16_t);
    case IEXR_DTYPE_FLOAT: return sizeof(float);
    default: return 0;
    }
}

struct ImageEXRChan
{
    DStr            iec_chanName;
    ImageEXRDType   iec_dataType { IEXR_DTYPE_FLOAT };
    DVec<uint8_t>   iec_dataSrc;

    void AllocDataSrc( size_t w, size_t h );
};

//
struct ImageEXRLayer
{
    DStr                iel_name;
    DVec<ImageEXRChan>  iel_chans;

    bool IsLayerDataLoaded() const
    {
        for (c_auto &ch : iel_chans)
            if ( ch.iec_dataSrc.empty() )
                return false;

        return true;
    }
};

//
#ifdef IMAGE_EXR_KEEP_FILE_OPEN
struct ImageEXRFileWork;
#endif

//
struct ImageEXR
{
    size_t                      ie_w {};
    size_t                      ie_h {};
    DStr                        ie_pathFName;
    DVec<uptr<ImageEXRLayer>>   ie_layers;

    ImageEXR( const DStr &pathFName );
#ifdef IMAGE_EXR_KEEP_FILE_OPEN
private:
    uptr<ImageEXRFileWork>      ie_oFileWork;
public:
    ~ImageEXR();
#endif

    const ImageEXRLayer *FindLayerByName( const DStr &name ) const
    {
        for (c_auto &oL : ie_layers)
            if ( oL->iel_name == name )
                return oL.get();

        return nullptr;
    }

    ImageEXRLayer *FindLayerByName( const DStr &name )
    {
        for (c_auto &oL : ie_layers)
            if ( oL->iel_name == name )
                return oL.get();

        return nullptr;
    }
};

//==================================================================
uptr<ImageEXR> ImageEXR_Load( const DStr &pathFName, const DStr &dummyLayerName );
void ImageEXR_LoadLayer( ImageEXR &ie, const DStr &loadLayerName );
uptr<image> ImageEXR_MakeImageFromLayer( ImageEXRLayer &layer, const ImageEXR &ie );

#endif

#endif

