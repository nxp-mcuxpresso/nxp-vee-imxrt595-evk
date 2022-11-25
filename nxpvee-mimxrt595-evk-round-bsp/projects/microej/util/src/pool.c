/*
 * C
 *
 * Copyright 2014-2021 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */

#include <stdio.h>
#include "pool.h"

POOL_status_t POOL_reserve_f(POOL_ctx_t * _st_pool_ctx,
		                     void ** _ppv_item_reserved)
{
	POOL_status_t e_return;
	unsigned char uc_i;
	unsigned char uc_found = 0;

	/* test entry function */
	if ((_ppv_item_reserved != NULL) &&
		 (_st_pool_ctx != NULL) &&
		 (_st_pool_ctx->pv_first_item != NULL))
	{
		/* looking for a free place in pool */
		for (uc_i = 0;(uc_i < _st_pool_ctx->uc_num_item_in_pool) && (!uc_found);uc_i++)
		{
			if (_st_pool_ctx->puc_item_status[uc_i] != POOL_USED)
			{
				uc_found = 1;
				_st_pool_ctx->puc_item_status[uc_i] = POOL_USED;
				*_ppv_item_reserved = (void*)((unsigned char*)_st_pool_ctx->pv_first_item + (uc_i * _st_pool_ctx->ui_size_of_item));
			}
		}

		/* test if poll is full */
		if (!uc_found)
		{
			e_return = POOL_NO_SPACE_AVAILABLE;
		}
		else
		{
			e_return = POOL_NO_ERROR;
		}
	}
	else
	{
		e_return = POOL_ERROR_IN_ENTRY_PARAMETERS;
	}

	return (e_return);
}


POOL_status_t POOL_free_f(POOL_ctx_t * _st_pool_ctx,
		                  void * const _pv_item_to_free)
{
	POOL_status_t e_return;
	unsigned char uc_i;
	unsigned char uc_found = 0;

	/* test entry function */
	if ((_pv_item_to_free != NULL) &&
		(_st_pool_ctx != NULL) &&
		(_st_pool_ctx->pv_first_item != NULL))
	{
		/* looking for item index to free place in pool */
		for (uc_i = 0;(uc_i < _st_pool_ctx->uc_num_item_in_pool) && (!uc_found);uc_i++)
		{
			if ((void*)((unsigned char*)(_st_pool_ctx->pv_first_item) + (uc_i * _st_pool_ctx->ui_size_of_item)) == _pv_item_to_free)
			{
				uc_found = 1;
				_st_pool_ctx->puc_item_status[uc_i] = POOL_FREE;
			}
		}

		/* test if item is found */
		if (!uc_found)
		{
			e_return = POOL_ITEM_NOT_FOUND_IN_POOL;
		}
		else
		{
			e_return = POOL_NO_ERROR;
		}
	}
	else
	{
		e_return = POOL_ERROR_IN_ENTRY_PARAMETERS;
	}

	return (e_return);
}


