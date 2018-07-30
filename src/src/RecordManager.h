#ifndef RECORDMANAGER_H
#define RECORDMANAGER_H

#include "BufferManager.h"
#include "Condition.h"
#include "Attribute.h"
#include <vector>
#include <string>

class API;

class RecordManager{
private:
	BufferManager bm;
	API *api;
public:
	RecordManager(API *api):api(api){}
	~RecordManager(){}

	bool Createtable(string tablename); //�������ļ�
	bool DropTable(string tablename);   //ɾ�����ļ�

	int InsertRecord(string tablename,char *record,int recordsize);//����һ����¼���ɹ����ز���λ�ÿ�ƫ���������ɹ�����-1
	int FindRecord(string tablename,vector<Condition> *condition);//���Ҿ����ض�ֵ�ļ�¼���ҵ�����1��û�з���0
	//�������������ļ�¼�����������̨����û��where����conditionlistΪnull,offsetΪ��ƫ��������δʹ���������ɲ�����,���ز��ҵ��ļ�¼����
	int SelectRecord(string tablename,vector<string> *attrbutename,vector<Condition> *conditionlist,int offset=-1);
	//ɾ�����������ļ�¼������ɾ����¼����
	int DeleteRecord(string tablename,vector<Condition> *conditionlist,int offset=-1);
	//��������(ֻ��Ϊ�˰Ѳ�������indexmanager)
	int IndexCreateFromRecord(string tablename,string indexname);
private:
	//�����¼����ɾ����ǵļ�¼��
	bool InsertRecordInBlock(BlockNode *block,char *record,int recordsize);
	int SelectRecordInBlock(BlockNode *block,int recordsize,vector<Attribute> *allattribute,vector<string> *attrbutename,vector<Condition> *conditionlist);
	int FindRecordInBlock(BlockNode *block,int recordsize,vector<Attribute> *allattribute,vector<Condition> *condition);
	int DeleteRecordInBlock(BlockNode *btmp,int recordsize,vector<Attribute> *allattribute,vector<Condition> *conditionlist);
	int IndexCreateFromRecord_Block(BlockNode *block,int recordsize,vector<Attribute> *allattribute, string indexname);
	bool ConditionFit(char *recordbegin,int recordsize,vector<Attribute> *allattribute,vector<Condition> *conditionlist);//ɸѡһ����¼�Ƿ���������
	bool JudgeContent(char *attributebegin,int type,Condition condition);
	void PrintRecord(char *recordbegin,int recordsize,vector<Attribute> *allattribute,vector<string> *attributename);//����attrbutename��ӡ�˼�¼
};

#endif
