//==================================================================
/// Image_RGBE.cpp
///
/// Created by Davide Pasca - 2015/1/16
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#include "stdafx.h"
#include "DHalf.h"
#include "Image_RGBE.h"

//=============================================================================
/*
posted to http://www.graphics.cornell.edu/~bjw/
written by Bruce Walter  (bjw@graphics.cornell.edu)  5/26/95
based on code written by Greg Ward
*/

typedef struct {
	int valid;            /* indicate which fields are valid */
	char programtype[16]; /* listed at beginning of file to identify it
						  * after "#?".  defaults to "RGBE" */
	float gamma;          /* image has already been gamma corrected with
						  * given gamma.  defaults to 1.0 (no correction) */
	float exposure;       /* a value of 1.0 in an image corresponds to
						  * <exposure> watts/steradian/m^2.
						  * defaults to 1.0 */
} rgbe_header_info;

/* flags indicating which fields in an rgbe_header_info are valid */
#define RGBE_VALID_PROGRAMTYPE 0x01
#define RGBE_VALID_GAMMA       0x02
#define RGBE_VALID_EXPOSURE    0x04

/* return codes for rgbe routines */
#define RGBE_RETURN_SUCCESS 0
#define RGBE_RETURN_FAILURE -1

/* read or write headers */
/* you may set rgbe_header_info to null if you want to */
int RGBE_WriteHeader(DUT::MemWriterDynamic &mw, int width, int height, rgbe_header_info *info);
void RGBE_ReadHeader(DUT::MemReader &mr, int *width, int *height, rgbe_header_info *info);

/* read or write pixels */
/* can read or write pixels in chunks of any size including single pixels*/
//void RGBE_WritePixels(DUT::MemWriterDynamic &mw, float *data, int numpixels);

/* read or write run length encoded files */
/* must be called to read or write whole scanlines */
//int RGBE_WritePixels_RLE(MemWriterDynamic &mw, float *data, int scanline_width, int num_scanlines);
//int RGBE_ReadPixels_RLE(DUT::MemReader &mr, float *data, int scanline_width, int num_scanlines);

//===============================================================================
void	RGBE_ReadPixels_RLE(
            DUT::MemReader &mr,
            U8 *rgbe_datap,
            bool destIsFloat,
            bool useFloat16,
            int scanline_width,
            int num_scanlines);

float	*RGBE_GetExpTable(void);
void	RGBE_GetFloatRGB( u_int rgbecol, float *out_floatrgb );

//=============================================================================
/// RGBE
//=============================================================================
enum {
    RGBE_DATA_RED,
    RGBE_DATA_GREEN,
    RGBE_DATA_BLUE,
    RGBE_DATA_SIZE
};

#if 0
//=============================================================================
/* standard conversion from float pixels to rgbe pixels */
/* note: you can remove the "inline"s if your compiler complains about it */
static inline void float2rgbe(U8 rgbe[4], float red, float green, float blue)
{
	float v;
	int e;

	v = red;
	if (green > v) v = green;
	if (blue > v) v = blue;
	if (v < 1e-32) {
		rgbe[0] = rgbe[1] = rgbe[2] = rgbe[3] = 0;
	}
	else {
		v = frexp(v,&e) * 256.0f/v;
		rgbe[0] = (U8) (red * v);
		rgbe[1] = (U8) (green * v);
		rgbe[2] = (U8) (blue * v);
		rgbe[3] = (U8) (e + 128);
	}
}
#endif

//=============================================================================
/* standard conversion from rgbe to float pixels */
/* note: Ward uses ldexp(col+0.5,exp-(128+8)).  However we wanted pixels */
/*       in the range [0,1] to map back into the range [0,1].            */
static inline Float3 rgbe2float(const U8 rgbe[4])
{
	if NOT( rgbe[3] )
        return Float3(0,0,0);

    float f = ldexp(1.0f,rgbe[3]-(int)(128+8));

    return { rgbe[0] * f,
             rgbe[1] * f,
             rgbe[2] * f };
}

#if 0
//=============================================================================
/* default minimal header. modify if you want more information in header */
int RGBE_WriteHeader(MemWriterDynamic &mw, int width, int height, rgbe_header_info *info)
{
	char *programtype = "RGBE";

	if (info && (info->valid & RGBE_VALID_PROGRAMTYPE))
		programtype = info->programtype;
	if (fprintf( mw, "#?%s\n",programtype) < 0)
		return rgbe_error(rgbe_write_error,NULL);
	/* The #? is to identify file type, the programtype is optional. */
	if (info && (info->valid & RGBE_VALID_GAMMA)) {
		if (fprintf( mw, "GAMMA=%g\n",info->gamma) < 0)
			return rgbe_error(rgbe_write_error,NULL);
	}
	if (info && (info->valid & RGBE_VALID_EXPOSURE)) {
		if (fprintf( mw, "EXPOSURE=%g\n",info->exposure) < 0)
			return rgbe_error(rgbe_write_error,NULL);
	}
	if (fprintf( mw, "FORMAT=32-bit_rle_rgbe\n\n" < 0)
		return rgbe_error(rgbe_write_error,NULL);
	if (fprintf( mw,  "-Y %d +X %d\n", height, width) < 0)
		return rgbe_error(rgbe_write_error,NULL);
	return RGBE_RETURN_SUCCESS;
}
#endif

//==================================================================
static DStr readCStringSub( DUT::MemReader &mr )
{
    DStr outStr;

    while (1)
    {
        if ( mr.IsEOF() )
            DEX_RUNTIME_ERROR( "No string ending" );

        auto ch = mr.ReadValue<u_char>();

        if ( ch == 0 || ch == '\r' || ch == '\n' )
            return outStr;

        outStr += (char)ch;
    }
}

//=============================================================================
/* minimal header reading.  modify if you want to parse more information */
void RGBE_ReadHeader(DUT::MemReader &mr, int *width, int *height, rgbe_header_info *info)
{
	int found_format = 0;
	if (info)
	{
		info->valid = 0;
		info->programtype[0] = 0;
		info->gamma = info->exposure = 1.0;
	}

    DStr buf;

    //
    buf = readCStringSub( mr );
    if ( buf.size() == 0 )
        DEX_RUNTIME_ERROR( "Error reading" );

	if ((buf[0] != '#')||(buf[1] != '?')) {
		/* if you want to require the magic token then uncomment the next line */
		/*return rgbe_error(rgbe_format_error,"bad initial token"; */
	}
	else
	if (info)
	{
		info->valid |= RGBE_VALID_PROGRAMTYPE;
        size_t i = 0;
		for (;i<sizeof(info->programtype)-1;i++)
		{
			if ((buf[i+2] == 0) || isspace(buf[i+2]))
				break;
			info->programtype[i] = buf[i+2];
		}
		info->programtype[i] = 0;

        buf = readCStringSub( mr );
        if ( buf.size() == 0 )
            DEX_RUNTIME_ERROR( "Error reading" );
	}

	for(;;)
	{
        float tempf;

		//if (strcmp(buf.c_str(),"FORMAT=32-bit_rle_rgbe\n" == 0)
		if (strcmp(buf.c_str(),"FORMAT=32-bit_rle_rgbe") == 0)
			found_format = 1;       /* format found so break out of loop */
		else
		if (info && (sscanf_s(buf.c_str(),"GAMMA=%g",&tempf) == 1))
		{
			info->gamma = tempf;
			info->valid |= RGBE_VALID_GAMMA;
		}
		else
		if (info && (sscanf_s(buf.c_str(),"EXPOSURE=%g",&tempf) == 1))
		{
			info->exposure = tempf;
			info->valid |= RGBE_VALID_EXPOSURE;
		}

        buf = readCStringSub( mr );
        if ( buf.size() == 0 )
        {
			if ( found_format )
				break;
			else
				DEX_RUNTIME_ERROR( "no FORMAT specifier found" );
        }
	}
	/*
	if (strcmp(buf,"\n" != 0)
	return rgbe_error(rgbe_format_error,
	"missing blank line after FORMAT specifier";
	*/
    buf = readCStringSub( mr );

	if (sscanf_s(buf.c_str(),"-Y %d +X %d",height,width) < 2)
        DEX_RUNTIME_ERROR("missing image size specifier");
}

#if 0
//=============================================================================
/* simple write routine that does not use run length encoding */
/* These routines can be made faster by allocating a larger buffer and
fread-ing and fwrite-ing the data in larger chunks */
int RGBE_WritePixels(MemWriterDynamic &mw, float *data, int numpixels)
{
	U8 rgbe[4];

	while (numpixels-- > 0) {
		float2rgbe(rgbe,data[RGBE_DATA_RED],
			data[RGBE_DATA_GREEN],data[RGBE_DATA_BLUE]);
		data += RGBE_DATA_SIZE;
		if (fwrite(rgbe, sizeof(rgbe), 1, fp) < 1)
			return rgbe_error(rgbe_write_error,NULL);
	}
	return RGBE_RETURN_SUCCESS;
}
#endif

//===============================================================================
/* simple read routine.  will not correctly handle run length encoding */
static void RGBE_ReadPixels(DUT::MemReader &mr, U8 *pDes, bool isFloat16, int numpixels)
{
	while (numpixels-- > 0)
	{
        U8 rgbe[4];
		mr.ReadArray( rgbe, sizeof(rgbe) );

        Float3 rgbf32 = rgbe2float( rgbe );

        if ( isFloat16 )
        {
            ((uint16_t *)pDes)[0] = HALF::FloatToHalf( rgbf32[0] );
            ((uint16_t *)pDes)[1] = HALF::FloatToHalf( rgbf32[1] );
            ((uint16_t *)pDes)[2] = HALF::FloatToHalf( rgbf32[2] );
            pDes += 3 * sizeof(uint16_t);
        }
        else
        {
            ((float *)pDes)[0] = rgbf32[0];
            ((float *)pDes)[1] = rgbf32[1];
            ((float *)pDes)[2] = rgbf32[2];
            pDes += 3 * sizeof(float);
        }
	}
}

//===============================================================================
// DAVIDE - special version to return simply the RGBE pixels as they are stored
static void RGBE_ReadPixelsRGBE( DUT::MemReader &mr, U8 *rgbe_datap, int numpixels )
{
	mr.ReadArray( rgbe_datap, 4*sizeof(*rgbe_datap)*numpixels );
}

#if 0
//=============================================================================
/* The code below is only needed for the run-length encoded files. */
/* Run length encoding adds considerable complexity but does */
/* save some space.  For each scanline, each channel (r,g,b,e) is */
/* encoded separately for better compression. */
static int RGBE_WriteBytes_RLE( DUT::MemWriterDynamic &mw, U8 *data, int numbytes)
{
#define MINRUNLENGTH 4
	int cur, beg_run, run_count, old_run_count, nonrun_count;
	U8 buf[2];

	cur = 0;
	while(cur < numbytes) {
		beg_run = cur;
		/* find next run of length at least 4 if one exists */
		run_count = old_run_count = 0;
		while((run_count < MINRUNLENGTH) && (beg_run < numbytes)) {
			beg_run += run_count;
			old_run_count = run_count;
			run_count = 1;
			while( (beg_run + run_count < numbytes) && (run_count < 127)
				&& (data[beg_run] == data[beg_run + run_count]))
				run_count++;
		}
		/* if data before next big run is a short run then write it as such */
		if ((old_run_count > 1)&&(old_run_count == beg_run - cur)) {
			buf[0] = 128 + old_run_count;   /*write short run*/
			buf[1] = data[cur];
			if (fwrite(buf,sizeof(buf[0])*2,1,fp) < 1)
				return rgbe_error(rgbe_write_error,NULL);
			cur = beg_run;
		}
		/* write out bytes until we reach the start of the next run */
		while(cur < beg_run) {
			nonrun_count = beg_run - cur;
			if (nonrun_count > 128)
				nonrun_count = 128;
			buf[0] = nonrun_count;
			if (fwrite(buf,sizeof(buf[0]),1,fp) < 1)
				return rgbe_error(rgbe_write_error,NULL);
			if (fwrite(&data[cur],sizeof(data[0])*nonrun_count,1,fp) < 1)
				return rgbe_error(rgbe_write_error,NULL);
			cur += nonrun_count;
		}
		/* write out next run if one was found */
		if (run_count >= MINRUNLENGTH) {
			buf[0] = 128 + run_count;
			buf[1] = data[beg_run];
			if (fwrite(buf,sizeof(buf[0])*2,1,fp) < 1)
				return rgbe_error(rgbe_write_error,NULL);
			cur += run_count;
		}
	}
	return RGBE_RETURN_SUCCESS;
#undef MINRUNLENGTH
}

//=============================================================================
int RGBE_WritePixels_RLE(DUT::MemWriterDynamic &mw, float *data, int scanline_width,
						 int num_scanlines)
{
	U8 rgbe[4];
	U8 *buffer;
	int i, err;

	if ((scanline_width < 8)||(scanline_width > 0x7fff))
		/* run length encoding is not allowed so write flat*/
		return RGBE_WritePixels( mw, data,scanline_width*num_scanlines);
	buffer = (U8 *)malloc(sizeof(U8)*4*scanline_width);
	if (buffer == NULL)
		/* no buffer space so write flat */
		return RGBE_WritePixels( mw, data,scanline_width*num_scanlines);
	while(num_scanlines-- > 0) {
		rgbe[0] = 2;
		rgbe[1] = 2;
		rgbe[2] = scanline_width >> 8;
		rgbe[3] = scanline_width & 0xFF;
		if (fwrite(rgbe, sizeof(rgbe), 1, fp) < 1) {
			free(buffer);
			return rgbe_error(rgbe_write_error,NULL);
		}
		for(i=0;i<scanline_width;i++) {
			float2rgbe(rgbe,data[RGBE_DATA_RED],
				data[RGBE_DATA_GREEN],data[RGBE_DATA_BLUE]);
			buffer[i] = rgbe[0];
			buffer[i+scanline_width] = rgbe[1];
			buffer[i+2*scanline_width] = rgbe[2];
			buffer[i+3*scanline_width] = rgbe[3];
			data += RGBE_DATA_SIZE;
		}
		/* write out each of the four channels separately run length encoded */
		/* first red, then green, then blue, then exponent */
		for(i=0;i<4;i++) {
			if ((err = RGBE_WriteBytes_RLE( mw, &buffer[i*scanline_width],
				scanline_width)) != RGBE_RETURN_SUCCESS) {
					free(buffer);
					return err;
			}
		}
	}
	free(buffer);
	return RGBE_RETURN_SUCCESS;
}
#endif

//===============================================================================
// DAVIDE - special version to return simply the RGBE pixels as they are stored
void RGBE_ReadPixels_RLE(
            DUT::MemReader &mr,
            U8 *rgbe_datap,
            bool destIsFloat,
            bool useFloat16,
            int scanline_width,
            int num_scanlines)
{
	std::vector<u_char>	scanline_buffer;

	if ((scanline_width < 8)||(scanline_width > 0x7fff))
	{
		// run length encoding is not allowed so read flat
        int numPixels = scanline_width * num_scanlines;

		if ( destIsFloat )
			RGBE_ReadPixels( mr, rgbe_datap, useFloat16, numPixels );
		else
			RGBE_ReadPixelsRGBE( mr, rgbe_datap, numPixels );

		return;
	}

	scanline_buffer.resize( scanline_width * (4+1) );	// add 1 to avoid bounds exception with calculating ptr_end

	/* read in each successive scanline */
	while(num_scanlines > 0)
	{
        U8 rgbe[4];
		mr.ReadArray( rgbe, sizeof(rgbe) );
		
		if ((rgbe[0] != 2)||(rgbe[1] != 2)||(rgbe[2] & 0x80))
		{
            int numPixels = scanline_width * num_scanlines;

			// this file is not run length encoded
			if ( destIsFloat )
			{
				DASSERT( 0 );

				RGBE_ReadPixels( mr, rgbe_datap, useFloat16, numPixels - 1 );
			}
			else
			{
				rgbe_datap[0] = rgbe[0];
				rgbe_datap[1] = rgbe[1];
				rgbe_datap[2] = rgbe[2];
				rgbe_datap[3] = rgbe[3];
				rgbe_datap += 4 * sizeof(*rgbe_datap);

				RGBE_ReadPixelsRGBE( mr, rgbe_datap, numPixels - 1 );
			}

			return;
		}

		if ((((int)rgbe[2])<<8 | rgbe[3]) != scanline_width)
		{
			DEX_RUNTIME_ERROR( "wrong scanline width" );
		}

		U8 *ptr = &scanline_buffer[0];
		/* read each of the four channels for the scanline into the buffer */
		for(int i=0; i < 4; ++i)
		{
			U8 *ptr_end = &scanline_buffer[(i+1)*scanline_width];
			while(ptr < ptr_end)
			{
                U8 buf[2];

				mr.ReadArray( buf, sizeof(buf[0])*2 );

				if (buf[0] > 128)
				{
					/* a run of the same value */
					int count = buf[0]-128;
					if ((count == 0)||(count > ptr_end - ptr))
					{
						DEX_RUNTIME_ERROR( "bad scanline data" );
					}
					while(count-- > 0)
						*ptr++ = buf[1];
				}
				else
				{
					/* a non-run */
					int count = buf[0];
					if ((count == 0)||(count > ptr_end - ptr))
					{
						DEX_RUNTIME_ERROR( "bad scanline data" );
					}

					*ptr++ = buf[1];

					if (--count > 0)
					{
						mr.ReadArray( ptr, sizeof(*ptr)*count );

						ptr += count;
					}
				}
			}
		}

		if ( destIsFloat )
		{
			for(int i=0; i < scanline_width; ++i)
			{
                U8 rgbe[4];
				rgbe[0] = scanline_buffer[i];
				rgbe[1] = scanline_buffer[i+scanline_width];
				rgbe[2] = scanline_buffer[i+2*scanline_width];
				rgbe[3] = scanline_buffer[i+3*scanline_width];

                Float3 rgbf32 = rgbe2float( rgbe );

                if ( useFloat16 )
                {
                    ((uint16_t *)rgbe_datap)[0] = HALF::FloatToHalf( rgbf32[0] );
                    ((uint16_t *)rgbe_datap)[1] = HALF::FloatToHalf( rgbf32[1] );
                    ((uint16_t *)rgbe_datap)[2] = HALF::FloatToHalf( rgbf32[2] );
                    rgbe_datap += 3 * sizeof(uint16_t);
                }
                else
                {
                    ((float *)rgbe_datap)[0] = rgbf32[0];
                    ((float *)rgbe_datap)[1] = rgbf32[1];
                    ((float *)rgbe_datap)[2] = rgbf32[2];
                    rgbe_datap += 3 * sizeof(float);
                }
			}
		}
		else
		{
			for(int i=0; i < scanline_width; ++i)
			{
				rgbe_datap[0] = scanline_buffer[i];
				rgbe_datap[1] = scanline_buffer[i+scanline_width];
				rgbe_datap[2] = scanline_buffer[i+2*scanline_width];
				rgbe_datap[3] = scanline_buffer[i+3*scanline_width];
				rgbe_datap += 4;
			}
		}

		num_scanlines--;
	}
}

//===============================================================================
// DAVIDE -
float *RGBE_GetExpTable(void)
{
	static float	exptable[256];
	int				i;

	if ( ((u_int *)exptable)[0] == 0 )
		for (i=0; i < 256; ++i)
			exptable[i] = ldexp( 1.0f, i - (int)(128+8) );

	return exptable;
}

//===============================================================================
// DAVIDE -
void RGBE_GetFloatRGB( u_int rgbecol, float *out_floatrgb )
{
	float	*tabp = RGBE_GetExpTable();

	float t = tabp[ (rgbecol >> 24) & 0xff ];
	out_floatrgb[0] = ((rgbecol >>  0) & 0xff) * t;
	out_floatrgb[1] = ((rgbecol >>  8) & 0xff) * t;
	out_floatrgb[2] = ((rgbecol >> 16) & 0xff) * t;
}

//==================================================================
void Image_RGBELoad( image &img, u_int imgBaseFlags, const U8 *pData, size_t dataSize )
{
    rgbe_header_info    info;
    int                 wd = 0;
    int                 he = 0;

    DUT::MemReader mr( pData, dataSize );

    try {
        RGBE_ReadHeader( mr, &wd, &he, &info );
    }
    catch ( ... )
    {
        DEX_RUNTIME_ERROR( "Failed reading the RGBE header" );
    }

    image::Params par;
    par.width   = wd;
    par.height  = he;
    par.depth   = 8*sizeof(U16) * 3;
    par.chans   = 3;
    par.flags   = imgBaseFlags | image::FLG_IS_FLOAT16; // keep the flags

    img.Setup( par );

    RGBE_ReadPixels_RLE( mr, img.GetPixelPtr(0,0), true, true, wd, he );
}

