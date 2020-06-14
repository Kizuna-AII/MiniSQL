#include "CatalogManager.h"

Common::Table * Catalog::CatalogManager::GetTable(size_t handle)
{
	std::string str = BMP->GetBuffer(handle);
	return StrToTable(str);
}

void Catalog::CatalogManager::ChangeTable(size_t handle, Common::Table * table)
{
	BMP->SetSize(0, handle);
	std::string returnStr = TableToStr(table);
	BMP->Write(returnStr, handle);
	return;
}

Catalog::CatalogManager::CatalogManager()
{
	BMP = nullptr;
	tableHandle = 0;
	return;
}

Catalog::CatalogManager::~CatalogManager()
{
	return;
}

void Catalog::CatalogManager::Initialization(Buffer::BufferManager * target)
{
	this->BMP = target;
	this->tableHandle = BMP->NewPage();
	BMP->Write("0", tableHandle);
	return;
}

size_t Catalog::CatalogManager::CreateTable(Common::Table * tableName)
{
	Common::Table* temp;
	if (FindTable(tableName->name) != 0) return 0;
	else
	{
		size_t handle = BMP->NewPage();
		std::string str = TableToStr(tableName);
		BMP->Write(str, handle);
		std::string tables = BMP->GetBuffer(tableHandle);
		std::istringstream tableStreamIn(tables);
		std::ostringstream tableStreamOut("");
		int count;
		tableStreamIn >> count;
		tableStreamOut << count + 1 << " ";
		if(count !=0)
		{
			for (int i = 0; i <= count - 1; i++)
			{
				std::string tempname;
				size_t temphandle;
				tableStreamIn >> tempname >> temphandle;
				tableStreamOut << tempname << " " << temphandle << " ";
			}
		}
		tableStreamOut << tableName->name << " " << handle << " ";
		std::string returnStr = tableStreamOut.str();
		BMP->SetSize(0, tableHandle);
		BMP->Write(returnStr, tableHandle);
		return handle;
	}
}

bool Catalog::CatalogManager::DeleteTable(std::string tableName)
{
	size_t tableHandle = FindTable(tableName);
	if (FindTable(tableName) == 0) return false;
	else
	{
		std::string tables = BMP->GetBuffer(tableHandle);
		std::istringstream tableStreamIn(tables);
		std::ostringstream tableStreamOut("");
		int count;
		tableStreamIn >> count;
		tableStreamOut << count - 1 << " ";
		if (count != 1)
		{
			for (int i = 0; i <= count - 1; i++)
			{
				std::string tempname;
				size_t temphandle;
				tableStreamIn >> tempname >> temphandle;
				if (tempname != tableName)
					tableStreamOut << tempname << " " << temphandle << " ";
			}
		}
		std::string returnStr = tableStreamOut.str();
		BMP->SetSize(0, tableHandle);
		BMP->Write(returnStr, tableHandle);
		return true;
	}
}

size_t Catalog::CatalogManager::FindTable(std::string tableName)
{
	std::string tables = BMP->GetBuffer(tableHandle);
	std::istringstream tableStream(tables);
	int count;
	tableStream >> count;
	if (count == 0) return 0;
	for (int i = 0; i <= count - 1; i++)
	{
		std::string name;
		size_t handle;
		tableStream >> name >> handle;
		if (name == tableName)	return handle;
	}
	return 0;
}

Common::Table * Catalog::CatalogManager::GetTable(std::string tableName)
{
	size_t handle = FindTable(tableName);
	if (handle == 0) return nullptr;
	std::string str = BMP->GetBuffer(handle);
	return StrToTable(str);
}

int Catalog::CatalogManager::CreateIndex(std::string tableName, std::string columnName, std::string indexName)
{
	size_t handle = FindTable(tableName);
	if (handle == 0) return 0;
	int index = FindIndex(tableName, indexName);
	if (index != 0) return -2;
	Common::Table* table = GetTable(handle);
	int i;
	for (i = 0; i <= table->attributes.size() - 1; i++)
	{
		if (table->attributes[i].name == columnName)
		{
			if (table->attributes[i].indexName != noIndex) return -2;
			else
			{
				table->attributes[i].indexName = indexName;
				ChangeTable(handle, table);
				delete table;
				return 1;
			}
		}
	}
	if (i == table->attributes.size()) return -1;
	return 0;
}

bool Catalog::CatalogManager::DeleteIndex(std::string tableName, std::string indexName)
{
	int index = FindIndex(tableName, indexName);
	if (index == 0) return false;
	size_t handle = FindTable(tableName);
	Common::Table* table = GetTable(handle);
	table->attributes[index - 1].indexName = noIndex;
	ChangeTable(handle, table);
	delete table;
	return true;
}

int Catalog::CatalogManager::FindIndex(std::string tableName, std::string indexName)
{
	size_t handle = FindTable(tableName);
	if (handle == 0) return 0;
	Common::Table* table = GetTable(handle);
	for (int i = 0; i <= table->attributes.size() - 1; i++)
	{
		if (table->attributes[i].indexName == indexName) return i + 1;
	}
	delete table;
	return false;
}

std::string Catalog::TableToStr(Common::Table * table)
{
	std::ostringstream str;
	str.clear();
	str << table->name << " ";
	str << table->attributes.size() << " "; 
	for (int i = 0; i <= table->attributes.size() - 1; i++)
	{
		str << table->attributes[i].name << " ";
		str << table->attributes[i].indexName << " ";
		str << table->attributes[i].type << " ";
		str << (table->attributes[i].unique ? "1" : "0") << " ";
		str << (table->attributes[i].primary ? "1" : "0") << " ";
	}
	std::string returnStr = str.str();
	str.clear();
	return returnStr;
}

Common::Table * Catalog::StrToTable(std::string str)
{
	Common::Table* returnTable = new Common::Table();
	std::istringstream msg(str);
	msg >> returnTable->name;
	int count;
	msg >> count;
	for (int i = 0; i <= count - 1; i++)
	{
		Common::Attribute newAttribute;
		int temp;
		msg >> newAttribute.name;
		msg >> newAttribute.indexName;
		msg >> newAttribute.type;
		msg >> temp;
		newAttribute.unique = (temp == 1);
		msg >> temp;
		newAttribute.primary = (temp == 1);
		returnTable->attributes.push_back(newAttribute);
	}
	return returnTable;
}
