#include "types.h"
#include "memlayout.h"
#include "x86.h"
#include "mmu.h"
#include "defs.h"
#include "param.h"
#include "fs.h"
#include "idt.h"
int main(void)
{
	cleanscreen();

	cprintf("\n\n\n     Welcome to Sevy-OS!\n\n\n");

	remap();

	init_idt();

	//print_test();

	//__asm__("sti\n");
	//__asm__("int $33\n");

	mkfs();
}


