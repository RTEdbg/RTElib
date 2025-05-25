/*
 * Copyright (c) Branko Premzel.
 *
 * SPDX-License-Identifier: MIT
 */

/**********************************************************************************
 * @file    rtedbg_timer_esp32.h
 * @author  Branko Premzel
 * @brief   Time measurement for data logging functions using ESP32 64-bit timer.
 *          The esp_timer_get_time() function is used to return the time in
 *          microseconds since the start.
 *
 * @version RTEdbg library v1.01.00
 **********************************************************************************/

#ifndef RTEDBG_TIMER_ESP32_H
#define RTEDBG_TIMER_ESP32_H

#ifdef __cplusplus
extern "C" {
#endif

#include "esp_timer.h"
#include "rtedbg.h"

#define RTE_TIMESTAMP_COUNTER_BITS  32U  // Number of timer counter bits used for the short timestamp


/***
 * @brief Initialize the peripheral for the timestamp counter.
 *
 * @note  The timer is initialised shortly before the app_main()
 *        function is called. No further initialisation is required.
 */

RTE_OPTIM_SIZE __STATIC_FORCEINLINE void rte_init_timestamp_counter(void)
{
}


/***
 * @brief Get the current value of the timestamp timer counter.
 *
 * @return The low 32 bits of the current value of the timer counter.
 *         Returns current timer value in microseconds.
 */

RTE_OPTIM_SIZE static uint32_t rte_get_timestamp(void)
{
    return (uint32_t)esp_timer_get_time();
}


#if (RTE_USE_LONG_TIMESTAMP != 0) && (!defined RTE_USE_INLINE_FUNCTIONS)

/*********************************************************************************
 * @brief Writes a message with a long timestamp to the circular buffer.
 *        The low timestamp bits are included in the FMT word together with the
 *        format ID. The higher 32 bits are stored in the message's DATA word.
 *********************************************************************************/

RTE_OPTIM_SIZE void rte_long_timestamp(void)
{
    uint64_t timestamp_64 = (uint64_t)esp_timer_get_time();
    uint32_t long_t_stamp = (uint32_t)(timestamp_64 >>
                                       ((32U - ((uint32_t)(RTE_FMT_ID_BITS))) - 1U + (RTE_TIMESTAMP_SHIFT) +
                                        (32U - (RTE_TIMESTAMP_COUNTER_BITS))));
    RTE_MSG1(MSG1_LONG_TIMESTAMP, F_SYSTEM, long_t_stamp);
}

#endif // (RTE_USE_LONG_TIMESTAMP != 0) && (!defined RTE_USE_INLINE_FUNCTIONS)

#ifdef __cplusplus
}
#endif

#endif /* RTEDBG_TIMER_ESP32_H */

/*==== End of file ====*/
