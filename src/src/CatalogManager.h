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
        //设置索引所在表的作用属性的index值（就是设置 某个attribute对象中的index值，详见attribute的结构定义）
	int revokeIndexOnTable(string tableName,string attributeName, string indexName);
        //释放索引所在表的作用属性的index值
public:
	BufferManager bm;
	CatalogManager();
	~CatalogManager();
	int addTable(string tableName,vector<Attribute>* tableAttribute,string primaryKeyName);
        //创建一个新的表信息并填入信息

	int addIndex(string indexName,string tableName,string attributeName,int type);
        //创建一个新的索引信息文件（若index.catalog文件没有建立），填入信息

	int findTable(string tableName);
        //判断该表信息文件是否存在 返回值待定

	int findIndex(string indexName);
        //判断该索引信息文件是否存在 返回值待定

	int dropTable(string tableName);
        //删除一个表信息文件   

	int dropIndex(string indexName);
        //删除一个索引信息

	int removeRecord(string tableName,int removeNum);
        //有记录删除时，修改表信息文件中的记录总数

	int insertRecord(string tableName,int insertNum);
        //有记录插入时，修改表信息文件中的记录总数

	int getRecordNum(string tableName);
        //获取表中的记录总数

	int getIndexType(string indexName);
        //获取索引数据类型信息

	int getAttribute(string tableName,vector<Attribute>* tableAttribute);
        //获取表中所有属性信息

	void getAllIndexInTable(string tableName, vector<string>& indexName);
        //获取一张表中所有索引信息

	int calculateSize(string tableName);
        //计算返回表中一条记录占用的字节数

	int calculateSize(int type);
        //计算返回不同数据类型所占字节数

	void getAllIndexInfo(vector<IndexInfo>* indexList);
        //获取所有索引信息
};


#endif
