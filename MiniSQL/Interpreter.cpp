#include "Interpreter.h"
using namespace Common;
using namespace std;
void Interpreter::ClearCommand(std::istream & fin){
	char ch = 0;
	int cnt = 0;//����ƥ�䣬��ֹ�ַ����ڵķֺű�ʶ��Ϊ������
	while (fin.get(ch) && (ch != ';' || cnt != 0)) {
		fin.get(ch);
		if (ch == '"')cnt ^= 1;
	}
	return;
}
void Interpreter::GetString(std::istream &fin, std::string &str) {
	str.clear();
	char ch;
	bool flag = 0;//�����ո����
	while (fin.get(ch)) {
		if (ch == '(' || ch == ')') {
			ch = ' ';//������Ϊ�ո�
		}
		if (ch == ' ') {
			if (!flag) {
				continue;//����ǰ���ո�
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
		if (ch == '(' || ch == ')')continue;//��������
		if (ch == ';') {
			fin.putback(ch);
			return;
		}
		if (ch == '"') {//�����������
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
		if (tmp == "primary") {//����primary key
			GetString(fin, tmp);
			if (tmp == "key") {
				GetString(fin, tmp);//��ȡ����ֵ
				for (int i = 0; i < res.size(); i++) {//��������
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
		else {//��tmp����������������ͨ��
			Attribute attri;
			attri.unique = attri.primary = 0;
			attri.indexName = "#NULL#";
			attri.name = tmp;
			GetString(fin, tmp);//��������
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
			else throw(API::wrong_command_error("���Զ�ȡ����"));
			GetString(fin, tmp);
			if (tmp == ",")fin.putback(',');//����Ƿ���uniqueԼ��
			else if (tmp == "unique") {
				attri.unique = 1;
			}
			res.push_back(attri);
		}
		GetString(fin, tmp);
		if (tmp[0] != ',')flag = 1;//���û�ж��ţ���Ϊ������
	} while (!flag);
	return res;
}

std::string Interpreter::GetValues(std::istream & fin)
{
	throw(API::not_completed_exception(""));
	return std::string();
}
