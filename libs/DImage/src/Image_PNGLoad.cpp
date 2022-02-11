//==================================================================
/// Image_PNGLoad.cpp
///
/// Created by Davide Pasca - 2010/4/17
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#if !defined(D_NOPNG)

#include "stdafx.h"
#include "png.h"
#include "pngstruct.h"
#include "DUT_Files.h"
#include "ImageConv.h"
#include "Image.h"

//==================================================================
bool Image_PNG_DetectSRGBFromFileName( const DStr &fname )
{
    // if it's not a normal, then assume it's srgb !
    return !ImageConv::IsNormalMapFName( fname );
}

//==================================================================
class FileWrapper
{
public:
	FILE	*mpFile;

	FileWrapper( const char *pFileName, const char *pMode ) :
		mpFile(NULL)
	{
		if ( fopen_s( &mpFile, pFileName, pMode ) )
			DEX_RUNTIME_ERROR( "Failed to open %s", pFileName);
	}

	~FileWrapper()
	{
		if ( mpFile )
			fclose( mpFile );
	}

	operator FILE*()
	{
		return mpFile;
	}
};

//==================================================================
static void png_cexcept_error(png_structp pPNG, png_const_charp msg)
{
	DEX_RUNTIME_ERROR( "PNG Error. %s", msg) ;
}

//==================================================================
struct MyPNGData
{
    const U8 *pCurDataPtr = nullptr;
    int      sizeLeft = 0;
};

//==================================================================
static void _png_read_fn(png_structp pPNG, png_bytep pChunkData, png_size_t chunkSize)
{
    MyPNGData &pngData = *(MyPNGData *)pPNG->io_ptr;

    DASSERT(chunkSize <= (size_t )(pngData.sizeLeft));

    memcpy( pChunkData, pngData.pCurDataPtr, chunkSize );

    pngData.pCurDataPtr = pngData.pCurDataPtr + chunkSize;
    pngData.sizeLeft -= (int)chunkSize;
}

//==================================================================
static void convert_RGB24_to_RGBA( image &img )
{
	u_int wd = img.mW;
	u_int he = img.mH;

	for (u_int y=0; y < he; ++y)
	{
		U8	*pRow = img.GetPixelPtr( 0, y );

		for (u_int i=wd; i > 0; --i)
		{
			u_int x = i - 1;

			U8	r = pRow[ x * 3 + 0 ];
			U8	g = pRow[ x * 3 + 1 ];
			U8	b = pRow[ x * 3 + 2 ];

			pRow[ x * 4 + 0 ] = r;
			pRow[ x * 4 + 1 ] = g;
			pRow[ x * 4 + 2 ] = b;
			pRow[ x * 4 + 3 ] = 0;
		}
	}
}

//==================================================================
static void swap16BitEndianess( image &img )
{
    const int chans = (int)img.mChans;
    ImageConv::RectProcess<U16>(
        img,
        [chans]( U16 *pPix )
        {
            for (int i=0; i != chans; ++i)
            {
                U8 *pPix8 = (U8 *)pPix;
                std::swap( pPix8[i*2+0], pPix8[i*2+1] );
            }
        });
}

//==================================================================
void Image_PNGLoad(
        image &img,
        u_int imgBaseFlags,
        const U8 *pData,
        size_t dataSize,
        bool forceAsSRGB )
{
	int						iBitDepth;
	int						iColorType;
    png_size_t              ulChannels;
    png_size_t              ulRowBytes;

	DVec<U8*>				pRowPointers;

    const int SIG_SIZE = 8;

	if NOT( png_check_sig(pData, SIG_SIZE) )
	{
		DEX_RUNTIME_ERROR( "Bad PNG" );
	}

    MyPNGData pngData;
    pngData.pCurDataPtr = pData + SIG_SIZE;
	pngData.sizeLeft = (int)dataSize - SIG_SIZE;

	// create the two png(-info) structures

	png_structp pPNG =
		png_create_read_struct(
					PNG_LIBPNG_VER_STRING,
					NULL,
					(png_error_ptr)png_cexcept_error,
					(png_error_ptr)NULL );

	if NOT( pPNG )
	{
		DEX_RUNTIME_ERROR( "Failed to create the structure") ;
	}

	png_infop pPNGInfo = png_create_info_struct(pPNG);

	if NOT( pPNGInfo )
	{
		png_destroy_read_struct(&pPNG, NULL, NULL);
		DEX_RUNTIME_ERROR( "Failed to the info structure") ;
	}

    bool hasGamma22 = false;

	try
    {
        // initialize the png structure
        png_set_read_fn(pPNG, &pngData, _png_read_fn);

        png_set_sig_bytes(pPNG, 8);

        // read all PNG info up to image data

        png_read_info(pPNG, pPNGInfo);

        // get width, height, bit-depth and color-type

		png_uint_32	wd = 0;
		png_uint_32	he = 0;

        png_get_IHDR(
				pPNG,
				pPNGInfo,
				&wd,
				&he,
				&iBitDepth,
				&iColorType,
				NULL,
				NULL,
				NULL );

        // expand images of all color-type and bit-depth to 3x8 bit RGB images
        // let the library process things like alpha, transparency, background

        //if (iBitDepth == 16)
        //    png_set_strip_16(pPNG);

        if (iColorType == PNG_COLOR_TYPE_PALETTE)
            png_set_expand(pPNG);

		if (iBitDepth < 8)
            png_set_expand(pPNG);

		if (png_get_valid(pPNG, pPNGInfo, PNG_INFO_tRNS))
            png_set_expand(pPNG);

		//if (iColorType == PNG_COLOR_TYPE_GRAY ||
        //    iColorType == PNG_COLOR_TYPE_GRAY_ALPHA)
        //    png_set_gray_to_rgb(pPNG);

        // set the background color to draw transparent and alpha images over.
/*
		png_color_16			*pBackground;

		if (png_get_bKGD(pPNG, info_ptr, &pBackground))
        {
            png_set_background(pPNG, pBackground, PNG_BACKGROUND_GAMMA_FILE, 1, 1.0);
			if ( pBkgColor )
			{
				pBkgColor->red   = (U8) pBackground->red;
				pBkgColor->green = (U8) pBackground->green;
				pBkgColor->blue  = (U8) pBackground->blue;
			}
        }
        else
        {
            pBkgColor = NULL;
        }
*/

        // gamma: http://onemanmmo.com/index.php?cmd=newsitem&comment=news.1.109.0

        // if required set gamma conversion
        int sRGBintent;
        if ( png_get_sRGB( pPNG, pPNGInfo, &sRGBintent ) != 0 )
        {
            hasGamma22 = true;
        }
        else
        {
            double gamma;
            if (png_get_gAMA(pPNG, pPNGInfo, &gamma))
            {
                png_set_gamma(pPNG, 2.2, gamma);
                hasGamma22 = true;
            }
        }

        // after the transformations have been registered update info_ptr data

        png_read_update_info(pPNG, pPNGInfo);

        // get again width, height and the new bit-depth and color-type

        png_get_IHDR(
				pPNG,
				pPNGInfo,
				&wd,
				&he,
				&iBitDepth,
				&iColorType,
				NULL,
				NULL,
				NULL);

        // row_bytes is the width x number of channels

        ulRowBytes = png_get_rowbytes(pPNG, pPNGInfo);
        ulChannels = png_get_channels(pPNG, pPNGInfo);

        // now we can allocate memory to store the image

		image::Params par;
		par.width	= wd;
		par.height	= he;
		par.depth	= (u_int)(iBitDepth * ulChannels);
		par.chans	= (u_int)ulChannels;
		par.rowPitch= (u_int)ulRowBytes;
        par.flags   = imgBaseFlags; // copy the flags

        //
        if ( forceAsSRGB && (par.chans == 3 || par.chans == 4) )
            hasGamma22 = true;

        // see if it has gamma
        if ( hasGamma22 )
            par.flags |= image::FLG_HAS_GAMMA22;
        else
            par.flags &= ~image::FLG_HAS_GAMMA22;

		img.Setup( par );

        // and allocate memory for an array of row-pointers

		pRowPointers.resize( he );

        // set the individual row-pointers to point at the correct offsets
        for (U32 i = 0; i < he; i++)
            pRowPointers[i] = img.GetPixelPtr( 0, i );

        // now we can go ahead and just read the whole image

        png_read_image( pPNG, &pRowPointers[0] );

        // read the additional chunks in the PNG file (not really needed)

        png_read_end( pPNG, NULL );

		// extend to 4 bytes per pixel if necessary
		if ( ulChannels == 3 && iBitDepth == 8 && img.mBytesPerPixel == 4 )
		{
			convert_RGB24_to_RGBA( img );
		}

		if ( iBitDepth == 16 )
		{
            swap16BitEndianess( img );

            // WARNING: if it's more than one channel, then convert to 8 bits !
			if ( ulChannels != 1 )
            {
                image::Params img8Par;
                img8Par.width  = img.mW;
                img8Par.height = img.mH;
                img8Par.chans  = img.mChans;
                img8Par.depth  = img8Par.chans * 8; // only change "depth"
                img8Par.flags  = img.mFlags;

                const int chans = (int)img.mChans;
                image img8( img8Par );
                ImageConv::BlitProcess<U16,U8>(
                        img, img8,
                        [chans](const U16 *pSrc, U8 *pDes)
                        {
                            for (int i=0; i != chans; ++i)
                            {
                                pDes[i] = (U8)(pSrc[i] >> 8);
                            }
                        });

                // copy the 8 bit image into the 16 one
                img = std::move( img8 ); // replace the image
            }
		}
    }
	catch ( ... )
    {
        png_destroy_read_struct(&pPNG, &pPNGInfo, NULL);
        throw;
    }

    png_destroy_read_struct(&pPNG, &pPNGInfo, NULL);
}

//==================================================================
void Image_PNGLoad( image &img, u_int imgBaseFlags, const char *pFName )
{
	DFileData fileData;
	DUT::GrabFile( pFName, fileData );

	bool isBad = false;

    // if it's not a normal, then assume it's srgb !
    bool forceAsSRGB = Image_PNG_DetectSRGBFromFileName( pFName );

	try {
		Image_PNGLoad( img, imgBaseFlags, &fileData[0], fileData.size(), forceAsSRGB );
	}
    catch ( ... ) {
		isBad = true;
	}

    if ( isBad )
        DEX_RUNTIME_ERROR( "File %s is not a valid PNG file", pFName) ;
}

#endif

