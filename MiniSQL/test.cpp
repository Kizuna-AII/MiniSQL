#include <iostream>
#include "Structure.h"
#include "BufferManager.h"
#include "CatalogManager.h"

int main()
{
	Buffer::BufferManager* BMP = new Buffer::BufferManager();
	BMP->Initialize();
	Catalog::CatalogManager* CMP = new Catalog::CatalogManager();
	CMP->Initialization(BMP);
	Common::Table* table = new Common::Table();
	table->name = "first_table";
	Common::Attribute attr;
	attr.indexName = "yes";
	attr.name = "first_column";
	attr.primary = true;
	attr.type = 0;
	attr.unique = true;
	table->attributes.push_back(attr);
	attr.name = "second_column";
	attr.indexName = Catalog::noIndex;
	attr.type = -1;
	table->attributes.push_back(attr);
	std::cout << CMP->CreateTable(table) << std::endl;
	std::cout << CMP->FindTable("first") << std::endl;
	std::cout << CMP->FindTable("first_table") << std::endl;
	std::cout << CMP->CreateIndex("first_table", "first_column", "yes") << std::endl;
	std::cout << CMP->CreateIndex("first_table", "second_column", "newindex") << std::endl;
	std::cout << CMP->DeleteIndex("first_table", "newindex") << std::endl;
	std::cout << CMP->FindIndex("first_table", "yes") << std::endl;
	std::cout << CMP->DeleteTable("second_table") << std::endl;
	std::cout << CMP->DeleteTable("first_table") << std::endl;
	std::cout << CMP->FindTable("first_table") << std::endl;

	std::cout << 111 << std::endl;

	return 0;
}