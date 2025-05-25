## Using RTEdbg with the ESP32

To enable logging with the RTEdbg toolkit in your ESP32 project, follow these steps:

1. **Include required source file and header files**

   In addition to the standard files required by RTEdbg (refer to section *2.5 Simple Demo Project* in the [RTEdbg manual](https://github.com/RTEdbg/RTEdbg/releases/download/Documentation/RTEdbg.library.and.tools.manual.pdf)), you must also include:
   - `rtedbg_generic_atomic.h` - a driver for circular buffer space allocation.
   - `rtedbg_timer_esp32.h` - a timestamp timer driver for ESP32.

2. **Initialize RTEdbg logging structure**

   Modify your `startup.c` file. Locate the function `esp_newlib_time_init()` and add a call to `rte_init()` *after* it. Alternatively, call `rte_init()` from your application code to initialize RTEdbg logging.

3. **Configure RTEdbg**

   Update or insert the following macros in your `rtedbg_config.h` file:

   ```c
   #define RTE_CPU_DRIVER           "rtedbg_generic_atomic.h"
   #define RTE_TIMER_DRIVER         "rtedbg_timer_esp32.h"
   #define RTE_TIMESTAMP_SHIFT        1U       // Right shift by 1 = divide by 2
   #define RTE_GET_TSTAMP_FREQUENCY() 1000000U // Timer frequency in Hz
   ```

4. **Ensure C11 compatibility**

   Make sure that your compiler supports and enables the C11 (or later) standard.

5. **Alternative circular buffer reservation driver**

   If `rtedbg_generic_atomic.h` fails to compile, replace it with `rtedbg_generic_irq_disable.h`. Also, in `rtedbg_config.h`, update the definitions as follows (replace the `#define RTE_CPU_DRIVER "rtedbg_generic_atomic.h"`):

   ```C
   #include "freertos/FreeRTOS.h"
   #include "freertos/portmacro.h"

   #define RTE_CPU_DRIVER           "rtedbg_generic_irq_disable.h"
   #define RTE_ENTER_CRITICAL()     uint32_t irq_tmp; portENTER_CRITICAL(&irq_tmp);
   #define RTE_EXIT_CRITICAL()      if (irq_tmp) { __enable_irq(); }
   ```

---

## Transferring Data to the Host

To decode and analyze the collected log data, it must be transferred from the embedded system to the host.

**Note:** The author does not have direct experience with ESP32 hardware. This document provides unverified guidance that we hope will be useful. We welcome contributions from users who successfully implement data transfer using the GDB server or serial port. If you develop a working solution, we would be happy to share your write-up or demo project and/or link to it from here. We also encourage any corrections or improvements to this document. 

The RTEgetData utility enables data transfer via a debug probe (GDB server protocol) or a serial port. It is currently only available on Windows.
See also the RTEdbg Manual - section *Save Embedded System Memory to a File Using an Eclipse IDE* or use an alternative method.

---

### Option 1: Transfer via ESP-IDF GDB server

1. **Edit OpenOCD Configuration**

   Open your device's OpenOCD config file (e.g., `esp32s3.cfg` or `esp32s3-builtin.cfg`). Add the following line **before** the device-specific `source` line:

   ```
   $_TARGETNAME configure -gdb-max-connections 3
   # Source the ESP32-S3 configuration file
   source [find target/esp32s3.cfg]
   ```

   > On dual-core devices, use `TARGETNAME_0` or `TARGETNAME_1` if needed.

   > Setting `-gdb-max-connections 3` allows **[RTEgetData](https://github.com/RTEdbg/RTEgetData)** to communicate with the embedded system in parallel with the IDE debugger.

3. **Start OpenOCD**

   You must run an OpenOCD server. For instructions, refer to the [ESP-IDF JTAG Debugging Guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/jtag-debugging/index.html).
   <br>The GDB server can also be started directly from the IDE when starting debugging.

4. **Known Limitation**

   When RTEgetData connects to the ESP-IDF GDB server over TCP/IP, the server typically halts embedded system code execution. No workaround has been identified yet.

---

### Option 2: Transfer via Serial Port

The **[RTEgetData](https://github.com/RTEdbg/RTEgetData)** utility also supports serial communication for log data transfer.

To use this method:
- Implement functionality in your firmware to send the `g_rtedbg` buffer over a serial channel.
- For an example, see the **[RTEcomLib_NUCLEO_C071RB_Demo](https://github.com/RTEdbg/RTEcomLib_NUCLEO_C071RB_Demo)** project, which demonstrates serial-based data transfer. This example also contains batch files for transferring data.

-----
## Write your own ESP32 timestamp timer driver

The `rtedbg_timer_esp32.h` timestamp driver provides the highest portability across ESP32 platforms. However, the `esp_timer_get_time()` function, which returns time in microseconds, is not the fastest and is limited to microsecond resolution. Finer resolution, down to CPU cycle time, and faster timestamp reading are achievable using direct timer counter read (inline) functions. These functions, while more performant, lack portability across ESP32 variants. 
For ESP-IDF v5.0 and later, the `esp_cpu_get_cycle_count()` function is available, returning the value of an internal counter that increments with each CPU clock cycle.

Here is an example of an inline function for the ESP32-S3/ESP32-C3 (RISC-V cores):
```C
static inline uint32_t get_cpu_cycle_count(void)
{
    uint32_t value;
    __asm__ volatile ("rdcycle %0" : "=r" (value));
    return value;
}
```