#include<iostream>
#include<cstdio>
#include<cstring>
#include<string>
#include<exception>
#include<map>
#include "API.h"
#include "Interpreter.h"

using namespace std;
using namespace API;

#define USEFILECOMMAND (true)
#define USEFILELOG (true)
#define TESTPATH (std::string("../test/"))
#define LOGFILENAME (std::string("log.txt"))

const std::string DEFAULTCOMMANDFILENAME("command.txt");

std::string COMMANDFILENAME(DEFAULTCOMMANDFILENAME);

class miniSQL
{
private:
	map<string, int>parsemap;
	Api* api;
	Interpreter* inter;
	void Initialize() {
		parsemap.clear();
		parsemap["create"] = 1;
		parsemap["table"] = 2;
		parsemap["index"] = 3;
		parsemap["drop"] = 4;
		parsemap["select"] = 5;
		parsemap["insert"] = 6;
		parsemap["delete"] = 7;
		parsemap["quit"] = 8;
		parsemap["execfile"] = 9;
		parsemap[""] = 10;
		return;
	}
	int Parse_once(istream &fin) {
		if (fin.peek() == ';')return -1;
		fin >> skipws;
		string comm;
		fin >> comm;
		map<string, int>::iterator iter = parsemap.find(comm);
		if (iter == parsemap.end()) {//指令错误
			return -1;
		}
		return iter->second;
	}
public:
	miniSQL() {
		this->Initialize();
		api = new Api();
		inter = new Interpreter();
		inter->api = api;
		return;
	}
	//运行数据库终端，需指定:输入流，输出流，运行结束提示信息
	void Run(istream &fin, ostream &fout, string finflag) {
		cout << "miniSQL running" << endl;
		bool flag = 1;
		do {
			int x = Parse_once(fin);
			try {
				switch (x) {
				case 1: {//create
					switch (Parse_once(fin)) {
					case 2: {//create table
						string tName;
						inter->GetString(fin, tName);
						vector<Common::Attribute> attributes = inter->GetAttributes(fin);
						inter->PeekEnd(fin);
						api->CreateTable(tName, attributes);
						break;
					}
					case 3: {//create index
						string indexName, tableName, attriName;
						inter->GetString(fin, indexName);//获取index名
						inter->GetString(fin, tableName);
						if (tableName == "on") {//则读取table名
							inter->GetString(fin, tableName);
							inter->GetString(fin, attriName);
						}
						else {
							throw(wrong_command_error("no table name"));
						}
						api->CreateIndex(indexName, tableName, attriName);
						inter->ClearCommand(fin);//跳过末尾';'
						break;
					}
					default: {
						inter->ClearCommand(fin);//抛弃整个错误语句
						throw(wrong_command_error(""));
						break;
					}
					}
					break;
				}
				case 4: {//drop
					switch (Parse_once(fin)) {
					case 2: {//drop table
						string tableName, temp;
						inter->GetString(fin, tableName);//获取table名
						api->DropTable(tableName);
						inter->ClearCommand(fin);//跳过末尾';'
						break;
					}
					case 3: {//drop index
						string indexName, temp;
						inter->GetString(fin, indexName);//获取index名
						inter->GetString(fin, temp);
						if (temp == "on") {//如果指定table，则读取table名
							inter->GetString(fin, temp);
						}
						else {
							temp = "";
						}
						api->DropIndex(indexName, temp);
						inter->ClearCommand(fin);//跳过末尾';'
						break;
					}
					default: {
						inter->ClearCommand(fin);//抛弃整个错误语句
						throw(wrong_command_error(""));
						break;
					}
					}
					break;
				}
				case 5: {//select
					string temp, tableName;
					vector<Common::Compares> conditions;
					inter->GetString(fin, temp);
					if (temp != "*")throw(wrong_command_error(""));
					inter->GetString(fin, temp);
					if (temp != "from")throw(wrong_command_error(""));
					inter->GetString(fin, tableName);
					if (!inter->PeekEnd(fin)) {
						inter->GetString(fin, temp);
						if (temp != "where")throw(wrong_command_error(""));
						conditions = inter->GetConditions(fin);//获取选择条件
						api->Select(tableName, &conditions);
					}
					else {
						api->Select(tableName, NULL);//无条件选择
					}
					api->OutPutResult(tableName, fout);
					break;
				}
				case 6: {//insert
					string temp, tableName;
					inter->GetString(fin, temp);
					if (temp != "into")throw(wrong_command_error(""));
					inter->GetString(fin, tableName);//读取表名
					inter->GetString(fin, temp);
					if (temp != "values")throw(wrong_command_error(""));
					Common::Tuple tuple = inter->GetTuple(fin, tableName);//读取values
					inter->PeekEnd(fin);
					api->Insert(tuple, tableName);//insert
					break;
				}
				case 7: {//delete
					string temp, tableName;
					vector<Common::Compares> conditions;
					inter->GetString(fin, temp);
					if (temp != "from")throw(wrong_command_error(""));
					inter->GetString(fin, tableName);
					if (!inter->PeekEnd(fin)) {
						inter->GetString(fin, temp);
						if (temp != "where")throw(wrong_command_error(""));
						conditions = inter->GetConditions(fin);//获取删除条件
						api->Delete(tableName, &conditions);
					}
					else {
						api->Delete(tableName, NULL);//无条件删除
					}
					break;
				}
				case 8: {//quit
					flag = 0;
					break;
				}
				case 9: {//exec
					char fname[30];
					fin.getline(fname, 30, ';');
					ifstream tmpin(fname);//打开对应文件
					Run(tmpin, fout, "exec fin");//递归处理文件读取
					break;
				}
				case 10: {
					fin.clear();
					break;
				}
				default: {
					inter->ClearCommand(fin);//抛弃整个错误语句
					throw(wrong_command_error(""));
					break;
				}
				}
			}
			catch (mexception &ex) {
				cout << ex.what() << endl;
				//cout << "wtf" << endl;
			}
			if (USEFILELOG)
			{
				FILE * tempStream;
				fclose(stdout);
				freopen_s(&tempStream, (TESTPATH + LOGFILENAME).c_str(), "a", stdout);
			}

		} while (flag);
		cout << finflag << endl;
		return;
	}
};


int main(int argc, char **argv) {
	/// initialize I/O
	if (argc >= 2) // 指定Command Filename
	{
		COMMANDFILENAME = argv[1];
	}
	FILE * tempStream;
	std::ios::sync_with_stdio(false);
	if (USEFILECOMMAND)
	{
		freopen_s(&tempStream, (TESTPATH + COMMANDFILENAME).c_str(), "r", stdin);
		std::cout << "Ues File Command (Filename = " << (TESTPATH + COMMANDFILENAME).c_str() << ")" << std::endl;
	}

	if (USEFILELOG)
		freopen_s(&tempStream, (TESTPATH + LOGFILENAME).c_str(), "w", stdout);
	/// start
	miniSQL* tar = new miniSQL();
	tar->Run(std::cin, std::cout, "quit");
	if (USEFILECOMMAND)
	{
		fclose(stdin);
		freopen_s(&tempStream, "CON", "r", stdin);
	}
	if (USEFILELOG)
	{
		fclose(stdout);
		freopen_s(&tempStream, "CON", "w", stdout);
	}
	free(tar);
	return 0;
}


/*
// sample program for BufferManager
int bufmain()
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
	
	// 文件大小修改
	BM.Initialize();
	BM.NewPage();
	BM.SetFilename("../test/big.txt");
	std::cout << "Old File Size = " << BM.GetFileSize() << std::endl;
	BM.SetFileSize(10);
	std::cout << "New File Size = " << BM.GetFileSize() << std::endl;

	getchar();

	return 0;
}
*/
/*
// sample program for catalog manager
int catalogmain()
{
	Buffer::BufferManager* BMP = new Buffer::BufferManager();
	BMP->Initialize();
	Catalog::CatalogManager* CMP = new Catalog::CatalogManager();
	CMP->Initialization(BMP);
	Common::Table* table = new Common::Table();
	table->name = "first_table";
	Common::Attribute attr;
	attr.indexName = "yes";
	attr.name = "first_column";
	attr.primary = true;
	attr.type = 0;
	attr.unique = true;
	table->attributes.push_back(attr);
	attr.name = "second_column";
	attr.indexName = Catalog::noIndex;
	attr.type = -1;
	table->attributes.push_back(attr);
	CMP->ShowTables();
	std::cout << CMP->CreateTable(table) << std::endl;
	std::cout << CMP->FindTable("first") << std::endl;
	std::cout << CMP->FindTable("first_table") << std::endl;
	std::cout << CMP->CreateIndex("first_table", "first_column", "yes") << std::endl;
	std::cout << CMP->CreateIndex("first_table", "second_column", "newindex") << std::endl;
	std::cout << CMP->FindIndex("first_table", "newindex") << std::endl;
	std::cout << CMP->FindTable("first_table") << std::endl;
	std::vector<std::string> temp = CMP->ShowIndex();
	for (int i = 0; i <= temp.size() - 1; i++)
	{
		std::cout << temp[i] << std::endl;
	}
	return 0;
}*/
