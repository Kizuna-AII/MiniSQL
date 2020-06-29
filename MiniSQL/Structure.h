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

//ͨ�õ����ݽṹͷ�ļ����κ�ģ�鶼���Դ�������ȡ���ݽṹ
namespace Buffer {
	/*
	Buffer Manager���𻺳����Ĺ�����Ҫ�����У�
	1.	������Ҫ����ȡָ�������ݵ�ϵͳ�������򽫻������е�����д�����ļ�
	2.	ʵ�ֻ��������滻�㷨������������ʱѡ����ʵ�ҳ�����滻
	3.	��¼�������и�ҳ��״̬�����Ƿ��޸Ĺ���
	4.	�ṩ������ҳ��pin���ܣ���������������ҳ���������滻��ȥ
	Ϊ��ߴ���I/O������Ч�ʣ����������ļ�ϵͳ�����ĵ�λ�ǿ飬��Ĵ�СӦΪ�ļ�ϵͳ����̽�����λ����������һ��ɶ�Ϊ4KB��8KB��
	*/
	constexpr int BLOCKNUM = 24; //�������
	constexpr int BLOCKCAPACITY = 4096; //��Ĵ�С
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

	class Compares { //����ж����������ݽṹ
	public:
		CompareType ctype; //CompareType�����ͼ�enum
		std::string attri; //��
		std::string value; //ֵ ���ڲ������ͣ�������string�洢
	};

	enum AttributeType {
		floatT = -1,
		intT = 0,
		charT = 1
	};

	class Attribute { //��
	public:
		std::string name; //������
		std::string indexName;//index������ ���û��index��Ϊ"#NULL#"
		int type; //-1-float  ; 0-int ; >0 - char ; type����0ʱ��ʾchar*�ĳ���
		bool unique;//�Ƿ�unique
		bool primary;//�Ƿ�Ϊ����
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
		std::string name;//�������
		std::vector<Attribute> attributes; //�ñ��е���
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
		T Get(int offset) {//��ȡint��float
			return *(T*)(this->data + offset);
		}
		std::string Get(const Attribute& attri, int offset) {//��ȡstring
			return std::string(this->data + offset, attri.type);
		}
		template<class T>
		void Set(int offset,T value) {//����int��float
			*(T*)(this->data + offset) = value;
		}
		template<>
		void Set(int offset, std::string str) {//����string
			memcpy(this->data+offset, str.c_str(), str.length());
		}
		//��std::string��ʽ����tuple����
		std::string GetString() {
			return std::string(this->data, this->len);
		}
		//��ָ�����ȳ�ʼ��tuple�Ŀռ�
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
		//�����Ӧ�ı�ͷ��������Ҫ��bytes����ʼ���ռ�
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
	//����Index����ʹ�ö������ļ�������Btree�ṹ����buffermanager���������� (?)

}
namespace Catalog {
	/*
	Catalog Manager����������ݿ������ģʽ��Ϣ��������
	1.	���ݿ������б�Ķ�����Ϣ������������ơ������ֶΣ��У����������������ڸñ��ϵ�������
	2.	����ÿ���ֶεĶ�����Ϣ�������ֶ����͡��Ƿ�Ψһ�ȡ�
	3.	���ݿ������������Ķ��壬���������������������Ǹ��ֶ��ϵȡ�
	Catalog Manager�������ṩ���ʼ�����������Ϣ�Ľӿڣ���Interpreter��APIģ��ʹ�á�
	*/

}

namespace API {
	extern std::vector<std::string> screenBuffer; //׼���������Ļ��Buffer�����磬��RecordManager��ɲ�ѯ����ʱ����Ҫ��������ݶ��������
	extern std::vector<std::string> inputBuffer; //׼���Ӷ�������ģ���Buffer������Insertʱ����Ҫinsert��tuple������ȥ��

	//��������RecordM.cpp��
}

namespace Record {
	//�ò�������Catalog��Buffer
	/*
	Record Manager��������¼�������ݵ������ļ�����Ҫ����Ϊʵ�������ļ��Ĵ�����ɾ�����ɱ�Ķ�����ɾ�����𣩡���¼�Ĳ��롢ɾ������Ҳ������������ṩ��Ӧ�Ľӿڡ�
	���м�¼�Ĳ��Ҳ���Ҫ���ܹ�֧�ֲ��������Ĳ��Һʹ�һ�������Ĳ��ң�������ֵ���ҡ�����ֵ���Һ�������ң���
	�����ļ���һ���������ݿ���ɣ����СӦ�뻺�������С��ͬ��һ�����а���һ����������¼��
	Ϊ�������ֻҪ��֧�ֶ�����¼�Ĵ洢���Ҳ�Ҫ��֧�ּ�¼�Ŀ��洢��
	*/
}