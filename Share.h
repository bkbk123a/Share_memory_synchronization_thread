#pragma once
#include <thread>
#include <mutex>
#include <windows.h>

using namespace std;

static LPCTSTR UNI_SHARE_DATA_ADDRESS = ("{A1234AAA-1234-5678-1234-1234123411}");//가상 공유메모리 어드레스
static LPCTSTR UNI_SHARE_MTX_ADDRESS = ("{A1234AAA-1234-5678-1234-1234123412}");//뮤텍스 어드레스
static LPCTSTR UNI_SHARE_EVENT_START_ADDRESS = ("{A1234AAA-1234-5678-1234-1234123413}");//이벤트 오브젝트 어드레스 
static LPCTSTR UNI_SHARE_EVENT_FINISH_ADDRESS = ("{A1234AAA-1234-5678-1234-1234123414}");//이벤트 오브젝트 어드레스

CONST BYTE ARRAY_MAX_SIZE = 128;//0~127까지 :: unsigned char(1바이트)


//::비트연산을 위한 구조체입니다.
typedef struct UNI_DIO_SHARE_MEM {
	BYTE in_bit[ARRAY_MAX_SIZE];
	BYTE out_bit[ARRAY_MAX_SIZE];

	BYTE data_update;
} UNI_DIO_SHARE_MEM;

class CBKMemory
{
	public:
		CBKMemory();
		~CBKMemory();

		shared_ptr<UNI_DIO_SHARE_MEM> up_Shard_Data;

		BOOL init();//시작시 할당하는 함수
		VOID terminate();//종료시 해제하는 함수 
		BYTE get_input(CONST BYTE input);//공유메모리 내 input확인
		BYTE get_output(CONST BYTE output);//공유메모리 내 output확인
		VOID set_input(CONST BYTE input, BOOL bOnOff);//On -> TRUE(1), Off ->FALSE(0)
		VOID set_output(CONST BYTE output, BOOL bOnOff);//On -> TRUE(1), Off ->FALSE(0)
		VOID flush();
		VOID startevent();




		HANDLE					 m_event_start;//커널모드 동기화를 위한 이벤트 
		HANDLE					 m_event_end;//커널모드 동기화를 위한 이벤트 오브젝트
		HANDLE					 m_mtx;//커널모드 동기화를 위한 뮤텍스 오브젝트
		UNI_DIO_SHARE_MEM *		 m_io_data;//매핑시킬 내용(UNI_DIO_SHARE_MEM을 가상 공유 메모리로 띄윤다  Address:UNI_SHARE_DATA_ADDRESS)
		int						 m_icscnt;//On시 + , Off시 -


	protected:
		HANDLE					m_hFileMap;
		CRITICAL_SECTION		m_cs;//유저모드 동기화
	
};

