/*************************************
*文件名:process.cpp
*说明:任务管理器获取进程信息源文件
*************************************/
#include <stdio.h>
#include "process.h"

// 链接库文件
#pragma comment (lib,"psapi.lib")

// 全局静态变量，记录当前CPU的预保留地址空间区域的分配粒度
// 会在第一次调用VMQuery函数时被初始化
static DWORD gs_dwAllocGran = 0;

BOOL GetProcessList(PProcessList pPL)
{
	HANDLE hProcessSnap;
	HANDLE hProcess;
	DWORD dwPriorityClass;

	// 初始化
	pPL->cProcess = 0;
	pPL->processInfo = new ProcessInfo[ReserveSize];
	ZeroMemory(pPL->processInfo, ReserveSize * sizeof(ProcessInfo));//内存清零

	// 先得到系统快照
	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hProcessSnap == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}

	// 必先初始化该结构体的dwSize
	pPL->processInfo[0].pe32.dwSize = sizeof(PROCESSENTRY32);

	// 先尝试获取第一个进程信息，失败则退出
	if (!Process32First(hProcessSnap, &pPL->processInfo[0].pe32))
	{
		CloseHandle(hProcessSnap);
		return FALSE;
	}

	int i = 0;
	do
	{
		pPL->processInfo[i].pmc.cb = sizeof(PROCESS_MEMORY_COUNTERS);

		// 获取优先级
		dwPriorityClass = 0;
		hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pPL->processInfo[i].pe32.th32ProcessID);
		// 获得进程使用内存情况
		if (hProcess)
		{
			GetProcessMemoryInfo(hProcess, &pPL->processInfo[i].pmc,
				sizeof(PROCESS_MEMORY_COUNTERS));
			CloseHandle(hProcess);
		}
		printf("%d\t%s", pPL->processInfo[i].pe32.th32ProcessID, pPL->processInfo[i].pe32.szExeFile);
		/*if (strlen(pPL->processInfo[i].pe32.szExeFile)<8)
		{
			printf("\t");
		}*/
		// 为了排版美观
		for (int j = strlen(pPL->processInfo[i].pe32.szExeFile) / 8; j < 4; j++)
		{
			printf("\t");
		}
		if ((strlen(pPL->processInfo[i].pe32.szExeFile) - 1) / 8 > 3)
		{
			printf("\t");
		}
		printf("%d\t\t%d\t%d\n", pPL->processInfo[i].pe32.th32ParentProcessID, pPL->processInfo[i].pe32.cntThreads, pPL->processInfo[i].pe32.pcPriClassBase);
		//printf("%d\t%s\t%d\t\t%d\n", pPL->processInfo[i].pe32.th32ProcessID, pPL->processInfo[i].pe32.szExeFile, pPL->processInfo[i].pe32.th32ParentProcessID, pPL->processInfo[i].pe32.cntThreads);
		//在下次循环前必先初始化该结构体的dwSize
		pPL->processInfo[++i].pe32.dwSize = sizeof(PROCESSENTRY32);
	} while (Process32Next(hProcessSnap, &pPL->processInfo[i].pe32));//获得下一个进程信息

	// 保存进程数量
	pPL->cProcess = i;

	CloseHandle(hProcessSnap);
	return TRUE;
}

void FreeProcessList(PProcessList pPL)
{
	delete pPL->processInfo;
	delete pPL;
}

BOOL GetProcessThreads(DWORD dwPID, PThreadList pTL)
{
	//初始化
	pTL->cThread = 0;
	pTL->thread = new THREADENTRY32[ReserveSize];
	ZeroMemory(pTL->thread, ReserveSize * sizeof(THREADENTRY32));

	HANDLE hThreadSnap = INVALID_HANDLE_VALUE;
	THREADENTRY32 te32;

	// 先得到系统快照
	hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
	if (hThreadSnap == INVALID_HANDLE_VALUE)
		return(FALSE);

	te32.dwSize = sizeof(THREADENTRY32);

	// 先尝试获取第一个线程信息，失败则退出
	if (!Thread32First(hThreadSnap, &te32))
	{
		CloseHandle(hThreadSnap);
		return FALSE;
	}

	int i = 0;
	do
	{
		//找到这个PID
		if (te32.th32OwnerProcessID == dwPID)
		{
			memcpy(&pTL->thread[i++], &te32, sizeof(THREADENTRY32));
			printf("%d\t\t%d\n", te32.th32ThreadID, te32.tpBasePri);
		}
	} while (Thread32Next(hThreadSnap, &te32));

	// 保存线程数量
	pTL->cThread = i;

	CloseHandle(hThreadSnap);
	return TRUE;
}

void FreeListProcessThreads(PThreadList pTL)
{
	delete pTL->thread;
	delete pTL;
}

BOOL GetSysMemoryInfo()
{
	MEMORYSTATUS ms;
	GlobalMemoryStatus(&ms);
	printf("\nMemory Usage\n");
	printf("Proportion:%d%%\n", ms.dwMemoryLoad);
	printf("Phys Total:%ld MB\n", ms.dwTotalPhys / 1048576);
	//printf("Used:%ld MB\n", (ms.dwTotalPhys - ms.dwAvailPhys) / 1048576);
	printf("Phys Available:%ld MB\n", ms.dwAvailPhys / 1048576);
	//此参数不正常，故注释
	//printf("Virtual Available:%ld MB\n", ms.dwAvailVirtual);
	printf("Page File Total:%ld MB\n", ms.dwTotalPageFile / 1048576);
	printf("Page File Available:%ld MB\n", ms.dwAvailPageFile / 1048576);
	return TRUE;
}

char* GetMemStorageText(DWORD dwStorage)
{
	char *p = new char[20];
	//根据值构造字符串
	switch (dwStorage)
	{
	case MEM_FREE:    strcpy(p, "Free   "); break;
	case MEM_RESERVE: strcpy(p, "Reserve"); break;
	case MEM_IMAGE:   strcpy(p, "Image  "); break;
	case MEM_MAPPED:  strcpy(p, "Mapped "); break;
	case MEM_PRIVATE: strcpy(p, "Private"); break;
	}
	return(p);
}

char* GetProtectText(DWORD dwProtect, char* szBuf, size_t chSize, BOOL bShowFlags)
{
	char* p = new char[20];
	//先去除三个特殊的保护属性标志，后面再判断
	switch (dwProtect & ~(PAGE_GUARD | PAGE_NOCACHE | PAGE_WRITECOMBINE))
	{
	case PAGE_READONLY:          strcpy(p, "-R--"); break;
	case PAGE_READWRITE:         strcpy(p, "-RW-"); break;
	case PAGE_WRITECOPY:         strcpy(p, "-RWC"); break;
	case PAGE_EXECUTE:           strcpy(p, "E---"); break;
	case PAGE_EXECUTE_READ:      strcpy(p, "ER--"); break;
	case PAGE_EXECUTE_READWRITE: strcpy(p, "ERW-"); break;
	case PAGE_EXECUTE_WRITECOPY: strcpy(p, "ERWC"); break;
	case PAGE_NOACCESS:          strcpy(p, "----"); break;
	}
	strcpy(szBuf, p);
	//添加三个特殊保护属性标志
	if (bShowFlags) {
		strcat(szBuf, " ");
		strcat(szBuf, (dwProtect & PAGE_GUARD)
			? TEXT("G") : "-");
		strcat(szBuf, (dwProtect & PAGE_NOCACHE)
			? TEXT("N") : "-");
		strcat(szBuf, (dwProtect & PAGE_WRITECOMBINE)
			? TEXT("W") : "-");
	}
	return(szBuf);
}

void ConstructRgnInfoLine(HANDLE hProcess, PVMQUERY pVMQ, PTSTR szLine, int cchMaxLen)
{
	sprintf(szLine, "%p\t%s\t%12u\t", pVMQ->pvRgnBaseAddress, GetMemStorageText(pVMQ->dwRgnStorage), pVMQ->RgnSize);
	/*
	//暂时不用输出内存类型，代码实际无bug
	if (pVMQ->dwRgnStorage != MEM_FREE) {
		sprintf(strchr(szLine, 0), "%u\t", pVMQ->dwRgnBlocks);
		GetProtectText(pVMQ->dwRgnProtection, strchr(szLine, 0),
			cchMaxLen - strlen(szLine), FALSE);
	}
	else {
		strcat(szLine, "\t");
	}*/
	strcat(szLine, "\t");
}

static BOOL VMQueryHelp(HANDLE hProcess, LPCVOID pvAddress,	VMQUERY_HELP *pVMQHelp)
{
	// 初始化
	ZeroMemory(pVMQHelp, sizeof(*pVMQHelp));
	// 根据传入地址获取此区域内存信息
	MEMORY_BASIC_INFORMATION mbi;
	BOOL bOk = (VirtualQueryEx(hProcess, pvAddress, &mbi, sizeof(mbi))
		== sizeof(mbi));
	if (!bOk)
		return(bOk);   // 此地址无法获取信息
	 // 记录区域基地址
	PVOID pvRgnBaseAddress = mbi.AllocationBase;
	// 用于遍历区域中所有的块
	PVOID pvAddressBlk = pvRgnBaseAddress;
	// 记录内存类型
	pVMQHelp->dwRgnStorage = mbi.Type;

	for (;;) {
		// 获取此块信息
		bOk = (VirtualQueryEx(hProcess, pvAddressBlk, &mbi, sizeof(mbi))
			== sizeof(mbi));
		// 无法获取
		if (!bOk)
			break;

		 // 判断此块是否属于同一区域，否则遍历结束
		if (mbi.AllocationBase != pvRgnBaseAddress)
			break;

		pVMQHelp->dwRgnBlocks++;             // 此区域记录增加一块
		pVMQHelp->RgnSize += mbi.RegionSize; // 此区域记录大小增加

		// 如果有PAGE_GUARD属性，这个值+1
		if ((mbi.Protect & PAGE_GUARD) == PAGE_GUARD)
			pVMQHelp->dwRgnGuardBlks++;

		if (pVMQHelp->dwRgnStorage == MEM_PRIVATE)
			pVMQHelp->dwRgnStorage = mbi.Type;

		// 获得下一块地址
		pvAddressBlk = (PVOID)((PBYTE)pvAddressBlk + mbi.RegionSize);
	}
	// 记录此区域是否是线程栈
	pVMQHelp->bRgnIsAStack = (pVMQHelp->dwRgnGuardBlks > 0);

	return(TRUE);
}

BOOL VMQuery(HANDLE hProcess, LPCVOID pvAddress, PVMQUERY pVMQ)
{
	//在第一次执行时设置分配颗粒大小
	if (gs_dwAllocGran == 0)
	{
		SYSTEM_INFO sinf;
		GetSystemInfo(&sinf);
		gs_dwAllocGran = sinf.dwAllocationGranularity;
	}
	//初始化
	ZeroMemory(pVMQ, sizeof(*pVMQ));

	// 获得MEMORY_BASIC_INFORMATION结构
	MEMORY_BASIC_INFORMATION mbi;
	BOOL bOk = (VirtualQueryEx(hProcess, pvAddress, &mbi, sizeof(mbi))
		== sizeof(mbi));
	if (!bOk)
		return(bOk);   // 地址无效

	// 根据MEMORY_BASIC_INFORMATION结构填充自定义的VMQUERY结构
	// 先填块数据成员
	switch (mbi.State)
	{
	case MEM_FREE:       // 空块（非保留块）
		pVMQ->pvBlkBaseAddress = NULL;
		pVMQ->BlkSize = 0;
		pVMQ->dwBlkProtection = 0;
		pVMQ->dwBlkStorage = MEM_FREE;
		break;

	case MEM_RESERVE:    // 保留块信息，直接复制
		pVMQ->pvBlkBaseAddress = mbi.BaseAddress;
		pVMQ->BlkSize = mbi.RegionSize;

		// 对于未提交更改的块，mbi.Protect是无效的
		// 所以显示的是保留块继承该区域的保护属性
		pVMQ->dwBlkProtection = mbi.AllocationProtect;
		pVMQ->dwBlkStorage = MEM_RESERVE;
		break;

	case MEM_COMMIT:     // 保留块信息，直接复制
		pVMQ->pvBlkBaseAddress = mbi.BaseAddress;
		pVMQ->BlkSize = mbi.RegionSize;
		pVMQ->dwBlkProtection = mbi.Protect;
		pVMQ->dwBlkStorage = mbi.Type;
		break;

	default:
		DebugBreak();
		break;
	}

	//填充区域数据成员
	VMQUERY_HELP VMQHelp;
	switch (mbi.State)
	{
	case MEM_FREE:       // 空块（非保留块）
		pVMQ->pvRgnBaseAddress = mbi.BaseAddress;
		pVMQ->dwRgnProtection = mbi.AllocationProtect;
		pVMQ->RgnSize = mbi.RegionSize;
		pVMQ->dwRgnStorage = MEM_FREE;
		pVMQ->dwRgnBlocks = 0;
		pVMQ->dwRgnGuardBlks = 0;
		pVMQ->bRgnIsAStack = FALSE;
		break;

	case MEM_RESERVE:    // 保留块信息，直接复制
		pVMQ->pvRgnBaseAddress = mbi.AllocationBase;
		pVMQ->dwRgnProtection = mbi.AllocationProtect;

		// 迭代所有块来获得完整区域信息
		VMQueryHelp(hProcess, pvAddress, &VMQHelp);

		pVMQ->RgnSize = VMQHelp.RgnSize;
		pVMQ->dwRgnStorage = VMQHelp.dwRgnStorage;
		pVMQ->dwRgnBlocks = VMQHelp.dwRgnBlocks;
		pVMQ->dwRgnGuardBlks = VMQHelp.dwRgnGuardBlks;
		pVMQ->bRgnIsAStack = VMQHelp.bRgnIsAStack;
		break;

	case MEM_COMMIT:     // 保留块信息，直接复制
		pVMQ->pvRgnBaseAddress = mbi.AllocationBase;
		pVMQ->dwRgnProtection = mbi.AllocationProtect;

		// 迭代所有块来获得完整区域信息
		VMQueryHelp(hProcess, pvAddress, &VMQHelp);

		pVMQ->RgnSize = VMQHelp.RgnSize;
		pVMQ->dwRgnStorage = VMQHelp.dwRgnStorage;
		pVMQ->dwRgnBlocks = VMQHelp.dwRgnBlocks;
		pVMQ->dwRgnGuardBlks = VMQHelp.dwRgnGuardBlks;
		pVMQ->bRgnIsAStack = VMQHelp.bRgnIsAStack;
		break;

	default:
		printf("default\n");
		break;
	}

	return(bOk);
}

BOOL GetProcessPTT(DWORD dwPId)
{
	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, dwPId);
	// 进程不存在
	if (hProcess == NULL)
	{
		printf("Cannot open the process\n");
		return FALSE;
	}
	// 创建快照
	HANDLE hProcessSnap;
	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	// 遍历虚拟地址空间并输出
	BOOL bOk = TRUE;
	PVOID pvAddress = NULL;
	while (bOk)
	{
		VMQUERY vmq;
		bOk = VMQuery(hProcess, pvAddress, &vmq);
		if (bOk)
		{
			// 构造输出字符串
			char szLine[1024];
			ConstructRgnInfoLine(hProcess, &vmq, szLine, _countof(szLine));
			printf("%s\n", szLine);
			// 尝试下一个区域
			pvAddress = ((PBYTE)vmq.pvRgnBaseAddress + vmq.RgnSize);
		}
	}
	CloseHandle(hProcess);
	return TRUE;
}
