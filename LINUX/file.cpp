#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "file.h"
#include "sblock.h"
#include "inode.h"
#include "dblock.h"
int UMARK = 0022;
//创建文件或目录所具有的默认权限 root为0022；普通为0002
int Min(int a, int b) {
	if (a < b) return a;
	else return b;
}
//分割最后一级目录
//例：/home/user/xxx/ 分割为 /home/user/ 和 xxx/
void split1(char *s, char *s1, char *s2) {
	int i;
	for (i = strlen(s) - 2; i >= 0; i--) {
		if (s[i] == '/') break;
	}
	i++;
	strcpy(s1, s);
	strcpy(s2, s + i);
	s1[i] = '\0';
}
//从第一个'/'处分割字符串
//分割为s=s1 s2 （s1包含'/'）
//例如：abc/def/g --> abc/ 和 def/g
int SplitPath(char *s, char *s1, char *s2) {
	//如果s为空串
	if (strcmp(s, "") == 0) {
		strcpy(s1, "");
		strcpy(s2, "");
		return 0;
	}
	//寻找'/'的位置
	int i;
	for (i = 0; s[i] != '\0'; i++) {
		if (s[i] == '/') break;
	}
	//如果s串中不包含'/'
	if (s[i] != '/') {
		strcpy(s1, s);
		strcpy(s2, "");
		return 1;
	}
	i++;
	strcpy(s1, s);
	strcpy(s2, s + i);
	s1[i] = '\0';
	return 2;
}
//分割文件名与目录
//例如：abc/def/g --> abc/def/ 和 g
void split2(char *s, char *s1, char *s2) {
	int i;
	for (i = strlen(s) - 1; i >= 0; i--) {
		if (s[i] == '/') break;
	}
	i++;
	strcpy(s1, s);
	strcpy(s2, s + i);
	s1[i] = '\0';
}
//判断文件是否具有写权限
int W_power(FILE *fp, int curInode, char username[]) {
	if (strcmp(username, "root") == 0) return 1;//超级管理员
	//获取inode信息
	struct Inode inode = GetInode(fp, curInode);
	if (strcmp(username, inode.owner) == 0) {
		if(inode.permission & (1 << 7)) return 1;
		/**** if (inode.permission/100 >= 2) return 1;****/
	}
	if (strcmp(GetGrp(fp, username), inode.group) == 0) {
		if (inode.permission & (1 << 4)) return 1;
		/****if ((inode.permission %100)/10 >= 2) return 1;****/
	}
	if (inode.permission & (1 << 1)) return 1;
	/****if ((inode.permission % 100) % 10 >= 2) return 1;****/
	return 0;
}
//判断文件是否具有读权限
int R_power(FILE *fp, int curInode, char username[]) {
	if (strcmp(username, "root") == 0) return 1;

	struct Inode inode = GetInode(fp, curInode);
	if (strcmp(username, inode.owner) == 0) {
		if (inode.permission & (1 << 8)) return 1;
	}
	if (strcmp(GetGrp(fp, username), inode.group) == 0) {
		if (inode.permission & (1 << 5)) return 1;
	}
	if (inode.permission & (1 << 2)) return 1;
	return 0;
}
//判断文件名是否合法
//检验文件名长度与只能包含数字与字母
int LegalFilename(char filename[]) {
	if (filename[strlen(filename) - 1] == '/')
		filename[strlen(filename) - 1] = '\0';
	if (strlen(filename) < FileNameLen) {
		int i;
		for (i = 0; filename[i] != '\0'; i++) {
			if (!('a' <= filename[i] && filename[i] <= 'z'
				|| 'A' <= filename[i] && filename[i] <= 'Z'
				|| '0' <= filename[i] && filename[i] <= '9')) {
				printf("文件名只能包含字母与数字...!\n");
				return 0;
			}
		}
	}
	else {
		printf("文件名长度不得超过%d!\n", FileNameLen);
		return 0;
	}
	return 1;
}
//创建文件
int newFile(FILE *fp, int curInode, const char fileName[], const char username[], const char group[]) {
	if (curInode == -1) {//绝对路径
		if (GetInodeNum(fp, -1, const_cast<char*>(fileName)) != -1) {
			printf("文件 '%s' 已存在!\n", fileName);
			return -1;
		}

		char s1[FileNum];
		char s2[FileNum];
		split2(const_cast<char*>(fileName), s1, s2);

		int iInode = GetInodeNum(fp, -1, s1);
		if (iInode == -1) {
			printf("文件路径错误!\n");
			return -1;
		}
		//递归至当前路径
		return newFile(fp, iInode, s2, username, group);
	}
	else { //相对路径
		int iNewInode = GetFreeInode(fp);
		struct Inode inodeB;
		strcpy(inodeB.name, fileName);
		inodeB.size = 0;
		inodeB.linknum = 1;
		inodeB.type = 1;
		strcpy(inodeB.owner, username);
		strcpy(inodeB.group, group);
		inodeB.permission = 0777-UMARK;//xwr-wr-wr
		time_t t = time(0);
		strftime(inodeB.time, sizeof(inodeB.time), "%Y/%m/%d %X %a 本年度第%j天 时区:%z", localtime(&t));
		memset(inodeB.addr0, -1, sizeof(inodeB.addr0));
		memset(inodeB.addr1, -1, sizeof(inodeB.addr1));
		memset(inodeB.addr2, -1, sizeof(inodeB.addr2));

		wInode(fp, inodeB, iNewInode);

		//char tmp[FileNum];
		//strcpy(tmp, fileName);

		struct DIR dirData;
		strcpy(dirData.dirName,fileName);
		dirData.ptrInode = iNewInode;

		//获取当前目录的数据
		struct Inode inodeA = GetInode(fp, curInode);

		char *data = (char *)malloc(inodeA.size + sizeof(struct DIR));
		RData(fp, curInode, data, inodeA.size);
		//将新文件路径连接至当前的路径下
		memcpy(data + inodeA.size, &dirData, sizeof(struct DIR));
		//清除旧的信息
		CleanData(fp, curInode);
		//更新新的信息
		WData(fp, curInode, data, inodeA.size + sizeof(struct DIR));
		return iNewInode;
	}
}
//写文件
void WFile(FILE *fp, int curInode, void *data, int size,const char username[]) {
	if (!W_power(fp, curInode, const_cast<char*>(username))) {
		printf("没有该文件的写权限!\n");//判断文件权限
		return;
	}
	struct Inode inode = GetInode(fp, curInode);
	//showInode(inode);
	CleanData(fp, curInode);
	WData(fp, curInode, data, size);
	inode = GetInode(fp, curInode);
	//showInode(inode);
}
//输出文件内容
void showFile(FILE *fp, int curInode, char username[]) {
	if (!R_power(fp, curInode, username)) {
		printf("没有该文件的阅读权限 !\n");
		return;
	}
	struct Inode inode = GetInode(fp, curInode);
	char *data = (char *)malloc(inode.size);
	RData(fp, curInode, data, inode.size);

	int i;
	for (i = 0; i < inode.size; i++) 
		putchar(data[i]);
	printf("\n");
}
//判断目录是否为空
int EmptyDir(FILE *fp, int iInode) {
	struct Inode inode = GetInode(fp, iInode);
	if (inode.type == 0) {
		if (inode.size == 2 * sizeof(struct DIR)) {
			return 1;
		}
		else {
			printf("当前目录不为空!\n");
		}
	}
	else {
		printf("目录不存在!\n");
	}
	return 0;
}
//输出inode的文件路径
void showInodePath(FILE *fp, int curInode) {
	struct Inode inode = GetInode(fp, curInode);
	if (strcmp(inode.name, "/") == 0) {//已经递归至根目录
		printf("/");
		return;
	}
	char *data = (char *)malloc(inode.size);
	RData(fp, curInode, data, inode.size);
	struct DIR *dirArray = (struct DIR *)data;
	showInodePath(fp, dirArray[1].ptrInode);//递归
	//递归方式输出
	printf("%s", inode.name);
	if (inode.type == 0)
		printf("/");
}
//增加inode的链接数
void AddLink(FILE *fp, int iInode, int x) {
	struct Inode inode = GetInode(fp, iInode);
	inode.linknum += x;
	wInode(fp, inode, iInode);
}
//删除inode
int DelInode(FILE *fp, int iInode) {
	struct Inode inode = GetInode(fp, iInode);
	if (inode.linknum == 0 && inode.type == 1
		|| inode.linknum == 1 && inode.type == 0) {
		PushFreeInode(fp, iInode);
		return 1;
	}
	return 0;
}
//读取数据
void RData(FILE *fp, int pInode, char *data, int size) {
	//读取pInode指向的inode块数据
	struct Inode inode = GetInode(fp, pInode);
	int i;
	//从4个直接块寻找
	for (i = 0; i < Add0Num; i++) {
		if (size == 0) return;//没有剩余数据
		int curDatablock = inode.addr0[i];//当前数据块号
		fseek(fp, Getdb_num(curDatablock), SEEK_SET);//指针移到数据块号处
		int readSize = Min(size, DBlockSize);//本次循环读取大小
		fread(data, readSize, 1, fp);//内容放到data中
		data += readSize;//data存储指针指向下一次存储的开始位置
		size -= readSize;//剩余需要读取的数据大小
	}
	//从一个一次间址块寻找
	for (int i = 0; i < Add1Num; i++) {
		if (size == 0) return;//没有剩余数据
		char *data1 = (char *)malloc(128 * sizeof(int));//512，即一个盘块大小
		int curDatablock = inode.addr1[i];//一次间址盘块号
		fseek(fp, Getdb_num(curDatablock), SEEK_SET);
		fread(data1, DBlockSize, 1, fp);//从一次间址盘块号获取数据盘块号
		int *addr = (int *)data1;//数据盘块号
		for (int j = 0; j < NADDRS; ++j) {//在间址块中寻找
			if (size == 0) return;//没有剩余数据
			if (addr[j] != -1) {
				fseek(fp, Getdb_num(addr[j]), SEEK_SET);
				int readSize = Min(size, DBlockSize);
				fread(data, readSize, 1, fp);
				data += readSize;
				size -= readSize;
			}
		}
	}
	//从一个二次间址块寻找
	for (int i = 0; i < Add2Num; i++) {
		if (size == 0) return;//没有剩余数据
		char *data1 = (char *)malloc(128 * sizeof(int));
		int curDatablock = inode.addr2[i];
		//printf("%d", curDatablock);
		fseek(fp, Getdb_num(curDatablock), SEEK_SET);
		fread(data1, DBlockSize, 1, fp);
		int *addr = (int *)data1;
		for (int j = 0; j < NADDRS; ++j) {
			if (size == 0) return;//没有剩余数据
			if (addr[j] != -1) {
				char *data2 = (char *)malloc(128 * sizeof(int));
				fseek(fp, Getdb_num(addr[j]), SEEK_SET);
				fread(data2, DBlockSize, 1, fp);
				int *addr1 = (int *)data2;
				for (int k = 0; k < NADDRS; ++k) {
					if (size == 0) return;//没有剩余数据
					if (addr1[k] != -1) {
						fseek(fp, Getdb_num(addr1[k]), SEEK_SET);
						int readSize = Min(size, DBlockSize);
						fread(data, readSize, 1, fp);
						data += readSize;
						size -= readSize;
					}
				}
			}
		}
	}
}
//清理数据
void CleanData(FILE *fp, int iInode) {
	struct Inode inode = GetInode(fp, iInode);

	int i;
	for (i = 0; i < Add0Num; i++) {
		if (inode.addr0[i] == -1) break;
		PushFreedb(fp, inode.addr0[i]);
		inode.addr0[i] = -1;
	}
	for (i = 0; i < Add1Num; i++) {
		if (inode.addr1[i] == -1) break;
		char *data1 = (char *)malloc(128 * sizeof(int));
		int curDatablock = inode.addr1[i];
		fseek(fp, Getdb_num(curDatablock), SEEK_SET);
		fread(data1, DBlockSize, 1, fp);
		int *addr = (int *)data1;
		int naddr = NADDRS;
		for (int i = 0; i < naddr; ++i) {
			if (addr[i] == -1) break;
			PushFreedb(fp, addr[i]);
			addr[i] = -1;
		}
		fseek(fp, Getdb_num(curDatablock), SEEK_SET);
		fwrite(&addr, sizeof(addr), 1, fp);
		PushFreedb(fp, inode.addr1[i]);
		inode.addr1[i] = -1;
	}
	for (i = 0; i < Add2Num; i++) {
		if (inode.addr2[i] == -1) break;
		char *data2 = (char *)malloc(128 * sizeof(int));
		int curDatablock = inode.addr2[i];
		fseek(fp, Getdb_num(curDatablock), SEEK_SET);
		fread(data2, DBlockSize, 1, fp);
		int *addr = (int *)data2;
		int naddr = NADDRS;
		for (int j = 0; j < NADDRS; j++) {
			if (addr[j] == -1) break;
			char *data3 = (char *)malloc(128 * sizeof(int));
			int curDatablock1 = addr[j];
			fseek(fp, Getdb_num(curDatablock1), SEEK_SET);
			fread(data3, DBlockSize, 1, fp);
			int *addr1 = (int *)data3;
			for (int k = 0; k < NADDRS; ++k) {
				if (addr1[k] == -1) break;
				PushFreedb(fp, addr1[k]);
				addr1[k] = -1;
			}
			fseek(fp, Getdb_num(curDatablock1), SEEK_SET);
			fwrite(&addr1, sizeof(addr1), 1, fp);
			PushFreedb(fp, addr[j]);
			addr[j] = -1;
		}
		fseek(fp, Getdb_num(curDatablock), SEEK_SET);
		fwrite(&addr, sizeof(addr), 1, fp);
		PushFreedb(fp, inode.addr2[i]);
		inode.addr2[i] = -1;
	}
	wInode(fp, inode, iInode);
}
//写入数据
void WData(FILE *fp, int pInode, void *vData, int size) {
	CleanData(fp, pInode);
	char *data = (char *)vData;
	//读取pInode指向的inode块数据
	struct Inode inode = GetInode(fp, pInode);
	inode.size = size;
	int i;
	for (i = 0; i < Add0Num; i++) {
		if (size == 0) break;
		int writeSize = Min(size, DBlockSize);
		int pDataBlock = GetFreedb(fp);
		inode.addr0[i] = pDataBlock;
		fseek(fp, Getdb_num(pDataBlock), SEEK_SET);
		fwrite(data, writeSize, 1, fp);
		data += writeSize;
		size -= writeSize;
	}
	for (i = 0; i < Add1Num; ++i) {
		if (size == 0) break;
		int pDataBlock = GetFreedb(fp);
		inode.addr1[i] = pDataBlock;
		int addr[128];
		for (int j = 0; j < NADDRS; ++j) {
			addr[j] = -1;
		}
		for (int j = 0; j < NADDRS; ++j) {
			if (size == 0) break;
			int writeSize = Min(size, DBlockSize);
			int pDataBlock1 = GetFreedb(fp);
			addr[j] = pDataBlock1;
			fseek(fp, Getdb_num(pDataBlock1), SEEK_SET);
			fwrite(data, writeSize, 1, fp);
			data += writeSize;
			size -= writeSize;
		}
		//printf("%d %d\n", pDataBlock, addr[0]);
		fseek(fp, Getdb_num(pDataBlock), SEEK_SET);
		fwrite(&addr, sizeof(addr), 1, fp);
	}
	for (i = 0; i < Add2Num; ++i) {
		if (size == 0) break;
		int pDataBlock = GetFreedb(fp);
		inode.addr2[i] = pDataBlock;
		int addr[128];
		for (int j = 0; j < NADDRS; ++j) {
			addr[j] = -1;
		}
		for (int j = 0; j < NADDRS; ++j) {
			if (size == 0) break;
			int pDataBlock1 = GetFreedb(fp);
			addr[j] = pDataBlock1;
			int addr1[128];
			for (int k = 0; k < NADDRS; ++k) {
				addr1[k] = -1;
			}
			for (int k = 0; k < NADDRS; ++k) {
				if (size == 0) break;
				int writeSize = Min(size, DBlockSize);
				int pDataBlock2 = GetFreedb(fp);
				addr1[k] = pDataBlock2;
				fseek(fp, Getdb_num(pDataBlock2), SEEK_SET);
				fwrite(data, writeSize, 1, fp);
				data += writeSize;
				size -= writeSize;
			}
			fseek(fp, Getdb_num(pDataBlock1), SEEK_SET);
			fwrite(&addr1, sizeof(addr), 1, fp);
		}
		fseek(fp, Getdb_num(pDataBlock), SEEK_SET);
		fwrite(&addr, sizeof(addr), 1, fp);
	}
	time_t t = time(0);
	strftime(inode.time, sizeof(inode.time),"%Y/%m/%d %X %a 本年度第%j天 时区:%z", localtime(&t));
	wInode(fp, inode, pInode);
}
//根据文件路径获取inode
int GetInodeNum(FILE *fp, int curInode, const char filePath[]) {
	//printf("curInode = %d, filePath = %s\n", curInode, filePath);
	int i;
	if (strcmp(filePath, "") == 0) //当前路径
		return curInode;
	if (curInode == -1) { //绝对路径
		char s1[FileNum];
		char s2[FileNum];
		SplitPath(const_cast<char*>(filePath), s1, s2);
		if (strcmp(s1, "/") != 0) {//路径错误，绝对路径一定以/开头
			printf("文件路径错误，未找到该文件...!\n");
			return -1;
		}

		struct Superb superblock = Getsb(fp);
		curInode = superblock.ptrRoot;
		//递归，从根目录开始寻找,即第一个inde
		return GetInodeNum(fp, curInode, s2);
	}
	else { //相对路径
		char s1[FileNum];
		char s2[FileNum];
		SplitPath(const_cast<char*>(filePath), s1, s2);
		//获得当前路径目录的inode
		struct Inode inode = GetInode(fp, curInode);

		char *data = (char *)malloc(inode.size);
		RData(fp, curInode, data, inode.size);
		//DIR{文件名；inode编号}
		//将当前节点内的所有内容形成链表 dir->{}->...->{}->{}->{}
		struct DIR *dir = (struct DIR *)data;
		//一个inode最多存放的文件个数
		int nDir = inode.size / sizeof(struct DIR);
		//eg. desktop/-->desktop
		if (s1[strlen(s1) - 1] == '/')
			s1[strlen(s1) - 1] = '\0';

		curInode = -1;
		for (int i = 0; i < nDir; i++) {
			if (strcmp(s1, dir[i].dirName) == 0) {
				curInode = dir[i].ptrInode;
				break;
			}
		}

		if (curInode == -1) {
			printf("路径错误!\n");
			return -1;
		}
		//递归查找
		return GetInodeNum(fp, curInode, s2);
	}
}
//不带错误提醒的获取inode函数
int GetInodeNum2(FILE *fp, int curInode, char filePath[]) {
	int i;
	if (strcmp(filePath, "") == 0) return curInode;
	if (curInode == -1) { //绝对路径
		char s1[FileNum];
		char s2[FileNum];
		SplitPath(filePath, s1, s2);
		if (strcmp(s1, "/") != 0) {
			return -1;
		}

		struct Superb superblock = Getsb(fp);
		curInode = superblock.ptrRoot;

		return GetInodeNum(fp, curInode, s2);
	}
	else { //相对路径
		char s1[FileNum];
		char s2[FileNum];
		SplitPath(filePath, s1, s2);

		struct Inode inode = GetInode(fp, curInode);

		char *data = (char *)malloc(inode.size);
		RData(fp, curInode, data, inode.size);

		struct DIR *dir = (struct DIR *)data;
		int nDir = inode.size / sizeof(struct DIR);

		if (s1[strlen(s1) - 1] == '/')
			s1[strlen(s1) - 1] = '\0';

		curInode = -1;
		for (i = 0; i < nDir; i++) {
			if (strcmp(s1, dir[i].dirName) == 0) {
				curInode = dir[i].ptrInode;
				break;
			}
		}

		if (curInode == -1) {
			return -1;
		}

		return GetInodeNum(fp, curInode, s2);
	}
}
//创建目录文件
int newDir(FILE *fp, int curInode, const char dirName[], const char username[], const char group[]) {
	//创建根目录
	if (strcmp(dirName, "/") == 0) {
		struct Superb superblock = Getsb(fp);
		if (superblock.ptrRoot != -1) {
			printf("根目录已存在！！\n");
			return -1;
		}

		struct DIR *dir = (struct DIR *)malloc(sizeof(struct DIR));
		strcpy(dir->dirName, ".");
		dir->ptrInode = GetFreeInode(fp);//分配空inode

		int iInode = dir->ptrInode;
		struct Inode inode = GetInode(fp, iInode);//对新的inode进行赋值
		strcpy(inode.name, "/");
		inode.size = sizeof(struct DIR);
		inode.linknum = 2;
		inode.type = 0;
		strcpy(inode.owner, username);
		strcpy(inode.group, group);
		inode.permission = 0666;
		time_t t = time(0);
		strftime(inode.time, sizeof(inode.time), "%Y/%m/%d %X %a 本年度第%j天 时区:%z", localtime(&t));
		//将地址全部初始化为-1
		memset(inode.addr0, -1, sizeof(inode.addr0));
		memset(inode.addr1, -1, sizeof(inode.addr1));
		memset(inode.addr2, -1, sizeof(inode.addr2));
		//向fp写入inode
		wInode(fp, inode, iInode);
		//向inode写入dir
		WData(fp, iInode, dir, sizeof(struct DIR));

		//showInode(inode);
		superblock = Getsb(fp);
		superblock.ptrRoot = iInode;

		//写入超级块
		W_Sblock(fp, superblock);
		//返回创建文件的inode编号
		return dir->ptrInode;
	}

	if (curInode == -1) {//绝对路径
		//判断是否存在该文件夹
		if (GetInodeNum(fp, -1, const_cast<char*>(dirName)) != -1) {
			printf("该文件目录已存在!\n");
			return -1;
		}
		char s1[FileNum];
		char s2[FileNum];
		split1(const_cast<char*>(dirName), s1, s2);

		int iInode = GetInodeNum(fp, -1, s1);
		if (iInode == -1) {
			printf("路径错误!\n");
			return -1;
		}
		//递归一直到当前目录，进入相对路径else
		return newDir(fp, iInode, s2, username, group);
	}
	else { //相对路径
		char tmp[FileNum];
		strcpy(tmp, dirName);
		tmp[strlen(tmp) - 1] = '\0';

		//获取空的inode
		int iNewInode = GetFreeInode(fp);
		struct DIR dir[2];
		strcpy(dir[0].dirName, ".");//当前目录
		dir[0].ptrInode = iNewInode;
		strcpy(dir[1].dirName, "..");//上级目录
		dir[1].ptrInode = curInode;
		//inode赋值
		struct Inode inodeB;
		strcpy(inodeB.name, tmp);
		inodeB.size = 2 * sizeof(struct DIR);
		inodeB.linknum = 2;
		inodeB.type = 0;
		strcpy(inodeB.owner, username);
		strcpy(inodeB.group, group);
		inodeB.permission = 0777;
		time_t t = time(0);
		strftime(inodeB.time, sizeof(inodeB.time), "%Y/%m/%d %X %a 本年度第%j天 时区:%z", localtime(&t));
		memset(inodeB.addr0, -1, sizeof(inodeB.addr0));
		memset(inodeB.addr1, -1, sizeof(inodeB.addr1));
		memset(inodeB.addr2, -1, sizeof(inodeB.addr2));

		wInode(fp, inodeB, iNewInode);
		WData(fp, iNewInode, dir, 2 * sizeof(struct DIR));

		struct DIR dirData;
		strcpy(dirData.dirName, tmp);
		dirData.ptrInode = iNewInode;

		//获取当前目录的数据
		struct Inode inodeA = GetInode(fp, curInode);

		char *data = (char *)malloc(inodeA.size + sizeof(struct DIR));
		RData(fp, curInode, data, inodeA.size);
		//将新的inode连接到当前的iNode下面
		memcpy(data + inodeA.size, &dirData, sizeof(struct DIR));
		//跟新当前inode数据
		CleanData(fp, curInode);
		WData(fp, curInode, data, inodeA.size + sizeof(struct DIR));

		inodeA = GetInode(fp, curInode);
		inodeA.linknum++;
		wInode(fp, inodeA, curInode);

		return iNewInode;
	}
}
//显示文件目录
void showDir(FILE* fp, int curInode) {
	struct Inode inode = GetInode(fp, curInode);

	if (inode.type != 0) {
		printf("这不是一个文件夹!\n");
		return;
	}
	//读数据，以dir形式分割
	char *data = (char *)malloc(inode.size);
	RData(fp, curInode, data, inode.size);
	struct DIR *dirArray = (struct DIR *)data;
	int nDir = inode.size / sizeof(struct DIR);
	for (int i = 0; i < nDir; i++) {
		printf("%s ", dirArray[i].dirName);
	}
	printf("\n");
	return;
}

