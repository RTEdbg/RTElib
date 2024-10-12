/*
 * Copyright (c) 2024 Branko Premzel.
 *
 * SPDX-License-Identifier: MIT
 */

/*******************************************************************************
 * @file    rtedbg_config.h
 * @author  Branko Premzel
 * @version RTEdbg library <DEVELOPMENT BRANCH>
 * @brief   Configuration file with compile-time options for data logging.
 *
 * @note This file must be customized for the processor core used in the embedded
 *       application, the specifics of the application, and the compiler.
 *
 * Any compiler or application specific definitions related to message logging with
 * the RTEdbg library must be included in this file or in the files for which an
 * include is defined here. See also the rtedbg_config.h file examples in the Demo folder.
 *
 * This file contains example option settings for the GCC, IAR EWARM, and ARM v5 and
 * v6 compilers integrated into the Keil MDK. Remove unnecessary definitions and adjust
 * settings according to your project requirements.
 ******************************************************************************/

#ifndef RTEDBG_CONFIG_H
#define RTEDBG_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"   // Target hardware and project specific definitions
    /* Replace this include file with the appropriate one depending on the project. */


/*******************************************************************************
 * Define the CPU core and hardware specific include files for the particular project.
 * Include one timer driver file from the c:\RTEdbg\Library\Portable\Timer folder
 * or design a custom timer driver.
 * Add one CPU specific driver from c:\RTEdbg\Library\Portable\CPU folder to your
 * project or a custom CPU driver.
 * Insert the driver file names in the fields marked with ## below.
 * The RTE_GET_TSTAMP_FREQUENCY() and RTE_TIMESTAMP_SHIFT macros are mandatory.
 *******************************************************************************
 */
#define RTE_TIMER_DRIVER  "## Insert the timer driver file path here ##" // e.g. "rtedbg_timer_cyccnt.h"
#define RTE_GET_TSTAMP_FREQUENCY() GetCoreClock() // Current core frequency [Hz]
#define RTE_TIMESTAMP_SHIFT   1U
  /* Divide the time-counter value for the timestamp by 2^N (minimal value = 1). */

/* CPU core-specific functions for buffer space reservation. */
#define RTE_CPU_DRIVER  "## Insert the CPU driver file path here ##" // e.g. "rtedbg_cortex_m_mutex.h"


/*****************************************
 *   SETUP THE RTEdbg FUNCTIONALITY
 ****************************************/

#define RTE_ENABLED                       1
  /* 1 - data logging functionality enabled
   * 0 - data logging disabled (excluded from build)
   */

#define RTE_FMT_ID_BITS                 10
   /* Specifies the number of format ID bits used to log a message.
    * This value must be the same as the RTEmsg command line parameter -N=value.
    * The value should not be larger than necessary to have more bits for the timestamp.
    * Number of bits available for the timestamp = 32 - RTE_FMT_ID_BITS - 1
    */

#define RTE_BUFFER_SIZE                2048
  /* Number of 32-bit words in the circular data buffer.
   * If the value of RTE_BUFFER_SIZE is a power of two, the data logging of messages is
   * a bit faster and the logging functions are also smaller.
   */

#define RTE_MAX_SUBPACKETS              16
  /* The maximum number of data subpackets in a message determines the maximum message size.
   * Length of a subpacket is four 32-bit words plus one FMT word.
   * Maximum logged message length for __rte_msgn() in bytes = RTE_MAX_SUBPACKETS * 16.
   * This parameter must be equal to or greater than the largest data structure to be logged
   * with a call to RTE_MSGN() or RTE_MSGX() macros.
   * This parameter is a safeguard to prevent an erroneously long message from overwriting
   * the entire circular buffer (the minimum value of this parameter is 1). It also indirectly
   * limits the maximum logging time.
   */

#define RTE_MSG_FILTERING_ENABLED         1
  /* 1 - message filtering enabled (mandatory if the single shot logging is enabled)
   * 0 - message filtering disabled (all messages will be logged; single shot logging not possible)
   */

#define RTE_FILTER_OFF_ENABLED            1
  /* 1 - The firmware can completely disable logging with rte_set_filter(0).
   *     In this case, message logging can only be re-enabled by debugger or by calling
   *     rte_set_filter(RTE_FORCE_ENABLE_FILTER) or rte_init(RTE_FORCE_ENABLE_FILTER, ...)
   * 0 - The message filter cannot be completely disabled by the firmware.
   */

#define RTE_FIRMWARE_MAY_SET_FILTER       1
  /* 1 - The rte_set_filter(), rte_get_filter() and rte_restore_filter() functions
   *     are available to the programmer.
   * 0 - The g_rtedbg.filter variable can only be set through the debugger interface or
   *     by calling the rte_init() function.
   */

#define RTE_MINIMIZED_CODE_SIZE           0
  /* 1 - Enables reduced code size but slower execution and higher stack usage for the
   *     following functions: __rte_msg0() ... __rte_msg4().
   * 0 - Faster code execution, reduced stack requirement and larger code size if most
   *     of the data logging functions are used.
   */

#define RTE_DELAYED_TSTAMP_READ           1
  /* 1 - The timestamp counter is read just before its value is needed for data logging.
   *     The code size is typically slightly smaller and the stack usage also.
   *     This setting is generally recommended for simpler microcontrollers (like the
   *     Cortex M0+, M3 or M4 core), since in most cases the code will run faster, be
   *     smaller and use less stack.
   * 0 - The timestamp counter is read early during data logging function execution.
   *     This typically results in faster execution on microcontrollers with complex CPU
   *     cores such as the Cortex M7. This is especially true when using a timestamp
   *     counter with a slower connection to the CPU core or a slower clock for the timer
   *     peripheral bus.
   */

#define RTE_USE_LONG_TIMESTAMP            1
  /* 1 - Long timestamp used in the project - rte_long_timestamp() function enabled.
   * 0 - Long timestamp not used (only relative times between messages are logged).
   */

#define RTE_SINGLE_SHOT_ENABLED           0
  /* 1 - Both post-mortem and single shot logging available to the programmer.
   *     Which logging method will be used is defined by the rte_init() function parameter.
   *     Note: Add the following to the config file
   *        #define RTE_STOP_SINGLE_SHOT_AT_FIRST_TOO_LARGE_MSG
   *     if no messages should be stored in the circular buffer after the first too large
   *     message that does not fit in the buffer.
   * 0 - Only post-mortem logging available (faster execution and smaller code size).
   *     This is the preferred method when single-shot logging is not required for code
   *     testing/debugging.
   */

#define RTE_DISCARD_TOO_LONG_MESSAGES     1
  /* 1 - Discard messages that are longer than the maximum allowed.
   * 0 - Shorten messages that are too long to the maximum size.
   */

//#define RTE_DATA_MEMORY_BARRIER() __DMB()
   /* Enable the memory barrier if you want to enable data logging on more than one core of a
    * multi-core device based on the Cortex-M architecture. The memory barrier instruction is
    * generally not required for data logging on a single-core Cortex-M-based device. Refer to
    * the device documentation if you are using a different type of CPU core. See the description
    * in the RTEdbg manual section:
    *        "Using the RTEdbg library on multi-core processors"
    */


/*********************************************************************************
 *              COMPILER-SPECIFIC DEFINITIONS
 *
 * Conditional compilation is used here, allowing the same configuration file to
 * be used for all demo projects and all tested compilers. Remove the unnecessary
 * definitions if you'll use this file for your project.
 ********************************************************************************/

/*-----------------------------------------------------------------------------
 * Define memory space for g_rtedbg data structures for logging data. Use an empty
 * macro if the location of the RTEdbg data structure is either not important or is
 * specified by e.g. linker parameters.
 * Modify the definition below if it does not fit your compiler.
 *-----------------------------------------------------------------------------*/
#define RTE_DBG_RAM  __attribute__((section("RTEDBG"))) __attribute__((used))

/*---------------------------------------------------------------------------
 * Code optimization parameters for the functions in the rtedbg.c file.
 *
 * The RTEdbg library functions in the rtedbg.c file are short and the compiler
 * options do not have much effect on the generated code.
 * Optimize your data-logging code for speed or size as needed. Small code is
 * usually fast, but actual speed, code size, and stack usage should be checked
 * with the compiler used for your project. The most aggressive settings will
 * not always produce the fastest and/or smallest code for all functions.
 *
 * Compilation options can be set in the IDE or Makefile for the redbg.c file.
 * In this case, the macro definitions listed below are not required.
 * Additional macros can be used to set up the compilation of the RTEdbg library
 * separately from the rest of the project.
 * Compilation options can be set in this configuration file or on the level of
 * the entire rtedbg.c file with the RTE_OPTIMIZE_CODE macro or for individual
 * function types with the following macros:
 *   RTE_OPTIM_SIZE  - Optimization of infrequently used functions (e.g. rte_init)
 *   RTE_OPTIM_SPEED - Optimization of frequently used data logging functions
 *   RTE_OPTIM_LARGE - Optimization of large functions rte_msgn() and rte_msgx()
 * If an individual macro is not defined, then the RTE_OPTIMIZE_CODE macro setting
 * or the v setting of the rtedbg.c file in the project is used for the corresponding
 * function types if RTE_OPTIMIZE_CODE is not defined either.
 *
 * The RTE_USE_ANY_TYPE_UNION macro definitions should also be defined for compilers
 * that allow any 32-bit value to be converted to a union and where strict ISO C
 * compatibility is not required. See the RTEdbg library manual for a detailed
 * description.
 *
 * RTE_COMPILE_TIME_PARAMETER_CHECK - If the macro is not defined, compile-time
 * static_assert() checks are skipped. This macro can be used to disable static
 * checking if the compiler does not support it, or if it is disabled by compilation
 * options - e.g. for C99 compatibility.
 *
 * None of the code optimization definitions is necessary.
 * Have a look at the sample configuration files in the demo folders and use the
 * settings from them if they are suitable for the compiler you are using.
 *---------------------------------------------------------------------------*/

#ifdef __cplusplus
}
#endif

#endif /* RTEDBG_CONFIG_H */

/*==== End of file ====*/
