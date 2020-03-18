#ifndef INODE_H
#define INODE_H

#include <stdio.h>
#include "user.h"
#include "file.h"
#define INodeNum 40//i节点数量
#define Add0Num 4//四个直接块
#define Add1Num 1//一个一次间址块
#define Add2Num 1//一个二次间址块
#define NADDRS 2
struct Inode {
	char name[FileNameLen]; //文件名
	int size;//文件大小
	int linknum;//文件联接计数
	int type;//类别(0表示文件夹，1表示文件)
	char owner[UserNameLen];//文件拥有者
	char group[GroupLen];//文件所属组
	int permission;//文件权限 wrx
	char time[83];//文件最后修改时间	
	int addr0[Add0Num]; //直接块号
	int addr1[Add1Num]; //一次间址
	int addr2[Add2Num]; //二次间址
};
struct Finode {
	int num; //空闲inode数目
	int stack[INodeNum]; //空闲inode栈，存放inode编号
};
int getInodeNum(int index);//获取iNode地址
struct Inode GetInode(FILE *fp, int index);//返回第index个inode
void wInode(FILE *fp, struct Inode inode, int index);//在系统中写入inode
void showInode(struct Inode inode);//输出inode数据
void InitInode(Finode *freeInode);//初始化空闲inode栈
int GetFreeInode(FILE *fp);//获取空闲inode
void PushFreeInode(FILE *fp, int index);//释放文件，添加空的inode
#define INodeSize sizeof(struct Inode)
#define FreeINodeSize sizeof(struct Finode)
#endif