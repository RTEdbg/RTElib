## Hardware drivers for the RTEdbg data logging library

This folder contains the Timestamp Timer and Circular Buffer Reservation (CPU core specific) driver files. 
If your CPU core or timer is not supported, modify the appropriate driver file according to the instructions in the RTEdbg manual - section *Library Source Code and Hardware-Dependent Drivers*. Also follow the instructions in the driver comments.

Add one timer driver file from the **"c:\RTEdbg\Library\Portable\Timer\"** folder and one CPU specific driver from the **"c:\RTEdbg\Library\Portable\CPU\"** folder in your project. Insert the driver file names in the fields marked with **##** in the **rtedbg_config.h** RTEdbg template configuration file.

The three CPU-specific drivers included in the RTEdbg library cover most of the 32-bit devices and generally do not require the programmer to write a new driver. 

The RTE_CPU_DRIVER macro must be defined in the *rtedbg_config.h* configuration file – for example	
```
      #define RTE_CPU_DRIVER "rtedbg_cortex_m_mutex.h"	
```      
It defines which CPU driver file is included during the project compilation.

The RTE_TIMER_DRIVER macro defines which timer driver will be used for the project – for example
```
      #define RTE_TIMER_DRIVER  "rtedbg_timer_cyccnt.h"
```
Some examples for the timestamp timer driver are included in the RTEdbg library. These drivers are device-specific. If there is no version available for your device, use the closest one as a starting point and modify it to suit your hardware. Follow the instructions in the RTEdbg manual - section 'Timestamp Drivers'.

**Note:** The *'rtedbg_cortex_m.h'* has been removed from the RTEdbg library. It has been replaced by *'rtedbg_generic_irq_disable.h'*. The new version is universal for all CPU cores that do not support mutex instructions.

**Contributing:** If your driver solves a common problem and could be useful to the wider community, open a pull request on GitHub and submit the driver file. Add it to the appropriate subfolder in the *Portable\Timer* or *Portable\CPU* folder, or create a new one. <br>
If you are in doubt about the correctness of your driver, contact the Field Application Engineer (FAE) of your processor manufacturer or supplier to verify the correctness of the implementation of the new timestamp timer or circular buffer reservation driver.
