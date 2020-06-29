#pragma once
#include <iostream>
#include <fstream>
#include <cstring>
#include <string>
#include <vector>
#include <set>
#include <cstdio>
#include <io.h>


template<typename T>
constexpr auto min(T a, T b) { return ((a) < (b) ? (a) : (b)); }
template<typename T>
constexpr auto max(T a, T b) { return ((a) > (b) ? (a) : (b)); }

const std::string WORKPATH = "../DataFiles/";

//通用的数据结构头文件，任何模块都可以从这里拿取数据结构
namespace Buffer {
	/*
	Buffer Manager负责缓冲区的管理，主要功能有：
	1.	根据需要，读取指定的数据到系统缓冲区或将缓冲区中的数据写出到文件
	2.	实现缓冲区的替换算法，当缓冲区满时选择合适的页进行替换
	3.	记录缓冲区中各页的状态，如是否被修改过等
	4.	提供缓冲区页的pin功能，及锁定缓冲区的页，不允许替换出去
	为提高磁盘I/O操作的效率，缓冲区与文件系统交互的单位是块，块的大小应为文件系统与磁盘交互单位的整数倍，一般可定为4KB或8KB。
	*/
	constexpr int BLOCKNUM = 24; //块的数量
	constexpr int BLOCKCAPACITY = 4096; //块的大小
	//
}

namespace Common {

	enum class CompareType {
		je = 0, //==
		jne = 1, //!=
		ja = 2, // >
		jae = 3, // >=
		jb = 4, // <
		jbe = 5 // <=
	};

	class Compares { //表达判断条件的数据结构
	public:
		CompareType ctype; //CompareType，类型见enum
		std::string attri; //列
		std::string value; //值 由于不定类型，所以用string存储
	};

	enum AttributeType {
		floatT = -1,
		intT = 0,
		charT = 1
	};

	class Attribute { //列
	public:
		std::string name; //属性名
		std::string indexName;//index的名称 如果没有index记为"#NULL#"
		int type; //-1-float  ; 0-int ; >0 - char ; type大于0时表示char*的长度
		bool unique;//是否unique
		bool primary;//是否为主键
		Attribute() {};
		Attribute(const Attribute& attr)
		{
			this->name = attr.name;
			this->indexName = attr.indexName;
			this->type = attr.type;
			this->unique = attr.unique;
			this->primary = attr.primary;
			return;
		}
		Attribute(std::string _name, std::string _indexName, int _type = 0, bool _unique = 0, bool _primary = 0) {
			this->name = _name;
			this->indexName = _indexName;
			this->type = _type;
			this->unique = _unique;
			this->primary = _primary;
			return;
		}
	};

	class Table {
	public:
		std::string name;//表的名称
		std::vector<Attribute> attributes; //该表含有的列
		int GetDataSize() {
			int res=0;
			for (auto atr : this->attributes) {
				if (atr.type > 0) res += atr.type;
				else res += (atr.type == -1) ? (sizeof(float)) : (sizeof(int));
			}
			return res;
		}
		Table() {};
		~Table() {};
	};
	class Tuple {
	private:
		int len=0;
		char* data;
	public:
		template<typename T>
		T Get(int offset) {//获取int和float
			return *(T*)(this->data + offset);
		}
		std::string Get(const Attribute& attri, int offset) {//获取string
			return std::string(this->data + offset, attri.type);
		}
		template<class T>
		void Set(int offset,T value) {//设置int和float
			*(T*)(this->data + offset) = value;
		}
		template<>
		void Set(int offset, std::string str) {//设置string
			memcpy(this->data+offset, str.c_str(), str.length());
		}
		//以std::string形式返回tuple内容
		std::string GetString() {
			return std::string(this->data, this->len);
		}
		//按指定长度初始化tuple的空间
		Tuple(const int _len) {
			this->len = len;
			data = new char[this->len+1];
			return;
		}
		Tuple(const int _len, const char* src) {
			this->len = _len;
			data = new char[this->len+1];
			memcpy(data, src, _len);
			return;
		}
		//传入对应的表头，计算需要的bytes并初始化空间
		Tuple(Table& tb) {
			this->len = tb.GetDataSize();
			data = new char[this->len+1];
			return;
		}
		Tuple(const Tuple &tp) {
			this->len = tp.len;
			this->data = new char[this->len+1];
			memcpy(this->data, tp.data, this->len);
			return;
		}
		~Tuple() {
			delete[](data);
			this->len = 0;
		}
	};
	//
}
namespace Index {
	//建议Index部分使用独立的文件来保存Btree结构，和buffermanager不产生交互 (?)

}
namespace Catalog {
	/*
	Catalog Manager负责管理数据库的所有模式信息，包括：
	1.	数据库中所有表的定义信息，包括表的名称、表中字段（列）数、主键、定义在该表上的索引。
	2.	表中每个字段的定义信息，包括字段类型、是否唯一等。
	3.	数据库中所有索引的定义，包括所属表、索引建立在那个字段上等。
	Catalog Manager还必需提供访问及操作上述信息的接口，供Interpreter和API模块使用。
	*/

}

namespace API {
	extern std::vector<std::string> screenBuffer; //准备输出到屏幕的Buffer。例如，当RecordManager完成查询操作时，把要输出的内容都丢到这里。
	extern std::vector<std::string> inputBuffer; //准备从丢给其他模块的Buffer。例如Insert时，把要insert的tuple都塞进去。

	//定义塞到RecordM.cpp了
}

namespace Record {
	//该部分依赖Catalog和Buffer
	/*
	Record Manager负责管理记录表中数据的数据文件。主要功能为实现数据文件的创建与删除（由表的定义与删除引起）、记录的插入、删除与查找操作，并对外提供相应的接口。
	其中记录的查找操作要求能够支持不带条件的查找和带一个条件的查找（包括等值查找、不等值查找和区间查找）。
	数据文件由一个或多个数据块组成，块大小应与缓冲区块大小相同。一个块中包含一条至多条记录，
	为简单起见，只要求支持定长记录的存储，且不要求支持记录的跨块存储。
	*/
}