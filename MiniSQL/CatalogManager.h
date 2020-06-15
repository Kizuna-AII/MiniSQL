#pragma once
#include "Structure.h"
#include "BufferManager.h"
#include <memory>
#include <string>
#include <sstream>

namespace Catalog
{
	// Tableת����string�ĺ���
	std::string TableToStr(Common::Table* table);
	// stringת����Table�ĺ���
	Common::Table* StrToTable(std::string str);
	const std::string noIndex = "#NULL#";

	class CatalogManager {
	private:
		Buffer::BufferManager* BMP;
		size_t tableHandle;
		// ���handle��Ӧhandle��������findtable
		Common::Table* GetTable(size_t handle);
		// �޸�table��Ϣ�������޸�����
		void ChangeTable(size_t handle, Common::Table* table);
	public:
		CatalogManager();
		~CatalogManager();
		// ����BMP�����ڴ洢������Ϣ��handle�Խ��г�ʼ��
		void Initialization(Buffer::BufferManager* target);

		// ����table����Ϣ�Լ��洢��handle 0-���Ѿ����� ����ָ�򴴽��ɹ���handle
		size_t CreateTable(Common::Table* tableName);
		// ����table�����֣�ɾ��ָ����table, true��ɾ���ɹ���false���Ҳ���table
		bool DeleteTable(std::string tableName);
		// ����table�����֣�����ֵΪhandle���Ҳ����򷵻�0
		size_t FindTable(std::string tableName);
		// ����table�����֣�����ֵΪTable*����������ڷ���nullptr
		Common::Table* GetTable(std::string tableName);
		// ��������,�������� 1:success 0:�Ҳ�����Ӧ�ı� -1:�Ҳ�����Ӧ���� -2:�Ѿ��������� 
		int CreateIndex(std::string tableName, std::string columnName, std::string indexName);
		// ɾ������,�������� True:success False:�Ҳ�����Ӧ������
		bool DeleteIndex(std::string tableName, std::string indexName);
		// ���������������������ͱ��������������Ƿ���ڣ�0-�����ڣ�1-attributes.size()-��Ӧ����
		int FindIndex(std::string tableName, std::string indexName);

	};
}