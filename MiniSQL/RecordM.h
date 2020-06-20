#pragma once
#include<cstdio>
#include<vector>
#include "Structure.h"
#include "BufferManager.h"
#include "IndexManager.h"
const int max_attributes=50;
namespace Record{
    class RecordManager{
    private:
		Buffer::BufferManager* BMP;
		Index::IndexManager* IDM;
		std::vector<int>rec,attri_offset;
		template<class T> bool cmp(T a,T b,int op) const;
        void rSelect(Common::Table* tableName,std::vector<Common::Compares>*condition, std::string& buffer,int maxSize,bool mode=0);
		void AddIndex(Common::Table* table, const char* str, int value);
		void RemoveIndex(Common::Table* table, const char* str);
	public:
		//指定RecordManager使用的BufferManager
		void LinkBufferManager(Buffer::BufferManager* target) {
			BMP = target;
			return;
		}
		void LinkIndex(Index::IndexManager* target) {
			IDM = target;
			return;
		}
		//传入table名,判断条件vector,要处理的块,将查询到的结果输出到common::ScreenBuffer
		void Select(Common::Table* table,std::vector<Common::Compares>*condition, const size_t & handle = DEFAULTHANDLE);

		//传入table名,判断条件vector,要处理的块,删除满足判断条件的行
		void Delete(Common::Table* table,std::vector<Common::Compares>*condition, const size_t & handle = DEFAULTHANDLE);
		//传入table名，从common::ScreenBuffer的InputBuffer中读取要插入的tuple，写入指定的block中。
		//写入成功的tuple会从InputBuffer中删除。这意味着如果调用rInsert()后InputBuffer中有剩余行，则指定的block已经写满。
		int Insert(Common::Table* table, const size_t & handle = DEFAULTHANDLE);
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
