#include "BufferManager.h"

void Buffer::BufferManager::RefreshPage(const size_t & index) throw(const char *)
{
	if (buffer[index] != nullptr && buffer[index]->pin == true) // �����滻����ҳ
		throw("Cannot Rewrite a PINNED Page");
	if (buffer[index] != nullptr) // �Ƴ���ҳ
	{
		this->ErasePage(index);	
	}
	buffer[index] = new Buffer::BlockNode;
	buffer[index]->handle = ++this->handleCount;
	buffer[index]->size = 0;
	buffer[index]->pin = false;
	buffer[index]->buf.resize(BLOCKCAPACITY, '\000'); // ����һ��buffer
	buffer[index]->fileOffset = 0;
	handleIndexMap.insert(std::pair<int, int>(handleCount, index));

}

void Buffer::BufferManager::ErasePage(const size_t & index) throw(const char *)
{
	if (buffer[index] != nullptr && buffer[index]->pin == true) // ����ɾ������ҳ
		throw("Cannot Erase a PINNED Page");
	if (buffer[index] == nullptr)
		throw("Cannot Erase an UNEXISTED Page");
	auto iter = handleIndexMap.find(buffer[index]->handle);
	handleIndexMap.erase(iter);
	free(buffer[index]);
}

size_t Buffer::BufferManager::ToIndex(const size_t & handle) const throw(const char *)
{
	if (handle == DEFAULTHANDLE) // Ĭ��Ϊ��ǰ���
		return ToIndex(handleCount);

	auto iter = handleIndexMap.find(handle);
	if (iter == handleIndexMap.end()) // ���������
	{
		throw ("Buffer Handle Does Not Exist");
	}
	return iter->second;
}

size_t Buffer::BufferManager::GetFreeIndex() const throw(const char *)
{
	if (this->GetFreePageNum() == 0) // û�п�ҳ
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
	if (size > BLOCKCAPACITY) // resize����
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

	inFile.seekg(buffer[index]->fileOffset, std::ios::beg); // ��λ
	inFile.read(inBuf, BLOCKCAPACITY);
	
	std::streamsize readSize = inFile.gcount();
	buffer[index]->size = (size_t)readSize; // ���´�С
	for (std::streamsize i = 0; i < readSize; i++)
		buffer[index]->buf[i] = inBuf[i];
	//buffer[index]->buf.replace(0, (size_t)readSize, inBuf, 0, (size_t)readSize);
	//std::cout << "buf = " << buffer[index]->buf << std::endl;
	buffer[index]->buf.resize(Buffer::BLOCKCAPACITY, '\000'); // ά��size
	buffer[index]->fileOffset += buffer[index]->size;
	inFile.close();
	return readSize;
}

std::streamsize Buffer::BufferManager::Save(const size_t & handle) const
{
	size_t index = this->ToIndex(handle);
	std::ofstream outFile;
	if (_access(buffer[index]->filename.c_str(), 0)) // û���ҵ��ļ�
	{
		outFile.open(buffer[index]->filename, std::ios::out);
		outFile.close();
	}
	outFile.open(buffer[index]->filename, std::ios::in | std::ios::out | std::ios::binary);
	if (!outFile.good())
		throw("Cannot Save File");
	outFile.seekp(buffer[index]->fileOffset, std::ios::beg); // ��λ
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
	Buffer::BufferManager BM; // BMʵ��

	// ��һ���֣�����buffer����
	for (int i = 0; i < 5; i++)
	{
		size_t handle = BM.NewPage(); // NewPage()������һ�����õ�buffer�ľ��handle
		// ����ÿ��ҳ�棬����Ψһ��handle��֮��Ӧ��������Ϊһ�ֱ�ʶ����handle����ʧЧ�����ǲ����ظ���
		std::cout << handle << ' ' << BM.GetFreePageNum() << std::endl; // GetFreePageNum()���Բ鿴ʣ�����ҳ��
		// ע�⵽��ʱû������buffer�����ÿ�ζ��Ḳ��ǰһ�ε�ҳ�棬ʣ�����ҳ��һֱ������
	}
	std::cout << std::endl;

	for (int i = 0; i < 10; i++)
		try
	{
		size_t handle = BM.NewPage();
		BM.SetPin(handle); // ������handle������ҳ��-1����ҳ�潫���ᱻ�滻
		std::cout << handle << ' ' << BM.GetFreePageNum() << std::endl;
		// ע�⵽��ʱ������buffer�����ÿ�ζ��������µ�ҳ�棬ʣ�����ҳ��һֱ�ǵݼ���ֱ������
	}
	catch (const char *msg) // ��ʣ�����ҳ��Ϊ0ʱ������NewPage()�����׳��쳣
	{
		std::cout << msg << std::endl;
	}
	std::cout << std::endl;

	for (int handle = 6; handle <= 13; handle += 2)
		BM.ResetPin(handle); // ����һ����buffer���ϲ�buffer�������ȼ�¼��Щ��������handle������ֻ�г�ʼ���ܽ���buffer��

	for (int i = 0; i < 5; i++) // ���Թ۲쵽�в���ҳ�汻����
		try
	{
		size_t handle = BM.NewPage(); // NewPage()�ղ�����bufferΪ����ǰbuffer��������ǰbuffer��Ҳֻ�ᱻNewPage()�޸�
		BM.SetPin(handle); // �����Ҫ������bufferΪ����ǰbuffer�������ȱʡhandle
		std::cout << handle << ' ' << BM.GetFreePageNum() << std::endl;
	}
	catch (const char *msg)
	{
		std::cout << msg << std::endl;
	}
	std::cout << std::endl;
	getchar();

	// Buffer�ڴ��д
	BM.Initialize(); // ��ʼ������buffer

	size_t handle = BM.NewPage();
	while (BM.Write("HelloWorld", handle) == 0) // ��˳��д�룬����ֵΪδд����ַ�����
	{
	}
	std::cout << "!";
	const std::string & str = BM.Read(handle); // Read()���ض���buffer�ַ����ĳ�����
	std::cout << str.c_str() << std::endl;

	BM.NewPage(); // ͬ���أ���ʱ��������ʡ��handle
	BM.Write("19260817");
	BM.Modify("19810", 2); // ����ʹ��Modify()�����滻ʽ�޸�
	std::cout << BM.Read().c_str() << " size = " << BM.GetSize() << std::endl;
	BM.SetSize(7); // ����ʹ��SetSize()�ض����ݣ�buffer��capacity���䣩
	std::cout << BM.Read().c_str() << " size = " << BM.GetSize() << std::endl;
	BM.Write("114514");
	std::cout << BM.Read().c_str() << " size = " << BM.GetSize() << std::endl;
	std::string & sstr = BM.GetBuffer(); // ����ʹ��GetBuffer()���Buffer�ַ�����ֱ������
	sstr.replace(13, 4, "2333"); // �����޸�Buffer
	BM.SetSize(17); // ��������޸�����˳��ȸı�����ֶ�ά��size
	std::cout << BM.Read().c_str() << " size = " << BM.GetSize() << std::endl;

	getchar();



	// �ļ���д
	BM.Initialize();

	std::streamsize offset = 0;
	BM.NewPage();
	BM.SetFilename("../test/out.txt"); // �����ļ���
	BM.Delete(); // ɾ��ԭ���ļ�
	BM.Write("1145142333");
	BM.SetFileOffset(0); // ��ʼƫ��Ϊ0
	offset += BM.Save(); // ���沢��¼�µ�ƫ��

	BM.SetSize(0); // �����buffer
	BM.SetFileOffset(0);
	std::cout << BM.IsExist() << std::endl; // ����ļ��Ƿ����
	std::streamsize size = BM.Load(); // ��ȡ�ļ�����ö�ȡ���ֽ���
	std::cout << BM.Read().c_str() << " size = " << size << std::endl;

	BM.SetSize(0);
	BM.SetFileOffset(offset); // ������ƫ��
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
	BM.Save(); // ������ȡ/д������Զ�����ƫ��
	BM.SetSize(8); // "HelloWor"
	BM.Save();

	BM.SetSize(0);
	BM.SetFileOffset(0);
	std::cout << BM.IsExist() << std::endl;
	while (BM.Load() != 0) // ��������
	{
		const std::string &str = BM.Read();
		std::cout << str.c_str() << "\nsize = " << BM.GetSize() << std::endl;
		BM.SetSize(0);
	}

	getchar();

	// �ļ���С�޸�
	BM.Initialize();
	BM.NewPage();
	BM.SetFilename("../test/big.txt");
	std::cout << "Old File Size = " << BM.GetFileSize() << std::endl;
	BM.SetFileSize(10);
	std::cout << "New File Size = " << BM.GetFileSize() << std::endl;

	getchar();
}
