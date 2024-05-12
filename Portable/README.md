## Hardware drivers for the RTEdbg data logging library

This folder contains the timestamp timer and circular memory reservation (CPU core specific) driver files. 
If your CPU core or timer is not supported, modify the most appropriate driver file according to the instructions in the manual.

Add one timer driver file from the **"c:\RTEdbg\Library\Portable\Timer\"** folder and one CPU specific driver from the **"c:\RTEdbg\Library\Portable\CPU\"** folder in your project. Insert the driver file names in the fields marked with **##** in the **rtedbg_config.h** RTEdbg template configuration file.

**Note:** If your driver solves a common problem and could be useful to the wider community, open a pull request on GitHub and submit the driver file. Add it to the appropriate subfolder in the *Portable\Timer* or the *Portable\CPU* folder, or create a new one.