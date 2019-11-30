/***************************************************
*�ļ���:main.cpp
*˵��:�����������Դ�ļ�
*��ע:	
		PDT:Page Directory Table		ҳĿ¼��
		PDE:Page Directory Entry		ҳĿ¼���ҳĿ¼���Ԫ�أ���ָ��PTT
		PTT:Page Table					ҳ��
		PTE:Page Table Entry			ҳ����(ҳ���Ԫ��)
		PEB:Process Environment Block	���̻�����
		TEB:Thread Environment Block	�̻߳�����
*�ο�https://blog.csdn.net/qq_41988448/article/details/102627239#PDE_14
*PTT��ȡ���ο���Windows���ı�̡�
*VS2017�е��Բ������˵�������Ŀ��->taskmanager����
				  ->����->�Ҳ��С����������
*���ڿ���̨��ִ�иó��򣬲�ע���������
***************************************************/
#include <cstdio>
#include <time.h>
#include <Windows.h>
#include <psapi.h>
#include <list>
#include "main.h"
#include "process.h"

void ShowHelpMenu()
{
	printf("Welcome to use the task manager.\n");
	printf("Command:\n");
	printf("-h\t\tShow this help.\n");
	printf("-a\t\tShow the infomation of processes and memory.\n");
	printf("-t [PID]\tShow the threads owned by the process.\n");
	printf("-m [PID]\tShow the PTT of the process.\n");
}

void ShowProcessMemoryInfo()
{
	printf("Show the list of processes and memory info\n");
	pl = new ProcessList();
	printf("PID\tProcess Name\t\t\tParent ID\tThreads\tPrivilege Class\n");
	GetProcessList(pl);
	FreeProcessList(pl);
	GetSysMemoryInfo();
}

void ShowProcessThreadsInfo(DWORD dwPID)
{
	printf("Show Process Threads\n");
	printf("Thread ID\tPrivilege\n");

	pTL = new ThreadList();
	GetProcessThreads(dwPID, pTL);
}

void ShowProcessPTT(DWORD dwPID)
{
	printf("Show the PTT of the process\n");
	//printf("Address\t\t\tType\t\tSize\tBlocks\tProtection Attribution\n");
	printf("Address\t\t\tType\t\tSize\n");
	GetProcessPTT(dwPID);
}

int main(int argc, char *argv[])
{
	// ���ô��ڱ���
	SetConsoleTitle("���������");

	// ��û����������ʱ������������˳���argc��ֵΪ1��Ĭ�ϲ����ǳ���·����
	if (argc == 1 || strcmp(argv[1], "-h") == 0)
	{
		ShowHelpMenu();
		return 0;
	}
	// ִ�в�����Ӧ����
	// ������չ���ܿ�ʹ�ú���ָ��������һ������index�����������ַ����������±꣬��ͨ�����±�ִ��index��Ӧ�ĺ���
	// �������if else
	// �˴��Ӽ�ֱ�ӱȽ�
	if (strcmp(argv[1], "-a") == 0)
	{
		ShowProcessMemoryInfo();
	}
	else if (strcmp(argv[1], "-t") == 0)
	{
		// ȱ�ٲ���
		if (argv[2] == NULL)
		{
			printf("Error:Please input the PID.\n");
			return 0;
		}
		ShowProcessThreadsInfo(atoi(argv[2]));
	}
	else if (strcmp(argv[1], "-m") == 0)
	{
		// ȱ�ٲ���
		if (argv[2] == NULL)
		{
			printf("Error:Please input the PID.\n");
			return 0;
		}
		ShowProcessPTT(atoi(argv[2]));
	}
	
	return 0;
}