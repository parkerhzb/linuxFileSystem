#ifndef USER_H
#define USER_H

#include <stdio.h>
#define UserNum 256//用户个数
#define UserNameLen 16//用户名长度
//#define PSDNUM 256//密码个数
#define psdLen 16//密码长度
#define GROUP 256//组个数
#define GroupLen 16//组名长度

struct User {
	char username[UserNameLen]; //用户名
	char password[psdLen]; //密码
	char group[GroupLen]; //用户所在组
};
//创建用户 返回用户类
struct User CreateUser(const char username[], const char password[], const char group[]);
int LegalUsername(char username[]);
int LegalPsd(char password[]);
char *GetGrp(FILE *fp, char username[]);//获取用户所在组
int login(FILE *fp, char username[], char password[]);//判断用户名密码是否正确
int usrhad(FILE *fp, char username[]);//判断用户是否存在
void chPsd(FILE *fp, char username[], char password[]);//修改密码
#define UserSize sizeof(struct User)
#endif
