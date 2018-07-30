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
	int degree;//��������Ҳ��һ�����ܴ洢key������

public:
	TreeNode(int degree, bool newLeaf = false);
	~TreeNode();

	int keycount;//key������
	TreeNode* parent;//�����
	TreeNode* nextLeafNode;//��һ��Ҷ��㣨���ý��ΪҶ���ʱ��Ч)

	vector<ElementType> keys;//�ý�������key
	vector<TreeNode*> childs;//ָ��ý�������ӽ���ָ��
	vector<offsetNumber> offset;//�ڿ��е�ƫ���������ý��ΪҶ���ʱ>=0������Ϊ-1)

	bool isLeaf;//�Ƿ�ΪҶ���
	bool isRoot();//�Ƿ�Ϊ�����
	bool search(ElementType key, int &index);//�ڸý��������key���ѵ���λ�ô洢��index��

	TreeNode* devide(ElementType &key, offsetNumber &offset);//���ѽ�㣬�������ұ߽���Ӧ��key��ƫ����
	
	int insertKey(ElementType key, offsetNumber offset = -1);	//�ڸý��������һ��key��������λ��
	bool deleteKey(int index);//����index��λ��ɾ���ý���һ��key
};








template <class ElementType>
TreeNode<ElementType>::TreeNode(int degree, bool newLeaf)
	:keycount(0), parent(NULL), nextLeafNode(NULL), isLeaf(newLeaf), degree(degree) {
	for(int i = 0; i < degree + 1; i++) {
		childs.push_back(NULL);
		keys.push_back(ElementType());		//����0
		offset.push_back(offsetNumber());	//����0
	}
	childs.push_back(NULL);
}

template <class ElementType>
TreeNode<ElementType>::~TreeNode() {}

template <class ElementType>
bool TreeNode<ElementType>::isRoot() {
	return parent == NULL;//û�и���㣬��Ϊ�����
}

template <class ElementType>
bool TreeNode<ElementType>::search(ElementType key, int &index) {//����key��λ�ò�������index��
	if(!keycount) {//û��key
		index = 0;
		return false;
	}
	else if(keys[0] > key) {//����Сkey��С
		index = 0;
		return false;
	}
	else if(keys[keycount - 1] < key) {//�����key����
		index = keycount;
		return false;
	}
	else {
		int left = 0, right = keycount - 1, pos = 0;
		while(right > left + 1) {			//���ַ���С��Χ��ֱ���������ڵ�key
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
		//�ж�ʣ�������key�Ƿ����Ҫ�ҵ�key
		if(keys[left] == key) {
			index = left;
			return true;
		}
		else if(keys[right] == key) {
			index = right;
			return true;
		}
		else {	//û�ҵ�key
			index = right;
			return false;
		}
	}
	return false;
}


template <class ElementType>
TreeNode<ElementType>* TreeNode<ElementType>::devide(ElementType &key, offsetNumber &offset) {//��һ�����ֳ������������ұߵĽ�㣬������Сkey��ƫ����
	int midofNode = (degree - 1) / 2;
	TreeNode* newNode = new TreeNode(this->degree, this->isLeaf);
	if(newNode == NULL) {
		cout << "Error: cannot allocate momeory when devide node (" << key << ")" << endl;
		return NULL;
	}

	if(isLeaf) {//Ҷ���
		key = keys[midofNode + 1];//�����½���key
		offset = this->offset[midofNode + 1];//�����½���ƫ����	
		for(int i = midofNode + 1; i < degree; i++) {//���ƽ����Ϣ
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
	else if(!isLeaf) {//��Ҷ���
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
int TreeNode<ElementType>::insertKey(ElementType key, offsetNumber offset) {	//Ĭ�ϵ�offsetΪ-1����ʾ��Ҷ��㣬>=0���ʾҶ���
	int index;
	bool ifexist = search(key, index);
	//ƫ�����������ʲ�ƥ��
	if(offset == -1 && isLeaf) {
		cout << "Error: cannot do this on leaf node" << endl;
		return -1;
	}
	if(offset != -1 && !isLeaf) {
		cout << "Error: cannot do this on non-leaf node" << endl;
		return -1;
	}
	//key�Ѿ�����
	if(ifexist) {
		cout << "Error: one value cannot appear in the index twice, value (" << key << ")" << endl;
		return -1;
	}
	else {//�����µ�key
		for(int i = keycount; i > index; i--) {
			keys[i] = keys[i - 1];
			if(offset != -1) this->offset[i] = this->offset[i - 1];
		}
		keys[index] = key;
		if(offset != -1) this->offset[index] = offset;

		if(offset == -1) {//��Ҷ�����Ҫ�����ӽ����Ϣ
			for(int i = keycount + 1; i > index + 1; i--)
				childs[i] = childs[i - 1];
		}
		keycount++;
		return index;
	}
}

template <class ElementType>
bool TreeNode<ElementType>::deleteKey(int index) {//ɾ��indexλ�õ�key
	if(index >= keycount) {//���λ�ò�����key
		cout << "Error: cannot find the " << index << "-th element" << endl;
		return false;
	}
	if(isLeaf) {//Ҷ���
		for(int i = index + 1; i < keycount; i++) {
			keys[i - 1] = keys[i];
			offset[i - 1] = offset[i];
		}
		keys[keycount - 1] = ElementType();
		offset[keycount - 1] = offsetNumber();
		keycount--;
	}
	else {//��Ҷ���
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