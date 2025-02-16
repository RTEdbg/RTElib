/*
 * Copyright (c) Branko Premzel.
 *
 * SPDX-License-Identifier: MIT
 */

/**********************************************************************************
 * @file    rtedbg_zero_timer.h
 * @author  Branko Premzel
 * @brief   Driver for projects where timestamps are not required.
 *          Logging functions are faster and shorter, but when testing, we only have
 *          data in the order it was logged, without time information.
 *
 * @version RTEdbg library <DEVELOPMENT BRANCH>
 **********************************************************************************/

#ifndef RTEDBG_ZERO_TIMER_H
#define RTEDBG_ZERO_TIMER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "rtedbg.h"


#define RTE_TIMESTAMP_COUNTER_BITS  32U


#if RTE_USE_LONG_TIMESTAMP != 0
#error "Long timestamps must be disabled"
#endif

/***
 * @brief Empty function because the timestamps are not needed.
 */

__STATIC_FORCEINLINE void rte_init_timestamp_counter(void)
{
}


/***
 * @brief Get the current value of the timestamp counter.
 *
 * @return Returns null because the timestamps are not used.
 */

__STATIC_FORCEINLINE uint32_t rte_get_timestamp(void)
{
    return 0UL;
}



#ifdef __cplusplus
}
#endif

#endif /* RTEDBG_ZERO_TIMER_H */

/*==== End of file ====*/
