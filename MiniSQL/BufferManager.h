#pragma once
#pragma warning( disable : 4290 ) // VS�����throw(typename)
#include "Structure.h"
#include <map>
#include <set>


constexpr size_t DEFAULTHANDLE = (size_t)4294967295;
#define TEST

namespace Buffer
{
	typedef struct BlockNode //���ڽ����ڴ���ļ������Ŀ�
	{  
		size_t handle; //�ÿ���block���еľ����Ψһ��ʶһ���ڴ�
		size_t size; //��ǰ��ʹ�õĴ�С(byte)
		std::string buf; //�ÿ��Ӧ�Ļ�������Ϊ�˷���blockҪ��string�ĳ���Ӧ������4096����
		bool pin; // �Ƿ�����
		std::string filename; //�ļ���
		std::streamsize fileOffset; //�ÿ�����������ļ���offset
	} *BlockNodeP;


	class BufferManager final
	{
#ifdef TEST
	public:
#endif
		BlockNodeP buffer[BLOCKNUM];
		size_t handleCount; // handle����������֤handle������ͬ
		std::map<int, int> handleIndexMap; // handle��Index��map
		std::set<int> freeIndexSet; // �ɱ�д��bufferҳ��

		void RefreshPage(const size_t & index) throw(const char *); // Ϊ�±�Ϊindex��buffer�����ռ䣬����Ѿ��������ʼ�������������׳������쳣
		void ErasePage(const size_t & index) throw(const char *); // ���±�Ϊindex��buffer����free�� �����׳������򲻴����쳣
		size_t ToIndex(const size_t & handle) const throw(const char *); // ת��handleΪbuffer�±꣬�����׳��������쳣
		
		size_t GetFreeIndex() const throw(const char *); // Ѱ��һ�������bufferҳ�������׳���дҳ�����쳣
	public:
		BufferManager() noexcept;
		~BufferManager() noexcept;
		void Initialize() noexcept; // ��ʼ������Buffer Manager

		size_t NewPage() throw(const char *); // ��ȡһ���µ�bufferҳ��������ҳhandle����֤Ψһ��
		size_t GetFreePageNum() const noexcept; // ��ȡ��ǰ����ҳ������

		void SetPin(const size_t & handle = DEFAULTHANDLE) throw(const char *); // ������Ӧhandle�Ŀ�
		void ResetPin(const size_t & handle = DEFAULTHANDLE) throw(const char *); // ������Ӧhandle�Ŀ�
		bool GetPin(const size_t & handle = DEFAULTHANDLE) throw(const char *); // ��ȡ��Ӧhandle�Ŀ������״̬

		size_t Write(const std::string & content, const size_t & handle = DEFAULTHANDLE);
			// ��handle��Ӧ�Ŀ�д��content������δд��ĳ��ȣ�ȫ��д��ɹ�����0��
		const std::string & Read(const size_t & handle = DEFAULTHANDLE) const;
			// ��handle��Ӧ�Ŀ��ȡ���ݣ����س������ַ���
		size_t Modify(const std::string & content, const size_t pos, const size_t & handle = DEFAULTHANDLE);
			// ��handle��Ӧ�Ŀ飬���±�pos��ʼ���������滻Ϊcontent������δ�޸ĵĳ��ȣ�ȫ���޸ĳɹ�����0��
		void SetSize(const size_t size, const size_t & handle = DEFAULTHANDLE);
			// ��handle��Ӧ�Ŀ飬���±�pos��ʼ���������滻Ϊ'\000'���޸�sizeֵ��ʵ��capacity����
		size_t GetSize(const size_t & handle = DEFAULTHANDLE) const;
			// ����handle��Ӧ���Ŀǰ��С
		std::string & GetBuffer(const size_t & handle = DEFAULTHANDLE);
			// ��handle��Ӧ�Ŀ��ȡ���ݣ������ַ�����ֱ�����ã��������ɹ���

		void SetFilename(const std::string &filename, const size_t & handle = DEFAULTHANDLE);
			// ����handle��Ӧ�Ŀ��I/O�ļ���
		std::string GetFilename(const size_t & handle = DEFAULTHANDLE) const;
			// ���handle��Ӧ�Ŀ��I/O�ļ���
		void SetFileOffset(const std::streamsize & fileOffset, const size_t & handle = DEFAULTHANDLE);
			// ����handle��Ӧ�Ŀ��I/Oƫ��
		std::streamsize GetFileOffset(const size_t & handle = DEFAULTHANDLE) const;
			// ���handle��Ӧ�Ŀ��I/Oƫ��
		std::streamsize Load(const size_t & handle = DEFAULTHANDLE);
			// ��handle��Ӧ�Ŀ�Ӷ������ļ��ж�ȡһ�����ݣ�������ʵ��ȡ���ֽ���
		std::streamsize Save(const size_t & handle = DEFAULTHANDLE) const;
			// ��handle��Ӧ�Ŀ���������ļ���д��һ�����ݣ�������ʵд����ֽ����������ļ��������򴴽��ļ�
		void Delete(const size_t & handle = DEFAULTHANDLE) const;
			// ɾ��handle��Ӧ�Ŀ��Ӧ��Ӳ���ļ�
		bool IsExist(const size_t & handle = DEFAULTHANDLE) const;

	};

};

