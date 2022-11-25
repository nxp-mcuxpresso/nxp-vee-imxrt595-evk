/*
 * C
 *
 * Copyright 2019-2022 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */

#if !defined __MEJ_DISABLE_H__
# define __MEJ_DISABLE_H__

/*
 * @brief Display tearing signal prevents glitches, however it reduces the maximum frame per seconds.
 *
 * This define disables the use of tearing. Comment it to enable tearing.
 */
#define MEJ_DISABLED_DISPLAY_TEARING

#endif // __MEJ_DISABLE_H__

