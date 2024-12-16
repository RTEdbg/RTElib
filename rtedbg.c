/*
 * Copyright (c) 2024 Branko Premzel.
 *
 * SPDX-License-Identifier: MIT
 */

/*********************************************************************************
 * @file    rtedbg.c
 * @author  Branko Premzel
 * @version RTEdbg library <DEVELOPMENT BRANCH>
 * @brief   Real-time reentrant data logging functions.
 *
 * Reentrant functions for logging binary data in the circular buffer. Data is
 * packed into subpackets containing one word with format ID and timestamp,
 * and zero to four DATA words. Long messages consist of multiple subpackets.
 * Only the last subpacket can be shorter than five words.
 * All subpackets belonging to the same recorded message have the same timestamp 
 * value. This allows the rtemsg decoding utility to determine which subpackets
 * belong to the same message and assemble/decode them correctly.
 ********************************************************************************/
                                                                            //lint -save -e970 -e506 -e778 -e831 -e835 -e845 -e904 -e9036 -e9125 -e9126 -e9139 -e9090 -e9141

#include "rtedbg_int.h"     // RTEdbg definitions and declarations
    RTE_OPTIMIZE_CODE       // Optional custom compilation settings for the entire rtedbg.c file

#if RTE_ENABLED != 0
// Project-specific includes - define them in the rtedbg_config.h
#include RTE_TIMER_DRIVER   // Timestamp timer driver
#include RTE_CPU_DRIVER     // Buffer space reservation macro specific to the CPU

rtedbg_t g_rtedbg RTE_DBG_RAM;  //!< Data structure with circular logging buffer

/********************************************************************************
 * @brief Initialize the data structures and clear the circular buffer if necessary.
 * The buffer is cleared after a power-on reset if the g_rtedbg structure has not
 * been initialized or on request with the function parameter "init_mode".
 *
 * @param initial_filter_value  Initial value for the message filter
 *
 * @param init_mode
 *        The following two values enable the post-mortem data logging mode:
 *        *) RTE_CONTINUE_LOGGING - Data logging structures are not completely
 *             cleared if they have already been initialized (data logging continues
 *             after e.g. restart or reset).
 *        *) RTE_RESTART_LOGGING - The data logging structures are completely
 *             cleared as after the first call of this function after power-on.
 *
 *        *) RTE_SINGLE_SHOT_LOGGING - Enable single shot logging mode
 *             If this mode was previously enabled, only the index is reset and the
 *             buffer is not cleared. This saves CPU time but has a downside.
 *             If one of the message(s) is not completely written due to a system
 *             error such as a bus error, then incorrect information would remain in
 *             the part of the buffer reserved for that particular message.
 *        *) RTE_SINGLE_SHOT_AND_ERASE_BUFFER - Enable single shot mode and erase the buffer
 *
 * @note  When the data logging mode is switched from post-mortem to single shot or vice
 *        versa by the firmware, the data logging buffer is completely cleared.
 *
 * @warning Multi-threaded systems: The message filter should not be enabled in any of
 *          the threads until this function has finished executing in the thread that
 *          called it. You should also make sure that all tasks have finished writing
 *          messages before calling this function.
 ********************************************************************************/

RTE_OPTIM_SIZE void rte_init(const uint32_t initial_filter_value, const uint32_t init_mode)
{
#if RTE_FILTER_OFF_ENABLED == 0
    UNUSED(initial_filter_value);
#endif

    uint32_t config_id = RTE_CONFIG_ID;                                     //lint !e9053
#if RTE_SINGLE_SHOT_ENABLED != 0
    if ((init_mode & RTE_SINGLE_SHOT_LOGGING_IS_ACTIVE) != 0U)
    {
        config_id |= RTE_SINGLE_SHOT_LOGGING_IS_ACTIVE;
        g_rtedbg.buf_index = 0U;
    }
#endif // RTE_SINGLE_SHOT_ENABLED != 0

    // If g_rtedbg has not yet been initialized, clear the header and circular buffer.
    if ((g_rtedbg.rte_cfg != config_id) || (init_mode >= RTE_RESTART_LOGGING))
    {
        /* Disable logging so that no task logs data during initialization. */
        g_rtedbg.filter = 0U;
        RTE_DATA_MEMORY_BARRIER();  // Make sure all CPU cores see the change.

        /* Initialize the g_rtedbg structure and buffer after a power-on reset or reboot. The
         * circular buffer must be set to 0xFFFFFFFF. This is the only value that does not
         * appear as normal data and enables the rtemsg data decoding software to detect that
         * part of the buffer has been reserved but not yet written to - e.g. because the task
         * logging data has been interrupted for a long time by higher priority tasks or services. */
#if defined RTE_USE_MEMSET
        memset(&g_rtedbg.buffer, RTE_ERASED_STATE & 0xFFu, sizeof(g_rtedbg.buffer));
#else
        int32_t count = (int32_t)((sizeof(g_rtedbg.buffer) / sizeof(uint32_t)) - 1U);
        do
        {
            *((volatile uint32_t *)(&g_rtedbg.buffer[(unsigned)count])) = RTE_ERASED_STATE;  //lint !e929
                // volatile used to prevent compiler from using the memset() function
                // memset() is slow in many embedded system library implementations (setting bytes instead of words)
            count--;
        }
        while (count >= 0);
#endif // defined RTE_USE_MEMSET

#if (RTE_FILTER_OFF_ENABLED != 0) && (RTE_MSG_FILTERING_ENABLED != 0)
        g_rtedbg.filter = initial_filter_value;
#if RTE_FIRMWARE_MAY_SET_FILTER == 1
        g_rtedbg.filter_copy = initial_filter_value;
#endif
#endif
        g_rtedbg.buf_index = 0U;
    }

    g_rtedbg.rte_cfg = config_id;
    g_rtedbg.buffer_size = (uint32_t)(RTE_BUFFER_SIZE) + 4U;

    // Set the timestamp frequency and initialize the timestamp timer
    g_rtedbg.timestamp_frequency = RTE_GET_TSTAMP_FREQUENCY();
    rte_init_timestamp_counter();

#if RTE_FILTER_OFF_ENABLED != 0
    rte_set_filter(initial_filter_value);
#else
#if RTE_MSG_FILTERING_ENABLED != 0
    g_rtedbg.filter = initial_filter_value;
#endif
#endif
}


/********************************************************************************
 * @brief Write only format ID and timestamp to circular buffer.
 *
 * @param fmt_id  Format ID number
 *        The format ID combines a format ID with the filter number.
 *        The lower bits contain extended data in the case of the EXT_MSG.. macro.
 *        See the macro RTE_PACK() to see how the two or three values are packed
 *        into one word (parameter fmt_id). The format ID is used to index the
 *        format string used for decoding (printing) the data on the host side.
 ********************************************************************************/

#if RTE_MINIMIZED_CODE_SIZE == 0

RTE_OPTIM_SPEED void __rte_msg0(const uint32_t fmt_id)
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


/********************************************************************************
 * @brief  Log a message containing one 32-bit word + timestamp/format ID
 *
 * @param  fmt_id  Format ID number - see the description of __rte_msg0().
 * @param  data1   Any 32-bit data
 ********************************************************************************/

RTE_OPTIM_SPEED void __rte_msg1(const uint32_t fmt_id, const rte_any32_t data1)
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


/********************************************************************************
 * @brief  Log a message containing two 32-bit words + timestamp/format ID
 *
 * @param  fmt_id        Format ID number - see the description of __rte_msg0().
 * @param  data1, data2  Any 32-bit data
 ********************************************************************************/

RTE_OPTIM_SPEED void __rte_msg2(const uint32_t fmt_id, const rte_any32_t data1, const rte_any32_t data2)
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


/********************************************************************************
 * @brief  Log a message containing three 32-bit words + timestamp/format ID
 *
 * @param  fmt_id           Format ID number - see the description of __rte_msg0().
 * @param  data1 ... data3  Any 32-bit data
 ********************************************************************************/

RTE_OPTIM_SPEED void __rte_msg3(const uint32_t fmt_id, const rte_any32_t data1,
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


/********************************************************************************
 * @brief  Log a message containing four 32-bit words + timestamp/format ID
 *
 * @param  fmt_id           Format ID number - see the description of __rte_msg0().
 * @param  data1 ... data4  Any 32-bit data
 ********************************************************************************/

RTE_OPTIM_SPEED void __rte_msg4(const uint32_t fmt_id, const rte_any32_t data1, const rte_any32_t data2,
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

#else // RTE_MINIMIZED_CODE_SIZE == 0
                                                                            //lint -e613
/***
 * @brief Minimized versions of the __rte_msg0() ... __rte_msg4() functions.
 *        See the __rte_msg0() ... __rte_msg4() function descriptions above.
 *        These functions are suitable for projects where only a small amount of program
 *        memory is available. Functions log data with calls to the __rte_msgn() function.
 *        If only few data logging functions are used for an individual project, the 
 *        non-minimized version of the functions can be shorter than the minimized one.
 */

RTE_OPTIM_SIZE void __rte_msg0(const uint32_t fmt_id)
{
    __rte_msgn(fmt_id, NULL, 0U);
}

RTE_OPTIM_SIZE void __rte_msg1(const uint32_t fmt_id, const rte_any32_t data1)
{
    uint32_t data = RTE_PARAM(data1);
    __rte_msgn(fmt_id, &data, sizeof(uint32_t));
}

RTE_OPTIM_SIZE void __rte_msg2(const uint32_t fmt_id, const rte_any32_t data1, const rte_any32_t data2)
{
    uint32_t data[2];
    data[0U] = RTE_PARAM(data1);
    data[1U] = RTE_PARAM(data2);
    __rte_msgn(fmt_id, &data[0U], sizeof(data));
}

RTE_OPTIM_SIZE void __rte_msg3(const uint32_t fmt_id, const rte_any32_t data1,
                               const rte_any32_t data2, const rte_any32_t data3)
{
    uint32_t data[3];
    data[0U] = RTE_PARAM(data1);
    data[1U] = RTE_PARAM(data2);
    data[2U] = RTE_PARAM(data3);
    __rte_msgn(fmt_id, &data[0U], sizeof(data));
}

RTE_OPTIM_SIZE void __rte_msg4(const uint32_t fmt_id, const rte_any32_t data1, const rte_any32_t data2,
                               const rte_any32_t data3, const rte_any32_t data4)
{
    uint32_t data[4];
    data[0U] = RTE_PARAM(data1);
    data[1U] = RTE_PARAM(data2);
    data[2U] = RTE_PARAM(data3);
    data[3U] = RTE_PARAM(data4);
    __rte_msgn(fmt_id, &data[0U], sizeof(data));
}

#endif // RTE_MINIMIZED_CODE_SIZE == 0


/********************************************************************************
 * @brief Log a message defined by address and size + timestamp/format ID.
 *
 * @param fmt_id       Format ID number - see the description of __rte_msg0().
 * @param address      Start address of data
 * @param data_length  Data length (bytes)
 *
 * @note  The address must be aligned if unaligned access is either not enabled or the
 *        CPU core does not support unaligned addressing. The data is copied as 32b words.
 *        If the message length is not divisible by 4 and the memory protection unit (MPU)
 *        is enabled, the additional bytes copied in the last word must not be outside the
 *        region accessible by the task calling this function. Use the __rte_msgx()
 *        function if such behaviour is a problem.
 ********************************************************************************/

RTE_OPTIM_LARGE void __rte_msgn(const uint32_t fmt_id,
                                volatile const void *const address, const uint32_t data_length)
{
    rtedbg_t *p_rtedbg = &g_rtedbg;
    volatile const uint32_t *addr = (volatile const uint32_t *)address;    //lint !e925 !e9079 !e9087
    uint32_t length = data_length;

#if RTE_DELAYED_TSTAMP_READ != 1
    uint32_t timestamp = (rte_get_timestamp() >> ((RTE_TIMESTAMP_SHIFT) - 1U)) & RTE_TIMESTAMP_MASK;
#endif

    if (RTE_MESSAGE_DISABLED(p_rtedbg->filter, fmt_id, (RTE_MINIMIZED_CODE_SIZE != 0) ? 0U : 4U))   //lint !e948 !e944
    {
        return;     // Discard the message if not enabled
    }

    if (length > RTE_MAX_MSG_SIZE)
    {
#if RTE_DISCARD_TOO_LONG_MESSAGES != 0
        return;
#else
        length = RTE_MAX_MSG_SIZE;
#endif
    }

    // Calculate the space required to copy the message to the circular buffer
    uint32_t no_words = ((length + 3U) / 4U) + ((length + 15U) / 16U);  // Add one FMT word for every four 32-bit DATA words
    if (no_words == 0U)
    {
        no_words = 1U;
    }

    uint32_t buf_index;
    RTE_RESERVE_SPACE(p_rtedbg, buf_index, no_words);                       //lint !e717

#if RTE_DELAYED_TSTAMP_READ != 0
    uint32_t timestamp = (rte_get_timestamp() >> ((RTE_TIMESTAMP_SHIFT) - 1U)) & RTE_TIMESTAMP_MASK;
#endif

#if RTE_MINIMIZED_CODE_SIZE != 0
    const unsigned fmt_mask = ((1U << ((uint32_t)(RTE_FMT_ID_BITS) - 4U)) - 1U) << 4U;
    timestamp |= ((fmt_id & fmt_mask) << (32U - (uint32_t)(RTE_FMT_ID_BITS))) | 1U;
#else
    const unsigned fmt_mask = ((1U << ((uint32_t)(RTE_FMT_ID_BITS) - 4U)) - 1U) << (32U - ((uint32_t)(RTE_FMT_ID_BITS) - 4U));
    timestamp |= ((fmt_id << (32U - ((uint32_t)(RTE_FMT_ID_BITS) - 4U))) & fmt_mask) | 1U;
#endif

    do
    {
        rte_pack_data_t data;                                               //lint !e9018
#if RTE_MINIMIZED_CODE_SIZE != 0
        data.w32.bits31 = 0xF0U;    // Add extended data
#else
        data.w32.bits31 = 0U;
#endif

        // Store data in the reserved space in the circular buffer
        uint32_t *data_packet = &g_rtedbg.buffer[buf_index];
        switch (no_words)
        {
            default:
                data.w32.data = *addr;
                addr++;
                data.w64 <<= 1U;
                *data_packet = data.w32.data;
                data_packet++;
                RTE_FALLTHROUGH; /* fallthrough */ //lint -fallthrough
            case 4U:
                data.w32.data = *addr;
                addr++;
                data.w64 <<= 1U;
                *data_packet = data.w32.data;
                data_packet++;
                RTE_FALLTHROUGH; /* fallthrough */ //lint -fallthrough
            case 3U:
                data.w32.data = *addr;
                addr++;
                data.w64 <<= 1U;
                *data_packet = data.w32.data;
                data_packet++;
                RTE_FALLTHROUGH; /* fallthrough */ //lint -fallthrough
            case 2U:
                data.w32.data = *addr;
                addr++;
                data.w64 <<= 1U;
                *data_packet = data.w32.data;
                data_packet++;
                RTE_FALLTHROUGH; /* fallthrough */ //lint -fallthrough
            case 1U:
                // Add the word with format ID and timestamp (and extended data bits in minimized mode)
#if RTE_MINIMIZED_CODE_SIZE != 0
                *data_packet = timestamp |
                              (((data.w32.bits31 & 0x0FU) | (fmt_id & (data.w32.bits31 >> 4U)))
                               << (32U - (uint32_t)(RTE_FMT_ID_BITS))
                              );
#else
                *data_packet = timestamp | (data.w32.bits31 << (32U - (uint32_t)(RTE_FMT_ID_BITS)));
#endif
                break;
        }

        buf_index += 5U;
        RTE_LIMIT_INDEX(buf_index)
        no_words -= 5U;
    }
    while ((int32_t)no_words > 0);
}


/********************************************************************************
 * @brief Log a message defined by address and size + timestamp/format ID.
 *        The maximum message length is either 255 bytes or (RTE_MAX_SUBPACKETS * 16) - 1
 *        bytes (whichever is less). The upper 8 bits of the last 32-bit data word
 *        written to the circular buffer define the message length (in bytes).
 *
 * @param fmt_id       Format ID number - see the description of __rte_msg0().
 * @param address      Start address of data
 * @param data_length  Data length (bytes)
 *
 * @note  This function allows you to log data whose length is not divisible by
 *        four and the length is unknown at compile time, or whose address does not
 *        need to be word aligned. The string type data also does not have to be
 *        null terminated. The data is read as bytes from the source address.
 ********************************************************************************/

RTE_OPTIM_LARGE void __rte_msgx(const uint32_t fmt_id,
                                volatile const void *const address, const uint32_t data_length)
{
    rtedbg_t *p_rtedbg = &g_rtedbg;
    uint32_t length = data_length;

#if RTE_DELAYED_TSTAMP_READ != 1
    uint32_t timestamp = (rte_get_timestamp() >> ((RTE_TIMESTAMP_SHIFT) - 1U)) & RTE_TIMESTAMP_MASK;
#endif

    if (RTE_MESSAGE_DISABLED(p_rtedbg->filter, fmt_id, 4U))
    {
        return;
    }

    // Calculate the space required to copy the message to the circular buffer
    if (length > (RTE_MAX_MSGX_SIZE - 1U))
    {
#if RTE_DISCARD_TOO_LONG_MESSAGES != 0
        return;
#else
        length = RTE_MAX_MSGX_SIZE - 1U;
#endif
    }

    uint32_t no_words = 2U + (length / 4U) + (length / 16U);
    uint32_t buf_index;
    RTE_RESERVE_SPACE(p_rtedbg, buf_index, no_words);                       //lint !e717

#if RTE_DELAYED_TSTAMP_READ != 0
    uint32_t timestamp = (rte_get_timestamp() >> ((RTE_TIMESTAMP_SHIFT) - 1U)) & RTE_TIMESTAMP_MASK;
#endif

    timestamp |= (fmt_id << (32U - ((uint32_t)(RTE_FMT_ID_BITS) - 4U))) | 1U;
    rte_pack_data_t data;                                                   //lint !e9018
    volatile const uint8_t *addr = (volatile const uint8_t *)address;      //lint !e925 !e9079
    int32_t remaining_bytes = (int32_t)length;

    do
    {
        data.w32.bits31 = 0U;
        no_words = 4U;
        uint32_t *data_packet = &g_rtedbg.buffer[buf_index];

        do
        {
            data.w32.data = 0U;
            switch (remaining_bytes)
            {
                default:
                    data.w32.data = ((uint32_t)addr[3U]) << 24U;
                    RTE_FALLTHROUGH; /* fallthrough */ //lint -fallthrough
                case 3:
                    data.w32.data |= ((uint32_t)addr[2U]) << 16U;
                    RTE_FALLTHROUGH; /* fallthrough */ //lint -fallthrough
                case 2:
                    data.w32.data |= ((uint32_t)addr[1U]) << 8U;
                    RTE_FALLTHROUGH; /* fallthrough */ //lint -fallthrough
                case 1:
                    data.w32.data |= (uint32_t)*addr;
                    break;
                case 0:
                    break;
            }

            addr += 4U;                                                     //lint !e9016
            remaining_bytes -= 4;
            if (remaining_bytes < 0)
            {
                // Add the length of data (top most byte of last DATA word)
                data.w32.data |= (length << 24U);
                no_words = 1U;      // Exit the loop
            }

            data.w64 <<= 1U;
            *data_packet = data.w32.data;
            data_packet++;
            no_words--;
        }
        while (no_words != 0U);

        // Add the 32-bit word with the format ID and timestamp
        *data_packet = timestamp | (data.w32.bits31 << (32U - (uint32_t)(RTE_FMT_ID_BITS)));
        buf_index += 5U;
        RTE_LIMIT_INDEX(buf_index)
    }
    while (remaining_bytes >= 0);
}


/********************************************************************************
 * @brief Write a string to the circular buffer. The maximum message length is
 *        limited by RTE_MAX_MSG_SIZE.
 *
 * @param fmt_id   Format ID number - see the description of __rte_msg0().
 * @param address  String start address
 ********************************************************************************/

RTE_OPTIM_SIZE void __rte_string(const uint32_t fmt_id, const char * const address)
{
    __rte_stringn(fmt_id, address, RTE_MAX_MSG_SIZE);
}


/********************************************************************************
 * @brief Write a string to the circular buffer. The maximum message length is limited
 *        by RTE_MAX_MSG_SIZE. If the length of the string (excluding the trailing
 *        null byte) is divisible by 4, the null byte at the end of the string is not
 *        saved to the buffer.
 *
 * @param fmt_id      Format ID number - see the description of __rte_msg0().
 * @param address     String start address
 * @param max_length  Maximum message length to be stored in the circular buffer
 ********************************************************************************/

RTE_OPTIM_SPEED void __rte_stringn(const uint32_t fmt_id,
                                   const char * const address, const uint32_t max_length)
{
    uint32_t length = max_length;
    if (RTE_MAX_MSG_SIZE < length)
    {
        length = RTE_MAX_MSG_SIZE;  // Limit the size to the maximum possible
    }

    const char *s = address;
    uint32_t len;
    for (len = 0U; (*s != '\0') && (len < length); len++)
    {
        s++;
    }

    __rte_msgn(fmt_id, address, len);
}


#if RTE_FIRMWARE_MAY_SET_FILTER != 0

/********************************************************************************
 * @brief Set the filter mask to enable/disable up to 32 message groups simultaneously.
 *        To completely disable data logging, set the filter value to zero. If simple
 *        re-enable is disabled with RTE_FILTER_OFF_ENABLED = 1, the filter must
 *        be re-enabled with a filter value of RTE_FORCE_ENABLE_ALL_FILTERS before proceeding.
 *        RTE_FORCE_ENABLE_ALL_FILTERS enables the logging of all messages by setting the
 *        filter variable to 0xFFFFFFFF. Once the filter is re-enabled (i.e., is no
 *        longer zero), any filter value can be set by calling this function.
 *        Filter number 0 (bit 31) can only be disabled by the filter parameter to 0.
 *
 * @param  filter  New message filter value
 ********************************************************************************/

RTE_OPTIM_SIZE void rte_set_filter(const uint32_t filter)
{
    uint32_t new_value = filter;
#if RTE_FILTER_OFF_ENABLED != 0
    RTE_DATA_MEMORY_BARRIER();          // Ensure visibility of changes across all CPU cores.
    if (g_rtedbg.filter == 0U)          // Are message filters completely disabled?
    {
        if (new_value != RTE_FORCE_ENABLE_ALL_FILTERS) // Enable even if completely disabled?
        {
            new_value = 0U;
        }
    }
#endif // RTE_FILTER_OFF_ENABLED != 0

    if (new_value != 0U)
    {
        // Filter #0 cannot be disabled unless all other filters are also disabled.
        new_value |= ~(uint32_t)RTE_FORCE_ENABLE_ALL_FILTERS;
        g_rtedbg.filter_copy = new_value;   // Store the last non-zero filter value
    }

    g_rtedbg.filter = new_value;
    RTE_DATA_MEMORY_BARRIER();          // Ensure visibility of changes across all CPU cores.
}


/********************************************************************************
 * @brief Restore the filter variable to the last non-zero value used before the
 *        filter variable was set to 0 by the firmware.
 ********************************************************************************/

RTE_OPTIM_SIZE void rte_restore_filter(void)
{
    g_rtedbg.filter = g_rtedbg.filter_copy;
    RTE_DATA_MEMORY_BARRIER();          // Ensure visibility of changes across all CPU cores.
}
#endif // RTE_FIRMWARE_MAY_SET_FILTER != 0


/********************************************************************************
 * @brief Retrieve the current value of the message filter.
 *
 * @return Current filter value (0 = filtering is completely disabled).
 ********************************************************************************/

RTE_OPTIM_SIZE uint32_t rte_get_filter(void)
{
    RTE_DATA_MEMORY_BARRIER();          // Ensure visibility of changes across all CPU cores.
    return g_rtedbg.filter;
}


/********************************************************************************
 * @brief Save the new timestamp frequency to the g_rtedbg structure and log
 *        the information in the circular data buffer. Call this function after
 *        changing the frequency of the timestamp timer or after changing the
 *        frequency of the processor (if the CPU clock is the timestamp timer clock).
 *
 * @param  new_frequency  New timestamp counter clock frequency value
 ********************************************************************************/

RTE_OPTIM_SIZE void rte_timestamp_frequency(const uint32_t new_frequency)
{
    g_rtedbg.timestamp_frequency = new_frequency;
    RTE_MSG1(MSG1_TSTAMP_FREQUENCY, F_SYSTEM, new_frequency)
}

#if ((RTE_TIMESTAMP_COUNTER_BITS) - (RTE_TIMESTAMP_SHIFT)) < (32U - 1U - (RTE_FMT_ID_BITS))
#error "The maximum RTE_TIMESTAMP_SHIFT value is limited to ensure the top logged timestamp bit flips."
#endif

#endif // RTE_ENABLED != 0
                                                                            //lint -restore
/*==== End of file ====*/
