#pragma once
#include "RecordM.h"
#include<cstring>
using namespace std;
using namespace Common;

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
        while(nowpos < maxSize ){
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
				//API::screenBuffer.push_back("a");
				API::screenBuffer.push_back(buffer.substr(posL,posR));
            }
            else if(mode==1){//记录需要删除的位置
                rec.push_back(posL);
            }
        }
        //输出
    }
	void RecordManager::AddIndex(Common::Table * table, const char * str,int value){
		string key;
		for (int j = 0; j < table->attributes.size(); j++) {
			if (table->attributes[j].indexName != "#NULL#") {//index非空时更新
				//setworkspace(table->name,table->attributes[j].name)
				if (table->attributes[j].type == -1) {//float
					key=string(str,sizeof(float));
				}
				else if (table->attributes[j].type == 0) {//int
					key = string(str, sizeof(int));
				}
				else {//char
					key = string(str, (size_t)table->attributes[j].type);
				}
				//addindex(key,value)
			}
		}
		return;
	}
	void RecordManager::RemoveIndex(Common::Table * table, const char * str){
		string key;
		for (int j = 0; j < table->attributes.size(); j++) {
			if (table->attributes[j].indexName != "#NULL#") {//index非空时更新
				//setworkspace(table->name,table->attributes[j].name)
				if (table->attributes[j].type == -1) {//float
					key = string(str, sizeof(float));
				}
				else if (table->attributes[j].type == 0) {//int
					key = string(str, sizeof(int));
				}
				else {//char
					key = string(str, (size_t)table->attributes[j].type);
				}
				//removeindex(key)
			}
		}
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
    //传入table名,判断条件vector,要处理的块,删除满足判断条件的行
	void RecordManager::Delete(Common::Table* table,std::vector<Common::Compares>*condition, const size_t & handle){
        rec.clear();//清空vector rec
        //由rSelect选出所有符合条件的行
		string& buffer = BMP->GetBuffer(handle);
		int mxSize = BMP->GetSize(handle);
        rSelect(table,condition,buffer, mxSize,true);
		//
		//处理所有符合条件的行，由于是删除操作，无需判断溢出
		int tupleLen = table->GetDataSize();
		int nowpos = mxSize - tupleLen;
        for(int i=0;i < rec.size();i++){
			while (nowpos == rec[rec.size()-1]) {//如果末尾行需要删除，则直接删除
				RemoveIndex(table, buffer.c_str() + nowpos);
				nowpos -= tupleLen;
			}
			if (i >= rec.size())break;
			//否则用末尾行替换当前要删除行
			RemoveIndex(table, buffer.c_str() + rec[i]);
			RemoveIndex(table, buffer.c_str() + nowpos);
			buffer.replace((size_t)rec[i], (size_t)tupleLen, buffer.c_str() + nowpos);
			AddIndex(table, buffer.c_str() + rec[i],rec[i]);
			nowpos -= tupleLen;
        }
        return;
    }
	//传入table名，从common::ScreenBuffer的InputBuffer中读取要插入的tuple，写入指定的block中。
	//写入成功的tuple会从InputBuffer中删除。这意味着如果调用rInsert()后InputBuffer中有剩余行，则指定的block已经写满。
	int RecordManager::Insert(Common::Table* table, const size_t & handle){
		
		vector<string>& ibuffer = API::inputBuffer;//引用inputbuffer
		int space;
		
		for (int i = ibuffer.size()-1; i >= 0; ) {
			space = BMP->GetSize(handle);
			if ((int)ibuffer[i].size() > space)break;///////
			BMP->Write(ibuffer[i], handle);
			AddIndex(table, ibuffer[i].c_str(),BMP->GetSize(handle));
			ibuffer.pop_back();
		}
		
        return 0;
    }

}