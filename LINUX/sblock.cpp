#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sblock.h"
#include "inode.h"
#include "dblock.h"
#include "file.h"
#include "user.h"

//初始化超级块
void Initsb(struct Superb *superblock) {
	superblock->ptrRoot = -1;//指向根目录的指针
	InitInode(&superblock->freeInode);//初始化空闲inode栈
	superblock->block[0] = 100;//空闲块100
	for (int i = 100; i > 0; i--) {
		superblock->block[101-i] = i;
	}
}
//设置根目录指针
void SetPtrRoot(FILE *fp, int p) {
	//读取超级块
	fseek(fp, 0, SEEK_SET);
	struct Superb superblock;
	fread(&superblock, SBlockSize, 1, fp);

	//修改根目录指针
	superblock.ptrRoot = p;

	//写入超级块,获取超级块的信息到fp
	W_Sblock(fp, superblock);
}
//读取超级块
struct Superb Getsb(FILE *fp) {
	struct Superb superblock;
	fseek(fp, 0, SEEK_SET);//将文件指针fp移到文件开头，偏移量为0
	//fread(object &obj, int size, int count, FILE *fp);
	//将fp指向的文件读到超级块中，每次大小sizeof，读count次
	fread(&superblock, sizeof(superblock), 1, fp);
	//返回超级块
	return superblock;
}

//获取超级块的信息写到fp
void W_Sblock(FILE *fp, struct Superb superblock) {
	fseek(fp,0, SEEK_SET);//指针定位到文件开头
	fwrite(&superblock, SBlockSize, 1, fp);//把superblock写到fp中
}

//显示超级块内容
void showSb(FILE *fp) {
	struct Superb superblock = Getsb(fp);
	printf("空闲iNode块：%d\n", superblock.freeInode.num);
	for (int i = 0; i < INodeNum; ++i) {
		printf("%d ",superblock.freeInode.stack[i]);
	}
	printf("\n空闲block块：%d\n",superblock.block[0]);
	for (int i = 1; i < 101; ++i) {
		printf("%d ", superblock.block[i]);
	}
	printf("\n");
}

//创建一个新的文件系统
void CreateSystem(const char fsName[]) {
	printf("...安装系统中...\n");
	Sleep(200);
	FILE *fp;
	fp = fopen(fsName, "wb+"); //正文文件读写方式打开，文件不存在，则建立文件；否则截取文件长度为0
	//文件系统大小(卷大小)
	int sumSize = SBlockSize + INodeNum * INodeSize + DBlockNum * DBlockSize;
	char fillChar = 0;
	fwrite(&fillChar, 0, sumSize, fp);//fp指向文件头部

	//初始化超级块
	printf("正在初始化超级块...\n");
	Sleep(300);
	struct Superb superblock;
	Initsb(&superblock);
	W_Sblock(fp, superblock);//超级块写入文件
	//成组链接
	//superBlock inode 100 99 ...1 200 199 ... 201 300 299 ... 201 ...1000 999 ... 901
	int blockNumber = superblock.block[1];//空闲盘块成组链接
	//每100块为一组
	for (; blockNumber <= DBlockNum; blockNumber += 100) {
		int block[101];
		//每一组默认放满100个空闲块，最后不足一百时取剩下的块数
		block[0] = Min(100, DBlockNum - blockNumber);
		if (block[0] == 100) {
			for (int j = 100; j > 0; j--) {
				block[101 - j] = j + blockNumber;
			}
		}
		else {
			block[1] = 0;
			for (int j = 0; j < block[0]; ++j) {
				block[j + 2] = blockNumber + block[0] - j;
			}
		}
		//获取数据块的地址
		fseek(fp, Getdb_num(blockNumber), SEEK_SET);
		fwrite(&block, sizeof(block), 1, fp);
	}


	//创建根目录/
	printf("正在创建根目录...\n");
	Sleep(300);
	int currentInode;
	currentInode = newDir(fp, -1, "/", "root", "root");
	SetPtrRoot(fp, currentInode);
	if (currentInode == -1) {
		printf("创建根目录失败！\n");
		remove(fsName);
		exit(1);
	}
	else {
		printf("创建根目录成功！\n");
	}

	//showInode(GetInode(fp, currentInode));
	//创建root/目录
	printf("正在创建root目录...\n");
	Sleep(300);
	if (newDir(fp, currentInode, "root/", "root", "root") == -1) {
		printf("root目录创建失败！\n");
		remove(fsName);
		exit(2);
	}
	else {
		printf("root目录创建成功！\n");
	}
	//创建home目录
	printf("正在创建home目录...\n");
	Sleep(300);
	if (newDir(fp, currentInode, "home/", "root", "root") == -1) {
		printf("home目录创建失败\n");
		remove(fsName);
		exit(3);
	}
	else {
		printf("home目录创建成功！\n");
	}

	//创建sys/目录
	printf("正在创建sys目录...\n");
	Sleep(300);
	currentInode = newDir(fp, currentInode, "sys/", "root", "root");
	if (currentInode == -1) {
		printf("sys目录创建失败！\n");
		remove(fsName);
		exit(4);
	}
	else {
		printf("sys目录创建成功！\n");
	}

	//创建密码文件
	struct User user[3];
	user[0] = CreateUser("root", "root", "root");
	user[1] = CreateUser("hzb", "123456", "a");
	user[2] = CreateUser("parker", "123456", "b");
	int tmpInode = newFile(fp, currentInode, "userinfo", "root", "root");
	struct Inode inode = GetInode(fp, tmpInode);
	inode.permission = 0750;
	wInode(fp, inode, tmpInode);
	WFile(fp, tmpInode, user, sizeof(user), "root");
	newDir(fp, -1, "/home/hzb/", "root", "root");
	newDir(fp, -1, "/home/parker/", "root", "root");
	printf("...系统安装成功...\n");
	int a;
	scanf("%d", &a);
	Sleep(200);
}