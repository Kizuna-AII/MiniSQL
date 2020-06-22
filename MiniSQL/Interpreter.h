#pragma once
#include<iostream>
#include<cstdio>
#include<cstring>
#include<string>
#include<vector>
#include "Structure.h"
#include "API.h"
class Interpreter
{
public:
	API::Api *api;//interpreterҪ�󶨵�api
	//������ȡ�ļ���������';'��ֹͣ��������������䷨ʱ���������������
	void ClearCommand(std::istream &fin);
	//��ȡһ���ַ�������������Ϊ�ո����ĩβ�н�����';'��ָ���','���򽫸÷��ŷŻ�������
	void GetString(std::istream &fin, std::string &str);
	//��ȡ�ļ����ĵ�һ���ǿո��ַ������ǽ�����';'��ָ���','����������������1������Żز�����0
	int PeekEnd(std::istream &fin);
	//��ȡ���ݿ�char()���͵�value
	void GetCharValue(std::istream &fin, std::string &str);
	//����һ��value������ΪTuple
	Common::Tuple GetTuple(std::istream & fin, std::string tableName);
	//��ȡwhere����Ӧ��condition
	std::vector<Common::Compares> GetConditions(std::istream &fin);
	//��ȡcreate table��Ҫ��attributes
	std::vector<Common::Attribute> GetAttributes(std::istream &fin);
};

