/*
 * Copyright (c) 2024 Branko Premzel.
 *
 * SPDX-License-Identifier: MIT
 */

/*********************************************************************************
 * @file    rtedbg_inline.h
 * @author  Branko Premzel
 * @brief   Inline versions of the data logging functions __rte_msg0 ... __rte_msg4.
 *          Inline versions of functions are generally faster than called functions.
 *          Include this file in all source files that need to use inline logging.
 *
 *          If the source file that includes this header file contains interrupt
 *          programs that have such a high priority that no other interrupt program
 *          can interrupt them, a faster non-reentrant version of circular buffer
 *          reservation can be used for logging, allowing even faster (less intrusive)
 *          logging. This is done by adding the macro
 *              #define RTE_USE_LOCAL_CPU_DRIVER "rtedbg_generic_non_reentrant.h"
 *          before #include "rtedbg_inline.h".
 *
 * @note    For documentation on these inline functions, see the descriptions of the
 *          regular versions of the same functions in rtedbg.c.
 *
 * @version RTEdbg library <DEVELOPMENT BRANCH>
 ********************************************************************************/

#ifndef RTEDBG_INLINE_H
#define RTEDBG_INLINE_H

#define RTE_USE_INLINE_FUNCTIONS
#include "rtedbg_int.h"
#include RTE_TIMER_DRIVER           // Timestamp timer driver
#if !defined RTE_USE_LOCAL_CPU_DRIVER
#include RTE_CPU_DRIVER             // CPU driver as defined in the rtedbg_config.h
#else
#include RTE_USE_LOCAL_CPU_DRIVER   // A different CPU driver may be defined for the given files.
#endif


__STATIC_FORCEINLINE void __rte_msg0(const uint32_t fmt_id)
{
    rtedbg_t *p_rtedbg = &g_rtedbg;

#if RTE_DELAYED_TSTAMP_READ != 1
    uint32_t timestamp = (rte_get_timestamp() >> ((RTE_TIMESTAMP_SHIFT) - 1U)) & RTE_TIMESTAMP_MASK;
#endif

    if (RTE_MESSAGE_DISABLED(p_rtedbg->filter, fmt_id, 0U))
    {
        return;     // Discard the message if not enabled
    }

    uint32_t buf_index;
    RTE_RESERVE_SPACE(p_rtedbg, buf_index, 1U);                             //lint !e717

#if RTE_DELAYED_TSTAMP_READ != 0
    uint32_t timestamp = (rte_get_timestamp() >> ((RTE_TIMESTAMP_SHIFT) - 1U)) & RTE_TIMESTAMP_MASK;
#endif

    p_rtedbg->buffer[buf_index] = timestamp | 1U | (fmt_id << (32U - (uint32_t)(RTE_FMT_ID_BITS)));
}


__STATIC_FORCEINLINE void __rte_msg1(const uint32_t fmt_id, const rte_any32_t data1)
{
    rtedbg_t *p_rtedbg = &g_rtedbg;

#if RTE_DELAYED_TSTAMP_READ != 1
    uint32_t timestamp = (rte_get_timestamp() >> ((RTE_TIMESTAMP_SHIFT) - 1U)) & RTE_TIMESTAMP_MASK;
#endif

    if (RTE_MESSAGE_DISABLED(p_rtedbg->filter, fmt_id, 1U))
    {
        return;
    }

    uint32_t buf_index;
    RTE_RESERVE_SPACE(p_rtedbg, buf_index, 2U);                             //lint !e717

    rte_pack_data_t data;                                                   //lint !e9018
    data.w32.bits31 = fmt_id;
    uint32_t *data_packet = &g_rtedbg.buffer[buf_index];

    data.w32.data = RTE_PARAM(data1);
    data.w64 <<= 1U;
    *data_packet = data.w32.data;
    data_packet++;

#if RTE_DELAYED_TSTAMP_READ != 0
    uint32_t timestamp = (rte_get_timestamp() >> ((RTE_TIMESTAMP_SHIFT) - 1U)) & RTE_TIMESTAMP_MASK;
#endif
    *data_packet = timestamp | 1U | (data.w32.bits31 << (32U - (uint32_t)(RTE_FMT_ID_BITS)));
}


__STATIC_FORCEINLINE void __rte_msg2(const uint32_t fmt_id, const rte_any32_t data1, const rte_any32_t data2)
{
    rtedbg_t *p_rtedbg = &g_rtedbg;

#if RTE_DELAYED_TSTAMP_READ != 1
    uint32_t timestamp = (rte_get_timestamp() >> ((RTE_TIMESTAMP_SHIFT) - 1U)) & RTE_TIMESTAMP_MASK;
#endif

    if (RTE_MESSAGE_DISABLED(p_rtedbg->filter, fmt_id, 2U))
    {
        return;
    }

    uint32_t buf_index;
    RTE_RESERVE_SPACE(p_rtedbg, buf_index, 3U);                             //lint !e717

    rte_pack_data_t data;                                                   //lint !e9018
    data.w32.bits31 = fmt_id;

    data.w32.data = RTE_PARAM(data1);
    data.w64 <<= 1U;
    uint32_t *data_packet = &g_rtedbg.buffer[buf_index];
    *data_packet = data.w32.data;
    data_packet++;

    data.w32.data = RTE_PARAM(data2);
    data.w64 <<= 1U;
    *data_packet = data.w32.data;
    data_packet++;

#if RTE_DELAYED_TSTAMP_READ != 0
    uint32_t timestamp = (rte_get_timestamp() >> ((RTE_TIMESTAMP_SHIFT) - 1U)) & RTE_TIMESTAMP_MASK;
#endif
    // The FMT word with timestamp is written as the last value after other values are already in the buffer
    *data_packet = timestamp | 1U | (data.w32.bits31 << (32U - (uint32_t)(RTE_FMT_ID_BITS)));
}


__STATIC_FORCEINLINE void __rte_msg3(const uint32_t fmt_id, const rte_any32_t data1,
                                const rte_any32_t data2, const rte_any32_t data3)
{
    rtedbg_t *p_rtedbg = &g_rtedbg;

#if RTE_DELAYED_TSTAMP_READ != 1
    uint32_t timestamp = (rte_get_timestamp() >> ((RTE_TIMESTAMP_SHIFT) - 1U)) & RTE_TIMESTAMP_MASK;
#endif

    if (RTE_MESSAGE_DISABLED(p_rtedbg->filter, fmt_id, 3U))
    {
        return;
    }

    uint32_t buf_index;
    RTE_RESERVE_SPACE(p_rtedbg, buf_index, 4U);                             //lint !e717

    rte_pack_data_t data;                                                   //lint !e9018
    data.w32.bits31 = fmt_id;

    data.w32.data = RTE_PARAM(data1);
    data.w64 <<= 1U;    // The top bit of all data words are packed to the FMT word
    uint32_t *data_packet = &g_rtedbg.buffer[buf_index];
    *data_packet = data.w32.data;
    data_packet++;

    data.w32.data = RTE_PARAM(data2);
    data.w64 <<= 1U;
    *data_packet = data.w32.data;
    data_packet++;

    data.w32.data = RTE_PARAM(data3);
    data.w64 <<= 1U;
    *data_packet = data.w32.data;
    data_packet++;

#if RTE_DELAYED_TSTAMP_READ != 0
    uint32_t timestamp = (rte_get_timestamp() >> ((RTE_TIMESTAMP_SHIFT) - 1U)) & RTE_TIMESTAMP_MASK;
#endif

    // The FMT word with timestamp is written as the last value after other values are already in the buffer
    *data_packet = timestamp | 1U | (data.w32.bits31 << (32U - (uint32_t)(RTE_FMT_ID_BITS)));
}


__STATIC_FORCEINLINE void __rte_msg4(const uint32_t fmt_id, const rte_any32_t data1, const rte_any32_t data2,
                                const rte_any32_t data3, const rte_any32_t data4)
{
    rtedbg_t *p_rtedbg = &g_rtedbg;

#if RTE_DELAYED_TSTAMP_READ != 1
    uint32_t timestamp = (rte_get_timestamp() >> ((RTE_TIMESTAMP_SHIFT) - 1U)) & RTE_TIMESTAMP_MASK;
#endif

    if (RTE_MESSAGE_DISABLED(p_rtedbg->filter, fmt_id, 4U))
    {
        return;
    }

    uint32_t buf_index;
    RTE_RESERVE_SPACE(p_rtedbg, buf_index, 5U);                             //lint !e717

    rte_pack_data_t data;                                                   //lint !e9018
    data.w32.bits31 = fmt_id;

    // Save data to the buffer
    data.w32.data = RTE_PARAM(data1);
    data.w64 <<= 1U;
    uint32_t *data_packet = &g_rtedbg.buffer[buf_index];
    *data_packet = data.w32.data;
    data_packet++;

    data.w32.data = RTE_PARAM(data2);
    data.w64 <<= 1U;
    *data_packet = data.w32.data;
    data_packet++;

    data.w32.data = RTE_PARAM(data3);
    data.w64 <<= 1U;
    *data_packet = data.w32.data;
    data_packet++;

    data.w32.data = RTE_PARAM(data4);
    data.w64 <<= 1U;
    *data_packet = data.w32.data;
    data_packet++;

#if RTE_DELAYED_TSTAMP_READ != 0
    uint32_t timestamp = (rte_get_timestamp() >> ((RTE_TIMESTAMP_SHIFT) - 1U)) & RTE_TIMESTAMP_MASK;
#endif

    // The FMT word with timestamp is written as the last value after other values are already in the buffer
    *data_packet = timestamp | 1U | (data.w32.bits31 << (32U - (uint32_t)(RTE_FMT_ID_BITS)));
}

#endif /* RTEDBG_INLINE_H */

/*==== End of file ====*/
