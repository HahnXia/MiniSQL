#include "RecordManager.h"
#include "API.h"
#include <iostream>
#include <fstream>
#include <stdio.h> //remove()函数

using namespace std;

//创建存放表的文件
bool RecordManager::Createtable(string tablename){
	string tablefilename=tablename+".table";
	ofstream afile(tablefilename.c_str());
	if(!afile){
		cout<<"create table file fails!"<<endl;
		return false;
	}
	else{
		afile.close();
		return true;
	}
}

bool RecordManager::DropTable(string tablename){
	string tablefilename=tablename+".table";
	if(remove(tablefilename.c_str()))
		return false;
	else
		return true;
}

int RecordManager::InsertRecord(string tablename,char *record,int recordsize){
	string tablefilename=tablename+".table";
	FileNode *ftmp=bm.getFile(tablefilename.c_str());
	BlockNode *btmp=bm.getBlockHead(ftmp);
	while(true){
		if(btmp == NULL){
			cout<<"insert "<<tablename<<" record fails"<<endl;
			return -1;
		}
		if(InsertRecordInBlock(btmp,record,recordsize)){//如果有删除记录可以被替代
			return btmp->offset;
		}
		if(recordsize+1<=bm.getBlockSize()-bm.getUsingSize(btmp)){//足够插入一条记录
			char *addr;
			addr=bm.getContent(btmp)+bm.getUsingSize(btmp);//块结点开始(不算第一个size_t)+已经使用的空间长度
			*addr=0;  //标记位表示是否被删除
			addr+=1;
			memcpy(addr,record,recordsize); //拷贝记录内容
			bm.setUsingSize(btmp,bm.getUsingSize(btmp)+recordsize+1);
			bm.set_dirty(btmp);//标记为脏节点，要重新写入文件
			return btmp->offset;
		}
		else{
			btmp=bm.getNextBlock(ftmp,btmp);
		}
	}
	return -1;
}

//是否有标记为删除的记录可以被新插入数据替代
bool RecordManager::InsertRecordInBlock(BlockNode *block,char *record,int recordsize){
	if(block==NULL){
		return false;
	}
	char *recordbegin=bm.getContent(block);
	char *blockend=recordbegin+bm.getUsingSize(block);
	while(recordbegin<blockend){
		if(*recordbegin==1){   //如果被标记为已删除，则插入记录
			*recordbegin=0;
			recordbegin+=1;
			memcpy(recordbegin,record,recordsize);
			bm.set_dirty(block);
			return true;
		}
		recordbegin+=recordsize+1;
	}
	return false;
}

//查找满足条件的记录并返回个数，出错返回-1
int RecordManager::SelectRecord(string tablename,vector<string> *attrbutename,vector<Condition> *conditionlist,int offset){
	string tablefilename=tablename+".table";
	FileNode *ftmp=bm.getFile(tablefilename.c_str());
	BlockNode *btmp=bm.getBlockHead(ftmp);

	//一条记录大小
	int recordsize;
	//获取全部属性
	vector<Attribute> allattribute;
	recordsize=api->GetAttribute(tablename,&allattribute);

	if(offset>=0){
		btmp=bm.getBlockByOffset(ftmp,offset);
		return SelectRecordInBlock(btmp,recordsize,&allattribute,attrbutename,conditionlist);
	}

	int count=0;//记录数
	while(true){
		if(btmp==NULL){
			cout<<"The records don't exsit!"<<endl;
			return -1;
		}
		int cntmp=SelectRecordInBlock(btmp,recordsize,&allattribute,attrbutename,conditionlist);
		count+=cntmp;
		if(btmp->isBottom){
			return count;
		}
		btmp=bm.getNextBlock(ftmp,btmp);
	}
	return -1;
}

int RecordManager::SelectRecordInBlock(BlockNode *block,int recordsize,vector<Attribute> *allattribute,vector<string> *attrbutename,vector<Condition> *conditionlist){
	if(block==NULL){
		return 0;
	}
	int count=0;
	char *recordbegin=bm.getContent(block);
	char *blockend=recordbegin+bm.getUsingSize(block);
	while(recordbegin<blockend){
		if (*recordbegin == 0){
			if (ConditionFit(recordbegin + 1, recordsize, allattribute, conditionlist)){
				PrintRecord(recordbegin + 1, recordsize, allattribute, attrbutename);
				count++;
			}
		}
		recordbegin+=recordsize+1;
	}
	return count;
}

//找到返回1，没找到返回0,出错返回-1
int RecordManager::FindRecord(string tablename,vector<Condition> *condition){
	string tablefilename=tablename+".table";
	FileNode *ftmp=bm.getFile(tablefilename.c_str());
	BlockNode *btmp=bm.getBlockHead(ftmp);
	int recordsize;
	vector<Attribute> allattribute;
	recordsize=api->GetAttribute(tablename,&allattribute);

	while(true){
		if(btmp==NULL){
			return -1;
		}
		if(btmp->isBottom){
			return FindRecordInBlock(btmp,recordsize,&allattribute,condition);
		}
		else{
			if(FindRecordInBlock(btmp,recordsize,&allattribute,condition)){
				return 1;
			}
			btmp=bm.getNextBlock(ftmp,btmp);
		}
	}
	return -1;
}

//在块节点中找到就立即返回1,否则返回0
int RecordManager::FindRecordInBlock(BlockNode *block,int recordsize,vector<Attribute> *allattribute,vector<Condition> *condition){
	if(block==NULL){
		return 0;
	}
	char *recordbegin=bm.getContent(block);
	char *blockend=recordbegin+bm.getUsingSize(block);
	while(recordbegin<blockend){
		if (*recordbegin==0){
			if (ConditionFit(recordbegin + 1, recordsize, allattribute, condition)){
				return 1;
			}
		}
		recordbegin+=recordsize+1;
	}
	return 0;
}

//删除满足条件的记录，出错返回-1
int RecordManager::DeleteRecord(string tablename,vector<Condition> *conditionlist,int offset){
	string tablefilename=tablename+".table";
	FileNode *ftmp=bm.getFile(tablefilename.c_str());
	BlockNode *btmp=bm.getBlockHead(ftmp);
	int recordsize;
	int count=0; //记录满足删除条件的记录个数
	vector<Attribute> allattribute;
	recordsize=api->GetAttribute(tablename,&allattribute);

	if(offset>=0){
		btmp=bm.getBlockByOffset(ftmp,offset);
		count=DeleteRecordInBlock(btmp,recordsize,&allattribute,conditionlist);
		return count;
	}

	while(true){
		if(btmp==NULL){
			return -1;
		}
		count+=DeleteRecordInBlock(btmp,recordsize,&allattribute,conditionlist);
		if(btmp->isBottom){
			return count;
		}
		btmp=bm.getNextBlock(ftmp,btmp);
	}
	return -1;
}

int RecordManager::DeleteRecordInBlock(BlockNode *btmp,int recordsize,vector<Attribute> *allattribute,vector<Condition> *conditionlist){
	if(btmp==NULL){
		return 0;
	}
	char *recordbegin=bm.getContent(btmp);
	char *blockend=recordbegin+bm.getUsingSize(btmp);
	int count=0;
	int data = 0;
	while(recordbegin<blockend){
		char *attributebegin=recordbegin+1;
		if (*recordbegin == 0){
			data = 1;
			if (ConditionFit(recordbegin + 1, recordsize, allattribute, conditionlist)){
				*recordbegin = 1;//1代表已被删除
				for (int i = 0; i < allattribute->size(); i++){
					int type = (*allattribute)[i].type;
					int typesize = api->TypeSizeGet(type);
					if ((*allattribute)[i].index != ""){
						api->deleteIndex((*allattribute)[i].index + ".index", attributebegin, type);//!!!!!!!!!!!!!!!
					}
					attributebegin += typesize;
				}//call API to remove the index
				bm.set_dirty(btmp);
				count++;
			}
		}
		recordbegin+=recordsize+1;
	}
	if (!data){
		cout << "Error: The table is empty!" << endl;
	}
	return count;
}

int RecordManager::IndexCreateFromRecord(string tablename,string indexname){
	string tablefilename=tablename+".table";
	FileNode *ftmp=bm.getFile(tablefilename.c_str());
	BlockNode *btmp=bm.getBlockHead(ftmp);
	int count=0;
	int recordsize;
	vector<Attribute> allattribute;
	recordsize=api->GetAttribute(tablename,&allattribute);

	while(true){
		if(btmp==NULL){
			return -1;
		}
		
		count+=IndexCreateFromRecord_Block(btmp,recordsize,&allattribute,indexname);
		
		if(btmp->isBottom){			
			return count;
		}
		btmp=bm.getNextBlock(ftmp,btmp);
	}
	return -1;
}

int RecordManager::IndexCreateFromRecord_Block(BlockNode *block,int recordsize,vector<Attribute> *allattribute, string indexname){
	if(block==NULL){
		return 0;
	}
	int count=0;
	int typesize;
	char *recordbegin=bm.getContent(block);
	char *contentbegin=recordbegin;
	char *blockend=recordbegin+bm.getUsingSize(block);
	while(recordbegin<blockend){
		if(*recordbegin==0){
			recordbegin+=1;  //跳过标记位
			for(int i=0;i<allattribute->size();i++){
				int type=(*allattribute)[i].type;
				typesize=api->TypeSizeGet(type);
				if((*allattribute)[i].index==indexname){
					api->InsertIndex(indexname,contentbegin,type,block->offset);
					count++;
				}
				contentbegin+=typesize;//跳到下一个属性
			}
			}
		else{
			recordbegin+=1;
		}
		recordbegin+=recordsize;
	}
	return count;
}

bool RecordManager::ConditionFit(char *recordbegin,int recordsize,vector<Attribute> *allattribute,vector<Condition> *conditionlist){
	if(conditionlist==NULL){
		return true;
	}
	char content[255];
	char *attributebegin=recordbegin;//记录每个属性开头的位置

	//遍历每个属性
	for(int i=0;i<allattribute->size();i++){
		int type=(*allattribute)[i].type;
		int typesize=api->TypeSizeGet(type);
		string attrbutename=(*allattribute)[i].name;
		memset(content,0,255);
		memcpy(content,attributebegin,typesize);
		for(int j=0;j<conditionlist->size();j++){//对每个条件进行遍历
			if((*conditionlist)[j].getAttr()==attrbutename){
				if(!JudgeContent(content,type,(*conditionlist)[j])){//一旦有一个条件不满足，就返回false
					return false;
				}
			}
		}
		attributebegin+=typesize;
	}
	return true;
}

bool RecordManager::JudgeContent(char *content,int type,Condition condition){
	if(type==TYPE_INT){
		int attributevalue=*(int *)content;
		return condition.check(attributevalue);
	}
	else if(type ==TYPE_FLOAT){
		float attributevalue=*(float *)content;
		return condition.check(attributevalue);
	}
	else{
		return condition.check(content);
	}
}

void RecordManager::PrintRecord(char *recordbegin,int recordsize,vector<Attribute> *allattribute,vector<string> *attributename){
	char content[255];
	char *attributebegin=recordbegin;//记录每个属性开头的位置

	for(int i=0;i<allattribute->size();i++){//遍历所有属性
		int type=(*allattribute)[i].type;
		int typesize=api->TypeSizeGet(type);
		memset(content,0,255);
		memcpy(content,attributebegin,typesize);
		for(int j=0;j<attributename->size();j++){//遍历要打印出来的属性
			if(((*allattribute)[i]).name==(*attributename)[j]){
				cout.setf(ios::left);
				if(type==TYPE_INT){
					int value=*(int *)content;
					cout<<"|";
					cout.width(8);
					cout<<value;
					cout<<"|";
				}
				else if(type==TYPE_FLOAT){
					float value=*(float *)content;
					cout<<"|";
					cout.width(10);
					cout<<value;
					cout<<"|";
				}
				else{
					cout<<"|";
					cout.width(type+5);
					cout<<content;
					cout<<"|";
				}
			}
		}
		attributebegin+=typesize;
	}
	cout<<endl;
	return;
}
