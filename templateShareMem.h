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

			//GENERIC_READ | GENERIC_WRITE,//������ ���� ���� ���� : �а� ���� �Ѵ�
			//				//FILE_SHARE_READ | FILE_SHARE_WRITE,//������ ������� ���� : ���� ��� ����ϰ� �ִµ� �ٸ� ���μ������� ���ٽ� �б�, ���� ���
			//				//NULL,//������ ���� �Ӽ� �����ϴ� ����ü ������, dafault NULL
			//				//OPEN_ALWAYS,//�׻� ������ ���� �����. ������ ������ �ִٸ� ����� 
			//				//FILE_ATTRIBUTE_NORMAL,//  FILE_ATTRUBUTE_NORMAL : �ƹ��� �Ӽ��� ���� ������ ����.
			//				//NULL);//���� ���� ������ �Ӽ� ������ ���ø� ����, dafault NULL

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
