/*
 * Copyright (c) 2024 Branko Premzel.
 *
 * SPDX-License-Identifier: MIT
 */

/**********************************************************************************
 * @file    rtedbg_timer_cyccnt.h
 * @author  Branko Premzel
 * @brief   Time measurement for data logging functions using CYCCNT CPU
 *          cycle counter integrated into the Cortex M CPU core. CYCCNT is
 *          available for devices with cores such as Cortex M3, M4, M7, M33.
 *
 * @note    The value of the processor cycle counter does not increment while
 *          the ARM Cortex M core is in sleep mode. The cycle counter is only
 *          incremented when the core is active and executing instructions.
 *          In sleep mode, the kernel shuts down and no commands are executed,
 *          so the cycle counter does not change. In sleep mode, the operation
 *          of peripheral devices is preserved. Therefore, when using sleep mode,
 *          it is better to use a peripheral timer for the timestamps.
 *
 * @version RTEdbg library <DEVELOPMENT BRANCH>
 **********************************************************************************/

#ifndef RTEDBG_TIMER_CYCCNT_H_
#define RTEDBG_TIMER_CYCCNT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "rtedbg.h"                      // Hardware and project specific definitions

#define RTE_TIMESTAMP_COUNTER_BITS  32U  // Number of timer counter bits available for the timestamp

#if RTE_USE_LONG_TIMESTAMP != 0
struct _tstamp64
{
    uint32_t l;    // Bottom and top part of the 64-bit timestamp
    uint32_t h;
} t_stamp;
#endif // RTE_USE_LONG_TIMESTAMP != 0


/***
 * @brief Initialize the peripheral for the time stamp counter and reset the counter.
 */

__STATIC_FORCEINLINE void rte_init_timestamp_counter(void)
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk; // Enable the DWT unit
    DWT->CYCCNT = 0;                                // Reset the cycle counter
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;            // and enable it

#if RTE_USE_LONG_TIMESTAMP != 0
    t_stamp.l = t_stamp.h = 0;                      // Reset the long timestamp
#endif
}


/***
 * @brief Get the current value of the timestamp counter.
 *
 * @return Current value of the timer counter.
 */

__STATIC_FORCEINLINE uint32_t rte_get_timestamp(void)
{
    return DWT->CYCCNT;
}


#if RTE_USE_LONG_TIMESTAMP != 0

/*********************************************************************************
 * @brief Writes a message with long time stamp to the buffer.
 *        The low bits of the timestamp are included in the message words with the
 *        format ID. Only the higher 32 bits are transmitted in the message's
 *        data part.
 *
 * @note  This function is not reentrant. Typically, calls should be made from a
 *        single section of the program that is periodically executed - e.g. from
 *        a timer interrupt routine.
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

#endif /* RTEDBG_TIMER_CYCCNT_H_ */

/*==== End of file ====*/

