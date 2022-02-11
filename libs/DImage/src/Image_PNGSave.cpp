//==================================================================
/// Image_PNGSave.cpp
///
/// Created by Davide Pasca - 2011/6/12
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#if !defined(D_NOPNG)

#include "stdafx.h"
#include "png.h"
#include "Image_PNG.h"

//==================================================================
static void convert_16_to_BigEndian( image &img )
{
	u_int wd = img.mW;
	u_int he = img.mH;

	for (u_int y=0; y < he; ++y)
	{
		U8	*pRow = img.GetPixelPtr( 0, y );

		for (u_int i=0; i < wd; ++i)
		{
			U8	a = pRow[ i * 2 + 0 ];
			U8	b = pRow[ i * 2 + 1 ];

			pRow[ i * 2 + 0 ] = b;
			pRow[ i * 2 + 1 ] = a;
		}
	}
}

//==================================================================
void Image_PNGSave( const image &img, const char *pFName, bool flipY )
{
    auto *fp = fopen (pFName, "wb");
    if NOT( fp )
        DEX_RUNTIME_ERROR( "Failed to open '%s' for writing", pFName );

    png_structp png_ptr = png_create_write_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if NOT( png_ptr )
        DEX_RUNTIME_ERROR( "Failed to create the PNG write struct" );

    png_infop info_ptr = png_create_info_struct (png_ptr);
    if NOT( info_ptr )
        DEX_RUNTIME_ERROR( "Failed to create the PNG struct info" );

	int	colorType;

    int depth = 8;

	if ( img.mDepth == 32 )
		colorType = PNG_COLOR_TYPE_RGB_ALPHA;
	else
	if ( img.mDepth == 24 )
		colorType = PNG_COLOR_TYPE_RGB;
	else
	if ( img.mDepth == 16 )
	{
		if ( img.mChans != 1 )
			DEX_RUNTIME_ERROR( "Unsupported color type" );

		colorType = PNG_COLOR_TYPE_GRAY;
		depth = 16;
	}
	else
	if ( img.mDepth == 8 )
		colorType = PNG_COLOR_TYPE_GRAY;
	else
	{
		DEX_RUNTIME_ERROR( "Unsupported color type" );
		return;
	}

    /* Set image attributes. */
    png_set_IHDR (png_ptr,
                  info_ptr,
                  img.mW,
                  img.mH,
                  depth,
                  colorType,
                  PNG_INTERLACE_NONE,
                  PNG_COMPRESSION_TYPE_DEFAULT,
                  PNG_FILTER_TYPE_DEFAULT);

    /* Initialize rows of PNG. */

	DVec<png_byte *>	rowPtrs;
	rowPtrs.resize( img.mH );

	const image	*pUseImg;
	image		xformImg;

	if ( img.mDepth == 16 && img.mChans == 1 )
	{
        image::Params par;
        par.pSrcImg = &img;
        par.doCopySrcImgPixels = true;

		xformImg.Setup( par );

		convert_16_to_BigEndian( xformImg );

		pUseImg = &xformImg;
	}
	else
		pUseImg = &img;

    for (u_int y = 0; y < pUseImg->mH; ++y)
	{
		u_int yy = flipY ? pUseImg->mH-1 - y : y;
		rowPtrs[y] = (png_byte *)pUseImg->GetPixelPtr( 0, yy );
	}

    /* Write the image data to "fp". */
    png_init_io (png_ptr, fp);
    png_set_rows (png_ptr, info_ptr, &rowPtrs[0] );
    png_write_png (png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);

    png_destroy_write_struct (&png_ptr, &info_ptr);
    fclose (fp);
}

#endif

