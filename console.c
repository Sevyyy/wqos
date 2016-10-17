#include "types.h"
#include "defs.h"
#include "memlayout.h"
#include "x86.h"
#include "mmu.h"
#include "param.h"
#include "kbd.h"

//print a char on che console
static void consputc(int);      

// print a number in base
static void printint(int xx, int base, int sign)
{
	static char digits[] = "0123456789abcdef";
	char buf[16];
	int i;
	uint x;

	if(sign && (sign = xx < 0))
		x = -xx;
	else
		x = xx;
	i = 0;
	do
	{
		buf[i++] = digits[x % base];
	}while((x /= base) != 0);

	if(sign)
		buf[i++] = '-';

	while(--i >= 0)
		consputc(buf[i]);
}

//the printf in c
void cprintf(char *fmt, ...)
{
	int i, c;
	uint *argp;
	char *s;

	if(fmt == 0)
		panic("null fmt");

	argp = (uint*)(void*)(&fmt + 1);
	for(i = 0; (c = fmt[i] & 0xff) != 0; i++)
	{
		if(c != '%')
		{
			consputc(c);
			continue;
		}
		c = fmt[++i] & 0xff;
		if(c == 0)
			break;
		switch(c)
		{
			case 'd':
				printint(*argp++, 10, 1);
				break;
			case 'x':
			case 'p':
				printint(*argp++, 16, 0);
				break;
			case 's':
				if((s = (char*)*argp++) == 0)
					s = "(null)";
				for(; *s; s++)
					consputc(*s);
				break;
			case '%':
				consputc('%');
				break;
			default:
				// Print unknown % sequence to draw attention.
      			consputc('%');
      			consputc(c);
      			break;
		}
	}
}

// panic
void panic(char *s)
{
	cli();
	cprintf(s);
	cprintf("\n");
	for(;;)
		;
}


#define BACKSPACE 0x100
#define CRTPORT 0x3d4
static ushort *crt = (ushort*)P2V(0xb8000); 
// print a char in the screen
static void cgaputc(int c)
{
	int pos;
	outb(CRTPORT, 14);
	pos = inb(CRTPORT + 1) << 8;
	outb(CRTPORT, 15);
	pos |= inb(CRTPORT + 1);

	if(c == '\n')
		pos += 80 - pos % 80;
	else if(c == BACKSPACE)
	{
		if(pos > 0)
			--pos;
	}
	else
		crt[pos++] = (c&0xff) | 0x0700;

	if(pos < 0 || pos > 25 * 80)
		panic("pos under/overflow");


    if((pos/80) >= 24)
    {  // Scroll up.
       memmove(crt, crt+80, sizeof(crt[0])*23*80);
       pos -= 80;
       memset(crt+pos, 0, sizeof(crt[0])*(24*80 - pos));
    }

    outb(CRTPORT, 14);
    outb(CRTPORT + 1, pos>>8);
    outb(CRTPORT, 15);
    outb(CRTPORT + 1, pos);
    crt[pos] = ' '| 0x0700;

} 

//clean the screen
void cleanscreen()
{
	for(int i = 0;i < 25 * 80; i++)
		crt[i] = ' ' | 0x0700;

	outb(CRTPORT, 14);
	outb(CRTPORT + 1, 0);
	outb(CRTPORT, 15);
	outb(CRTPORT + 1, 0);
	crt[0] = ' ' | 0x0700;
}

// put a char
void consputc(int c)
{
	cgaputc(c);
}

static uchar *charcode[4] = {normalmap, shiftmap, ctlmap, ctlmap};

// get a char from the key board
int kbdgetc(void)
{
	static uint shift;
	int c;
	uchar data, st;

	st = inb(KBSTATP);
	if((st & KBS_DIB) == 0)    //fail
		return -1;
	data = inb(KBDATAP);
	if(data == 0xE0)         
	{
		shift |= E0ESC;
		return 0;
	}
	else if(data & 0x80)   //key released ??
	{
		data = (shift & E0ESC ? data : data & 0x7F);
		shift &= ~(shiftcode[data] | E0ESC);
		return 0;
	}
	else if(shift & E0ESC)
	{
		data |= 0x80;
		shift &= ~E0ESC;
	}
	shift |= shiftcode[data];
	shift ^= togglecode[data];

	c = charcode[shift & (CTL | SHIFT)][data];

	if(shift & CAPSLOCK)
	{
		if('a' <= c && c <= 'z')
			c += 'A' - 'a';
		else if('A' <= c && c <= 'Z')
			c += 'a' - 'A';
 	}
 	return c;
}

//keyboard interupt
void kbdintr(void)
{
	consoleintr(kbdgetc);
}

// a buffer to save the chars from keyboard to wait for use
#define INPUT_BUF 64
struct
{
	char buf[INPUT_BUF];
	uint r;    //read
	uint w;    //write
	uint e;    //edit
} input;

//console interupt
void consoleintr(int (*getc)(void))
{
	int c;
	while((c = getc()) >= 0)
	{
		
		if(c == '\x7f')
		{
			{
				input.w--;
			}
		}
		
		else if(c != 0)
		{
			c = (c == '\r') ? '\n' : c;
			input.buf[input.w++ % INPUT_BUF] = c;
			if(input.w == INPUT_BUF)
			{
				input.w = 0;
			}
		}
	}
}

//get a char from the console
int consgetc(void)
{
	int c;
	kbdintr();
	
	if(input.r != input.w)
	{
		c = input.buf[input.r++ % INPUT_BUF];
		if(input.r == INPUT_BUF)
			input.r = 0;
		return c;
	}
	return 0;
}

//get a real char
int getchar(void)
{
	int c;
	while((c = consgetc()) == 0)
		;
	return c;
}

//buffer for readline()
#define BUFLEN 1024
static char buf[BUFLEN];

//read a line, something like readline() in c
char* readline(const char *prompt)
{
	int i, c;
	if(prompt != NULL)
		cprintf("%s", prompt);

	i = 0;
	while(1)
	{

		c = getchar();
		if(c < 0)
		{
			cprintf("read error: %e\n", c);
			return NULL;
		}
		else if((c == '\b' || c == '\x7f') && i > 0)
		{
			consputc(BACKSPACE);
			i--;
		}
		else if(c >= ' ' && i < BUFLEN - 1)
		{
			consputc(c);
			buf[i++] = c;
		}
		else if(c == '\n' || c == '\r')
		{
			consputc('\n');
			buf[i] = 0;
			return buf;
		}
	}
}

