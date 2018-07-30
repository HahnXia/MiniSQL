#include "IndexManager.h"
#include "API.h"

IndexManager::IndexManager(API* ap) :api(ap) {//构造函数，需要读取index文件并建立对应B+树
	vector<IndexInfo> indexList;
	string filepath;
	api->getAllIndex(&indexList);
	for(auto i = indexList.begin(); i != indexList.end(); i++) {
		filepath = i->indexName + ".index";
		readIndex(filepath, i->type);
	}
}

IndexManager::~IndexManager() {
	//销毁对象前将index的改变写回缓冲区中
	for(intMap::iterator itInt = indexIntMap.begin(); itInt != indexIntMap.end(); itInt++) {
		if(itInt->second) {
			itInt->second->WriteBack();
			delete itInt->second;
		}
	}
	for(stringMap::iterator itString = indexStringMap.begin(); itString != indexStringMap.end(); itString++) {
		if(itString->second) {
			itString->second->WriteBack();
			delete itString->second;
		}
	}
	for(floatMap::iterator itFloat = indexFloatMap.begin(); itFloat != indexFloatMap.end(); itFloat++) {
		if(itFloat->second) {
			itFloat->second->WriteBack();
			delete itFloat->second;
		}
	}
}

int IndexManager::getDegree(int type) {//获取块能存储的key数量
	int degree = buffer.getBlockSize() / (getKeySize(type) + sizeof(offsetNumber));
	return degree;
}
int IndexManager::getKeySize(int type) {//获取key的大小
	if(type == TYPE_FLOAT) return sizeof(float);
	else if(type == TYPE_INT) return sizeof(int);
	else if(type > 0) return type + 1;
	else {
		cout << "ERROR: Invalid type" << endl;
		return -1;
	}
}
void IndexManager::setKey(int type, string key) {//将key存入一个临时变量中
	stringstream ss;
	ss << key;
	if(type == TYPE_FLOAT) ss >> this->floatTmp;
	else if(type == TYPE_INT) ss >> this->intTmp;
	else if(type > 0) ss >> this->stringTmp;
	else cout << "ERROR: Invalid type" << endl;
	ss.clear();
}

void IndexManager::createIndex(string filePath, int type) {//建立索引
	ifstream newfile_in(filePath.c_str());
	ofstream newfile_out;
	if(newfile_in) {
		cout << "in Create index: file " << filePath << "already exist" << endl;
		newfile_in.close();
	}
	else {
		newfile_out.open(filePath.c_str());
		newfile_out.close();
	}


	int keysize = getKeySize(type);
	int degree = getDegree(type);
	//创建新文件，并建立对应B+树
	if(type == TYPE_INT) {
		BPlusTree<int> *tree = new BPlusTree<int>(filePath, keysize, degree);
		indexIntMap.insert(intMap::value_type(filePath, tree));
	}
	else if(type == TYPE_FLOAT) {
		BPlusTree<float> *tree = new BPlusTree<float>(filePath, keysize, degree);
		indexFloatMap.insert(floatMap::value_type(filePath, tree));
	}
	else if(type > 0) {
		BPlusTree<string> *tree = new BPlusTree<string>(filePath, keysize, degree);
		indexStringMap.insert(stringMap::value_type(filePath, tree));
	}
	else { 
		cout << "ERROR: in create index: Invalid type" << endl;
	}
}
void IndexManager::dropIndex(string filePath, int type) {//删除索引
	ifstream oldfile(filePath.c_str());
	if(oldfile.is_open()) {
		oldfile.close();
		remove(filePath.c_str());
	}
	//在map容器中查找索引名，找到后删除B+树及该记录
	if(type == TYPE_INT) {
		intMap::iterator itInt = indexIntMap.find(filePath);
		if(itInt == indexIntMap.end()) {
			cout << "Error:in drop index: index not exits" << endl;
			return;
		}
		else {
			delete itInt->second;
			indexIntMap.erase(itInt);
		}
	}
	else if(type == TYPE_FLOAT) {
		floatMap::iterator itFloat = indexFloatMap.find(filePath);
		if(itFloat == indexFloatMap.end()) {
			cout << "Error:in drop index: index not exits" << endl;
			return;
		}
		else {
			delete itFloat->second;
			indexFloatMap.erase(itFloat);
		}
	}
	else if(type > 0) {
		stringMap::iterator itString = indexStringMap.find(filePath);
		if(itString == indexStringMap.end()) {
			cout << "Error:in drop index: index not exits" << endl;
			return;
		}
		else {
			delete itString->second;
			indexStringMap.erase(itString);
		}
	}
	else {
		cout << "ERROR: in drop index: Invalid type" << endl;
	}
}
offsetNumber IndexManager::searchIndex(string filePath, string key, int type) {//查找索引，并返回偏移量
	setKey(type, key);
	//在索引对应B+树中查找key
	if(type == TYPE_INT) {
		intMap::iterator itInt = indexIntMap.find(filePath);
		if(itInt == indexIntMap.end()) {
			cout << "Error:in search index, index not exits" << endl;
			return -1;
		}
		else {
			return itInt->second->Search(intTmp);
		}
	}
	else if(type == TYPE_FLOAT) {
		floatMap::iterator itFloat = indexFloatMap.find(filePath);
		if(itFloat == indexFloatMap.end()) {
			cout << "Error:in search index, index not exits" << endl;
			return -1;
		}
		else {
			return itFloat->second->Search(floatTmp);
		}
	}
	else if(type > 0) {
		stringMap::iterator itString = indexStringMap.find(filePath);
		if(itString == indexStringMap.end()) {
			cout << "Error:in search index, index not exits" << endl;
			return -1;
		}
		else {
			return itString->second->Search(stringTmp);
		}
	}
	else {
		cout << "ERROR: in search index: Invalid type" << endl;
		return -2;
	}
}
void IndexManager::insertIndex(string filePath, string key, offsetNumber blockOffset, int type) {//在指定位置插入key
	setKey(type, key);
	//在索引对应B+树中，根据偏移量插入key
	if(type == TYPE_INT) {
		intMap::iterator itInt = indexIntMap.find(filePath);
		if(itInt == indexIntMap.end()) {
			cout << "Error:in insert index, index not exits" << endl;
			return;
		}
		else {
			itInt->second->Insert(intTmp, blockOffset);
			return;
		}
	}
	else if(type == TYPE_FLOAT) {
		floatMap::iterator itFloat = indexFloatMap.find(filePath);
		if(itFloat == indexFloatMap.end()) {
			cout << "Error:in insert index, index not exits" << endl;
			return;
		}
		else {
			itFloat->second->Insert(floatTmp, blockOffset);
			return;
		}
	}
	else if(type > 0) {
		stringMap::iterator itString = indexStringMap.find(filePath);
		if(itString == indexStringMap.end()) {
			cout << "Error:in insert index, index not exits" << endl;
			return;
		}
		else {
			itString->second->Insert(stringTmp, blockOffset);
			return;
		}
	}
	else {
		cout << "ERROR: in insert index: Invalid type" << endl;
		return;
	}
}
void IndexManager::deleteIndex(string filePath, string key, int type) {//删除key
	setKey(type, key);
	//在索引对应B+树中删除key
	if(type == TYPE_INT) {
		intMap::iterator itInt = indexIntMap.find(filePath);
		if(itInt == indexIntMap.end()) {
			cout << "Error:in delete index, index not exits" << endl;
			return;
		}
		else {
			itInt->second->Delete(intTmp);
			return;
		}
	}
	else if(type == TYPE_FLOAT) {
		floatMap::iterator itFloat = indexFloatMap.find(filePath);
		if(itFloat == indexFloatMap.end()) {
			cout << "Error:in delete index, index not exits" << endl;
			return;
		}
		else {
			itFloat->second->Delete(floatTmp);
			return;
		}
	}
	else if(type > 0) {
		stringMap::iterator itString = indexStringMap.find(filePath);
		if(itString == indexStringMap.end()) {
			cout << "Error:in delete index, index not exits" << endl;
			return;
		}
		else {
			itString->second->Delete(stringTmp);
			return;
		}
	}
	else {
		cout << "ERROR: in delete index: Invalid type" << endl;
		return;
	}
}


void IndexManager::readIndex(string filePath, int type) {//读入索引文件，并创建B+树
	ifstream newfile_in(filePath.c_str());
	ofstream newfile_out;
	if(!newfile_in) {
		cout << "in Create index: file " << filePath << "not exist" << endl;
		newfile_in.close();
	}
	else {
		newfile_out.open(filePath.c_str());
		newfile_out.close();
	}


	int keysize = getKeySize(type);
	int degree = getDegree(type);
	//根据文件名创建不同类型的B+数，并在map容器中建立映射
	if(type == TYPE_INT) {
		BPlusTree<int> *tree = new BPlusTree<int>(filePath, keysize, degree);
		indexIntMap.insert(intMap::value_type(filePath, tree));
	}
	else if(type == TYPE_FLOAT) {
		BPlusTree<float> *tree = new BPlusTree<float>(filePath, keysize, degree);
		indexFloatMap.insert(floatMap::value_type(filePath, tree));
	}
	else if(type > 0) {
		BPlusTree<string> *tree = new BPlusTree<string>(filePath, keysize, degree);
		indexStringMap.insert(stringMap::value_type(filePath, tree));
	}
	else {
		cout << "ERROR: in create index: Invalid type" << endl;
	}
}