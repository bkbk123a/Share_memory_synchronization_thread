
#include <iostream>
#include <process.h>
#include "Share.h"
using namespace std;




CBKMemory::CBKMemory()
{
	up_Shard_Data = std::make_shared<UNI_DIO_SHARE_MEM>();//공유 데이터 Share_Ptr 생성


	::InitializeCriticalSection(&m_cs);//유저모드 동기화 - cs오브젝트 생성
}


CBKMemory::~CBKMemory()
{
	::DeleteCriticalSection(&m_cs);//유저모드 동기화 - cs오브젝트 소멸 
}


BOOL CBKMemory::init()//초기화하는 함수입니다. 
{
	auto buf_size = sizeof(UNI_DIO_SHARE_MEM);
	//1. 파일 매핑 커널 오브젝트를 생성한다 . 마지막인자는 커널 오브젝트 네임을 위한 것이며, 이는 프로세스간 공유에 활용된다.
	m_hFileMap = ::CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, (DWORD)buf_size, UNI_SHARE_DATA_ADDRESS);//구조체 가상 공유 메모리로 띄움 :: INVALID_HANDLE_VALUE값
	if (NULL == m_hFileMap)			return FALSE;

	//2.프로세스의 주소 공간 상에 파일 매핑 오브젝트 매핑시킨다 
	m_io_data = (UNI_DIO_SHARE_MEM *)::MapViewOfFile(m_hFileMap, FILE_MAP_ALL_ACCESS, 0, 0, buf_size);
	if (NULL == m_io_data)
	{
		DWORD dwCode = ::GetLastError();
		return FALSE;
	}

	//커널모드 동기화 기법입니다.
	//뮤텍스 오브젝트 기반과, 이벤트 오브젝트 기반 동기화
	m_mtx = ::CreateMutex(NULL, FALSE, UNI_SHARE_MTX_ADDRESS);//뮤텍스 오브젝트 동기화를 위함 - 뮤텍스 오브젝트 생성한다.
	if (NULL == m_mtx)
	{
		DWORD dwCode = ::GetLastError();
		return FALSE;
	}

	//뮤텍스 동기화와 다른 특징중 하나가, 두번째 매개변수 TRUE로 주게되면 auto-reset 이 아닌, manual-reset 가능하다.
	m_event_start = ::CreateEvent(NULL, TRUE, FALSE, UNI_SHARE_EVENT_START_ADDRESS);//이벤트 오브젝트 동기화를 위함 - 이벤트 오브젝트 생성한다
	if (NULL == m_event_start)
	{
		DWORD dwCode = ::GetLastError();
		return FALSE;
	}

	m_event_end = ::CreateEvent(NULL, TRUE, FALSE, UNI_SHARE_EVENT_FINISH_ADDRESS);//이벤트 오브젝트 동기화를 위함 - 이벤트 오브젝트 생성한다
	if (NULL == m_event_end)
	{
		DWORD dwCode = ::GetLastError();
		return FALSE;
	}

	return TRUE;
}



BYTE CBKMemory::get_input(CONST BYTE input)//특정 인풋 조회 함수
{
	return up_Shard_Data->in_bit[input];
}

BYTE CBKMemory::get_output(CONST BYTE output)//특정 아웃풋 조회 함수
{
	return up_Shard_Data->out_bit[output];
}

VOID CBKMemory::set_input(CONST BYTE input, BOOL bOnOff)
{
	WaitForSingleObject(m_mtx, INFINITE);//커널모드 동기화 기법(뮤텍스 기반) : 동기화 진입
	if(TRUE == bOnOff)
	{
		up_Shard_Data->in_bit[input] = 1;
	}
	else
	{
		up_Shard_Data->in_bit[input] = 0;
	}
	ReleaseMutex(m_mtx);//크리티컬 섹션 탈출합니다.
	return;
}


VOID CBKMemory::set_output(CONST BYTE Output, BOOL bOnOff)
{
	WaitForSingleObject(m_mtx, INFINITE);//커널모드 동기화 기법(뮤텍스 기반) : 동기화 진입
	if (TRUE == bOnOff)
	{
		up_Shard_Data->out_bit[Output] = 1;
	}
	else
	{
		up_Shard_Data->out_bit[Output] = 0;
	}
	ReleaseMutex(m_mtx);//크리티컬 섹션 탈출합니다.

	return;
}


VOID CBKMemory::flush()//초기화 함수
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


	//up_Shard_Data 스마트포인터(쉐어드 포인터)는 자동으로 해제된다.
	if (NULL != m_mtx)
	{
		rtn &= ::CloseHandle(m_mtx);//mtx 오브젝트 소멸 
		m_mtx = NULL;
	}

	/////매핑시킨 m_io_data 내용 해제합니다. 
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
