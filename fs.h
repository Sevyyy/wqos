#ifndef _FS_H
#define _FS_H

#include "types.h"

#define MAX_NAME 256
#define MAX_INODE 4096
#define MAXBUF 1024
#define DISK_SIZE 1024 * 1024 * 10   //10M
#define SECTOR_SIZE 512      //bytes
#define ROOT 0
#define FS_BASE 5120

#define INODE_SIZE 24
#define DIR_ENTRY_SIZE 16
#define MAX_DIR SECTOR_SIZE / DIR_ENTRY_SIZE

#define INODE_MAP_OFFSET 1
#define INODE_MAP_SIZE 1
#define INODE_ARRAY_OFFSET 2
#define INODE_ARRAY_SIZE  4096*INODE_SIZE/SECTOR_SIZE
#define SECTOR_MAP_OFFSET 3 + 4096*INODE_SIZE/SECTOR_SIZE
#define SECTOR_MAP_SIZE 5
#define DATA_SECT 8 + 4096*INODE_SIZE/SECTOR_SIZE

#define DATA_BASE = FS_BASE + DATA_SECT

struct segment
{
	uint offset;
	uint size;
};                   // in sector

struct inode
{
	uint i_num;    // the number of an inode
	uint i_pnum;   // the number of the parent inode
	uint i_size;   // the size of file or the size of a directory
	uint i_start_sect;     //the start sector of the file
	uint i_nsect;   // the number of sectors the file use
	uint i_mode;     // 1 = file    2 = directory 
};

#define FILE_MODE 1
#define DIR_MODE 2

struct super_block
{
	uint magic;         // a magic number   //0x1111
	struct segment inode_map;   // inode bit map    //2,1
	struct segment inode_array;    // the inode table  3,
	struct segment sector_map;    // the sector bitmap
	uint data_sect;            //the start  sector of the data
};


struct dir_ent
{
	uint i_num;    // the number of an inode
	char name[12];   // the name of the entry
};

void waitdisk(void);
void readsect(void *dst, uint offset);
void readseg(uchar * pa, uint count, uint offset);
void writesect(void *src, uint offset);
void writeseg(uchar * pa, uint count, uint offset);
void mkfs();
void get_inode(uint num, struct inode * node);
void write_inode(uint num, struct inode * node);
void add_entry(struct inode * node, struct dir_ent * dir);
int alloc_inode();
void free_inode(uint num);
uint alloc_data();
void free_sector(uint num);

struct command{
	const char * name;
	void (*func)(char * p);
};
void runcmd(char * p);
void do_ls(char * p);
void do_mkdir(char * p);
void do_cd(char * p);
void do_touch(char * p);
void do_catch(char * p);
void do_pwd(char * p);
void do_help(char * p);
void do_pray(char * p);
void do_dance(char * p);
void do_quit(char * p);
void do_echo(char * p);
#endif