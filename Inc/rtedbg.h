/*
 * Copyright (c) 2024 Branko Premzel.
 *
 * SPDX-License-Identifier: MIT
 */

/*******************************************************************************
 * @file    rtedbg.h
 * @author  Branko Premzel
 * @brief   RTEdbg library function declarations and macro definitions.
 * @note    This file must be included in all source files that use data logging.
 * @version RTEdbg library <DEVELOPMENT BRANCH>
 *******************************************************************************/

#ifndef RTEDBG_H
#define RTEDBG_H

#include <stdint.h>
#if defined RTE_USE_MEMSET
#include <string.h>
#endif
#include "rtedbg_config.h"  // Project-specific configuration file.
#include "rte_system_fmt.h" // System message filter and format ID definitions


#ifdef __cplusplus
extern "C" {
#endif

/****** rte_init() init_mode parameter definitions ******/
#define RTE_CONTINUE_LOGGING   0U
        /* Continue post-mortem data logging without clearing the circular buffer */
#if RTE_SINGLE_SHOT_ENABLED != 0
#define RTE_SINGLE_SHOT_LOGGING   1U
        /* Enable single-shot logging and reset circular buffer index */
#endif
#define RTE_RESTART_LOGGING  2U
        /* Enable post-mortem mode and clear the circular buffer */
#if RTE_SINGLE_SHOT_ENABLED != 0
#define RTE_SINGLE_SHOT_AND_ERASE_BUFFER  3U
        /* Enable single shot logging and clear the circular buffer */
#endif


/************************ COMMON DEFINITIONS ***************************/
        
#define RTE_ENABLE_ALL_FILTERS        0xFFFFFFFFU
        // Enable all 32 filter groups.
        
#define RTE_FORCE_ENABLE_ALL_FILTERS  0x7FFFFFFFU
        // Always enable all filters, even if they are completely disabled with filter = 0.
        
#define RTE_MAX_MSG_SIZE  ((uint32_t)(RTE_MAX_SUBPACKETS) * 4U * sizeof(uint32_t))
        // Maximum message size in bytes logged by a single function call.
        
// The RTE_MSGX() has an upper limit of 255 bytes or RTE_MAX_MSG_SIZE (whichever is smaller).
#if (RTE_MAX_SUBPACKETS) > (256U / (4U * 4U))
        // 256 = max. message length including data length (first byte)
#define RTE_MAX_MSGX_SIZE  256U
#else
#define RTE_MAX_MSGX_SIZE  ((uint32_t)(RTE_MAX_SUBPACKETS) * 16U)
#endif

#define RTE_ERASED_STATE  0xFFFFFFFFU    // Erased state of the circular buffer.


/************************************************************************************
 * Functions that "convert" the float or double value to uint32_t
 * Must be used if the compiler does not support the rte_any32_t union or it is disabled.
 ***********************************************************************************/

#ifndef __STATIC_FORCEINLINE
#define __STATIC_FORCEINLINE  static inline
#endif

__STATIC_FORCEINLINE uint32_t float_par(const float number)
{
    union
    {
        float    f;
        uint32_t u;
    } value;
    value.f = number;
    return value.u;
}

__STATIC_FORCEINLINE uint32_t double_par(const double number)
{
    union
    {
        float    f;
        uint32_t u;
    } value;
    value.f = (float)number;
    return value.u;
}


/* Extended message macros - they combine the format ID, filter number and 
 * extended data into one value before calling the data logging function.
 * They are defined here to enable compilation with RTE_ENABLED = 0
 */
#define RTE_EXT_MSG0_1(fmt, filter_no, ext_data)                                    \
    INTRTE_EXT_MSG0(fmt, filter_no, ext_data, 1U)
#define RTE_EXT_MSG0_2(fmt, filter_no, ext_data)                                    \
    INTRTE_EXT_MSG0(fmt, filter_no, ext_data, 3U)
#define RTE_EXT_MSG0_3(fmt, filter_no, ext_data)                                    \
    INTRTE_EXT_MSG0(fmt, filter_no, ext_data, 7U)
#define RTE_EXT_MSG0_4(fmt, filter_no, ext_data)                                    \
    INTRTE_EXT_MSG0(fmt, filter_no, ext_data, 15U)
#define RTE_EXT_MSG0_5(fmt, filter_no, ext_data)                                    \
    INTRTE_EXT_MSG0(fmt, filter_no, ext_data, 31U)
#define RTE_EXT_MSG0_6(fmt, filter_no, ext_data)                                    \
    INTRTE_EXT_MSG0(fmt, filter_no, ext_data, 63U)
#define RTE_EXT_MSG0_7(fmt, filter_no, ext_data)                                    \
    INTRTE_EXT_MSG0(fmt, filter_no, ext_data, 127U)
#define RTE_EXT_MSG0_8(fmt, filter_no, ext_data)                                    \
    INTRTE_EXT_MSG0(fmt, filter_no, ext_data, 255U)

#define RTE_EXT_MSG1_1(fmt, filter_no, data1, ext_data)                             \
    INTRTE_EXT_MSG1(fmt, filter_no, data1, ext_data, 3U)
#define RTE_EXT_MSG1_2(fmt, filter_no, data1, ext_data)                             \
    INTRTE_EXT_MSG1(fmt, filter_no, data1, ext_data, 7U)
#define RTE_EXT_MSG1_3(fmt, filter_no, data1, ext_data)                             \
    INTRTE_EXT_MSG1(fmt, filter_no, data1, ext_data, 15U)
#define RTE_EXT_MSG1_4(fmt, filter_no, data1, ext_data)                             \
    INTRTE_EXT_MSG1(fmt, filter_no, data1, ext_data, 31U)
#define RTE_EXT_MSG1_5(fmt, filter_no, data1, ext_data)                             \
    INTRTE_EXT_MSG1(fmt, filter_no, data1, ext_data, 63U)
#define RTE_EXT_MSG1_6(fmt, filter_no, data1, ext_data)                             \
    INTRTE_EXT_MSG1(fmt, filter_no, data1, ext_data, 127U)
#define RTE_EXT_MSG1_7(fmt, filter_no, data1, ext_data)                             \
    INTRTE_EXT_MSG1(fmt, filter_no, data1, ext_data, 255U)

#define RTE_EXT_MSG2_1(fmt, filter_no, data1, data2, ext_data)                      \
    INTRTE_EXT_MSG2(fmt, filter_no, data1, data2, ext_data, 7U)
#define RTE_EXT_MSG2_2(fmt, filter_no, data1, data2, ext_data)                      \
    INTRTE_EXT_MSG2(fmt, filter_no, data1, data2, ext_data, 15U)
#define RTE_EXT_MSG2_3(fmt, filter_no, data1, data2, ext_data)                      \
    INTRTE_EXT_MSG2(fmt, filter_no, data1, data2, ext_data, 31U)
#define RTE_EXT_MSG2_4(fmt, filter_no, data1, data2, ext_data)                      \
    INTRTE_EXT_MSG2(fmt, filter_no, data1, data2, ext_data, 63U)
#define RTE_EXT_MSG2_5(fmt, filter_no, data1, data2, ext_data)                      \
    INTRTE_EXT_MSG2(fmt, filter_no, data1, data2, ext_data, 127U)
#define RTE_EXT_MSG2_6(fmt, filter_no, data1, data2, ext_data)                      \
    INTRTE_EXT_MSG2(fmt, filter_no, data1, data2, ext_data, 255U)

#define RTE_EXT_MSG3_1(fmt, filter_no, data1, data2, data3, ext_data)               \
    INTRTE_EXT_MSG3(fmt, filter_no, data1, data2, data3, ext_data, 15U)
#define RTE_EXT_MSG3_2(fmt, filter_no, data1, data2, data3, ext_data)               \
    INTRTE_EXT_MSG3(fmt, filter_no, data1, data2, data3, ext_data, 31U)
#define RTE_EXT_MSG3_3(fmt, filter_no, data1, data2, data3, ext_data)               \
    INTRTE_EXT_MSG3(fmt, filter_no, data1, data2, data3, ext_data, 63U)
#define RTE_EXT_MSG3_4(fmt, filter_no, data1, data2, data3, ext_data)               \
    INTRTE_EXT_MSG3(fmt, filter_no, data1, data2, data3, ext_data, 127U)
#define RTE_EXT_MSG3_5(fmt, filter_no, data1, data2, data3, ext_data)               \
    INTRTE_EXT_MSG3(fmt, filter_no, data1, data2, data3, ext_data, 255U)

#define RTE_EXT_MSG4_1(fmt, filter_no, data1, data2, data3, data4, ext_data)        \
    INTRTE_EXT_MSG4(fmt, filter_no, data1, data2, data3, data4, ext_data, 31U)
#define RTE_EXT_MSG4_2(fmt, filter_no, data1, data2, data3, data4, ext_data)        \
    INTRTE_EXT_MSG4(fmt, filter_no, data1, data2, data3, data4, ext_data, 63U)
#define RTE_EXT_MSG4_3(fmt, filter_no, data1, data2, data3, data4, ext_data)        \
    INTRTE_EXT_MSG4(fmt, filter_no, data1, data2, data3, data4, ext_data, 127U)
#define RTE_EXT_MSG4_4(fmt, filter_no, data1, data2, data3, data4, ext_data)        \
    INTRTE_EXT_MSG4(fmt, filter_no, data1, data2, data3, data4, ext_data, 255U)

#ifdef _lint
#define static_assert(condition,message)
#endif  // _lint

#if RTE_ENABLED != 0
/************************************************************************************
 * MACROS to pass message filter and format ID to functions.
 ***********************************************************************************/

/* The following macros combine the format ID and filter number into a single value
 * that is the fmt_id parameter for data logging functions. */

#if !defined(static_assert) && !defined(__cplusplus)
#define static_assert(condition, message)  _Static_assert(condition, message)
#endif

/* @brief A macro to check the packed value of a parameter at compile time for data logging
 * functions. This is also a check if the parameters are constants known at compile time.
 *
 * @param filter_no Number of filter to be used for the message
 * @param fmt       Format ID (ID of format definition)
 * @param and_mask  AND mask to check if the fmt parameter contains redundant low bits
 */
#if !defined(RTE_COMPILE_TIME_PARAMETER_CHECK)
#define RTE_CHECK_PARAMETERS(filter_no, fmt, and_mask)
#else
#define RTE_CHECK_PARAMETERS(filter_no, fmt, and_mask)                                           \
    static_assert((filter_no) < 32U, "The filter value number must be between 0 and 31.");       \
    static_assert((fmt) < (1U << (uint32_t)(RTE_FMT_ID_BITS)), "Format ID value out of range."); \
    static_assert((fmt & and_mask) == 0,                                                         \
                  "Invalid format ID value - lowest bit(s) must be 0.");
/* Note:
 *    This error typically occurs when the programmer uses the wrong message name
 *    (format ID) - e.g. MSG0_... in macro RTE_EXT_MSG0_... or MSG1_ in RTE_MSGN_...
 */
#endif

#ifndef _lint
/* Pack the format ID and filter number in one word. */
#define RTE_PACK(filter, fmt, shift)                                                                \
    (((((RTE_MSG_FILTERING_ENABLED != 0) ? ((filter) & 0x1FU) : 0U) << (uint32_t)(RTE_FMT_ID_BITS)) \
      | ((fmt) & ((1U << (uint32_t)(RTE_FMT_ID_BITS)) - 1U))                                        \
      ) >> ((RTE_MINIMIZED_CODE_SIZE != 0) ? 0U : shift))

#define RTE_PACK_MSGX(filter, fmt)                                                                  \
    (((((RTE_MSG_FILTERING_ENABLED != 0) ? ((filter) & 0x1FU) : 0U) << (uint32_t)(RTE_FMT_ID_BITS)) \
      | ((fmt) & ((1U << (uint32_t)(RTE_FMT_ID_BITS)) - 1U))                                        \
      ) >> 4U)
#else   // _lint
#define RTE_PACK(filter, fmt, shift)  (fmt)
#define RTE_PACK_MSGX(filter, fmt)    (fmt)
#endif  // _lint

/***
 * THE FOLLOWING MACROS COMPOSE THE FORMAT ID, EXTENDED DATA, AND THE FILTER NUMBER
 * AND ALLOW THESE VALUES TO BE VERIFIED AT COMPILATION TIME.
 *
 * Parameters should be checked before the function is called, not at compile time.
 * Static assertions have been declared to catch common usage errors at compile time.
 * If the assertion were implemented in a function, it would increase the execution time.
 * The programmer would also lose the ability to correct the format definitions before
 * the firmware is tested or even used in the field.
 */
#define RTE_MSG0(fmt, filter_no)                                                    \
{                                                                                   \
    RTE_CHECK_PARAMETERS(filter_no, fmt, 0U);                                       \
    __rte_msg0(RTE_PACK(filter_no, fmt, 0U));                                       \
}

#define RTE_MSG1(fmt, filter_no, data1)                                             \
{                                                                                   \
    RTE_CHECK_PARAMETERS(filter_no, fmt, 1U);                                       \
    __rte_msg1(RTE_PACK(filter_no, fmt, 1U), (rte_any32_t)(data1));                 \
}

#define RTE_MSG2(fmt, filter_no, data1, data2)                                      \
{                                                                                   \
    RTE_CHECK_PARAMETERS(filter_no, fmt, 3U);                                       \
    __rte_msg2(RTE_PACK(filter_no, fmt, 2U), (rte_any32_t)(data1),                  \
               (rte_any32_t)(data2));                                               \
}

#define RTE_MSG3(fmt, filter_no, data1, data2, data3)                               \
{                                                                                   \
    RTE_CHECK_PARAMETERS(filter_no, fmt, 7U);                                       \
    __rte_msg3(RTE_PACK(filter_no, fmt, 3U), (rte_any32_t)(data1),                  \
               (rte_any32_t)(data2), (rte_any32_t)(data3));                         \
}

#define RTE_MSG4(fmt, filter_no, data1, data2, data3, data4)                        \
{                                                                                   \
    RTE_CHECK_PARAMETERS(filter_no, fmt, 15U);                                      \
    __rte_msg4(RTE_PACK(filter_no, fmt, 4U), (rte_any32_t)(data1),                  \
               (rte_any32_t)(data2), (rte_any32_t)(data3),                          \
               (rte_any32_t)(data4));                                               \
}

#define INTRTE_EXT_MSG0(fmt, filter_no, ext_data, mask)                             \
{                                                                                   \
    RTE_CHECK_PARAMETERS(filter_no, fmt, mask);                                     \
    __rte_msg0(RTE_PACK(filter_no, (fmt | ((ext_data) & mask)), 0U));               \
}

#define INTRTE_EXT_MSG1(fmt, filter_no, data1, ext_data, mask)                      \
{                                                                                   \
    RTE_CHECK_PARAMETERS(filter_no, fmt, mask);                                     \
    __rte_msg1(RTE_PACK(filter_no, fmt | (((ext_data) & (mask >> 1U)) << 1U), 1U),  \
               (rte_any32_t)(data1));                                               \
}

#define INTRTE_EXT_MSG2(fmt, filter_no, data1, data2, ext_data, mask)               \
{                                                                                   \
    RTE_CHECK_PARAMETERS(filter_no, fmt, mask);                                     \
    __rte_msg2(RTE_PACK(filter_no, fmt | (((ext_data) & (mask >> 2U)) << 2U), 2U),  \
               (rte_any32_t)(data1), (rte_any32_t)(data2));                         \
}

#define INTRTE_EXT_MSG3(fmt, filter_no, data1, data2, data3, ext_data, mask)        \
{                                                                                   \
    RTE_CHECK_PARAMETERS(filter_no, fmt, mask);                                     \
    __rte_msg3(RTE_PACK(filter_no, fmt | (((ext_data) & (mask >> 3U)) << 3U), 3U),  \
               (rte_any32_t)(data1), (rte_any32_t)(data2), (rte_any32_t)(data3));   \
}

#define INTRTE_EXT_MSG4(fmt, filter_no, data1, data2, data3, data4, ext_data, mask) \
{                                                                                   \
    RTE_CHECK_PARAMETERS(filter_no, fmt, mask);                                     \
    __rte_msg4(RTE_PACK(filter_no, fmt | (((ext_data) & (mask >> 4U)) << 4U), 4U),  \
               (rte_any32_t)(data1), (rte_any32_t)(data2),                          \
               (rte_any32_t)(data3), (rte_any32_t)(data4));                         \
}

#define RTE_MSGN(fmt, filter_no, address, size)                                     \
{                                                                                   \
    RTE_CHECK_PARAMETERS(filter_no, fmt, 15U);                                      \
    __rte_msgn(RTE_PACK(filter_no, fmt, 4U), address, size);                        \
}

#define RTE_MSGX(fmt, filter_no, address, size)                                     \
{                                                                                   \
    RTE_CHECK_PARAMETERS(filter_no, fmt, 15U);                                      \
    __rte_msgx(RTE_PACK_MSGX(filter_no, fmt), address, size);                       \
}

#define RTE_STRINGN(fmt, filter_no, address, size)                                  \
{                                                                                   \
    RTE_CHECK_PARAMETERS(filter_no, fmt, 15U);                                      \
    __rte_stringn(RTE_PACK(filter_no, fmt, 4U), address, size);                     \
}

#define RTE_STRING(fmt, filter_no, address)                                         \
{                                                                                   \
    RTE_CHECK_PARAMETERS(filter_no, fmt, 15U);                                      \
    __rte_string(RTE_PACK(filter_no, fmt, 4U), address);                            \
}

#if defined(_lint) && defined(RTE_USE_ANY_TYPE_UNION)
#undef RTE_USE_ANY_TYPE_UNION
#endif

#if defined(RTE_USE_ANY_TYPE_UNION) && !defined(__cplusplus)
/* Union defined to avoid built-in type conversions between function calls in RTE_MSG_().
 * Example: The default type conversion converts a float value to an unsigned integer
 *          when "#define rte_any32_t uint32_t" is used.
 * This is also an indirect check if the data type fits uint32_t.
 */
typedef union {
    uint32_t           _uint32;
    unsigned int       _unsigned_int;
    unsigned           _unsigned;
    int32_t            _int32;
    int                _int;
    short int          _short_int;
    short              _short;
    unsigned short int _u_short_int;
    uint16_t           _uint16;
    int16_t            _int16;
    uint8_t            _uint8;
    int8_t             _int8;
    char               _char;
    unsigned char      _uchar;
    float              _float;
    void *             _pvoid;
} rte_any32_t;

#else  /* if defined RTE_USE_ANY_TYPE_UNION */

/* If casting to union is not possible or allowed for your compiler, only casting to uint32_t
 * can be used. The use of a float value as a parameter of the macro RTE_MSG_() is no longer
 * possible, because the type conversion to uint32_t converts the float value to an integer (it
 * strips the decimal part).
 * Use the functions float_par() and double_par() to "convert" the parameters of the logging 
 * function to "uint32_t" and log the float values at no extra cost.
 */
#define rte_any32_t  uint32_t
#endif // defined RTE_USE_ANY_TYPE_UNION


/**************************
 *  FUNCTION DEFINITIONS
 **************************/

// The fmt_id parameter contains format ID, extended data, and filter number information.
#ifndef RTE_USE_INLINE_FUNCTIONS
void __rte_msg0(const uint32_t fmt_id);
void __rte_msg1(const uint32_t fmt_id, const rte_any32_t data1);
void __rte_msg2(const uint32_t fmt_id, const rte_any32_t data1, const rte_any32_t data2);
void __rte_msg3(const uint32_t fmt_id, const rte_any32_t data1, const rte_any32_t data2,
                const rte_any32_t data3);
void __rte_msg4(const uint32_t fmt_id, const rte_any32_t data1, const rte_any32_t data2,
                const rte_any32_t data3, const rte_any32_t data4);
#endif // RTE_USE_INLINE_FUNCTIONS
void __rte_msgn(const uint32_t fmt_id, volatile const void * const address, const uint32_t data_length);
void __rte_msgx(const uint32_t fmt_id, volatile const void * const address, const uint32_t data_length);
void __rte_string(const uint32_t fmt_id, const char * const address);
void __rte_stringn(const uint32_t fmt_id, const char * const address, const uint32_t max_length);

void rte_init(const uint32_t initial_filter_value, const uint32_t init_mode);
uint32_t rte_get_filter(void);

/********************************************************************************
 * @brief  Restart the time measurement - i.e. after the system reboots from sleep mode.
 *         The restart is indicated by the long timestamp value of 0xFFFFFFFFU.
 *         It tells the RTEmsg data decoding application that the following timestamps
 *         are not continuations of previous ones.
 ********************************************************************************/
#define RTE_RESTART_TIMING() RTE_MSG1(MSG1_LONG_TIMESTAMP, F_SYSTEM, 0xFFFFFFFFu)

#if RTE_USE_LONG_TIMESTAMP != 0
void rte_long_timestamp(void);
#else
#define rte_long_timestamp()
#endif

void rte_timestamp_frequency(const uint32_t new_frequency);

#if RTE_FIRMWARE_MAY_SET_FILTER != 0
void rte_set_filter(uint32_t filter);
void rte_restore_filter(void);
#else
#define rte_set_filter(filter)
#define rte_restore_filter()
#endif

#ifdef __cplusplus
}
#endif

#else // RTE_ENABLED != 0
#define rte_init(filter, mode)
#define RTE_MSG0(fmt_id, filter)
#define RTE_MSG1(fmt_id, filter, data)
#define RTE_MSG2(fmt_id, filter, data1, data2)
#define RTE_MSG3(fmt_id, filter, data1, data2, data3)
#define RTE_MSG4(fmt_id, filter, data1, data2, data3, data4)
#define INTRTE_EXT_MSG0(fmt_id, filter, ext_data, mask)
#define INTRTE_EXT_MSG1(fmt_id, filter, data1, ext_data, mask)
#define INTRTE_EXT_MSG2(fmt_id, filter, data1, data2, ext_data, mask)
#define INTRTE_EXT_MSG3(fmt_id, filter, data1, data2, data3, ext_data, mask)
#define INTRTE_EXT_MSG4(fmt_id, filter, data1, data2, data3, data4, ext_data, mask)
#define RTE_MSGN(fmt_id, filter, address, length)
#define RTE_MSGX(fmt_id, filter, address, length)
#define RTE_STRING(fmt_id, filter, address)
#define RTE_STRINGN(fmt_id, filter, address, length)
#define rte_long_timestamp()
#define rte_timestamp_frequency(new_frequency)
#define rte_get_filter() 0
#define rte_restore_filter()
#define rte_set_filter(filter)
#define RTE_RESTART_TIMING()
#endif // RTE_ENABLED != 0

#endif /* RTEDBG_H */

/*==== End of file ====*/
