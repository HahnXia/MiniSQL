#include "API.h"
#include <iostream>
#include <sstream>

#define UNKOWN_FILE 8
#define TABLE_FILE 9
#define INDEX_FILE 10
using namespace std;

API::API(){}
API::~API(){}

bool API::CreateTable(string tablename, vector<Attribute> *tableattribute, string primarykeyname){
	if (cm->findTable(tablename) == TABLE_FILE){
		cout << "Error: " << tablename << " already exists!" << endl;
		return false;
	}
	if (rm->Createtable(tablename) && cm->addTable(tablename, tableattribute, primarykeyname)){
		if (primarykeyname != " "){
			string indexname = primarykeyname + "_Primary_" + tablename;
			for (vector<Attribute>::iterator iter = tableattribute->begin(); iter != tableattribute->end(); iter++){
				if (iter->name == primarykeyname){
					iter->index = indexname;
				}
			}
			string indexfilename = indexname + ".index";
			int type = getAttrType(tableattribute, primarykeyname);
			//			if(type!=UNKOWN_TYPE){
			im->createIndex(indexfilename, type);
			cm->addIndex(indexname, tablename, primarykeyname, type);

			//			}
			//			else{
			//				cout<<"Error: Type error of attribute "<<primarykeyname<<"in table "<<tablename<<endl;
			//				return false;
			//			}
		}
		return true;
	}
	return false;
}

bool API::DropTable(string tablename){
	if (cm->findTable(tablename) != TABLE_FILE){
		cout << "Error: Table " << tablename << " does not exist!" << endl;
		return false;
	}
	//删除全部相关索引
	vector<string> indexname;
	cm->getAllIndexInTable(tablename, indexname);
	for (vector<string>::iterator iter = indexname.begin(); iter != indexname.end(); iter++){
		DropIndex(*iter);
	}
	if (rm->DropTable(tablename) && cm->dropTable(tablename)){
		return true;
	}
	return false;
}

void API::CreateIndex(string indexname, string tablename, string attributename){
	if (cm->findIndex(indexname) == INDEX_FILE){
		cout << "There is an index " << indexname << " already exists!" << endl;
		return;
	}
	if (cm->findTable(tablename) != TABLE_FILE){
		cout << "Error: Table " << tablename << " does not exist!" << endl;
		return;
	}

	int isfind = 0;
	int type;

	//判断属性是否为单值
	vector<Attribute> allattribute;
	if (cm->getAttribute(tablename, &allattribute)){
		for (vector<Attribute>::iterator iter = allattribute.begin(); iter != allattribute.end(); iter++){
			if (iter->name == attributename){
				isfind = 1;
				if (!iter->ifUnique){
					cout << "Error: cannot create index on a not unique attribute of " << attributename << " !" << endl;
					return;
				}
				type = iter->type;
				break;
			}
		}
	}
	if (isfind == 0){
		cout << "Error: Table " << tablename << " doesnot have the attribute" << attributename << " !" << endl;
		return;
	}

	string indexfilename = indexname + ".index";
	if (cm->addIndex(indexname, tablename, attributename, type)){
		im->createIndex(indexfilename, type);
		rm->IndexCreateFromRecord(tablename, indexname);
	}
	else{
		cout << "Some error in creating index!" << endl;
	}
}

void API::DropIndex(string indexname){
	if (cm->findIndex(indexname) != INDEX_FILE){
		cout << "Index " << indexname << " doesnot exist!" << endl;
		return;
	}
	int type = cm->getIndexType(indexname);
	string indexfilename = indexname + ".index";
	if (cm->dropIndex(indexname)){
		im->dropIndex(indexfilename, type);
	}
	else{
		cout << "Some error in droping index " << indexname << "!" << endl;
	}
	return;
}

void API::InsertRecord(string tablename, vector<string> *attributevalue){
	if (cm->findTable(tablename) != TABLE_FILE){
		cout << "Error: Table " << tablename << " doesnot exist!" << endl;
		return;
	}

	vector<Attribute> atmp;
	cm->getAttribute(tablename, &atmp);
	vector<string>::iterator iter2 = attributevalue->begin();
	for (vector<Attribute>::iterator iter1 = atmp.begin(); iter1 != atmp.end(); iter1++, iter2++){
		if (iter1->index != ""){//判断索引属性数据是否会重复
			int offset = im->searchIndex(iter1->index + ".index", *iter2, iter1->type);
			if (offset != -1){//找不到会返回-1
				cout << "Error: data duplication in unique attribute!" << endl;
				return;
			}
		}
		if (iter1->ifUnique == true){//判断单值属性是否会重复
			Condition condition(iter1->name, OP_EQUAL, *iter2);
			vector<Condition> conditionlist;
			conditionlist.insert(conditionlist.end(), condition);
			if (rm->FindRecord(tablename, &conditionlist)){
				cout << "Insert fails because unique value exists!" << endl;
				return;
			}
		}
	}

	char* recordstring = recordStringGet(tablename, attributevalue);
	if (recordstring == NULL) return;
	int recordsize = cm->calculateSize(tablename);
	int blockoffset = rm->InsertRecord(tablename, recordstring, recordsize);
	if (blockoffset != -1){
		InsertIndexInRecord(recordstring, recordsize, &atmp, blockoffset);
		cm->insertRecord(tablename, 1);//更改catalog记录数目
	}
	delete[] recordstring;
}


void API::SelectRecord(string tablename, vector<string> *attributename, vector<Condition> *conditionlist){
	if (cm->findTable(tablename) != TABLE_FILE){
		cout << "Error: Table " << tablename << " doesnot exist!" << endl;
		return;
	}

	vector<string> aname;
	vector<Attribute> atmp;
	cm->getAttribute(tablename, &atmp);
	for (vector<Attribute>::iterator iter = atmp.begin(); iter != atmp.end(); iter++){
		aname.push_back(iter->name);
	}
	//如果select *,将属性名全部传入
	if (attributename == NULL){
		attributename = &aname;
	}
	else{ //判断是否select的所有属性都在表属性中
		int isfound = 0;
		vector<string>::iterator iter1;
		for (iter1 = attributename->begin(); iter1 != attributename->end(); iter1++){
			isfound = 0;
			for (vector<string>::iterator iter2 = aname.begin(); iter2 != aname.end(); iter2++){
				if (*iter1 == *iter2){
					isfound = 1;
					break;
				}
			}
			if (!isfound){
				break;
			}
		}
		if (!isfound){
			cout << "Error: The attribute " << *iter1 << " is not in the table!" << endl;
			return;
		}
	}

	//判断条件中的属性名是否都在表属性中
	if (conditionlist != NULL){
		int found = 0;
		vector<string>::iterator iter2;
		for (vector<Condition>::iterator iter1 = conditionlist->begin(); iter1 != conditionlist->end(); iter1++){
			found = 0;
			for (iter2 = aname.begin(); iter2 != aname.end(); iter2++){
				if (iter1->getAttr() == *iter2){
					found = 1;
					break;
				}
			}
			if (!found){
				break;
			}
		}
		if (!found){
			cout << "Error: the attribute " << *iter2 << " in condition is not in the table!" << endl;
			return;
		}
	}

	int offset = -1;
	//看条件中的一些属性是否有索引
	if (conditionlist != NULL){
		Attribute *tmp = NULL;
		for (vector<Condition>::iterator iter1 = conditionlist->begin(); iter1 != conditionlist->end(); iter1++){
			for (int iter2 = 0; iter2<atmp.size(); iter2++){
				if (iter1->getAttr() == atmp[iter2].name){
					tmp = &atmp[iter2];
					break;
				}
			}
			if (iter1->getOP() == OP_EQUAL && tmp->index != ""){
				offset = im->searchIndex(tmp->index + ".index", iter1->getval(), tmp->type);
				break;
			}
		}
	}

	int state = rm->SelectRecord(tablename, attributename, conditionlist, offset);
	return;
}

void API::DeleteRecord(string tablename, vector<Condition> *conditionlist){
	if (cm->findTable(tablename) != TABLE_FILE){
		cout << "Error: Table " << tablename << " does not exist!" << endl;
		return;
	}

	vector<string> aname;
	vector<Attribute> atmp;
	cm->getAttribute(tablename, &atmp);
	for (vector<Attribute>::iterator iter = atmp.begin(); iter != atmp.end(); iter++){
		aname.push_back(iter->name);
	}

	//判断条件中的属性名是否都在表属性中
	if (conditionlist != NULL){
		int found = 0;
		vector<Condition>::iterator iter1;
		for (iter1 = conditionlist->begin(); iter1 != conditionlist->end(); iter1++){
			found = 0;
			for (vector<string>::iterator iter2 = aname.begin(); iter2 != aname.end(); iter2++){
				if (iter1->getAttr() == *iter2){
					found = 1;
					break;
				}
			}
			if (!found){
				break;
			}
		}
		if (!found){
			cout << "Error: the attribute " << iter1->getAttr() << " in condition is not in the table!" << endl;
			return;
		}
	}

	int offset = -1;
	//看条件中的一些属性是否有索引
	if (conditionlist != NULL){
		Attribute *tmp = NULL;
		for (vector<Condition>::iterator iter1 = conditionlist->begin(); iter1 != conditionlist->end(); iter1++){
			for (int iter2 = 0; iter2<atmp.size(); iter2++){
				if (iter1->getAttr() == atmp[iter2].name){
					tmp = &atmp[iter2];
					break;
				}
			}
			if (iter1->getOP() == OP_EQUAL && tmp->index != ""){
				offset = im->searchIndex(tmp->index + ".index", iter1->getval(), tmp->type);
				break;
			}
		}
	}

	//返回删除数量
	int num = rm->DeleteRecord(tablename, conditionlist, offset);
	int state = cm->removeRecord(tablename, num);
	if (num >= 0) {
		cout << num << " records are deleted!" << endl;
	}
	return;
}

void API::InsertIndexInRecord(char *recordbegin, int recordsize, vector<Attribute> *allattribute, int blockoffset){
	char *attributebegin = recordbegin;
	for (vector<Attribute>::iterator iter = allattribute->begin(); iter != allattribute->end(); iter++){
		int typesize = TypeSizeGet(iter->type);
		if (iter->index != ""){
			InsertIndex(iter->index, attributebegin, iter->type, blockoffset);
		}
		attributebegin += typesize;
	}
}

void API::InsertIndex(string indexname, char* attributebegin, int type, int blockoffset){
	string content = "";
	stringstream tmp;
	if (type == TYPE_INT)
	{
		int value = *((int*)attributebegin);
		tmp << value;
	}
	else if (type == TYPE_FLOAT)
	{
		float value = *((float*)attributebegin);
		tmp << value;
	}
	else
	{
		char value[255];
		memset(value, 0, 255);
		memcpy(value, attributebegin, type*sizeof(char));
		string stringTmp = value;
		tmp << stringTmp;
	}
	tmp >> content;
	im->insertIndex(indexname + ".index", content, blockoffset, type);
}

void API::deleteIndex(string indexFileName, char* attributebegin, int type){
	string content = "";
	stringstream tmp;
	if (type == TYPE_INT)
	{
		int value = *((int*)attributebegin);
		tmp << value;
	}
	else if (type == TYPE_FLOAT)
	{
		float value = *((float*)attributebegin);
		tmp << value;
	}
	else
	{
		char value[255];
		memset(value, 0, 255);
		memcpy(value, attributebegin, type*sizeof(char));
		string stringTmp = value;
		tmp << stringTmp;
	}
	tmp >> content;
	im->deleteIndex(indexFileName, content, type);
}

void API::getAllIndex(vector<IndexInfo>* indexlist){
	cm->getAllIndexInfo(indexlist);
}

int API::GetAttribute(string tablename, vector<Attribute> *attribute){
	int state = cm->getAttribute(tablename, attribute);
	int recordsize = 0;
	for (vector<Attribute>::iterator iter = attribute->begin(); iter != attribute->end(); iter++){
		recordsize += TypeSizeGet(iter->type);
	}
	return recordsize;
}

int API::TypeSizeGet(int type){
	return cm->calculateSize(type);
}

int API::getAttrType(vector<Attribute>* tableAttribute, string attrName){
	int type;
	for (vector<Attribute>::iterator iter = tableAttribute->begin(); iter != tableAttribute->end(); iter++){
		if (iter->name == attrName){
			type = iter->type;
			break;
		}
	}
	return type;
}

char* API::recordStringGet(string tableName, vector<string>* recordContent){
	vector<Attribute> atmp;
	cm->getAttribute(tableName, &atmp);
	char *content = new char[2000];
	char *contenthead = content;
	memset(content, 0, 2000);
	vector<string>::iterator iter2 = recordContent->begin();
	for (vector<Attribute>::iterator iter1 = atmp.begin(); iter1 != atmp.end(); iter1++, iter2++){
		int type = iter1->type;
		int typesize = TypeSizeGet(type);
		stringstream ss;
		ss << *iter2;
		if (type == TYPE_INT){
			string tmp;
			ss >> tmp;
			for (int i = 0; i<tmp.size(); i++){
				if (i == 0){
					if (tmp.at(i) == '-') continue;
				}
				if (tmp.at(i)>'9' || tmp.at(i)<'0'){
					cout << "Error:" << tmp << "is not int type!" << endl;
					return NULL;
				}
			}
			int int_tmp;
			int_tmp = atoi(tmp.c_str());
			memcpy(content, (char*)&int_tmp, sizeof(int));
		}
		else if (type == TYPE_FLOAT){
			float float_tmp;
			ss >> float_tmp;
			memcpy(content, (char*)&float_tmp, sizeof(float));
		}
		else{
			memcpy(content, iter2->c_str(), typesize);
		}
		content += typesize;
	}
	return contenthead;
}