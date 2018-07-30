#ifndef MiniSQL_BPlusTree_H
#define MiniSQL_BPlusTree_H

#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <vector>
#include <string>
#include "BufferManager.h"
#include "BPlusTreeNode.h"
using namespace std;

template <class ElementType>
struct NodeInfo {//�����Ϣ�࣬���ڼ�¼�����Ϣ���Ƚ������ռ�ռ�С
	TreeNode<ElementType>* pNode;//ָ���Ӧ����ָ��
	int index;//��Ҫ��key�ڸýڵ��е�λ��
	bool exist;//�ýڵ����Ƿ������Ҫ��key
};

template <class ElementType>
class BPlusTree {//B+����
private:
	BufferManager buffer;//������
	typedef TreeNode<ElementType>* Node;//���ڵ�ָ�붨��ΪNode��������д

	string filename;//��Ӧ�������ļ���
	Node root;//���ĸ��ڵ�
	Node leafhead;//��һ��Ҷ���
	FileNode* file;//�����ļ����
	int degree;//���Ķ�����Ҳ��һ�����ܱ���key�ĸ���
	int keycount;//����key�ĸ���
	int level;//���Ĳ���
	int nodecount;//���Ľ����
	int keysize;//key�Ĵ�С

	ElementType FindMin(Node pNode);//����pNode����Ӧ��������С��key
	bool AfterInsert(Node pNode);//��pNode��ִ��������ĵ���
	bool AfterDelete(Node pNode);//��pNode��ִ����ɾ����ĵ���
	void FindLeaf(Node pNode, ElementType key, NodeInfo<ElementType> &info);//��pNode��Ӧ�����в���key����Ҷ��㣬��������Ϣ������info��

public:
	BPlusTree(string filename, int keysize, int degree);
	~BPlusTree();
	void ReadTree();//���ļ��ж���key��Ϣ�����ɶ�Ӧ����
	void DropTree(Node pNode);//ɾ��pNode����Ӧ����
	void WriteBack();//�����е�key��Ϣд�ػ�����

	offsetNumber Search(ElementType key);//����һ��key����������ƫ����
	bool Insert(ElementType key, offsetNumber offset);//��offsetλ�ò���һ��key
	bool Delete(ElementType key);//ɾ��һ��key

};




template <class ElementType>
BPlusTree<ElementType>::BPlusTree(string filename, int keysize, int degree)
	:filename(filename), keycount(0), level(1), nodecount(1), root(NULL), leafhead(NULL), keysize(keysize), file(NULL), degree(degree) {
	root = new TreeNode<ElementType>(degree, true);

	leafhead = root;
	ReadTree();
}

template <class ElementType>
BPlusTree<ElementType>:: ~BPlusTree() {
	DropTree(root);
}

template <class ElementType>
void BPlusTree<ElementType>::DropTree(Node pnode) {//ɾ��������
	if(!pnode) return;
	if(!pnode->isLeaf) {
		for(int i = 0; i <= pnode->keycount; i++) {
			DropTree(pnode->childs[i]);//�ݹ�ɾ������
			pnode->childs[i] = NULL;
		}
	}
	delete pnode;
	return;
}

template <class ElementType>
void BPlusTree<ElementType>::ReadTree() {
	//���ݶ�Ӧ�ļ���ȡ���ͷ
	file = buffer.getFile(filename.c_str());
	BlockNode *tempblock = buffer.getBlockHead(file);

	while(1) {
		if(!tempblock) break;
		{
			int offsetsize = sizeof(offsetNumber);
			char* indexbegin = buffer.getContent(tempblock);
			char* offsetbegin = indexbegin + keysize;
			ElementType key;
			offsetNumber offset;
			while(offsetbegin - indexbegin < buffer.getUsingSize(tempblock)) {//ѭ������key��offset������������
				key = *(ElementType*)indexbegin;
				offset = *(offsetNumber*)offsetbegin;
				Insert(key, offset);
				indexbegin += keysize + offsetsize;
				offsetbegin += keysize + offsetsize;
			}
		}

		if(tempblock->isBottom) break;
		tempblock = buffer.getNextBlock(file, tempblock);
	}

}

template <class ElementType>
void BPlusTree<ElementType>::WriteBack() {//��key��offset��Ϣд�ػ�����
	BlockNode * tempblock = buffer.getBlockHead(file);
	Node tempnode = leafhead;
	int offsetsize = sizeof(offsetNumber);
	while(tempnode != NULL) {//��Ҷ����ж�ȡkey��offset��Ϣ����д�ؿ���
		buffer.setUsingSize(tempblock, 0);
		buffer.set_dirty(tempblock);
		for(int i = 0; i < tempnode->keycount; i++) {
			char * key = (char *)&(tempnode->keys[i]);
			char * offset = (char *)&(tempnode->offset[i]);
			memcpy(buffer.getContent(tempblock) + buffer.getUsingSize(tempblock), key, keysize);
			buffer.setUsingSize(tempblock, buffer.getUsingSize(tempblock) + keysize);
			memcpy(buffer.getContent(tempblock) + buffer.getUsingSize(tempblock), offset, offsetsize);
			buffer.setUsingSize(tempblock, buffer.getUsingSize(tempblock) + offsetsize);
		}
		tempblock = buffer.getNextBlock(file, tempblock);
		tempnode = tempnode->nextLeafNode;
	}
	//д��ʣ�µĿտ�
	while(true) {
		if(tempblock->isBottom) break;
		buffer.setUsingSize(tempblock, 0);
		buffer.set_dirty(tempblock);
		tempblock = buffer.getNextBlock(file, tempblock);
	}
}

template <class ElementType>
offsetNumber BPlusTree<ElementType>::Search(ElementType key) {//�����в���key������ƫ����
	if(!root) return -1;
	NodeInfo<ElementType> targetnode;
	FindLeaf(root, key, targetnode);//��Ҷ����в���key
	if (!targetnode.exist) {//û�ҵ�
		return -1;
	}else {
		return targetnode.pNode->offset[targetnode.index];
	}
}

template <class ElementType>
bool BPlusTree<ElementType>::Insert(ElementType key, offsetNumber offset) {//����ƫ��������key
	NodeInfo<ElementType> newnode;
	if(!root) {//��Ϊ�գ���û�и��ڵ�
		root = new TreeNode<ElementType>(degree, true);
		leafhead = root;
	}
	FindLeaf(root, key, newnode);
	if(newnode.exist) {//�����Ѵ��ڸ�key
		cout << "Error:Cannot inserrt key to index: the duplicated key!" << endl;
		return false;
	}
	else {
		newnode.pNode->insertKey(key, offset);
		if(newnode.pNode->keycount == degree)//��������key�������ڶ����������ڶ���-1������Ҫ����
			AfterInsert(newnode.pNode);
		keycount++;
		return true;
	}
}

template <class ElementType>
bool BPlusTree<ElementType>::Delete(ElementType key) {//������ɾ��key
	NodeInfo<ElementType> newnode;
	if(!root) {//��Ϊ��
		cout << "Error:Cannot delete key from index: the tree not exist!" << endl;
		return false;
	}
	FindLeaf(root, key, newnode);
	if(!newnode.exist) {//����û��key
		cout << "Error:Cannot delete key from index: the key not exist!" << endl;
		return false;
	}
	else {
		if(newnode.pNode->isRoot()) {//���ڵ㣬ֱ��ɾ��
			newnode.pNode->deleteKey(newnode.index);
			keycount--;
		}
		else {
			if(newnode.index == 0 && leafhead != newnode.pNode) {//key�ڷ�֧�У����ڵ�ǰ������
				int index = 0;

				Node parent = newnode.pNode->parent;//�ڸ��߲������key
				bool if_found_in_branch = parent->search(key, index);
				while(!if_found_in_branch) {
					if(parent->parent)
						parent = parent->parent;
					else
						break;
					if_found_in_branch = parent->search(key, index);
				}

				parent->keys[index] = newnode.pNode->keys[1];

				newnode.pNode->deleteKey(newnode.index);
				keycount--;
			}
			else {//key��Ҷ�����
				newnode.pNode->deleteKey(newnode.index);
				keycount--;
			}
		}
		if((newnode.pNode->keycount >= (degree - 1) / 2 - 1)||this->nodecount==1)//ɾ���������
			return true;
		else//���������
			return AfterDelete(newnode.pNode);
	}
}

template <class ElementType>
bool BPlusTree<ElementType>::AfterInsert(Node pNode) {//����key��ĵ���
	ElementType key;
	offsetNumber offset;	
	Node newnode = pNode->devide(key, offset);//��ǰ������
	nodecount++;
	if(pNode->isRoot()) {//ԭ���Ϊ����㣬���Ѻ�Ӧ�����µĸ����
		Node root = new TreeNode<ElementType>(degree, false);
		if(root == NULL) {//�����½��ʧ��
			cout << "Error: can not allocate memory for the new root after insert" << endl;
			return false;
		}
		else {//�����Ϣ����
			this->root = root;
			root->insertKey(key, -1);
			root->childs[0] = pNode;
			root->childs[1] = newnode;
			pNode->parent = root;
			newnode->parent = root;
			level++;//���Ĳ�������
			nodecount++;
			return true;
		}
	}
	else {//���Ǹ����
		Node parent = pNode->parent;
		int index = parent->insertKey(key);//������в����½����С��key
		parent->childs[index + 1] = newnode;//���������һ���ӽ��
		newnode->parent = parent;
		if(parent->keycount == degree) {//������key��Ҳ�������ƣ���Ҫ����
			return AfterInsert(parent);
		}
		return true;
	}
}

template <class ElementType>
ElementType BPlusTree<ElementType>::FindMin(Node pNode) {//������Сkey
	if(pNode->childs[0]) {//���и�С��������ݹ����
		return FindMin(pNode->childs[0]);
	}
	else {//���򷵻ص�ǰ�����С��key
		return pNode->keys[0];
	}
}

template <class ElementType>
bool BPlusTree<ElementType>::AfterDelete(Node pNode) {//ɾ��key�����
	int half_of_degree = (degree - 1) / 2;//������һ�룬���ں�������key���Ƿ��㹻
	if(pNode->isRoot()) {//���ڵ�
		if(pNode->keycount == 0) {
			return false;
		}
		else if(pNode->isLeaf) {//���Ҳ��Ҷ��㣬�����������
			delete pNode;
			root = leafhead = NULL;
			nodecount = level = 0;
		}
		else {//����С������������Ϊ���ڵ�
			root = pNode->childs[0];
			root->parent = NULL;
			delete pNode;
			nodecount--;
			level--;
		}
	}
	else if(pNode->isLeaf) {//Ҷ���
		Node parent = pNode->parent;
		int index = 0;
		parent->search(pNode->keys[0], index);//���ҵ�ǰ�����Сkey�ڸ������λ��
		Node brother = NULL;
		if(index != 0) {//��Сkey��������ߣ�˵����ǰ��������ֵ�
			brother = parent->childs[index];
			if(brother->keycount > half_of_degree) {//���ֵ�key���㹻�������ֵ��а�һ��key�����չ�key��
				typename vector<ElementType>::iterator v1 = pNode->keys.begin();
				vector<offsetNumber>::iterator v2 = pNode->offset.begin();
				pNode->keys.insert(v1, brother->keys[brother->keycount - 1]);
				pNode->offset.insert(v2, brother->offset[brother->keycount - 1]);
				brother->deleteKey(brother->keycount - 1);
				pNode->keycount++;
				parent->keys[index] = pNode->keys[0];
				return true;
			}
			else {//���ֵ�key������������ǰ����key�������ֵܣ���ɾ����ǰ���
				for(int i = 0; i < pNode->keycount; i++) {
					brother->keys[i + brother->keycount] = pNode->keys[i];
					brother->offset[i + brother->keycount] = pNode->offset[i];
				}
				brother->keycount += pNode->keycount;
				brother->nextLeafNode = pNode->nextLeafNode;
				parent->deleteKey(index);
				delete pNode;
				nodecount--;
				return AfterDelete(parent);//ɾ����ǰ����ҲҪ���������
			}
		}
		else {//ȥ�ұ߽�����ҿ��õ�key
			//��ǰ�����Сkey�ڸ���������ߣ���˲�ȷ���Ǹ������ĸ����ӣ���Ҫ�����
			if(pNode == parent->childs[0]) {
				brother = parent->childs[1];
			}
			else {
				brother = parent->childs[index + 2];
			}
			if(brother->keycount > half_of_degree) {//���ֵ�key���㹻�������ֵ��а�һ��key�����չ�key��
				pNode->keys[pNode->keycount] = brother->keys[0];
				pNode->offset[pNode->keycount] = brother->offset[0];
				pNode->keycount++;
				brother->deleteKey(0);
				if(parent->childs[0] == pNode)
					parent->keys[0] = brother->keys[0];
				else
					parent->keys[1] = brother->keys[0];
				return true;
			}
			else {//���ֵ�key�������������ֵܵ�key���뵱ǰ���
				for(int i = 0; i < brother->keycount; i++) {
					pNode->keys[i + pNode->keycount] = brother->keys[i];
					pNode->offset[i + pNode->keycount] = brother->offset[i];
				}
				pNode->keycount += brother->keycount;
				pNode->nextLeafNode = brother->nextLeafNode;
				if(parent->childs[0] == pNode)
					parent->deleteKey(0);
				else
					parent->deleteKey(1);
				delete brother;
				nodecount--;
				return AfterDelete(parent);//ɾ�����ֵܺ󣬸����ҲҪ����
			}
		}
	}
	else {//����Ҷ��㣬����Ҷ���Ĵ���������Ҫ����offset������Ҫע��keyת��ʱ�ӽ��Ĵ���
		Node parent = pNode->parent;
		int index = 0;
		parent->search(pNode->keys[0], index);
		Node brother = NULL;
		if(index != 0) {
			brother = parent->childs[index];
			if(brother->keycount > half_of_degree - 1) {
				typename vector<ElementType>::iterator v1 = pNode->keys.begin();
				typename vector<Node>::iterator v2 = pNode->childs.begin();
				pNode->keys.insert(v1, FindMin(pNode));
				pNode->childs.insert(v2, brother->childs[brother->keycount]);
				pNode->keycount++;
				parent->keys[index] = FindMin(pNode);
				if(brother->childs[brother->keycount])
					brother->childs[brother->keycount]->parent = pNode;		
				brother->deleteKey(brother->keycount - 1);
				return true;
			}
			else {
				brother->keys[brother->keycount] = FindMin(pNode);
				brother->keycount++;
				for(int i = 0; i < pNode->keycount; i++) {
					brother->keys[i + brother->keycount] = pNode->keys[i];
					brother->childs[i + brother->keycount] = pNode->childs[i];
					brother->childs[i + brother->keycount]->parent = brother;	
				}
				brother->childs[brother->keycount + pNode->keycount] = pNode->childs[pNode->keycount];
				brother->childs[brother->keycount + pNode->keycount]->parent = brother;	
				brother->keycount += pNode->keycount;
				brother->nextLeafNode = pNode->nextLeafNode;
				parent->deleteKey(index);
				delete pNode;
				nodecount--;
				return AfterDelete(parent);
			}
		}
		else {	
			if(parent->childs[0] == pNode)
				brother = parent->childs[1];
			else
				brother = parent->childs[2];
			ElementType tempkeys = pNode->childs[0]->keys[0];					
			if(brother->keycount > half_of_degree - 1) {	
				pNode->childs[pNode->keycount + 1] = brother->childs[0];
				pNode->keys[pNode->keycount] = FindMin(brother);
				pNode->childs[pNode->keycount + 1]->parent = pNode;
				pNode->keycount++;
				brother->childs[0] = brother->childs[1];	
				brother->deleteKey(0);
				if(parent->childs[0] == pNode)
					parent->keys[0] = FindMin(brother);
				else
					parent->keys[1] = FindMin(brother);
				return true;
			}
			else {
				pNode->keys[pNode->keycount] = FindMin(brother);
				pNode->keycount++;
				for(int i = 0; i < brother->keycount; i++) {
					pNode->keys[i + pNode->keycount] = brother->keys[i];
					pNode->childs[i + pNode->keycount] = brother->childs[i];
					pNode->childs[i + pNode->keycount]->parent = pNode;	
				}
				pNode->childs[pNode->keycount + brother->keycount] = brother->childs[brother->keycount];
				pNode->childs[pNode->keycount + brother->keycount]->parent = pNode;	
				pNode->keycount += brother->keycount;
				pNode->nextLeafNode = brother->nextLeafNode;
				parent->deleteKey(index);
				delete brother;
				nodecount--;
				return AfterDelete(parent);
			}
		}
	}
}

template <class ElementType>
void BPlusTree<ElementType>::FindLeaf(Node pNode, ElementType key, NodeInfo<ElementType> &info) {//��Ҷ����в���key��������Ϣ������info��
	int index = 0;
	if(pNode->search(key, index)) {//pNode��Ӧ���������ҵ�key
		if(pNode->isLeaf) {//Ҷ���
			info.pNode = pNode;
			info.exist = true;
			info.index = index;
		}
		else {//��Ҷ��㣬����index�ҵ���ӦҶ���
			pNode = pNode->childs[index + 1];
			while(!pNode->isLeaf) {
				pNode = pNode->childs[0];
			}

			info.pNode = pNode;
			info.exist = true;
			info.index = 0;//Ӧ��Ϊ���Ҷ���ĵ�һ��key
		}
	}
	else {//�Ҳ���key
		if(pNode->isLeaf) {
			info.pNode = pNode;
			info.exist = false;
			info.index = index;
		}
		else {
			FindLeaf(pNode->childs[index], key, info);
		}
	}
}
#endif
