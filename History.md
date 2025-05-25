## RTEdbg data logging library revision history
### 1.00.00 - 2024-05-11
* Initial release

### 1.00.01 - 2024-10-12
* Driver file *rtedbg_cortex_m.h* replaced by more universal *rtedbg_generic_irq_disable.h*, suitable for all 32-bit CPU cores that do not support mutex instructions (not only for ARM Cortex M)
* New driver file *rtedbg_generic_atomic.h* suitable for all 32-bit CPU cores supporting mutex instructions
* Improved *rtedbg_cortex_m_mutex.h* driver file
* Shorter rte_init() function under certain compile time options
* Fixed some compile warnings
* Improved comments and fixed typos

### 1.00.02 - 2024-11-17
* Added the `rtedbg_generic_atomic_smp.h` buffer space reservation driver
* Fixed a bug in the STM32 TIM2 timestamp drivers
* Added issue and pull request templates
* Updated Readme files

### 1.00.03 - 2024-12-16
* Added `rtedbg_generic_non_reentrant.h` buffer reservation driver
* Added `rtedbg_zero_timer.h` timestamp timer driver for projects that do not require timestamps
* Added `rtedbg_inline.h' file with inline versions of data logging functions
* Improved compile time check
* Modified timestamp timer drivers to allow use of inline logging functions
* Updated Readme files

### 1.01.00 - 2025-02-16
* Added logging of messages on unlined addresses for processors that do not support unlined addressing in hardware.
* Added conditional translation option for an even smaller data logging code footprint.

### 1.01.00 - 2025-05-23
* Timestamp [timer driver](https://github.com/RTEdbg/RTElib/tree/master/Portable/Timer/ESP32/rtedbg_timer_esp32.h) for the ESP32 and [instructions](https://github.com/RTEdbg/RTElib/tree/master/Portable/Timer/ESP32/Readme.md) added.