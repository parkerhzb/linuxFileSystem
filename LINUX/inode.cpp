#include <stdio.h>

#include "inode.h"
#include "sblock.h"
#include "dblock.h"

//获取iNode的地址
int getInodeNum(int index) {
	//超级块大小 + index*inode大小
	return SBlockSize + index * INodeSize;
}
//获取inode
struct Inode GetInode(FILE *fp, int index) {
	struct Inode inode;
	//指针指向第p个inode偏移量的位置
	fseek(fp, getInodeNum(index), SEEK_SET);
	//文件信息读入新的inode
	fread(&inode, sizeof(inode), 1, fp);
	return inode;
}
//在系统中写入inode，获取inode的信息，偏移量为p个inode
void wInode(FILE *fp, struct Inode inode, int index) {
	fseek(fp, getInodeNum(index), SEEK_SET);
	//把inode信息写到fp中
	fwrite(&inode, sizeof(inode), 1, fp);
}
//初始化空闲inode栈
void InitInode(Finode *freeInode) {
	int i;
	freeInode->num = INodeNum;
	//初始化inode编号
	for (i = 0; i < INodeNum; i++) {
		freeInode->stack[i] = i;
	}
}
//输出inode数据
void showInode(struct Inode inode) {
	int i;
	printf("inode:\n");
	printf("    name = %s\n", inode.name);
	printf("    size = %d\n", inode.size);
	printf("    linkNum = %d\n", inode.linknum);
	printf("    type = %d\n", inode.type);
	printf("    owner = %s\n", inode.owner);
	printf("    group = %s\n", inode.group);
	printf("    permission = %o\n", inode.permission);
	printf("    time = %s\n", inode.time);
	printf("    addr0:");//直接块
	for (i = 0; i < Add0Num; i++)
		printf(" %d", inode.addr0[i]);
	printf("\n    addr1:");//一次间址块
	for (i = 0; i < Add1Num; i++)
		printf(" %d", inode.addr1[i]);
	printf("\n    addr2:");//二次间址块
	for (i = 0; i < Add2Num; i++)
		printf(" %d", inode.addr2[i]);
	printf("\n");
}
//获取空闲inode的inode号
int GetFreeInode(FILE *fp) {
	struct Superb superblock = Getsb(fp);
	if (superblock.freeInode.num == 0) {
		printf("inode栈已满，没有可用inode!\n");
		return -1;
	}
	superblock.freeInode.num--;
	//新获取的inode编号
	int inodeNmuber = superblock.freeInode.stack[superblock.freeInode.num];
	//将使用掉的stack[inodeNumber]标记为0
	superblock.freeInode.stack[superblock.freeInode.num] = 0;
	W_Sblock(fp, superblock);
	//printf("%d\n", inodeNmuber);
	return inodeNmuber;
}
//释放文件，添加inode
void PushFreeInode(FILE *fp, int index) {
	struct Superb superblock = Getsb(fp);
	//最近一个被占用的stack[]存为释放inode编号
	superblock.freeInode.stack[superblock.freeInode.num] = index;
	superblock.freeInode.num++;
	W_Sblock(fp, superblock);
}