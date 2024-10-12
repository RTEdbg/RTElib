/*
 * Copyright (c) 2024 Branko Premzel.
 *
 * SPDX-License-Identifier: MIT
 */

/*******************************************************************************
 * @file    rtedbg_generic_atomic.h
 * @author  Branko Premzel
 * @version RTEdbg library <DEVELOPMENT BRANCH>
 *
 * @brief  Circular buffer space reservation using atomic operations library.
 *         See also description in the Readme.md file.
 *         The compiler should be C11 compatible.
 *         Use the processor core specific version if available - e.g. the
 *         "rtedbg_cortex_m_mutex.h" since in general it is more optimized and
 *         faster than this generic one.
 *
 * @note   The RTE_MULTICORE_SUPPORT macro must be defined if logging data with
 *         the RTEdbg library on a multi-core processor and all cores write to a
 *         common circular data buffer located in shared RAM.
 *         See also the section 'Using the RTEdbg Library on Multi-Core Processors'
 *         in the RTEdbg Manual.
 *
 * @note   RTE_RESERVE_SPACE is defined as a macro instead of an inline function
 *         because the compiler typically generates smaller code when single-shot
 *         logging is enabled.
 ******************************************************************************/

#ifndef RTEDBG_GENERIC_ATOMIC_H
#define RTEDBG_GENERIC_ATOMIC_H

#include "stdatomic.h"

#ifndef RTE_MEMORY_ORDER
#ifdef RTE_MULTICORE_SUPPORT
#define RTE_MEMORY_ORDER memory_order_seq_cst
#else
#define RTE_MEMORY_ORDER memory_order_relaxed
#endif
#endif // RTE_MEMORY_ORDER

#if RTE_SINGLE_SHOT_ENABLED == 0

/* The post-mortem and streaming mode debugging are possible. The code is
 * faster and smaller compared to the single-shot enabled version.
 */

#define RTE_RESERVE_SPACE(ptr, buf_idx, size)                               \
do                                                                          \
{                                                                           \
    _Atomic uint32_t * buff_idx = (_Atomic uint32_t *)&ptr->buf_index;      \
    uint32_t new_index;                                                     \
    uint32_t tmp;                                                           \
    do                                                                      \
    {                                                                       \
        tmp = atomic_load_explicit(buff_idx, RTE_MEMORY_ORDER);             \
        buf_idx = tmp;                                                      \
        RTE_LIMIT_INDEX(buf_idx)                                            \
        new_index = buf_idx + (size);                                       \
        tmp = atomic_compare_exchange_weak_explicit(                        \
            buff_idx, &tmp, new_index, RTE_MEMORY_ORDER, RTE_MEMORY_ORDER); \
    }                                                                       \
    while (tmp == 0);                                                       \
} while(0)

#else   /* RTE_SINGLE_SHOT_ENABLED == 1 */

/* Single shot and post mortem / streaming data logging are possible.
 * Post mortem logging is the default mode. Single shot logging must be
 * enabled by calling function rte_init() with the appropriate parameter.
 */

#define RTE_RESERVE_SPACE(ptr, buf_idx, size)                               \
do {                                                                        \
    _Atomic uint32_t * buff_idx = (_Atomic uint32_t *)&ptr->buf_index;      \
    uint32_t new_index;                                                     \
    uint32_t tmp;                                                           \
    do                                                                      \
    {                                                                       \
        tmp = atomic_load_explicit(buff_idx, RTE_MEMORY_ORDER);             \
        buf_idx = tmp;                                                      \
        if (ptr->rte_cfg & RTE_SINGLE_SHOT_LOGGING_IS_ACTIVE)               \
        {                                                                   \
            /* Check if there is enough space for the complete message */   \
            if ((buf_idx + (size)) >= (uint32_t)(RTE_BUFFER_SIZE))          \
            {                                                               \
               RTE_STOP_MESSAGE_LOGGING();                                  \
               return;           /* Exit the __rte_msg function. */         \
            }                                                               \
        }                                                                   \
        RTE_LIMIT_INDEX(buf_idx)                                            \
        new_index = buf_idx + (size);                                       \
        tmp = atomic_compare_exchange_weak_explicit(                        \
            buff_idx, &tmp, new_index, RTE_MEMORY_ORDER, RTE_MEMORY_ORDER); \
    }                                                                       \
    while (tmp == 0);                                                       \
} while(0)

#endif /* RTE_SINGLE_SHOT_ENABLED == 0 */

#endif  // RTEDBG_GENERIC_ATOMIC_H

/*==== End of file ====*/
