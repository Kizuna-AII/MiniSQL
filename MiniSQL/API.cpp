#include "API.h"
#include<algorithm>
using namespace std;
#define DEBUGINFO false

bool API::Api::CheckUnique(Common::Table* table, Common::Tuple & tuple){
	int offset = 0;
	for (int i = 0; i < table->attributes.size(); i++) {
		if (table->attributes[i].unique || table->attributes[i].primary) {
			Common::Compares tmp;
			tmp.attri = table->attributes[i].name;
			tmp.ctype = Common::CompareType::je;
			string key="";
			if (table->attributes[i].type == -1) {//float
				key = to_string(tuple.Get<float>(offset));
				offset += sizeof(float);
			}
			else if (table->attributes[i].type == 0) {//int
				key = to_string(tuple.Get<int>(offset));
				offset += sizeof(int);
			}
			else {//char
				key = tuple.Get(table->attributes[i], offset);
				offset += table->attributes[i].type;
			}
			tmp.value = key;
			vector<Common::Compares>tmpCmp;
			tmpCmp.clear();
			tmpCmp.push_back(tmp);
			screenBuffer.clear();
			Select(table->name, &tmpCmp);//Select��Ӧ��key
			if (screenBuffer.size() > 0) {
				//��key�Ѵ���
				return 0;
			}
		}
	}
	return 1;//�����ڳ�ͻ������1
}


API::Api::Api()
{
	bufferm = Buffer::BufferManager();
	catalogm = Catalog::CatalogManager();
	catalogm.Initialization(&bufferm);
	recordm = Record::RecordManager();
	recordm.LinkBufferManager(&bufferm);
	recordm.LinkIndex(&indexm);
	indexm = Index::IndexManager();
	indexm.linkBufferManager(&bufferm);
	vector<std::string> indices = catalogm.ShowIndex();
	for(auto i: indices){
		indexm.setWorkspace(i);
		indexm.createIndex();
	}
	return;
}

Common::Table * API::Api::GetTableByName(std::string & tableName)
{
	Common::Table* newTable = catalogm.GetTable(tableName);
	if (newTable == nullptr)
		throw(table_notfind_error(tableName));
	return newTable;
}

Common::Tuple * API::Api::GetOneTuple(Common::Table * table, int offset)
{
	int tupleLen = table->GetDataSize();
	
	string fileName = Record::RecordManager::GetRecordFileName(table->name); //�ļ�����Ϊ table_rec
	int handle = bufferm.NewPage();
	bufferm.SetFilename(fileName, handle);
	bufferm.SetPin(handle);
	bufferm.SetFileOffset(offset, handle);
	bufferm.Load(handle);//��ȡָ���Ŀ�
	string &tmp = bufferm.GetBuffer(handle);
	Common::Tuple* tuple = new Common::Tuple(tupleLen, tmp.c_str());//��ȡ��ĵ�һ��tuple
	bufferm.ResetPin(handle);
	return tuple;
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
	// ��ʼ��result�������ļ����еĿ�
	string fileName = Record::RecordManager::GetRecordFileName(table->name);
	int handle = bufferm.NewPage();
	bufferm.SetFilename(fileName, handle);
	int len = bufferm.GetFileSize(handle);
	std::set<int> result;
	for(int i = 0; i < len; i += Buffer::BLOCKCAPACITY)
		result.insert(i);
	if (conditions != NULL) {
		// ѡ����������index�����ϵ�����
		std::vector<Common::Compares> conditionsOnIndex;
		for (auto con : *conditions) {
			indexm.setWorkspace(table->name, con.attri);
			if (indexm.existIndex())
				conditionsOnIndex.push_back(con);
		}
		// ��indexm������Щ��index���ϵ�����select
		// ��������ÿ�������Ľ��ȡ����
		for (auto con : conditionsOnIndex) {
			indexm.setWorkspace(table->name, con.attri);
			std::set<int> tmp = indexm.select(con);
			std::vector<int> newResult;
			std::set_intersection(result.begin(), result.end(), tmp.begin(), tmp.end(), std::back_inserter(newResult));
			result.clear();
			for (auto i : newResult)
				result.insert(i);
		}
	}
	for(auto i: result)
		offsets.push_back(i);
}

void API::Api::CreateTable(std::string tableName, std::vector<Common::Attribute>& attributes)
{
	//Debug
	if(DEBUGINFO){
		cout << "DEBUG info:" << endl;
		cout << "table:"<<tableName << endl;
		cout << "attri:" << endl;
		for (int i = 0; i < attributes.size(); i++) {
			cout << attributes[i].name << " type: " << attributes[i].type <<"  primary:"<<attributes[i].primary<<"  uni:"<<attributes[i].unique;
			cout << endl;
		}
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
	if(DEBUGINFO){
		//DEBUG
		cout << "DEBUG info:" << endl;
		cout << "index name:" << indexName << endl;
		cout << "table:" << on << endl;
		cout << "attri:" << attri <<endl;
	}
	//DEBUG
	int result = catalogm.CreateIndex(on, attri, indexName);
	if (result == 0)
		throw(table_notfind_error(on));
	if (result == -1)
		throw(column_notfind_error(attri));
	if (result == -2)
		throw(index_exist_error(indexName));
	// catalog manager create index success
	
	Common::Table* table = GetTableByName(on);
	int datawidth = 10;
	for(auto i: table->attributes){
		if(i.name == attri){
			if(i.type <=0 ) datawidth = 10;
			else datawidth = i.type + 1;
		}
	}
	indexm.setWorkspace(on, attri);
	indexm.createIndex(datawidth);
}

void API::Api::Select(std::string from, std::vector<Common::Compares>* conditions)
{
	if(DEBUGINFO){
		//DEBUG
		cout << "DEBUG info Select:" << endl;
		cout << "from :" << from << endl;
		cout << "conditions:" <<endl;
		if (conditions != NULL) {
			for (int i = 0; i < conditions->size(); i++) {
				cout << (*conditions)[i].attri << "	" << (int)(*conditions)[i].ctype << "	" << (*conditions)[i].value << endl;
			}
		}
	}
	//
	//�ҵ���Ӧ�ı�
	Common::Table* table = GetTableByName(from);
	string fileName = Record::RecordManager::GetRecordFileName(from);; //�ļ�����Ϊ table_rec
	int handle = bufferm.NewPage();
	bufferm.SetFilename(fileName, handle);
	if (!bufferm.IsExist(handle))return;//�ļ������ڣ�ֱ�ӷ���
	bufferm.SetPin(handle);
	std::vector<int>offsets;
	offsets.clear();
	GetOffsets(offsets, table, conditions);//��ȡ��Ҫ��ȡ�Ŀ�λ��	
	API::screenBuffer.clear();
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
	if(DEBUGINFO){
	//DEBUG
		cout << "DEBUG info Insert:" << endl;
		cout << "into:" << into <<endl;
		cout << "values:" << endl;
		cout << tuple.GetString() << endl;
	}
	//
	Common::Table* table = GetTableByName(into);//��ȡtable
	if (CheckUnique(table, tuple) == 0) {
		throw(record_notfind_error(""));
		return;
	}
	API::inputBuffer.push_back(tuple.GetString());
	string fileName = Record::RecordManager::GetRecordFileName(into);; //�ļ�����Ϊ table_rec
	int handle = bufferm.NewPage();
	bufferm.SetFilename(fileName, handle);
	if (!bufferm.IsExist(handle)) {
		bufferm.SetSize(0,handle);
		bufferm.Save(handle);
	}
	long long offset = 0;
	bufferm.SetPin(handle);
	do {
		bufferm.SetFileOffset(bufferm.GetFileSize(handle), handle);
		bufferm.Load(handle);//��ȡ�ļ�
		//
		if (bufferm.GetSize(handle) < Buffer::BLOCKCAPACITY) {//����ÿ��п���
			recordm.Insert(table, handle);//����recormִ�в��������ͬʱ����index
			bufferm.Save(handle);//����
		}
		//
		offset += Buffer::BLOCKCAPACITY;
	} while (!inputBuffer.empty());
	bufferm.ResetPin(handle);
	return;
}

void API::Api::Delete(std::string from, std::vector<Common::Compares>* conditions)
{
	if(DEBUGINFO){
		//DEBUG
		cout << "DEBUG info Detete:" << endl;
		cout << "from :" << from << endl;
		cout << "conditions:" << endl;
		for (int i = 0; i < conditions->size(); i++) {
			cout << (*conditions)[i].attri << "	" << (int)(*conditions)[i].ctype << "	" << (*conditions)[i].value << endl;
		}
	}
	//
	Common::Table* table = GetTableByName(from);//��ȡ��ͷ
	std::vector<int>offsets;
	offsets.clear();
	GetOffsets(offsets, table, conditions);//��ȡ��Ҫ��ȡ�Ŀ�λ��
	string fileName = Record::RecordManager::GetRecordFileName(from);; //�ļ�����Ϊ table_rec
	int handle = bufferm.NewPage();
	bufferm.SetFilename(fileName, handle);
	bufferm.SetPin(handle);
	recordm.ClearDelRec();
	for (int i = 0; i < offsets.size(); i++) {//���δ���ÿ��offset
		long long tmp = offsets[i];
		bufferm.SetFileOffset(tmp, handle);
		bufferm.Load(handle);//��ȡָ���Ŀ�
		recordm.Delete(table, conditions, handle);
		//��ÿ����ʹ��recordm->Delete()����tuple;
		bufferm.Save(handle);//ɾ���󱣴�
	}
	bufferm.ResetPin(handle);
	recordm.FillBlanks(table);//���ļ�ĩβ��tuple��ɾ���������µĿ�λ
	return;
}

void API::Api::DropIndex(std::string target, std::string from)
{
	if(DEBUGINFO){
		//DEBUG
		cout << "DEBUG info dropindex:" << endl;
		cout << "index:" << target << "	" << "from:" << from << endl;
	}
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
	if(DEBUGINFO){
		//DEBUG
		cout << "DEBUG info DropTable:" << endl;
		cout << "target:"<<target << endl;
	}

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
	// record manager delete table
	bufferm.NewPage();
	bufferm.SetFilename(Record::RecordManager::GetRecordFileName(target));
	bufferm.Delete();
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
				//float
				fout << *(float*)(screenBuffer[i].c_str() + offset) << "	";
				offset += sizeof(float);
			}
			else if (table->attributes[j].type == 0) {
				fout << *(int*)(screenBuffer[i].c_str() + offset) << "	";
				offset += sizeof(int);
				//int
			}
			else {
				//char
				//fout << (char*)(screenBuffer[i].c_str() + offset) << "	";
				fout << string((char*)(screenBuffer[i].c_str() + offset),table->attributes[j].type) << "	";
				offset += table->attributes[j].type;
			}
		}
		fout << endl;
	}
	//throw(not_completed_exception());
	return;
}
