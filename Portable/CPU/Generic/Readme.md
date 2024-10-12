# Generic versions of the CPU core-specific circular buffer reservation drivers

The folder contains two driver versions:
* **rtedbg_generic_atomic.h** - Space reservation using the Atomic operations library (mutex instructions).
* **rtedbg_generic_irq_disable.h** - Space reservation using interrupt disable/enable. Should be used for all devices with a CPU core that does not support mutex instructions.

See also the following sections of the RTEdbg manual:
* 'Data logging in RTOS-based applications' if your code runs under an RTOS.
* 'Using the RTEdbg Library on Multi-Core Processors' if you plan to log to the shared *g_rtedbg* data structure on a multi-core device.

<br>

## rtedbg_generic_atomic.h
Circular buffer space reservation using the [Atomic operations library](https://en.cppreference.com/w/c/atomic). This driver is suitable for all devices with CPU cores supporting [Mutual Exclusion](https://en.wikipedia.org/wiki/Mutual_exclusion) (mutex instructions).

**Requirements:** The compiler must be C11 compatible, the header file "stdatomic.h" must be available for your compiler, and the functions *atomic_load_explicit() / atomic_compare_exchange_weak_explicit()* must be supported.

The following macro must be defined when logging data with the RTEdbg library on a multi-core processor and all cores write to a common circular data buffer located in shared RAM.
```
#define RTE_MULTICORE_SUPPORT 
```
**Note:** Check for the processor core-specific version, if available (e.g. *rtedbg_cortex_m_mutex.h* for the ARM Cortex-M3/M4/M7/M33/etc.). It is usually more optimized and faster than this generic one.

The incomplete list below includes processors for which this driver should work. It has only been tested on the ARM Cortex-M devices that support mutex instructions.
* ARM Cortex-M3, M4, M7, M33, M85
* Renesas RXv2, SuperH SH-4A
* Tensilica Xtensa LX6 (e.g. ESP32 with Tensilica core)
* PIC32MX, PIC32MZ
* RISC-V with Atomic extension (e.g. ESP32-C6)

<br>

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

#### Example for the ARM Cortex-M0/M0+/M23 families
Use this version for a bare-bone system without RTOS. Also use it in RTOS-based systems if tasks always run in privileged mode, such as the FreeRTOS ports without Memory Protection Unit (MPU) support.
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

#### Example for the Renesas RX family
The *#include "intrinsics.h"* header file must also be added to the *rtedbg_config.h* file.
```
#define RTE_ENTER_CRITICAL()                     \
    uint32_t irq_temp = __get_interrupt_state(); \
    __disable_interrupt();

#define RTE_EXIT_CRITICAL()                      \
    __set_interrupt_state(irq_temp);
```