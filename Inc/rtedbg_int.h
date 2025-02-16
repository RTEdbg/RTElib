/*
 * Copyright (c) Branko Premzel.
 *
 * SPDX-License-Identifier: MIT
 */

/*********************************************************************************
 * @file    rtedbg_int.h
 * @author  Branko Premzel
 * @brief   Internal definitions for the real-time data logging functions.
 * @note    This file should be included in your program code only if the firmware needs
 *          access to the g_rtedbg data structure to e.g. transfer data to the host.
 * @version RTEdbg library <DEVELOPMENT BRANCH>
 ********************************************************************************/

#ifndef RTEDBG_INT_H
#define RTEDBG_INT_H

#include "rtedbg.h"

// Test if the value is a power of 2 and between 2^2 and 2^31
// Result is FALSE if the value is not in the range or not a power of 2
#define RTE_IS_POWER_OF_2(n)                                                                      \
 (                                                                                                \
  ((n) == (1U << 31U)) || ((n) == (1U << 30U)) || ((n) == (1U << 29U)) || ((n) == (1U << 28U)) || \
  ((n) == (1U << 27U)) || ((n) == (1U << 26U)) || ((n) == (1U << 25U)) || ((n) == (1U << 24U)) || \
  ((n) == (1U << 23U)) || ((n) == (1U << 22U)) || ((n) == (1U << 21U)) || ((n) == (1U << 20U)) || \
  ((n) == (1U << 19U)) || ((n) == (1U << 18U)) || ((n) == (1U << 17U)) || ((n) == (1U << 16U)) || \
  ((n) == (1U << 15U)) || ((n) == (1U << 14U)) || ((n) == (1U << 13U)) || ((n) == (1U << 12U)) || \
  ((n) == (1U << 11U)) || ((n) == (1U << 10U)) || ((n) == (1U <<  9U)) || ((n) == (1U <<  8U)) || \
  ((n) == (1U <<  7U)) || ((n) == (1U <<  6U)) || ((n) == (1U <<  5U)) || ((n) == (1U <<  4U)) || \
  ((n) == (1U <<  3U)) || ((n) == (1U <<  2U))                                                    \
 )

#if RTE_IS_POWER_OF_2((RTE_BUFFER_SIZE))
// If the buffer size is a power of 2 then the index limiting is a bit faster and code smaller
#define RTE_LIMIT_INDEX(idx)  {idx &= ((uint32_t)(RTE_BUFFER_SIZE) - 1U);}
#else
#define RTE_LIMIT_INDEX(idx)  {if (idx >= (uint32_t)(RTE_BUFFER_SIZE)) {idx = 0U;}}
#endif

#define RTE_TIMESTAMP_MASK  (0xFFFFFFFFU >> (uint32_t)(RTE_FMT_ID_BITS))

#define RTE_HEADER_SIZE  (sizeof(g_rtedbg) - sizeof(g_rtedbg.buffer))

/***********************************************************************************
 * The configuration word defines the embedded system RTEdbg configuration.
 * Bit    0: 0 - post-mortem logging is active
 *           1 = single shot logging is enabled
 *        1: 1 = RTE_MSG_FILTERING_ENABLED, 0 - disabled
 *        2: 1 = RTE_FILTER_OFF_ENABLED, 0 - filter off not possible
 *        3: 1 = RTE_SINGLE_SHOT_ENABLED, 0 - only post mortem mode possible
 *        4: 1 = RTE_USE_LONG_TIMESTAMP, 0 - long timestamps disabled
 *  5 ..  7: reserved for future use
 *  8 .. 11: RTE_TIMESTAMP_SHIFT (0 = shift by 1, 1 = shift by 2, etc.)
 * 12 .. 14: RTE_FMT_ID_BITS     (offset 9 => values 0 .. 7 = 9 .. 16)
 *       15: reserved for future use
 * 16 .. 23: RTE_MAX_SUBPACKETS  (1 .. 256 - value 0 = 256)
 * 24 .. 30: RTE_HDR_SIZE (header size - number of 32b words)
 *       31: RTE_BUFF_SIZE_RTE_IS_POWER_OF_2 (1 = buffer size is power of 2, 0 - is not)
 ***********************************************************************************/
#define RTE_SINGLE_SHOT_LOGGING_IS_ACTIVE  1U   /* Use bit zero to indicate single shot mode = active */
#define RTE_BUFF_SIZE_RTE_IS_POWER_OF_2    ((RTE_IS_POWER_OF_2(RTE_BUFFER_SIZE)) ? 1U : 0U)
#if RTE_BUFF_SIZE_RTE_IS_POWER_OF_2
#define RTE_BUFF_SIZE_IS_POWER_OF_2   1U
#else
#define RTE_BUFF_SIZE_IS_POWER_OF_2   0U
#endif

#define RTE_CONFIG_ID                                                         \
    (                                                                         \
        /* Bit 0 reserved for: RTE_SINGLE_SHOT_LOGGING_IS_ACTIVE */           \
        ((uint32_t)RTE_MSG_FILTERING_ENABLED                 * (1U <<  1U)) + \
        ((uint32_t)RTE_FILTER_OFF_ENABLED                    * (1U <<  2U)) + \
        ((uint32_t)RTE_SINGLE_SHOT_ENABLED                   * (1U <<  3U)) + \
        ((uint32_t)RTE_USE_LONG_TIMESTAMP                    * (1U <<  4U)) + \
        ((((uint32_t)RTE_TIMESTAMP_SHIFT) - 1U)              * (1U <<  8U)) + \
        ((((uint32_t)RTE_FMT_ID_BITS) - 9U)                  * (1U << 12U)) + \
        ((((uint32_t)RTE_MAX_SUBPACKETS) & 0xFFU)            * (1U << 16U)) + \
        (((sizeof(g_rtedbg) - sizeof(g_rtedbg.buffer)) / 4U) * (1U << 24U)) + \
        (RTE_BUFF_SIZE_IS_POWER_OF_2                         * (1U << 31U))   \
    )

// Check if values fit into the predefined space
#if ((RTE_MAX_SUBPACKETS) > 256U) || ((RTE_MAX_SUBPACKETS) < 1U)
#error "The max. number of message sub-packets must be in the range from 1 to max. 256"
#endif

#if ((RTE_TIMESTAMP_SHIFT) > 16U) || ((RTE_TIMESTAMP_SHIFT) < 1U)
#error "The RTE_TIMESTAMP_SHIFT must have a value between min. 1 and 16"
#endif

#if ((RTE_FMT_ID_BITS) > 16U) || ((RTE_FMT_ID_BITS) < 9U)
#error "The RTE_FMT_ID_BITS must have a value between min. 9 and max. 16"
#endif

#if (RTE_MSG_FILTERING_ENABLED > 1) || (RTE_MSG_FILTERING_ENABLED < 0)
#error "The RTE_MSG_FILTERING_ENABLED must have a value of 0 or 1"
#endif

#if (RTE_FILTER_OFF_ENABLED > 1) || (RTE_FILTER_OFF_ENABLED < 0)
#error "The RTE_FILTER_OFF_ENABLED must have a value of 0 or 1"
#endif

#if (RTE_SINGLE_SHOT_ENABLED > 1) || (RTE_SINGLE_SHOT_ENABLED < 0)
#error "The RTE_SINGLE_SHOT_ENABLED must have a value of 0 or 1"
#endif

#if (RTE_USE_LONG_TIMESTAMP > 1) || (RTE_USE_LONG_TIMESTAMP < 0)
#error "The RTE_USE_LONG_TIMESTAMP must have a value of 0 or 1"
#endif


#if RTE_MSG_FILTERING_ENABLED != 0
#ifndef RTE_MESSAGE_DISABLED
// This macro version was optimized for CPU cores with a barrel shifter.
// Define the simplified version as an inline function or macro in the
// rtedbg_config.h if your CPU core does not have a barrel shifter.
#define RTE_MESSAGE_DISABLED(filter, fmt, shift_bits) \
    (((int32_t)((filter) << ((fmt) >> ((uint32_t)(RTE_FMT_ID_BITS) - (shift_bits))))) >= 0)
#endif  // RTE_MESSAGE_DISABLED
#else
#define RTE_MESSAGE_DISABLED(filter, fmt, shift_bits)  0U
#if RTE_SINGLE_SHOT_ENABLED != 0
#error "Single shot logging is only available when message filtering is enabled."
#endif
#endif

// Empty optimization definitions if the rtedbg.c file optimization will be set in
// the IDE (or makefile) or inherited from the complete project setup.
#if !defined RTE_OPTIMIZE_CODE
#define RTE_OPTIMIZE_CODE
#endif

#if !defined RTE_OPTIM_SIZE
#define RTE_OPTIM_SIZE
#endif

#if !defined RTE_OPTIM_SPEED
#define RTE_OPTIM_SPEED
#endif

#if !defined RTE_OPTIM_LARGE
#define RTE_OPTIM_LARGE
#endif


/*********************************************************************************
 * @brief Data structure for rtedbg library data logging.
 *
 * @note The firmware must initialize this structure with rte_init() before calling
 *       RTEdbg library data logging functions.
 *********************************************************************************/

typedef struct
{
    //---- g_rtedbg structure header start -----------------------------------
    volatile uint32_t buf_index;
        /*!< Index to the circular data logging buffer.
         *   It points to the location where the next message will be written.
         */
    volatile uint32_t filter;
        /*!< Enable/disable 32 message filters - each bit enables a group of messages.
         *   Bit 31 = filter #0, bit 30 = filter #1, ... bit 0 = filter #31.
         */
    uint32_t rte_cfg;                   /*!< RTEdbg configuration word. */
    uint32_t timestamp_frequency;       /*!< Timestamp timer frequency [Hz]. */
    uint32_t filter_copy;
        /*!< A copy of the filter variable. Contains the last non-zero value before
         *   message logging was stopped by the firmware. Host software or the firmware
         *   can restore the value after logging is paused, for example, to send a
         *   snapshot of the logging buffer to the host.
         * Note: The value is valid when the firmware manipulates the message filter,
         *       not when it is manipulated by a debug probe. As a rule, the debug
         *       probe only sets the filter variable and not filter_copy.
         */
    uint32_t buffer_size;
        /*!< The size of the circular data logging buffer  (RTE_BUFFER_SIZE + 4).
             It includes four additional words at the end of the buffer to speed up data logging.
         */
    //---- g_rtedbg structure header end -----------------------------------

    uint32_t buffer[(uint32_t)(RTE_BUFFER_SIZE) + 4U];
        /*!< Circular data logging buffer + 4 word trailer. */
        /* @note The additional four words make it possible to speed up the execution of
         * the code, since the check to see if the index is already at the end of the
         * buffer is performed only once per data subpacket.
         */
} rtedbg_t;

extern rtedbg_t g_rtedbg;   // Global data logging structure

/*********************************************************************************
 * @brief Union defined to move the top bit of 32-bit data words into an FMT word
 *        that combines bit 31 of the DATA words with the format ID and timestamp.
 *********************************************************************************/
typedef union
{
    uint64_t w64;
    struct
    {
        uint32_t data;      // A 32-bit data word that has to be saved to the circular buffer.
        uint32_t bits31;    // To shift bit 31 of each DATA word into this variable.
    } w32;
} rte_pack_data_t;


#if defined RTE_USE_ANY_TYPE_UNION
#define RTE_PARAM(par)  par._uint32
#else
#define RTE_PARAM(par)  par
#endif

#if defined RTE_STOP_SINGLE_SHOT_AT_FIRST_TOO_LARGE_MSG
#define RTE_STOP_MESSAGE_LOGGING()  ptr->filter = 0
#else
#define RTE_STOP_MESSAGE_LOGGING()
#endif

#ifndef RTE_DATA_MEMORY_BARRIER
#define RTE_DATA_MEMORY_BARRIER()
#endif

#if (RTE_TIMESTAMP_SHIFT) < 1U
#error "The timestamp shift value must be one or more."
#endif

#if (RTE_TIMESTAMP_SHIFT) > ((32U - 1U) - (RTE_FMT_ID_BITS))
#error "The value of RTE_TIMESTAMP_SHIFT is too large."
#endif

#if (RTE_BUFFER_SIZE) < (((RTE_MAX_SUBPACKETS) * 5U) * 4U)
#error "The buffer should be at least four times the size of the largest message."
#endif

#if RTE_MSG_FILTERING_ENABLED == 0
#if (RTE_FILTER_OFF_ENABLED != 0) || (RTE_FIRMWARE_MAY_SET_FILTER != 0)
#error "All filter-related definitions must be zero when message filtering is disabled."
#endif
#endif

/* Suppress fallthrough warnings (some compilers accept the 'fallthrough' comment) */
#if defined __has_attribute
#if __has_attribute(fallthrough)
#define RTE_FALLTHROUGH __attribute__((fallthrough))  // Compiler supports __attribute__
#else
#define RTE_FALLTHROUGH  // Compiler does not support __attribute__((fallthrough))
#endif
#else
#define RTE_FALLTHROUGH  // Compiler might not support preprocessor checks for attributes
#endif  // defined __has_attribute

#if (RTE_HANDLE_UNALIGNED_MEMORY_ACCESS == 1) && (RTE_DISCARD_MSGS_WITH_UNALIGNED_ADDRESS == 1)
#error "RTE_HANDLE_UNALIGNED_MEMORY_ACCESS enables special handling of unaligned accesses and prevents them from being discarded."
#endif

#endif /* RTEDBG_INT_H */

/*==== End of file ====*/
