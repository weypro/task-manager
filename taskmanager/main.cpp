/***************************************************
*文件名:main.cpp
*说明:任务管理器主源文件
*备注:	
		PDT:Page Directory Table		页目录表
		PDE:Page Directory Entry		页目录表项（页目录表的元素），指向PTT
		PTT:Page Table					页表
		PTE:Page Table Entry			页表项(页表的元素)
		PEB:Process Environment Block	进程环境块
		TEB:Thread Environment Block	线程环境块
*参考https://blog.csdn.net/qq_41988448/article/details/102627239#PDE_14
*PTT获取，参考《Windows核心编程》
*VS2017中调试参数：菜单栏“项目”->taskmanager属性
				  ->调试->右侧有“命令参数”
*请在控制台中执行该程序，并注意输入参数
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
	// 设置窗口标题
	SetConsoleTitle("任务管理器");

	// 当没有其他参数时，输出帮助后退出（argc初值为1，默认参数是程序路径）
	if (argc == 1 || strcmp(argv[1], "-h") == 0)
	{
		ShowHelpMenu();
		return 0;
	}
	// 执行参数对应功能
	// 若需扩展功能可使用函数指针数组做一个索引index，遍历参数字符串数组获得下标，再通过此下标执行index对应的函数
	// 避免大量if else
	// 此处从简，直接比较
	if (strcmp(argv[1], "-a") == 0)
	{
		ShowProcessMemoryInfo();
	}
	else if (strcmp(argv[1], "-t") == 0)
	{
		// 缺少参数
		if (argv[2] == NULL)
		{
			printf("Error:Please input the PID.\n");
			return 0;
		}
		ShowProcessThreadsInfo(atoi(argv[2]));
	}
	else if (strcmp(argv[1], "-m") == 0)
	{
		// 缺少参数
		if (argv[2] == NULL)
		{
			printf("Error:Please input the PID.\n");
			return 0;
		}
		ShowProcessPTT(atoi(argv[2]));
	}
	
	return 0;
}