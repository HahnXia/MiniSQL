#ifndef MiniSQL_BPlusTreeNode_H
#define MiniSQL_BPlusTreeNode_H
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <vector>
#include <string>
using namespace std;

typedef int offsetNumber;

template <class ElementType>
class TreeNode {
private:
	int degree;//结点度数，也是一块中能存储key的数量

public:
	TreeNode(int degree, bool newLeaf = false);
	~TreeNode();

	int keycount;//key的数量
	TreeNode* parent;//父结点
	TreeNode* nextLeafNode;//下一个叶结点（当该结点为叶结点时有效)

	vector<ElementType> keys;//该结点的所有key
	vector<TreeNode*> childs;//指向该结点所有子结点的指针
	vector<offsetNumber> offset;//在块中的偏移量（当该结点为叶结点时>=0，否则为-1)

	bool isLeaf;//是否为叶结点
	bool isRoot();//是否为根结点
	bool search(ElementType key, int &index);//在该结点中搜索key，搜到的位置存储在index中

	TreeNode* devide(ElementType &key, offsetNumber &offset);//分裂结点，并保存右边结点对应的key和偏移量
	
	int insertKey(ElementType key, offsetNumber offset = -1);	//在该结点中增加一个key，并返回位置
	bool deleteKey(int index);//根据index的位置删除该结点的一个key
};








template <class ElementType>
TreeNode<ElementType>::TreeNode(int degree, bool newLeaf)
	:keycount(0), parent(NULL), nextLeafNode(NULL), isLeaf(newLeaf), degree(degree) {
	for(int i = 0; i < degree + 1; i++) {
		childs.push_back(NULL);
		keys.push_back(ElementType());		//存入0
		offset.push_back(offsetNumber());	//存入0
	}
	childs.push_back(NULL);
}

template <class ElementType>
TreeNode<ElementType>::~TreeNode() {}

template <class ElementType>
bool TreeNode<ElementType>::isRoot() {
	return parent == NULL;//没有父结点，即为根结点
}

template <class ElementType>
bool TreeNode<ElementType>::search(ElementType key, int &index) {//搜索key的位置并保存在index中
	if(!keycount) {//没有key
		index = 0;
		return false;
	}
	else if(keys[0] > key) {//比最小key还小
		index = 0;
		return false;
	}
	else if(keys[keycount - 1] < key) {//比最大key还大
		index = keycount;
		return false;
	}
	else {
		int left = 0, right = keycount - 1, pos = 0;
		while(right > left + 1) {			//二分法缩小范围，直到两个相邻的key
			pos = (right + left) / 2;
			if(keys[pos] == key) {
				index = pos;
				return true;
			}
			else if(keys[pos] < key) {
				left = pos;
			}
			else if(keys[pos] > key) {
				right = pos;
			}
		}
		//判断剩余的两个key是否就是要找的key
		if(keys[left] == key) {
			index = left;
			return true;
		}
		else if(keys[right] == key) {
			index = right;
			return true;
		}
		else {	//没找到key
			index = right;
			return false;
		}
	}
	return false;
}


template <class ElementType>
TreeNode<ElementType>* TreeNode<ElementType>::devide(ElementType &key, offsetNumber &offset) {//将一个结点分成两个，返回右边的结点，及其最小key和偏移量
	int midofNode = (degree - 1) / 2;
	TreeNode* newNode = new TreeNode(this->degree, this->isLeaf);
	if(newNode == NULL) {
		cout << "Error: cannot allocate momeory when devide node (" << key << ")" << endl;
		return NULL;
	}

	if(isLeaf) {//叶结点
		key = keys[midofNode + 1];//保存新结点的key
		offset = this->offset[midofNode + 1];//保存新结点的偏移量	
		for(int i = midofNode + 1; i < degree; i++) {//复制结点信息
			newNode->keys[i - midofNode - 1] = this->keys[i];
			newNode->offset[i - midofNode - 1] = this->offset[i];
			this->keys[i] = ElementType();
			this->offset[i] = offsetNumber();
		}

		newNode->nextLeafNode = this->nextLeafNode;
		this->nextLeafNode = newNode;
		newNode->parent = this->parent;
		newNode->keycount = midofNode;
		this->keycount = midofNode + 1;
	}
	else if(!isLeaf) {//非叶结点
		key = keys[midofNode];
		for(int i = midofNode + 1; i < degree + 1; i++) {
			newNode->childs[i - midofNode - 1] = this->childs[i];
			newNode->childs[i - midofNode - 1]->parent = newNode;
			this->childs[i] = NULL;
			if(i != degree) {
				newNode->keys[i - midofNode - 1] = this->keys[i];
				this->keys[i] = ElementType();
			}
		}

		this->keys[midofNode] = ElementType();
		newNode->parent = this->parent;
		newNode->keycount = midofNode;
		this->keycount = midofNode;
		this->nextLeafNode = newNode;		
	}
	return newNode;
}

template <class ElementType>
int TreeNode<ElementType>::insertKey(ElementType key, offsetNumber offset) {	//默认的offset为-1，表示非叶结点，>=0则表示叶结点
	int index;
	bool ifexist = search(key, index);
	//偏移量与结点性质不匹配
	if(offset == -1 && isLeaf) {
		cout << "Error: cannot do this on leaf node" << endl;
		return -1;
	}
	if(offset != -1 && !isLeaf) {
		cout << "Error: cannot do this on non-leaf node" << endl;
		return -1;
	}
	//key已经存在
	if(ifexist) {
		cout << "Error: one value cannot appear in the index twice, value (" << key << ")" << endl;
		return -1;
	}
	else {//插入新的key
		for(int i = keycount; i > index; i--) {
			keys[i] = keys[i - 1];
			if(offset != -1) this->offset[i] = this->offset[i - 1];
		}
		keys[index] = key;
		if(offset != -1) this->offset[index] = offset;

		if(offset == -1) {//非叶结点需要调整子结点信息
			for(int i = keycount + 1; i > index + 1; i--)
				childs[i] = childs[i - 1];
		}
		keycount++;
		return index;
	}
}

template <class ElementType>
bool TreeNode<ElementType>::deleteKey(int index) {//删除index位置的key
	if(index >= keycount) {//这个位置不存在key
		cout << "Error: cannot find the " << index << "-th element" << endl;
		return false;
	}
	if(isLeaf) {//叶结点
		for(int i = index + 1; i < keycount; i++) {
			keys[i - 1] = keys[i];
			offset[i - 1] = offset[i];
		}
		keys[keycount - 1] = ElementType();
		offset[keycount - 1] = offsetNumber();
		keycount--;
	}
	else {//非叶结点
		for(int i = index + 1; i < keycount; i++) {
			keys[i - 1] = keys[i];
		}
		for(int i = index + 1; i < keycount; i++) {
			childs[i] = childs[i + 1];
		}
		keys[keycount - 1] = ElementType();
		childs[keycount] = NULL;
		keycount--;
	}
	return true;
}



#endif