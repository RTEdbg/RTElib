#ifndef RTE_RTE_SYSTEM_FMT_H
#define RTE_RTE_SYSTEM_FMT_H
/* "rte_system_fmt.h" - Format definitions for the RTEdbg library functions */

/* The filter definition F_SYSTEM is mandatory. It must be defined even if filtering */
/* is not used. This filter is not reserved for system messages. Can be used for     */
/* all messages that should not be disabled.                                         */
// FILTER(F_SYSTEM, "System and other important messages")

/* The following two system format IDs are mandatory also */
// MSG1_LONG_TIMESTAMP   "0x%X"
/* Long timestamp = top 32-bits of the long timestamp */

// MSG1_TSTAMP_FREQUENCY "Timestamp frequency: %[32u](*1e-6)g MHz"
#endif
