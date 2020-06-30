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

#define USEFILECOMMAND (false)
#define USEFILELOG (false)
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
						vector<Common::Attribute> attributes = inter->GetAttributes(fin, tName);
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
					char fname[30], ch;
					while (fin.get(ch))
						if (ch != ' ' && ch != '\t')
						{
							fin.putback(ch);
							break;
						}
					fin.getline(fname, 30, ';');
					std::cout << "fname = " << fname << "!" << std::endl;
					ifstream tmpin(fname);//打开对应文件
					if (!tmpin) // 打开失败
						throw(wrong_command_error("No Such Command File"));
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
				std::cout << "Command Success!" << std::endl;
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