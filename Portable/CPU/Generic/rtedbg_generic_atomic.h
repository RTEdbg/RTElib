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
 * @brief  Circular buffer space reservation using atomic operations library for
 *         single-core devices. Refer to the Readme.md file for more details.
 *         The compiler must be C11 compatible or newer.
 *         Use a processor core-specific version if available, such as
 *         "rtedbg_cortex_m_mutex.h", as it is generally more optimized and
 *         faster than this generic version.
 *         On some architectures, using atomic_compare_exchange_strong_explicit()
 *         instead of atomic_compare_exchange_weak_explicit() may result in
 *         faster code execution.
 *
 * @note   RTE_RESERVE_SPACE is defined as a macro instead of an inline function
 *         because the compiler typically generates smaller code when single-shot
 *         logging is enabled.
 ******************************************************************************/

#ifndef RTEDBG_GENERIC_ATOMIC_H
#define RTEDBG_GENERIC_ATOMIC_H

#include "stdatomic.h"

#if RTE_SINGLE_SHOT_ENABLED == 0

/* Post-mortem and streaming mode debugging are possible. The code is
 * faster and smaller compared to the single-shot enabled version.
 */

#define RTE_RESERVE_SPACE(ptr, index, size)                                   \
do                                                                            \
{                                                                             \
    _Atomic uint32_t *buff_idx = (_Atomic uint32_t *)&ptr->buf_index;         \
    uint32_t temp;                                                            \
    do                                                                        \
    {                                                                         \
        temp = atomic_load_explicit(buff_idx, memory_order_relaxed);          \
        index = temp;                                                         \
        RTE_LIMIT_INDEX(index)                                                \
    }                                                                         \
    while (!atomic_compare_exchange_weak_explicit(                            \
            buff_idx, &temp, index + (size),                                  \
            memory_order_relaxed, memory_order_relaxed));                     \
} while(0)

#else   /* RTE_SINGLE_SHOT_ENABLED == 1 */

/* Single-shot and post-mortem/streaming data logging modes are possible.
 * Post-mortem logging is the default logging mode. Single-shot logging must be
 * enabled by calling the rte_init() function with the appropriate parameter.
 */

#define RTE_RESERVE_SPACE(ptr, index, size)                                   \
do {                                                                          \
    _Atomic uint32_t *buff_idx = (_Atomic uint32_t *)&ptr->buf_index;         \
    uint32_t temp;                                                            \
    do                                                                        \
    {                                                                         \
        temp = atomic_load_explicit(buff_idx, memory_order_relaxed);          \
        index = temp;                                                         \
        if (ptr->rte_cfg & RTE_SINGLE_SHOT_LOGGING_IS_ACTIVE)                 \
        {                                                                     \
            /* Check if there is enough space for the complete message */     \
            if ((index + (size)) >= (uint32_t)(RTE_BUFFER_SIZE))              \
            {                                                                 \
                RTE_STOP_MESSAGE_LOGGING();                                   \
                return;           /* Exit the __rte_msg() function. */        \
            }                                                                 \
        }                                                                     \
        RTE_LIMIT_INDEX(index)                                                \
    }                                                                         \
    while (!atomic_compare_exchange_weak_explicit(                            \
            buff_idx, &temp, index + (size),                                  \
            memory_order_relaxed, memory_order_relaxed));                     \
} while(0)

#endif /* RTE_SINGLE_SHOT_ENABLED == 0 */

#endif  // RTEDBG_GENERIC_ATOMIC_H

/*==== End of file ====*/
