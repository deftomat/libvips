/* load radlab from a file
 *
 * 5/12/11
 * 	- from tiffload.c
 */

/*

    This file is part of VIPS.
    
    VIPS is free software; you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301  USA

 */

/*

    These files are distributed with VIPS - http://www.vips.ecs.soton.ac.uk

 */

/*
#define DEBUG
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /*HAVE_CONFIG_H*/
#include <vips/intl.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <vips/vips.h>
#include <vips/buf.h>
#include <vips/internal.h>

#include "pforeign.h"

#ifdef HAVE_RADIANCE

typedef struct _VipsForeignLoadRadStream {
	VipsForeignLoad parent_object;

	/* Load from a stream.
	 */
	VipsStreami *streami;

} VipsForeignLoadRadStream;

typedef VipsForeignLoadClass VipsForeignLoadRadStreamClass;

G_DEFINE_TYPE( VipsForeignLoadRadStream, vips_foreign_load_rad_stream, 
	VIPS_TYPE_FOREIGN_LOAD );

static VipsForeignFlags
vips_foreign_load_rad_stream_get_flags( VipsForeignLoad *load )
{
	/* The rad reader supports sequential read.
	 */
	return( VIPS_FOREIGN_SEQUENTIAL );
}

static int
vips_foreign_load_rad_stream_header( VipsForeignLoad *load )
{
	VipsForeignLoadRadStream *stream = (VipsForeignLoadRadStream *) load;

	if( vips__rad_header( stream->streami, load->out ) )
		return( -1 );

	return( 0 );
}

static int
vips_foreign_load_rad_stream_load( VipsForeignLoad *load )
{
	VipsForeignLoadRadStream *stream = (VipsForeignLoadRadStream *) load;

	if( vips__rad_load( stream->streami, load->real ) )
		return( -1 );

	return( 0 );
}

static void
vips_foreign_load_rad_stream_class_init( VipsForeignLoadRadStreamClass *class )
{
	GObjectClass *gobject_class = G_OBJECT_CLASS( class );
	VipsObjectClass *object_class = (VipsObjectClass *) class;
	VipsForeignLoadClass *load_class = (VipsForeignLoadClass *) class;

	gobject_class->set_property = vips_object_set_property;
	gobject_class->get_property = vips_object_get_property;

	object_class->nickname = "radload_stream";
	object_class->description = _( "load rad from stream" );

	load_class->is_a_stream = vips__rad_israd;
	load_class->get_flags = vips_foreign_load_rad_stream_get_flags;
	load_class->header = vips_foreign_load_rad_stream_header;
	load_class->load = vips_foreign_load_rad_stream_load;

	VIPS_ARG_OBJECT( class, "streami", 1,
		_( "Streami" ),
		_( "Stream to load from" ),
		VIPS_ARGUMENT_REQUIRED_INPUT, 
		G_STRUCT_OFFSET( VipsForeignLoadRadStream, streami ),
		VIPS_TYPE_STREAMI );

}

static void
vips_foreign_load_rad_stream_init( VipsForeignLoadRadStream *stream )
{
}

typedef struct _VipsForeignLoadRad {
	VipsForeignLoad parent_object;

	/* Filename for load.
	 */
	char *filename; 

} VipsForeignLoadRad;

typedef VipsForeignLoadClass VipsForeignLoadRadClass;

G_DEFINE_TYPE( VipsForeignLoadRad, vips_foreign_load_rad, 
	VIPS_TYPE_FOREIGN_LOAD );

static VipsForeignFlags
vips_foreign_load_rad_get_flags_filename( const char *filename )
{
	/* The rad reader supports sequential read.
	 */
	return( VIPS_FOREIGN_SEQUENTIAL );
}

static VipsForeignFlags
vips_foreign_load_rad_get_flags( VipsForeignLoad *load )
{
	VipsForeignLoadRad *rad = (VipsForeignLoadRad *) load;

	return( vips_foreign_load_rad_get_flags_filename( rad->filename ) );
}

static int
vips_foreign_load_rad_is_a( const char *filename )
{
	VipsStreami *streami;
	int result;

	if( !(streami = vips_streami_new_from_file( filename )) )
		return( -1 );
	result = vips__rad_israd( streami );
	VIPS_UNREF( streami );

	return( result );
}

static int
vips_foreign_load_rad_header( VipsForeignLoad *load )
{
	VipsForeignLoadRad *rad = (VipsForeignLoadRad *) load;

	VipsStreami *streami;

	if( !(streami = vips_streami_new_from_file( rad->filename )) )
		return( -1 );
	if( vips__rad_header( streami, load->out ) ) {
		VIPS_UNREF( streami );
		return( -1 );
	}
	VIPS_UNREF( streami );

	return( 0 );
}

static int
vips_foreign_load_rad_load( VipsForeignLoad *load )
{
	VipsForeignLoadRad *rad = (VipsForeignLoadRad *) load;

	VipsStreami *streami;

	if( !(streami = vips_streami_new_from_file( rad->filename )) )
		return( -1 );
	if( vips__rad_load( streami, load->real ) ) {
		VIPS_UNREF( streami );
		return( -1 );
	}
	VIPS_UNREF( streami );

	return( 0 );
}

static void
vips_foreign_load_rad_class_init( VipsForeignLoadRadClass *class )
{
	GObjectClass *gobject_class = G_OBJECT_CLASS( class );
	VipsObjectClass *object_class = (VipsObjectClass *) class;
	VipsForeignClass *foreign_class = (VipsForeignClass *) class;
	VipsForeignLoadClass *load_class = (VipsForeignLoadClass *) class;

	gobject_class->set_property = vips_object_set_property;
	gobject_class->get_property = vips_object_get_property;

	object_class->nickname = "radload";
	object_class->description = _( "load a Radiance image from a file" );

	foreign_class->suffs = vips__rad_suffs;

	/* is_a() is not that quick ... lower the priority.
	 */
	foreign_class->priority = -50;

	load_class->is_a = vips_foreign_load_rad_is_a;
	load_class->get_flags_filename = 
		vips_foreign_load_rad_get_flags_filename;
	load_class->get_flags = vips_foreign_load_rad_get_flags;
	load_class->header = vips_foreign_load_rad_header;
	load_class->load = vips_foreign_load_rad_load;

	VIPS_ARG_STRING( class, "filename", 1, 
		_( "Filename" ),
		_( "Filename to load from" ),
		VIPS_ARGUMENT_REQUIRED_INPUT, 
		G_STRUCT_OFFSET( VipsForeignLoadRad, filename ),
		NULL );
}

static void
vips_foreign_load_rad_init( VipsForeignLoadRad *rad )
{
}

typedef struct _VipsForeignLoadRadBuffer {
	VipsForeignLoad parent_object;

	/* Load from a buffer.
	 */
	VipsArea *buf;

} VipsForeignLoadRadBuffer;

typedef VipsForeignLoadClass VipsForeignLoadRadBufferClass;

G_DEFINE_TYPE( VipsForeignLoadRadBuffer, vips_foreign_load_rad_buffer, 
	VIPS_TYPE_FOREIGN_LOAD );

static gboolean
vips_foreign_load_rad_buffer_is_a_buffer( const void *buf, size_t len )
{
	VipsStreami *streami;
	gboolean result;

	if( !(streami = vips_streami_new_from_memory( buf, len )) )
		return( FALSE );
	result = vips__rad_israd( streami );
	VIPS_UNREF( streami );

	return( result );
}

static VipsForeignFlags
vips_foreign_load_rad_buffer_get_flags( VipsForeignLoad *load )
{
	/* The rad reader supports sequential read.
	 */
	return( VIPS_FOREIGN_SEQUENTIAL );
}

static int
vips_foreign_load_rad_buffer_header( VipsForeignLoad *load )
{
	VipsForeignLoadRadBuffer *buffer = (VipsForeignLoadRadBuffer *) load;

	VipsStreami *streami;

	if( !(streami = vips_streami_new_from_memory( buffer->buf->data, 
		buffer->buf->length )) ) 
		return( -1 );
	if( vips__rad_header( streami, load->out ) ) {
		VIPS_UNREF( streami );
		return( -1 );
	}
	VIPS_UNREF( streami );

	return( 0 );
}

static int
vips_foreign_load_rad_buffer_load( VipsForeignLoad *load )
{
	VipsForeignLoadRadBuffer *buffer = (VipsForeignLoadRadBuffer *) load;

	VipsStreami *streami;

	if( !(streami = vips_streami_new_from_memory( buffer->buf->data, 
		buffer->buf->length )) ) 
		return( -1 );
	if( vips__rad_load( streami, load->real ) ) {
		VIPS_UNREF( streami );
		return( -1 );
	}
	VIPS_UNREF( streami );

	return( 0 );
}

static void
vips_foreign_load_rad_buffer_class_init( VipsForeignLoadRadBufferClass *class )
{
	GObjectClass *gobject_class = G_OBJECT_CLASS( class );
	VipsObjectClass *object_class = (VipsObjectClass *) class;
	VipsForeignLoadClass *load_class = (VipsForeignLoadClass *) class;

	gobject_class->set_property = vips_object_set_property;
	gobject_class->get_property = vips_object_get_property;

	object_class->nickname = "radload_buffer";
	object_class->description = _( "load rad from buffer" );

	load_class->is_a_buffer = vips_foreign_load_rad_buffer_is_a_buffer;
	load_class->get_flags = vips_foreign_load_rad_buffer_get_flags;
	load_class->header = vips_foreign_load_rad_buffer_header;
	load_class->load = vips_foreign_load_rad_buffer_load;

	VIPS_ARG_BOXED( class, "buffer", 1, 
		_( "Buffer" ),
		_( "Buffer to load from" ),
		VIPS_ARGUMENT_REQUIRED_INPUT, 
		G_STRUCT_OFFSET( VipsForeignLoadRadBuffer, buf ),
		VIPS_TYPE_BLOB );

}

static void
vips_foreign_load_rad_buffer_init( VipsForeignLoadRadBuffer *buffer )
{
}

#endif /*HAVE_RADIANCE*/

/**
 * vips_radload:
 * @filename: file to load
 * @out: (out): output image
 * @...: %NULL-terminated list of optional named arguments
 *
 * Read a Radiance (HDR) file into a VIPS image. 
 *
 * Radiance files are read as #VIPS_CODING_RAD. They have one byte for each of
 * red, green and blue, and one byte of shared exponent. Some operations (like
 * vips_extract_area()) can work directly with images in this format, but 
 * mmany (all the arithmetic operations, for example) will not. Unpack 
 * #VIPS_CODING_RAD images to 3 band float with vips_rad2float() if 
 * you want to do arithmetic on them.
 *
 * This operation ignores some header fields, like VIEW and DATE. It will not 
 * rotate/flip as the FORMAT string asks.
 *
 * Sections of this reader from Greg Ward and Radiance with kind permission. 
 *
 * See also: vips_image_new_from_file().
 *
 * Returns: 0 on success, -1 on error.
 */
int
vips_radload( const char *filename, VipsImage **out, ... )
{
	va_list ap;
	int result;

	va_start( ap, out );
	result = vips_call_split( "radload", ap, filename, out ); 
	va_end( ap );

	return( result );
}

/**
 * vips_radload_buffer:
 * @buf: (array length=len) (element-type guint8): memory area to load
 * @len: (type gsize): size of memory area
 * @out: (out): image to write
 * @...: %NULL-terminated list of optional named arguments
 *
 * Exactly as vips_radload(), but read from a HDR-formatted memory block.
 *
 * You must not free the buffer while @out is active. The 
 * #VipsObject::postclose signal on @out is a good place to free. 
 *
 * See also: vips_radload().
 *
 * Returns: 0 on success, -1 on error.
 */
int
vips_radload_buffer( void *buf, size_t len, VipsImage **out, ... )
{
	va_list ap;
	VipsBlob *blob;
	int result;

	/* We don't take a copy of the data or free it.
	 */
	blob = vips_blob_new( NULL, buf, len );

	va_start( ap, out );
	result = vips_call_split( "radload_buffer", ap, blob, out );
	va_end( ap );

	vips_area_unref( VIPS_AREA( blob ) );

	return( result );
}

/**
 * vips_radload_stream:
 * @streami: stream to load from
 * @out: (out): output image
 * @...: %NULL-terminated list of optional named arguments
 *
 * Exactly as vips_radload(), but read from a stream. 
 *
 * See also: vips_radload().
 *
 * Returns: 0 on success, -1 on error.
 */
int
vips_radload_stream( VipsStreami *streami, VipsImage **out, ... )
{
	va_list ap;
	int result;

	va_start( ap, out );
	result = vips_call_split( "radload_stream", ap, streami, out );
	va_end( ap );

	return( result );
}

