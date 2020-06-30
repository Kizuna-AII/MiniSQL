#pragma once
#include "RecordM.h"
#include<cstring>
using namespace std;
using namespace Common;
namespace API {
	std::vector<std::string> screenBuffer; //准备输出到屏幕的Buffer。例如，当RecordManager完成查询操作时，把要输出的内容都丢到这里。
	std::vector<std::string> inputBuffer; //准备从丢给其他模块的Buffer。例如Insert时，把要insert的tuple都塞进去。
}
namespace Record{

    //选择指定行
    //mode为0时，输出到common::ScreenBuffer
    //mode为1时，输出到vector rec
    void RecordManager::rSelect(Common::Table* tableName,std::vector<Common::Compares>*condition, string& buffer ,int maxSize,bool mode){
        //读入
        int attri_num = tableName->attributes.size();
		const char* cbuffer = buffer.c_str();
		//
        //查询
        //int stk[max_attributes],top=0;
		//根据表头计算每行的长度
		
		attri_offset.clear();
		int tupleSize = 0;
		for (int i = 0, _type; i < attri_num; i++) {
			attri_offset.push_back(tupleSize);//记录每个attribute的起始位置
			_type = tableName->attributes[i].type;
			if (_type > 0)tupleSize += _type;
			else tupleSize += (_type == -1) ? (sizeof(float)) : (sizeof(int));
		}
		
		int nowpos = 0, nxtpos = 0;
        while(nowpos + tupleSize < maxSize ){
			Tuple tp(tupleSize, cbuffer + nowpos);
            int posL=nowpos;
            //top=0;
            bool flag=1; //判断是否满足条件
            //对于每个条件，O(sizeof(attri))暴力查找其对应的列
            //由于题目限定条件数不会很多，该暴力不会很劣
			int conSize = (condition != NULL) ? (int)condition->size() : 0;
            for(int i=0;i< conSize;i++){
                for(int j=0;j< attri_num ;j++){
                    if(tableName->attributes[j].name==(*condition)[i].attri){
                        if(tableName->attributes[j].type==-1){//float
                            flag&=cmp(tp.Get<float>(attri_offset[j]) ,stof((*condition)[i].value),(int)(*condition)[i].ctype);
                        }
                        else if(tableName->attributes[j].type==0){//int
                            flag&=cmp(tp.Get<int>(attri_offset[j]),stoi((*condition)[i].value),(int)(*condition)[i].ctype);
                        }
                        else if(tableName->attributes[j].type>0){//char
                            flag&=cmp(tp.Get(tableName->attributes[j], attri_offset[j]),(*condition)[i].value,(int)(*condition)[i].ctype);
                        }
                        break;
                    }
                }
            }
			nowpos += tupleSize;
            int posR=nowpos;
            if(flag){//符合条件
                //输出到ScreenBuffer
				API::screenBuffer.push_back(buffer.substr(posL,posR-posL));
            }
            else if(mode==1){//记录需要删除的位置
                rec.push_back(posL);
            }
        }
        //输出
    }
	void RecordManager::AddIndex(Common::Table * table, const char * str,int value){
		string key;
		int offset = 0;
		for (int j = 0; j < table->attributes.size(); j++) {
			if (table->attributes[j].indexName != "#NULL#") {//index非空时更新
				if (table->attributes[j].type == -1) {//float
					key = to_string(*(float*)(str + offset));
					offset += sizeof(float);
				}
				else if (table->attributes[j].type == 0) {//int
					key = to_string(*(int*)(str + offset));
					offset += sizeof(int);
				}
				else {//char
					key = string(str+offset, (size_t)table->attributes[j].type);
					offset += table->attributes[j].type;
				}
				IDM->setWorkspace(table->name, table->attributes[j].name);
				IDM->insert(key, value);
			}
		}
		return;
	}
	void RecordManager::RemoveIndex(Common::Table * table, const char * str){
		string key;
		int offset = 0;
		for (int j = 0; j < table->attributes.size(); j++) {
			if (table->attributes[j].indexName != "#NULL#") {//index非空时更新
				if (table->attributes[j].type == -1) {//float
					key = to_string(*(float*)(str + offset));
					offset += sizeof(float);
				}
				else if (table->attributes[j].type == 0) {//int
					key = to_string(*(int*)(str + offset));
					offset += sizeof(int);
				}
				else {//char
					key = string(str + offset, (size_t)table->attributes[j].type);
					offset += table->attributes[j].type;
				}
				IDM->setWorkspace(table->name, table->attributes[j].name);
				IDM->remove(key);
			}
		}
		return;
	}
	std::string RecordManager::GetRecordFileName(const std::string & tablename)
	{
		return "../DataFiles/Record/" + tablename + "_rec";
	}
	void RecordManager::ClearDelRec(){
		this->rec.clear();
		return;
	}
	//传入table名,判断条件vector,要处理的块
    //将查询到的结果输出到common::ScreenBuffer
    void RecordManager::Select(Common::Table* tableName,std::vector<Common::Compares>*condition, const size_t & handle){
		string buffer = BMP->GetBuffer(handle);
		int mxSize = BMP->GetSize(handle);
		rSelect(tableName,condition,buffer,mxSize,false);
        return;
    }
	void RecordManager::FillBlanks(Common::Table * table){
		string fileName = Record::RecordManager::GetRecordFileName(table->name); //文件命名为 table_rec
		int handle = BMP->NewPage();
		BMP->SetPin(handle);
		int endHandle = BMP->NewPage();
		BMP->SetPin(endHandle);
		BMP->SetFilename(fileName, handle);
		BMP->SetFilename(fileName, endHandle);
		long long last = rec[0];
		BMP->SetFileOffset(last,handle);
		BMP->Load(handle);
		//
		int tupleLen = table->GetDataSize();
		string& buffer = BMP->GetBuffer(handle);
		for (int i = 0; i < rec.size(); i++) {//依次处理每个offset
			long long tmp = rec[i];
			if (tmp >= last + Buffer::BLOCKCAPACITY) {//如果需要，加载新块
				BMP->Save(handle);
				BMP->SetFileOffset(tmp, handle);
				last = tmp;
				BMP->Load(handle);
				buffer = BMP->GetBuffer(handle);
			}
			//读取文件最末一行
			long long src = BMP->GetFileSize(handle) - tupleLen;
			if (rec[i] == src) {
				BMP->SetFileSize(src, handle);
				continue;
			}
			BMP->SetFileOffset(src, endHandle);
			BMP->Load(endHandle);
			//用最末一行替换当前空位，更新索引
			RemoveIndex(table, BMP->GetBuffer(endHandle).c_str());
			AddIndex(table, BMP->GetBuffer(endHandle).c_str(),rec[i]);
			buffer.replace((size_t)rec[i] - last, (size_t)tupleLen, BMP->GetBuffer(endHandle).c_str());
			//删除最末一行，更新文件长度
			BMP->SetFileSize(src, handle);
		}
		//
		BMP->Save(handle);
		BMP->ResetPin(handle);
		BMP->ResetPin(endHandle);
		//
	}
    //传入table名,判断条件vector,要处理的块,删除满足判断条件的行
	void RecordManager::Delete(Common::Table* table,std::vector<Common::Compares>*condition, const size_t & handle){
        //由rSelect选出所有符合条件的行
		string& buffer = BMP->GetBuffer(handle);
		int mxSize = BMP->GetSize(handle);
        rSelect(table,condition,buffer, mxSize,true);
		//获取块自身的偏移量
		int blockOffset = BMP->GetFileOffset(handle);
		//处理所有符合条件的行，删除所有索引
		for (int i = 0; i < rec.size(); i++) {
			RemoveIndex(table, buffer.c_str() + rec[i]);
			rec[i] += blockOffset;//将块内偏移量更新为全局偏移量
		}
        return;
    }
	//传入table名，从common::ScreenBuffer的InputBuffer中读取要插入的tuple，写入指定的block中。
	//写入成功的tuple会从InputBuffer中删除。这意味着如果调用rInsert()后InputBuffer中有剩余行，则指定的block已经写满。
	int RecordManager::Insert(Common::Table* table, const size_t & handle){
		
		vector<string>& ibuffer = API::inputBuffer;//引用inputbuffer
		int space;
		int tupleLen = table->GetDataSize();
		//int tupleLen = table->GetDataSize();
		for (int i = ibuffer.size()-1; i >= 0; i--) {
			space = Buffer::BLOCKCAPACITY - BMP->GetSize(handle);
			if ((int)tupleLen > space)break;
			//更新索引
			AddIndex(table, ibuffer[i].c_str(), BMP->GetFileOffset(handle)+BMP->GetSize(handle));
			//写入数据
			ibuffer[i].resize(tupleLen, '\000');
			BMP->Write(ibuffer[i], handle);
			ibuffer.pop_back();
		}
        return 0;
    }

}