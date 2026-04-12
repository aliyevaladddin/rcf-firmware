/*
 * [RCF:NOTICE][RCF:PROTECTED]
 * Linker Script for STM32F407VGTx — RCF v1.3 Mil-Spec (FROZEN).
 * Final Memory Map v1.3.
 */

ENTRY(Reset_Handler)

_estack = 0x20020000;    /* end of RAM */

_Min_Heap_Size = 0x200;
_Min_Stack_Size = 0x400;

MEMORY
{
  FLASH (rx)      : ORIGIN = 0x08000000, LENGTH = 1024K
  
  /* [CRITICAL] Sector 1 (16KB) reserved for MPK — Isolated from OTA */
  MPK (rx)        : ORIGIN = 0x08004000, LENGTH = 2K
  
  RAM (xrw)       : ORIGIN = 0x20000000, LENGTH = 128K
  CCMRAM (xrw)    : ORIGIN = 0x10000000, LENGTH = 64K
  OTP (r)         : ORIGIN = 0x1FFF7A00, LENGTH = 528
}

SECTIONS
{
  .isr_vector :
  {
    . = ALIGN(4);
    KEEP(*(.isr_vector))
    . = ALIGN(4);
  } >FLASH

  .text :
  {
    . = ALIGN(4);
    *(.text)
    *(.text*)
    . = ALIGN(4);
    _etext = .;
  } >FLASH

  .rodata :
  {
    . = ALIGN(4);
    *(.rodata)
    *(.rodata*)
    . = ALIGN(4);
  } >FLASH

  /* [RCF-MILSPEC] Isolated MPK Storage Section */
  .mpk_storage 0x08004000 :
  {
    KEEP(*(.mpk_storage))
    . = ALIGN(2048);
  } >MPK

  /* [RCF-MILSPEC] Zero-Wait Crypto Isolation */
  .ccmram :
  {
    . = ALIGN(4);
    _sccmram = .;
    *(.ccmram)
    *(.ccmram*)
    . = ALIGN(4);
    _eccmram = .;
  } >CCMRAM AT> FLASH

  _sidata = LOADADDR(.data);

  .data : 
  {
    . = ALIGN(4);
    _sdata = .;
    *(.data)
    *(.data*)
    . = ALIGN(4);
    _edata = .;
  } >RAM AT> FLASH

  .bss :
  {
    _sbss = .;
    __bss_start__ = _sbss;
    *(.bss)
    *(.bss*)
    *(COMMON)
    . = ALIGN(4);
    _ebss = .;
    __bss_end__ = _ebss;
  } >RAM

  /* [RCF-MILSPEC] OTP Hardware MEK mapping */
  .otp_mek 0x1FFF7A00 (NOLOAD) :
  {
    KEEP(*(.otp_mek))
  } >OTP

  /DISCARD/ :
  {
    libc.a ( * )
    libm.a ( * )
    libgcc.a ( * )
  }
}
