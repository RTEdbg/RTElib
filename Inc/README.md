This folder contains the following files:
* **rtedbg.h** - Definitions of data structures, functions and macros.
* **rtedbg_int.h** - Internal definitions for data logging functions.
* **rtedbg_config_template.h** - RTEdbg configuration file – see RTEdbg Configuration File. Add it to the project and rename to "rtedbg_config.h".

You must include all of these files in your project and add #include "rtedbg.h" to each file containing instrumented code (where logging macros are used).