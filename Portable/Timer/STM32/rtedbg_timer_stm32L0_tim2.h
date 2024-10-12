/*
 * Copyright (c) 2024 Branko Premzel.
 *
 * SPDX-License-Identifier: MIT
 */

/**********************************************************************************
 * @file    rtedbg_timer_stm32l0_tim2.h
 * @author  Branko Premzel
 * @brief   Time measurement for the data logging functions.
 *          TIM2 16-bit timer driver for the STM32L0 family.
 * @version RTEdbg library <DEVELOPMENT BRANCH>
 *
 * @note This file has to be adapted to the particular embedded application hardware.
 *       The current driver version assumes a fixed CPU clock frequency.
 *       This is a minimalist version of the driver made without using the STM32 HAL
 *       or low level HAL functions.
 **********************************************************************************/

#ifndef RTEDBG_TIMER_STM32L0_TIM2_H
#define RTEDBG_TIMER_STM32L0_TIM2_H

#ifdef __cplusplus
extern "C" {
#endif

#include "rtedbg.h"

#define RTE_TIMESTAMP_COUNTER_BITS  17U  // Number of timer counter bits available for the timestamp
    // Note: The value is (16 + 1) since the 16-bit counter is shifted left by 1
    // in the rte_get_timestamp() function.

#define RTE_TIMESTAMP_PRESCALER     8U
    // Divide the 16 MHz clock by (8 * 2) to get a 1 us timestamp resolution.
    // Additional division by 2 is due to implementation of data logging functions


#if RTE_USE_LONG_TIMESTAMP != 0
struct _tstamp64
{
    uint32_t l;    // Bottom part of the 64-bit timestamp
    uint32_t h;    // Top part of the 64-bit timestamp
} t_stamp;
#endif // RTE_USE_LONG_TIMESTAMP != 0


/***
 * @brief Initialize peripheral for the timestamp counter.
 */

__STATIC_FORCEINLINE void rte_init_timestamp_counter(void)
{
    // Enable the TIM2 clock
    SET_BIT(RCC->APB1ENR, RCC_APB1ENR_TIM2EN);
    (void)RCC->APB1ENR;    // Delay after enabling an RCC peripheral clock

    // Set TIM2 registers to their reset values.
    SET_BIT(RCC->APB1RSTR, RCC_APB1ENR_TIM2EN);
    CLEAR_BIT(RCC->APB1RSTR, RCC_APB1ENR_TIM2EN);

    // Set only the registers which must have values different from reset.
    TIM2->PSC = RTE_TIMESTAMP_PRESCALER - 1;
    TIM2->CR1 = TIM_CR1_CEN;        // Enable the timer counter

#if RTE_USE_LONG_TIMESTAMP != 0
    t_stamp.l = t_stamp.h = 0;      // Reset the long timestamp
#endif
}


/***
 * @brief Get the current value of the timestamp counter.
 *
 * @return Current value of the time counter.
 */

__STATIC_FORCEINLINE uint32_t rte_get_timestamp(void)
{
    return (uint32_t)(TIM2->CNT << 1U);
    // Shifted left to retain 16 bit timestamp resolution since the data logging
    // functions overwrite bit 0 with a 1. One bit of the timestamp counter would
    // be lost if the 16-bit value would not be shifted left.
    // The RTE_TIMESTAMP_COUNTER_BITS value must be 17 (16 + 1) because of this.
}


#if (RTE_FMT_ID_BITS) < (32U - 16U - 1U)
// Small values of number format ID bits not possible for current implementation of this driver
#error "RTE_FMT_ID_BITS must be min. 15 for a 16-bit timer TIM2 and current implementation of this driver."
#endif


#if RTE_USE_LONG_TIMESTAMP != 0

/*********************************************************************************
 * @brief  Writes a message with long timestamp to the buffer.
 *         The low bits of the timestamp are included in the message words with the
 *         format ID. Only the higher 32 bits are transmitted in the message's
 *         data part.
 *
 * @note    This function is not reentrant. Typically, calls should be made from a
 *          single section of the program that is periodically executed - e.g.
 *          from a timer interrupt routine.
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

#endif /* RTEDBG_TIMER_STM32L0_TIM2_H */

/*==== End of file ====*/
