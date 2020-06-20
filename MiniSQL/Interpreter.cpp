#include "Interpreter.h"
using namespace Common;
using namespace std;
void Interpreter::ClearCommand(std::istream & fin){
	char ch = 0;
	int cnt = 0;//括号匹配，防止字符串内的分号被识别为结束符
	while (fin.get(ch) && (ch != ';' || cnt != 0)) {
		fin.get(ch);
		if (ch == '"')cnt ^= 1;
	}
	return;
}
void Interpreter::GetString(std::istream &fin, std::string &str) {
	str.clear();
	char ch;
	bool flag = 0;//遇到空格结束
	while (fin.get(ch)) {
		if (ch == '\n' || ch == '\r')continue;
		if (ch == '(' || ch == ')') {
			ch = ' ';//括号视为空格
		}
		if (ch == ' ') {
			if (!flag) {
				continue;//忽略前导空格
			}
			else return;
		}
		if (ch == ';' || ch==',') {
			fin.putback(ch);
			return;
		}
		str += ch;
		flag = 1;
	}
	return;
}

int Interpreter::PeekEnd(std::istream & fin){
	char ch;
	ch = fin.peek();
	while (ch == ' ') { fin >> ch; ch = fin.peek(); }
	if (ch == EOF || ch == ',' || ch == ';') {
		fin >> ch;
		return 1;
	}
	return 0;
}

void Interpreter::GetCharValue(std::istream & fin, std::string & str){
	str.clear();
	char ch;
	bool flag = 0;
	while (fin.get(ch)) {
		if (ch == '(' || ch == ')')continue;//跳过括号
		if (ch == ';') {
			fin.putback(ch);
			return;
		}
		if (ch == '\"' || ch=='\'') {//检测两端引号
			if (flag) {
				return;
			}
			flag = 1;
		}
		if (flag) {
			str += ch;
		}
	}
	return;
}

Common::Tuple Interpreter::GetTuple(std::istream & fin,std::string tableName)
{
	Table* table=api->GetTableByName(tableName);
	Common::Tuple res(*table);//根据表头申请空间
	int offset = 0;
	for (int i = 0; i < table->attributes.size(); i++) {
		char ch;
		ch = fin.peek();
		while (ch == ' ' || ch=='(') { fin >> ch; ch = fin.peek(); }//跳过空格和左括号
		if (table->attributes[i].type == -1) {//float
			this->PeekEnd(fin);
			float fl;
			fin >> fl;
			res.Set<float>(offset, fl);
			offset += sizeof(float);
		}
		else if (table->attributes[i].type == 0) {//int
			int num;
			fin >> num;
			res.Set<int>(offset, num);
			offset += sizeof(int);
		}
		else if (table->attributes[i].type > 0) {//char
			string str;
			this->GetCharValue(fin, str);
			str.resize(table->attributes[i].type, 0);//长度不足时填充
			res.Set(offset, str);
			offset += str.size();
		}
	}
	return res;
}

std::vector<Common::Compares> Interpreter::GetConditions(std::istream & fin)
{
	vector<Compares> res;
	res.clear();
	Compares tmpCmp;
	string temp;
	bool flag = 0;
	do {
		this->GetString(fin,temp);
		tmpCmp.attri = temp;
		this->GetString(fin, temp);
		if (temp == ">") {
			tmpCmp.ctype = CompareType::ja;
		}
		else if (temp == ">=") {
			tmpCmp.ctype = CompareType::jae;
		}
		else if (temp == "==") {
			tmpCmp.ctype = CompareType::je;
		}
		else if (temp == "!=") {
			tmpCmp.ctype = CompareType::jne;
		}
		else if (temp == "<") {
			tmpCmp.ctype = CompareType::jb;
		}
		else if (temp == "<=") {
			tmpCmp.ctype = CompareType::jbe;
		}
		else {
			throw(API::wrong_command_error("wrong symbols"));
		}
		this->GetString(fin, temp);
		tmpCmp.value = temp;
		if (temp[0] == '\'')temp.erase(0);//如果是字符串，消除首尾引号
		if (temp[temp.length() - 1] == '\'')temp.erase(temp.length() - 1);
		res.push_back(tmpCmp);
		this->GetString(fin, temp);//检查条件是否结束
		if (temp == "and") {
			continue;
		}
		else {
			this->PeekEnd(fin);//终止
			flag = 1;
		}
	} while (!flag);
	return res;
}

std::vector<Common::Attribute> Interpreter::GetAttributes(std::istream & fin)
{
	vector<Attribute>res;
	res.clear();
	bool flag = 0;
	string tmp;
	do {
		GetString(fin, tmp);
		if (tmp == "primary") {//处理primary key
			GetString(fin, tmp);
			if (tmp == "key") {
				GetString(fin, tmp);//获取主键值
				for (int i = 0; i < res.size(); i++) {//暴力查找
					if (res[i].name == tmp) {
						res[i].primary = 1;
						break;
					}
				}
			}
			else {
				throw(API::wrong_command_error(""));
			}
		}
		else {//将tmp视作列名，处理普通列
			Attribute attri;
			attri.unique = attri.primary = 0;
			attri.indexName = "#NULL#";
			attri.name = tmp;
			GetString(fin, tmp);//读入类型
			if (tmp == "int") {
				attri.type = AttributeType::intT;
			}
			else if (tmp == "float") {
				attri.type = AttributeType::floatT;
			}
			else if (tmp == "char") {
				int x; fin >> x;
				attri.type = x;
			}
			else throw(API::wrong_command_error("属性读取错误"));
			GetString(fin, tmp);
			if (tmp == "unique") {
				attri.unique = 1;
			}
			res.push_back(attri);
		}
		GetString(fin, tmp);
		if (fin.peek() != ',')flag = 1;//如果没有逗号，认为语句结束
		else {
			PeekEnd(fin);
		}
	} while (!flag);
	return res;
}
