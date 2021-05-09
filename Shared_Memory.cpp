#include <stdio.h>
#include <windows.h>
#include <process.h> 
#include <thread>
#include <iostream>
#include "Share.h"

#define NUM_THREADS 6
constexpr INT ARRAY_MAX = 128;//0~127까지 :: unsigned char(1바이트)
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
	
	BKObj->init();//프로세스 1내용들 초기화하는 함수입니다. 메모리매핑 / 커널모드 이벤트 오브젝트 생성 
	m_event_other_process = CreateEvent(NULL, FALSE, NULL,NULL);//프로세스 2커널모드 이벤트 오브젝트 생성 auto_reset모드입니다.


	hThread1[0] = (HANDLE)_beginthreadex(NULL, 0, output_threadOddOn, NULL, 0, NULL);
	hThread1[1] = (HANDLE)_beginthreadex(NULL, 0, output_threadEvenOn, NULL, 0, NULL);
	hThread1[2] = (HANDLE)_beginthreadex(NULL, 0, output_threadOddOff, NULL, 0, NULL);
	hThread1[3] = (HANDLE)_beginthreadex(NULL, 0, output_threadEvenOff, NULL, 0, NULL);
	hThread1[4] = (HANDLE)_beginthreadex(NULL, 0, output_threadAllOn, NULL, 0, NULL);
	hThread1[5] = (HANDLE)_beginthreadex(NULL, 0, output_threadAllOff, NULL, 0, NULL);
	hThread2 = (HANDLE)_beginthreadex(NULL, 0, update_output, NULL, 0, NULL);
	

	SetEvent(BKObj->m_event_start);//프로세스 1에서 발생시키는 이벤트
	


	WaitForMultipleObjects(6, hThread1, TRUE, INFINITE);//전쓰레드 연산처리 대기
	SetEvent(m_event_other_process);//프로세스 2에서 발생시키는 이벤트 

	ResetEvent(BKObj->m_event_start);
	WaitForSingleObject(hThread2, INFINITE);
	CloseHandle(m_event_other_process);//프로세스 2에서 발생시키는 이벤트 소멸

	BKObj->terminate();//공유메모리 해제 및 오브젝트(뮤텍스, 이벤트) 소멸


    return 0;
}




////////////쓰레드 호출함수 6개와 다른 프로세스에서 호출하는 update_output함수


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
			if (BKObj->up_Shard_Data->out_bit[i] == 1)//아웃풋의 내용들을 인풋에 넣는다. 
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
	ResetEvent(m_event_other_process);//이벤트 오브젝트 기반 동기화 manual-reset모드입니다.
	std::cout << "쓰레드의 성질로 인해 아웃풋 변화와 인풋 변화는 계속 달라질 수 있습니다."<<endl;
	std::cout << "왜냐하면 on(1)과 off(1)로만 조절하기 때문에 쓰레드 내에서 개입이 생길 수 있기 때문입니다." << endl;
	std::cout << "현재 7개의 쓰레드가 돌아가고 있습니다."<<endl;
	std::cout << "아웃풋 변화 : " << iOutputCnt << " / " << "인풋 변화 : " << iInputCnt<< "-----------------합:128"<<endl;
	std::cout << "하지만 임계영역 방문 횟수 계산을 해보면, 커널모드 동기화를 통해 다중 프로세스여도" << endl;
	std::cout << "쓰레드값 : "<< BKObj->m_icscnt <<" / 만약 0이 나온다면 정답입니다" << endl;
	return 0;
}
