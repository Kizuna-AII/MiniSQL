#pragma once
#include "RecordM.h"
#include<cstring>
using namespace std;
using namespace Common;
namespace API {
	std::vector<std::string> screenBuffer; //׼���������Ļ��Buffer�����磬��RecordManager��ɲ�ѯ����ʱ����Ҫ��������ݶ��������
	std::vector<std::string> inputBuffer; //׼���Ӷ�������ģ���Buffer������Insertʱ����Ҫinsert��tuple������ȥ��
}
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
        while(nowpos + tupleSize < maxSize ){
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
				API::screenBuffer.push_back(buffer.substr(posL,posR-posL));
            }
            else if(mode==1){//��¼��Ҫɾ����λ��
                rec.push_back(posL);
            }
        }
        //���
    }
	void RecordManager::AddIndex(Common::Table * table, const char * str,int value){
		string key;
		int offset = 0;
		for (int j = 0; j < table->attributes.size(); j++) {
			if (table->attributes[j].indexName != "#NULL#") {//index�ǿ�ʱ����
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
			if (table->attributes[j].indexName != "#NULL#") {//index�ǿ�ʱ����
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
	//����table��,�ж�����vector,Ҫ����Ŀ�
    //����ѯ���Ľ�������common::ScreenBuffer
    void RecordManager::Select(Common::Table* tableName,std::vector<Common::Compares>*condition, const size_t & handle){
		string buffer = BMP->GetBuffer(handle);
		int mxSize = BMP->GetSize(handle);
		rSelect(tableName,condition,buffer,mxSize,false);
        return;
    }
	void RecordManager::FillBlanks(Common::Table * table){
		string fileName = Record::RecordManager::GetRecordFileName(table->name); //�ļ�����Ϊ table_rec
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
		for (int i = 0; i < rec.size(); i++) {//���δ���ÿ��offset
			long long tmp = rec[i];
			if (tmp >= last + Buffer::BLOCKCAPACITY) {//�����Ҫ�������¿�
				BMP->Save(handle);
				BMP->SetFileOffset(tmp, handle);
				last = tmp;
				BMP->Load(handle);
				buffer = BMP->GetBuffer(handle);
			}
			//��ȡ�ļ���ĩһ��
			long long src = BMP->GetFileSize(handle) - tupleLen;
			if (rec[i] == src) {
				BMP->SetFileSize(src, handle);
				continue;
			}
			BMP->SetFileOffset(src, endHandle);
			BMP->Load(endHandle);
			//����ĩһ���滻��ǰ��λ����������
			RemoveIndex(table, BMP->GetBuffer(endHandle).c_str());
			AddIndex(table, BMP->GetBuffer(endHandle).c_str(),rec[i]);
			buffer.replace((size_t)rec[i] - last, (size_t)tupleLen, BMP->GetBuffer(endHandle).c_str());
			//ɾ����ĩһ�У������ļ�����
			BMP->SetFileSize(src, handle);
		}
		//
		BMP->Save(handle);
		BMP->ResetPin(handle);
		BMP->ResetPin(endHandle);
		//
	}
    //����table��,�ж�����vector,Ҫ����Ŀ�,ɾ�������ж���������
	void RecordManager::Delete(Common::Table* table,std::vector<Common::Compares>*condition, const size_t & handle){
        //��rSelectѡ�����з�����������
		string& buffer = BMP->GetBuffer(handle);
		int mxSize = BMP->GetSize(handle);
        rSelect(table,condition,buffer, mxSize,true);
		//��ȡ�������ƫ����
		int blockOffset = BMP->GetFileOffset(handle);
		//�������з����������У�ɾ����������
		for (int i = 0; i < rec.size(); i++) {
			RemoveIndex(table, buffer.c_str() + rec[i]);
			rec[i] += blockOffset;//������ƫ��������Ϊȫ��ƫ����
		}
        return;
    }
	//����table������common::ScreenBuffer��InputBuffer�ж�ȡҪ�����tuple��д��ָ����block�С�
	//д��ɹ���tuple���InputBuffer��ɾ��������ζ���������rInsert()��InputBuffer����ʣ���У���ָ����block�Ѿ�д����
	int RecordManager::Insert(Common::Table* table, const size_t & handle){
		
		vector<string>& ibuffer = API::inputBuffer;//����inputbuffer
		int space;
		int tupleLen = table->GetDataSize();
		//int tupleLen = table->GetDataSize();
		for (int i = ibuffer.size()-1; i >= 0; i--) {
			space = Buffer::BLOCKCAPACITY - BMP->GetSize(handle);
			if ((int)tupleLen > space)break;
			//��������
			AddIndex(table, ibuffer[i].c_str(), BMP->GetFileOffset(handle)+BMP->GetSize(handle));
			//д������
			ibuffer[i].resize(tupleLen, '\000');
			BMP->Write(ibuffer[i], handle);
			ibuffer.pop_back();
		}
        return 0;
    }

}