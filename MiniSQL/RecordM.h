#pragma once
#include<cstdio>
#include<vector>
#include "Structure.h"
#include "BufferManager.h"
const int max_attributes=50;
namespace Record{
    class RecordManager{
    private:
		Buffer::BufferManager* BMP;
        class records{
			public:
            std::vector<int> st;
			std::vector<int> ed;
        };
		std::vector<std::string>rec;
		template<class T> bool cmp(T a,T b,int op) const;
        void rSelect(Common::Table* tableName,std::vector<Common::Compares>*condition, std::string& buffer,int maxSize,bool mode=0);
	public:
		//ָ��RecordManagerʹ�õ�BufferManager
		void LinkBufferManager(Buffer::BufferManager* target) {
			BMP = target;
			return;
		}
		//����table��,�ж�����vector,Ҫ����Ŀ�,����ѯ���Ľ�������common::ScreenBuffer
		void Select(Common::Table* tableName,std::vector<Common::Compares>*condition, const size_t & handle = DEFAULTHANDLE);

		//����table��,�ж�����vector,Ҫ����Ŀ�,ɾ�������ж���������
		void Delete(Common::Table* tableName,std::vector<Common::Compares>*condition, const size_t & handle = DEFAULTHANDLE);
		//����table������common::ScreenBuffer��InputBuffer�ж�ȡҪ�����tuple��д��ָ����block�С�
		//д��ɹ���tuple���InputBuffer��ɾ��������ζ���������rInsert()��InputBuffer����ʣ���У���ָ����block�Ѿ�д����
		int Insert(Common::Table* tableName, const size_t & handle = DEFAULTHANDLE);
	};
	template<class T>
   	bool RecordManager::cmp(T a,T b,int op) const{  
        //op: 0 - == ; 1 - != ; 2 - > ; 3 - >= ; 4 - < ; 5 - <= ;
        switch(op){
            case 0:{
                return a == b;
            }
            case 1:{
                return a != b;
            }
            case 2:{
                return a > b;
            }
            case 3:{
                return a >= b;
            }
            case 4:{
                return a < b;
            }
            case 5:{
                return a <= b;
            }
        }
        return false;
    }
}
