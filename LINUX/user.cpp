#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "user.h"
#include "file.h"
#include "inode.h"

//创建用户 返回用户类
struct User CreateUser(const char username[], const char password[], const char group[]) {
	struct User user;
	strcpy(user.username, username);
	strcpy(user.password, password);
	strcpy(user.group, group);
	return user;
}
//判断密码是否合法
int LegalPsd(char password[]) {
	int i;
	if (strlen(password) >= psdLen) {
		printf("密码长度小于%d位\n", psdLen);
		return 0;
	}
	for (i = 0; password[i] != '\0'; i++) {
		char ch = password[i];
		if (!(32 <= ch && ch <= 126)) {
			printf("密码包含非法字符!\n");
			return 0;
		}
	}
	return 1;
}
//判断用户名是否合法
int LegalUsername(char username[]) {
	int i;
	if (strlen(username) >= UserNameLen) {
		printf("用户名长度小于%d位\n", UserNameLen);
		return 0;
	}
	for (i = 0; username[i] != '\0'; i++) {
		char ch = username[i];
		if (!('a' <= ch && ch <= 'z' || 'A' <= ch && ch <= 'Z' || '0' <= ch && ch <= '9')) {
			printf("用户名只能包含数字与字母!\n");
			return 0;
		}
	}
	return 1;
}

//获取用户所在组
char *GetGrp(FILE *fp, char username[]) {
	//获取存放用户信息的inode
	int iInode = GetInodeNum(fp, -1, "/sys/userinfo");
	struct Inode inode = GetInode(fp, iInode);
	char *data = (char *)malloc(inode.size);
	RData(fp, iInode, data, inode.size);

	struct User *userArray = (struct User *)data;
	int nUser = inode.size / sizeof(struct User);

	int i;
	for (i = 0; i < nUser; i++) {
		if (strcmp(userArray[i].username, username) == 0) {
			char *ans = (char *)malloc(GroupLen);
			strcpy(ans, userArray[i].group);
			return ans;
		}
	}
	return NULL;
}

//判断用户名密码是否正确
int login(FILE *fp, char username[], char password[]) {
	int iInode = GetInodeNum(fp, -1, "/sys/userinfo");
	struct Inode inode = GetInode(fp, iInode);
	char *data = (char *)malloc(inode.size);
	RData(fp, iInode, data, inode.size);

	struct User *userArray = (struct User *)data;
	int nUser = inode.size / sizeof(struct User);

	int i;
	for (i = 0; i < nUser; i++) {
		if (strcmp(userArray[i].username, username) == 0) {
			if (strcmp(userArray[i].password, password) == 0) return 1;
			else return 0;
		}
	}

	return 0;
}

//修改密码
void chPsd(FILE *fp, char username[], char password[]) {
	int iInode = GetInodeNum(fp, -1, "/sys/userinfo");
	struct Inode inode = GetInode(fp, iInode);
	char *data = (char *)malloc(inode.size);
	RData(fp, iInode, data, inode.size);

	struct User *userArray = (struct User *)data;
	int nUser = inode.size / sizeof(struct User);

	int i;
	for (i = 0; i < nUser; i++) {
		if (strcmp(userArray[i].username, username) == 0) {
			strcpy(userArray[i].password, password);
			break;
		}
	}
	WData(fp, iInode, userArray, inode.size);
}
//判断用户是否存在
int usrhad(FILE *fp, char username[]) {
	int iInode = GetInodeNum(fp, -1, "/sys/userinfo");
	struct Inode inode = GetInode(fp, iInode);
	char *data = (char *)malloc(inode.size);
	RData(fp, iInode, data, inode.size);

	struct User *userArray = (struct User *)data;
	int nUser = inode.size / sizeof(struct User);

	int i;
	for (i = 0; i < nUser; i++) {
		//匹配到，存在该用户
		if (strcmp(userArray[i].username, username) == 0) {
			return 1;
		}
	}
	return 0;
}