/******************************************************************************
 *
 * Default Linker Command file for the Texas Instruments TM4C1294NCPDT
 *
 * This is derived from revision 12770 of the TivaWare Library.
 *
 *****************************************************************************/

--retain=g_pfnVectors

#define FLASH_ADD     0x0000C000 /*0x00000000*/
#define FLASH_LEN     0x000F4000 /*0x00100000*/

MEMORY
{
    FLASH (RX) : origin = FLASH_ADD, length = FLASH_LEN
    SRAM (RWX) : origin = 0x20000000, length = 0x00040000
}

/* The following command line options are set as part of the CCS project.    */
/* If you are building using the command line, or for some reason want to    */
/* define them here, you can uncomment and modify these lines as needed.     */
/* If you are using CCS for building, it is probably better to make any such */
/* modifications in your CCS project and leave this file alone.              */
/*                                                                           */
/* --heap_size=0                                                             */
/* --stack_size=256                                                          */
/* --library=rtsv7M4_T_le_eabi.lib                                           */

/* Section allocation in memory */

SECTIONS
{
    .intvecs:   > FLASH_ADD
    .text   :   > FLASH
    .const  :   > FLASH
    .cinit  :   > FLASH
    .pinit  :   > FLASH
    .init_array : > FLASH

    .vtable :   > 0x20000000
    .data   :   > SRAM
    .bss    :   > SRAM
    .sysmem :   > SRAM
    .stack  :   > SRAM
}

__STACK_TOP = __stack + 512;
