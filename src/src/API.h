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

	bool CreateTable(string tablename,vector<Attribute> *tableattribute,string primarykeyname);   //建表，传入属性及主键名,成功返回true
	bool DropTable(string tablename);   //删除表

	//创建索引(传入索引名、表名和列名)|create index 索引名 on 表名 ( 列名 );
	void CreateIndex(string indexname,string tablename,string attributename);    
	void DropIndex(string indexname);    //删除索引

	//插入一条记录,传入表名和属性值
	void InsertRecord(string tablename,vector<string> *attributevalue);   
	//查找记录,若查找属性为*，传入attributename为NULL,若无where选择条件，可不传入conditionlist参数;对于select * from xx,可只传入xx表名
	void SelectRecord(string tablename,vector<string> *attributename=NULL,vector<Condition> *conditionlist=NULL);  
	//删除记录,若无where选择条件，无需传入conditionlist参数
	void DeleteRecord(string tablename,vector<Condition> *conditionlist=NULL);

	//插入索引(每条记录,传入记录起始位置，记录大小，表所有属性，记录所在块偏移量)
	void InsertIndexInRecord(char *recordbegin,int recordsize,vector<Attribute> *allattribute,int blockoffset);
	//插入索引(针对每个属性，传入这个属性的索引名，属性指针，属性类型，插入的单条记录所在块偏移量)
	void InsertIndex(string indexname,char* attributebegin,int type,int blockoffset);
	//删除索引
	void deleteIndex(string indexFileName,char* attributebegin,int type);
	void getAllIndex(vector<IndexInfo>* indexlist);

	//获取table属性保存在attribute中并返回一条记录的大小（不包括标记位）
	int GetAttribute(string tablename,vector<Attribute> *attribute);
	//获取实际类型大小，type对应Attribute中所给数值
	int TypeSizeGet(int type);
private:
	//获取tableAttribute中属性名为attrname的类型
	int getAttrType(vector<Attribute>* tableAttribute, string attrName);
	//将属性值从vector<string>类型转化为char*
	char* recordStringGet(string tableName, vector<string>* recordContent);
};

#endif
