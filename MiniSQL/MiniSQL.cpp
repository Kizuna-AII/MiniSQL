#include "Structure.h"
#include "BufferManager.h"

//int main()
//{
//    std::cout << "Hello World!\n"; 
//	return 0;
//}

// sample program for BufferManager
int main()
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

	for (int handle = 6; handle <= 13; handle+=2)
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
	{}
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
	{}
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

	return 0;
}