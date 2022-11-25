/****************************************************************************
 *
 * ftsystem.c
 *
 *   ANSI-specific FreeType low-level system interface (body).
 *
 * Copyright (C) 1996-2021 by
 * David Turner, Robert Wilhelm, and Werner Lemberg.
 *
 * This file is part of the FreeType project, and may only be used,
 * modified, and distributed under the terms of the FreeType project
 * license, LICENSE.TXT.  By continuing to use, modify, or distribute
 * this file you indicate that you have read the license and
 * understand and accept it fully.
 *
 * Copyright 2019-2022 MicroEJ Corp. This file has been modified by MicroEJ Corp.
 */

  /**************************************************************************
   *
   * This file contains the default interface used by FreeType to access
   * low-level, i.e. memory management, i/o access as well as thread
   * synchronisation.  It can be replaced by user-specific routines if
   * necessary.
   *
   */


#include <ft2build.h>
#include FT_CONFIG_CONFIG_H
#include <freetype/internal/ftdebug.h>
#include <freetype/internal/ftstream.h>
#include <freetype/ftsystem.h>
#include <freetype/fterrors.h>
#include <freetype/fttypes.h>

#include <BESTFIT_ALLOCATOR.h>

#include "microvg_helper.h"
#include "microvg_configuration.h"

  /**************************************************************************
   *
   *                      MEMORY MANAGEMENT INTERFACE
   *
   */

  /**************************************************************************
   *
   * It is not necessary to do any error checking for the
   * allocation-related functions.  This will be done by the higher level
   * routines like ft_mem_alloc() or ft_mem_realloc().
   *
   */

BESTFIT_ALLOCATOR freetypeAllocatorInstance;

static uint8_t _ft_heap[VG_FEATURE_FREETYPE_HEAP_SIZE];

#define FT_HEAP_START   ( &_ft_heap[0] )
#define FT_HEAP_END     ( &_ft_heap[VG_FEATURE_FREETYPE_HEAP_SIZE] )

#ifdef MICROVG_MONITOR_HEAP
static uint32_t current_heap_size = 0;
#endif

  /**************************************************************************
   *
   * @Function:
   *   ft_alloc
   *
   * @Description:
   *   The memory allocation function.
   *
   * @Input:
   *   memory ::
   *     A pointer to the memory object.
   *
   *   size ::
   *     The requested size in bytes.
   *
   * @Return:
   *   The address of newly allocated block.
   */
  FT_CALLBACK_DEF( void* )
  ft_alloc( FT_Memory  memory,
            long       size )
  {
    FT_UNUSED( memory );

    void* ptr = BESTFIT_ALLOCATOR_allocate(&freetypeAllocatorInstance, size);

	#ifdef MICROVG_MONITOR_HEAP
    if(NULL != ptr) {
		current_heap_size += size;
		MEJ_LOG_INFO_MICROVG("FT Heap - alloc -> size= %d\n", current_heap_size);
    }
	#endif
    
    return ptr;
  }


  /**************************************************************************
   *
   * @Function:
   *   ft_realloc
   *
   * @Description:
   *   The memory reallocation function.
   *
   * @Input:
   *   memory ::
   *     A pointer to the memory object.
   *
   *   cur_size ::
   *     The current size of the allocated memory block.
   *
   *   new_size ::
   *     The newly requested size in bytes.
   *
   *   block ::
   *     The current address of the block in memory.
   *
   * @Return:
   *   The address of the reallocated memory block.
   */
  FT_CALLBACK_DEF( void* )
  ft_realloc( FT_Memory  memory,
              long       cur_size,
              long       new_size,
              void*      block )
  {
    FT_UNUSED( memory );
    FT_UNUSED( cur_size );

    void* _pNewBock = block;

    if ( new_size > cur_size ) {
        _pNewBock = BESTFIT_ALLOCATOR_allocate(&freetypeAllocatorInstance, new_size);

        if (_pNewBock != NULL) {
            memcpy(_pNewBock, block, cur_size);
        }

        BESTFIT_ALLOCATOR_free(&freetypeAllocatorInstance, block);
    }

	#ifdef MICROVG_MONITOR_HEAP
	if (NULL != _pNewBock) {
		current_heap_size -= cur_size;
		current_heap_size += new_size;

		MEJ_LOG_INFO_MICROVG("FT Heap - realloc -> size= %d\n", current_heap_size);
	}
	#endif

    return _pNewBock;
}

  /**************************************************************************
   *
   * @Function:
   *   ft_free
   *
   * @Description:
   *   The memory release function.
   *
   * @Input:
   *   memory ::
   *     A pointer to the memory object.
   *
   *   block ::
   *     The address of block in memory to be freed.
   */
  FT_CALLBACK_DEF( void )
  ft_free( FT_Memory  memory,
           void*      block )
  {
    FT_UNUSED( memory );

	#ifdef MICROVG_MONITOR_HEAP
    uint32_t * ptr  = (uint32_t*) block;
	current_heap_size -= *(ptr -1) & 0x7FFFFFFF;
	MEJ_LOG_INFO_MICROVG("FT Heap - free -> size= %d\n", current_heap_size);
	#endif

    BESTFIT_ALLOCATOR_free(&freetypeAllocatorInstance, block);
  }


  /**************************************************************************
   *
   *                    RESOURCE MANAGEMENT INTERFACE
   *
   */

#ifndef FT_CONFIG_OPTION_DISABLE_STREAM_SUPPORT

  /**************************************************************************
   *
   * The macro FT_COMPONENT is used in trace mode.  It is an implicit
   * parameter of the FT_TRACE() and FT_ERROR() macros, used to print/log
   * messages during execution.
   */
#undef  FT_COMPONENT
#define FT_COMPONENT  io

  /* We use the macro STREAM_FILE for convenience to extract the       */
  /* system-specific stream handle from a given FreeType stream object */
#define STREAM_FILE( stream )  ( (FT_FILE*)stream->descriptor.pointer )


  /**************************************************************************
   *
   * @Function:
   *   ft_ansi_stream_close
   *
   * @Description:
   *   The function to close a stream.
   *
   * @Input:
   *   stream ::
   *     A pointer to the stream object.
   */
  FT_CALLBACK_DEF( void )
  ft_ansi_stream_close( FT_Stream  stream )
  {
    ft_fclose( STREAM_FILE( stream ) );

    stream->descriptor.pointer = NULL;
    stream->size               = 0;
    stream->base               = NULL;
  }


  /**************************************************************************
   *
   * @Function:
   *   ft_ansi_stream_io
   *
   * @Description:
   *   The function to open a stream.
   *
   * @Input:
   *   stream ::
   *     A pointer to the stream object.
   *
   *   offset ::
   *     The position in the data stream to start reading.
   *
   *   buffer ::
   *     The address of buffer to store the read data.
   *
   *   count ::
   *     The number of bytes to read from the stream.
   *
   * @Return:
   *   The number of bytes actually read.  If `count' is zero (this is,
   *   the function is used for seeking), a non-zero return value
   *   indicates an error.
   */
  FT_CALLBACK_DEF( unsigned long )
  ft_ansi_stream_io( FT_Stream       stream,
                     unsigned long   offset,
                     unsigned char*  buffer,
                     unsigned long   count )
  {
    FT_FILE*  file;


    if ( !count && offset > stream->size )
      return 1;

    file = STREAM_FILE( stream );

    if ( stream->pos != offset )
      ft_fseek( file, (long)offset, SEEK_SET );

    return (unsigned long)ft_fread( buffer, 1, count, file );
  }


  /* documentation is in ftstream.h */

  FT_BASE_DEF( FT_Error )
  FT_Stream_Open( FT_Stream    stream,
                  const char*  filepathname )
  {
    FT_FILE*  file;


    if ( !stream )
      return FT_THROW( Invalid_Stream_Handle );

    stream->descriptor.pointer = NULL;
    stream->pathname.pointer   = (char*)filepathname;
    stream->base               = NULL;
    stream->pos                = 0;
    stream->read               = NULL;
    stream->close              = NULL;

    file = ft_fopen( filepathname, "rb" );
    if ( !file )
    {
      FT_ERROR(( "FT_Stream_Open:"
                 " could not open `%s'\n", filepathname ));

      return FT_THROW( Cannot_Open_Resource );
    }

    ft_fseek( file, 0, SEEK_END );
    stream->size = (unsigned long)ft_ftell( file );
    if ( !stream->size )
    {
      FT_ERROR(( "FT_Stream_Open:" ));
      FT_ERROR(( " opened `%s' but zero-sized\n", filepathname ));
      ft_fclose( file );
      return FT_THROW( Cannot_Open_Stream );
    }
    ft_fseek( file, 0, SEEK_SET );

    stream->descriptor.pointer = file;
    stream->read  = ft_ansi_stream_io;
    stream->close = ft_ansi_stream_close;

    FT_TRACE1(( "FT_Stream_Open:" ));
    FT_TRACE1(( " opened `%s' (%ld bytes) successfully\n",
                filepathname, stream->size ));

    return FT_Err_Ok;
  }

#endif /* !FT_CONFIG_OPTION_DISABLE_STREAM_SUPPORT */

#ifdef FT_DEBUG_MEMORY

  extern FT_Int
  ft_mem_debug_init( FT_Memory  memory );

  extern void
  ft_mem_debug_done( FT_Memory  memory );

#endif


  /* documentation is in ftobjs.h */

  FT_BASE_DEF( FT_Memory )
  FT_New_Memory( void )
  {
    //static FT_Memory  memory;
    static struct FT_MemoryRec_  memory;

    memory.user    = 0;
    memory.alloc   = ft_alloc;
    memory.realloc = ft_realloc;
    memory.free    = ft_free;

    BESTFIT_ALLOCATOR_new(&freetypeAllocatorInstance);
    BESTFIT_ALLOCATOR_initialize(&freetypeAllocatorInstance, (uint32_t) FT_HEAP_START, (uint32_t) FT_HEAP_END);

#ifdef FT_DEBUG_MEMORY
#error unsupported
      ft_mem_debug_init( memory );
#endif

    return &memory;
  }


  /* documentation is in ftobjs.h */

  FT_BASE_DEF( void )
  FT_Done_Memory( FT_Memory  memory )
  {
#ifdef FT_DEBUG_MEMORY
    ft_mem_debug_done( memory );
#endif
    ft_sfree( memory );
  }


/* END */
