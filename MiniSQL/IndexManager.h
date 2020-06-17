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
		Node readNodeFromDisk(int loc);
		void writeNodeToDisk(Node _n);
		void splitNode(Node _n);
		void mergeNode(Node _a, Node _b);
		void updateAncIndex(Node _n, std::string _old, std::string _new);
		void rebuild();
	public:
		static Buffer::BufferManager *BM;
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
		void setWorkspace(std::string _table, std::string _attribute);
		// ѡ��Ҫ��дindex�ı�����С������Ǻ��������Ļ���	
		
		void createIndex(int _datawidth);
		// �ڵ�ǰ�ı���д�������
		// ����������еĿ�ȣ��ֽ���������ȷ��B+���Ĳ���
		
		void dropIndex();
		// �ڵ�ǰ�ı����drop����
		
		int find(std::string _key);
		// ͨ��key���Ҷ�Ӧ��tuple
		
		void insert(std::string _key, int _value);
		// ����һ��<key, value>��
		// key������string�����������int/double��Ҫת��string
		// value��ָ��ĳ��tuple��"ָ��"����ʽ��API�������index managerֻ����洢��һ��Ϣ
		
		void remove(std::string _key);
		// ɾ��ĳkey��Ӧ��<key, value>��
		
		void linkBufferManager(Buffer::BufferManager* _BM);
		// ��BufferManager�����ڶ�дӲ��ǰδ�󶨻�����RE��
		
		static void sample();
		// IndexManagerʹ������
	};
}