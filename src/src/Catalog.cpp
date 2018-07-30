#include "CatalogManager.h"
#define STRING_SIZE 100
using namespace std;

/* ���¶���catalog�ļ��е����ݴ�Žṹ��
   ���� ������.catalog���ļ�   int-insert�ļ�¼��  char[100]-������     int-���Ե�����  
                               char[100]-������     int-type            bool-ifUnique       char[100]-���Զ�Ӧ��index

   ���� ��index.catalog���ļ�  bool-isDelete       char[100]-indexName  char[100]tableName  char[100]-Attribute     int-type
*/



CatalogManager::CatalogManager(){

}

CatalogManager::~CatalogManager(){

}

/*done!*/
int CatalogManager::addTable(string tablename, vector<Attribute>* tableAttribute, string primaryKeyName){
	string FileName = tablename + ".catalog";    // ���������
	ofstream f;
	f.open(FileName.c_str(), ios::out);          //�������ʽ�򿪱����û�����½�һ��������������
	if (!f){
		return 0;                                //����ʧ�ܣ�����0
	}
	else{
		f.close();                               //ֱ�ӹرգ�������ͨ��bufferд������
	}
	FileNode *fn = bm.getFile(FileName.c_str()); //����getFile��������ļ�ָ��
	BlockNode *bn = bm.getBlockHead(fn);		 //����getBlockHead������ø��ļ����׸����ַ
	char *strpos;                                  //����ָ�򿽱��ַ�����λ��

	if (bn){
		char *beginAddress = bm.getContent(bn);  //�׵�ַָ��
		int *recordNum;                          //ָ�� �ܼ�¼����Ӧλ�� ָ��
		int *attributeNum;                       //ָ�� ��¼��������λ�� ָ��
		int *type;
		bool *ifUnique;

		recordNum = (int*)beginAddress;
		*recordNum = 0;							 //�ܼ�¼����ʼ��Ϊ0
		beginAddress += sizeof(int);             //beginAddress���ƶ�Ӧ��С

		strpos = beginAddress;
		strcpy_s(strpos, STRING_SIZE, primaryKeyName.c_str());
		beginAddress += STRING_SIZE;

		attributeNum = (int*)beginAddress;        //��������
		*attributeNum = tableAttribute->size();
		beginAddress += sizeof(int);

		for (int i = 0; i < tableAttribute->size(); i++){          //forѭ�����δ��������
			strpos = beginAddress;                                
			strcpy_s(strpos, STRING_SIZE, (*tableAttribute)[i].name.c_str());  //��������
			beginAddress += STRING_SIZE;

			type = (int*)beginAddress;
			*type = (*tableAttribute)[i].type;
			beginAddress += sizeof(int);

			ifUnique = (bool*)beginAddress;
			*ifUnique = (*tableAttribute)[i].ifUnique;
			beginAddress += sizeof(bool);

			strpos = beginAddress;
			strcpy_s(strpos, STRING_SIZE, (*tableAttribute)[i].index.c_str());  //�����Զ�Ӧ��index��
			beginAddress += STRING_SIZE;
		}

		bm.setUsingSize(bn, bm.getUsingSize(bn) + 3 * sizeof(int) + 3 * STRING_SIZE + sizeof(bool));
		bm.set_dirty(bn);                         //��Ϊ���
		return 1;								  //�����ɹ�������1
	}
	else{
		return 0;								  //����ʧ�ܣ�����0
	}
}

/*done!*/
int CatalogManager::addIndex(string indexName, string tableName, string attributeNme, int type){
	string FileName = "index.catalog";
	ifstream f_in;
	ofstream f_out;
	f_in.open(FileName.c_str(), ios::in);
	if (!f_in){                                                  // ��Ψһһ����index�ļ������ʧ��˵��û�д���������f_out����
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
	char *strpos;                                                 //����ָ�򿽱��ַ�����λ��
	char *beginAddress;
	bool *isDelete;
	int *Type;
	int flag = 0;                                                 //��������Ƿ����ҵ���һ��isDelete�������������ݸ���ԭ��ɾ������
	while (true){
		if (!bn){
			return 0;                                             //��ȡ���ַʧ�� ����0
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
		if (flag){                                                 //���������ѭ�����ҵ���һ��������delete�������򸲸�
			*isDelete = false;
			beginAddress += sizeof(bool);                          //�޸�isDelete

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
		if (bm.getUsingSize(bn) +3*STRING_SIZE + sizeof(int) + sizeof(bool) <= bm.getBlockSize()){//�жϸÿ�ʣ��ռ��ܷ񹻴���һ��index����
			/*  �洢���� */
			beginAddress = bm.getContent(bn) + bm.getUsingSize(bn);//����ʼλ���ض�λ���ÿ�ĩβ �ÿ����ݿ�ʼ+��ʹ�ÿռ�
			isDelete = (bool*)beginAddress;
			*isDelete = false;
			beginAddress += sizeof(bool);                          //�޸�isDelete

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

			/* �޸ĸĿ�ʹ�ÿռ䣬dirty */
			bm.setUsingSize(bn, bm.getUsingSize(bn) + 3 * STRING_SIZE + sizeof(int) + sizeof(bool));
			bm.set_dirty(bn);
			flag = setIndexOnTable(tableName, attributeNme, indexName);
			break;
		}
		else{
			bn = bm.getNextBlock(fn, bn);                           //����治�£���ȡ��һ����
		}
	}
	return flag;
}

/*done��*/
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
	if (remove(FileName.c_str())){                                    //ɾ��ָ�����ļ� �ɹ��򷵻�0��ʧ���򷵻�-1      
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
			beginAddress += sizeof(bool);          //����isdelete
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

/*done! ����������������⣬�����Ǵ����rumovenum����*/
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
			return *recordNum;                        //���ص�ǰ��¼��
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
		return *recordNum;                        //���ص�ǰ��¼��
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
		return *recordNum;                        //���ص�ǰ��¼��
	}
	else{
		return 0;
	}
}

/*  done!  ��ȡ��ӦindexName������ */
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
		if (bn->isBottom || type != -2){                                         //����ҵ�������������index
			break;
		}
		bn = bm.getNextBlock(fn, bn);                                             //��ȡ��һ����
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
		beginAddress += sizeof(int) + STRING_SIZE; //�׵�ַ���ƣ��������ܼ�¼��������
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

/*..str != "" ���ɣ�������. done! */
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
