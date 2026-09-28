MEMORY
{
    FLASH1 (rx) : ORIGIN = 0x08000000, LENGTH = 1024K
    RAM0  (rwx) : ORIGIN = 0x24000000, LENGTH = 512K
}

_estack = ORIGIN(RAM0) + LENGTH(RAM0);

SECTIONS
{
    .data :
    {
        *(.data*)
    } > RAM0 

    .text :
    {
        *(.text*)
    } > FLASH1

}
