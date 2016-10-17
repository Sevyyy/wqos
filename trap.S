
.globl idt_flush

idt_flush:
    call test1
	movl 4(%esp), %eax
	lidt (%eax)
	ret

#define ISR_NOERR(name, num)   \
	.long name;                \ 
	.globl name;               \
	.type name, @function;     \
	.align 2;                  \
	name:                      \
		pushl $0;              \
		pushl $(num);          \
		jmp isr_common_stub
		
#define IRQ(name, num)         \
	.long name;                \ 
	.globl name;               \
	.type name, @function;     \
	.align 2;                  \
	name:                      \
		hlt;                   \
		pushl $0;              \
		pushl $(num);          \
		jmp irq_common_stub

#define ISR_ERR(name, num)     \
	.long name;                \ 
	.globl name;               \
	.type name, @function;     \
	.align 2;                  \
	name:                      \
		pushl $(num);          \
		jmp isr_common_stub


ISR_NOERR(isr0, 0)
ISR_NOERR(isr1, 1)
ISR_NOERR(isr2, 2)
ISR_NOERR(isr3, 3)
ISR_NOERR(isr4, 4)
ISR_NOERR(isr5, 5)
ISR_NOERR(isr6, 6)
ISR_NOERR(isr7, 7)
ISR_ERR  (isr8, 8)
ISR_NOERR(isr9, 9)
ISR_ERR  (isr10, 10)
ISR_ERR  (isr11, 11)
ISR_ERR  (isr12, 12)
ISR_ERR  (isr13, 13)
ISR_ERR  (isr14, 14)
ISR_NOERR(isr15, 15)
ISR_NOERR(isr16, 16)
ISR_ERR  (isr17, 17)
ISR_NOERR(isr18, 18)
ISR_NOERR(isr19, 19)

ISR_NOERR(isr20, 20)
ISR_NOERR(isr21, 21)
ISR_NOERR(isr22, 22)
ISR_NOERR(isr23, 23)
ISR_NOERR(isr24, 24)
ISR_NOERR(isr25, 25)
ISR_NOERR(isr26, 26)
ISR_NOERR(isr27, 27)
ISR_NOERR(isr28, 28)
ISR_NOERR(isr29, 29)
ISR_NOERR(isr30, 30)
ISR_NOERR(isr31, 31)

ISR_NOERR(isr128, 128)

IRQ(irq0, 32)	
IRQ(irq1, 33)
IRQ(irq2, 34) 
IRQ(irq3, 35) 
IRQ(irq4, 36) 
IRQ(irq5, 37) 
IRQ(irq6, 38) 
IRQ(irq7, 39) 
IRQ(irq8, 40) 
IRQ(irq9, 41) 
IRQ(irq10, 42) 
IRQ(irq11, 43) 
IRQ(irq12, 44) 
IRQ(irq13, 45) 
IRQ(irq14, 46) 
IRQ(irq15, 47)

.globl isr_common_stub

isr_common_stub:
	pushal
	mov %ds, %ax
	pushl %eax
	movw $16, %ax 
	movw %ax, %ds
   	movw %ax, %es
   	movw %ax, %fs
   	movw %ax, %gs
    movw %ax, %ss

	pushl %esp
   	call isr_handler
	add $4, %esp

   	popl %ebx       
   	movw %bx, %ds
   	movw %bx, %es
   	movw %bx, %fs
   	movw %bx, %gs
    movw %bx, %ss

   	popal                    
   	add $(8), %esp   
	sti
   	iret       

.globl irq_common_stub

irq_common_stub:
	pushal
	mov %ds, %ax
	pushl %eax
	movw $16, %ax 
	movw %ax, %ds
   	movw %ax, %es
   	movw %ax, %fs
   	movw %ax, %gs
    movw %ax, %ss

	pushl %esp
   	call irq_handler
	add $4, %esp

   	popl %ebx       
   	movw %bx, %ds
   	movw %bx, %es
   	movw %bx, %fs
   	movw %bx, %gs
    movw %bx, %ss

   	popal                    
   	add $(8), %esp   
	sti
   	iret       