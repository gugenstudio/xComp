//==================================================================
/// image_JPEG.cpp
///
/// Created by Davide Pasca - 2010/12/6
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#if !defined(D_NOJPEG)

#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#define XMD_H	// shit !
extern "C"
{
# ifndef MACOSX
	#include "jinclude.h"
# endif
	#include "jpeglib.h"
	#include "jerror.h"
};
#undef XMD_H	// un-shit !


#include "dlog.h"
#include "DUT_MemFile.h"
#include "Image_JPEG.h"

// for the time being, for windows only
#if defined(WIN32)

//==================================================================
static void write_JPEG_file(
			const char * filename,
			int quality,
			const JSAMPLE * image_buffer,
			int image_width,
			int image_height,
			bool flipY
			)
{
	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;
	/* More stuff */
	FILE * outfile;		/* target file */
	int row_stride;		/* physical row width in image buffer */

	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);

	if ( 0 != fopen_s( &outfile, filename, "wb" ) )
	{
		DEX_RUNTIME_ERROR( "can't open %s\n", filename );
	}
	jpeg_stdio_dest(&cinfo, outfile);

	cinfo.image_width = image_width; 	/* image width and height, in pixels */
	cinfo.image_height = image_height;
	cinfo.input_components = 3;		/* # of color components per pixel */
	cinfo.in_color_space = JCS_RGB; 	/* colorspace of input image */

	jpeg_set_defaults(&cinfo);
	jpeg_set_quality(&cinfo, quality, TRUE /* limit to baseline-JPEG values */);

	jpeg_start_compress(&cinfo, TRUE);
	row_stride = image_width * 3;	/* JSAMPLEs per row in image_buffer */

	const JSAMPLE *row_pointer[1];	/* pointer to JSAMPLE row[s] */

	while (cinfo.next_scanline < cinfo.image_height)
	{
		u_int	srcY = cinfo.next_scanline;

		if ( flipY )
			srcY = image_height - 1 - srcY;

		row_pointer[0] = &image_buffer[ srcY * row_stride ];

		jpeg_write_scanlines(&cinfo, (JSAMPARRAY)row_pointer, 1);
	}

	jpeg_finish_compress(&cinfo);
	fclose(outfile);

	jpeg_destroy_compress(&cinfo);
}

//==================================================================
void Image_JPEGSave( const image &img, const char *pFName, bool flipY )
{
	write_JPEG_file(
				pFName,
				100,
				(const JSAMPLE *)img.GetPixelPtr(0,0),
				img.mW,
				img.mH,
				flipY );
}

//==================================================================
#endif

/*
* memsrc.c
*
* Copyright (C) 1994-1996, Thomas G. Lane.
* This file is part of the Independent JPEG Group's software.
* For conditions of distribution and use, see the accompanying README file.
*
* This file contains decompression data source routines for the case of
* reading JPEG data from a memory buffer that is preloaded with the entire
* JPEG file. This would not seem especially useful at first sight, but
* a number of people have asked for it.
* This is really just a stripped-down version of jdatasrc.c. Comparison
* of this code with jdatasrc.c may be helpful in seeing how to make
* custom source managers for other purposes.
*/

/* this is not a core library module, so it doesn't define JPEG_INTERNALS */
//#include "jinclude.h"
//#include "jpeglib.h"
//#include "jerror.h"


/* Expanded data source object for memory input */

typedef struct {
	struct jpeg_source_mgr pub; /* public fields */

	JOCTET eoi_buffer[2]; /* a place to put a dummy EOI */
} my_source_mgr;

typedef my_source_mgr * my_src_ptr;


/*
* Initialize source --- called by jpeg_read_header
* before any data is actually read.
*/

METHODDEF(void)
init_source (j_decompress_ptr cinfo)
{
	/* No work, since jpeg_memory_src set up the buffer pointer and count.
	* Indeed, if we want to read multiple JPEG images from one buffer,
	* this *must* not do anything to the pointer.
	*/
}


/*
* Fill the input buffer --- called whenever buffer is emptied.
*
* In this application, this routine should never be called; if it is called,
* the decompressor has overrun the end of the input buffer, implying we
* supplied an incomplete or corrupt JPEG datastream. A simple error exit
* might be the most appropriate response.
*
* But what we choose to do in this code is to supply dummy EOI markers
* in order to force the decompressor to finish processing and supply
* some sort of output image, no matter how corrupted.
*/

METHODDEF(boolean)
fill_input_buffer (j_decompress_ptr cinfo)
{
	my_src_ptr src = (my_src_ptr) cinfo->src;

	WARNMS(cinfo, JWRN_JPEG_EOF);

	/* Create a fake EOI marker */
	src->eoi_buffer[0] = (JOCTET) 0xFF;
	src->eoi_buffer[1] = (JOCTET) JPEG_EOI;
	src->pub.next_input_byte = src->eoi_buffer;
	src->pub.bytes_in_buffer = 2;

	return TRUE;
}


/*
* Skip data --- used to skip over a potentially large amount of
* uninteresting data (such as an APPn marker).
*
* If we overrun the end of the buffer, we let fill_input_buffer deal with
* it. An extremely large skip could cause some time-wasting here, but
* it really isn't supposed to happen ... and the decompressor will never
* skip more than 64K anyway.
*/

METHODDEF(void)
skip_input_data (j_decompress_ptr cinfo, long num_bytes)
{
	my_src_ptr src = (my_src_ptr) cinfo->src;

	if (num_bytes > 0) {
		while (num_bytes > (long) src->pub.bytes_in_buffer) {
			num_bytes -= (long) src->pub.bytes_in_buffer;
			(void) fill_input_buffer(cinfo);
			/* note we assume that fill_input_buffer will never return FALSE,
			* so suspension need not be handled.
			*/
		}
		src->pub.next_input_byte += (size_t) num_bytes;
		src->pub.bytes_in_buffer -= (size_t) num_bytes;
	}
}


/*
* An additional method that can be provided by data source modules is the
* resync_to_restart method for error recovery in the presence of RST markers.
* For the moment, this source module just uses the default resync method
* provided by the JPEG library. That method assumes that no backtracking
* is possible.
*/


/*
* Terminate source --- called by jpeg_finish_decompress
* after all data has been read. Often a no-op.
*
* NB: *not* called by jpeg_abort or jpeg_destroy; surrounding
* application must deal with any cleanup that should happen even
* for error exit.
*/

METHODDEF(void)
term_source (j_decompress_ptr cinfo)
{
	/* no work necessary here */
}


/*
* Prepare for input from a memory buffer.
*/
//GLOBAL(void)
static void
jpeg_memory_src (j_decompress_ptr cinfo, const JOCTET * buffer, size_t bufsize)
{
	my_src_ptr src;

	/* The source object is made permanent so that a series of JPEG images
	* can be read from a single buffer by calling jpeg_memory_src
	* only before the first one.
	* This makes it unsafe to use this manager and a different source
	* manager serially with the same JPEG object. Caveat programmer.
	*/
	if (cinfo->src == NULL) { /* first time for this JPEG object? */
		cinfo->src = (struct jpeg_source_mgr *)
			(*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT,
                                        ((size_t )sizeof(my_source_mgr)));
	}

	src = (my_src_ptr) cinfo->src;
	src->pub.init_source = init_source;
	src->pub.fill_input_buffer = fill_input_buffer;
	src->pub.skip_input_data = skip_input_data;
	src->pub.resync_to_restart = jpeg_resync_to_restart; /* use default method */
	src->pub.term_source = term_source;

	src->pub.next_input_byte = buffer;
	src->pub.bytes_in_buffer = bufsize;
}

//=============================================================================
struct my_error_mgr {
	struct jpeg_error_mgr pub;	/* "public" fields */

	jmp_buf setjmp_buffer;	/* for return to caller */
};

typedef struct my_error_mgr * my_error_ptr;

//=============================================================================
static void my_error_exit (j_common_ptr cinfo)
{
	/* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
	my_error_ptr myerr = (my_error_ptr) cinfo->err;

	/* Always display the message. */
	/* We could postpone this until after returning, if we chose. */
	(*cinfo->err->output_message) (cinfo);

	/* Return control to the setjmp point */
	longjmp(myerr->setjmp_buffer, 1);
}

// Print error messages to the log so they appear on platforms that
// don't have proper stdout / stderr
static void my_output_message(j_common_ptr cinfo)
{
    char buffer[JMSG_LENGTH_MAX];
    (*cinfo->err->format_message)(cinfo, buffer);
    DPRINT("JPEG Lib Error: %s", buffer);
}

//=============================================================================
typedef struct
{
	struct jpeg_destination_mgr pub;	/* public fields */
	JOCTET *buffer;                     /* start of buffer */
	int bufsize;                        /* buffer size */
	int datacount;                      /* finale data size */
} memory_destination_mgr;

typedef memory_destination_mgr *mem_dest_ptr;

//=============================================================================
// Initialize destination --- called by jpeg_start_compress before any
// data is actually written.
#if 0
METHODDEF(void)
init_destination (j_compress_ptr cinfo)
{
  mem_dest_ptr dest = (mem_dest_ptr) cinfo->dest;
  dest->pub.next_output_byte = dest->buffer;
  dest->pub.free_in_buffer = dest->bufsize;
  dest->datacount=0;
}
#endif

//=============================================================================
//  Empty the output buffer --- called whenever buffer fills up.
#if 0
METHODDEF(boolean)
empty_output_buffer (j_compress_ptr cinfo)
{
  mem_dest_ptr dest = (mem_dest_ptr) cinfo->dest;
  dest->pub.next_output_byte = dest->buffer;
  dest->pub.free_in_buffer = dest->bufsize;

  return TRUE;
}
#endif

//=============================================================================
// Terminate destination --- called by jpeg_finish_compress
// after all data has been written.  Usually needs to flush buffer.
#if 0
METHODDEF(void)
term_destination (j_compress_ptr cinfo)
{
  /* expose the finale compressed image size */

  mem_dest_ptr dest = (mem_dest_ptr) cinfo->dest;
  dest->datacount = dest->bufsize - dest->pub.free_in_buffer;

}
#endif

//=============================================================================
#if 0
//GLOBAL(void)
static void
jpeg_memory_dest(j_compress_ptr cinfo, JOCTET *buffer,int bufsize)
{
  mem_dest_ptr dest;
  if (cinfo->dest == NULL) {    /* first time for this JPEG object? */
    cinfo->dest = (struct jpeg_destination_mgr *)
      (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT,
                                  sizeof(memory_destination_mgr));
  }

  dest = (mem_dest_ptr) cinfo->dest;
  dest->bufsize=bufsize;
  dest->buffer=buffer;
  dest->pub.init_destination = init_destination;
  dest->pub.empty_output_buffer = empty_output_buffer;
  dest->pub.term_destination = term_destination;
}
#endif

//==================================================================
//==================================================================
void Image_JPEGLoad( image &img, u_int imgBaseFlags, const U8 *pData, size_t dataSize )
{
    struct jpeg_decompress_struct cinfo;
    struct my_error_mgr jerr;

    // We set up the normal JPEG error routines, then override
    // error_exit and output_message
    cinfo.err = jpeg_std_error(&jerr.pub);
    jerr.pub.error_exit = my_error_exit;
    jerr.pub.output_message = my_output_message;
    /* Establish the setjmp return context for my_error_exit to use. */
    if (setjmp(jerr.setjmp_buffer))
    {
        /* If we get here, the JPEG code has signaled an error.
        * We need to clean up the JPEG object, close the input file, and return.
        */
        jpeg_destroy_decompress(&cinfo);
		DEX_RUNTIME_ERROR( "JPEG file load failed (data %p [%x, %x, %x, %x, ...], "
                  "size %d)", pData, pData[0], pData[1], pData[2], pData[3],
                  (int )dataSize );
    }
    /* Now we can initialize the JPEG decompression object. */
    jpeg_create_decompress(&cinfo);

	jpeg_memory_src( &cinfo, (const JOCTET *)pData, dataSize );

	jpeg_read_header(&cinfo, TRUE);

	jpeg_start_decompress(&cinfo);

	/* JSAMPLEs per row in output buffer */
	//int	row_stride = cinfo.output_width * cinfo.output_components;

	image::Params par;
	par.width	= cinfo.output_width;
	par.height	= cinfo.output_height;

	par.chans = cinfo.output_components;

    // copy the flags
    par.flags = imgBaseFlags;

	switch ( cinfo.output_components )
	{
	case 1:
        par.depth = 8;
        break;

	case 3:
        par.depth = 24;
        // consider RGB JPEG always in sRGB space
        par.flags |= image::FLG_HAS_GAMMA22;
        break;

	default:
		DEX_RUNTIME_ERROR( "Unsupported JPEG format" );
		break;
	}

	img.Setup( par );

	/* Make a one-row-high sample array that will go away when done with image */
	//JSAMPARRAY buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);

    int y = 0;
    while (cinfo.output_scanline < cinfo.output_height)
    {
		U8	*pRow = img.GetPixelPtr( 0, y );

		JSAMPROW  buffer[1] = { pRow };
        jpeg_read_scanlines(&cinfo, buffer, 1);

        y++;
    }

    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
}

#endif

