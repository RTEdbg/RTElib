## RTEdbg data logging library revision history
### 1.00.00 - 2024-05-11
* Initial release
### 1.00.01 - 2024-10-12
* Driver file *rtedbg_cortex_m.h* replaced by more universal *rtedbg_generic_irq_disable.h*, suitable for all 32-bit CPU cores that do not support mutex instructions (not only for ARM Cortex M).
* New driver file *rtedbg_generic_atomic.h* suitable for all 32-bit CPU cores supporting mutex instructions.
* Improved *rtedbg_cortex_m_mutex.h* driver file.
* Shorter rte_init() function under certain compile time options.
* Fixed some compile warnings.
* Improved comments and fixed typos.