/*
 * Copyright 2020-2022 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */

#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include "LLEXT_RES_impl.h"
#include "sni.h"

/*
 * Fake external resource loader: load resources from SOAR space using hidden API
 * "SNIX_get_resource". First, add a '/' to the given path, look for resource. If
 * not found, add suffix '_ext'.
 */

/* Defines -------------------------------------------------------------------*/

#define PATH__BUFFER_SIZE 500
#define NB_RESOURCES 10
#define FREE_BLOCK 0

/* Types ---------------------------------------------------------------------*/

typedef struct {
	void* data;
	uint32_t size;
	uint32_t offset; // only for this driver (not required by SNIX_get_resource())
	bool addressable; // only for this driver (not required by SNIX_get_resource())
} SNIX_resource;


/* Extern --------------------------------------------------------------------*/

extern int32_t SNIX_get_resource(char* path, SNIX_resource* resource);

/* Globals -------------------------------------------------------------------*/

static char g_path_buffer[PATH__BUFFER_SIZE];
static SNIX_resource g_resources[NB_RESOURCES];
static bool initialized = false;

static int32_t get_free_block(void)
{
	for(int32_t free_block = 0; free_block<NB_RESOURCES; free_block++)
	{
		if (FREE_BLOCK == g_resources[free_block].size)
		{
			return free_block;
		}				
	}
	return -1;
}

/**
 * Open the resource whose name is the string pointed to by path.
 * @param path a null terminated string.
 * @return the resource ID on success, a negative value on error.
 */
RES_ID LLEXT_RES_open(const char* path)
{	
	printf("%s: %s\n", __FUNCTION__, path);

	// look for free block
	int32_t free_block;
	if (!initialized)
	{
		for(free_block = 0; free_block<NB_RESOURCES; free_block++)
		{
			g_resources[free_block].size = FREE_BLOCK;
			g_resources[free_block].offset = 0;
			g_resources[free_block].addressable = false;
		}
		initialized = true;
		free_block = 0;
	}
	else
	{
		free_block = get_free_block();
		if (free_block == -1)
		{
			printf("[%s] no more free block to load resource %s.", __FUNCTION__, path);	
			return -1;			
		}
	}
	SNIX_resource* p_res = &g_resources[free_block];

	// check for path length
	char* local_path = (char*)path;
	size_t path_length = strlen(local_path);
	size_t size_max = PATH__BUFFER_SIZE - 6 /*"_naddr"*/;
	if ( path_length > size_max ) {
		printf("[%s] path too long (limited to %d characters): %s (%d characters).", __FUNCTION__, size_max, local_path, path_length);
		return -1;
	}

	// copy path in path_buffer and add prefix '/'
	strncpy( g_path_buffer + 1, path, path_length + 1 /* /0 */ );
	*g_path_buffer = '/';
	// printf("AAA path = %s\npath_buffer = %s\n", path, g_path_buffer);

	// look for resource
	int32_t ret = SNIX_get_resource((char*)g_path_buffer, p_res);
	if (ret == 0)
	{
		// resource found
		p_res->addressable = true;
		printf("FOUNDED %s 0x%p %u (addr=%u)\n", g_path_buffer, p_res->data, p_res->size, p_res->addressable );
		return (RES_ID)free_block;
	}

	// add '_addr' at the end of path
	char* tmp_path = (char*)g_path_buffer;
	tmp_path++; // '/'
	tmp_path += path_length;
	*tmp_path++ = '_';
	*tmp_path++ = 'a';
	*tmp_path++ = 'd';
	*tmp_path++ = 'd';
	*tmp_path++ = 'r';
	*tmp_path++ = '\0';
	//printf("BBB path = %s\npath_buffer = %s\n", path, g_path_buffer);

	ret = SNIX_get_resource((char*)g_path_buffer, p_res);
	if (ret == 0)
	{
		// resource found
		p_res->addressable = true;
		printf("FOUNDED %s 0x%p %u (addr=%u)\n", g_path_buffer, p_res->data, p_res->size, p_res->addressable );
		return (RES_ID)free_block;
	}

	// add '_naddr' at the end of path
	tmp_path = (char*)g_path_buffer;
	tmp_path++; // '/'
	tmp_path += path_length;
	*tmp_path++ = '_';
	*tmp_path++ = 'n';
	*tmp_path++ = 'a';
	*tmp_path++ = 'd';
	*tmp_path++ = 'd';
	*tmp_path++ = 'r';
	*tmp_path++ = '\0';
	//printf("CCC path = %s\npath_buffer = %s\n", path, g_path_buffer);

	ret = SNIX_get_resource((char*)g_path_buffer, p_res);
	if (ret == 0)
	{
		// resource found
		p_res->addressable = false;
		printf("FOUNDED %s 0x%p %u (addr=%u)\n", g_path_buffer, p_res->data, p_res->size, p_res->addressable );
		return (RES_ID)free_block;
	}
	// not found
	return -1;	
}

/**
 * Close the resource referenced by the given resourceID
 * @param resourceID an ID returned by the LLEXT_RES_open function.
 * @return LLEXT_RES_OK on success, a negative value on error.
 */
int32_t LLEXT_RES_close(RES_ID resourceID)
{
	int32_t id = (int32_t)resourceID;
	if (id < 0 ||  FREE_BLOCK == g_resources[id].size)
	{		
		// invalid resource or resource has been already closed!
		printf("resource already closed! %u: %u\n", resourceID, g_resources[(int32_t)resourceID].size);

		// error code may not used by GE: force a timeout
		while(1);
		//return -1;
	}
	printf("CLOSE %u\n", id);
	g_resources[id].size = FREE_BLOCK;
	g_resources[id].offset = 0;
	g_resources[id].addressable = false;
	return LLEXT_RES_OK;
}

/**
 * Returns the resource base address or -1 when not available.
 *
 * The resource base address is an address available in CPU address space range. That means the
 * CPU can access to the resource memory using the same assembler instructions than the CPU internal memories.
 * In this case this memory is not considered as external memory. This address will be used by
 * the caller even if the resource is closed.
 *
 * If the memory is outside the CPU address space range, this function should returns -1. In this
 * case the caller will use this memory as external memory and will have to perform some memory copies in RAM.
 * The memory accesses may be too slow. To force the caller to consider this memory as an external
 * memory whereas this memory is available in CPU address space range, this function should returns -1 too.
 *
 * The returned address is always the resource base address. This address must not change during application
 * execution.
 *
 * @param resourceID an ID returned by the openResource method.
 * @return resource address or -1 when address is outside CPU address space range.
 */
int32_t LLEXT_RES_getBaseAddress(RES_ID resourceID)
{
	return g_resources[(int32_t)resourceID].addressable ? (int32_t)g_resources[(int32_t)resourceID].data : -1;
}

/**
 * Try to read size bytes from the stream referenced by the given resourceID.
 * The read bytes are copied at given ptr. If given ptr is <code>NULL</code>
 * then size bytes are skipped from the stream. Parameter <code>size</code>
 * is a pointer on an integer which defines the maximum number of bytes to
 * read or skip. This value must be updated by the number of bytes read or
 * skipped.
 * @param resourceID an ID returned by the LLEXT_RES_open function.
 * @param ptr the buffer into which the data is read. <code>NULL</code> to skip data.
 * @param size the maximum number of bytes to read or skip.
 * @return LLEXT_RES_OK on success, LLEXT_RES_EOF when end of file is reached, another negative value on error.
 */
int32_t LLEXT_RES_read(RES_ID resourceID, void* ptr, int32_t* size)
{
	if (NULL != ptr)
	{
		uint8_t* addr = (uint8_t*)g_resources[(int32_t)resourceID].data;
		addr += g_resources[(int32_t)resourceID].offset;
		memcpy(ptr, (void*)addr, (size_t)*size);		
	}
	g_resources[(int32_t)resourceID].offset += *size;
	return LLEXT_RES_OK;
}

/**
 * Returns an estimate of the number of bytes that can be read from the stream referenced by the given resourceID without blocking by the next read.
 * @param resourceID an ID returned by the LLEXT_RES_open function.
 * @return an estimate of the number of bytes that can be read, 0 when end of file is reached, a negative value on error.
 */
int32_t LLEXT_RES_available(RES_ID resourceID)
{	
	return g_resources[(int32_t)resourceID].size - g_resources[(int32_t)resourceID].offset;
}

/**
 * Sets the file position indicator for the stream pointed to by resourceID.
 * The new position, measured in bytes, is obtained by adding offset bytes to the start of the file.
 * @param resourceID an ID returned by the LLEXT_RES_open function.
 * @param offset new value in bytes of the file position indicator.
 * @return LLEXT_RES_OK on success, a negative value on error.
 */
int32_t LLEXT_RES_seek(RES_ID resourceID, int64_t offset)
{
	g_resources[(int32_t)resourceID].offset = (int32_t) offset;
	return LLEXT_RES_OK;
}

/**
 * Obtains the current value of the file position indicator for the stream pointed to by resourceID.
 * @param resourceID an ID returned by the LLEXT_RES_open function.
 * @return the current value in bytes of the file position indicator on success, a negative value on error.
 */
int64_t LLEXT_RES_tell(RES_ID resourceID)
{	
	return g_resources[(int32_t)resourceID].offset;
}
