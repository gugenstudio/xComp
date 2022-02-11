//==================================================================
/// Image_JPEG.h
///
/// Created by Davide Pasca - 2010/12/6
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#ifndef IMAGE_JPEG_H
#define IMAGE_JPEG_H

#if !defined(D_NOJPEG)

#include "Image.h"

//==================================================================
void Image_JPEGSave( const image &img, const char *pFName, bool flipY=false );
void Image_JPEGLoad( image &img, u_int imgBaseFlags, const U8 *pData, size_t dataSize );

#endif

#endif
