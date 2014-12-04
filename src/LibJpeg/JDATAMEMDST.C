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
 * jdatadst.c
 *
 * Copyright (C) 1994-1996, Thomas G. Lane.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file contains compression data destination routines for the case of
 * emitting JPEG data to a file (or any stdio stream).  While these routines
 * are sufficient for most applications, some will want to use a different
 * destination manager.
 * IMPORTANT: we assume that fwrite() will correctly transcribe an array of
 * JOCTETs into 8-bit-wide elements on external storage.  If char is wider
 * than 8 bits on your machine, you may need to do some tweaking.
 */

/* this is not a core library module, so it doesn't define JPEG_INTERNALS */
#include "jinclude.h"
#include "jpeglib.h"
#include "jerror.h"

typedef struct
{
	struct jpeg_destination_mgr pub;      
	JOCTET *buffer;                       
	int bufsize;                          
	int datacount;                        
} memory_destination_mgr;

METHODDEF(void)
init_destination (j_compress_ptr cinfo)
{
	memory_destination_mgr* dest = (memory_destination_mgr*) cinfo->dest;
	dest->pub.next_output_byte = dest->buffer;
	dest->pub.free_in_buffer = dest->bufsize;
	dest->datacount=0;
}

METHODDEF(boolean)
empty_output_buffer (j_compress_ptr cinfo)
{
	memory_destination_mgr* dest = (memory_destination_mgr*) cinfo->dest;
	dest->pub.next_output_byte = dest->buffer;
	dest->pub.free_in_buffer = dest->bufsize;
	
	return TRUE;
}

METHODDEF(void)
term_destination (j_compress_ptr cinfo)
{
	memory_destination_mgr* dest = (memory_destination_mgr*) cinfo->dest;
	dest->datacount = dest->bufsize - dest->pub.free_in_buffer;
}

GLOBAL(void)
jpeg_memory_dest(j_compress_ptr cinfo, JOCTET *buffer,int bufsize)
{
	memory_destination_mgr* dest;
	if (cinfo->dest == NULL)
	{    /* first time for this JPEG object? */
		cinfo->dest = (struct jpeg_destination_mgr *)
			(*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT,
			sizeof(memory_destination_mgr));
	}
	
	dest = (memory_destination_mgr*) cinfo->dest;
	dest->bufsize=bufsize;
	dest->buffer=buffer;
	dest->pub.init_destination = init_destination;
	dest->pub.empty_output_buffer = empty_output_buffer;
	dest->pub.term_destination = term_destination;
}

GLOBAL(int)
jpeg_memory_get_data_count(j_compress_ptr cinfo)
{
	memory_destination_mgr* dest = (memory_destination_mgr*) cinfo->dest;
	return (dest->bufsize - dest->pub.free_in_buffer);
}
