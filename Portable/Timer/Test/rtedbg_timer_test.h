/*
 * Copyright (c) 2024 Branko Premzel.
 *
 * SPDX-License-Identifier: MIT
 */

/**********************************************************************************
 * @file    rtedbg_timer_test.h
 * @author  Branko Premzel
 * @brief   This file is only for testing data logging features on new processors
 *          or with new compilers. It counts logged messages, not time.
 *          The timestamp value included in the messages is therefore reproducible.
 *          It does not depend on the specific hardware used to test the data logging
 *          functions (and also the compiler version/settings).
 *
 * @version RTEdbg library <DEVELOPMENT BRANCH>
 **********************************************************************************/

#ifndef RTEDBG_TEST_TIMER_H
#define RTEDBG_TEST_TIMER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "rtedbg.h"


#define RTE_TIMESTAMP_COUNTER_BITS  32U  // Number of timer counter bits available for the timestamp


#if RTE_USE_LONG_TIMESTAMP != 0
struct _tstamp64
{
    uint32_t l;   // Bottom part of the 64-bit timestamp
    uint32_t h;   // Top part of the 64-bit timestamp
} t_stamp;
#endif // RTE_USE_LONG_TIMESTAMP != 0

static uint32_t g_message_counter;


/***
 * @brief Initialize the emulated peripheral for the timestamp and reset it.
 */

__STATIC_FORCEINLINE void rte_init_timestamp_counter(void)
{
    g_message_counter = 0;

#if RTE_USE_LONG_TIMESTAMP != 0
    t_stamp.l = t_stamp.h = 0;
#endif
}


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


#if RTE_USE_LONG_TIMESTAMP != 0

/*********************************************************************************
 * @brief  Writes a message with long timestamp to the buffer.
 *         The low bits of the timestamp are included in the message words with the
 *         format ID. Only the higher 32 bits are transmitted in the message's
 *         data part.
 *********************************************************************************/

RTE_OPTIM_SIZE void rte_long_timestamp(void)
{
    uint32_t timestamp =
        (uint32_t)(rte_get_timestamp() << (32U - (RTE_TIMESTAMP_COUNTER_BITS)));

    if (t_stamp.l > timestamp)    // Counter rolled over?
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

#endif // RTE_USE_LONG_TIMESTAMP != 0

#ifdef __cplusplus
}
#endif

#endif /* RTEDBG_TEST_TIMER_H */

/*==== End of file ====*/

