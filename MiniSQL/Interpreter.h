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
	//��ȡһ���ַ��������ĩβ�н�����';'���򽫸÷��ŷŻ�������
	void GetString(std::istream &fin, std::string &str);
	//��ȡ���ݿ�char()���͵�value
	void GetCharValue(std::istream &fin, std::string &str);
	//��ȡwhere����Ӧ��condition
	std::vector<Common::Compares> GetConditions(std::istream &fin);
	//��ȡcreate table��Ҫ��attributes
	std::vector<Common::Attribute> GetAttributes(std::istream &fin);
	//��ȡinsert��Ҫ��values����װΪ�������ַ���
	std::string GetValues(std::istream &fin);
};

