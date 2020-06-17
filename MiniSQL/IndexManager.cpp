#include "IndexManager.h"

Buffer::BufferManager *Index::Tree::BM = NULL;

int Index::Node::findKeyLoc(std::string _key){
	// find the first key that strictly > _key
	// if no such key exists, return key.size()
	int l = 0, r = key.size();
	while(l != r){
		int mid = (l + r) / 2;
		if(key[mid] > _key) r = mid;
		else l = mid + 1;
	}
	return l;
	// unoptimized version, O(n)
	/*
	int loc = 0;
	while(loc < key.size() && _key >= key[loc]){
		loc++;
	}
	if(l != loc){
		std::cout << l << " " << loc << "node" << id << std::endl;
		throw((std::string)"Stop here!");
	}
	return loc;
	*/
}

int Index::Node::rightSibling(){
	if(!isLeaf || ptr.size() != key.size() + 1) return -1;
	return ptr[ptr.size() - 1];
}

Index::Node::operator std::string(){
	// id#\n
	// parent#\n
	// isLeaf#\n
	// key[0]#key[1]#...#key[n-1]#\n
	// ptr[0]#...#ptr[n-1]#\n
	// ####...#\n
	std::string ret = std::to_string(id) + "#\n" 
		+ std::to_string(parent) + "#\n"
		+ std::to_string(isLeaf) + "#\n";
	for(auto i: key){
		ret += i;
		ret += "#";
	}
	ret += "\n";
	for(auto i: ptr){
		ret += std::to_string(i);
		ret += "#";
	}
	ret += "\n";
	for(int i = ret.length(); i < Buffer::BLOCKCAPACITY - 1; i++)
		ret += "#";
	ret += "\n";
	return ret;
}

Index::Node::Node(std::string _s){
	std::string current = "";
	int status = 0;
	for(auto c: _s){
		if(c == '\n'){
			status++;
			current = "";
		}
		else if(c == '#'){
			if(status == 0) id = std::stoi(current);
			else if (status == 1) parent = std::stoi(current);
			else if (status == 2) isLeaf = std::stoi(current);
			else if (status == 3) key.push_back(current);
			else if (status == 4) ptr.push_back(std::stoi(current));
			else /* reading place-holder '#', do nothing */;
			current = "";
		}
		else current += c;
	}
}

Index::Node::Node(int _id){
	id = _id;
	parent = -1;
	isLeaf = true;
}

Index::Node Index::Tree::readNodeFromDisk(int loc){
	if(loc == -1) return Node();
	BM->NewPage();
	BM->SetFilename("../test/" + name + ".index");
	BM->SetFileOffset(loc * Buffer::BLOCKCAPACITY);
	BM->Load();
	return Node(BM->Read());
}

void Index::Tree::writeNodeToDisk(Node _n){
	BM->NewPage();
	BM->Write(_n);
	BM->SetFilename("../test/" + name + ".index");
	BM->SetFileOffset(_n.id * Buffer::BLOCKCAPACITY);
	BM->Save();
}

void Index::Tree::splitNode(Node _n){
	if(_n.key.size() < degree){
		// no need to split
		writeNodeToDisk(_n);
		return;
	}
	Node sibling(size++);
	sibling.isLeaf = _n.isLeaf;
	sibling.parent = _n.parent;
	int mid = _n.key.size() / 2;
	for(int i = mid; i < _n.key.size(); i++)
		sibling.key.push_back(_n.key[i]);
	for(int i = mid; i < _n.ptr.size(); i++)
		sibling.ptr.push_back(_n.ptr[i]);
	_n.key.resize(mid);
	_n.ptr.resize(mid);
	if(_n.isLeaf) _n.ptr.push_back(sibling.id);

	// update the parent's information
	Node parent;
	if(_n.parent == -1){
		// a new root
		parent = Node(size++);
		parent.isLeaf = false;
		parent.ptr.push_back(_n.id);
		_n.parent = parent.id;
		sibling.parent = parent.id;
		root = parent.id;
	}
	else{
		// load the parent from disk
		parent = readNodeFromDisk(_n.parent);
	}
	int loc = 0;
	while(parent.ptr[loc] != _n.id) loc++;
	parent.ptr.insert(parent.ptr.begin() + loc + 1, sibling.id);
	if(_n.isLeaf){
		parent.key.insert(parent.key.begin() + loc, sibling.key[0]);
	}
	else{
		parent.key.insert(parent.key.begin() + loc, _n.key[mid - 1]);
		_n.key.pop_back();
	}
	// save changes to disk
	writeNodeToDisk(_n);
	writeNodeToDisk(sibling);
	// recursively go up - check the parent
	splitNode(parent);
}

void Index::Tree::mergeNode(Node _a, Node _b){
	// merge node _b to _a
	/*
	if(_a.isLeaf) _a.ptr.pop_back();
	for(auto k: _b.key)
		_a.key.push_back(k);
	for(auto p: _b.ptr)
		_a.ptr.push_back(p);
	writeNodeToDisk(_a);
	*/
}

void Index::Tree::updateAncIndex(Node _n, std::string _old, std::string _new){
	Node anc = readNodeFromDisk(_n.parent);
	while(anc.id != -1){
		int loc = anc.findKeyLoc(_old) - 1;
		if(loc >= 0 && anc.key[loc] == _old)
			anc.key[loc] = _new;
		writeNodeToDisk(anc);
		anc = readNodeFromDisk(anc.parent);
	}
}

void Index::Tree::rebuild(){
	std::vector<std::string> keys;
	std::vector<int> ptrs;
	Node node = readNodeFromDisk(root);
	while(!node.isLeaf)
		node = readNodeFromDisk(node.ptr[0]);
	while(node.id != -1){
		for(int i = 0; i < node.key.size(); i++){
			keys.push_back(node.key[i]);
			ptrs.push_back(node.ptr[i]);
		}
		node = readNodeFromDisk(node.rightSibling());
	}
	destroy();
	size = 1;
	root = 0;
	BM->Write(Node(0));// NEW root of the B+ tree
	BM->Save();
	for(int i = 0; i < keys.size(); i++){
		insert(keys[i], ptrs[i]);
	}
}

int Index::Tree::find(std::string _key){
	Node node = readNodeFromDisk(root);
	while(!node.isLeaf){// until this is a leaf node, do:
		int childNo = node.findKeyLoc(_key);
		node = readNodeFromDisk(node.ptr[childNo]);
	}
	int loc = node.findKeyLoc(_key);
	loc--;
	if(loc < 0 || node.key[loc] != _key) throw("Key " + _key + " doesn't exist");
	return node.ptr[loc];
}

void Index::Tree::insert(std::string _key, int _value){
	Node node = readNodeFromDisk(root);// read the root from disk
	while(!node.isLeaf){// until this is a leaf node, do:
		int childNo = node.findKeyLoc(_key);
		node = readNodeFromDisk(node.ptr[childNo]);
	}
	int loc = node.findKeyLoc(_key);
	node.key.insert(node.key.begin() + loc, _key);
	node.ptr.insert(node.ptr.begin() + loc, _value);
	splitNode(node);
}

void Index::Tree::remove(std::string _key){
	Node node = readNodeFromDisk(root);
	while(!node.isLeaf){// until this is a leaf node, do:
		int childNo = node.findKeyLoc(_key);
		node = readNodeFromDisk(node.ptr[childNo]);
	}
	int loc = node.findKeyLoc(_key) - 1;
	if(loc < 0 || node.key[loc] != _key) throw("Key " + _key + " doesn't exist");
	// delete the key and corresponding ptr
	node.key.erase(node.key.begin() + loc);
	node.ptr.erase(node.ptr.begin() + loc);
	// I: no need to change
	if(node.key.size() >= degree / 2 || node.parent == -1){
		writeNodeToDisk(node);
		return;
	}
	// II: borrow a key from the right sibling
	Node sibling = readNodeFromDisk(node.rightSibling());
	if(sibling.id != -1 && sibling.key.size() > degree / 2){
		// if the right sibling exists and has enough keys
		node.key.push_back(sibling.key[0]);
		node.ptr.insert(node.ptr.end() - 1, sibling.ptr[0]);
		sibling.key.erase(sibling.key.begin());
		sibling.ptr.erase(sibling.ptr.begin());
		writeNodeToDisk(node);
		writeNodeToDisk(sibling);
		if(loc == 0) updateAncIndex(node, _key, node.key[0]);
		updateAncIndex(sibling, node.key[node.key.size() - 1], sibling.key[0]);
		return;
	}
	// III: no enough keys in the right sibling / no right sibling
	writeNodeToDisk(node);
	rebuild();
	// mergeNode(node, sibling);
}

Index::Tree::Tree(std::string _name, int _datawidth){
	name = _name;
	degree = (Buffer::BLOCKCAPACITY - 2*10) / (_datawidth + 10) - 1;
	// 10: bytes per Node id
	BM->NewPage();
	BM->SetFilename("../test/" + name + ".index");
	size = 0;
	if(BM->IsExist()){
		// load this index from disk
		while(BM->Load()){
			size++;
			Node node = Node(BM->Read());
			if(node.parent == -1) root = node.id;// find the root
			BM->SetSize(0);
		}
	}
	else{
		// create a new file for this index on disk
		size++;
		root = 0;
		BM->Write(Node(0));// root of the B+ tree
		BM->Save();
	}
}

Index::Tree::Tree(){
	// this tree doesn't exist
}

void Index::Tree::destroy(){
	BM->NewPage();
	BM->SetFilename("../test/" + name + ".index");
	BM->Delete();
}

void Index::IndexManager::setWorkspace(std::string _table, std::string _attribute){
	workspace = _table + "#" + _attribute;
}

void Index::IndexManager::createIndex(int _datasize){
	if(trees.find(workspace) != trees.end())
		throw("Index on " + workspace + " already existed.");
	trees[workspace] = Tree(workspace, _datasize);
}

void Index::IndexManager::dropIndex(){
	trees[workspace].destroy();
	trees.erase(workspace);
}

int Index::IndexManager::find(std::string _key){
	return trees[workspace].find(_key);
}

void Index::IndexManager::insert(std::string _key, int _value){
	trees[workspace].insert(_key, _value);
}

void Index::IndexManager::remove(std::string _key){
	trees[workspace].remove(_key);
}

void Index::IndexManager::linkBufferManager(Buffer::BufferManager * _BM){
	Tree::BM = _BM;
}

void Index::IndexManager::sample(){
	Index::IndexManager IM;
	Buffer::BufferManager BM;
	IM.linkBufferManager(&BM);
	IM.setWorkspace("student", "sname");
	IM.createIndex(800);
	while(1)
		try{
			std::string cmd, key;
			std::cin >> cmd >> key;
			if(cmd == "ins"){
				int value;
				std::cin >> value;
				IM.insert(key, value);
			}
			else if(cmd == "del"){
				IM.remove(key);
			}
			else{
				std::cout << IM.find(key) << std::endl;
			}
		}
		catch(std::string errmsg){
			std::cout << errmsg << std::endl;
		}
}
