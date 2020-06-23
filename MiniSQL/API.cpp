#include "API.h"
#include<algorithm>
using namespace std;


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
			Select(table->name, &tmpCmp);//Select对应的key
			if (screenBuffer.size() > 0) {
				//该key已存在
				return 0;
			}
		}
	}
	return 1;//不存在冲突，返回1
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

Common::Tuple * API::Api::GetOneTuple(Common::Table * table, int offset)
{
	int tupleLen = table->GetDataSize();
	
	string fileName = table->name + "_rec"; //文件命名为 table_rec
	int handle = bufferm.NewPage();
	bufferm.SetFilename(fileName, handle);
	bufferm.SetPin(handle);
	bufferm.SetFileOffset(offset, handle);
	bufferm.Load(handle);//读取指定的块
	string &tmp = bufferm.GetBuffer(handle);
	Common::Tuple* tuple = new Common::Tuple(tupleLen, tmp.c_str());//获取块的第一个tuple
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
	// 初始化result，包括文件所有的块
	string fileName = table->name + "_rec";
	int handle = bufferm.NewPage();
	bufferm.SetFilename(fileName, handle);
	int len = bufferm.GetFileSize(handle);
	std::set<int> result;
	for(int i = 0; i < len; i += Buffer::BLOCKCAPACITY)
		result.insert(i);
	if (conditions != NULL) {


		// 选出所有在有index的列上的条件
		std::vector<Common::Compares> conditionsOnIndex;
		for (auto con : *conditions) {
			indexm.setWorkspace(table->name, con.attri);
			if (indexm.existIndex())
				conditionsOnIndex.push_back(con);
		}
		// 用indexm按照那些在index列上的条件select
		// 方法：对每个条件的结果取交集
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
	//找到对应的表
	Common::Table* table = GetTableByName(from);
	string fileName = from + "_rec"; //文件命名为 table_rec
	int handle = bufferm.NewPage();
	bufferm.SetFilename(fileName, handle);
	if (!bufferm.IsExist(handle))return;//文件不存在，直接返回
	bufferm.SetPin(handle);
	std::vector<int>offsets;
	offsets.clear();
	GetOffsets(offsets, table, conditions);//获取需要读取的块位置	
	API::screenBuffer.clear();
	for (int i = 0; i < offsets.size(); i++) {//依次处理每个offset
		long long tmp = offsets[i];
		bufferm.SetFileOffset(tmp, handle);
		bufferm.Load(handle);//读取指定的块
		recordm.Select(table, conditions, handle);
		//对每个块使用recordm->Select()查找tuple;
	}
	bufferm.ResetPin(handle);
	//recordm的输出在API::screenBuffer中，以二进制数据形式存储
	//之后会在主程序调用OutPutResult()显示结果
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
	Common::Table* table = GetTableByName(into);//获取table
	if (CheckUnique(table, tuple) == 0) {
		throw(record_notfind_error(""));
		return;
	}
	API::inputBuffer.push_back(tuple.GetString());
	string fileName = into + "_rec"; //文件命名为 table_rec
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
		bufferm.Load(handle);//读取文件
		//
		if (bufferm.GetSize(handle) < Buffer::BLOCKCAPACITY) {//如果该块有空余
			recordm.Insert(table, handle);//调用recorm执行插入操作，同时更新index
			bufferm.Save(handle);//保存
		}
		//
		offset += Buffer::BLOCKCAPACITY;
	} while (!inputBuffer.empty());
	bufferm.ResetPin(handle);
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
	Common::Table* table = GetTableByName(from);//获取表头
	std::vector<int>offsets;
	offsets.clear();
	GetOffsets(offsets, table, conditions);//获取需要读取的块位置
	string fileName = from + "_rec"; //文件命名为 table_rec
	int handle = bufferm.NewPage();
	bufferm.SetFilename(fileName, handle);
	bufferm.SetPin(handle);
	recordm.ClearDelRec();
	for (int i = 0; i < offsets.size(); i++) {//依次处理每个offset
		long long tmp = offsets[i];
		bufferm.SetFileOffset(tmp, handle);
		bufferm.Load(handle);//读取指定的块
		recordm.Delete(table, conditions, handle);
		//对每个块使用recordm->Delete()处理tuple;
		bufferm.Save(handle);//删除后保存
	}
	bufferm.ResetPin(handle);
	recordm.FillBlanks(table);//用文件末尾的tuple填删除操作留下的空位
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

	// index manager 对所有是index的列删除对应的B+树
	Common::Table* table = GetTableByName(target);//获取table
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
	for (int j = 0; j < table->attributes.size(); j++) {//输出表头
		fout << table->attributes[j].name << "	";
	}
	fout << endl;
	for (int i = 0,offset; i < screenBuffer.size(); i++) {
		offset = 0;
		for (int j = 0; j < table->attributes.size(); j++) {//对于一行，依次输出各列值
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
				fout << string((char*)(screenBuffer[i].c_str() + offset),table->attributes[j].type) << "	";
				offset += table->attributes[j].type;
			}
		}
		fout << endl;
	}
	//throw(not_completed_exception());
	return;
}
