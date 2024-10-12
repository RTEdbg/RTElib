/*
 * Copyright (c) 2024 Branko Premzel.
 *
 * SPDX-License-Identifier: MIT
 */

/*******************************************************************************
 * @file    rtedbg_generic_irq_disable.h
 * @author  Branko Premzel
 * @version RTEdbg library <DEVELOPMENT BRANCH>
 *
 * @brief  Circular buffer space reservation using interrupt disable / enable.
 *         => Check for the processor core-specific version, if available. 
 *         It may be more optimized and faster than this generic one.
 *         See also description in the Readme.md file.
 *
 *         Do not use this file for cores that support privileged mode since the
 *         interrupt disable has no effect if used in unprivileged software (such
 *         as the ARM Cortex CPU cores), e.g. in a RTOS task.
 *
 *         Interrupts are disabled for only a few CPU cycles while space is
 *         reserved in circular memory.
 *
 * The following macros have to be added to the rtedbg_config.h project file:
 *  #define RTE_ENTER_CRITICAL() // Save status indicating whether interrupts are enabled and disable them
 *  #define RTE_EXIT_CRITICAL()  // Enable interrupts again, if they have been enabled before.
 * 
 * Example for the ARM Cortex M0/M0+/M23:
 *  #define RTE_ENTER_CRITICAL()  uint32_t irq_tmp = __get_PRIMASK(); __disable_irq();
 *  #define RTE_EXIT_CRITICAL()   if (irq_tmp == 0U) { __enable_irq(); }
 *
 * @note   RTE_RESERVE_SPACE is defined as a macro instead of an inline function
 *         because the compiler typically generates smaller code when single-shot
 *         logging is enabled.
 ******************************************************************************/

#ifndef RTEDBG_GENERIC_IRQ_DISABLE_H
#define RTEDBG_GENERIC_IRQ_DISABLE_H

#if RTE_SINGLE_SHOT_ENABLED == 0

/* The post-mortem and streaming debugging mode are possible. The code is faster and
 * smaller compared to the single-shot enabled version.
 */
#define RTE_RESERVE_SPACE(ptr, buf_idx, size)                        \
do {                                                                 \
    RTE_ENTER_CRITICAL()                                             \
    buf_idx = ptr->buf_index;                                        \
    RTE_LIMIT_INDEX(buf_idx)                                         \
    ptr->buf_index = buf_idx + (size);                               \
    RTE_EXIT_CRITICAL()                                              \
} while(0)

#else   /* RTE_SINGLE_SHOT_ENABLED == 1 */

/* Single shot and post mortem / streaming data logging are possible.
 * Post mortem logging is the default mode. Single shot logging must be
 * enabled by calling function rte_init() with the appropriate parameter.
 */
#define RTE_RESERVE_SPACE(ptr, buf_idx, size)                        \
do {                                                                 \
    RTE_ENTER_CRITICAL()                                             \
    buf_idx = ptr->buf_index;                                        \
    if (ptr->rte_cfg & RTE_SINGLE_SHOT_LOGGING_IS_ACTIVE)            \
    {                                                                \
        /* Check if enough space for the complete message */         \
        if ((buf_idx + (size)) >= (uint32_t)(RTE_BUFFER_SIZE))       \
        {                                                            \
            RTE_STOP_MESSAGE_LOGGING();                              \
            RTE_EXIT_CRITICAL()                                      \
            return;        /* and exit the __rte_msg function. */    \
        }                                                            \
    }                                                                \
    RTE_LIMIT_INDEX(buf_idx)                                         \
    ptr->buf_index = buf_idx + (size);                               \
    RTE_EXIT_CRITICAL()                                              \
} while(0)
#endif /* RTE_SINGLE_SHOT_ENABLED == 0 */

#endif  // RTEDBG_GENERIC_IRQ_DISABLE_H

/*==== End of file ====*/

