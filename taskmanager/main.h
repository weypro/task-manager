/*************************************
*文件名:process.h
*说明:任务管理器主头文件
*************************************/
#ifndef _MAIN_H_
#define _MAIN_H_

// 输出帮助菜单
void ShowHelpMenu();
// 输出所有进程和内存信息
void ShowProcessMemoryInfo();
// 输出进程所拥有的的线程信息
void ShowProcessThreadsInfo(DWORD dwPID);
// 输出进程页表
void ShowProcessPTT(DWORD dwPID);

#endif