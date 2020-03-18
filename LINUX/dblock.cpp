#include <stdio.h>

#include "dblock.h"
#include "sblock.h"
#include "inode.h"

//获取数据块的地址
int Getdb_num(int index) {
	//超级块大小 + (所有inode大小)i节点num*i节点大小 + index*数据块大小(512)
	return SBlockSize + INodeNum * INodeSize + index * DBlockSize;
}

//获取一个空闲数据块
int GetFreedb(FILE *fp) {
	//将fp内容读到超级块，返回该超级块
	struct Superb superblock = Getsb(fp);
	//没有空闲块：空闲盘块号第一位(空闲块数)=1，第二位(是否占用)=0
	if (superblock.block[0] == 1 && superblock.block[1] == 0) return -1;
	//在当前空闲栈里有空闲块
	if (superblock.block[0] > 1) {
		//最近可用的空闲块号
		int blockNumber = superblock.block[superblock.block[0]];
		//将该块置为已使用
		superblock.block[superblock.block[0]] = 0;
		//可用数减一
		superblock.block[0]--;
		//将超级块内容写入fp，获取超级块内容
		W_Sblock(fp, superblock);
		//返回当前使用后的空闲块号
		return blockNumber;
	}
	//有空闲块，不在当前空闲栈
	else {
		//新建空闲栈
		int newblock[101];
		newblock[0] = 100;
		//fp指向文件开头,偏移量 第一个空闲块的大小
		fseek(fp, Getdb_num(superblock.block[1]), SEEK_SET);
		//将fp内容放入新空闲块栈中
		fread(&newblock + 1, sizeof(newblock) - 1, 1, fp);
		//更新超级块的空闲块栈的内容
		for (int i = 0; i < 101; ++i) {
			superblock.block[i] = newblock[i];
		}
		//超级块内容写入fp，获取超级块内容
		W_Sblock(fp, superblock);
		//进行递归判断这个新的空闲栈的可用空间
		return GetFreedb(fp);
	}
}

//释放数据，新增空闲块
void PushFreedb(FILE *fp, int index) {
	//将fp内容读到超级块，返回该超级块
	struct Superb superblock = Getsb(fp);
	//当前空闲栈的空闲块数少于100
	if (superblock.block[0] < 100) {
		superblock.block[superblock.block[0] + 1] = index;
		superblock.block[0]++;//空闲块数加一
	}
	//当前空闲栈均空闲
	else {
		//取一个满的空闲栈
		int newblock[101] = { 0 };
		newblock[0] = 2;
		newblock[1] = superblock.block[1] - 100;
		newblock[2] = index;
		//空闲栈的使用回到之前一个
		for (int i = 0; i < 101; ++i) {
			superblock.block[i] = newblock[i];
		}
	}
	//获取超级块信息到fp
	W_Sblock(fp, superblock);
}