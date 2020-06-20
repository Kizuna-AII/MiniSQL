#include "API.h"
using namespace std;
namespace API {
	std::vector<std::string> screenBuffer; //׼���������Ļ��Buffer�����磬��RecordManager��ɲ�ѯ����ʱ����Ҫ��������ݶ��������
	std::vector<std::string> inputBuffer; //׼���Ӷ�������ģ���Buffer������Insertʱ����Ҫinsert��tuple������ȥ��
}

API::Api::Api()
{
	bufferm = Buffer::BufferManager();
	catalogm = Catalog::CatalogManager();
	catalogm.Initialization(&bufferm);
	indexm = Index::IndexManager();
	indexm.linkBufferManager(&bufferm);
	recordm = Record::RecordManager();
	recordm.LinkBufferManager(&bufferm);
	recordm.LinkIndex(&indexm);
	return;
}

Common::Table * API::Api::GetTableByName(std::string & tableName)
{
	Common::Table* newTable = catalogm.GetTable(tableName);
	if (newTable == nullptr)
		throw(table_notfind_error(tableName));
	return newTable;
}

//void API::Api::AddIndex(Common::Table * table, int order, std::string key, int value)
//{
//	throw(not_completed_exception());
//}
//
//void API::Api::DelIndex(Common::Table * table, int order, std::string key)
//{
//	throw(not_completed_exception());
//}

void API::Api::GetOffsets(std::vector<int>&offsets,Common::Table * table, std::vector<Common::Compares>* conditions)
{
	//��鿴Select�Ĵ�����Ϊ
	throw(not_completed_exception());
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
	if (conditions != NULL) {
		for (int i = 0; i < conditions->size(); i++) {
			cout << (*conditions)[i].attri << "	" << (int)(*conditions)[i].ctype << "	" << (*conditions)[i].value << endl;
		}
	}
	//
	//�ҵ���Ӧ�ı�
	Common::Table* table = GetTableByName(from);
	std::vector<int>offsets;
	offsets.clear();
	GetOffsets(offsets, table, conditions);//��ȡ��Ҫ��ȡ�Ŀ�λ��
	string fileName = from + "_rec"; //�ļ�����Ϊ table_rec
	int handle = bufferm.NewPage();
	bufferm.SetFilename(fileName, handle);
	bufferm.SetPin(handle);
	for (int i = 0; i < offsets.size(); i++) {//���δ���ÿ��offset
		long long tmp = offsets[i];
		bufferm.SetFileOffset(tmp, handle);
		bufferm.Load(handle);//��ȡָ���Ŀ�
		recordm.Select(table, conditions, handle);
		//��ÿ����ʹ��recordm->Select()����tuple;
	}
	bufferm.ResetPin(handle);
	//recordm�������API::screenBuffer�У��Զ�����������ʽ�洢
	//֮��������������OutPutResult()��ʾ���
	return;
}

void API::Api::Insert(Common::Tuple & tuple, std::string into)
{
	//DEBUG
	cout << "DEBUG info Insert:" << endl;
	cout << "into:" << into <<endl;
	cout << "values:" << endl;
	cout << tuple.GetString() << endl;
	//
	Common::Table* table = GetTableByName(into);//��ȡtable
	API::inputBuffer.push_back(tuple.GetString());
	string fileName = into + "_rec"; //�ļ�����Ϊ table_rec
	int handle = bufferm.NewPage();
	bufferm.SetFilename(fileName, handle);
	
	long long offset = 0;
	bufferm.SetPin(handle);
	do {
		bufferm.SetFileOffset(offset, handle);
		bufferm.Load(handle);//��ȡ�ļ�
		//
		if (bufferm.GetSize(handle) < Buffer::BLOCKCAPACITY) {//����ÿ��п���
			recordm.Insert(table, handle);//����recormִ�в��������ͬʱ����index
			bufferm.Save(handle);//����

			// index manager ��������index���ж�Ӧ��B+������ <key, offset> ��
			int tupleOffset = 0;
			for(auto col: table->attributes){
				if(col.indexName!="#NULL#"){
					indexm.setWorkspace(into, col.name);	
					indexm.insert(tuple.Get(col, tupleOffset), offset);
				}
			}

		}
		//
		offset += Buffer::BLOCKCAPACITY;
	} while (!inputBuffer.empty());
	bufferm.ResetPin(handle);
	//throw(not_completed_exception());
	return;
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
	Common::Table* table = GetTableByName(from);//��ȡ��ͷ
	std::vector<int>offsets;
	offsets.clear();
	GetOffsets(offsets, table, conditions);//��ȡ��Ҫ��ȡ�Ŀ�λ��
	string fileName = from + "_rec"; //�ļ�����Ϊ table_rec
	int handle = bufferm.NewPage();
	bufferm.SetFilename(fileName, handle);
	bufferm.SetPin(handle);
	for (int i = 0; i < offsets.size(); i++) {//���δ���ÿ��offset
		long long tmp = offsets[i];
		bufferm.SetFileOffset(tmp, handle);
		bufferm.Load(handle);//��ȡָ���Ŀ�
		recordm.Delete(table, conditions, handle);
		//��ÿ����ʹ��recordm->Delete()����tuple;
		bufferm.Save(handle);//ɾ���󱣴�
	}
	bufferm.ResetPin(handle);
	//
	//throw(not_completed_exception());
	return;
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
}

void API::Api::DropTable(std::string target)
{
	//DEBUG
	cout << "DEBUG info DropTable:" << endl;
	cout << "target:"<<target << endl;

	// index manager ��������index����ɾ����Ӧ��B+��
	Common::Table* table = GetTableByName(target);//��ȡtable
	for(auto col: table->attributes){
		if(col.indexName!="#NULL#"){
			indexm.setWorkspace(target, col.name);	
			indexm.dropIndex();
		}
	}
	// catalog manager delete index
	bool deleteTable = catalogm.DeleteTable(target);
	if (deleteTable == false)
		throw(table_notfind_error(target));
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
	//throw(not_completed_exception());
	return;
}
