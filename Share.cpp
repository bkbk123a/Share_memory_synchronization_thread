
#include <iostream>
#include <process.h>
#include "Share.h"
using namespace std;




CBKMemory::CBKMemory()
{
	up_Shard_Data = std::make_shared<UNI_DIO_SHARE_MEM>();//���� ������ Share_Ptr ����


	::InitializeCriticalSection(&m_cs);//������� ����ȭ - cs������Ʈ ����
}


CBKMemory::~CBKMemory()
{
	::DeleteCriticalSection(&m_cs);//������� ����ȭ - cs������Ʈ �Ҹ� 
}


BOOL CBKMemory::init()//�ʱ�ȭ�ϴ� �Լ��Դϴ�. 
{
	auto buf_size = sizeof(UNI_DIO_SHARE_MEM);
	//1. ���� ���� Ŀ�� ������Ʈ�� �����Ѵ� . ���������ڴ� Ŀ�� ������Ʈ ������ ���� ���̸�, �̴� ���μ����� ������ Ȱ��ȴ�.
	m_hFileMap = ::CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, (DWORD)buf_size, UNI_SHARE_DATA_ADDRESS);//����ü ���� ���� �޸𸮷� ��� :: INVALID_HANDLE_VALUE��
	if (NULL == m_hFileMap)			return FALSE;

	//2.���μ����� �ּ� ���� �� ���� ���� ������Ʈ ���ν�Ų�� 
	m_io_data = (UNI_DIO_SHARE_MEM *)::MapViewOfFile(m_hFileMap, FILE_MAP_ALL_ACCESS, 0, 0, buf_size);
	if (NULL == m_io_data)
	{
		DWORD dwCode = ::GetLastError();
		return FALSE;
	}

	//Ŀ�θ�� ����ȭ ����Դϴ�.
	//���ؽ� ������Ʈ ��ݰ�, �̺�Ʈ ������Ʈ ��� ����ȭ
	m_mtx = ::CreateMutex(NULL, FALSE, UNI_SHARE_MTX_ADDRESS);//���ؽ� ������Ʈ ����ȭ�� ���� - ���ؽ� ������Ʈ �����Ѵ�.
	if (NULL == m_mtx)
	{
		DWORD dwCode = ::GetLastError();
		return FALSE;
	}

	//���ؽ� ����ȭ�� �ٸ� Ư¡�� �ϳ���, �ι�° �Ű����� TRUE�� �ְԵǸ� auto-reset �� �ƴ�, manual-reset �����ϴ�.
	m_event_start = ::CreateEvent(NULL, TRUE, FALSE, UNI_SHARE_EVENT_START_ADDRESS);//�̺�Ʈ ������Ʈ ����ȭ�� ���� - �̺�Ʈ ������Ʈ �����Ѵ�
	if (NULL == m_event_start)
	{
		DWORD dwCode = ::GetLastError();
		return FALSE;
	}

	m_event_end = ::CreateEvent(NULL, TRUE, FALSE, UNI_SHARE_EVENT_FINISH_ADDRESS);//�̺�Ʈ ������Ʈ ����ȭ�� ���� - �̺�Ʈ ������Ʈ �����Ѵ�
	if (NULL == m_event_end)
	{
		DWORD dwCode = ::GetLastError();
		return FALSE;
	}

	return TRUE;
}



BYTE CBKMemory::get_input(CONST BYTE input)//Ư�� ��ǲ ��ȸ �Լ�
{
	return up_Shard_Data->in_bit[input];
}

BYTE CBKMemory::get_output(CONST BYTE output)//Ư�� �ƿ�ǲ ��ȸ �Լ�
{
	return up_Shard_Data->out_bit[output];
}

VOID CBKMemory::set_input(CONST BYTE input, BOOL bOnOff)
{
	WaitForSingleObject(m_mtx, INFINITE);//Ŀ�θ�� ����ȭ ���(���ؽ� ���) : ����ȭ ����
	if(TRUE == bOnOff)
	{
		up_Shard_Data->in_bit[input] = 1;
	}
	else
	{
		up_Shard_Data->in_bit[input] = 0;
	}
	ReleaseMutex(m_mtx);//ũ��Ƽ�� ���� Ż���մϴ�.
	return;
}


VOID CBKMemory::set_output(CONST BYTE Output, BOOL bOnOff)
{
	WaitForSingleObject(m_mtx, INFINITE);//Ŀ�θ�� ����ȭ ���(���ؽ� ���) : ����ȭ ����
	if (TRUE == bOnOff)
	{
		up_Shard_Data->out_bit[Output] = 1;
	}
	else
	{
		up_Shard_Data->out_bit[Output] = 0;
	}
	ReleaseMutex(m_mtx);//ũ��Ƽ�� ���� Ż���մϴ�.

	return;
}


VOID CBKMemory::flush()//�ʱ�ȭ �Լ�
{
	for (auto i = 0; i < ARRAY_MAX_SIZE; i++)
	{
		if (up_Shard_Data->out_bit[i] == 1)
		{
			up_Shard_Data->out_bit[i] = 0;
		}

		if (up_Shard_Data->in_bit[i] == 1)
		{
			up_Shard_Data->in_bit[i] = 0;
		}
	}
}

VOID CBKMemory::startevent()
{
	SetEvent(m_event_start);
}




VOID CBKMemory::terminate()
{
	BOOL rtn = TRUE;

	flush();

	if (NULL != m_event_start)
	{
		rtn &= (0 != ::CloseHandle(m_event_start));
		m_event_start = NULL;
	}

	if (NULL != m_event_end)
	{
		rtn &= (0 != ::CloseHandle(m_event_end));
		m_event_end = NULL;
	}


	//up_Shard_Data ����Ʈ������(����� ������)�� �ڵ����� �����ȴ�.
	if (NULL != m_mtx)
	{
		rtn &= ::CloseHandle(m_mtx);//mtx ������Ʈ �Ҹ� 
		m_mtx = NULL;
	}

	/////���ν�Ų m_io_data ���� �����մϴ�. 
	//1.CreateFileMapping
	//2.MapViewOfFile
	//3.UnmapOfFile
	//4.CloseHandle;
	if (NULL != m_io_data)
	{
		rtn &= (0 != ::UnmapViewOfFile(m_io_data));
		m_io_data = NULL;
	}

	if (NULL != m_hFileMap)
	{
		rtn &= (0 != ::CloseHandle(m_hFileMap));
		m_hFileMap = NULL;
	}

	return;
}


//VOID CBKMemory::output_threadOddOn()
//{
//	int iTemp = (int)ARRAY_MAX_SIZE;
//	for (auto i = 0; i < iTemp; i++)
//	{
//		if ((iTemp % 2) == 1)
//		{
//			set_output((BYTE)iTemp, TRUE);
//		}
//	}
//}
//
//VOID CBKMemory::output_threadOddOff(CONST BYTE byte)
//{
//	for (auto i = 0; i < byte; i++)
//	{
//		if ((byte % 2) == 1)
//		{
//			set_output(byte, FALSE);
//		}
//	}
//}
//
//VOID CBKMemory::output_threadEvenOn(CONST BYTE byte)
//{
//	for (auto i = 0; i < byte; i++)
//	{
//		if ((byte % 2) == 0)
//		{
//			set_output(byte, TRUE);
//		}
//	}
//}
//
// VOID CBKMemory::output_threadEvenOff(CONST BYTE byte)
// {
//	 for (auto i = 0; i < byte; i++)
//	 {
//		 if ((byte % 2) == 0)
//		 {
//			 set_output(byte, FALSE);
//		 }
//	 }
// }
//
//VOID CBKMemory::output_threadAllOn(CONST BYTE byte)
//{
//	for (auto i = 0; i < byte; i++)
//	{
//		set_output(byte, TRUE);
//	}
//}
//
//VOID CBKMemory::output_threadAllOff(CONST BYTE byte)
//{
//	for (auto i = 0; i < byte; i++)
//	{
//		set_output(byte, TRUE);
//	}
//}
//
