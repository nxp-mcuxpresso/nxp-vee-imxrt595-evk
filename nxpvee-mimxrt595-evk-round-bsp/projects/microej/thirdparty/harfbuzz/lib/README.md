<!--
	Markdown
-->

# Harfbuzz code files

The CPP files are located in `harfbuzz/src` folder. 

Only two CPP files must be compiled:
- `harfbuzz.cc` is a wrapper over the other harfbuzz code files.
- `hb-alloc.c` provides the memory allocation functions over a BESTFIT_ALLOCATOR.

The include files are located in `harfbuzz/inc` folder.

# Howto Build harfbuzz.a Library

The harfbuzz CPP code can not be compiled by IAR Embedded Workbench. 
An alternative is to build a `harfbuzz.a` library file with the Arm GnuToolChain and to add this library to the IAR project.

Notes:

- The `harfbuzz.a` library is built by a makefile. 
- The Arm GnuToolChain must be added to the PATH environment variable.
- The build needs Freetype's include files. These files are located by default in `../../freetype/inc` but then can be provided to the makefile with the `FREETYPE_INCLUDE_DIR` variable.

`make harfbuzz.a FREETYPE_INCLUDE_DIR=<path_to_freetype_include_dir>`

An IAR Embedded Workbench project (`harfbuzz_lib.ewp`) is available to build the `harfbuzz.a` library from IAR. 
This project has a prebuild action command that executes the Arm GnuToolChain makefile.
This project can be included in the IAR workspace.

# GCC object files

The compiled `harfbuzz.o` refers to some LIBC and LIBGCC functions.
These functions will be missing if the `harfbuzz.a` library is used with another compiler.
Some LIBC and LIBGCC object files are automatically extracted from `libc.a` and `libgcc.a` and added to the `harfbuzz.a` archive file to bypass this issue.

---
_Copyright 2022 MicroEJ Corp. All rights reserved._
_Use of this source code is governed by a BSD-style license that can be found with this software._
