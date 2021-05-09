#pragma once
#include <thread>
#include <mutex>
#include <windows.h>

using namespace std;

static LPCTSTR UNI_SHARE_DATA_ADDRESS = ("{A1234AAA-1234-5678-1234-1234123411}");//���� �����޸� ��巹��
static LPCTSTR UNI_SHARE_MTX_ADDRESS = ("{A1234AAA-1234-5678-1234-1234123412}");//���ؽ� ��巹��
static LPCTSTR UNI_SHARE_EVENT_START_ADDRESS = ("{A1234AAA-1234-5678-1234-1234123413}");//�̺�Ʈ ������Ʈ ��巹�� 
static LPCTSTR UNI_SHARE_EVENT_FINISH_ADDRESS = ("{A1234AAA-1234-5678-1234-1234123414}");//�̺�Ʈ ������Ʈ ��巹��

CONST BYTE ARRAY_MAX_SIZE = 128;//0~127���� :: unsigned char(1����Ʈ)


//::��Ʈ������ ���� ����ü�Դϴ�.
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

		BOOL init();//���۽� �Ҵ��ϴ� �Լ�
		VOID terminate();//����� �����ϴ� �Լ� 
		BYTE get_input(CONST BYTE input);//�����޸� �� inputȮ��
		BYTE get_output(CONST BYTE output);//�����޸� �� outputȮ��
		VOID set_input(CONST BYTE input, BOOL bOnOff);//On -> TRUE(1), Off ->FALSE(0)
		VOID set_output(CONST BYTE output, BOOL bOnOff);//On -> TRUE(1), Off ->FALSE(0)
		VOID flush();
		VOID startevent();




		HANDLE					 m_event_start;//Ŀ�θ�� ����ȭ�� ���� �̺�Ʈ 
		HANDLE					 m_event_end;//Ŀ�θ�� ����ȭ�� ���� �̺�Ʈ ������Ʈ
		HANDLE					 m_mtx;//Ŀ�θ�� ����ȭ�� ���� ���ؽ� ������Ʈ
		UNI_DIO_SHARE_MEM *		 m_io_data;//���ν�ų ����(UNI_DIO_SHARE_MEM�� ���� ���� �޸𸮷� ������  Address:UNI_SHARE_DATA_ADDRESS)
		int						 m_icscnt;//On�� + , Off�� -


	protected:
		HANDLE					m_hFileMap;
		CRITICAL_SECTION		m_cs;//������� ����ȭ
	
};

