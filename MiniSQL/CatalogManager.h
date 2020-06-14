#pragma once
#include "Structure.h"
#include "BufferManager.h"
#include <memory>
#include <string>
#include <sstream>

namespace Catalog
{
	// Table转换至string的函数
	std::string TableToStr(Common::Table* table);
	// string转换至Table的函数
	Common::Table* StrToTable(std::string str);
	const std::string noIndex = "#NULL#";

	class CatalogManager {
	private:
		Buffer::BufferManager* BMP;
		size_t tableHandle;
		// 获得handle对应handle，必须先findtable
		Common::Table* GetTable(size_t handle);
		// 修改table信息，不能修改名字
		void ChangeTable(size_t handle, Common::Table* table);
	public:
		CatalogManager();
		~CatalogManager();
		// 传入BMP和用于存储基本信息的handle以进行初始化
		void Initialization(Buffer::BufferManager* target);

		// 传入table的信息以及存储的handle 0-表已经存在 否则指向创建成功的handle
		size_t CreateTable(Common::Table* tableName);
		// 传入table的名字，删除指定的table, true则删除成功，false则找不到table
		bool DeleteTable(std::string tableName);
		// 传入table的名字，返回值为handle若找不到则返回0
		size_t FindTable(std::string tableName);
		// 传入table的名字，返回值为Table*，如果不存在返回nullptr
		Common::Table* GetTable(std::string tableName);
		// 创建索引,返回类型 1:success 0:找不到对应的表 -1:找不到对应的列 -2:已经存在索引 
		int CreateIndex(std::string tableName, std::string columnName, std::string indexName);
		// 删除索引,返回类型 True:success False:找不到对应的索引
		bool DeleteIndex(std::string tableName, std::string indexName);
		// 查找索引，传入索引名和表名，返回索引是否存在，0-不存在，1-attributes.size()-对应列数
		int FindIndex(std::string tableName, std::string indexName);

	};
}