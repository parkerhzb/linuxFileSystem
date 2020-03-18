#ifndef DBLOCK_H
#define DBLOCK_H
#include <stdio.h>

#define DBlockSize 512//数据块大小
#define DBlockNum 1000//数据块数量

int Getdb_num(int index);//获取index数据块的地址
int GetFreedb(FILE *fp);//获取空闲数据块
void PushFreedb(FILE *fp, int index);//释放数据，新增空闲块

#endif