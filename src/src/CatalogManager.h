#ifndef MiniSQL_CatalogManager_H
#define MiniSQL_CatalogManager_H

#define UNKOWN_FILE 8
#define TABLE_FILE 9
#define INDEX_FILE 10

#include <string>
#include <vector>
#include "Attribute.h"
#include "BufferManager.h"
#include "IndexManager.h"
#include <fstream>
using namespace std;
class CatalogManager{
private:
	int setIndexOnTable(string tableName,string attributeName,string indexName);
        //�����������ڱ���������Ե�indexֵ���������� ĳ��attribute�����е�indexֵ�����attribute�Ľṹ���壩
	int revokeIndexOnTable(string tableName,string attributeName, string indexName);
        //�ͷ��������ڱ���������Ե�indexֵ
public:
	BufferManager bm;
	CatalogManager();
	~CatalogManager();
	int addTable(string tableName,vector<Attribute>* tableAttribute,string primaryKeyName);
        //����һ���µı���Ϣ��������Ϣ

	int addIndex(string indexName,string tableName,string attributeName,int type);
        //����һ���µ�������Ϣ�ļ�����index.catalog�ļ�û�н�������������Ϣ

	int findTable(string tableName);
        //�жϸñ���Ϣ�ļ��Ƿ���� ����ֵ����

	int findIndex(string indexName);
        //�жϸ�������Ϣ�ļ��Ƿ���� ����ֵ����

	int dropTable(string tableName);
        //ɾ��һ������Ϣ�ļ�   

	int dropIndex(string indexName);
        //ɾ��һ��������Ϣ

	int removeRecord(string tableName,int removeNum);
        //�м�¼ɾ��ʱ���޸ı���Ϣ�ļ��еļ�¼����

	int insertRecord(string tableName,int insertNum);
        //�м�¼����ʱ���޸ı���Ϣ�ļ��еļ�¼����

	int getRecordNum(string tableName);
        //��ȡ���еļ�¼����

	int getIndexType(string indexName);
        //��ȡ��������������Ϣ

	int getAttribute(string tableName,vector<Attribute>* tableAttribute);
        //��ȡ��������������Ϣ

	void getAllIndexInTable(string tableName, vector<string>& indexName);
        //��ȡһ�ű�������������Ϣ

	int calculateSize(string tableName);
        //���㷵�ر���һ����¼ռ�õ��ֽ���

	int calculateSize(int type);
        //���㷵�ز�ͬ����������ռ�ֽ���

	void getAllIndexInfo(vector<IndexInfo>* indexList);
        //��ȡ����������Ϣ
};


#endif
