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
		if (ch == ';') {
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
	throw(API::not_completed_exception());
	return std::vector<Common::Compares>();
}

std::vector<Common::Attribute> Interpreter::GetAttributes(std::istream & fin)
{
	vector<Attribute>res;
	res.clear();

	throw(API::not_completed_exception());
	return std::vector<Common::Attribute>();
}

std::string Interpreter::GetValues(std::istream & fin)
{
	throw(API::not_completed_exception());
	return std::string();
}
