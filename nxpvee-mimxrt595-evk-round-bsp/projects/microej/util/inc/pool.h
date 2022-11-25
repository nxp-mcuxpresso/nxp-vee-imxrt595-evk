/*
 * C
 *
 * Copyright 2014-2021 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */

/*
 * The module provide function to simply manage a
 * Fixed memory pool size.
 *
 * @attention the maximum number of item in pool is 255.
 */

#ifndef POOL_FIXED_SIZE_MEMORY_MANAGEMENT_H
#define POOL_FIXED_SIZE_MEMORY_MANAGEMENT_H

/** @brief list of status for one element */
typedef enum
{
	POOL_FREE,
	POOL_USED
}POOL_item_status_t;

/** @brief define pool type */
typedef struct {
	void * pv_first_item;                 /**< pointer on first element in pool */
	POOL_item_status_t * puc_item_status; /**< pointer on array status item */
	unsigned int ui_size_of_item;         /**< size of one element */
	unsigned char uc_num_item_in_pool;    /**< number of element in pool */
}POOL_ctx_t;

/** @brief list of module constant */
typedef enum
{
	POOL_NO_ERROR,
	POOL_ERROR_IN_ENTRY_PARAMETERS,
	POOL_NO_SPACE_AVAILABLE,
	POOL_ITEM_NOT_FOUND_IN_POOL,
}POOL_status_t;

/**
 * @brief function to reserved one place in pool
 *
 * @param[in,out] _st_pool_ctx        pool context
 * @param[out]    _ppv_item_reserved  pointer on reserved item
 *
 * @return @see POOL_status_t
 */
POOL_status_t POOL_reserve_f(POOL_ctx_t * _st_pool_ctx,
		                     void ** _ppv_item_reserved);


/**
 * @brief function to reserved one place in pool
 *
 * @param[in,out] _st_pool_ctx      pool context
 * @param[in]     _pv_item_to_free  pointer item to free
 *
 * @return @see POOL_status_t
 */
POOL_status_t POOL_free_f(POOL_ctx_t * _st_pool_ctx,
		                  void * const _pv_item_to_free);


#endif /* POOL_FIXED_SIZE_MEMORY_MANAGEMENT_H */
