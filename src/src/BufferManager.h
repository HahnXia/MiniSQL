#ifndef BufferManager_h
#define BufferManager_h

#include <iostream>
#include <fstream>
#include <cstring>
using namespace std;

#define MACTESTMODE 0

#if MACTESTMODE
#define MAX_FILE_NUM 10
#define MAX_BLOCK_NUM 10
#else
#define MAX_FILE_NUM 64
#define MAX_BLOCK_NUM 4096
#endif

#define MAX_FILE_NAME 128
#define BLOCK_SIZE 4096

class FileNode;
class BlockNode;

class FileNode
{
	friend class BufferManager;
private:
	char* FileName;//�ļ����������������ͬ���ļ�
	// FileNode* preFile;//˫������
	// FileNode* nextFile;
	BlockNode* HeadBlock;//���ļ��ĵ�һ��block
	bool pin;//����ļ�����û�б�����
	//bool isUsed;//�ļ���������ļ���û�б���ʹ��
};

class BlockNode//�����block���Ǵ�������ڴ��block�������洢������ص�������Ϣ
{
	friend class BufferManager;
public:
	bool isBottom;//���block�ǲ��������ļ������һ��block
	int offset;//���block������ļ���������λ�ã�-1�������blockû�б�ʹ��
private:
	char* address;//�����������ڴ��еĵ�ַ
	BlockNode* preBlock;//�ļ��е�ǰһ��block
	BlockNode* nextBlock;
	FileNode* block_file;//�����ļ�
	bool pin;//���block��û�б�����
	bool dirty;//���block��û�б��޸Ĺ�
	bool LRU;//�滻����LRU,����������Ƿ������µ�block
	size_t usingSize;//���block��ʹ�õĿռ䳤��
};

class BufferManager
{
private:
	FileNode* HeadFile;//��һ���ļ�
	FileNode FilePool[MAX_FILE_NUM];//�ļ���
	BlockNode BlockPool[MAX_BLOCK_NUM];//�����ڴ����block��
	int LastReplaced;//block���У����һ����ˢ�¹���block
	int FileNum;//�ù���file��block��
	int BlockNum;
	void refresh_file(FileNode &File);//ˢ��һ���ļ�
	void refresh_block(BlockNode &Block);//ˢ��һ��block
	BlockNode* getBlock(FileNode* curFile, BlockNode* curBlock);//��ָ����curBlock�ĺ�һ����ļ��ж�ȡ���ڴ���
	//size_t getUsingSizeInBlock(BlockNode* block);//ȡ��һ��block��ʹ�õĿռ��С
	void writeBackToDisk(const char* fileName, BlockNode* block);//д����
	void writeAllToDisk();//�ѻ��������еĿ鶼д������
public:
	BufferManager();
	~BufferManager();
	FileNode* getFile(const char* fileName);//ȡ��һ���ļ��������ļ������֣�Ҳ���Ǳ������,�����ǰû������ļ�����������ļ�
	BlockNode* getBlockHead(FileNode* file);//ȡ�õ�һ���ļ��ĵ�һ��block,���������������һ��
	BlockNode* getNextBlock(FileNode* file, BlockNode* block);//ȡ��һ���ļ���ĳ��block�ĺ�һ��block�������ĳһ��blockָ�����Ѿ������ڴ������block
	BlockNode* getBlockByOffset(FileNode* file, int offsetNum);//����block���ļ��е�λ������ȥ��block�����Ա�֤��ͷ�������֮�����е�block�������ڴ���
	void set_dirty(BlockNode* block);//��ĳ���ڴ��е�block���Ϊ���Ĺ�dirty
	void set_pin(FileNode* file, bool pin);//����ĳ���ļ�������״̬
	void set_pin(BlockNode* block, bool pin);//����ĳ��block������״̬
	size_t getUsingSize(BlockNode* block);//ȡ��ĳ��block�Ѿ�ʹ�õĿռ��С
	void setUsingSize(BlockNode* block, size_t usingSize);//����ĳ��block�Ѿ�ʹ�õĿռ��С
	char* getContent(BlockNode* block);//ȡ��ĳ��block��������ݵ�ͷָ��
	//ÿ��block�ĵ�һsize_t�ڱ��������block�Ѿ�ʹ�õ��ֽ�����Ϣ
	void delete_fileNode(const char* fileName);//ɾ���ļ�����ĳ���ļ�,�������ڴ���
	int getBlockSize();//����ÿ��blockʵ���ܴ�������ݵ��ֽ���
};



#endif /* BufferManager_h */
