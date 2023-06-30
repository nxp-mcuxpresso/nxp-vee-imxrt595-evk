/*
 Copyright 2022 MicroEJ Corp. All rights reserved.
 Use of this source code is governed by a BSD-style license that can be found with this software.
*/

MEMORY
{
  /* Define each memory region */
  QSPI_FLASH (rx) : ORIGIN = 0x8000000, LENGTH = 0x800000 /* 8M bytes (alias Flash) */
  FRAMEBUFFER0 (rwx) : ORIGIN = 0x20000000, LENGTH = 0x50000 /* Framebuffer0 */
  FRAMEBUFFER1 (rwx) : ORIGIN = 0x20050000, LENGTH = 0x50000 /* Framebuffer1 */
  SRAM (rwx) : ORIGIN = 0x200A0000, LENGTH = 0x460000 /* Remaining RAM */
}

  /* Define a symbol for the top of each memory region */
  __base_QSPI_FLASH = 0x8000000  ; /* QSPI_FLASH */
  __base_Flash = 0x8000000 ; /* Flash */
  __top_QSPI_FLASH = 0x8000000 + 0x800000 ; /* 8M bytes */
  __top_Flash = 0x8000000 + 0x800000 ; /* 8M bytes */
  __base_SRAM = 0x20000000  ; /* SRAM */
  __base_RAM = 0x20000000 ; /* RAM */
  __top_SRAM = 0x20000000 + 0x300000 ; /* 3M bytes */
  __top_RAM = 0x20000000 + 0x300000 ; /* 3M bytes */
  __base_EXTRAM = 0x28000000 ; /* EXTRAM */
  __top_EXTRAM = 0x28800000 ;
