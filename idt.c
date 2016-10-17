#include "idt.h"
#include "defs.h"
#include "x86.h"
static void idt_set_gate(uchar num, uint base, ushort sel, uchar flags);

struct idt_entry idt_entries[256];
struct idt_ptr the_idt_ptr;
interrupt_handler_t interrupt_handlers[256];
int limit;
int base;

void test1()
{
	cprintf("Hello\n");
}

void init_idt()
{
	memset((uchar*)&interrupt_handlers, 0, sizeof(interrupt_handler_t)*256);
	memset((uchar*)&idt_entries, 0, sizeof(struct idt_entry) * 256);
	limit = (sizeof(struct idt_entry) * 256);
	base  = (uint)&idt_entries;
	the_idt_ptr.limit = limit;
	the_idt_ptr.base = base;

	//init the pic
	outb(0x20,0x11);
	outb(0xA0,0x11);

	outb(0x21,0x20);
	outb(0xA1,0x28);

	outb(0x21,0x04);
	outb(0xA1,0x02);

	outb(0x21,0x01);
	outb(0xA1,0x01);

	outb(0x21,0x0);
	outb(0xA1,0x0);

	idt_set_gate(0 , (unsigned int)isr0 , 0x08, 0x8E);
	idt_set_gate(1 , (unsigned int)isr1 , 0x08, 0x8E);
	idt_set_gate(2 , (unsigned int)isr2 , 0x08, 0x8E);
	idt_set_gate(3 , (unsigned int)isr3 , 0x08, 0x8E);
	idt_set_gate(4 , (unsigned int)isr4 , 0x08, 0x8E);
	idt_set_gate(5 , (unsigned int)isr5 , 0x08, 0x8E);
	idt_set_gate(6 , (unsigned int)isr6 , 0x08, 0x8E);
	idt_set_gate(7 , (unsigned int)isr7 , 0x08, 0x8E);
	idt_set_gate(8 , (unsigned int)isr8 , 0x08, 0x8E);
	idt_set_gate(9 , (unsigned int)isr9 , 0x08, 0x8E);
	idt_set_gate(10, (unsigned int)isr10, 0x08, 0x8E);
	idt_set_gate(11, (unsigned int)isr11, 0x08, 0x8E);
	idt_set_gate(12, (unsigned int)isr12, 0x08, 0x8E);
	idt_set_gate(13, (unsigned int)isr13, 0x08, 0x8E);
	idt_set_gate(14, (unsigned int)isr14, 0x08, 0x8E);
	idt_set_gate(15, (unsigned int)isr15, 0x08, 0x8E);
	idt_set_gate(16, (unsigned int)isr16, 0x08, 0x8E);
	idt_set_gate(17, (unsigned int)isr17, 0x08, 0x8E);
	idt_set_gate(18, (unsigned int)isr18, 0x08, 0x8E);
	idt_set_gate(19, (unsigned int)isr19, 0x08, 0x8E);
	idt_set_gate(20, (unsigned int)isr20, 0x08, 0x8E);
	idt_set_gate(21, (unsigned int)isr21, 0x08, 0x8E);
	idt_set_gate(22, (unsigned int)isr22, 0x08, 0x8E);
	idt_set_gate(23, (unsigned int)isr23, 0x08, 0x8E);
	idt_set_gate(24, (unsigned int)isr24, 0x08, 0x8E);
	idt_set_gate(25, (unsigned int)isr25, 0x08, 0x8E);
	idt_set_gate(26, (unsigned int)isr26, 0x08, 0x8E);
	idt_set_gate(27, (unsigned int)isr27, 0x08, 0x8E);
	idt_set_gate(28, (unsigned int)isr28, 0x08, 0x8E);
	idt_set_gate(29, (unsigned int)isr29, 0x08, 0x8E);
	idt_set_gate(30, (unsigned int)isr30, 0x08, 0x8E);
	idt_set_gate(31, (unsigned int)isr31, 0x08, 0x8E);

	idt_set_gate(128,(unsigned int)isr128,0x08, 0x8E);

    idt_set_gate(32, (unsigned int)irq0, 0x08, 0x8E);
	idt_set_gate(33, (unsigned int)irq1 , 0x08, 0x8E);
	idt_set_gate(34, (unsigned int)irq2 , 0x08, 0x8E);
	idt_set_gate(35, (unsigned int)irq3 , 0x08, 0x8E);
	idt_set_gate(36, (unsigned int)irq4 , 0x08, 0x8E);
	idt_set_gate(37, (unsigned int)irq5 , 0x08, 0x8E);
	idt_set_gate(38, (unsigned int)irq6 , 0x08, 0x8E);
	idt_set_gate(39, (unsigned int)irq7 , 0x08, 0x8E);
	idt_set_gate(40, (unsigned int)irq8 , 0x08, 0x8E);
	idt_set_gate(41, (unsigned int)irq9 , 0x08, 0x8E);
	idt_set_gate(42, (unsigned int)irq10, 0x08, 0x8E);
	idt_set_gate(43, (unsigned int)irq11, 0x08, 0x8E);
	idt_set_gate(44, (unsigned int)irq12, 0x08, 0x8E);
	idt_set_gate(45, (unsigned int)irq13, 0x08, 0x8E);
	idt_set_gate(46, (unsigned int)irq14, 0x08, 0x8E);
	idt_set_gate(47, (unsigned int)irq15, 0x08, 0x8E);

	lidt(base, limit);
	return;
}
//just for a test...
void print_test()
{
	//cprintf("addr : %x\n", base);
	//cprintf("size : %x\n", limit);
	for(int i = 0;i < 500000000;i++)
		;
}

static void idt_set_gate(uchar num, uint base, ushort sel, uchar flags)
{
	idt_entries[num].base_low = base & 0xFFFF;
	idt_entries[num].base_high = (base >> 16) & 0xFFFF;

	idt_entries[num].sel = sel;
	idt_entries[num].always0 = 0;
	idt_entries[num].flags = flags;
}

void isr_handler(struct registers *regs)
{
	if(interrupt_handlers[regs->int_no])
	{
		interrupt_handlers[regs->int_no](regs);
	}
    else
    {
		cprintf("no int is : %d\n", regs->int_no);
	}
}

void irq_handler(struct registers *regs)
{
	if(regs->int_no>=40)
	//slave pic
	{
		outb(0xA0,0x20);//reset 信号
	}
	outb(0x20,0x20);//reset
	if(interrupt_handlers[regs->int_no])
	{
		interrupt_handlers[regs->int_no](regs);
	}
}

void register_int_handler(uchar num, interrupt_handler_t h)
{
	interrupt_handlers[num]=h;
}
