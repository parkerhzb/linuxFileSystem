#include <stdio.h>
#include <io.h>
#include <cstdio>
#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <Windows.h>
#include <string>

#include "user.h"
#include "file.h"
#include "inode.h"
#include "sblock.h"
using namespace std;
#define CMDLEN 50//指令的长度
#define FILESYSTEM "sys.bin"//系统文件
extern int UMARK;//默认权限
char cmd[CMDLEN];
char maincmd[CMDLEN];
char directoryName[FileNum];
char username[UserNum];
char password[UserNum];
int curDir;//当前目录
void welcome()
{
	cout << '\n' << "\n系统启动中";
	string wel = ">> >> >> ";
	for (int n = 0; n < 10; n++)
	{
		cout << wel;
		Sleep(300);
	}
	Sleep(1000);
	system("cls");
}
int main() {
	FILE *fp;
	fp = fopen(FILESYSTEM, "rb+");
	//判断文件系统是否存在
	if (fp == NULL) {
		printf("系统文件不存在!\n");
		CreateSystem(FILESYSTEM);
		fp = fopen(FILESYSTEM, "rb+");
	}
	welcome();
	//判断用户名，密码是否正确
	while (1) {
		printf("*****欢迎来到HZB的Linux文件管理系统*****\n请输入用户名: ");
		scanf("%s", username);
		printf("请输入密码: ");
		scanf("%s", password);
		if (login(fp, username, password)) {
			printf("\n用户%s登录成功\n", username);
			break;
		}
		else {
			printf("用户名或密码错误!\n");
			Sleep(1000);
			system("cls");
		}
	}
	//加载目录到用户目录
	if (strcmp(username, "root") == 0) {
		curDir = GetInodeNum(fp, -1, "/root/");
	}
	else {
		directoryName[0] = '\0';
		strcat(directoryName, "/home/");
		strcat(directoryName, username);
		strcat(directoryName, "/");
		curDir = GetInodeNum(fp, -1, directoryName);
	}
	while (1) {
		printf("%s@HZBLinux:", username);
		showInodePath(fp, curDir);
		strcmp(username, "root") == 0 ? putchar('#') : putchar('$');
		putchar(' ');
		scanf("%s", maincmd);
		if (strcmp(maincmd, "ls") == 0) {
			char a[CMDLEN];
			gets_s(a);
			if (strlen(a) == 0) {
				showDir(fp, curDir);
			}
			else
			{
				sscanf(a, "%s", a);
				int tem = GetInodeNum(fp, -1, a);
				if (tem != -1)
					showDir(fp, tem);
			}
		}
		else if (strcmp(maincmd, "mkdir") == 0) {
			char a[CMDLEN];
			scanf("%s", a);
			if (LegalFilename(a)) {
				if (GetInodeNum2(fp, curDir, a) == -1) {
					strcat(a, "/");
					newDir(fp, curDir, a, username, GetGrp(fp, username));
				}
				else
				{
					printf("文件夹%s已存在\n", a);
				}
			}
		}
		else if (strcmp(maincmd, "pwd") == 0) {
			showInodePath(fp, curDir);
			printf("\n");
		}
		else if (strcmp(maincmd, "cd") == 0) {
			char a[CMDLEN];
			scanf("%s", a);
			int iInode;
			if (a[0] == '/')
				iInode = GetInodeNum(fp, -1, a);
			else
				iInode = GetInodeNum(fp, curDir, a);
			if (iInode != -1) {
				struct Inode inode = GetInode(fp, iInode);
				if (inode.type == 0) {
					curDir = iInode;
				}
				else {
					printf("这不是一个文件目录!\n");
				}
			}
		}
		else if (strcmp(maincmd, "chmod") == 0) {
			char a1[CMDLEN], a2[CMDLEN];
			scanf("%s %s", a1, a2);
			int pri; //chmod abc file  a,b,c表User、Group、及Other的权限。 r=4，w=2，x=1
			if (sscanf(a1, "%o", &pri) == 1 && 0 <= pri && pri <= 0777) {
				int iInode;

				if (a2[0] == '/') //绝对路径
					iInode = GetInodeNum(fp, -1, a2);
				else //相对路径
					iInode = GetInodeNum(fp, curDir, a2);
				if (iInode != -1) {
					struct Inode inode = GetInode(fp, iInode);
					inode.permission = pri;
					wInode(fp, inode, iInode);
				}
				else {
					printf("未找到该文件!\n");
				}
			}
			else {
				printf("输入权限格式错误，权限为八进制数字[000,777] r=4，w=2，x=1\n");
			}
		}
		else if (strcmp(maincmd, "chgrp") == 0) {
			char a1[CMDLEN], a2[CMDLEN];
			scanf("%s %s", a1, a2);
			int iInode;
			if (a2[0] == '/')
				iInode = GetInodeNum(fp, -1, a2);
			else
				iInode = GetInodeNum(fp, curDir, a2);
			if (iInode != -1) {
				if (W_power(fp, iInode, username)) {
					struct Inode inode = GetInode(fp, iInode);
					strcpy(inode.group, a1);
					wInode(fp, inode, iInode);
				}
				else {
					printf("你没有权限修改该文件!\n");
				}
			}
		}
		else if (strcmp(maincmd, "chown") == 0) {
			char a1[CMDLEN], a2[CMDLEN];
			scanf("%s %s", a1, a2);
			if (usrhad(fp, a1)) {
				int iInode;
				if (a2[0] == '/') iInode = GetInodeNum(fp, -1, a2);
				else iInode = GetInodeNum(fp, curDir, a2);

				if (iInode != -1) {
					if (W_power(fp, iInode, username)) {
						struct Inode inode = GetInode(fp, iInode);
						strcpy(inode.owner, a1);
						wInode(fp, inode, iInode);
					}
					else {
						printf("你没有权限修改该文件!\n");
					}
				}
			}
			else {
				printf("用户%s不存在!\n", a2);
			}
		}
		else if (strcmp(maincmd, "rmdir") == 0) {
			char a[CMDLEN];
			scanf("%s", a);
			if (LegalFilename(a)) {
				int dstInode = GetInodeNum(fp, curDir, a);
				if (dstInode != -1) {
					if (EmptyDir(fp, dstInode)) {
						AddLink(fp, dstInode, -1);
						AddLink(fp, curDir, -1);
						DelInode(fp, dstInode);

						struct Inode inode = GetInode(fp, curDir);
						char *data = (char *)malloc(inode.size);
						RData(fp, curDir, data, inode.size);

						struct DIR *dirArray = (struct DIR *)data;
						int nDir = inode.size / sizeof(struct DIR);
						int i, j;
						for (i = 0; i < nDir; i++) {
							if (strcmp(dirArray[i].dirName, a) == 0) {
								for (j = i; j < nDir - 1; j++) {
									dirArray[j] = dirArray[j + 1];
								}
								break;
							}
						}
						WData(fp, curDir, dirArray, inode.size - sizeof(struct DIR));
					}
				}
			}
			else {
				printf("未找到该目录文件夹!\n");
			}
		}
		else if (strcmp(maincmd, "mv") == 0) {
			char a1[CMDLEN], a2[CMDLEN];
			scanf("%s %s", a1, a2);
			if (LegalFilename(a1) && LegalFilename(a2)) {
				if (a1[strlen(a1) - 1] == '/')
					a1[strlen(a1) - 1] = '\0';
				if (a2[strlen(a2) - 1] == '/')
					a2[strlen(a2) - 1] = '\0';
				int iInode = GetInodeNum(fp, curDir, a1);
				if (iInode == -1) {
					printf("文件%s不存在!\n", a1);
				}
				else {
					int newInode = GetInodeNum2(fp, curDir, a2);
					if (newInode != -1) {
						printf("文件%s已经存在!\n", a2);
					}
					else {
						struct Inode cinode = GetInode(fp, curDir);
						char *data = (char *)malloc(cinode.size);
						RData(fp, curDir, data, cinode.size);

						struct DIR *dirArray = (struct DIR *)data;
						int nDir = cinode.size / sizeof(struct DIR);
						for (int i = 0; i < nDir; i++) {
							if (strcmp(dirArray[i].dirName, a1) == 0) {
								strcpy(dirArray[i].dirName, a2);
								struct Inode modifyInode = GetInode(fp, dirArray[i].ptrInode);
								strcpy(modifyInode.name, a2);
								wInode(fp, modifyInode, dirArray[i].ptrInode);
								break;
							}
						}
						WData(fp, curDir, dirArray, cinode.size);
					}
				}
			}
			else {
				printf("文件名错误!\n");
			}
		}
		else if (strcmp(maincmd, "rm") == 0) {
			char a[CMDLEN];
			scanf("%s", a);
			if (a[strlen(a) - 1] != '\0' && LegalFilename(a)) {
				int dstInode = GetInodeNum(fp, curDir, a);
				if (dstInode != -1) {
					struct Inode tmpInode = GetInode(fp, dstInode);
					if (tmpInode.type != 0) {
						int link_num = tmpInode.linknum;
						CleanData(fp, dstInode);

						AddLink(fp, dstInode, -1);
						DelInode(fp, dstInode);

						struct Inode inode = GetInode(fp, curDir);
						char *data = (char *)malloc(inode.size);
						RData(fp, curDir, data, inode.size);

						struct DIR *dirArray = (struct DIR *)data;
						int nDir = inode.size / sizeof(struct DIR);
						int i, j;
						if (link_num > 1) {//存在链接文件
							int flag = link_num;//连接数
							char name[FileNum][CMDLEN];
							for (i = 0; flag&&i < nDir; ++i) {
								if (GetInodeNum(fp, curDir, dirArray[i].dirName) == dstInode) {
									strcpy(name[link_num - flag], dirArray[i].dirName);
									flag--;
								}
							}
							for (i = 0; i < nDir; i++) {
								if (strcmp(dirArray[i].dirName, name[flag]) == 0) {
									for (j = i; j < nDir - 1; j++) {
										dirArray[j] = dirArray[j + 1];
									}
									i--;
									nDir--;
									flag++;
									if (flag == link_num)
										break;
								}
							}
						}
						else {
							for (i = 0; i < nDir; i++) {
								if (strcmp(dirArray[i].dirName, a) == 0) {
									for (j = i; j < nDir - 1; j++) {
										dirArray[j] = dirArray[j + 1];
									}
									break;
								}
							}
						}


						WData(fp, curDir, dirArray, inode.size - link_num * sizeof(struct DIR));
					}
					else {
						printf("目标为文件夹，不能删除!\n");
					}
				}
			}
			else {
				printf("文件名错误!\n");
			}
		}
		else if (strcmp(maincmd, "cp") == 0) {
			char a1[CMDLEN], a2[CMDLEN];
			scanf("%s %s", a1, a2);
			if (LegalFilename(a2)) {
				if (a1[strlen(a1) - 1] == '/')
					a1[strlen(a1) - 1] = '\0';
				if (a2[strlen(a2) - 1] == '/')
					a2[strlen(a2) - 1] = '\0';
				int iInode = GetInodeNum(fp, curDir, a1);
				if (iInode == -1) {
					printf("文件%s不存在!\n", a1);
				}
				else {
					int newInode = GetInodeNum2(fp, curDir, a2);
					if (newInode != -1) {
						printf("文件%已经存在!\n", a2);
						continue;//目标文件已存在改路径下，则跳出执行
					}
					struct Inode inode = GetInode(fp, iInode);
					if (inode.type == 0) {
						printf("不能对文件夹进行操作!\n");
					}
					else {
						char *data = (char *)alloca(inode.size);
						RData(fp, iInode, data, inode.size);
						int dstInode = newFile(fp, curDir, a2, username, GetGrp(fp, username));
						WData(fp, dstInode, data, inode.size);
					}
				}
			}
		}
		else if (strcmp(maincmd, "ln") == 0) {
			char a1[CMDLEN], a2[CMDLEN];
			scanf("%s %s", a1, a2);
			if (GetInodeNum2(fp, curDir, a2) != -1) {
				printf("文件 '%s' 已存在!\n", a2);
				continue;
			}
			if (a2[strlen(a2) - 1] != '/' && LegalFilename(a2)) {
				int iInode;
				if (a1[0] == '/') //绝对路径
					iInode = GetInodeNum2(fp, -1, a1);
				else//相对路径
					iInode = GetInodeNum2(fp, curDir, a1);
				if (iInode == -1) {
					printf("文件%s不存在!\n", a1);
				}
				else {
					struct Inode inode = GetInode(fp, iInode);
					if (inode.type == 0) {
						printf("这是一个文件夹目录!\n");
					}
					else {
						struct Inode inodeDir = GetInode(fp, curDir);
						char *dirData = (char *)malloc(inodeDir.size + sizeof(struct DIR));
						RData(fp, curDir, dirData, inodeDir.size);

						struct DIR newItem;
						strcpy(newItem.dirName, a2);
						newItem.ptrInode = iInode;

						memcpy(dirData + inodeDir.size, &newItem, sizeof(struct DIR));
						//更新链接文件所在目录
						inodeDir.size += sizeof(struct DIR);
						wInode(fp, inodeDir, curDir);
						WData(fp, curDir, dirData, inodeDir.size);
						AddLink(fp, iInode, 1);
					}
				}
			}
			else {
				printf("不能链接文件夹！\n");
			}
		}
		else if (strcmp(maincmd, "stat") == 0) {
			char a[CMDLEN];
			scanf("%s", a);
			int dstInode;
			if (a[0] == '/')
				dstInode = GetInodeNum(fp, -1, a);
			else
				dstInode = GetInodeNum(fp, curDir, a);
			struct Inode inode = GetInode(fp, dstInode);
			if (inode.type != 0 && inode.type != 1)
			{
				printf("不存在%s文件或目录\n", a);
				continue;
			}
			showInode(inode);
		}
		else if (strcmp(maincmd, "vim") == 0) {
			char a[CMDLEN];
			scanf("%s", a);
			if (a[strlen(a) - 1] != '/' && LegalFilename(a)) {
				int iInode = GetInodeNum2(fp, curDir, a);
				if (iInode == -1) {//文件不存在
					newFile(fp, curDir, a, username, GetGrp(fp, username));
					iInode = GetInodeNum2(fp, curDir, a);
				}
				Inode tem = GetInode(fp, iInode);
				if (tem.type == 0) {
					printf("文件夹不可用vim编辑\n");
					continue;
				}
				//打开并清空文件
				FILE* temfp = fopen("osfile.txt", "w+");
				//读原数据
				char *data = (char *)malloc(tem.size);
				RData(fp, iInode, data, tem.size);
				fwrite(data, 1, tem.size, temfp);
				fclose(temfp);
				system("notepad osfile.txt");

				temfp = fopen("osfile.txt", "rt");
				int length = 0;
				for (; fgetc(temfp) != EOF; length++);
				fclose(temfp);
				//读取修改后的文件
				temfp = fopen("osfile.txt", "r+");
				fread(data, 1, length, temfp);
				fclose(temfp);

				WFile(fp, iInode, data, length, username);
			}
			else {
				printf("命令语法错误，只能操作文件！\n");
			}
		}
		else if (strcmp(maincmd, "touch") == 0) {
			char a[CMDLEN];
			scanf("%s", a);
			if (a[strlen(a) - 1] != '/' && LegalFilename(a)) {
				int iInode = GetInodeNum2(fp, curDir, a);
				if (iInode != -1) {
					printf("文件%s已经存在!\n", a);
				}
				else {
					newFile(fp, curDir, a, username, GetGrp(fp, username));
				}
			}
			else {
				printf("使用语法错误，touch+新建文件名\n");
			}
		}
		else if (strcmp(maincmd, "edit") == 0) {
			char a[CMDLEN];
			scanf("%s", a);
			int iInode;
			if (a[0] == '/') //绝对路径下寻找
				iInode = GetInodeNum(fp, -1, a);
			else//相对路径下寻找
				iInode = GetInodeNum(fp, curDir, a);
			if (iInode != -1) {
				Inode tem = GetInode(fp, iInode);
				if (tem.type == 0) {
					printf("目录、文件夹不可编辑！\n");
					continue;
				}
				//打开并清空文件
				FILE* temfp = fopen("osfile.txt", "w+");
				//读原数据
				char *data = (char *)malloc(tem.size);
				RData(fp, iInode, data, tem.size);
				fwrite(data, 1, tem.size, temfp);
				fclose(temfp);
				system("notepad osfile.txt");//notepad只有txt关闭后才会继续执行以下代码
				//获取文件长度
				temfp = fopen("osfile.txt", "rt");
				int length = 0;
				for (; fgetc(temfp) != EOF; length++);
				fclose(temfp);
				//读取修改后的文件
				temfp = fopen("osfile.txt", "r+");
				char *data1 = (char *)malloc(length);
				fread(data1, 1, length, temfp);
				fclose(temfp);
				WFile(fp, iInode, data1, length, username);
			}
		}
		else if (strcmp(maincmd, "cat") == 0) {
			char a[CMDLEN];
			scanf("%s", a);
			int iInode;
			if (a[0] == '/')
				iInode = GetInodeNum(fp, -1, a);
			else
				iInode = GetInodeNum(fp, curDir, a);
			if (iInode != -1) {
				struct Inode inode = GetInode(fp, iInode);
				if (inode.type == 1) {
					showFile(fp, iInode, username);
				}
				else {
					printf("%s文件夹，这不是一个可读文件\n", a);
				}
			}
		}
		else if (strcmp(maincmd, "passwd") == 0) {
			char passwd[UserNum];
			getchar();
			printf("请输入原密码: ");
			gets_s(passwd);
			while (!login(fp, username, passwd)) {
				printf("密码错误！\n");
				printf("请输入原密码: ");
				gets_s(passwd);
			}

			printf("请输入新密码: ");
			gets_s(passwd);
			while (!LegalPsd(passwd)) {
				printf("密码格式错误，请重新输入新密码: ");
				gets_s(passwd);
			}
			chPsd(fp, username, passwd);
		}
		else if (strcmp(maincmd, "clean") == 0) {
			system("cls");
		}
		else if (strcmp(maincmd, "shutdown") == 0) {
			printf("正在关机 ... ... ... ...");
			Sleep(500);
			break;
		}
		else if (strcmp(maincmd, "umask") == 0) {
			char a[CMDLEN];
			gets_s(a);
			//umask
			if (strlen(a) == 0) {
				printf("%04o\n", UMARK);
			}
			//umask number
			else {
				sscanf(a, "%o", &UMARK);
			}
		}
		else if (strcmp(maincmd, "user") == 0) {
			int iInode = GetInodeNum(fp, -1, "/sys/userinfo");
			struct Inode inode = GetInode(fp, iInode);
			char *data = (char *)malloc(inode.size);
			RData(fp, iInode, data, inode.size);

			struct User *userArray = (struct User *)data;
			int nUser = inode.size / sizeof(struct User);

			int i;
			for (i = 0; i < nUser; i++) {
				printf("%s  ", userArray[i].username);
			}
			cout << endl;
		}
		else if (strcmp(maincmd, "adduser") == 0) {
			if (strcmp(username, "root") == 0) {
				cout << "请输入用户名：";
				char name[UserNameLen]; scanf("%s", &name);
				cout << "\n请输入密码：";
				char psd[psdLen]; scanf("%s", &psd);
				cout << "\n请输入组名：";
				char grp[GroupLen]; scanf("%s", &grp);
				if (LegalUsername(name) && LegalPsd(psd)) {
					int iInode = GetInodeNum(fp, -1, "/sys/userinfo");
					struct Inode inode = GetInode(fp, iInode);
					char *data = (char *)malloc(inode.size);
					RData(fp, iInode, data, inode.size);

					struct User *userArray = (struct User *)data;
					int nUser = inode.size / sizeof(struct User);

					struct User newuser;
					newuser.username[UserNameLen] = name[UserNameLen];
					newuser.password[psdLen] = psd[psdLen];
					newuser.group[GroupLen] = grp[GroupLen];
					memcpy(userArray + nUser, &newuser, sizeof(struct User));
					//清除旧的信息
					CleanData(fp, iInode);
					//更新新的信息
					WData(fp, iInode, data, inode.size + sizeof(struct User));
				}
			}
			else
			{
				printf("没有权限添加用户\n");
			}
		}
		else if (strcmp(maincmd, "sbinfo") == 0) {
			showSb(fp);
		}
		else if (strcmp(maincmd, "help") == 0) {
			printf("1.	ls  (path)                    **显示文件目录\n");
			printf("2.	chmod xxx file                  改变文件权限\n");
			printf("3.	chown owner file                改变文件拥有者\n");
			printf("4.	chgrp group file                改变文件所属组\n");
			printf("5.	pwd                             显示当前目录\n");
			printf("6.	cd path                         改变当前目录\n");
			printf("7.	mkdir dir                       创建子目录\n");
			printf("8.	rmdir dir                       删除子目录\n");
			printf("9.	vim filename                  **操作文件\n");
			printf("10.	touch filename                **新建文件\n");
			printf("11.	edit filename                 **修改文件内容\n");
			printf("12.	stat file                     **显示文件的状态信息\n");
			printf("13.	mv Filename1 Filename2          改变文件名\n");
			printf("14.	cp file1 file2                  拷贝文件\n");
			printf("15.	rm file                         删除文件\n");
			printf("16.	ln file filename                建立文件联接到当前目录\n");
			printf("17.	cat file                        显示文件内容\n");
			printf("18.	umask                         **显示创建文件时的默认屏蔽码\n");
			printf("19.	umask num                     **设置创建文件时的默认屏蔽码\n");
			printf("20.	passwd                          修改用户口令\n");
			printf("21.	user                          **查看所有用户\n");
			printf("22.	adduser                        *新增用户\n");
			printf("23.	clean                         **清屏\n");
			printf("24.	shoutdown                       关机\n");
		}
		else {
			printf("没有定义的命令,未找到该指令!\n");
		}
	}
	fclose(fp);
}