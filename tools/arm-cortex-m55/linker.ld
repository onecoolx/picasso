MEMORY
{
  FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 2M
  RAM (xrw)   : ORIGIN = 0x20000000, LENGTH = 128K
}

ENTRY(main)
SECTIONS
{
  .text :
  {
    *(.vectors)
    *(.text*)
    *(.rodata*)
  } > FLASH
  .ARM.exidx :
  {
    . = ALIGN(4);
    __exidx_start = .;
    *(.ARM.exidx* .gnu.linkonce.armexidx.*)
    . = ALIGN(4);
    __exidx_end = .;
  } > FLASH
  .data :
  {
    . = ALIGN(4);
    __data_start__ = .;
    *(.data*)
    . = ALIGN(4);
    __data_end__ = .;
  } > RAM AT > FLASH
  .bss :
  {
    . = ALIGN(4);
    __bss_start__ = .;
    *(.bss*)
    *(COMMON)
    . = ALIGN(4);
    __bss_end__ = .;
  } > RAM
  PROVIDE(end = __bss_end__);

  .stack :
  {
    . = ALIGN(4);
    . = . + 8K;
    . = ALIGN(4);
    _estack = .;
  } > RAM
  .heap :
  {
    . = ALIGN(4);
    _sheap = .;
    . = . + 32K;
    . = ALIGN(4);
    _eheap = .;
  } > RAM
}
