
MEMORY 
{
	FLASH(Rx) : ORIGIN = 0x08000000, LENGTH = 1M
	SRAM(RWX) : ORIGIN = 0x20000000, LENGTH = 128K

}


SECTIONS 
{
	.text : {
		*(.vector*)
		*(.text*)
		*(.rodata*)
		. = ALIGN(4);
		_etext = .;
	} > FLASH

	.data : {
		_sdata = .;
		*(.data*)
		. = ALIGN(4);
		_edata = .;
	} > SRAM AT > FLASH

	.bss : {
		_sbss = .;
		*(.bss*)
		. = ALIGN(4);
		_ebss = .;
	} > SRAM

}
