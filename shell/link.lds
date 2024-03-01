ENTRY(_start)
SECTIONS
{
    /* 为了方便调试，没有从0x80000000开始，以免与first_task冲突 */
	. = 0x81000000;
	.text : {
		*(*.text)
	}

	.rodata : {
		*(*.rodata)
	}

	.data : {
		*(*.data)
	}

	.bss : {
		PROVIDE(__bss_start__ = .);
		*(*.bss)
		PROVIDE(__bss_end__ = .);
	}
}