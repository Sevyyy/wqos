OBJS = \
	console.o\
	entrypgtable.o\
	fs.o\
	idt.o\
	main.o\
	string.o\
	trap.o\


CC = gcc
AS = as
LD = ld
OBJCOPY = objcopy


CFLAGS = -fno-pic -static -fno-builtin -fno-strict-aliasing -fvar-tracking -fvar-tracking-assignments -O0 -g -Wall -MD -gdwarf-2 -m32   -fno-omit-frame-pointer
CFLAGS += $(shell $(CC) -fno-stack-protector -E -x c /dev/null >/dev/null 2>&1 && echo -fno-stack-protector)
ASFLAGS = -m32 -gdwarf-2 -Wa,-divide
LDFLAGS += -m $(shell $(LD) -V | grep elf_i386 2>/dev/null)

myos.img: bootblock kernel
	dd if=/dev/zero of=myos.img count=25000
	dd if=bootblock of=myos.img conv=notrunc
	dd if=kernel of=myos.img seek=1 conv=notrunc
	qemu -hda myos.img -m 512 

bootblock: bootasm.S bootmain.c
	$(CC) $(CFLAGS) -fno-pic -O -nostdinc -I. -c bootmain.c
	$(CC) $(CFLAGS) -fno-pic -nostdinc -I. -c bootasm.S
	$(LD) $(LDFLAGS) -N -e start -Ttext 0x7C00 -o bootblock.o bootasm.o bootmain.o
	$(OBJCOPY) -S -O binary -j .text bootblock.o bootblock
	perl ./sign.pl bootblock

kernel: $(OBJS) entry.o kernel.ld
	$(LD) $(LDFLAGS) -T kernel.ld -o kernel entry.o $(OBJS) -b binary
clean:
	rm -f *.tex *.dvi *.idx *.aux *.log *.ind *.ilg *.o *.d *.asm *.sym bootblock kernel myos.img 
qemu:
	qemu-system-i386 -hda myos.img -m 512

debug:
	qemu-system-i386 -hda myos.img -m 512 -gdb tcp::26000 -S



