// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

/** @file az_pal_os_api.h
*    @brief      A minimalistic platform agnostic abstraction.
*/

#ifndef AZ_PAL_OS_API_H
#define AZ_PAL_OS_API_H

#include "azure_macro_utils/macro_utils.h"
#include "umock_c/umock_c_prod.h"

#include "az_pal_os.h"

#ifdef __cplusplus
extern "C" {
#endif

    /**
     * @brief    This API initialize and returns a valid lock handle.
     *
     * @param[in/out]   lock    The <tt>AZ_PAL_OS_LOCK *</tt> that points to the lock handle.
     */
    MOCKABLE_FUNCTION(, void, az_pal_os_lock_init, AZ_PAL_OS_LOCK *, lock);

    /**
     * @brief    The lock instance is destroyed.
     *
     * @param[in]       lock    The <tt>AZ_PAL_OS_LOCK *</tt> that points to a valid lock handle.
     */
    MOCKABLE_FUNCTION(, void, az_pal_os_lock_deinit, AZ_PAL_OS_LOCK *, lock);

    /**
     * @brief    Acquires a lock on the given lock handle. Uses platform
     *             specific mutex primitives in its implementation.
     *
     * @param[in]       lock    The <tt>AZ_PAL_OS_LOCK *</tt> that points to a valid lock handle.
     */
    MOCKABLE_FUNCTION(, void, az_pal_os_lock_acquire, AZ_PAL_OS_LOCK *, lock);

    /**
     * @brief    Releases the lock on the given lock handle. Uses platform
     *             specific mutex primitives in its implementation.
     *
     * @param[in]       lock    The <tt>AZ_PAL_OS_LOCK *</tt> that points to a valid lock handle.
     */
    MOCKABLE_FUNCTION(, void, az_pal_os_lock_release, AZ_PAL_OS_LOCK *, lock);

#ifdef __cplusplus
}
#endif

#endif /* AZ_PAL_OS_API_H */