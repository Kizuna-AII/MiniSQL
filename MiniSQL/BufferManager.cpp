#include "BufferManager.h"

void Buffer::BufferManager::RefreshPage(const size_t & index) throw(const char *)
{
	if (buffer[index] != nullptr && buffer[index]->pin == true) // 尝试替换已锁页
		throw("Cannot Rewrite a PINNED Page");
	if (buffer[index] != nullptr) // 移除旧页
	{
		this->ErasePage(index);	
	}
	buffer[index] = new Buffer::BlockNode;
	buffer[index]->handle = ++this->handleCount;
	buffer[index]->size = 0;
	buffer[index]->pin = false;
	buffer[index]->buf.resize(BLOCKCAPACITY, '\000'); // 申请一块buffer
	buffer[index]->fileOffset = 0;
	handleIndexMap.insert(std::pair<int, int>(handleCount, index));

}

void Buffer::BufferManager::ErasePage(const size_t & index) throw(const char *)
{
	if (buffer[index] != nullptr && buffer[index]->pin == true) // 尝试删除已锁页
		throw("Cannot Erase a PINNED Page");
	if (buffer[index] == nullptr)
		throw("Cannot Erase an UNEXISTED Page");
	auto iter = handleIndexMap.find(buffer[index]->handle);
	handleIndexMap.erase(iter);
	free(buffer[index]);
}

size_t Buffer::BufferManager::ToIndex(const size_t & handle) const throw(const char *)
{
	if (handle == DEFAULTHANDLE) // 默认为当前句柄
		return ToIndex(handleCount);

	auto iter = handleIndexMap.find(handle);
	if (iter == handleIndexMap.end()) // 句柄不存在
	{
		throw ("Buffer Handle Does Not Exist");
	}
	return iter->second;
}

size_t Buffer::BufferManager::GetFreeIndex() const throw(const char *)
{
	if (this->GetFreePageNum() == 0) // 没有空页
		throw("Cannot Find Free Buffer");
	auto iter = freeIndexSet.begin();
	size_t index = *iter;
	return index;
}

Buffer::BufferManager::BufferManager() noexcept
{
	handleCount = 0;
	tmpBufferSize = BLOCKCAPACITY;
	for (size_t i = 0; i < BLOCKNUM; i++)
	{
		buffer[i] = nullptr;
		freeIndexSet.insert(i);
	}
}

Buffer::BufferManager::~BufferManager() noexcept
{
	for (size_t i = 0; i < BLOCKNUM; i++)
		if (buffer[i] != nullptr)
			free(buffer[i]);
}

void Buffer::BufferManager::Initialize() noexcept
{
	handleCount = 0;
	freeIndexSet.clear();
	handleIndexMap.clear();
	for (size_t i = 0; i < BLOCKNUM; i++)
	{
		if (buffer[i] != nullptr)
			free(buffer[i]);
		buffer[i] = nullptr;
		freeIndexSet.insert(i);
	}
}

size_t Buffer::BufferManager::NewPage() throw(const char *)
{
	size_t index = this->GetFreeIndex();
	this->RefreshPage(index);
	return handleCount;
}

size_t Buffer::BufferManager::GetFreePageNum() const noexcept
{
	return freeIndexSet.size();
}

void Buffer::BufferManager::SetPin(const size_t & handle) throw(const char *)
{
	size_t index = this->ToIndex(handle);
	buffer[index]->pin = true;
	auto iter = freeIndexSet.find(index);
	freeIndexSet.erase(iter);
}

void Buffer::BufferManager::ResetPin(const size_t & handle) throw(const char *)
{
	size_t index = this->ToIndex(handle);
	buffer[index]->pin = false;
	freeIndexSet.insert(index);
}

bool Buffer::BufferManager::GetPin(const size_t & handle) throw(const char *)
{
	size_t index = this->ToIndex(handle);
	return buffer[index]->pin;
}

size_t Buffer::BufferManager::Write(const std::string & content, const size_t & handle)
{
	size_t index = this->ToIndex(handle);
	std::string & buf = buffer[index]->buf;
	size_t len = min(content.size(), BLOCKCAPACITY - buffer[index]->size);
	buf.replace(buffer[index]->size, len, content.substr(0, len));
	buffer[index]->size += len;
	return size_t(content.size() - len);
}

const std::string & Buffer::BufferManager::Read(const size_t & handle) const
{
	size_t index = this->ToIndex(handle);
	return buffer[index]->buf;
}

size_t Buffer::BufferManager::Modify(const std::string & content, const size_t pos, const size_t & handle)
{
	size_t index = this->ToIndex(handle);
	std::string & buf = buffer[index]->buf;
	size_t len = min(content.size(), BLOCKCAPACITY - buffer[index]->size);
	buf.replace(pos, len, content.substr(0, len));
	buffer[index]->size = max(buffer[index]->size, len + pos);
	return size_t(content.size() - len);
}

void Buffer::BufferManager::SetSize(const size_t size, const size_t & handle)
{
	if (size > BLOCKCAPACITY) // resize过大
		throw("Buffer Set Size OUT of RANGE");
	size_t index = this->ToIndex(handle);
	std::string & buf = buffer[index]->buf;
	for (size_t i = size; i < BLOCKCAPACITY; i++)
		buffer[index]->buf[i] = '\000';
	buffer[index]->size = size;
}

size_t Buffer::BufferManager::GetSize(const size_t & handle) const
{
	size_t index = this->ToIndex(handle);
	return buffer[index]->size;
}

std::string & Buffer::BufferManager::GetBuffer(const size_t & handle)
{
	size_t index = this->ToIndex(handle);
	return buffer[index]->buf;
}

void Buffer::BufferManager::SetFilename(const std::string & filename, const size_t & handle)
{
	size_t index = this->ToIndex(handle);
	buffer[index]->filename = filename;
}

std::string Buffer::BufferManager::GetFilename(const size_t & handle) const
{
	size_t index = this->ToIndex(handle);
	return buffer[index]->filename;
}

void Buffer::BufferManager::SetFileOffset(const std::streamsize & fileOffset, const size_t & handle)
{
	size_t index = this->ToIndex(handle);
	buffer[index]->fileOffset = fileOffset;
}

std::streamsize Buffer::BufferManager::GetFileOffset(const size_t & handle) const
{
	size_t index = this->ToIndex(handle);
	return buffer[index]->fileOffset;
}

std::streamsize Buffer::BufferManager::Load(const size_t & handle)
{
	static char inBuf[BLOCKCAPACITY + 1];
	size_t index = this->ToIndex(handle);
	std::ifstream inFile(buffer[index]->filename, std::ios::in | std::ios::binary);
	if (!inFile.good())
		throw("Cannot Load File");

	inFile.seekg(buffer[index]->fileOffset, std::ios::beg); // 定位
	inFile.read(inBuf, BLOCKCAPACITY);
	
	std::streamsize readSize = inFile.gcount();
	buffer[index]->size = (size_t)readSize; // 更新大小
	for (std::streamsize i = 0; i < readSize; i++)
		buffer[index]->buf[i] = inBuf[i];
	//buffer[index]->buf.replace(0, (size_t)readSize, inBuf, 0, (size_t)readSize);
	//std::cout << "buf = " << buffer[index]->buf << std::endl;
	buffer[index]->buf.resize(Buffer::BLOCKCAPACITY, '\000'); // 维持size
	buffer[index]->fileOffset += buffer[index]->size;
	inFile.close();
	return readSize;
}

std::streamsize Buffer::BufferManager::Save(const size_t & handle) const
{
	size_t index = this->ToIndex(handle);
	std::ofstream outFile;
	if (_access(buffer[index]->filename.c_str(), 0)) // 没有找到文件
	{
		outFile.open(buffer[index]->filename, std::ios::out);
		outFile.close();
	}
	outFile.open(buffer[index]->filename, std::ios::in | std::ios::out | std::ios::binary);
	if (!outFile.good())
		throw("Cannot Save File");
	outFile.seekp(buffer[index]->fileOffset, std::ios::beg); // 定位
	outFile.write(buffer[index]->buf.c_str(), buffer[index]->size);
	buffer[index]->fileOffset += buffer[index]->size;
	outFile.close();
	return buffer[index]->size;
}

void Buffer::BufferManager::Delete(const size_t & handle) const
{
	size_t index = this->ToIndex(handle);
	remove(buffer[index]->filename.c_str());
}

bool Buffer::BufferManager::IsExist(const size_t & handle) const
{
	size_t index = this->ToIndex(handle);
	return !_access(buffer[index]->filename.c_str(), 0);
}

DWORD Buffer::BufferManager::GetFileSize(const size_t & handle) const
{
	wchar_t wFilename[100];
	swprintf(wFilename, 100, L"%hs", this->GetFilename(handle).c_str());
	HANDLE fileHandle = ::CreateFile(wFilename, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (fileHandle == INVALID_HANDLE_VALUE)
	{
		::CloseHandle(fileHandle);
		throw("Cannot Load File");
	}
	DWORD res = ::GetFileSize(fileHandle, NULL);
	::CloseHandle(fileHandle);
	return res;
}

void Buffer::BufferManager::SetFileSize(const DWORD & size, const size_t & handle)
{
	int fileHandle;
	::_sopen_s(&fileHandle, this->GetFilename(handle).c_str(), _O_RDWR, _SH_DENYNO, _S_IREAD | _S_IWRITE);
	if (fileHandle == -1)
	{
		::_close(fileHandle);
		throw("Cannot Load File");
	}
	::_chsize(fileHandle, (long)size);
	::_close(fileHandle);
}

void Buffer::BufferManager::TestMain()
{
	Buffer::BufferManager BM; // BM实体

	// 第一部分：申请buffer例程
	for (int i = 0; i < 5; i++)
	{
		size_t handle = BM.NewPage(); // NewPage()将返回一个可用的buffer的句柄handle
		// 对于每个页面，都有唯一的handle与之对应，可以视为一种标识符（handle可能失效，但是不会重复）
		std::cout << handle << ' ' << BM.GetFreePageNum() << std::endl; // GetFreePageNum()可以查看剩余可用页数
		// 注意到此时没有锁定buffer，因此每次都会覆盖前一次的页面，剩余可用页数一直是满的
	}
	std::cout << std::endl;

	for (int i = 0; i < 10; i++)
		try
	{
		size_t handle = BM.NewPage();
		BM.SetPin(handle); // 锁定该handle，可用页数-1，该页面将不会被替换
		std::cout << handle << ' ' << BM.GetFreePageNum() << std::endl;
		// 注意到此时锁定了buffer，因此每次都会申请新的页面，剩余可用页数一直是递减，直至用完
	}
	catch (const char *msg) // 当剩余可用页数为0时，尝试NewPage()将会抛出异常
	{
		std::cout << msg << std::endl;
	}
	std::cout << std::endl;

	for (int handle = 6; handle <= 13; handle += 2)
		BM.ResetPin(handle); // 解锁一部分buffer（上层buffer必须事先记录那些被锁定的handle，否则只有初始化能解锁buffer）

	for (int i = 0; i < 5; i++) // 可以观察到有部分页面被解锁
		try
	{
		size_t handle = BM.NewPage(); // NewPage()刚产生的buffer为“当前buffer”，“当前buffer”也只会被NewPage()修改
		BM.SetPin(handle); // 如果需要操作的buffer为“当前buffer”则可以缺省handle
		std::cout << handle << ' ' << BM.GetFreePageNum() << std::endl;
	}
	catch (const char *msg)
	{
		std::cout << msg << std::endl;
	}
	std::cout << std::endl;
	getchar();

	// Buffer内存读写
	BM.Initialize(); // 初始化所有buffer

	size_t handle = BM.NewPage();
	while (BM.Write("HelloWorld", handle) == 0) // 按顺序写入，返回值为未写入的字符个数
	{
	}
	std::cout << "!";
	const std::string & str = BM.Read(handle); // Read()返回对于buffer字符串的常引用
	std::cout << str.c_str() << std::endl;

	BM.NewPage(); // 同样地，临时操作可以省略handle
	BM.Write("19260817");
	BM.Modify("19810", 2); // 可以使用Modify()进行替换式修改
	std::cout << BM.Read().c_str() << " size = " << BM.GetSize() << std::endl;
	BM.SetSize(7); // 可以使用SetSize()截短数据（buffer的capacity不变）
	std::cout << BM.Read().c_str() << " size = " << BM.GetSize() << std::endl;
	BM.Write("114514");
	std::cout << BM.Read().c_str() << " size = " << BM.GetSize() << std::endl;
	std::string & sstr = BM.GetBuffer(); // 可以使用GetBuffer()获得Buffer字符串的直接引用
	sstr.replace(13, 4, "2333"); // 自由修改Buffer
	BM.SetSize(17); // 如果自由修改造成了长度改变必须手动维护size
	std::cout << BM.Read().c_str() << " size = " << BM.GetSize() << std::endl;

	getchar();



	// 文件读写
	BM.Initialize();

	std::streamsize offset = 0;
	BM.NewPage();
	BM.SetFilename("../test/out.txt"); // 设置文件名
	BM.Delete(); // 删除原有文件
	BM.Write("1145142333");
	BM.SetFileOffset(0); // 初始偏移为0
	offset += BM.Save(); // 保存并记录新的偏移

	BM.SetSize(0); // 仅清空buffer
	BM.SetFileOffset(0);
	std::cout << BM.IsExist() << std::endl; // 检查文件是否存在
	std::streamsize size = BM.Load(); // 读取文件并获得读取的字节数
	std::cout << BM.Read().c_str() << " size = " << size << std::endl;

	BM.SetSize(0);
	BM.SetFileOffset(offset); // 设置新偏移
	BM.Write("1919810");
	BM.Save();

	BM.SetSize(0);
	BM.SetFileOffset(0);
	std::cout << BM.IsExist() << std::endl;
	size = BM.Load();
	std::cout << BM.Read().c_str() << " size = " << size << std::endl;
	getchar();

	BM.SetSize(0);
	BM.SetFileOffset(0);
	while (BM.Write("HelloWorld") == 0)
	{
	}
	BM.SetFilename("../test/big.txt");
	BM.Delete();
	BM.Save();
	BM.Save(); // 连续读取/写入可以自动设置偏移
	BM.SetSize(8); // "HelloWor"
	BM.Save();

	BM.SetSize(0);
	BM.SetFileOffset(0);
	std::cout << BM.IsExist() << std::endl;
	while (BM.Load() != 0) // 连续读入
	{
		const std::string &str = BM.Read();
		std::cout << str.c_str() << "\nsize = " << BM.GetSize() << std::endl;
		BM.SetSize(0);
	}

	getchar();

	// 文件大小修改
	BM.Initialize();
	BM.NewPage();
	BM.SetFilename("../test/big.txt");
	std::cout << "Old File Size = " << BM.GetFileSize() << std::endl;
	BM.SetFileSize(10);
	std::cout << "New File Size = " << BM.GetFileSize() << std::endl;

	getchar();
}
