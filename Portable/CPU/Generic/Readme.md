# Generic versions of the circular buffer reservation drivers

Each driver includes a RTE_RESERVE_SPACE() macro. This macro is used in data logging functions to reserve space in the circular buffer in a re-entrant manner. 

The *Portable/CPU/Generic* folder contains three driver versions:
* **rtedbg_generic_irq_disable.h** - Space reservation using interrupt disable/enable. Should be used for all devices with a CPU core that does not support mutex instructions. It can also be used for all other processors if short term interrupt disabling is not a problem.
* **rtedbg_generic_atomic.h** - Space reservation using the Atomic operations library (mutex instructions) for single-core devices.
* **rtedbg_generic_atomic_smp.h** - Space reservation using the Atomic operations library for multi-core devices (SMP - symetric multiprocessing).
* **rtedbg_generic_non_reentrant.h** - Space reservation without re-entry protection. Use when the programmer can ensure that the logging functions are called only from parts of the program that can never be executed simultaneously. Data logging is faster and program memory usage is reduced.

See also the following sections of the RTEdbg manual:
* 'Data logging in RTOS-based applications' if your code runs under an RTOS.
* 'Using the RTEdbg Library on Multi-Core Processors' if you plan to log to the shared *g_rtedbg* data structure on a multi-core device.

Define the memory barrier macro if you want to enable data logging on more than one core of a multi-core device. See below for an example for the Cortex-M architecture. <br>
&nbsp; &nbsp; &nbsp; `#define RTE_DATA_MEMORY_BARRIER() __DMB()` <br>
The memory barrier instruction is generally not required for data logging based on the RTEdbg library on a single-core ARM Cortex-M device.

## rtedbg_generic_atomic.h
Circular buffer space reservation using the [Atomic operations library](https://en.cppreference.com/w/c/atomic) for single-core devices. This driver is suitable for devices with CPU core supporting [Mutual Exclusion](https://en.wikipedia.org/wiki/Mutual_exclusion) (mutex instructions). The compiler must be at least C11 compatible or newer.

**Note:** Check for a processor core-specific version, if available (e.g. *rtedbg_cortex_m_mutex.h* for the ARM Cortex-M3/M4/M7/M33/etc.). It is usually more optimized and faster than this generic one.

The incomplete list below includes processors for which this driver should work. It has only been tested on the ARM Cortex-M devices that support mutex instructions.
* ARM Cortex-M3, M4, M7, M33, M85, ...
* Renesas RXv2, SuperH SH-4A
* Tensilica Xtensa LX6 (e.g. ESP32 with Tensilica core, dual-core devices if data logging runs on only one core).
* PIC32MX, PIC32MZ
* RISC-V with Atomic extension (e.g. ESP32-C6)

## rtedbg_generic_atomic_smp.h
Circular buffer space reservation using the [Atomic operations library](https://en.cppreference.com/w/c/atomic) for multi-core devices. The g_rtedbg data structure must be in a part of memory that is accessible to all cores. This driver is suitable for devices with CPU cores supporting [Mutual Exclusion](https://en.wikipedia.org/wiki/Mutual_exclusion) (mutex instructions). The driver is not suitable for heterogeneous multiprocessing, e.g. for devices with ARM Cortex-M7 and Cortex-M0+ cores. The compiler must be at least C11 compatible or newer. It should be suitable for heterogeneous multiprocessing if both cores support mutex instructions, e.g for devices with ARM Cortex-M7 and Cortex-M4.

## rtedbg_generic_irq_disable.h
This driver implements circular buffer space reservation using interrupt disable/enable. Use it for simple CPU cores that do not support mutex instructions. Note that interrupt enable / disable generally does not work as expected by a typical programmer in a unprivileged task running under RTOS control. See the RTEdbg manual (section *'Data logging in RTOS-based applications'*) for a complete description and additional instructions.

**The logging functions are not reentrant with respect to interrupts that cannot be disabled.** Disabling interrupts only blocks interrupts (from peripherals or internal like SVC, PendSV, SysTick). But it does not block NMI and exception handlers like Hard Fault.

The following macros must be added to the *rtedbg_config.h* project file:
```
#define RTE_ENTER_CRITICAL() // Store status indicating whether interrupts are enabled and disable them.
#define RTE_EXIT_CRITICAL()  // Enable interrupts again if they were previously enabled.
```
### Examples for a few CPU core families
One of the methods shown in the examples below is likely to be usable on other processors as well, since the interrupt disable/enable functions are supported by several compilers and their libraries.

#### ARM Cortex families (M0/M0+/M23, ...)
Use this version for a bare-bone system without RTOS. Also use it in RTOS-based systems if tasks always run in privileged mode, such as the FreeRTOS ports without MPU (Memory Protection Unit) support.
```
#define RTE_ENTER_CRITICAL()            \
    uint32_t irq_tmp = __get_PRIMASK(); \
    __disable_irq();

#define RTE_EXIT_CRITICAL()             \
    if (irq_tmp == 0U)                  \
    {                                   \
        __enable_irq();                 \
    }
```

#### Renesas 32-bit families (RX)
The *#include "intrinsics.h"* header file must also be added to the *rtedbg_config.h* file.
```
#define RTE_ENTER_CRITICAL()                     \
    uint32_t irq_temp = __get_interrupt_state(); \
    __disable_interrupt();

#define RTE_EXIT_CRITICAL()                      \
    __set_interrupt_state(irq_temp);
```
