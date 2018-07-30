#ifndef API_H
#define API_H

#include "CatalogManager.h"
#include "RecordManager.h"
#include "IndexManager.h"
#include "Attribute.h"
#include "Condition.h"
#include <vector>
#include <string>

class API{
public:
	CatalogManager *cm;
	RecordManager *rm;
	IndexManager *im;
public:
	API();
	~API();

	bool CreateTable(string tablename,vector<Attribute> *tableattribute,string primarykeyname);   //�����������Լ�������,�ɹ�����true
	bool DropTable(string tablename);   //ɾ����

	//��������(����������������������)|create index ������ on ���� ( ���� );
	void CreateIndex(string indexname,string tablename,string attributename);    
	void DropIndex(string indexname);    //ɾ������

	//����һ����¼,�������������ֵ
	void InsertRecord(string tablename,vector<string> *attributevalue);   
	//���Ҽ�¼,����������Ϊ*������attributenameΪNULL,����whereѡ���������ɲ�����conditionlist����;����select * from xx,��ֻ����xx����
	void SelectRecord(string tablename,vector<string> *attributename=NULL,vector<Condition> *conditionlist=NULL);  
	//ɾ����¼,����whereѡ�����������贫��conditionlist����
	void DeleteRecord(string tablename,vector<Condition> *conditionlist=NULL);

	//��������(ÿ����¼,�����¼��ʼλ�ã���¼��С�����������ԣ���¼���ڿ�ƫ����)
	void InsertIndexInRecord(char *recordbegin,int recordsize,vector<Attribute> *allattribute,int blockoffset);
	//��������(���ÿ�����ԣ�����������Ե�������������ָ�룬�������ͣ�����ĵ�����¼���ڿ�ƫ����)
	void InsertIndex(string indexname,char* attributebegin,int type,int blockoffset);
	//ɾ������
	void deleteIndex(string indexFileName,char* attributebegin,int type);
	void getAllIndex(vector<IndexInfo>* indexlist);

	//��ȡtable���Ա�����attribute�в�����һ����¼�Ĵ�С�����������λ��
	int GetAttribute(string tablename,vector<Attribute> *attribute);
	//��ȡʵ�����ʹ�С��type��ӦAttribute��������ֵ
	int TypeSizeGet(int type);
private:
	//��ȡtableAttribute��������Ϊattrname������
	int getAttrType(vector<Attribute>* tableAttribute, string attrName);
	//������ֵ��vector<string>����ת��Ϊchar*
	char* recordStringGet(string tableName, vector<string>* recordContent);
};

#endif
