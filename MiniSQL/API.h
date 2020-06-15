#pragma once
#include <string>
#include <vector>
#include <fstream>
#include "Structure.h"
#include "BufferManager.h"
#include "CatalogManager.h"
#include "RecordM.h"

namespace API {
	//Exceptions
	class mexception : std::exception {
	protected:
		std::string msg;
	public:
		mexception() :msg("") {};
		mexception(const char* _msg) :msg(_msg) {};
		virtual char const* what() const { return ""; }
	};
	class not_completed_exception : mexception {
		virtual char const* what() const { return (this->msg + " | function not implemented").c_str(); }
	};
	class wrong_command_error : mexception {
		virtual char const* what() const { return "wrong command"; }
	};
	
	//
	class Api {
	private:
		Buffer::BufferManager bufferm;
		Catalog::CatalogManager catalogm;
		Record::RecordManager recordm;
		//index

	public:
		Common::Table* GetTableByName(std::string& tableName);
		//处理Create Table语句
		void CreateTable(std::string &tableName, std::vector<Common::Attribute>&attributes);
		//指定index name,所在表格on，所在属性attri，执行Create Index操作
		void CreateIndex(std::string indexName,std::string on,std::string attri);
		//处理Select语句
		void Select(std::vector<std::string>&attri, std::string from, std::vector<Common::Compares>*conditions);
		//处理Insert语句,需要解析插入值
		void Insert(std::vector<std::string>&attri, std::string into);
		//处理Delete语句
		void Delete(std::string from, std::vector<Common::Compares>*conditions);
		//处理Drop Index，传入要删除的index名和所在table，执行操作
		void DropIndex(std::string target, std::string from = "");
		//处理Drop Table，传入要删除的table名，执行操作
		void DropTable(std::string target);
	};
}