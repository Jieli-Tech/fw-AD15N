// *INDENT-OFF*
#include  "maskrom_stubs.ld"
//config

_BOOT_RAM_BEGIN = _MASK_MEM_BEGIN - 0x100 - 0x50 - 0x24;
MEMORY
{
	app_code(rx)        : ORIGIN = 0x100100,            LENGTH = 64M-0x100
    ram0(rw)            : ORIGIN = 0x0,                 LENGTH = _BOOT_RAM_BEGIN
    boot_ram(rw)        : ORIGIN = _BOOT_RAM_BEGIN,     LENGTH = 0x24
}

ENTRY(_start)

SECTIONS
{
    /* L1 memory sections */

    . = ORIGIN(boot_ram);
    .boot_data ALIGN(4):
    {
         *(.boot_info)
    } > boot_ram

    . = ORIGIN(ram0);
    .data ALIGN(4):
    {
        PROVIDE(data_buf_start = .);
        *(.data*)
        *(.*.data)
        *(.common)
        *(.ram_code)
		*(.volatile_ram_code)

        *(.*.text.cache.L1)
        *(.*.text.cache.L2)
        *(.*.text.cache.L3)
    } > ram0

	.debug_data ALIGN(4):
	{
        PROVIDE(debug_buf_start = .);
        *(.debug_bss)
        *(.debug_data)
    } > ram0


    .bss (NOLOAD) : SUBALIGN(4)
    {
        PROVIDE(bss_buf_start = .);
        . = ALIGN(32);
        *(.intr_stack)
        . = ALIGN(32);
		*(.stack_magic);
        . = ALIGN(32);
        *(.stack)
        . = ALIGN(32);
		*(.stack_magic0);
        . = ALIGN(32);
		_system_data_begin = .;
        *(.bss)
        *(.*.data.bss)
    	*(.non_volatile_ram)
        _system_data_end = .;
        *(.DAC_BUFFER)
        *(.AUDIO_ADC_BUFFER)
    } > ram0

    .effect_buf ALIGN(4):
    {
        PROVIDE(effect_buf_start = .);
        . = ALIGN(4);
        *(.sp_data)
        . = ALIGN(4);
        *(.rs_data)
    } > ram0

    .a_dec_buf ALIGN(32):
    {
        PROVIDE(a_buf_start = .);
        *(.a_data);
        PROVIDE(a_buf_end = .);
    } > ram0

    /* OVERLAY : */
    OVERLAY : AT(0x200000)
    {
        .d_midi_ctrl
        {
            PROVIDE(midi_ctrl_buf_start = .);
            *(.midi_ctrl_buf);
            PROVIDE(midi_ctrl_buf_end = .);
        }
        .d_midi
        {
            PROVIDE(midi_buf_start = .);
            *(.midi_buf);
            PROVIDE(midi_buf_end = .);
        }
        .d_ump3
        {
            PROVIDE(ump3_buf_start = .);
            *(.ump3_data);
            PROVIDE(ump3_buf_end = .);
        }
        .d_f1a
        {
            PROVIDE(f1a_1_buf_start = .);
            *(.f1a_1_buf);
            PROVIDE(f1a_1_buf_end = .);
           PROVIDE(f1a_2_buf_start = .);
            *(.f1a_2_buf);
           PROVIDE(f1a_2_buf_end = .);
        }
    } > ram0

    .heap_buf ALIGN(4):
    {
        PROVIDE(_free_start = .);
        . = LENGTH(ram0) + ORIGIN(ram0) - 1;
        PROVIDE(_free_end = .);
    } > ram0


    _ram_end = .;


    . = ORIGIN(app_code);
    .app_code ALIGN(32):
    {
        *startup.o(.text)
        . = ALIGN(32);
        *(*.f1a_code)
		_VERSION_BEGIN = .;
        KEEP(*(.version))
		_VERSION_END = .;
        *(.debug)
        *(.debug_const)
        *(.debug_code)
        *(.debug_string)
        /* *memset.o(.text .rodata*) */
        /* *memcmp.o(.text .rodata*) */
        *(*.text.const)
        *(*.text)
        *(.text)
        *(.app_root)
        *(.vm)
        . = ALIGN(32);
        _SPI_CODE_START = .;
        *(.spi_code)
        . = ALIGN(4);
        _SPI_CODE_END = .;
        *(.rodata*)
        *(.ins)

        . = ALIGN(4);
		loop_detect_handler_begin = .;
		KEEP(*(.loop_detect_region))
		loop_detect_handler_end = .;

        . = ALIGN(4);
        device_node_begin = .;
        PROVIDE(device_node_begin = .);
        KEEP(*(.device))
        _device_node_end = .;
        PROVIDE(device_node_end = .);

		vfs_ops_begin = .;
		KEEP(*(.vfs_operations))
		vfs_ops_end = .;

		. = ALIGN(4);
	    lp_target_begin = .;
	    PROVIDE(lp_target_begin = .);
	    KEEP(*(.lp_target))
	    lp_target_end = .;
	    PROVIDE(lp_target_end = .);


	    app_size = .;
        . = ALIGN(32);
        text_end = .;
    } >app_code


    bss_begin       = ADDR(.bss);
    bss_size        = SIZEOF(.bss);

    /*除堆栈外的bss区*/
    bss_size1       = _system_data_end - _system_data_begin;
    bss_begin1      = _system_data_begin;

	data_addr  = ADDR(.data) ;
	data_begin = text_end ;
	data_size =  SIZEOF(.data) + SIZEOF(.debug_data);

    text_size       = SIZEOF(.app_code);

    _sdk_text_addr = ADDR(.app_code);
    _sdk_text_size = text_size;
    _sdk_data_size = data_size;
}

