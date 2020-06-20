#include "API.h"
using namespace std;
namespace API {
	std::vector<std::string> screenBuffer; //准备输出到屏幕的Buffer。例如，当RecordManager完成查询操作时，把要输出的内容都丢到这里。
	std::vector<std::string> inputBuffer; //准备从丢给其他模块的Buffer。例如Insert时，把要insert的tuple都塞进去。
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
	//请查看Select的代码行为
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
	//找到对应的表
	Common::Table* table = GetTableByName(from);
	std::vector<int>offsets;
	offsets.clear();
	GetOffsets(offsets, table, conditions);//获取需要读取的块位置
	string fileName = from + "_rec"; //文件命名为 table_rec
	int handle = bufferm.NewPage();
	bufferm.SetFilename(fileName, handle);
	bufferm.SetPin(handle);
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
	API::inputBuffer.push_back(tuple.GetString());
	string fileName = into + "_rec"; //文件命名为 table_rec
	int handle = bufferm.NewPage();
	bufferm.SetFilename(fileName, handle);
	
	long long offset = 0;
	bufferm.SetPin(handle);
	do {
		bufferm.SetFileOffset(offset, handle);
		bufferm.Load(handle);//读取文件
		//
		if (bufferm.GetSize(handle) < Buffer::BLOCKCAPACITY) {//如果该块有空余
			recordm.Insert(table, handle);//调用recorm执行插入操作，同时更新index
			bufferm.Save(handle);//保存

			// index manager 向所有是index的列对应的B+树插入 <key, offset> 对
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
	Common::Table* table = GetTableByName(from);//获取表头
	std::vector<int>offsets;
	offsets.clear();
	GetOffsets(offsets, table, conditions);//获取需要读取的块位置
	string fileName = from + "_rec"; //文件命名为 table_rec
	int handle = bufferm.NewPage();
	bufferm.SetFilename(fileName, handle);
	bufferm.SetPin(handle);
	for (int i = 0; i < offsets.size(); i++) {//依次处理每个offset
		long long tmp = offsets[i];
		bufferm.SetFileOffset(tmp, handle);
		bufferm.Load(handle);//读取指定的块
		recordm.Delete(table, conditions, handle);
		//对每个块使用recordm->Delete()处理tuple;
		bufferm.Save(handle);//删除后保存
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
				fout << *(char*)(screenBuffer[i].c_str() + offset) << "	";
				offset += table->attributes[j].type;
			}
		}
		fout << endl;
	}
	//throw(not_completed_exception());
	return;
}
