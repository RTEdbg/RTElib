/*
 * Copyright (c) 2024 Branko Premzel.
 *
 * SPDX-License-Identifier: MIT
 */

/*******************************************************************************
 * @file    rtedbg_generic_atomic_smp.h
 * @author  Branko Premzel
 * @version RTEdbg library <DEVELOPMENT BRANCH>
 *
 * @brief  Circular buffer space reservation using atomic operations library for
 *         multi-core devices. See also description in the Readme.md file.
 *         The compiler should be C11 compatible or newer.
 *         It is possible to use it on single-core processors, but the conservative
 *         use of memory barriers will result in excessive CPU time consumption.
 *         For single-core processors, the "rtedbg_generic_atomic.h" driver is
 *         generally more suitable.
 *
 * @note   Verify that this driver is suitable for your device and make any necessary
 *         adjustments. Use atomic_load_explicit() and atomic_compare_exchange_weak_explicit()
 *         or atomic_compare_exchange_strong_explicit() with the most appropriate
 *         memory_order parameter values. This way, you get faster code execution and
 *         the best solution for your architecture. The atomic_compare_exchange_weak()
 *         and atomic_load() functions use the memory_order_seq_cst memory order - the
 *         most conservative one.
 *         See also the section 'Using the RTEdbg Library on Multi-Core Processors' in
 *         the RTEdbg Manual. Also follow processor family memory sharing instructions.
 *
 * @note   RTE_RESERVE_SPACE is defined as a macro instead of an inline function
 *         because the compiler typically generates smaller code when single-shot
 *         logging is enabled.
 ******************************************************************************/

#ifndef RTEDBG_GENERIC_ATOMIC_SMP_H
#define RTEDBG_GENERIC_ATOMIC_SMP_H

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
        temp = atomic_load(buff_idx);                                         \
        index = temp;                                                         \
        RTE_LIMIT_INDEX(index)                                                \
    }                                                                         \
    while (!atomic_compare_exchange_weak(buff_idx, &temp, index + (size)));   \
    atomic_thread_fence(memory_order_release);                                \
} while(0)

#else   /* RTE_SINGLE_SHOT_ENABLED == 1 */

/* Single-shot and post-mortem/streaming data logging modes are possible.
 * Post-mortem logging is the default logging mode. Single-shot logging must be
 * enabled by calling the function rte_init() with the appropriate parameter.
 */

#define RTE_RESERVE_SPACE(ptr, index, size)                                   \
do {                                                                          \
    _Atomic uint32_t *buff_idx = (_Atomic uint32_t *)&ptr->buf_index;         \
    uint32_t temp;                                                            \
    do                                                                        \
    {                                                                         \
        temp = atomic_load(buff_idx);                                         \
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
    while (!atomic_compare_exchange_weak(buff_idx, &temp, index + (size)));   \
    atomic_thread_fence(memory_order_release);                                \
} while(0)

#endif /* RTE_SINGLE_SHOT_ENABLED == 0 */

#endif  // RTEDBG_GENERIC_ATOMIC_SMP_H

/*==== End of file ====*/
