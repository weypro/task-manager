/*************************************
*�ļ���:process.h
*˵��:�����������ȡ������Ϣͷ�ļ�
*************************************/
#ifndef _PROCESS_H_
#define _PROCESS_H_

#include <Windows.h>
#include <psapi.h>
#include <TlHelp32.h>
#include "process.h"

#define ReserveSize 1024//�����ʼ����С


/*****************�ṹ�嶨��*******************/
// ������Ϣ
typedef struct __tagProcessInfo {
	PROCESSENTRY32 pe32;
	PROCESS_MEMORY_COUNTERS pmc;
} ProcessInfo, *PProcessInfo;
// ������Ϣ��
typedef struct __tagProcessList {
	DWORD cProcess;
	PProcessInfo processInfo;
} ProcessList, *PProcessList;
// ȫ�־�̬������Ϣ��
static PProcessList pl;

// �߳���Ϣ��
typedef struct __tagThreadList {
	DWORD cThread;
	THREADENTRY32 *thread;
} ThreadList, *PThreadList;
// ȫ�־�̬�߳���Ϣ��
static PThreadList pTL;

// ����ҳ��
typedef struct _tagProcessPTE{
	// ������Ϣ
	PVOID  pvRgnBaseAddress; // ����ռ��������ʼ��ַ
	DWORD  dwRgnProtection;  // ������ָ���ı������ԣ���PAGE_READWRITE
	SIZE_T RgnSize;			 // ��Ԥ������Ĵ�С�����ֽ�Ϊ��λ
	DWORD  dwRgnStorage;     // ���������и��������ڴ�����ͣ�����MEM_Free/Image/Mapped/Private
	DWORD  dwRgnBlocks;		 // �����п������
	DWORD  dwRgnGuardBlks;   // ��������PAGE_GUARD�������Ա�־�Ŀ��������ͨ����0��1�������1����˵����������Ϊ���߳�ջ��Ԥ��
	BOOL   bRgnIsAStack;     // �������Ƿ����߳�ջ

	// ����Ϣ
	PVOID  pvBlkBaseAddress; // �����ʼ��ַ
	DWORD  dwBlkProtection;  // �ÿ�ָ���ı������ԣ���PAGE_READWRITE
	SIZE_T BlkSize;			 // ��Ĵ�С
	DWORD  dwBlkStorage;     // �������ڴ�����ͣ�����MEM_Free/Image/Mapped/Private
} VMQUERY, *PVMQUERY;

// �����ṹ
typedef struct {
	SIZE_T RgnSize;			 // ��¼�����С֮��
	DWORD  dwRgnStorage;     // �ڴ����ͣ���MEM_FREE��
	DWORD  dwRgnBlocks;		 // �����еĿ���
	DWORD  dwRgnGuardBlks;   // ����0˵���������߳�ջ
	BOOL   bRgnIsAStack;     // Ϊ��˵���������߳�ջ
} VMQUERY_HELP;


/*****************��������*******************/
// ��ý����б�
BOOL GetProcessList(PProcessList pPL);
// ��������б�
void FreeProcessList(PProcessList pPL);
// ���ϵͳ�ڴ���Ϣ
BOOL GetSysMemoryInfo();
// ����PID������߳���Ϣ
BOOL GetProcessThreads(DWORD dwPID, PThreadList pTL);
// �����߳��б�
void FreeListProcessThreads(PThreadList pTL);
// ��ý���ҳ��
BOOL GetProcessPTT(DWORD dwPId);
/*����PTT�Ļ�ȡ������Windows���ı�̡�������13��*/
// ������ȡ�ڴ�������Ϣ
BOOL VMQuery(HANDLE hProcess, LPCVOID pvAddress, PVMQUERY pVMQ);
// ������һ����ȡ��Ϣ
static BOOL VMQueryHelp(HANDLE hProcess, LPCVOID pvAddress, VMQUERY_HELP *pVMQHelp);
// �������ݹ����ڴ�������Ϣ���
void ConstructRgnInfoLine(HANDLE hProcess, PVMQUERY pVMQ, PTSTR szLine, int cchMaxLen);
// ����������תΪ����
char* GetProtectText(DWORD dwProtect, char* szBuf, size_t chSize, BOOL bShowFlags);
// ���ڴ���������תΪ����
char* GetMemStorageText(DWORD dwStorage);
#endif