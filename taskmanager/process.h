/*************************************
*文件名:process.h
*说明:任务管理器获取进程信息头文件
*************************************/
#ifndef _PROCESS_H_
#define _PROCESS_H_

#include <Windows.h>
#include <psapi.h>
#include <TlHelp32.h>
#include "process.h"

#define ReserveSize 1024//定义初始化大小


/*****************结构体定义*******************/
// 进程信息
typedef struct __tagProcessInfo {
	PROCESSENTRY32 pe32;
	PROCESS_MEMORY_COUNTERS pmc;
} ProcessInfo, *PProcessInfo;
// 进程信息表
typedef struct __tagProcessList {
	DWORD cProcess;
	PProcessInfo processInfo;
} ProcessList, *PProcessList;
// 全局静态进程信息表
static PProcessList pl;

// 线程信息表
typedef struct __tagThreadList {
	DWORD cThread;
	THREADENTRY32 *thread;
} ThreadList, *PThreadList;
// 全局静态线程信息表
static PThreadList pTL;

// 进程页表
typedef struct _tagProcessPTE{
	// 区域信息
	PVOID  pvRgnBaseAddress; // 虚拟空间区域的起始地址
	DWORD  dwRgnProtection;  // 该区域指定的保护属性，如PAGE_READWRITE
	SIZE_T RgnSize;			 // 所预定区域的大小，以字节为单位
	DWORD  dwRgnStorage;     // 用于区域中各块物理内存的类型，比如MEM_Free/Image/Mapped/Private
	DWORD  dwRgnBlocks;		 // 区域中块的数量
	DWORD  dwRgnGuardBlks;   // 区域中有PAGE_GUARD保护属性标志的块的数量，通常是0或1，如果是1，则说明该区域是为了线程栈而预定
	BOOL   bRgnIsAStack;     // 区域中是否有线程栈

	// 块信息
	PVOID  pvBlkBaseAddress; // 块的起始地址
	DWORD  dwBlkProtection;  // 该块指定的保护属性，如PAGE_READWRITE
	SIZE_T BlkSize;			 // 块的大小
	DWORD  dwBlkStorage;     // 块物理内存的类型，比如MEM_Free/Image/Mapped/Private
} VMQUERY, *PVMQUERY;

// 辅助结构
typedef struct {
	SIZE_T RgnSize;			 // 记录区域大小之和
	DWORD  dwRgnStorage;     // 内存类型，如MEM_FREE等
	DWORD  dwRgnBlocks;		 // 区域中的块数
	DWORD  dwRgnGuardBlks;   // 大于0说明区域有线程栈
	BOOL   bRgnIsAStack;     // 为真说明区域有线程栈
} VMQUERY_HELP;


/*****************函数声明*******************/
// 获得进程列表
BOOL GetProcessList(PProcessList pPL);
// 清理进程列表
void FreeProcessList(PProcessList pPL);
// 获得系统内存信息
BOOL GetSysMemoryInfo();
// 根据PID获得其线程信息
BOOL GetProcessThreads(DWORD dwPID, PThreadList pTL);
// 清理线程列表
void FreeListProcessThreads(PThreadList pTL);
// 获得进程页表
BOOL GetProcessPTT(DWORD dwPId);
/*关于PTT的获取，见《Windows核心编程》第五版第13章*/
// 遍历获取内存区域信息
BOOL VMQuery(HANDLE hProcess, LPCVOID pvAddress, PVMQUERY pVMQ);
// 迭代进一步获取信息
static BOOL VMQueryHelp(HANDLE hProcess, LPCVOID pvAddress, VMQUERY_HELP *pVMQHelp);
// 根据数据构造内存区域信息输出
void ConstructRgnInfoLine(HANDLE hProcess, PVMQUERY pVMQ, PTSTR szLine, int cchMaxLen);
// 将保护属性转为文字
char* GetProtectText(DWORD dwProtect, char* szBuf, size_t chSize, BOOL bShowFlags);
// 将内存区域类型转为文字
char* GetMemStorageText(DWORD dwStorage);
#endif