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
	class mexception : std::exception {
	protected:
		std::string msg;
	public:
		mexception() :msg("") {};
		mexception(const char* _msg) :msg(_msg) {};
		virtual char const* what() const { return ""; }
	};
	//
	class not_completed_exception : mexception {
	public:
		not_completed_exception() { this->msg =""; }
		not_completed_exception(const char* _msg) { this->msg = _msg; }
		virtual char const* what() const { return (this->msg + " | function not implemented").c_str(); }
	};
	//
	class wrong_command_error : mexception {
	public:
		wrong_command_error(const char* _msg) { this->msg = _msg; }
		virtual char const* what() const { return "wrong command"; }
	};

	// Exception in Catalog Manager
	// TABLE_EXIST_ERROR : Create Table
	class table_exist_error : mexception {
	public:
		table_exist_error(const std::string _msg) { this->msg = _msg; } 
		virtual char const* what() const { return ("Table "+this->msg+" already exist!").c_str(); }
	};
	// TABLE_NOTFIND_ERROR : Delete Table & Create Index & Find Table
	class table_notfind_error : mexception {
	public:
		table_notfind_error(const std::string _msg) { this->msg = _msg; }
		virtual char const* what() const { return ("Table " + this->msg + " don't exist!").c_str();}
	};
	// COLUMN_NOTFIND_ERROR : Create Index
	class column_notfind_error : mexception {
	public:
		column_notfind_error(const std::string _msg) { this->msg = _msg; }
		virtual char const* what() const { return ("Column " + this->msg + " don't exist!").c_str(); }
	};
	// INDEX_EXIST_ERROR : Create Index
	class index_exist_error : mexception {
	public:
		index_exist_error(const std::string _msg) { this->msg = _msg; }
		virtual char const* what() const { return ("Index " + this->msg + " already exists").c_str(); }
	};
	// INDEX_NOTFIND_ERROR : Delete Index
	class index_notfind_error : mexception {
	public:
		index_notfind_error(const std::string _msg) { this->msg = _msg; }
		virtual char const* what() const { return ("Index " + this->msg + " don't exist!").c_str(); }
	};

	//
	class Api {
	private:
		Buffer::BufferManager bufferm;
		Catalog::CatalogManager catalogm; // ��һ��ʹ��ǰ�������Initialization(BufferManager*)
		Record::RecordManager recordm;
		Index::IndexManager indexm;

	public:
		//���캯��
		Api();
		//��ȡTable��Ϣ
		Common::Table* GetTableByName(std::string& tableName);
		//ָ��table��conditions������index��ѯ�������offsets��������Ҫ���ҵ�tuple�����ڿ��offset
		//���磬��[0,4096)��[4096,8192)֮����ж��Ŀ��tuple��Ӧ��vector������0��4096
		//���conditionsΪNULL,�򷵻ؿ��Ը��������ļ���offsets
		void GetOffsets(std::vector<int>&offsets,Common::Table* table, std::vector<Common::Compares>*conditions);
		//����Create Table���
		void CreateTable(std::string tableName, std::vector<Common::Attribute>&attributes);
		//ָ��index name,���ڱ��on����������attri��ִ��Create Index����
		void CreateIndex(std::string indexName,std::string on,std::string attri);
		//����Select���,��conditionsΪNULL��������
		void Select(std::string from, std::vector<Common::Compares>*conditions);
		//����Insert���,��Ҫ��������ֵ
		void Insert(Common::Tuple &tuple, std::string into);
		//����Delete���,��conditionsΪNULL��������
		void Delete(std::string from, std::vector<Common::Compares>*conditions);
		//����Drop Index������Ҫɾ����index��������table��ִ�в���
		void DropIndex(std::string target, std::string from = "");
		//����Drop Table������Ҫɾ����table����ִ�в���
		void DropTable(std::string target);
		//���ScreenBuffer�е�����
		void OutPutResult(std::string tableName, std::ostream & fout);
	};
}