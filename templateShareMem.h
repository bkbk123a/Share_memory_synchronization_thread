#pragma once



template <class T> class CFileMapMemory
{
public:
	CFileMapMemory(void) : m_file(INVALID_HANDLE_VALUE), m_memMapping(NULL), m_ptr(NULL)
	{
	}

	~CFileMapMemory(void)
	{
		MemFree();
	}

public:


	T* alloc(std::size_t byte, LPCTSTR name, LPCTSTR txt_path = NULL)
	{
		MemFree();

		if (0 == byte)
			return NULL;

		if (NULL != txt_path)
		{
			m_txtMapping = CreateFile(txt_path,
				GENERIC_READ | GENERIC_WRITE,
				FILE_SHARE_READ | FILE_SHARE_WRITE,
				NULL,
				OPEN_ALWAYS,
				FILE_ATTRIBUTE_NORMAL,
				NULL);

			//GENERIC_READ | GENERIC_WRITE,//파일의 접근 권한 지정 : 읽고 쓰고 둘다
			//				//FILE_SHARE_READ | FILE_SHARE_WRITE,//파일의 공유모드 지정 : 내가 열어서 사용하고 있는데 다른 프로세스에서 접근시 읽기, 쓰기 허용
			//				//NULL,//파일의 보안 속성 지정하는 구조체 포인터, dafault NULL
			//				//OPEN_ALWAYS,//항상 파일을 새로 만든다. 기존에 파일이 있다면 덮어쓰기 
			//				//FILE_ATTRIBUTE_NORMAL,//  FILE_ATTRUBUTE_NORMAL : 아무런 속성도 없는 파일을 생성.
			//				//NULL);//새로 만들 파일의 속성 제공할 템플릿 파일, dafault NULL

			if (INVALID_HANDLE_VALUE == m_txtMapping)
				return NULL;

			m_memMapping = CreateFileMapping(m_txtMapping, NULL, PAGE_READWRITE, 0, (DWORD)byte, name);
		}
		else
		{
			m_memMapping = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, (DWORD)byte, name);
		}

		if (NULL == m_memMapping)
			return NULL;

		m_ptr = (T*)MapViewOfFile(m_memMapping, FILE_MAP_ALL_ACCESS, 0, 0, byte);

		DWORD code = 0;
		if (NULL == m_ptr)
		{
			code = GetLastError();
		}

		return m_ptr;
	}

	VOID MemFree()
	{
		if (NULL != m_ptr)
		{
			UnmapViewOfFile(m_ptr);
			m_ptr = NULL;
		}

		if (NULL != m_memMapping)
		{
			CloseHandle(m_memMapping);
			m_memMapping = NULL;
		}

		if (NULL != m_file)
		{
			CloseHandle(m_memMapping);
			m_file = INVALID_HANDLE_VALUE;
		}
	}

protected:
	HANDLE m_txtMapping;
	HANDLE m_memMapping;
	T* m_ptr;
};
