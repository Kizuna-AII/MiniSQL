#include "API.h"
using namespace std;

Common::Table * API::Api::GetTableByName(std::string & tableName)
{
	throw(not_completed_exception());
	return nullptr;
}

void API::Api::CreateTable(std::string tableName, std::vector<Common::Attribute>& attributes)
{
	throw(not_completed_exception());
}

void API::Api::CreateIndex(std::string indexName, std::string on, std::string attri)
{
	throw(not_completed_exception());
}

void API::Api::Select(std::string from, std::vector<Common::Compares>* conditions)
{
	throw(not_completed_exception());
}

void API::Api::Insert(Common::Tuple & tuple, std::string into)
{
	throw(not_completed_exception());
}

void API::Api::Delete(std::string from, std::vector<Common::Compares>* conditions)
{
	throw(not_completed_exception());
}

void API::Api::DropIndex(std::string target, std::string from)
{
	throw(not_completed_exception());
}

void API::Api::DropTable(std::string target)
{
	throw(not_completed_exception());
}
