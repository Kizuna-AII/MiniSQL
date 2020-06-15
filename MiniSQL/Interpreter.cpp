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
		if (ch == '"') {//检测两端引号
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

std::vector<Common::Compares> Interpreter::GetConditions(std::istream & fin)
{
	throw(API::not_completed_exception(""));
	return std::vector<Common::Compares>();
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
					if (res[i].name == "tmp") {
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
			if (tmp == ",")fin.putback(',');//检查是否有unique约束
			else if (tmp == "unique") {
				attri.unique = 1;
			}
			res.push_back(attri);
		}
		GetString(fin, tmp);
		if (tmp[0] != ',')flag = 1;//如果没有逗号，认为语句结束
	} while (!flag);
	return res;
}

std::string Interpreter::GetValues(std::istream & fin)
{
	throw(API::not_completed_exception(""));
	return std::string();
}
