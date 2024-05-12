/*
 * Copyright (c) 2024 Branko Premzel.
 *
 * SPDX-License-Identifier: MIT
 */

/*******************************************************************************
 * @file    rtedbg_cortex_m.h
 * @author  Branko Premzel
 * @version RTEdbg library <DEVELOPMENT BRANCH>
 *
 *
 * @brief  ARM Cortex core specific functions for the buffer space reservation.
 *         This version is suitable for ARM Cortex cores that do not support
 *         mutex instructions (e.g. M0/M0+/M23). Use 'rtedbg_cortex_m_mutex.h'
 *         for all cores that support mutex instructions. Do not use this file for
 *         Cortex M cores that support privileged mode since the interrupt disable
 *         has no effect if used in unprivileged software, e.g. in a RTOS task.
 *         Interrupts are disabled for only a few CPU cycles while space is
 *         reserved in circular memory.
 *
 * @note   RTE_RESERVE_SPACE is defined as a macro instead of an inline function
 *         because the compiler typically generates smaller code when single-shot
 *         logging is enabled.
 ******************************************************************************/

/********************************************************************************
 * @brief  Reserve space in the circular buffer. Since the Cortex M0 and M0+ cores do
 *         not support mutex instructions, interrupts should only be disabled for the
 *         duration of the space reservation and not while copying data to the buffer.
 *         Interrupts are therefore only disabled for a very short time.
 ********************************************************************************/

#ifndef RTEDBG_CORTEX_M_H
#define RTEDBG_CORTEX_M_H

#if RTE_SINGLE_SHOT_ENABLED == 0

/* The post-mortem and streaming debugging mode are possible. The code is faster and
 * smaller compared to the single-shot enabled version.
 */
#define RTE_RESERVE_SPACE(ptr, buf_idx, size)                        \
do {                                                                 \
    uint32_t primask = __get_PRIMASK();                              \
    __disable_irq();                                                 \
    buf_idx = ptr->buf_index;                                        \
    RTE_LIMIT_INDEX(buf_idx)                                         \
    ptr->buf_index = buf_idx + (size);                               \
    if (primask == 0U)                                               \
    {                                                                \
        __enable_irq();                                              \
    }                                                                \
} while(0)

#else   /* RTE_SINGLE_SHOT_ENABLED == 0 */

/* Single shot and post mortem / streaming data logging are possible.
 * Post mortem logging is the default mode. Single shot logging must be
 * enabled by calling function rte_init() with the appropriate parameter.
 */
#define RTE_RESERVE_SPACE(ptr, buf_idx, size)                        \
do {                                                                 \
    uint32_t primask = __get_PRIMASK();                              \
    __disable_irq();                                                 \
    buf_idx = ptr->buf_index;                                        \
    if (ptr->rte_cfg & RTE_SINGLE_SHOT_LOGGING_IS_ACTIVE)            \
    {                                                                \
        /* Check if enough space for the complete message */         \
        if ((buf_idx + (size)) >= (uint32_t)(RTE_BUFFER_SIZE))       \
        {                                                            \
            RTE_STOP_MESSAGE_LOGGING();                              \
            if (primask == 0U)                                       \
            {                                                        \
               __enable_irq();                                       \
            }                                                        \
            return;        /* and exit the __rte_msg function. */    \
        }                                                            \
    }                                                                \
    RTE_LIMIT_INDEX(buf_idx)                                         \
    ptr->buf_index = buf_idx + (size);                               \
    if (primask == 0U)                                               \
    {                                                                \
        __enable_irq();                                              \
    }                                                                \
} while(0)
#endif /* RTE_SINGLE_SHOT_ENABLED == 0 */

#endif  // RTEDBG_CORTEX_M_H

/*==== End of file ====*/

