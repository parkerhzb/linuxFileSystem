#ifndef FILE_H
#define FILE_H

#include <stdio.h>
#include <Windows.h>

#define FileNum 256//文件数
#define FileNameLen 16//合法文件名长度

struct DIR {
	char dirName[FileNameLen];//文件名
	int ptrInode;//inode
};
int Min(int a, int b);//取最小值
//从第一个'/'处分割字符串;分割为s=s1 s2 （s1包含'/'）
//例如：abc/def/g --> abc/ 和 def/g
int SplitPath(char *s, char *s1, char *s2);
//分割最后一级目录
//例：/home/user/xxx/ 分割为 /home/user/ 和 xxx/
void split1(char *s, char *s1, char *s2);
void split2(char *s, char *s1, char *s2);//分割文件名与目录
int W_power(FILE *fp, int curInode, char username[]);//判断写权限
int R_power(FILE *fp, int curInode, char username[]);//判断读权限
int LegalFilename(char filename[]);//判断文件名是否合法
int GetInodeNum(FILE *fp, int curInode,const char filePath[]);//根据文件路径获取inode
int GetInodeNum2(FILE *fp, int curInode, char filePath[]);
//创建目录文件、文件夹
int newDir(FILE *fp, int curInode,const char dirName[], const char username[], const char group[]);
void showDir(FILE* fp, int curInode);//显示文件目录
//创建文件
int newFile(FILE *fp, int curInode, const char fileName[], const char username[], const char group[]);
void WFile(FILE *fp, int curInode, void *data, int size,const char username[]);//写文件
void showFile(FILE *fp, int curInode, char username[]);//输出文件内容
void showInodePath(FILE *fp, int curInode);//输出inode的文件路径
void AddLink(FILE *fp, int iInode, int x);//增加文件链接数
int DelInode(FILE *fp, int iInode);//删除inode节点
int EmptyDir(FILE *fp, int iInode);//判断目录是否为空
void RData(FILE *fp, int pInode, char *data, int size);//读取数据
void CleanData(FILE *fp, int iInode);//清理数据
void WData(FILE *fp, int pInode, void *vData, int size);//写入数据

#endif