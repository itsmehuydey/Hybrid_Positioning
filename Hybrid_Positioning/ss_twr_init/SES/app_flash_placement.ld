/* SoftDevice + App memory settings */
FLASH_START = 0x26000;
FLASH_SIZE  = 0x5A000;

RAM_START   = 0x20002100;
RAM_SIZE    = 0xDDE0;

/* Memory regions */
MEMORY
{
    FLASH (rx) : ORIGIN = FLASH_START, LENGTH = FLASH_SIZE
    RAM   (rwx): ORIGIN = RAM_START,   LENGTH = RAM_SIZE
}

/* Sections */
SECTIONS
{
    .vectors :
    {
        KEEP(*(.vectors))
        __vectors_end__ = .;
    } > FLASH

    .init :
    {
        *(.init)
        *(.init_rodata)
    } > FLASH

    .text :
    {
        __text_start__ = .;
        *(.text*)
        *(.rodata*)
        __text_end__ = .;
    } > FLASH

    .ARM.exidx :
    {
        __exidx_start = .;
        *(.ARM.exidx*)
        __exidx_end = .;
    } > FLASH

    .ctors :
    {
        __ctors_start__ = .;
        KEEP(*(.ctors))
        __ctors_end__ = .;
    } > FLASH

    .dtors :
    {
        __dtors_start__ = .;
        KEEP(*(.dtors))
        __dtors_end__ = .;
    } > FLASH

    /* FAST SECTION */
    .fast :
    {
        __fast_load_start__ = LOADADDR(.fast);
        __fast_start__ = .;
        *(.fast*)
        __fast_end__ = .;
    } > RAM AT > FLASH

    /* DATA */
    .data :
    {
        __data_load_start__ = LOADADDR(.data);
        __data_start__ = .;
        *(.data*)
        __data_end__ = .;
    } > RAM AT > FLASH

    /* TDATA */
    .tdata :
    {
        __tdata_load_start__ = LOADADDR(.tdata);
        __tdata_start__ = .;
        *(.tdata*)
        __tdata_end__ = .;
    } > RAM AT > FLASH

    /* NRF_SECTIONS */
    .nrf_sections :
    {
        __start_nrf_sections = .;
        *(SORT(.nrf_sections*))
        __stop_nrf_sections = .;
    } > FLASH

    /* Observers (SoftDevice requires these) */
    .sdh_req_observers :
    {
        __start_sdh_req_observers = .;
        *(.sdh_req_observers*)
        __stop_sdh_req_observers = .;
    } > FLASH

    .sdh_state_observers :
    {
        __start_sdh_state_observers = .;
        *(.sdh_state_observers*)
        __stop_sdh_state_observers = .;
    } > FLASH

    .sdh_stack_observers :
    {
        __start_sdh_stack_observers = .;
        *(.sdh_stack_observers*)
        __stop_sdh_stack_observers = .;
    } > FLASH

    /* LOG + FS */
    .log_const_data :
    {
        __start_log_const_data = .;
        *(.log_const_data*)
        __stop_log_const_data = .;
    } > FLASH

    .log_dynamic_data :
    {
        __start_log_dynamic_data = .;
        *(.log_dynamic_data*)
        __stop_log_dynamic_data = .;
    } > RAM AT > FLASH

    .fs_data :
    {
        __start_fs_data = .;
        *(.fs_data*)
        __stop_fs_data = .;
    } > RAM AT > FLASH

    /* BSS */
    .bss :
    {
        __bss_start__ = .;
        *(.bss*)
        *(COMMON)
        __bss_end__ = .;
    } > RAM

    /* NON INIT */
    .non_init (NOLOAD):
    {
        *(.non_init*)
    } > RAM

    /* HEAP */
    .heap (NOLOAD):
    {
        __heap_start__ = .;
        *(.heap*)
        __heap_end__ = .;
    } > RAM

    /* STACK */
    .stack (NOLOAD):
    {
        __stack_start__ = .;
        *(.*stack*)
        __stack_end__ = .;
    } > RAM

    _estack = ORIGIN(RAM) + LENGTH(RAM);
}
