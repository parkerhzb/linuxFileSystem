#ifndef SBLOCK_H
#define SBLOCK_H
#include "inode.h"
struct Superb {
	int ptrRoot; //指向根目录的指针
	Finode freeInode;//空闲inode
	int block[101];//空闲盘块号栈
};
void Initsb(struct Superb *superblock);//初始化超级块
struct Superb Getsb(FILE *fp);//返回超级块
void W_Sblock(FILE *fp, struct Superb superblock);//超级块的信息写到fp
void SetPtrRoot(FILE *fp, int p);//设置根目录的指针
void showSb(FILE *fp);//显示超级块内容
void CreateSystem(const char fsName[]);//创建文件系统
#define SBlockSize sizeof(struct Superb)//超级块的大小
#endif