//==================================================================
/// Image_PNG.h
///
/// Created by Davide Pasca - 2011/7/16
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#ifndef IMAGE_PNG_H
#define IMAGE_PNG_H

#if !defined(D_NOPNG)

#include "Image.h"

//==================================================================
void Image_PNGSave( const image &img, const char *pFName, bool flipY );

void Image_PNGLoad( image &img, u_int imgBaseFlags, const char *pFName );

void Image_PNGLoad(
        image &img,
        u_int imgBaseFlags,
        const U8 *pData,
        size_t dataSize,
        bool forceAsSRGB );

bool Image_PNG_DetectSRGBFromFileName( const DStr &fname );

#endif

#endif
