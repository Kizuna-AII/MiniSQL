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
		records tmp;
		tmp.st.clear();
		tmp.ed.clear();
		int tupleSize = 0;
		for (int i = 0, _type; i < attri_num; i++) {
			tmp.st.push_back(tupleSize);//��¼ÿ��attribute����ʼλ��
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
            bool flag=1; //�ж��Ƿ���������
            //����ÿ��������O(sizeof(attri))�����������Ӧ����
            //������Ŀ�޶�����������ܶ࣬�ñ����������
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
            if(flag){//��������
                //�����ScreenBuffer
				//API::screenBuffer.push_back("a");
				API::screenBuffer.push_back(buffer.substr(posL,posR));
            }
            else if(mode==1){//����������,����mode�ж��Ƿ���Ҫ����ѡ��
                rec.push_back(buffer.substr(posL,posR));
            }
        }
        //���
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
	void RecordManager::Delete(Common::Table* tableName,std::vector<Common::Compares>*condition, const size_t & handle){
        rec.clear();//���vector rec
        //��rSelectѡ�����з�����������
		string& buffer = BMP->GetBuffer(handle);
        rSelect(tableName,condition,buffer,true);
		//�����з�����������д��buffer��������ɾ�������������ж����
		buffer.clear();
		BMP->SetSize(0, handle);
        for(register int i=0;i < (int)rec.size();i++){
			BMP->Write(rec[i], handle);
        }
        return;
    }
	//����table������common::ScreenBuffer��InputBuffer�ж�ȡҪ�����tuple��д��ָ����block�С�
	//д��ɹ���tuple���InputBuffer��ɾ��������ζ���������rInsert()��InputBuffer����ʣ���У���ָ����block�Ѿ�д����
	int RecordManager::Insert(Common::Table* tableName, const size_t & handle){
		
		vector<string>& ibuffer = API::inputBuffer;//����inputbuffer
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