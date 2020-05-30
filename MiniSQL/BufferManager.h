#pragma once
#pragma warning( disable : 4290 ) // VS会忽略throw(typename)
#include "Structure.h"
#include <map>
#include <set>


constexpr size_t DEFAULTHANDLE = (size_t)4294967295;
#define TEST

namespace Buffer
{
	typedef struct BlockNode //用于进行内存和文件交互的块
	{  
		size_t handle; //该块在block池中的句柄，唯一标识一块内存
		size_t size; //当前已使用的大小(byte)
		std::string buf; //该块对应的缓冲区。为了符合block要求，string的长度应控制在4096以内
		bool pin; // 是否被锁定
		std::string filename; //文件名
		std::streamsize fileOffset; //该块对于其所属文件的offset
	} *BlockNodeP;


	class BufferManager final
	{
#ifdef TEST
	public:
#endif
		BlockNodeP buffer[BLOCKNUM];
		size_t handleCount; // handle计数器，保证handle两两不同
		std::map<int, int> handleIndexMap; // handle到Index的map
		std::set<int> freeIndexSet; // 可被写的buffer页池

		void RefreshPage(const size_t & index) throw(const char *); // 为下标为index的buffer创建空间，如果已经存在则初始化参数，可能抛出已锁异常
		void ErasePage(const size_t & index) throw(const char *); // 对下标为index的buffer进行free， 可能抛出已锁或不存在异常
		size_t ToIndex(const size_t & handle) const throw(const char *); // 转换handle为buffer下标，可能抛出不存在异常
		
		size_t GetFreeIndex() const throw(const char *); // 寻找一个空余的buffer页，可能抛出可写页已满异常
	public:
		BufferManager() noexcept;
		~BufferManager() noexcept;
		void Initialize() noexcept; // 初始化整个Buffer Manager

		size_t NewPage() throw(const char *); // 获取一个新的buffer页，返回新页handle（保证唯一）
		size_t GetFreePageNum() const noexcept; // 获取当前空余页的数量

		void SetPin(const size_t & handle = DEFAULTHANDLE) throw(const char *); // 锁定对应handle的块
		void ResetPin(const size_t & handle = DEFAULTHANDLE) throw(const char *); // 解锁对应handle的块
		bool GetPin(const size_t & handle = DEFAULTHANDLE) throw(const char *); // 获取对应handle的块的锁定状态

		size_t Write(const std::string & content, const size_t & handle = DEFAULTHANDLE);
			// 向handle对应的块写入content，返回未写入的长度（全部写入成功返回0）
		const std::string & Read(const size_t & handle = DEFAULTHANDLE) const;
			// 向handle对应的块读取数据，返回常引用字符串
		size_t Modify(const std::string & content, const size_t pos, const size_t & handle = DEFAULTHANDLE);
			// 向handle对应的块，从下标pos开始，将内容替换为content，返回未修改的长度（全部修改成功返回0）
		void SetSize(const size_t size, const size_t & handle = DEFAULTHANDLE);
			// 将handle对应的块，从下标pos开始，将内容替换为'\000'，修改size值，实际capacity不变
		size_t GetSize(const size_t & handle = DEFAULTHANDLE) const;
			// 返回handle对应块的目前大小
		std::string & GetBuffer(const size_t & handle = DEFAULTHANDLE);
			// 向handle对应的块读取数据，返回字符串的直接引用（便于自由管理）

		void SetFilename(const std::string &filename, const size_t & handle = DEFAULTHANDLE);
			// 设置handle对应的块的I/O文件名
		std::string GetFilename(const size_t & handle = DEFAULTHANDLE) const;
			// 获得handle对应的块的I/O文件名
		void SetFileOffset(const std::streamsize & fileOffset, const size_t & handle = DEFAULTHANDLE);
			// 设置handle对应的块的I/O偏移
		std::streamsize GetFileOffset(const size_t & handle = DEFAULTHANDLE) const;
			// 获得handle对应的块的I/O偏移
		std::streamsize Load(const size_t & handle = DEFAULTHANDLE);
			// 令handle对应的块从二进制文件中读取一块数据，返回真实读取的字节数
		std::streamsize Save(const size_t & handle = DEFAULTHANDLE) const;
			// 令handle对应的块向二进制文件中写入一块数据，返回真实写入的字节数；若该文件不存在则创建文件
		void Delete(const size_t & handle = DEFAULTHANDLE) const;
			// 删除handle对应的块对应的硬盘文件
		bool IsExist(const size_t & handle = DEFAULTHANDLE) const;

	};

};

