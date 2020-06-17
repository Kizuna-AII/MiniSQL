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
		records tmp;
		tmp.st.clear();
		tmp.ed.clear();
		int tupleSize = 0;
		for (int i = 0, _type; i < attri_num; i++) {
			tmp.st.push_back(tupleSize);//记录每个attribute的起始位置
			_type = tableName->attributes[i].type;
			if (_type > 0)tupleSize += _type;
			else tupleSize += (_type == -1) ? (sizeof(float)) : (sizeof(int));
			//tmp.ed.push_back(nxtpos);
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
                            flag&=cmp(tp.Get<float>(tmp.st[j]) ,stof((*condition)[i].value),(int)(*condition)[i].ctype);
                        }
                        else if(tableName->attributes[j].type==0){//int
                            flag&=cmp(tp.Get<int>(tmp.st[j]),stoi((*condition)[i].value),(int)(*condition)[i].ctype);
                        }
                        else if(tableName->attributes[j].type>0){//char
                            flag&=cmp(tp.Get(tableName->attributes[j],tmp.st[j]),(*condition)[i].value,(int)(*condition)[i].ctype);
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
            else if(mode==1){//不符合条件,根据mode判断是否需要反向选择
                rec.push_back(buffer.substr(posL,posR));
            }
        }
        //输出
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
	void RecordManager::Delete(Common::Table* tableName,std::vector<Common::Compares>*condition, const size_t & handle){
        rec.clear();//清空vector rec
        //由rSelect选出所有符合条件的行
		string& buffer = BMP->GetBuffer(handle);
        rSelect(tableName,condition,buffer,true);
		//将所有符合条件的行写回buffer，由于是删除操作，无需判断溢出
		buffer.clear();
		BMP->SetSize(0, handle);
        for(register int i=0;i < (int)rec.size();i++){
			BMP->Write(rec[i], handle);
        }
        return;
    }
	//传入table名，从common::ScreenBuffer的InputBuffer中读取要插入的tuple，写入指定的block中。
	//写入成功的tuple会从InputBuffer中删除。这意味着如果调用rInsert()后InputBuffer中有剩余行，则指定的block已经写满。
	int RecordManager::Insert(Common::Table* tableName, const size_t & handle){
		
		vector<string>& ibuffer = API::inputBuffer;//引用inputbuffer
		int space;
		
		for (int i = ibuffer.size()-1; i >= 0; ) {
			space = BMP->GetSize(handle);
			if ((int)ibuffer[i].size() > space)break;
			BMP->Write(ibuffer[i], handle);
			ibuffer.pop_back();
		}
		
        return 0;
    }

}