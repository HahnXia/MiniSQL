#include "RecordManager.h"
#include "API.h"
#include <iostream>
#include <fstream>
#include <stdio.h> //remove()����

using namespace std;

//������ű���ļ�
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
		if(InsertRecordInBlock(btmp,record,recordsize)){//�����ɾ����¼���Ա����
			return btmp->offset;
		}
		if(recordsize+1<=bm.getBlockSize()-bm.getUsingSize(btmp)){//�㹻����һ����¼
			char *addr;
			addr=bm.getContent(btmp)+bm.getUsingSize(btmp);//���㿪ʼ(�����һ��size_t)+�Ѿ�ʹ�õĿռ䳤��
			*addr=0;  //���λ��ʾ�Ƿ�ɾ��
			addr+=1;
			memcpy(addr,record,recordsize); //������¼����
			bm.setUsingSize(btmp,bm.getUsingSize(btmp)+recordsize+1);
			bm.set_dirty(btmp);//���Ϊ��ڵ㣬Ҫ����д���ļ�
			return btmp->offset;
		}
		else{
			btmp=bm.getNextBlock(ftmp,btmp);
		}
	}
	return -1;
}

//�Ƿ��б��Ϊɾ���ļ�¼���Ա��²����������
bool RecordManager::InsertRecordInBlock(BlockNode *block,char *record,int recordsize){
	if(block==NULL){
		return false;
	}
	char *recordbegin=bm.getContent(block);
	char *blockend=recordbegin+bm.getUsingSize(block);
	while(recordbegin<blockend){
		if(*recordbegin==1){   //��������Ϊ��ɾ����������¼
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

//�������������ļ�¼�����ظ�����������-1
int RecordManager::SelectRecord(string tablename,vector<string> *attrbutename,vector<Condition> *conditionlist,int offset){
	string tablefilename=tablename+".table";
	FileNode *ftmp=bm.getFile(tablefilename.c_str());
	BlockNode *btmp=bm.getBlockHead(ftmp);

	//һ����¼��С
	int recordsize;
	//��ȡȫ������
	vector<Attribute> allattribute;
	recordsize=api->GetAttribute(tablename,&allattribute);

	if(offset>=0){
		btmp=bm.getBlockByOffset(ftmp,offset);
		return SelectRecordInBlock(btmp,recordsize,&allattribute,attrbutename,conditionlist);
	}

	int count=0;//��¼��
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

//�ҵ�����1��û�ҵ�����0,������-1
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

//�ڿ�ڵ����ҵ�����������1,���򷵻�0
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

//ɾ�����������ļ�¼��������-1
int RecordManager::DeleteRecord(string tablename,vector<Condition> *conditionlist,int offset){
	string tablefilename=tablename+".table";
	FileNode *ftmp=bm.getFile(tablefilename.c_str());
	BlockNode *btmp=bm.getBlockHead(ftmp);
	int recordsize;
	int count=0; //��¼����ɾ�������ļ�¼����
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
				*recordbegin = 1;//1�����ѱ�ɾ��
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
			recordbegin+=1;  //�������λ
			for(int i=0;i<allattribute->size();i++){
				int type=(*allattribute)[i].type;
				typesize=api->TypeSizeGet(type);
				if((*allattribute)[i].index==indexname){
					api->InsertIndex(indexname,contentbegin,type,block->offset);
					count++;
				}
				contentbegin+=typesize;//������һ������
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
	char *attributebegin=recordbegin;//��¼ÿ�����Կ�ͷ��λ��

	//����ÿ������
	for(int i=0;i<allattribute->size();i++){
		int type=(*allattribute)[i].type;
		int typesize=api->TypeSizeGet(type);
		string attrbutename=(*allattribute)[i].name;
		memset(content,0,255);
		memcpy(content,attributebegin,typesize);
		for(int j=0;j<conditionlist->size();j++){//��ÿ���������б���
			if((*conditionlist)[j].getAttr()==attrbutename){
				if(!JudgeContent(content,type,(*conditionlist)[j])){//һ����һ�����������㣬�ͷ���false
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
	char *attributebegin=recordbegin;//��¼ÿ�����Կ�ͷ��λ��

	for(int i=0;i<allattribute->size();i++){//������������
		int type=(*allattribute)[i].type;
		int typesize=api->TypeSizeGet(type);
		memset(content,0,255);
		memcpy(content,attributebegin,typesize);
		for(int j=0;j<attributename->size();j++){//����Ҫ��ӡ����������
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
