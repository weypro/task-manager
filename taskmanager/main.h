/*************************************
*�ļ���:process.h
*˵��:�����������ͷ�ļ�
*************************************/
#ifndef _MAIN_H_
#define _MAIN_H_

// ��������˵�
void ShowHelpMenu();
// ������н��̺��ڴ���Ϣ
void ShowProcessMemoryInfo();
// ���������ӵ�еĵ��߳���Ϣ
void ShowProcessThreadsInfo(DWORD dwPID);
// �������ҳ��
void ShowProcessPTT(DWORD dwPID);

#endif