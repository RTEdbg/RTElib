/*
 * Copyright (c) 2024 Branko Premzel.
 *
 * SPDX-License-Identifier: MIT
 */

/**********************************************************************************
 * @file    rtedbg_timer_test.h
 * @author  Branko Premzel
 * @brief   This file is for testing data logging features on new processors
 *          or with new compilers. It counts logged messages, not time.
 *          The timestamp value included in the messages is reproducible and
 *          does not depend on the specific hardware or compiler version/settings.
 *
 * @version RTEdbg library v1.00.03
 **********************************************************************************/

#ifndef RTEDBG_TEST_TIMER_H
#define RTEDBG_TEST_TIMER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "rtedbg.h"


#define RTE_TIMESTAMP_COUNTER_BITS  32U  // Number of timer counter bits available for the timestamp

#if !defined RTE_USE_INLINE_FUNCTIONS
#if RTE_USE_LONG_TIMESTAMP != 0
struct _tstamp64
{
    uint32_t l;   // Bottom part of the 64-bit timestamp
    uint32_t h;   // Top part of the 64-bit timestamp
} t_stamp;
#endif // RTE_USE_LONG_TIMESTAMP != 0

static uint32_t g_message_counter;


/***
 * @brief Initialize the emulated timestamp counter and reset it.
 *        It increases by 1 for each message recorded.
 */

__STATIC_FORCEINLINE void rte_init_timestamp_counter(void)
{
    g_message_counter = 0;

#if RTE_USE_LONG_TIMESTAMP != 0
    t_stamp.l = t_stamp.h = 0;
#endif
}
#endif  // !defined RTE_USE_INLINE_FUNCTIONS


/***
 * @brief Get the current value of the timestamp counter.
 *
 * @return Current value of the time counter.
 */

__STATIC_FORCEINLINE uint32_t rte_get_timestamp(void)
{
    g_message_counter += 2;
    return g_message_counter;
}


#if (RTE_TIMESTAMP_SHIFT) != 1
#error "The timestamp shift must be 1 for this driver!"
#endif


#if (RTE_USE_LONG_TIMESTAMP != 0) && (!defined RTE_USE_INLINE_FUNCTIONS)

/*********************************************************************************
 * @brief  Writes a message with a long timestamp to the buffer.
 *         The lower bits of the timestamp are included in the message words with the
 *         format ID. Only the upper 32 bits are transmitted in the message's
 *         data part.
 *********************************************************************************/

RTE_OPTIM_SIZE void rte_long_timestamp(void)
{
    uint32_t timestamp =
        (uint32_t)(rte_get_timestamp() << (32U - (RTE_TIMESTAMP_COUNTER_BITS)));

    if (t_stamp.l > timestamp)    // Has the counter rolled over?
    {
        t_stamp.h++;
    }

    t_stamp.l = timestamp;
    uint64_t timestamp_64 = (uint64_t)timestamp | ((uint64_t)t_stamp.h << 32U);
    uint32_t long_t_stamp = (uint32_t)(timestamp_64 >>
                                ((32U - ((uint32_t)(RTE_FMT_ID_BITS))) - 1U + (RTE_TIMESTAMP_SHIFT) +
                                 (32U - (RTE_TIMESTAMP_COUNTER_BITS))));
    RTE_MSG1(MSG1_LONG_TIMESTAMP, F_SYSTEM, long_t_stamp);
}

#endif // (RTE_USE_LONG_TIMESTAMP != 0) && (!defined RTE_USE_INLINE_FUNCTIONS)

#ifdef __cplusplus
}
#endif

#endif /* RTEDBG_TEST_TIMER_H */

/*==== End of file ====*/
