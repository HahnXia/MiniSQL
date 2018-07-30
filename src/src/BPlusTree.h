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
struct NodeInfo {//结点信息类，用于记录结点信息，比结点类所占空间小
	TreeNode<ElementType>* pNode;//指向对应结点的指针
	int index;//需要的key在该节点中的位置
	bool exist;//该节点中是否存在需要的key
};

template <class ElementType>
class BPlusTree {//B+树类
private:
	BufferManager buffer;//缓冲区
	typedef TreeNode<ElementType>* Node;//树节点指针定义为Node，方便书写

	string filename;//对应的索引文件名
	Node root;//树的根节点
	Node leafhead;//第一个叶结点
	FileNode* file;//树的文件结点
	int degree;//树的度数，也是一块中能保存key的个数
	int keycount;//树中key的个数
	int level;//数的层数
	int nodecount;//数的结点数
	int keysize;//key的大小

	ElementType FindMin(Node pNode);//返回pNode所对应子树中最小的key
	bool AfterInsert(Node pNode);//在pNode上执行完插入后的调整
	bool AfterDelete(Node pNode);//在pNode上执行完删除后的调整
	void FindLeaf(Node pNode, ElementType key, NodeInfo<ElementType> &info);//在pNode对应子树中查找key所在叶结点，并将其信息保存在info中

public:
	BPlusTree(string filename, int keysize, int degree);
	~BPlusTree();
	void ReadTree();//从文件中读入key信息并生成对应的树
	void DropTree(Node pNode);//删除pNode所对应子树
	void WriteBack();//将树中的key信息写回缓冲区

	offsetNumber Search(ElementType key);//查找一个key，并返回其偏移量
	bool Insert(ElementType key, offsetNumber offset);//在offset位置插入一个key
	bool Delete(ElementType key);//删除一个key

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
void BPlusTree<ElementType>::DropTree(Node pnode) {//删除整棵树
	if(!pnode) return;
	if(!pnode->isLeaf) {
		for(int i = 0; i <= pnode->keycount; i++) {
			DropTree(pnode->childs[i]);//递归删除子树
			pnode->childs[i] = NULL;
		}
	}
	delete pnode;
	return;
}

template <class ElementType>
void BPlusTree<ElementType>::ReadTree() {
	//根据对应文件获取块的头
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
			while(offsetbegin - indexbegin < buffer.getUsingSize(tempblock)) {//循环读入key和offset，并插入树中
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
void BPlusTree<ElementType>::WriteBack() {//将key和offset信息写回缓冲区
	BlockNode * tempblock = buffer.getBlockHead(file);
	Node tempnode = leafhead;
	int offsetsize = sizeof(offsetNumber);
	while(tempnode != NULL) {//从叶结点中读取key和offset信息，并写回块中
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
	//写回剩下的空块
	while(true) {
		if(tempblock->isBottom) break;
		buffer.setUsingSize(tempblock, 0);
		buffer.set_dirty(tempblock);
		tempblock = buffer.getNextBlock(file, tempblock);
	}
}

template <class ElementType>
offsetNumber BPlusTree<ElementType>::Search(ElementType key) {//在树中查找key并返回偏移量
	if(!root) return -1;
	NodeInfo<ElementType> targetnode;
	FindLeaf(root, key, targetnode);//在叶结点中查找key
	if (!targetnode.exist) {//没找到
		return -1;
	}else {
		return targetnode.pNode->offset[targetnode.index];
	}
}

template <class ElementType>
bool BPlusTree<ElementType>::Insert(ElementType key, offsetNumber offset) {//根据偏移量插入key
	NodeInfo<ElementType> newnode;
	if(!root) {//树为空，还没有根节点
		root = new TreeNode<ElementType>(degree, true);
		leafhead = root;
	}
	FindLeaf(root, key, newnode);
	if(newnode.exist) {//树中已存在该key
		cout << "Error:Cannot inserrt key to index: the duplicated key!" << endl;
		return false;
	}
	else {
		newnode.pNode->insertKey(key, offset);
		if(newnode.pNode->keycount == degree)//插入后结点的key数量等于度数（即大于度数-1），需要调整
			AfterInsert(newnode.pNode);
		keycount++;
		return true;
	}
}

template <class ElementType>
bool BPlusTree<ElementType>::Delete(ElementType key) {//从树中删除key
	NodeInfo<ElementType> newnode;
	if(!root) {//树为空
		cout << "Error:Cannot delete key from index: the tree not exist!" << endl;
		return false;
	}
	FindLeaf(root, key, newnode);
	if(!newnode.exist) {//树中没有key
		cout << "Error:Cannot delete key from index: the key not exist!" << endl;
		return false;
	}
	else {
		if(newnode.pNode->isRoot()) {//根节点，直接删除
			newnode.pNode->deleteKey(newnode.index);
			keycount--;
		}
		else {
			if(newnode.index == 0 && leafhead != newnode.pNode) {//key在分支中，且在当前结点左边
				int index = 0;

				Node parent = newnode.pNode->parent;//在更高层里查找key
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
			else {//key在叶结点中
				newnode.pNode->deleteKey(newnode.index);
				keycount--;
			}
		}
		if((newnode.pNode->keycount >= (degree - 1) / 2 - 1)||this->nodecount==1)//删除后不需调整
			return true;
		else//否则调整树
			return AfterDelete(newnode.pNode);
	}
}

template <class ElementType>
bool BPlusTree<ElementType>::AfterInsert(Node pNode) {//插入key后的调整
	ElementType key;
	offsetNumber offset;	
	Node newnode = pNode->devide(key, offset);//当前结点分裂
	nodecount++;
	if(pNode->isRoot()) {//原结点为根结点，分裂后应创建新的根结点
		Node root = new TreeNode<ElementType>(degree, false);
		if(root == NULL) {//创建新结点失败
			cout << "Error: can not allocate memory for the new root after insert" << endl;
			return false;
		}
		else {//结点信息调整
			this->root = root;
			root->insertKey(key, -1);
			root->childs[0] = pNode;
			root->childs[1] = newnode;
			pNode->parent = root;
			newnode->parent = root;
			level++;//树的层数增加
			nodecount++;
			return true;
		}
	}
	else {//不是根结点
		Node parent = pNode->parent;
		int index = parent->insertKey(key);//父结点中插入新结点最小的key
		parent->childs[index + 1] = newnode;//父结点增加一个子结点
		newnode->parent = parent;
		if(parent->keycount == degree) {//父结点的key数也超过限制，需要调整
			return AfterInsert(parent);
		}
		return true;
	}
}

template <class ElementType>
ElementType BPlusTree<ElementType>::FindMin(Node pNode) {//查找最小key
	if(pNode->childs[0]) {//还有更小子树，则递归查找
		return FindMin(pNode->childs[0]);
	}
	else {//否则返回当前结点最小的key
		return pNode->keys[0];
	}
}

template <class ElementType>
bool BPlusTree<ElementType>::AfterDelete(Node pNode) {//删除key后调整
	int half_of_degree = (degree - 1) / 2;//度数的一半，用于衡量结点的key数是否足够
	if(pNode->isRoot()) {//根节点
		if(pNode->keycount == 0) {
			return false;
		}
		else if(pNode->isLeaf) {//如果也是叶结点，则清空整颗树
			delete pNode;
			root = leafhead = NULL;
			nodecount = level = 0;
		}
		else {//将最小子树提上来作为根节点
			root = pNode->childs[0];
			root->parent = NULL;
			delete pNode;
			nodecount--;
			level--;
		}
	}
	else if(pNode->isLeaf) {//叶结点
		Node parent = pNode->parent;
		int index = 0;
		parent->search(pNode->keys[0], index);//查找当前结点最小key在父结点中位置
		Node brother = NULL;
		if(index != 0) {//最小key不在最左边，说明当前结点有左兄弟
			brother = parent->childs[index];
			if(brother->keycount > half_of_degree) {//左兄弟key数足够，从左兄弟中搬一个key过来凑够key数
				typename vector<ElementType>::iterator v1 = pNode->keys.begin();
				vector<offsetNumber>::iterator v2 = pNode->offset.begin();
				pNode->keys.insert(v1, brother->keys[brother->keycount - 1]);
				pNode->offset.insert(v2, brother->offset[brother->keycount - 1]);
				brother->deleteKey(brother->keycount - 1);
				pNode->keycount++;
				parent->keys[index] = pNode->keys[0];
				return true;
			}
			else {//左兄弟key数不够，将当前结点的key并入左兄弟，并删除当前结点
				for(int i = 0; i < pNode->keycount; i++) {
					brother->keys[i + brother->keycount] = pNode->keys[i];
					brother->offset[i + brother->keycount] = pNode->offset[i];
				}
				brother->keycount += pNode->keycount;
				brother->nextLeafNode = pNode->nextLeafNode;
				parent->deleteKey(index);
				delete pNode;
				nodecount--;
				return AfterDelete(parent);//删除当前结点后也要调整父结点
			}
		}
		else {//去右边结点中找可用的key
			//当前结点最小key在父结点的最左边，因此不确定是父结点的哪个孩子，需要分情况
			if(pNode == parent->childs[0]) {
				brother = parent->childs[1];
			}
			else {
				brother = parent->childs[index + 2];
			}
			if(brother->keycount > half_of_degree) {//右兄弟key数足够，从右兄弟中搬一个key过来凑够key数
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
			else {//右兄弟key数不够，将右兄弟的key并入当前结点
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
				return AfterDelete(parent);//删除右兄弟后，父结点也要调整
			}
		}
	}
	else {//不是叶结点，类似叶结点的处理，但不需要处理offset，而是要注意key转移时子结点的处理
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
void BPlusTree<ElementType>::FindLeaf(Node pNode, ElementType key, NodeInfo<ElementType> &info) {//在叶结点中查找key，并将信息保存在info中
	int index = 0;
	if(pNode->search(key, index)) {//pNode对应子树中能找到key
		if(pNode->isLeaf) {//叶结点
			info.pNode = pNode;
			info.exist = true;
			info.index = index;
		}
		else {//非叶结点，根据index找到对应叶结点
			pNode = pNode->childs[index + 1];
			while(!pNode->isLeaf) {
				pNode = pNode->childs[0];
			}

			info.pNode = pNode;
			info.exist = true;
			info.index = 0;//应该为这个叶结点的第一个key
		}
	}
	else {//找不到key
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
