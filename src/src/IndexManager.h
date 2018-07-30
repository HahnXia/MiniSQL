#ifndef __Minisql__IndexManager__
#define __Minisql__IndexManager__

#include <map>
#include <string>
#include <sstream>
#include <fstream>
//#include "API.h"
#include "Attribute.h"
#include "BPlusTree.h"

using namespace std;
class API;



class IndexInfo {
public:
	IndexInfo(){}
	IndexInfo(string name, string table, string attr, int type):indexName(name), tableName(table), Attribute(attr), type(type) {
	}
	string indexName;//index名
	string tableName;//表名
	string Attribute;//属性名
	int type;//属性类型（三种之一）
};

/*建好的索引是以.index文件的形式存在磁盘上的
数据库运行时，IndexManager类创建实例，将磁盘上的.index文件读入，并建立相应的B+树，再通过map容器映射起来
由于B+树和.index文件已建立映射，因此本接口的调用者不需要知道B+树的名称和实现，只需知道.index文件的名称和索引类型，即可对索引进行操作*/
class IndexManager {
private:
	BufferManager buffer;//index所使用的buffer

	//三种类型的map容器，将每个文件名（string）和一种B+树（BPlusTree）对应起来，用于保存和管理index
	typedef map<string, BPlusTree<int> *> intMap;
	typedef map<string, BPlusTree<string> *> stringMap;
	typedef map<string, BPlusTree<float> *> floatMap;

	API * api;

	//容器的创建
	/*static */intMap indexIntMap;
	/*static */stringMap indexStringMap;
	/*static */floatMap indexFloatMap;

	//三种类型的临时变量，用来保存三种类型的搜索码
	int intTmp;
	float floatTmp;
	string stringTmp;

	int getDegree(int type);//得到一个block中所能保存的index数量
	int getKeySize(int type);//得到当前搜索码的大小
	void setKey(int type, string key);//设置当前的搜索码

public:
	IndexManager(API* ap);//构造函数，参数为API，会调用API来获取.index文件列表并导入index信息
	~IndexManager();//析构函数，销毁对象时会将所有index信息写回磁盘中

	void createIndex(string filePath, int type);//创建一个指定名称（filePath）和类型（type）的.index文件
												//例：createIndex("salary_index.index",TYPE_INT);
												//会创建一个名为“salary_index”的.index文件，类型为TYPE_INT

	void dropIndex(string filePath, int type);//删除一个指定名称（filePath）和类型（type）的.index文件，用法同上

	offsetNumber searchIndex(string filePath, string key, int type);//获取指定名称（filePath）和类型（type）的.index文件中搜索码为key的index位置
																	//.index文件中index的位置是用偏移量（一个整数）来表示的，知道了偏移量就能很容易地定位到相应的index
																	//例：searchIndex("salary_index.index","10000",TYPE_INT);
																	//会搜索名为“salary_index”且类型为TYPE_INT的.index文件中搜索码为“10000”的index所在位置

	void insertIndex(string filePath, string key, offsetNumber blockOffset, int type);//在一个指定名称（filePath）和类型（type）的.index文件中
																					  //位于blockOffset的位置插入一个搜索码为key的index
																					  //例：insertIndex("salary_index.index","20000",88,TYPE_INT);
																					  //会在名为“salary_index”且类型为TYPE_INT的.index文件中88的位置插入一个搜索码为“20000”的index

	void deleteIndex(string filePath, string key, int type);//在一个指定名称（filePath）和类型（type）的.index文件中删除一个搜索码为key的index
															//例：deleteIndex("salary_index.index","20000",TYPE_INT);
															//会在名为“salary_index”且类型为TYPE_INT的.index文件中删除搜索码为“20000”的index

	void readIndex(string filePath, int type);//从一个指定名称（filePath）和类型（type）的.index文件中读取index信息，建立对应B+树，并在相应的map容器中建立二者的映射
};



#endif
