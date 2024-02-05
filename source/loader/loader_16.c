__asm__(".code16gcc");

static void show_msg(const char* msg)
{
    char c;
    while((c = *msg++) != '\0') 
    {
        //使用内联汇编方式打印字符串
        //内链汇编示例：https://wiki.osdev.org/Inline_Assembly/Examples
        __asm__ __volatile__ (
            "mov $0xe,%%ah\n\t"
            "mov %[ch],%%al\n\t"
            "int $0x10"::[ch]"r"(c)
        );
	    

    }
}

//实模式
void loader_entry (void)
{
    show_msg("[wjos] - loading os...\n\r");
    for(;;)
    {
        
    }
}