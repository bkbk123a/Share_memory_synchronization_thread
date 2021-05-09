#include <stdio.h>
#include <windows.h>
#include <process.h> 
#include <thread>
#include <iostream>
#include "Share.h"

#define NUM_THREADS 6
constexpr INT ARRAY_MAX = 128;//0~127���� :: unsigned char(1����Ʈ)
constexpr BYTE ONE_BYTE = 128;


unsigned WINAPI output_threadOddOn(void *arg);
unsigned WINAPI output_threadEvenOn(void *arg);
unsigned WINAPI output_threadOddOff(void *arg);
unsigned WINAPI output_threadEvenOff(void *arg);
unsigned WINAPI output_threadAllOn(void *arg);
unsigned WINAPI output_threadAllOff(void *arg);
unsigned WINAPI update_output(void *arg);





CBKMemory *BKObj = new CBKMemory;
HANDLE m_event_other_process;


int main() 
{	
	HANDLE hThread1[NUM_THREADS];
	HANDLE hThread2;
	
	BKObj->init();//���μ��� 1����� �ʱ�ȭ�ϴ� �Լ��Դϴ�. �޸𸮸��� / Ŀ�θ�� �̺�Ʈ ������Ʈ ���� 
	m_event_other_process = CreateEvent(NULL, FALSE, NULL,NULL);//���μ��� 2Ŀ�θ�� �̺�Ʈ ������Ʈ ���� auto_reset����Դϴ�.


	hThread1[0] = (HANDLE)_beginthreadex(NULL, 0, output_threadOddOn, NULL, 0, NULL);
	hThread1[1] = (HANDLE)_beginthreadex(NULL, 0, output_threadEvenOn, NULL, 0, NULL);
	hThread1[2] = (HANDLE)_beginthreadex(NULL, 0, output_threadOddOff, NULL, 0, NULL);
	hThread1[3] = (HANDLE)_beginthreadex(NULL, 0, output_threadEvenOff, NULL, 0, NULL);
	hThread1[4] = (HANDLE)_beginthreadex(NULL, 0, output_threadAllOn, NULL, 0, NULL);
	hThread1[5] = (HANDLE)_beginthreadex(NULL, 0, output_threadAllOff, NULL, 0, NULL);
	hThread2 = (HANDLE)_beginthreadex(NULL, 0, update_output, NULL, 0, NULL);
	

	SetEvent(BKObj->m_event_start);//���μ��� 1���� �߻���Ű�� �̺�Ʈ
	


	WaitForMultipleObjects(6, hThread1, TRUE, INFINITE);//�������� ����ó�� ���
	SetEvent(m_event_other_process);//���μ��� 2���� �߻���Ű�� �̺�Ʈ 

	ResetEvent(BKObj->m_event_start);
	WaitForSingleObject(hThread2, INFINITE);
	CloseHandle(m_event_other_process);//���μ��� 2���� �߻���Ű�� �̺�Ʈ �Ҹ�

	BKObj->terminate();//�����޸� ���� �� ������Ʈ(���ؽ�, �̺�Ʈ) �Ҹ�


    return 0;
}




////////////������ ȣ���Լ� 6���� �ٸ� ���μ������� ȣ���ϴ� update_output�Լ�


unsigned WINAPI output_threadOddOn(void *arg)
{
	//WaitForSingleObject(BKObj->m_event_start, INFINITE);

	for (auto i = 0; i < ARRAY_MAX; i++)
	{
		WaitForSingleObject(BKObj->m_mtx, INFINITE);
		BKObj->m_icscnt++;
		ReleaseMutex(BKObj->m_mtx);
		if ((i % 2) == 1)
		{
			BKObj->set_output((BYTE)i, TRUE);
		}
	}
	//ResetEvent(BKObj->m_event_start);
	return 0;
}

unsigned WINAPI output_threadEvenOn(void *arg)
{

	//WaitForSingleObject(BKObj->m_event_start, INFINITE);

	for (auto i = 0; i < ARRAY_MAX; i++)
	{
		WaitForSingleObject(BKObj->m_mtx, INFINITE);
		BKObj->m_icscnt++;
		ReleaseMutex(BKObj->m_mtx);
		if ((i % 2) == 0)
		{
			BKObj->set_output((BYTE)i, TRUE);
		}
	}

	//ResetEvent(BKObj->m_event_start);
	return 0;
}
unsigned WINAPI output_threadOddOff(void *arg)
{
	//WaitForSingleObject(BKObj->m_event_start, INFINITE);
	
	for (auto i = 0; i < ARRAY_MAX; i++)
	{
		WaitForSingleObject(BKObj->m_mtx, INFINITE);
		BKObj->m_icscnt--;
		ReleaseMutex(BKObj->m_mtx);
		if ((i % 2) == 1)
		{
			BKObj->set_output((BYTE)i, FALSE);
		}
	}

	//ResetEvent(BKObj->m_event_start);
	return 0;
}
unsigned WINAPI output_threadEvenOff(void *arg)
{
	//WaitForSingleObject(BKObj->m_event_start, INFINITE);

	for (auto i = 0; i < ARRAY_MAX; i++)
	{
		WaitForSingleObject(BKObj->m_mtx, INFINITE);
		BKObj->m_icscnt--;
		ReleaseMutex(BKObj->m_mtx);
		if ((i % 2) == 0)
		{
			BKObj->set_output((BYTE)i, FALSE);
		}
	}

	//ResetEvent(BKObj->m_event_start);
	return 0;
}
unsigned WINAPI output_threadAllOn(void *arg)
{
	//WaitForSingleObject(BKObj->m_event_start, INFINITE);

	for (auto i = 0; i < ARRAY_MAX; i++)
	{
		WaitForSingleObject(BKObj->m_mtx, INFINITE);
		BKObj->m_icscnt++;
		ReleaseMutex(BKObj->m_mtx);
		BKObj->set_output((BYTE)i, TRUE);
	}

	//ResetEvent(BKObj->m_event_start);
	return 0;
}

unsigned WINAPI output_threadAllOff(void *arg)
{
	//WaitForSingleObject(BKObj->m_event_start, INFINITE);

	for (auto i = 0; i < ARRAY_MAX; i++)
	{
		WaitForSingleObject(BKObj->m_mtx, INFINITE);
		BKObj->m_icscnt--;
		ReleaseMutex(BKObj->m_mtx);
		BKObj->set_output((BYTE)i, FALSE);
	}

	//ResetEvent(BKObj->m_event_start);
	return 0;
}




unsigned WINAPI update_output(void *arg)
{

	DWORD dwWaitResult = WaitForSingleObject(m_event_other_process, INFINITE);
	int iOutputCnt(0);
	int iInputCnt(0);

	switch (dwWaitResult)
	{
	case WAIT_OBJECT_0:
		for (auto i = 0; i < (int)ONE_BYTE; i++)
		{
			if (BKObj->up_Shard_Data->out_bit[i] == 1)//�ƿ�ǲ�� ������� ��ǲ�� �ִ´�. 
			{
				iOutputCnt++;
				BKObj->up_Shard_Data->in_bit[i] = 1;
			}
			else if (BKObj->up_Shard_Data->out_bit[i] == 0)
			{
				iInputCnt++;
				BKObj->up_Shard_Data->in_bit[i] = 0;
			}
		}

	case WAIT_ABANDONED:
		break;
	}
	ResetEvent(m_event_other_process);//�̺�Ʈ ������Ʈ ��� ����ȭ manual-reset����Դϴ�.
	std::cout << "�������� ������ ���� �ƿ�ǲ ��ȭ�� ��ǲ ��ȭ�� ��� �޶��� �� �ֽ��ϴ�."<<endl;
	std::cout << "�ֳ��ϸ� on(1)�� off(1)�θ� �����ϱ� ������ ������ ������ ������ ���� �� �ֱ� �����Դϴ�." << endl;
	std::cout << "���� 7���� �����尡 ���ư��� �ֽ��ϴ�."<<endl;
	std::cout << "�ƿ�ǲ ��ȭ : " << iOutputCnt << " / " << "��ǲ ��ȭ : " << iInputCnt<< "-----------------��:128"<<endl;
	std::cout << "������ �Ӱ迵�� �湮 Ƚ�� ����� �غ���, Ŀ�θ�� ����ȭ�� ���� ���� ���μ�������" << endl;
	std::cout << "�����尪 : "<< BKObj->m_icscnt <<" / ���� 0�� ���´ٸ� �����Դϴ�" << endl;
	return 0;
}
