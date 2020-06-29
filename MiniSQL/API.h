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
		virtual std::string what() const { return "exp"; }
	};
	//
	class not_completed_exception : public mexception {
	public:
		not_completed_exception() { this->msg =""; }
		not_completed_exception(const char* _msg) { this->msg = _msg; }
		std::string what() const { return (this->msg + " | function not implemented"); }
	};
	//
	class wrong_command_error : public mexception {
	public:
		wrong_command_error(const char* _msg) { this->msg = _msg; }
		std::string what() const { return (std::string("wrong command: ") + this->msg); }
	};

	// Exception in Catalog Manager
	// TABLE_EXIST_ERROR : Create Table
	class table_exist_error : public mexception {
	public:
		table_exist_error(const std::string _msg) { this->msg = _msg; } 
		virtual std::string what() const { return ("Table "+this->msg+" already exist!"); }
	};
	// TABLE_NOTFIND_ERROR : Delete Table & Create Index & Find Table
	class table_notfind_error : public mexception {
	public:
		table_notfind_error(const std::string _msg) { this->msg = _msg; }
		virtual std::string what() const { return ("Table " + this->msg + " don't exist!");}
	};
	// RECORD_NOTFIND_ERROR : Insert
	class record_notfind_error : public mexception {
	public:
		record_notfind_error(const std::string _msg) { this->msg = _msg; }
		virtual std::string what() const { return ("Unique key " + this->msg + " exist!"); }
	};
	// COLUMN_NOTFIND_ERROR : Create Index
	class column_notfind_error : public mexception {
	public:
		column_notfind_error(const std::string _msg) { this->msg = _msg; }
		virtual std::string what() const { return ("Column " + this->msg + " don't exist!"); }
	};
	// INDEX_EXIST_ERROR : Create Index
	class index_exist_error : mexception {
	public:
		index_exist_error(const std::string _msg) { this->msg = _msg; }
		virtual std::string what() const { return ("Index " + this->msg + " already exists"); }
	};
	// INDEX_NOTFIND_ERROR : Delete Index
	class index_notfind_error : mexception {
	public:
		index_notfind_error(const std::string _msg) { this->msg = _msg; }
		virtual std::string what() const { return ("Index " + this->msg + " don't exist!"); }
	};

	//
	class Api {
	private:
		Buffer::BufferManager bufferm;
		Catalog::CatalogManager catalogm; // ��һ��ʹ��ǰ�������Initialization(BufferManager*)
		Record::RecordManager recordm;
		Index::IndexManager indexm;
		bool CheckUnique(Common::Table* table, Common::Tuple& tuple);

	public:
		//���캯��
		Api();
		//��ȡTable��Ϣ
		Common::Table* GetTableByName(std::string& tableName);
		//ָ��table����ȡoffsetλ�õ�Tuple
		Common::Tuple* GetOneTuple(Common::Table* table, int offset);
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