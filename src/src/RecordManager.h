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

	bool Createtable(string tablename); //建立表文件
	bool DropTable(string tablename);   //删除表文件

	int InsertRecord(string tablename,char *record,int recordsize);//插入一条记录，成功返回插入位置块偏移量，不成功返回-1
	int FindRecord(string tablename,vector<Condition> *condition);//查找具有特定值的记录，找到返回1，没有返回0
	//查找满足条件的记录并输出到控制台，若没有where条件conditionlist为null,offset为块偏移量，若未使用索引，可不输入,返回查找到的记录数量
	int SelectRecord(string tablename,vector<string> *attrbutename,vector<Condition> *conditionlist,int offset=-1);
	//删除满足条件的记录，返回删除记录数量
	int DeleteRecord(string tablename,vector<Condition> *conditionlist,int offset=-1);
	//创建索引(只是为了把参数传给indexmanager)
	int IndexCreateFromRecord(string tablename,string indexname);
private:
	//插入记录到有删除标记的记录中
	bool InsertRecordInBlock(BlockNode *block,char *record,int recordsize);
	int SelectRecordInBlock(BlockNode *block,int recordsize,vector<Attribute> *allattribute,vector<string> *attrbutename,vector<Condition> *conditionlist);
	int FindRecordInBlock(BlockNode *block,int recordsize,vector<Attribute> *allattribute,vector<Condition> *condition);
	int DeleteRecordInBlock(BlockNode *btmp,int recordsize,vector<Attribute> *allattribute,vector<Condition> *conditionlist);
	int IndexCreateFromRecord_Block(BlockNode *block,int recordsize,vector<Attribute> *allattribute, string indexname);
	bool ConditionFit(char *recordbegin,int recordsize,vector<Attribute> *allattribute,vector<Condition> *conditionlist);//筛选一条记录是否满足条件
	bool JudgeContent(char *attributebegin,int type,Condition condition);
	void PrintRecord(char *recordbegin,int recordsize,vector<Attribute> *allattribute,vector<string> *attributename);//根据attrbutename打印此记录
};

#endif
