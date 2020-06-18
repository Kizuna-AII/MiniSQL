#include "API.h"
using namespace std;
namespace API {
	std::vector<std::string> screenBuffer; //׼���������Ļ��Buffer�����磬��RecordManager��ɲ�ѯ����ʱ����Ҫ��������ݶ��������
	std::vector<std::string> inputBuffer; //׼���Ӷ�������ģ���Buffer������Insertʱ����Ҫinsert��tuple������ȥ��
}

Common::Table * API::Api::GetTableByName(std::string & tableName)
{
	Common::Table* newTable = catalogm.GetTable(tableName);
	if (newTable == nullptr)
		throw(table_notfind_error(tableName));
	return newTable;
}

void API::Api::CreateTable(std::string tableName, std::vector<Common::Attribute>& attributes)
{
	//Debug
	cout << "DEBUG info:" << endl;
	cout << "table:"<<tableName << endl;
	cout << "attri:" << endl;
	for (int i = 0; i < attributes.size(); i++) {
		cout << attributes[i].name << " type: " << attributes[i].type <<"  primary:"<<attributes[i].primary<<"  uni:"<<attributes[i].unique;
		cout << endl;
	}

	Common::Table* newTable = new Common::Table();
	newTable->name = tableName;
	newTable->attributes = attributes;
	size_t newhandle = catalogm.CreateTable(newTable);
	delete newTable;
	if (newhandle == 0)
		throw(table_exist_error(tableName));
	return;
}

void API::Api::CreateIndex(std::string indexName, std::string on, std::string attri)
{
	//DEBUG
	cout << "DEBUG info:" << endl;
	cout << "index name:" << indexName << endl;
	cout << "table:" << on << endl;
	cout << "attri:" << attri <<endl;
	//DEBUG
	int result = catalogm.CreateIndex(on, attri, indexName);
	if (result == 0)
		throw(table_notfind_error(on));
	if (result == -1)
		throw(column_notfind_error(attri));
	if (result == -2)
		throw(index_exist_error(indexName));
	// catalog manager create index success
	indexm.setWorkspace(on, attri);
	indexm.createIndex();
	// throw(not_completed_exception());
}

void API::Api::Select(std::string from, std::vector<Common::Compares>* conditions)
{
	//DEBUG
	cout << "DEBUG info Select:" << endl;
	cout << "from :" << from << endl;
	cout << "conditions:" <<endl;
	for (int i = 0; i < conditions->size(); i++) {
		cout << (*conditions)[i].attri << "	" << (int)(*conditions)[i].ctype << "	" << (*conditions)[i].value << endl;
	}
	//
	//�ҵ���Ӧ�ı�
	Common::Table* table = GetTableByName(from);
	//��index�ҳ�����Ҫ��ѯ�Ŀ�
	//����ÿ���飬ʹ��recordm->Select(���ָ�룬����vector��ָ�룬buffer handle);
	//recordm�������API::screenBuffer�У��Զ�����������ʽ�洢
	//֮��������������OutPutResult()��ʾ���
	throw(not_completed_exception());
}

void API::Api::Insert(Common::Tuple & tuple, std::string into)
{
	//DEBUG
	cout << "DEBUG info Insert:" << endl;
	cout << "into:" << into <<endl;
	cout << "values:" << endl;
	cout << tuple.GetString() << endl;
	//
	throw(not_completed_exception());
}

void API::Api::Delete(std::string from, std::vector<Common::Compares>* conditions)
{
	//DEBUG
	cout << "DEBUG info Detete:" << endl;
	cout << "from :" << from << endl;
	cout << "conditions:" << endl;
	for (int i = 0; i < conditions->size(); i++) {
		cout << (*conditions)[i].attri << "	" << (int)(*conditions)[i].ctype << "	" << (*conditions)[i].value << endl;
	}
	//
	throw(not_completed_exception());
}

void API::Api::DropIndex(std::string target, std::string from)
{
	//DEBUG
	cout << "DEBUG info dropindex:" << endl;
	cout << "index:" << target << "	" << "from:" << from << endl;
	//
	std::string indexName = catalogm.DeleteIndex(from, target);
	if (indexName == Catalog::noIndex)
		throw(index_notfind_error(target));
	// catalog manager delete index success
	indexm.setWorkspace(indexName);
	indexm.dropIndex();
	// throw(not_completed_exception());
}

void API::Api::DropTable(std::string target)
{
	//DEBUG
	cout << "DEBUG info DropTable:" << endl;
	cout << "target:"<<target << endl;
	//

	bool deleteTable = catalogm.DeleteTable(target);
	if (deleteTable == false)
		throw(table_notfind_error(target));

	// index manager delete index

	throw(not_completed_exception());
}

void API::Api::OutPutResult(std::string tableName,std::ostream & fout)
{
	Common::Table* table = GetTableByName(tableName);
	for (int j = 0; j < table->attributes.size(); j++) {//�����ͷ
		fout << table->attributes[j].name << "	";
	}
	fout << endl;
	for (int i = 0,offset; i < screenBuffer.size(); i++) {
		offset = 0;
		for (int j = 0; j < table->attributes.size(); j++) {//����һ�У������������ֵ
			if (table->attributes[j].type == -1) {
				//int
				fout << *(float*)(screenBuffer[i].c_str() + offset) << "	";
				offset += sizeof(float);
			}
			else if (table->attributes[j].type == 0) {
				fout << *(int*)(screenBuffer[i].c_str() + offset) << "	";
				offset += sizeof(int);
				//float
			}
			else {
				//char
				fout << *(char*)(screenBuffer[i].c_str() + offset) << "	";
				offset += table->attributes[j].type;
			}
		}
		fout << endl;
	}
	throw(not_completed_exception());
}
