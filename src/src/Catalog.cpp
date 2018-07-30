#include "CatalogManager.h"
#define STRING_SIZE 100
using namespace std;

/* 重新定义catalog文件中的数据存放结构：
   对于 “表名.catalog”文件   int-insert的记录数  char[100]-主键名     int-属性的数量  
                               char[100]-属性名     int-type            bool-ifUnique       char[100]-属性对应的index

   对于 “index.catalog”文件  bool-isDelete       char[100]-indexName  char[100]tableName  char[100]-Attribute     int-type
*/



CatalogManager::CatalogManager(){

}

CatalogManager::~CatalogManager(){

}

/*done!*/
int CatalogManager::addTable(string tablename, vector<Attribute>* tableAttribute, string primaryKeyName){
	string FileName = tablename + ".catalog";    // 定义表名称
	ofstream f;
	f.open(FileName.c_str(), ios::out);          //以输出方式打开表，如果没有则新建一个，如果有则清空
	if (!f){
		return 0;                                //操作失败，返回0
	}
	else{
		f.close();                               //直接关闭，接下来通过buffer写入数据
	}
	FileNode *fn = bm.getFile(FileName.c_str()); //调用getFile函数获得文件指针
	BlockNode *bn = bm.getBlockHead(fn);		 //调用getBlockHead函数获得该文件的首个块地址
	char *strpos;                                  //用来指向拷贝字符串的位置

	if (bn){
		char *beginAddress = bm.getContent(bn);  //首地址指针
		int *recordNum;                          //指向 总记录数对应位置 指针
		int *attributeNum;                       //指向 记录属性数量位置 指针
		int *type;
		bool *ifUnique;

		recordNum = (int*)beginAddress;
		*recordNum = 0;							 //总记录数初始化为0
		beginAddress += sizeof(int);             //beginAddress后移对应大小

		strpos = beginAddress;
		strcpy_s(strpos, STRING_SIZE, primaryKeyName.c_str());
		beginAddress += STRING_SIZE;

		attributeNum = (int*)beginAddress;        //属性数量
		*attributeNum = tableAttribute->size();
		beginAddress += sizeof(int);

		for (int i = 0; i < tableAttribute->size(); i++){          //for循环依次存入表属性
			strpos = beginAddress;                                
			strcpy_s(strpos, STRING_SIZE, (*tableAttribute)[i].name.c_str());  //存属性名
			beginAddress += STRING_SIZE;

			type = (int*)beginAddress;
			*type = (*tableAttribute)[i].type;
			beginAddress += sizeof(int);

			ifUnique = (bool*)beginAddress;
			*ifUnique = (*tableAttribute)[i].ifUnique;
			beginAddress += sizeof(bool);

			strpos = beginAddress;
			strcpy_s(strpos, STRING_SIZE, (*tableAttribute)[i].index.c_str());  //存属性对应的index名
			beginAddress += STRING_SIZE;
		}

		bm.setUsingSize(bn, bm.getUsingSize(bn) + 3 * sizeof(int) + 3 * STRING_SIZE + sizeof(bool));
		bm.set_dirty(bn);                         //设为脏块
		return 1;								  //操作成功，返回1
	}
	else{
		return 0;								  //操作失败，返回0
	}
}

/*done!*/
int CatalogManager::addIndex(string indexName, string tableName, string attributeNme, int type){
	string FileName = "index.catalog";
	ifstream f_in;
	ofstream f_out;
	f_in.open(FileName.c_str(), ios::in);
	if (!f_in){                                                  // 打开唯一一个总index文件，如果失败说明没有创建，则用f_out创建
		f_out.open(FileName.c_str(), ios::out);
		if (!f_out){
			return 0;
		}
		else{
			f_out.close();
		}
	}
	else{
		f_in.close();
	}
	FileNode *fn = bm.getFile(FileName.c_str());
	BlockNode *bn = bm.getBlockHead(fn);
	char *strpos;                                                 //用来指向拷贝字符串的位置
	char *beginAddress;
	bool *isDelete;
	int *Type;
	int flag = 0;                                                 //用来标记是否是找到了一个isDelete的区域，用新数据覆盖原被删除数据
	while (true){
		if (!bn){
			return 0;                                             //获取块地址失败 返回0
		}
		beginAddress = bm.getContent(bn);
		isDelete = (bool*)beginAddress;
		for (int i = 0; i < bm.getUsingSize(bn) /( 3 * STRING_SIZE + sizeof(int) + sizeof(bool)); i++){
			if (*isDelete == true){
				flag = 1;
				break;
			}
			beginAddress += 3 * sizeof(STRING_SIZE) + sizeof(bool) + sizeof(int);
			isDelete = (bool*)beginAddress;
		}
		if (flag){                                                 //如果在上面循环中找到了一个曾经被delete的区域则覆盖
			*isDelete = false;
			beginAddress += sizeof(bool);                          //修改isDelete

			strpos = beginAddress;
			strcpy_s(strpos, STRING_SIZE, indexName.c_str());
			strpos += STRING_SIZE;
			strcpy_s(strpos, STRING_SIZE, tableName.c_str());
			strpos += STRING_SIZE;
			strcpy_s(strpos, STRING_SIZE, attributeNme.c_str());
			beginAddress += 3 * STRING_SIZE;
			
			Type = (int*)beginAddress;
			*Type = type;
			beginAddress += sizeof(int);
			
			break;
		}
		if (bm.getUsingSize(bn) +3*STRING_SIZE + sizeof(int) + sizeof(bool) <= bm.getBlockSize()){//判断该块剩余空间能否够存下一个index数据
			/*  存储数据 */
			beginAddress = bm.getContent(bn) + bm.getUsingSize(bn);//把起始位置重定位到该块末尾 该块内容开始+已使用空间
			isDelete = (bool*)beginAddress;
			*isDelete = false;
			beginAddress += sizeof(bool);                          //修改isDelete

			strpos = beginAddress;
			strcpy_s(strpos, STRING_SIZE, indexName.c_str());
			strpos += STRING_SIZE;
			strcpy_s(strpos, STRING_SIZE, tableName.c_str());
			strpos += STRING_SIZE;
			strcpy_s(strpos, STRING_SIZE, attributeNme.c_str());
			beginAddress += 3 * STRING_SIZE;

			Type = (int*)beginAddress;
			*Type = type;
			beginAddress += sizeof(int);

			/* 修改改块使用空间，dirty */
			bm.setUsingSize(bn, bm.getUsingSize(bn) + 3 * STRING_SIZE + sizeof(int) + sizeof(bool));
			bm.set_dirty(bn);
			flag = setIndexOnTable(tableName, attributeNme, indexName);
			break;
		}
		else{
			bn = bm.getNextBlock(fn, bn);                           //如果存不下，就取下一个块
		}
	}
	return flag;
}

/*done！*/
int CatalogManager::findTable(string tableName){
	string FileName = tableName + ".catalog";
	ifstream f;
	f.open(FileName.c_str(), ios::in);
	if (!f){
		return -1;
	}
	else{
		f.close();
		return TABLE_FILE;
	}
}

/*done!*/
int CatalogManager::findIndex(string indexName){

	string FileName("index.catalog");
	ifstream f;
	f.open(FileName.c_str());
	if (!f){
		return -1;
	}
	else{
		f.close();
	}
	int flag;
	FileNode *fn = bm.getFile(FileName.c_str());
	BlockNode *bn = bm.getBlockHead(fn);
	while (true){
		if (bn){
			char *beginAddress;
			bool *isDelete;
			char *strpos;
			char tempstr[100];
			string str;

			beginAddress = bm.getContent(bn);
			isDelete = (bool*)beginAddress;
			beginAddress += sizeof(bool);
			flag = UNKOWN_FILE;

			for (int i = 0; i < bm.getUsingSize(bn) / (3 * STRING_SIZE + sizeof(int) + sizeof(bool)); i++){

				strpos = beginAddress;
				strcpy_s(tempstr, STRING_SIZE, strpos);
				str = tempstr;

				if (*isDelete == false && str == indexName){
					flag = INDEX_FILE;
					break;
				}

				beginAddress += 3 * STRING_SIZE + sizeof(int);
				isDelete = (bool*)beginAddress;
				beginAddress += sizeof(bool);
			}
		}
		else{
			return 0;
		}
		if (flag == INDEX_FILE || bn->isBottom) {
			break;
		}
		bn = bm.getNextBlock(fn, bn);
	}
	return flag;
}

/*done!*/
int CatalogManager::dropTable(string tableName){
	string FileName = tableName + ".catalog";
	bm.delete_fileNode(FileName.c_str());
	if (remove(FileName.c_str())){                                    //删除指定的文件 成功则返回0，失败则返回-1      
		return 0;
	}
	return 1;
}

/*done!*/
int CatalogManager::dropIndex(string indexName){
	FileNode *fn = bm.getFile("index.catalog");
	BlockNode *bn = bm.getBlockHead(fn);
	int flag = 0;
	IndexInfo *index = NULL;
	while (true){
		char *beginAddress;
		char *strpos;
		char tempstr[100];
		string str;
		if (!bn){
			return 0;
		}
		else{
			beginAddress = bm.getContent(bn);
			beginAddress += sizeof(bool);          //跳过isdelete
			int i;
			for (i = 0; i < bm.getUsingSize(bn) / (3 * STRING_SIZE + sizeof(int) + sizeof(bool)); i++){
				strpos = beginAddress;
				strcpy_s(tempstr, STRING_SIZE, strpos);
				str = tempstr;
				if (str == indexName){
					flag = 1;
					break;
				}
				beginAddress += (3 * STRING_SIZE + sizeof(int) + sizeof(bool));
			}
		}
		if (flag){
			string str1, str2, str3;
			bool *isDelete;
			beginAddress -= sizeof(bool);
			isDelete = (bool*)beginAddress;
			*isDelete = true;
			beginAddress += sizeof(bool);
			
			strpos = beginAddress;
			strcpy_s(tempstr, STRING_SIZE, strpos);
			str1 = tempstr;
			strpos += STRING_SIZE;
			strcpy_s(tempstr, STRING_SIZE, strpos);
			str2 = tempstr;
			strpos += STRING_SIZE;
			strcpy_s(tempstr, STRING_SIZE, strpos);
			str3 = tempstr;
			flag = revokeIndexOnTable(str2, str3, str1);
			break;
		}
		else{
			bn = bm.getNextBlock(fn, bn);
		}
	}
	return flag;
}

/*done! 但是昨晚测试有问题，可能是传入的rumovenum问题*/
int CatalogManager::removeRecord(string tableName, int removeNum){
	string FileName = tableName + ".catalog";
	FileNode *fn = bm.getFile(FileName.c_str());
	BlockNode *bn = bm.getBlockHead(fn);
	if (bn){
		char *beginAddress = bm.getContent(bn);
		int *recordNum = (int*)beginAddress;
		if (*recordNum < removeNum){
			cout << "Error: in CatalogManager::removeRecord,the number of records needed to delete is greater than current records number." << endl;
			return 0;
		}
		else{
			*recordNum -= removeNum;
			bm.set_dirty(bn);
			return *recordNum;                        //返回当前记录数
		}
	}
	else{
		return 0;
	}
}

/*done!*/
int CatalogManager::insertRecord(string tableName, int insertNum){
	string FileName = tableName + ".catalog";
	FileNode *fn = bm.getFile(FileName.c_str());
	BlockNode *bn = bm.getBlockHead(fn);
	if (bn){
		char *beginAddress = bm.getContent(bn);
		int *recordNum = (int*)beginAddress;
		*recordNum += insertNum;
		bm.set_dirty(bn);
		return *recordNum;                        //返回当前记录数
	}
	else{
		return 0;
	}
}

/*done!*/
int CatalogManager::getRecordNum(string tableName){
	string FileName = tableName + ".catalog";
	FileNode *fn = bm.getFile(FileName.c_str());
	BlockNode *bn = bm.getBlockHead(fn);
	if (bn){
		char *beginAddress = bm.getContent(bn);
		int *recordNum = (int*)beginAddress;
		return *recordNum;                        //返回当前记录数
	}
	else{
		return 0;
	}
}

/*  done!  获取对应indexName的类型 */
int CatalogManager::getIndexType(string indexName){
	string FileName("Index.catalog");
	FileNode *fn = bm.getFile(FileName.c_str());
	BlockNode *bn = bm.getBlockHead(fn);
	int type = -2;
	while (true){
		if (bn){
			char *beginAddress;
			bool *isDelete;
			char *strpos;
			char tempstr[100];
			int *Type;
			string str;
			beginAddress = bm.getContent(bn);
			isDelete = (bool*)beginAddress;
			beginAddress += sizeof(bool);
			
			for (int i = 0; i < bm.getUsingSize(bn) / (3 * STRING_SIZE + sizeof(int) + sizeof(bool)); i++){
				strpos = beginAddress;
				strcpy_s(tempstr, STRING_SIZE, strpos);
				str = tempstr;
				if (str == indexName && *isDelete == false){
					beginAddress += 3 * STRING_SIZE;
					Type = (int*)beginAddress;
					type = *Type;
					break;
				}
				beginAddress += 3 * STRING_SIZE + sizeof(int);
				isDelete = (bool*)beginAddress;
				beginAddress += sizeof(bool);
			}
		}
		else{
			return -2;
		}
		if (bn->isBottom || type != -2){                                         //如果找到了满足条件的index
			break;
		}
		bn = bm.getNextBlock(fn, bn);                                             //获取下一个块
	}
	return 1;
}

/*done!*/
int CatalogManager::getAttribute(string tableName, vector<Attribute>* tableAttribute){
	string FileName = tableName + ".catalog";
	FileNode *fn = bm.getFile(FileName.c_str());
	BlockNode *bn = bm.getBlockHead(fn);
	char *strpos;
	char tempstr[100];
	int *type;
	bool *ifUnique;
	if (bn){
		char* beginAddress = bm.getContent(bn);
		beginAddress += sizeof(int) + STRING_SIZE; //首地址后移，跳过表总记录数，主键
		int* attributeNum = (int*)beginAddress;
		beginAddress += sizeof(int);
		Attribute* a = new Attribute;
		for (int i = 0; i < *attributeNum; i++){
			
			strpos = beginAddress;
			strcpy_s(tempstr, STRING_SIZE, strpos);
			a->name = tempstr;
			beginAddress += STRING_SIZE;

			type = (int*)beginAddress;
			a->type = *type;
			beginAddress += sizeof(int);

			ifUnique = (bool*)beginAddress;
			a->ifUnique = *ifUnique;
			beginAddress += sizeof(bool);

			strpos = beginAddress;
			strcpy_s(tempstr, STRING_SIZE, strpos);
			a->index = tempstr;
			beginAddress += STRING_SIZE;

			tableAttribute->push_back(*a);
		}
		return 1;
	}
	else{
		return 0;
	}
}

/*..str != "" 存疑！！！！. done! */
void CatalogManager::getAllIndexInTable(string tableName, vector<string>& indexName){
	string FileName = tableName + ".catalog";
	FileNode *fn = bm.getFile(FileName.c_str());
	BlockNode *bn = bm.getBlockHead(fn);
	char* strpos;
	char tempstr[100];
	string str;

	char* beginAddress = bm.getContent(bn);
	beginAddress += sizeof(int) + STRING_SIZE;
	int* attributeNum = (int*)beginAddress;
	beginAddress += 2 * sizeof(int) + sizeof(bool) + STRING_SIZE;
	

	for (int i = 0; i < *attributeNum; i++){
		strpos = beginAddress;
		strcpy_s(tempstr, STRING_SIZE, strpos);
		str = tempstr;
		if (str != ""){
			indexName.push_back(str);
		}
		beginAddress += 2 * STRING_SIZE + sizeof(int) + sizeof(bool);
	}
}


int CatalogManager::calculateSize(string tableName){
	string FileName = tableName + ".catalog";
	FileNode *fn = bm.getFile(FileName.c_str());
	BlockNode *bn = bm.getBlockHead(fn);
	int *Type;
	int singleRecordSize = 0;
	if (bn){
		char* beginAddress = bm.getContent(bn);
		beginAddress += sizeof(int) + STRING_SIZE;
		int* attributeNum = (int*)beginAddress;
		beginAddress += STRING_SIZE + sizeof(int);
		
		for (int i = 0; i < *attributeNum; i++){
			Type = (int*)beginAddress;
			if (*Type == -1){
				singleRecordSize += sizeof(float);
			}
			else if (*Type == 0){
				singleRecordSize += sizeof(int);
			}
			else if (*Type > 0){
				singleRecordSize += *Type*sizeof(char);
			}
			else{
				cout << "Error: in CatalogManager::calculateSize,type of record error." << endl;
				return 0;
			}
			beginAddress += 2 * STRING_SIZE + sizeof(bool) + sizeof(int);
		}
		return singleRecordSize;
	}
	else{
		return 0;
	}

}

/*done!*/
int CatalogManager::calculateSize(int type){
	if (type == -1){
		return sizeof(float);
	}
	else if (type == 0){
		return sizeof(int);
	}
	else{
		return sizeof(char)*type;
	}
}

/*done!*/
int CatalogManager::setIndexOnTable(string tableName, string attributeName, string indexName){
	string FileName = tableName + ".catalog";
	FileNode *fn = bm.getFile(FileName.c_str());
	BlockNode *bn = bm.getBlockHead(fn);
	if (bn){
		char* strpos;
		char tempstr[100];
		string str;
		char* beginAddress = bm.getContent(bn);
		beginAddress += sizeof(int) + STRING_SIZE;
		int* attributeNum = (int*)beginAddress;
		beginAddress += sizeof(int);
		int i;
		for (i = 0; i < *attributeNum; i++){
			strpos = beginAddress;
			strcpy_s(tempstr, STRING_SIZE, strpos);
			str = tempstr;
			if (str == attributeName){
				beginAddress += STRING_SIZE + sizeof(int) + sizeof(bool);
				strpos = beginAddress;
				strcpy_s(strpos,STRING_SIZE,indexName.c_str() );
				bm.set_dirty(bn);
				break;
			}
			beginAddress += 2 * STRING_SIZE + sizeof(int) + sizeof(bool);
		}
		if (i < *attributeNum){
			return 1;
		}
		else{
			return 0;
		}
	}
	return 1;
}


/*done!*/
int CatalogManager::revokeIndexOnTable(string tableName, string attributeName, string indexName){
	string FileName = tableName + ".catalog";
	FileNode *fn = bm.getFile(FileName.c_str());
	BlockNode *bn = bm.getBlockHead(fn);
	char* strpos;
	char tempstr[100];
	string str1, str2;
	if (bn){
		char* beginAddress = bm.getContent(bn);
		beginAddress += sizeof(int) + STRING_SIZE;
		int* attributeNum = (int*)beginAddress;
		beginAddress += sizeof(int);
		int i;
		for (i = 0; i < *attributeNum; i++){
			strpos = beginAddress;
			strcpy_s(tempstr, STRING_SIZE, strpos);
			str1 = tempstr;

			strpos += STRING_SIZE + sizeof(int) + sizeof(bool);
			strcpy_s(tempstr, STRING_SIZE, strpos);
			str2 = tempstr;

			if (str1 == attributeName && str2 == indexName){
				memset(strpos, 0, STRING_SIZE);
				bm.set_dirty(bn);
				break;
			}
			beginAddress += 2 * STRING_SIZE + sizeof(int) + sizeof(bool);
		}
		if (i < *attributeNum){
			return 1;
		}
		else{
			return 0;
		}
	}
	return 1;
}

/*done!*/
void CatalogManager::getAllIndexInfo(vector<IndexInfo>* indexList){
	string FileName("Index.catalog");
	FileNode *fn = bm.getFile(FileName.c_str());
	BlockNode *bn = bm.getBlockHead(fn);
	while (true){
		if (bn){
			char *beginAddress;
			bool *isDelete;
			char *strpos;
			char tempstr[100];
			int *type;
			string str;

			beginAddress = bm.getContent(bn);
			isDelete = (bool*)beginAddress;
			IndexInfo *a = new IndexInfo;

			for (int i = 0; i < bm.getUsingSize(bn) / (3 * STRING_SIZE + sizeof(int) + sizeof(bool)); i++){
				if (*isDelete == false){
					beginAddress += sizeof(bool);
					strpos = beginAddress;
					strcpy_s(tempstr, STRING_SIZE, strpos);
					str = tempstr;
					a->indexName = str;

					strpos += STRING_SIZE;
					strcpy_s(tempstr, STRING_SIZE, strpos);
					str = tempstr;
					a->tableName = str;

					strpos += STRING_SIZE;
					strcpy_s(tempstr, STRING_SIZE, strpos);
					str = tempstr;
					a->Attribute = str;

					beginAddress += 3 * STRING_SIZE;
					type = (int*)beginAddress;
					a->type = *type;
					beginAddress += sizeof(int);

					indexList->push_back(*a);
					isDelete = (bool*)beginAddress;
				}
				else{
					beginAddress += 3 * STRING_SIZE + sizeof(int) + sizeof(bool);
					isDelete = (bool*)beginAddress;
				}
			}
		}
		if (bn->isBottom){
			break;
		}
		bn = bm.getNextBlock(fn, bn);
	}
}
