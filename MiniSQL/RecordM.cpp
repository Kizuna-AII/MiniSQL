#pragma once
#include "RecordM.h"
#include<cstring>
using namespace std;
using namespace Common;

namespace Record{

    //ѡ��ָ����
    //modeΪ0ʱ�������common::ScreenBuffer
    //modeΪ1ʱ�������vector rec
    void RecordManager::rSelect(Common::Table* tableName,std::vector<Common::Compares>*condition, string& buffer ,int maxSize,bool mode){
        //����
        int attri_num = tableName->attributes.size();
		const char* cbuffer = buffer.c_str();
		//
        //��ѯ
        //int stk[max_attributes],top=0;
		//���ݱ�ͷ����ÿ�еĳ���
		
		attri_offset.clear();
		int tupleSize = 0;
		for (int i = 0, _type; i < attri_num; i++) {
			attri_offset.push_back(tupleSize);//��¼ÿ��attribute����ʼλ��
			_type = tableName->attributes[i].type;
			if (_type > 0)tupleSize += _type;
			else tupleSize += (_type == -1) ? (sizeof(float)) : (sizeof(int));
		}
		
		int nowpos = 0, nxtpos = 0;
        while(nowpos < maxSize ){
			Tuple tp(tupleSize, cbuffer + nowpos);
            int posL=nowpos;
            //top=0;
            bool flag=1; //�ж��Ƿ���������
            //����ÿ��������O(sizeof(attri))�����������Ӧ����
            //������Ŀ�޶�����������ܶ࣬�ñ����������
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
            if(flag){//��������
                //�����ScreenBuffer
				//API::screenBuffer.push_back("a");
				API::screenBuffer.push_back(buffer.substr(posL,posR));
            }
            else if(mode==1){//��¼��Ҫɾ����λ��
                rec.push_back(posL);
            }
        }
        //���
    }
	void RecordManager::AddIndex(Common::Table * table, const char * str,int value){
		string key;
		for (int j = 0; j < table->attributes.size(); j++) {
			if (table->attributes[j].indexName != "#NULL#") {//index�ǿ�ʱ����
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
			if (table->attributes[j].indexName != "#NULL#") {//index�ǿ�ʱ����
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
    //����table��,�ж�����vector,Ҫ����Ŀ�
    //����ѯ���Ľ�������common::ScreenBuffer
    void RecordManager::Select(Common::Table* tableName,std::vector<Common::Compares>*condition, const size_t & handle){
		string buffer = BMP->GetBuffer(handle);
		int mxSize = BMP->GetSize(handle);
		rSelect(tableName,condition,buffer,mxSize,false);
        return;
    }
    //����table��,�ж�����vector,Ҫ����Ŀ�,ɾ�������ж���������
	void RecordManager::Delete(Common::Table* table,std::vector<Common::Compares>*condition, const size_t & handle){
        rec.clear();//���vector rec
        //��rSelectѡ�����з�����������
		string& buffer = BMP->GetBuffer(handle);
		int mxSize = BMP->GetSize(handle);
        rSelect(table,condition,buffer, mxSize,true);
		//
		//�������з����������У�������ɾ�������������ж����
		int tupleLen = table->GetDataSize();
		int nowpos = mxSize - tupleLen;
        for(int i=0;i < rec.size();i++){
			while (nowpos == rec[rec.size()-1]) {//���ĩβ����Ҫɾ������ֱ��ɾ��
				RemoveIndex(table, buffer.c_str() + nowpos);
				nowpos -= tupleLen;
			}
			if (i >= rec.size())break;
			//������ĩβ���滻��ǰҪɾ����
			RemoveIndex(table, buffer.c_str() + rec[i]);
			RemoveIndex(table, buffer.c_str() + nowpos);
			buffer.replace((size_t)rec[i], (size_t)tupleLen, buffer.c_str() + nowpos);
			AddIndex(table, buffer.c_str() + rec[i],rec[i]);
			nowpos -= tupleLen;
        }
        return;
    }
	//����table������common::ScreenBuffer��InputBuffer�ж�ȡҪ�����tuple��д��ָ����block�С�
	//д��ɹ���tuple���InputBuffer��ɾ��������ζ���������rInsert()��InputBuffer����ʣ���У���ָ����block�Ѿ�д����
	int RecordManager::Insert(Common::Table* table, const size_t & handle){
		
		vector<string>& ibuffer = API::inputBuffer;//����inputbuffer
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