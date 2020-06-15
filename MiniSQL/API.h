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
		//����Create Table���
		void CreateTable(std::string &tableName, std::vector<Common::Attribute>&attributes);
		//ָ��index name,���ڱ��on����������attri��ִ��Create Index����
		void CreateIndex(std::string indexName,std::string on,std::string attri);
		//����Select���
		void Select(std::vector<std::string>&attri, std::string from, std::vector<Common::Compares>*conditions);
		//����Insert���,��Ҫ��������ֵ
		void Insert(std::vector<std::string>&attri, std::string into);
		//����Delete���
		void Delete(std::string from, std::vector<Common::Compares>*conditions);
		//����Drop Index������Ҫɾ����index��������table��ִ�в���
		void DropIndex(std::string target, std::string from = "");
		//����Drop Table������Ҫɾ����table����ִ�в���
		void DropTable(std::string target);
	};
}