/*************************************
*�ļ���:process.cpp
*˵��:�����������ȡ������ϢԴ�ļ�
*************************************/
#include <stdio.h>
#include "process.h"

// ���ӿ��ļ�
#pragma comment (lib,"psapi.lib")

// ȫ�־�̬��������¼��ǰCPU��Ԥ������ַ�ռ�����ķ�������
// ���ڵ�һ�ε���VMQuery����ʱ����ʼ��
static DWORD gs_dwAllocGran = 0;

BOOL GetProcessList(PProcessList pPL)
{
	HANDLE hProcessSnap;
	HANDLE hProcess;
	DWORD dwPriorityClass;

	// ��ʼ��
	pPL->cProcess = 0;
	pPL->processInfo = new ProcessInfo[ReserveSize];
	ZeroMemory(pPL->processInfo, ReserveSize * sizeof(ProcessInfo));//�ڴ�����

	// �ȵõ�ϵͳ����
	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hProcessSnap == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}

	// ���ȳ�ʼ���ýṹ���dwSize
	pPL->processInfo[0].pe32.dwSize = sizeof(PROCESSENTRY32);

	// �ȳ��Ի�ȡ��һ��������Ϣ��ʧ�����˳�
	if (!Process32First(hProcessSnap, &pPL->processInfo[0].pe32))
	{
		CloseHandle(hProcessSnap);
		return FALSE;
	}

	int i = 0;
	do
	{
		pPL->processInfo[i].pmc.cb = sizeof(PROCESS_MEMORY_COUNTERS);

		// ��ȡ���ȼ�
		dwPriorityClass = 0;
		hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pPL->processInfo[i].pe32.th32ProcessID);
		// ��ý���ʹ���ڴ����
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
		// Ϊ���Ű�����
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
		//���´�ѭ��ǰ���ȳ�ʼ���ýṹ���dwSize
		pPL->processInfo[++i].pe32.dwSize = sizeof(PROCESSENTRY32);
	} while (Process32Next(hProcessSnap, &pPL->processInfo[i].pe32));//�����һ��������Ϣ

	// �����������
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
	//��ʼ��
	pTL->cThread = 0;
	pTL->thread = new THREADENTRY32[ReserveSize];
	ZeroMemory(pTL->thread, ReserveSize * sizeof(THREADENTRY32));

	HANDLE hThreadSnap = INVALID_HANDLE_VALUE;
	THREADENTRY32 te32;

	// �ȵõ�ϵͳ����
	hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
	if (hThreadSnap == INVALID_HANDLE_VALUE)
		return(FALSE);

	te32.dwSize = sizeof(THREADENTRY32);

	// �ȳ��Ի�ȡ��һ���߳���Ϣ��ʧ�����˳�
	if (!Thread32First(hThreadSnap, &te32))
	{
		CloseHandle(hThreadSnap);
		return FALSE;
	}

	int i = 0;
	do
	{
		//�ҵ����PID
		if (te32.th32OwnerProcessID == dwPID)
		{
			memcpy(&pTL->thread[i++], &te32, sizeof(THREADENTRY32));
			printf("%d\t\t%d\n", te32.th32ThreadID, te32.tpBasePri);
		}
	} while (Thread32Next(hThreadSnap, &te32));

	// �����߳�����
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
	//�˲�������������ע��
	//printf("Virtual Available:%ld MB\n", ms.dwAvailVirtual);
	printf("Page File Total:%ld MB\n", ms.dwTotalPageFile / 1048576);
	printf("Page File Available:%ld MB\n", ms.dwAvailPageFile / 1048576);
	return TRUE;
}

char* GetMemStorageText(DWORD dwStorage)
{
	char *p = new char[20];
	//����ֵ�����ַ���
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
	//��ȥ����������ı������Ա�־���������ж�
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
	//����������Ᵽ�����Ա�־
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
	//��ʱ��������ڴ����ͣ�����ʵ����bug
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
	// ��ʼ��
	ZeroMemory(pVMQHelp, sizeof(*pVMQHelp));
	// ���ݴ����ַ��ȡ�������ڴ���Ϣ
	MEMORY_BASIC_INFORMATION mbi;
	BOOL bOk = (VirtualQueryEx(hProcess, pvAddress, &mbi, sizeof(mbi))
		== sizeof(mbi));
	if (!bOk)
		return(bOk);   // �˵�ַ�޷���ȡ��Ϣ
	 // ��¼�������ַ
	PVOID pvRgnBaseAddress = mbi.AllocationBase;
	// ���ڱ������������еĿ�
	PVOID pvAddressBlk = pvRgnBaseAddress;
	// ��¼�ڴ�����
	pVMQHelp->dwRgnStorage = mbi.Type;

	for (;;) {
		// ��ȡ�˿���Ϣ
		bOk = (VirtualQueryEx(hProcess, pvAddressBlk, &mbi, sizeof(mbi))
			== sizeof(mbi));
		// �޷���ȡ
		if (!bOk)
			break;

		 // �жϴ˿��Ƿ�����ͬһ���򣬷����������
		if (mbi.AllocationBase != pvRgnBaseAddress)
			break;

		pVMQHelp->dwRgnBlocks++;             // �������¼����һ��
		pVMQHelp->RgnSize += mbi.RegionSize; // �������¼��С����

		// �����PAGE_GUARD���ԣ����ֵ+1
		if ((mbi.Protect & PAGE_GUARD) == PAGE_GUARD)
			pVMQHelp->dwRgnGuardBlks++;

		if (pVMQHelp->dwRgnStorage == MEM_PRIVATE)
			pVMQHelp->dwRgnStorage = mbi.Type;

		// �����һ���ַ
		pvAddressBlk = (PVOID)((PBYTE)pvAddressBlk + mbi.RegionSize);
	}
	// ��¼�������Ƿ����߳�ջ
	pVMQHelp->bRgnIsAStack = (pVMQHelp->dwRgnGuardBlks > 0);

	return(TRUE);
}

BOOL VMQuery(HANDLE hProcess, LPCVOID pvAddress, PVMQUERY pVMQ)
{
	//�ڵ�һ��ִ��ʱ���÷��������С
	if (gs_dwAllocGran == 0)
	{
		SYSTEM_INFO sinf;
		GetSystemInfo(&sinf);
		gs_dwAllocGran = sinf.dwAllocationGranularity;
	}
	//��ʼ��
	ZeroMemory(pVMQ, sizeof(*pVMQ));

	// ���MEMORY_BASIC_INFORMATION�ṹ
	MEMORY_BASIC_INFORMATION mbi;
	BOOL bOk = (VirtualQueryEx(hProcess, pvAddress, &mbi, sizeof(mbi))
		== sizeof(mbi));
	if (!bOk)
		return(bOk);   // ��ַ��Ч

	// ����MEMORY_BASIC_INFORMATION�ṹ����Զ����VMQUERY�ṹ
	// ��������ݳ�Ա
	switch (mbi.State)
	{
	case MEM_FREE:       // �տ飨�Ǳ����飩
		pVMQ->pvBlkBaseAddress = NULL;
		pVMQ->BlkSize = 0;
		pVMQ->dwBlkProtection = 0;
		pVMQ->dwBlkStorage = MEM_FREE;
		break;

	case MEM_RESERVE:    // ��������Ϣ��ֱ�Ӹ���
		pVMQ->pvBlkBaseAddress = mbi.BaseAddress;
		pVMQ->BlkSize = mbi.RegionSize;

		// ����δ�ύ���ĵĿ飬mbi.Protect����Ч��
		// ������ʾ���Ǳ�����̳и�����ı�������
		pVMQ->dwBlkProtection = mbi.AllocationProtect;
		pVMQ->dwBlkStorage = MEM_RESERVE;
		break;

	case MEM_COMMIT:     // ��������Ϣ��ֱ�Ӹ���
		pVMQ->pvBlkBaseAddress = mbi.BaseAddress;
		pVMQ->BlkSize = mbi.RegionSize;
		pVMQ->dwBlkProtection = mbi.Protect;
		pVMQ->dwBlkStorage = mbi.Type;
		break;

	default:
		DebugBreak();
		break;
	}

	//����������ݳ�Ա
	VMQUERY_HELP VMQHelp;
	switch (mbi.State)
	{
	case MEM_FREE:       // �տ飨�Ǳ����飩
		pVMQ->pvRgnBaseAddress = mbi.BaseAddress;
		pVMQ->dwRgnProtection = mbi.AllocationProtect;
		pVMQ->RgnSize = mbi.RegionSize;
		pVMQ->dwRgnStorage = MEM_FREE;
		pVMQ->dwRgnBlocks = 0;
		pVMQ->dwRgnGuardBlks = 0;
		pVMQ->bRgnIsAStack = FALSE;
		break;

	case MEM_RESERVE:    // ��������Ϣ��ֱ�Ӹ���
		pVMQ->pvRgnBaseAddress = mbi.AllocationBase;
		pVMQ->dwRgnProtection = mbi.AllocationProtect;

		// �������п����������������Ϣ
		VMQueryHelp(hProcess, pvAddress, &VMQHelp);

		pVMQ->RgnSize = VMQHelp.RgnSize;
		pVMQ->dwRgnStorage = VMQHelp.dwRgnStorage;
		pVMQ->dwRgnBlocks = VMQHelp.dwRgnBlocks;
		pVMQ->dwRgnGuardBlks = VMQHelp.dwRgnGuardBlks;
		pVMQ->bRgnIsAStack = VMQHelp.bRgnIsAStack;
		break;

	case MEM_COMMIT:     // ��������Ϣ��ֱ�Ӹ���
		pVMQ->pvRgnBaseAddress = mbi.AllocationBase;
		pVMQ->dwRgnProtection = mbi.AllocationProtect;

		// �������п����������������Ϣ
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
	// ���̲�����
	if (hProcess == NULL)
	{
		printf("Cannot open the process\n");
		return FALSE;
	}
	// ��������
	HANDLE hProcessSnap;
	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	// ���������ַ�ռ䲢���
	BOOL bOk = TRUE;
	PVOID pvAddress = NULL;
	while (bOk)
	{
		VMQUERY vmq;
		bOk = VMQuery(hProcess, pvAddress, &vmq);
		if (bOk)
		{
			// ��������ַ���
			char szLine[1024];
			ConstructRgnInfoLine(hProcess, &vmq, szLine, _countof(szLine));
			printf("%s\n", szLine);
			// ������һ������
			pvAddress = ((PBYTE)vmq.pvRgnBaseAddress + vmq.RgnSize);
		}
	}
	CloseHandle(hProcess);
	return TRUE;
}
