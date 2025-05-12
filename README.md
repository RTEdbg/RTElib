# Minimally intrusive binary data logging/tracing library

![LogoRTEdbg–small](https://github.com/RTEdbg/RTEdbg/assets/144953452/e123f541-1d05-44ca-a85e-34a7abeded22)

## Introduction

**This repository contains library of functions for minimally intrusive code instrumentation (data logging/tracing). It is part of the RTEdbg toolkit.**

See the **[RTEdbg main repository](https://github.com/RTEdbg/RTEdbg)** for the complete description and links to documentation and repositories. The complete toolkit can be downloaded from the main repository only. See the main benefits and features of the new toolkit in **[RTEdbg Presentation](https://github.com/RTEdbg/RTEdbg/releases/download/Documentation/RTEdbg.Presentation.pdf)**.

The data logging/tracing library is designed for maximum execution speed, low memory and stack requirements, and portability. Code instrumentation is minimally intrusive because raw binary data is logged along with an automatically assigned format definition ID (transparent to the programmer). Any data type or entire structures/buffers can be logged, including bit fields and packed structures. The stack requirements for this solution are minimal, virtually eliminating the possibility of stack overflows after code instrumentation. In addition, there is no need for format strings or data formatting/tagging functions in the embedded system. The small number of functions makes the solution easy to use and learn.

Data logging functions are reentrant and do not disable interrupts if the microcontroller supports the mutex instructions. The code is optimized for 32-bit devices and can be integrated into C and C++ code. Using this library avoids all the problems of printf-style debugging modes. All format strings are stored only on the host computer, where the decoding of the logged data is performed. Since the data is recorded in binary form, the number of functions is very small and it is possible to learn how to use them quickly. Format definitions are printf-style strings, so they are familiar to programmers.

All data logging functions are contained in the file *rtedbg.c*. See the comments for details and additional description in the document "*How the Data Logging is Implemented in the RTEdbg Library.pdf*" - see folder RTEdbg/Doc in the distribution ZIP file.

**The library is ready for use** on devices with an ARM Cortex-M core and other little-endian 32-bit devices. To port the RTEdbg library to a new microcontroller family, use one of the two generic drivers suitable for the CPU core (preferably the *rtedbg_generic_atomic.h* if the core supports atomic/mutex instructions) and add a timestamp timer driver suitable for your hardware.

## How to contribute or get help
Follow the [Contributing Guidelines](https://github.com/RTEdbg/RTEdbg/blob/master/docs/CONTRIBUTING.md) for bug reports and feature requests regarding the RTEdbg library. Follow the [Discussions Guidelines](https://docs.github.com/en/discussions/guides/best-practices-for-community-conversations-on-github) for general discusion about this repository.
Use [RTEdbg.freeforums.net](https://rtedbg.freeforums.net/) for general discussion about the RTEdbg toolkit.

When asking a support question, be clear and take the time to explain your problem properly. If your problem is not strictly related to this project, we recommend that you use [Stack Overflow](https://stackoverflow.com/), [r/Embedded](https://www.reddit.com/r/embedded/) or similar question-and-answer website instead. First, check if the [RTEdbg manual](https://github.com/RTEdbg/RTEdbg/releases/download/Documentation/RTEdbg.library.and.tools.manual.pdf) already contains an answer to your question or a solution to your problem.

## Repository Structure
This repository contains the RTEdbg data logging library. The complete data logging code is in the file 'rtedbg.c'. Copy it into your project. <br>
The subfolders contain the following:
* **Inc:** Header files that must be included in your project. Rename 'rtedbg_config_template.h' to 'rtedbg_config.h' and modify it to suit your needs. A 'rtedbg_config.h' from one of the demo projects can also be used as a starting point (if the particular demo project is similar to yours).
* **Portable:** Header files with the CPU-specific and timestamp timer-specific drivers. Add only one driver from each group to your project. Modify the driver to meet your project's requirements if an exact match is not found. <br>
See also the Readme.md files in the subfolders for additional documentation.
* **Fmt:** Format definition header files that must be added to your project.

See the **[RTEdbg main repository](https://github.com/RTEdbg/RTEdbg)** (&Rightarrow; *Repository Structure*) for links to all RTEdbg repositories that ar part of the RTEdbg toolkit.

The author put great emphasis on robustness and low complexity of the RTEdbg library and RTEmsg data decoding application code. Please report bugs or suggest improvements / corrections. Most of the toolkit code is contained in host tools such as the RTEmsg (binary data decoding tool) application that runs on the host computer. See the list of **[Repositories](https://github.com/RTEdbg/RTEdbg/blob/master/README.md#Repository-Structure)** that are part of the RTEdbg toolkit.

**Note:** The code is documented in Doxygen style. However, the project is not ready to automatically generate documentation with Doxygen.
