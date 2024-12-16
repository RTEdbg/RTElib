/*
 * Copyright (c) 2024 Branko Premzel.
 *
 * SPDX-License-Identifier: MIT
 */

/*******************************************************************************
 * @file    rtedbg_generic_non_reentrant.h
 * @author  Branko Premzel
 * @version RTEdbg library <DEVELOPMENT BRANCH>
 *
 * @brief  Driver for projects where re-entry of data logging is not required.
 *         Logging functions are faster and shorter. The programmer must ensure that
 *         no logging functions are executed simultaneously in any part of the code.
 ******************************************************************************/

#ifndef RTEDBG_GENERIC_NON_REENTRANT_H
#define RTEDBG_GENERIC_NON_REENTRANT_H

#if RTE_SINGLE_SHOT_ENABLED == 0

/* Post-mortem and streaming debugging modes are possible. The code
 * is faster and smaller compared to the single-shot enabled version.
 */
#define RTE_RESERVE_SPACE(ptr, buf_idx, size)   \
    buf_idx = ptr->buf_index;                   \
    RTE_LIMIT_INDEX(buf_idx)                    \
    ptr->buf_index = buf_idx + (size);

#else   /* RTE_SINGLE_SHOT_ENABLED == 1 */

/* Single-shot and post-mortem/streaming data logging are possible.
 * Post-mortem logging is the default mode. Single-shot logging must be
 * enabled by calling the function rte_init() with the appropriate parameter.
 */
#define RTE_RESERVE_SPACE(ptr, buf_idx, size)                        \
    buf_idx = ptr->buf_index;                                        \
    if (ptr->rte_cfg & RTE_SINGLE_SHOT_LOGGING_IS_ACTIVE)            \
    {                                                                \
        /* Check if there is enough space for the complete message */\
        if ((buf_idx + (size)) >= (uint32_t)(RTE_BUFFER_SIZE))       \
        {                                                            \
            RTE_STOP_MESSAGE_LOGGING();                              \
            return;        /* Exit the __rte_msg?() function. */     \
        }                                                            \
    }                                                                \
    RTE_LIMIT_INDEX(buf_idx)                                         \
    ptr->buf_index = buf_idx + (size);
#endif /* RTE_SINGLE_SHOT_ENABLED == 0 */

#endif  // RTEDBG_GENERIC_NON_REENTRANT_H

/*==== End of file ====*/
