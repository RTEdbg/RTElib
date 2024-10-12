## Mandatory format definition files

This folder contains the format definition files.
The following two are mandatory and must be included in every project:
* **rte_main_fmt.h** - main format definition file - contains INCLUDE() directives for all project format definition files.
* **rte_system_fmt.h** - system message format definition file (do not modify this file).

See the **Demo** folder in the distribution ZIP file for examples of format definition files.

**Note:** The *INCLUDE(rte_system_fmt.h)* must be the first directive in the *rte_main_fmt.h*.