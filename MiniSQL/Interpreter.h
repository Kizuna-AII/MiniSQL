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
	API::Api *api;//interpreter要绑定的api
	//持续读取文件流，遇到';'后停止。用于遇到错误句法时抛弃整个错误语句
	void ClearCommand(std::istream &fin);
	//获取一个字符串，如果末尾有结束符';'，则将该符号放回输入流
	void GetString(std::istream &fin, std::string &str);
	//获取数据库char()类型的value
	void GetCharValue(std::istream &fin, std::string &str);
	//获取where语句对应的condition
	std::vector<Common::Compares> GetConditions(std::istream &fin);
	//获取create table需要的attributes
	std::vector<Common::Attribute> GetAttributes(std::istream &fin);
	//获取insert需要的values，封装为二进制字符串
	std::string GetValues(std::istream &fin);
};

