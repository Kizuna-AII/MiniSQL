#pragma once
#include <string>
#include <vector>
#include <fstream>
#include "Structure.h"
#include "BufferManager.h"
#include "CatalogManager.h"
#include "RecordM.h"
#include "IndexManager.h"
namespace API {
	//Exceptions


	class mexception{
	protected:
		std::string msg;
	public:
		mexception() :msg("") {};
		mexception(const char* _msg) :msg(_msg) {};
		virtual std::string what() const { return "EXP"; }
	};
	//
	class not_completed_exception : public mexception {
	public:
		not_completed_exception() { this->msg =""; }
		not_completed_exception(const char* _msg) { this->msg = _msg; }
		std::string what() const { return (this->msg + " | Function Not Implemented"); }
	};
	//
	class wrong_command_error : public mexception {
	public:
		wrong_command_error(const char* _msg) { this->msg = _msg; }
		std::string what() const { return (std::string("Wrong Command: ") + this->msg); }
	};

	// Exception in Catalog Manager
	// TABLE_EXIST_ERROR : Create Table
	class table_exist_error : public mexception {
	public:
		table_exist_error(const std::string _msg) { this->msg = _msg; } 
		virtual std::string what() const { return ("Table "+this->msg+" Already Exist!"); }
	};
	// TABLE_NOTFIND_ERROR : Delete Table & Create Index & Find Table
	class table_notfind_error : public mexception {
	public:
		table_notfind_error(const std::string _msg) { this->msg = _msg; }
		virtual std::string what() const { return ("Table " + this->msg + " Don't Exist!");}
	};
	// RECORD_NOTFIND_ERROR : Insert
	class record_notfind_error : public mexception {
	public:
		record_notfind_error(const std::string _msg) { this->msg = _msg; }
		virtual std::string what() const { return ("Unique key " + this->msg + " Exist!"); }
	};
	// COLUMN_NOTFIND_ERROR : Create Index
	class column_notfind_error : public mexception {
	public:
		column_notfind_error(const std::string _msg) { this->msg = _msg; }
		virtual std::string what() const { return ("Column " + this->msg + " Don't Exist!"); }
	};
	// INDEX_EXIST_ERROR : Create Index
	class index_exist_error : public mexception {
	public:
		index_exist_error(const std::string _msg) { this->msg = _msg; }
		virtual std::string what() const { return ("Index " + this->msg + " Already Exists"); }
	};
	// INDEX_NOTFIND_ERROR : Delete Index
	class index_notfind_error : public mexception {
	public:
		index_notfind_error(const std::string _msg) { this->msg = _msg; }
		virtual std::string what() const { return ("Index " + this->msg + " Don't Exist!"); }
	};

	//
	class Api {
	private:
		Buffer::BufferManager bufferm;
		Catalog::CatalogManager catalogm; // 第一次使用前必须调用Initialization(BufferManager*)
		Record::RecordManager recordm;
		Index::IndexManager indexm;
		bool CheckUnique(Common::Table* table, Common::Tuple& tuple);

	public:
		//构造函数
		Api();
		//获取Table信息
		Common::Table* GetTableByName(std::string& tableName);
		//指定table，获取offset位置的Tuple
		Common::Tuple* GetOneTuple(Common::Table* table, int offset);
		//指定table和conditions，根据index查询结果，向offsets中填入需要查找的tuple的所在块的offset
		//例如，在[0,4096)和[4096,8192)之间均有多个目标tuple，应在vector中填入0和4096
		//如果conditions为NULL,则返回可以覆盖整个文件的offsets
		void GetOffsets(std::vector<int>&offsets,Common::Table* table, std::vector<Common::Compares>*conditions);
		//处理Create Table语句
		void CreateTable(std::string tableName, std::vector<Common::Attribute>&attributes);
		//指定index name,所在表格on，所在属性attri，执行Create Index操作
		void CreateIndex(std::string indexName,std::string on,std::string attri);
		//处理Select语句,若conditions为NULL则无条件
		void Select(std::string from, std::vector<Common::Compares>*conditions);
		//处理Insert语句,需要解析插入值
		void Insert(Common::Tuple &tuple, std::string into);
		//处理Delete语句,若conditions为NULL则无条件
		void Delete(std::string from, std::vector<Common::Compares>*conditions);
		//处理Drop Index，传入要删除的index名和所在table，执行操作
		void DropIndex(std::string target, std::string from = "");
		//处理Drop Table，传入要删除的table名，执行操作
		void DropTable(std::string target);
		//输出ScreenBuffer中的内容
		void OutPutResult(std::string tableName, std::ostream & fout);

		
	};
}