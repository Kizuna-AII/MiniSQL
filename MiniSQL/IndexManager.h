#pragma once
#include "Structure.h"
#include "BufferManager.h"
#include <string>
namespace Index{
	class Node{
	public:
		int id;
		int parent;
		bool isLeaf;
		std::vector<int> ptr;
		std::vector<std::string> key;
		int findKeyLoc(std::string _key);
		int rightSibling();
		operator std::string();
		Node(std::string _s);
		Node(int _id = -1);
	};

	class Tree{
	private:
		std::string name;
		int size;
		int degree;
		int root;
		void splitNode(Node _n);
		void mergeNode(Node _a, Node _b);
		void updateAncIndex(Node _n, std::string _old, std::string _new);
		void rebuild();
	public:
		static Buffer::BufferManager *BM;
		Node readNodeFromDisk(int loc);
		void writeNodeToDisk(Node _n);
		Node findNode(std::string _key);
		Node leftMostNode();
		int find(std::string _key);
		void insert(std::string _key, int _value);
		void remove(std::string _key);
		void destroy();
		Tree(std::string _name, int _datawidth);
		Tree();
	};

	class IndexManager{
	private:
		std::map<std::string, Tree> trees;
		std::string workspace;
	public:
		// 选定要读写index的表格与列——这是后续操作的基础	
		void setWorkspace(std::string _table_attr);
		void setWorkspace(std::string _table, std::string _attribute);
		
		// 在当前的表格、列创建索引
		// 必须给出该列的宽度（字节数），以确定B+树的叉数
		void createIndex(int _datawidth = 50);
		
		// 在当前的表格、列drop索引
		void dropIndex();

		// 检查在当前表格、列上是否存在index
		bool existIndex();
		
		// 通过key查找对应的tuple
		int find(std::string _key);

		// 通过条件查找对应的【对应的块】offsets
		// 例如，找到了ptr信息{1000,4097,5100}， 结果会是{0,4096}
		std::set<int> select(Common::Compares _con);
		
		// 插入一个<key, value>对
		// key必须以string给出，如果是int/double需要转成string
		// value即指向某个tuple的"指针"，形式由API层给出，index manager只负责存储这一信息
		void insert(std::string _key, int _value);
		
		// 删除某key对应的<key, value>对
		void remove(std::string _key);
		
		// 绑定BufferManager——在读写硬盘前未绑定会引发RE！
		void linkBufferManager(Buffer::BufferManager* _BM);
		
		// IndexManager使用样例
		static void sample();
	};
}