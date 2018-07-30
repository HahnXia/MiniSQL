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
	string indexName;//index��
	string tableName;//����
	string Attribute;//������
	int type;//�������ͣ�����֮һ��
};

/*���õ���������.index�ļ�����ʽ���ڴ����ϵ�
���ݿ�����ʱ��IndexManager�ഴ��ʵ�����������ϵ�.index�ļ����룬��������Ӧ��B+������ͨ��map����ӳ������
����B+����.index�ļ��ѽ���ӳ�䣬��˱��ӿڵĵ����߲���Ҫ֪��B+�������ƺ�ʵ�֣�ֻ��֪��.index�ļ������ƺ��������ͣ����ɶ��������в���*/
class IndexManager {
private:
	BufferManager buffer;//index��ʹ�õ�buffer

	//�������͵�map��������ÿ���ļ�����string����һ��B+����BPlusTree����Ӧ���������ڱ���͹���index
	typedef map<string, BPlusTree<int> *> intMap;
	typedef map<string, BPlusTree<string> *> stringMap;
	typedef map<string, BPlusTree<float> *> floatMap;

	API * api;

	//�����Ĵ���
	/*static */intMap indexIntMap;
	/*static */stringMap indexStringMap;
	/*static */floatMap indexFloatMap;

	//�������͵���ʱ���������������������͵�������
	int intTmp;
	float floatTmp;
	string stringTmp;

	int getDegree(int type);//�õ�һ��block�����ܱ����index����
	int getKeySize(int type);//�õ���ǰ������Ĵ�С
	void setKey(int type, string key);//���õ�ǰ��������

public:
	IndexManager(API* ap);//���캯��������ΪAPI�������API����ȡ.index�ļ��б�����index��Ϣ
	~IndexManager();//�������������ٶ���ʱ�Ὣ����index��Ϣд�ش�����

	void createIndex(string filePath, int type);//����һ��ָ�����ƣ�filePath�������ͣ�type����.index�ļ�
												//����createIndex("salary_index.index",TYPE_INT);
												//�ᴴ��һ����Ϊ��salary_index����.index�ļ�������ΪTYPE_INT

	void dropIndex(string filePath, int type);//ɾ��һ��ָ�����ƣ�filePath�������ͣ�type����.index�ļ����÷�ͬ��

	offsetNumber searchIndex(string filePath, string key, int type);//��ȡָ�����ƣ�filePath�������ͣ�type����.index�ļ���������Ϊkey��indexλ��
																	//.index�ļ���index��λ������ƫ������һ������������ʾ�ģ�֪����ƫ�������ܺ����׵ض�λ����Ӧ��index
																	//����searchIndex("salary_index.index","10000",TYPE_INT);
																	//��������Ϊ��salary_index��������ΪTYPE_INT��.index�ļ���������Ϊ��10000����index����λ��

	void insertIndex(string filePath, string key, offsetNumber blockOffset, int type);//��һ��ָ�����ƣ�filePath�������ͣ�type����.index�ļ���
																					  //λ��blockOffset��λ�ò���һ��������Ϊkey��index
																					  //����insertIndex("salary_index.index","20000",88,TYPE_INT);
																					  //������Ϊ��salary_index��������ΪTYPE_INT��.index�ļ���88��λ�ò���һ��������Ϊ��20000����index

	void deleteIndex(string filePath, string key, int type);//��һ��ָ�����ƣ�filePath�������ͣ�type����.index�ļ���ɾ��һ��������Ϊkey��index
															//����deleteIndex("salary_index.index","20000",TYPE_INT);
															//������Ϊ��salary_index��������ΪTYPE_INT��.index�ļ���ɾ��������Ϊ��20000����index

	void readIndex(string filePath, int type);//��һ��ָ�����ƣ�filePath�������ͣ�type����.index�ļ��ж�ȡindex��Ϣ��������ӦB+����������Ӧ��map�����н������ߵ�ӳ��
};



#endif
