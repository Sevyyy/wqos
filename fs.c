#include "fs.h"
#include "x86.h"
#include "types.h"
#include "defs.h"

// the superblock 
struct super_block spb = {
	0x1111,
    {INODE_MAP_OFFSET,INODE_MAP_SIZE},
    {INODE_ARRAY_OFFSET,INODE_ARRAY_SIZE},
    {SECTOR_MAP_OFFSET,SECTOR_MAP_SIZE},
    DATA_SECT
};

// all the commands and there pointer to the function
static struct command commands[] = {
	{"ls"      ,do_ls},
	{"mkdir"   ,do_mkdir},
	{"cd"      ,do_cd},
	{"touch"   ,do_touch},
	{"catch"   ,do_catch},
	{"pwd"     ,do_pwd},
	{"help"    ,do_help},
	{"pray"    ,do_pray},
	{"dance"   ,do_dance},
	{"quit"    ,do_quit},
	{"echo"    ,do_echo}
};

#define NCOMMANDS (sizeof(commands) / sizeof(commands[0]))
//INIT IN MKFS()
uchar inode_map[INODE_MAP_SIZE * SECTOR_SIZE];
uchar inode_array[INODE_ARRAY_SIZE * SECTOR_SIZE];
uchar sector_map[SECTOR_MAP_SIZE * SECTOR_SIZE];
//uchar data[DISK_SIZE];

//variable !!
uint pwd, root;
char path[64];
char cur_dir[64];


void waitdisk(void)
{
	//wait for disk ready
	while((inb(0x1F7) & 0xC0) != 0x40)
		;
}

// read a single sector at offset(sector) into dest
void readsect(void *dst, uint offset)
{
	waitdisk();
	outb(0x1F2, 1);
	outb(0x1F3, offset);
    outb(0x1F4, offset >> 8);
    outb(0x1F5, offset >> 16);
    outb(0x1F6, (offset >> 24) | 0xE0);
    outb(0x1F7, 0x20); 

    waitdisk();
    insl(0x1F0, dst, SECTOR_SIZE / 4);
}

//read count sectors from the filesystem (remrmber the offset + fs_base)
void readseg(uchar * pa, uint count, uint offset)
{
	offset = offset + FS_BASE;
	for(int i = 0;i < count; i++, pa += SECTOR_SIZE)
	{
		readsect(pa, offset + i);
	}
}

//the same as read but 0x30 for the command
void writesect(void *dst, uint offset)
{
	waitdisk();

	outb(0x1F2, 1);
	outb(0x1F3, offset);
    outb(0x1F4, offset >> 8);
    outb(0x1F5, offset >> 16);
    outb(0x1F6, (offset >> 24) | 0xE0);
    outb(0x1F7, 0x30); 

    waitdisk();
    outsl(0x1F0, dst, SECTOR_SIZE / 4);
}

void writeseg(uchar * pa, uint count, uint offset)
{
	offset = offset + FS_BASE;
	for(int i = 0;i < count; i++, pa += SECTOR_SIZE)
	{
		writesect(pa, offset + i);
	}
}

// initialize the file system
void mkfs()
{
	
	//write super block
	writesect(&spb, 0 + FS_BASE);

	//get the 3 array
	readseg(inode_map, INODE_MAP_SIZE, INODE_MAP_OFFSET);
	readseg(inode_array, INODE_ARRAY_SIZE, INODE_ARRAY_OFFSET);
	readseg(sector_map, SECTOR_MAP_SIZE, SECTOR_MAP_OFFSET);

////////////////////////////////////////////////////////////////////
	//creat the root directory
	struct dir_ent root_ent;                                     //run this code for the first time the file system is made
	struct inode root_inode;

	root_inode.i_num = alloc_inode();
	root_inode.i_pnum = root_inode.i_num;
	root_inode.i_size = 0; // .?  ..?
	root_inode.i_start_sect = alloc_data();
	root_inode.i_nsect = 1;
	root_inode.i_mode = 2;

	root_ent.i_num = root_inode.i_num;

	strcpy(root_ent.name, ".");

	add_entry(&root_inode, &root_ent);
	strcpy(root_ent.name, "..");
	add_entry(&root_inode, &root_ent);

	write_inode(root_inode.i_num, &root_inode);

	pwd = root_inode.i_num;
	root = root_inode.i_num;
	strcpy(path, "/");
	strcpy(cur_dir, "");
/////////////////////////////////////////////////////////////////

////////////////////////////////////////////
	/*
	root = 0;                                     if you run this code but not above(when the file system already put into use) 
	pwd = 0;                                      the file will be saved as long as you quit everytime before you close the qemu
	strcpy(path, "/");
	*/
////////////////////////////////////////////

	//start the shell
	char *buffer;
	while(1)
	{
		char welcome[32];
		strcpy(welcome, "\n[Sevy-OS ");
		strcpy(welcome + strlen(welcome), cur_dir);
		strcpy(welcome + strlen(welcome), "] $ ");
		//buffer = readline("Sevy-OS > ");
		buffer = readline(welcome);
		if(buffer != NULL)
			runcmd(buffer);
	}
	
}

void get_inode(uint num, struct inode * node)
{
	memmove(node, inode_array + num * INODE_SIZE, INODE_SIZE);
}


void write_inode(uint num, struct inode * node)
{
	memmove(inode_array + num * INODE_SIZE, node, INODE_SIZE);
}

void add_entry(struct inode * node, struct dir_ent * dir)
{
	uchar temp[SECTOR_SIZE];
	readseg(temp, 1, DATA_SECT + node->i_start_sect);
	memmove(temp + DIR_ENTRY_SIZE * node->i_size, dir, DIR_ENTRY_SIZE);
	writeseg(temp, 1, DATA_SECT + node->i_start_sect);
	//memmove(data + node->i_start_sect * SECTOR_SIZE + node->i_size * DIR_ENTRY_SIZE, dir, DIR_ENTRY_SIZE);
	node->i_size ++;
}

int alloc_inode()
{
	int ans;
	for(int j = 0;j < SECTOR_SIZE * INODE_MAP_SIZE; j++)
	{
		for(int k = 0;k < 8; k++)
		{
			if(!(inode_map[j] & (1 << k)))
			{
				inode_map[j] |= (1 << k);
				ans = j * 8 + k;
				return ans;
			}
		}
	}
	return -1;
}

void free_inode(uint num)
{
	int k = num % 8;
	int j = num / 8;
	inode_map[j] &= ~(1 << k);
}

uint alloc_data()
{
	int ans;
	for(int j = 0;j < SECTOR_SIZE * SECTOR_MAP_SIZE; j++)
	{
		for(int k = 0;k < 8; k++)
		{
			if(!(sector_map[j] & (1 << k)))
			{
				sector_map[j] |= (1 << k);
				ans = j * 8 + k;
				return ans;
			}
		}
	}
	return -1;
}

void free_sector(uint num)
{
	int k = num % 8;
	int j = num / 8;
	sector_map[j] &= ~(1 << k);
}

void runcmd(char * p)
{
	char cmd[16];
	char para[32];
	strsplit(p, ' ', cmd, para);
	// cprintf("fullcmd : %s\n", p);
	// cprintf("cmd : %s\n", cmd);
	// cprintf("para : %s\n", para);
	for(int i = 0; i < NCOMMANDS; i++)
	{
		if(!strcmp(commands[i].name, cmd))
		{
			commands[i].func(para);
			return;
		}
	}
	cprintf("Unknown command!\n");
}

// the function for all commands

void do_ls(char * p)
{
	struct inode cur_node, item;
	struct dir_ent ent;
	get_inode(pwd, &cur_node);

	uchar temp[SECTOR_SIZE];
	readseg(temp, 1, DATA_SECT + cur_node.i_start_sect);

	for(int i = 0; i < cur_node.i_size;i++)
	{
		memmove(&ent, temp + i * DIR_ENTRY_SIZE, DIR_ENTRY_SIZE);
		get_inode(ent.i_num, &item);
		cprintf("%s ------ %s\n", item.i_mode == 2?"dir ":"file", ent.name);	
		//cprintf("%s\n",ent.name);
	}
}

void do_mkdir(char * p)
{
	struct inode new_node, cur_node;
	struct dir_ent new_dir;
	get_inode(pwd, &cur_node);

	new_node.i_num = alloc_inode();
	new_node.i_start_sect = alloc_data();
	new_node.i_size = 0;
	new_node.i_nsect = 1;
	new_node.i_mode = 2;
	new_node.i_pnum = pwd;

	new_dir.i_num = pwd;
	strcpy(new_dir.name, "..");
	add_entry(&new_node, &new_dir);
	strcpy(new_dir.name, ".");
	add_entry(&new_node, &new_dir);

	new_dir.i_num = new_node.i_num;
	strcpy(new_dir.name, p);
	add_entry(&cur_node, &new_dir);

	write_inode(pwd, &cur_node);
	write_inode(new_node.i_num, &new_node);
}

void do_cd(char * p)
{
	struct inode cur_node, new_node;
	struct dir_ent ent;
	if(!strcmp(p,"."))
	{	
		return;
	}
	get_inode(pwd, &cur_node);
	uchar temp[SECTOR_SIZE];
	readseg(temp, 1, DATA_SECT + cur_node.i_start_sect);
	for(int i = 0;i < cur_node.i_size; i++)
	{
		memmove(&ent, temp + i * DIR_ENTRY_SIZE, DIR_ENTRY_SIZE);
		if(!strcmp(p, ent.name))
		{
			get_inode(ent.i_num, &new_node);
			if(new_node.i_mode == 1)
			{
				cprintf("Can not cd to a file.\n");
				return;
			}
			if(!strcmp(p, ".."))
			{
				if(pwd == root)
					return;
				int last = 0;
				for(int j = 0;path[j];j++)
					if(path[j] == '/')
						last = j;
				path[last] = '\0';
				if(last == 0)
				{
					path[0] = '/';
					path[1] = '\0';
				}
				pwd = ent.i_num;
				last = 0;
				for(int j = 0;path[j];j++)
					if(path[j] == '/')
						last = j;
				strcpy(cur_dir, path + last + 1);
			}
			else
			{
				if(strcmp(path,"/"))
					strcat(path, "/");
				strcat(path, ent.name);
				strcpy(cur_dir, ent.name);
				pwd = ent.i_num;
			}
			return;
		}
	}
	cprintf("No such file or directory.\n");
}

void do_touch(char * p)
{
	struct inode cur_node, new_node;
	struct dir_ent ent;
	get_inode(pwd, &cur_node);

	new_node.i_num = alloc_inode();
	new_node.i_start_sect = alloc_data();
	new_node.i_nsect = 1;
	new_node.i_pnum = pwd;
	new_node.i_mode = 1;
	new_node.i_size = 0;

	ent.i_num = new_node.i_num;
	strcpy(ent.name, p);
	add_entry(&cur_node, &ent);

	write_inode(pwd, &cur_node);
	write_inode(new_node.i_num, &new_node);
}

void do_catch(char * p)
{
	struct inode cur_node, file;
	struct dir_ent ent;
	get_inode(pwd, &cur_node);

	uchar temp[SECTOR_SIZE];
	readseg(temp, 1, DATA_SECT + cur_node.i_start_sect);

	for(int i = 0;i < cur_node.i_size; i++)
	{
		memmove(&ent, temp + i * DIR_ENTRY_SIZE, DIR_ENTRY_SIZE);
		if(!strcmp(ent.name, p))
		{
			get_inode(ent.i_num, &file);
			if(file.i_mode == 2)
			{
				cprintf("Can not catch a directory.\n");
				return;
			}
			else
			{
				uchar file_buf[SECTOR_SIZE];
				readseg(file_buf, 1, DATA_SECT + file.i_start_sect);
				cprintf("%s\n", file_buf);
				return;
			}
		}
	}
	cprintf("No such file.\n");
}

void do_pwd(char * p)
{
	cprintf("%s\n", path);
}

void do_help(char * p)
{
	cprintf("The command you can type as follow:\n");
	for(int i = 0;i < NCOMMANDS; i++)
	{
		cprintf("  %s\n",commands[i].name);
	}
}

void do_pray(char * p)
{
	cprintf("                                                                           \n");
	cprintf("      ***       ***            ***       ***            ***       ***      \n");
	cprintf("      * *       * *            * *       * *            * *       * *      \n");
	cprintf("                                                                           \n");
	cprintf("            *                        *                        *            \n");
	cprintf("            *                        *                        *            \n");
	cprintf("    ***           ***        ***           ***        ***           ***    \n");
	cprintf("     ***         ***          ***         ***          ***         ***     \n");
	cprintf("      ***       ***            ***       ***            ***       ***      \n");
	cprintf("       ***********              ***********              ***********       \n");
	cprintf("        ********                 *********                *********        \n");
	cprintf("                                                                           \n");
}

void do_dance(char * p)
{
	cleanscreen();
	for(int i = 0;i < 1000;i++)
	{
		for(int j = 0;j < 25; j++)
		{
			for(int k = 0;k < 80; k++)
			{
				int t = i % 24;
				if(t < 12)
				{
					if(k <= 3 * t + 2 || k >= 80 - 3 * t - 2 || j <= t || j >= 25 - t)
						cprintf("*");
					else
						cprintf(" ");
				}
				else
				{
					t = 23 - t;
					if(k <= 3 * t + 2 || k >= 80 - 3 * t - 2 || j <= t || j >= 25 - t)
						cprintf("*");
					else
						cprintf(" ");
				}
			}
		//cprintf("\n");
		}
		for(int o = 0;o < 10000000;o++)
			;
		cleanscreen();
	}
}

void do_quit(char * p)
{
	writeseg(inode_map, INODE_MAP_SIZE, INODE_MAP_OFFSET);
	writeseg(inode_array, INODE_ARRAY_SIZE, INODE_ARRAY_OFFSET);
	writeseg(sector_map, SECTOR_MAP_SIZE, SECTOR_MAP_OFFSET);
}

void do_echo(char *p)
{
	struct inode cur_node, file;
	struct dir_ent ent;
	get_inode(pwd, &cur_node);

	uchar temp[SECTOR_SIZE];
	readseg(temp, 1, DATA_SECT + cur_node.i_start_sect);

	for(int i = 0;i < cur_node.i_size; i++)
	{
		memmove(&ent, temp + i * DIR_ENTRY_SIZE, DIR_ENTRY_SIZE);
		if(!strcmp(ent.name, p))
		{
			get_inode(ent.i_num, &file);
			if(file.i_mode == 2)
			{
				cprintf("Can not echo to a directory.\n");
				return;
			}
			else
			{
				uchar file_buf[SECTOR_SIZE];
				char * read = readline("");
				strcpy(file_buf, read);
				int file_size = strlen(file_buf);
				writeseg(file_buf, 1, DATA_SECT + file.i_start_sect);
				file.i_size = file_size;
				write_inode(file.i_num, &file);
				return;
			}
		}
	}
	cprintf("No such file.\n");
}
