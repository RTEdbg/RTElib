/*
 * Copyright (c) 2024 Branko Premzel.
 *
 * SPDX-License-Identifier: MIT
 */

/*******************************************************************************
 * @file    rtedbg_cortex_m_mutex.h
 * @author  Branko Premzel
 * @version RTEdbg library <DEVELOPMENT BRANCH>
 *
 * @brief  ARM Cortex core specific functions for the buffer space reservation.
 *         This version is for ARM Cortex cores that support mutex instructions
 *         (e.g. M3/M4/M7/M55/M85/M33). Use 'rtedbg_cortex_m.h' for all cores
 *         that do not support mutex instructions.
 *
 * @note   RTE_RESERVE_SPACE is defined as a macro instead of an inline function
 *         because the compiler typically generates smaller code when single-shot
 *         logging is enabled.
 ******************************************************************************/

/********************************************************************************
 * @brief  Reserve space in the circular buffer with the use of mutex instructions.
 *         Interrupts are therefore not disabled at all during data logging.
 ********************************************************************************/

#ifndef RTEDBG_CORTEX_M_MUTEX_H
#define RTEDBG_CORTEX_M_MUTEX_H

#if RTE_SINGLE_SHOT_ENABLED == 0

/* The post-mortem and streaming mode debugging are possible. The code is
 * faster and smaller compared to the single-shot enabled version.
 */
#define RTE_RESERVE_SPACE(ptr, buf_idx, size)                               \
do {                                                                        \
    uint32_t new_index;                                                     \
    do                                                                      \
    {                                                                       \
        buf_idx = __LDREXW(&ptr->buf_index);                                \
        RTE_LIMIT_INDEX(buf_idx)                                          \
        new_index = buf_idx + (size);                                       \
    }                                                                       \
    while (__STREXW(new_index, &ptr->buf_index) != 0);                      \
    RTE_DATA_MEMORY_BARRIER(); /* Make sure all CPU cores see the change. */\
} while(0)

#else   /* RTE_SINGLE_SHOT_ENABLED == 0 */

/* Single shot and post mortem / streaming data logging are possible.
 * Post mortem logging is the default mode. Single shot logging must be
 * enabled by calling function rte_init() with the appropriate parameter.
 */
#define RTE_RESERVE_SPACE(ptr, buf_idx, size)                               \
do {                                                                        \
    uint32_t new_index;                                                     \
    do                                                                      \
    {                                                                       \
        buf_idx = __LDREXW(&ptr->buf_index);                                \
        if (ptr->rte_cfg & RTE_SINGLE_SHOT_LOGGING_IS_ACTIVE)               \
        {                                                                   \
            /* Check if enough space for the complete message */            \
            if ((buf_idx + (size)) >= (uint32_t)(RTE_BUFFER_SIZE))          \
            {                                                               \
               RTE_STOP_MESSAGE_LOGGING();                                  \
               return;           /* and exit the __rte_msg function. */     \
            }                                                               \
        }                                                                   \
        RTE_LIMIT_INDEX(buf_idx)                                            \
        new_index = buf_idx + (size);                                       \
    }                                                                       \
    while (__STREXW(new_index, &ptr->buf_index) != 0);                      \
    RTE_DATA_MEMORY_BARRIER(); /* Make sure all CPU cores see the change. */\
} while(0)

#endif /* RTE_SINGLE_SHOT_ENABLED == 0 */

#endif  // RTEDBG_CORTEX_M_MUTEX_H

/*==== End of file ====*/
