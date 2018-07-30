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
	char* FileName;//文件名，用这个来区别不同的文件
	// FileNode* preFile;//双向链表
	// FileNode* nextFile;
	BlockNode* HeadBlock;//该文件的第一个block
	bool pin;//这个文件的有没有被锁定
	//bool isUsed;//文件池中这个文件有没有被被使用
};

class BlockNode//这里的block都是代表读到内存的block，类里面储存了相关的属性信息
{
	friend class BufferManager;
public:
	bool isBottom;//这个block是不是所属文件的最后一个block
	int offset;//这个block在这个文件中所处的位置，-1代表这个block没有被使用
private:
	char* address;//具体数据在内存中的地址
	BlockNode* preBlock;//文件中的前一个block
	BlockNode* nextBlock;
	FileNode* block_file;//所属文件
	bool pin;//这个block有没有被锁定
	bool dirty;//这个block有没有被修改过
	bool LRU;//替换策略LRU,这个量代表是否是最新的block
	size_t usingSize;//这个block被使用的空间长度
};

class BufferManager
{
private:
	FileNode* HeadFile;//第一个文件
	FileNode FilePool[MAX_FILE_NUM];//文件池
	BlockNode BlockPool[MAX_BLOCK_NUM];//读到内存里的block池
	int LastReplaced;//block池中，最后一个被刷新过的block
	int FileNum;//用过的file和block数
	int BlockNum;
	void refresh_file(FileNode &File);//刷新一个文件
	void refresh_block(BlockNode &Block);//刷新一个block
	BlockNode* getBlock(FileNode* curFile, BlockNode* curBlock);//将指定的curBlock的后一块从文件中读取到内存中
	//size_t getUsingSizeInBlock(BlockNode* block);//取得一个block被使用的空间大小
	void writeBackToDisk(const char* fileName, BlockNode* block);//写磁盘
	void writeAllToDisk();//把缓存区所有的块都写到磁盘
public:
	BufferManager();
	~BufferManager();
	FileNode* getFile(const char* fileName);//取得一个文件，根据文件的名字，也就是表的名字,如果当前没有这个文件就生成这个文件
	BlockNode* getBlockHead(FileNode* file);//取得得一个文件的第一个block,如果不存在则申请一个
	BlockNode* getNextBlock(FileNode* file, BlockNode* block);//取得一个文件的某个block的后一个block，这里的某一个block指的是已经读到内存里面的block
	BlockNode* getBlockByOffset(FileNode* file, int offsetNum);//根据block在文件中的位置来从去得block，可以保证从头到这个点之间所有的block都会在内存中
	void set_dirty(BlockNode* block);//将某个内存中的block标记为更改过dirty
	void set_pin(FileNode* file, bool pin);//更改某个文件的锁定状态
	void set_pin(BlockNode* block, bool pin);//更改某个block的锁定状态
	size_t getUsingSize(BlockNode* block);//取得某个block已经使用的空间大小
	void setUsingSize(BlockNode* block, size_t usingSize);//更改某个block已经使用的空间大小
	char* getContent(BlockNode* block);//取得某个block里面的内容的头指针
	//每个block的第一size_t节保存了这个block已经使用的字节数信息
	void delete_fileNode(const char* fileName);//删除文件池中某个文件,仅仅在内存中
	int getBlockSize();//返回每个block实际能储存的数据的字节数
};



#endif /* BufferManager_h */
