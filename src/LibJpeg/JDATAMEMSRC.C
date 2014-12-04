////////////////////////////////////////////////////////////////////////
//
//	Note : this file is included as part of the Smaller Animals Software
//	JpegFile package. Though this file has not been modified from it's 
//	original IJG 6a form, it is not the responsibility on the Independent
//	JPEG Group to answer questions regarding this code.
//	
//	Any questions you have about this code should be addressed to :
//
//	CHRISDL@PAGESZ.NET	- the distributor of this package.
//
//	Remember, by including this code in the JpegFile package, Smaller 
//	Animals Software assumes all responsibilities for answering questions
//	about it. If we (SA Software) can't answer your questions ourselves, we 
//	will direct you to people who can.
//
//	Thanks, CDL.
//
////////////////////////////////////////////////////////////////////////

/*
 * jdatasrc.c
 *
 * Copyright (C) 1994-1996, Thomas G. Lane.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file contains decompression data source routines for the case of
 * reading JPEG data from a file (or any stdio stream).  While these routines
 * are sufficient for most applications, some will want to use a different
 * source manager.
 * IMPORTANT: we assume that fread() will correctly transcribe an array of
 * JOCTETs from 8-bit-wide elements on external storage.  If char is wider
 * than 8 bits on your machine, you may need to do some tweaking.
 */

/* this is not a core library module, so it doesn't define JPEG_INTERNALS */
#include "jinclude.h"
#include "jpeglib.h"
#include "jerror.h"

/* Init source method for JPEG data source manager.  Called by 
   jpeg_read_header() before any data is actually read.  See 
   libjpeg.doc from the JPEG lib distribution.  */ 

METHODDEF(void)
our_common_init_source (j_decompress_ptr cinfo) 
{ 
} 
 
 
/* Method to terminate data source.  Called by 
   jpeg_finish_decompress() after all data has been processed.  */ 
 
METHODDEF(void )
our_common_term_source (j_decompress_ptr cinfo) 
{ 
} 
 
 
/* Fill input buffer method for JPEG data source manager.  Called 
   whenever more data is needed.  We read the whole image in one step, 
   so this only adds a fake end of input marker at the end.  */ 
 
METHODDEF(boolean)
our_memory_fill_input_buffer (j_decompress_ptr cinfo) 
{ 
	/* Insert a fake EOI marker.  */ 
	struct jpeg_source_mgr *src = cinfo->src; 
	static JOCTET buffer[2]; 
	
	buffer[0] = (JOCTET) 0xFF; 
	buffer[1] = (JOCTET) JPEG_EOI; 
	
	src->next_input_byte = buffer; 
	src->bytes_in_buffer = 2; 
	return 1; 
} 
 
 
/* Method to skip over NUM_BYTES bytes in the image data.  CINFO->src 
   is the JPEG data source manager.  */ 
 
METHODDEF(void)
our_memory_skip_input_data (j_decompress_ptr cinfo, long num_bytes) 
{ 
	struct jpeg_source_mgr *src = (struct jpeg_source_mgr *) cinfo->src; 
	
	if (src) 
    { 
		if (num_bytes > (long) src->bytes_in_buffer) 
			ERREXIT (cinfo, JERR_INPUT_EOF); 
		
		src->bytes_in_buffer -= num_bytes; 
		src->next_input_byte += num_bytes; 
    } 
} 
 
 
/* Set up the JPEG lib for reading an image from DATA which contains 
   LEN bytes.  CINFO is the decompression info structure created for 
   reading the image.  */ 
 
GLOBAL(void)
jpeg_memory_src (j_decompress_ptr cinfo, JOCTET *data, unsigned int len) 
{ 
	struct jpeg_source_mgr *src; 
	
	if (cinfo->src == NULL) 
    { 
		/* First time for this JPEG object?  */ 
		cinfo->src = (struct jpeg_source_mgr *) 
			(*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT, 
			sizeof (struct jpeg_source_mgr)); 
		src = (struct jpeg_source_mgr *) cinfo->src; 
		src->next_input_byte = data; 
    } 
	
	src = (struct jpeg_source_mgr *) cinfo->src; 
	src->init_source = our_common_init_source; 
	src->fill_input_buffer = our_memory_fill_input_buffer; 
	src->skip_input_data = our_memory_skip_input_data; 
	src->resync_to_restart = jpeg_resync_to_restart; /* Use default method.  */ 
	src->term_source = our_common_term_source; 
	src->bytes_in_buffer = len; 
	src->next_input_byte = data; 
} 
 
 
