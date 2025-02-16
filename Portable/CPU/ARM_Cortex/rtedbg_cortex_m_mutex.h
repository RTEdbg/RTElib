/*
 * Copyright (c) Branko Premzel.
 *
 * SPDX-License-Identifier: MIT
 */

/*******************************************************************************
 * @file    rtedbg_cortex_m_mutex.h
 * @author  Branko Premzel
 * @version RTEdbg library <DEVELOPMENT BRANCH>
 *
 * @brief  ARM Cortex core-specific functions for buffer space reservation.
 *         This version is for ARM Cortex-M cores that support exclusive access
 *         instructions (e.g., M3/M4/M7/M55/M85/M33). Use 'rtedbg_generic_irq_disable.h'
 *         for ARM Cortex-M cores that do not support these instructions.
 *
 * @note   This driver version is suitable for single-core devices or multi-core
 *         devices where data logging is performed for only one core or separately
 *         for each core.
 *         Use the generic symmetric multi-core device driver with support for 
 *         the atomic operations library in "rtedbg_generic_atomic_smp.h" and follow
 *         the instructions in the readme and RTEdbg manual if you plan to use a
 *         common g_rtedbg data logging structure for all CPU cores. Also, follow
 *         processor family memory sharing instructions.
 *
 * @note   RTE_RESERVE_SPACE is defined as a macro instead of an inline function
 *         because the compiler typically generates smaller code when single-shot
 *         logging is enabled.
 ******************************************************************************/

/********************************************************************************
 * @brief  Reserve space in the circular buffer using exclusive access instructions.
 *         Interrupts are not disabled during data logging.
 ********************************************************************************/

#ifndef RTEDBG_CORTEX_M_MUTEX_H
#define RTEDBG_CORTEX_M_MUTEX_H

#if RTE_SINGLE_SHOT_ENABLED == 0

/* Post-mortem and streaming mode debugging are possible. The code is
 * faster and smaller compared to the single-shot enabled version.
 */
#define RTE_RESERVE_SPACE(ptr, buf_idx, size)                               \
do {                                                                        \
    uint32_t new_index;                                                     \
    do                                                                      \
    {                                                                       \
        buf_idx = __LDREXW(&ptr->buf_index);                                \
        RTE_LIMIT_INDEX(buf_idx)                                            \
        new_index = buf_idx + (size);                                       \
    }                                                                       \
    while (__STREXW(new_index, &ptr->buf_index) != 0);                      \
} while(0)

#else   /* RTE_SINGLE_SHOT_ENABLED == 1 */

/* Single-shot and post-mortem/streaming data logging are possible.
 * Post-mortem logging is the default mode. Single-shot logging must be
 * enabled by calling the function rte_init() with the appropriate parameter.
 */
#define RTE_RESERVE_SPACE(ptr, buf_idx, size)                               \
do {                                                                        \
    uint32_t new_index;                                                     \
    do                                                                      \
    {                                                                       \
        buf_idx = __LDREXW(&ptr->buf_index);                                \
        if (ptr->rte_cfg & RTE_SINGLE_SHOT_LOGGING_IS_ACTIVE)               \
        {                                                                   \
            /* Check if there is enough space for the complete message */   \
            if ((buf_idx + (size)) >= (uint32_t)(RTE_BUFFER_SIZE))          \
            {                                                               \
               RTE_STOP_MESSAGE_LOGGING();                                  \
               __CLREX();                                                   \
               return;           /* Exit the __rte_msg function. */         \
            }                                                               \
        }                                                                   \
        RTE_LIMIT_INDEX(buf_idx)                                            \
        new_index = buf_idx + (size);                                       \
    }                                                                       \
    while (__STREXW(new_index, &ptr->buf_index) != 0);                      \
} while(0)

#endif /* RTE_SINGLE_SHOT_ENABLED == 0 */

#endif  // RTEDBG_CORTEX_M_MUTEX_H

/*==== End of file ====*/
