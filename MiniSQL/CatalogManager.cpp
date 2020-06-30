#include "CatalogManager.h"

Common::Table * Catalog::CatalogManager::GetTable(size_t handle)
{
	std::string str = BMP->Read(handle);
	return StrToTable(str);
}

void Catalog::CatalogManager::ChangeTable(size_t handle, Common::Table * table)
{
	BMP->SetSize(0, handle);
	std::string returnStr = TableToStr(table);
	BMP->Write(returnStr, handle);
	BMP->Delete(handle);
	BMP->SetFileOffset(0, handle);
	BMP->Save(handle);
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
	if (this->tableHandle == 0)
		this->tableHandle = BMP->NewPage();
	BMP->SetFilename(CATALOGWORKPATH + "#base#.txt", this->tableHandle);
	BMP->SetPin(tableHandle);
	if (!BMP->IsExist(this->tableHandle))
	{
		BMP->Write("0", this->tableHandle);
		BMP->Save(this->tableHandle);
		return;
	}
	BMP->Load(this->tableHandle);
	std::string tables = BMP->Read(tableHandle);
	std::istringstream tableStreamIn(tables);
	std::ostringstream tableStreamOut("");
	int count;
	tableStreamIn >> count;
	tableStreamOut << count << " ";
	if (count != 0)
	{
		for (int i = 0; i <= count - 1; i++)
		{
			std::string tempname;
			size_t temphandle;
			tableStreamIn >> tempname >> temphandle;
			temphandle = BMP->NewPage();
			if (temphandle == 0)
				temphandle = BMP->NewPage();
			BMP->SetPin(temphandle);
			BMP->SetFilename(CATALOGWORKPATH + tempname + ".txt", temphandle);
			BMP->Load(temphandle);
			tableStreamOut << tempname << " " << temphandle << " ";
		}
		BMP->SetSize(0, this->tableHandle);
		BMP->Write(tableStreamOut.str(), this->tableHandle);
		BMP->Delete(this->tableHandle);
		BMP->SetFileOffset(0, this->tableHandle);
		BMP->Save(this->tableHandle);
	}
	return;
}

size_t Catalog::CatalogManager::CreateTable(Common::Table * tableName)
{
	if (FindTable(tableName->name) != 0) return 0;
	else
	{
		size_t handle = BMP->NewPage();
		if (handle == 0)
			handle = BMP->NewPage();
		BMP->SetPin(handle);
		std::string str = TableToStr(tableName);
		BMP->Write(str, handle);
		BMP->SetFilename(CATALOGWORKPATH + tableName->name + ".txt", handle);
		BMP->Save(handle);
		//std::cout << str << std::endl;
		std::string tables = BMP->Read(this->tableHandle);
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
		BMP->Delete(tableHandle);
		BMP->SetFileOffset(0, tableHandle);
		BMP->Save(tableHandle);
		return handle;
	}
}

bool Catalog::CatalogManager::DeleteTable(std::string tableName)
{
	size_t handle = FindTable(tableName);
	if (handle == 0) return false;
	else
	{
		BMP->SetSize(0, handle);
		BMP->Delete(handle);
		BMP->ResetPin(handle);
		std::string tables = BMP->Read(tableHandle);
		std::istringstream tableStreamIn(tables);
		std::ostringstream tableStreamOut("");
		int count;
		tableStreamIn >> count;
		tableStreamOut << count - 1;
		if (count != 1)
		{
			for (int i = 0; i <= count - 1; i++)
			{
				std::string tempname;
				size_t temphandle;
				tableStreamIn >> tempname >> temphandle;
				if (tempname != tableName)
					tableStreamOut << " " << tempname << " " << temphandle;
			}
		}
		std::string returnStr = tableStreamOut.str();
		BMP->SetSize(0, tableHandle);
		BMP->Write(returnStr, tableHandle);
		BMP->Delete(tableHandle);
		BMP->SetFileOffset(0, tableHandle);
		BMP->Save(tableHandle);
		return true;
	}
}

size_t Catalog::CatalogManager::FindTable(std::string tableName)
{
	std::string tables = BMP->Read(tableHandle);
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
	std::string str = BMP->Read(handle);
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
	if (indexName == noIndex) indexName = tableName + "#" + columnName;
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

std::string Catalog::CatalogManager::DeleteIndex(std::string tableName, std::string indexName)
{
	int index = FindIndex(tableName, indexName);
	if (index == 0) return noIndex;
	size_t handle = FindTable(tableName);
	Common::Table* table = GetTable(handle);
	table->attributes[index - 1].indexName = noIndex;
	ChangeTable(handle, table);
	std::string returnStr = tableName + "#" + table->attributes[index - 1].name;
	delete table;
	return returnStr;
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

void Catalog::CatalogManager::ShowTables()
{
	std::cout << BMP->Read(tableHandle) << std::endl;
	return;
}

std::vector<std::string> Catalog::CatalogManager::ShowIndex()
{
	std::vector<std::string> result;
	std::string tables = BMP->Read(tableHandle);
	std::istringstream tableStreamIn(tables);
	int count;
	tableStreamIn >> count;
	if (count == 0) return result;
	else
	{
		for (int i = 0; i <= count - 1; i++)
		{
			size_t temphandle;
			std::string tempname;
			tableStreamIn >> tempname >> temphandle;
			std::string str = BMP->Read(temphandle);
			Common::Table* temptable = StrToTable(str);
			for (int j = 0; j <= temptable->attributes.size() - 1; j++)
			{
				if (temptable->attributes[j].indexName != noIndex)
					result.push_back(tempname + "#" + temptable->attributes[j].name);
			}
			delete temptable;
		}
	}
	return result;
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
