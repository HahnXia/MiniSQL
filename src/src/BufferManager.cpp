#include "BufferManager.h"
#pragma warning(disable:4996)
void BufferManager::refresh_file(FileNode &File){
	memset(File.FileName, 0, MAX_FILE_NAME);
	//File.preFile=File.nextFile=NULL;
	File.HeadBlock = NULL;
	File.pin = false;
	//File.isUsed=false;
}

void BufferManager::refresh_block(BlockNode &Block){
	Block.preBlock = Block.nextBlock = NULL;
	Block.block_file = NULL;
	Block.offset = -1;
	Block.pin = Block.dirty = Block.isBottom = Block.LRU = false;
	Block.usingSize = 0;
	size_t zero = 0;

	memset(Block.address, 0, BLOCK_SIZE);
	memcpy(Block.address, (char*)&zero, sizeof(size_t));
}

BlockNode* BufferManager::getBlock(FileNode *curFile, BlockNode *curBlock){
	BlockNode *btmp;

	if (BlockNum == 0){
		btmp = &BlockPool[BlockNum++];
	}
	else if (BlockNum<MAX_BLOCK_NUM){
		int i;
		for (i = 0; BlockPool[i].offset != -1; i++);
		btmp = &BlockPool[i];
		BlockNum++;
	}
	else{
		int i;
		//在文件池中找到LRU的坑的位置
		for (i = (LastReplaced == MAX_BLOCK_NUM - 1 ? 0 : LastReplaced + 1); !(BlockPool[i].LRU == false && BlockPool[i].pin == false); i = (i == MAX_BLOCK_NUM - 1) ? 0 : i + 1)
			if (BlockPool[i].LRU == true)
				BlockPool[i].LRU = false;
		//释放这个坑,包括这个block在对应文件block表的处理和内容写到磁盘
		LastReplaced = i;
		btmp = &BlockPool[i];
		if (btmp->preBlock)
			btmp->preBlock->nextBlock = btmp->nextBlock;
		if (btmp->nextBlock)
			btmp->nextBlock->preBlock = btmp->preBlock;
		if (!btmp->preBlock){
			if (btmp->block_file->HeadBlock != btmp){
				cout << "program bug" << endl;
				exit(2);
			}
			btmp->block_file->HeadBlock = btmp->nextBlock;
		}
		writeBackToDisk(btmp->block_file->FileName, btmp);
		refresh_block(*btmp);
	}

	//申请到了btmp作为新的block的空间
	btmp->LRU = true;
	btmp->block_file = curFile;
	//将新的block加入到文件的block表中
	if (!curBlock){
		if (curFile->HeadBlock){//可能原来的头block已经被从block池中挤掉了,这里的block并不是头block
			curFile->HeadBlock->preBlock = btmp;
			btmp->nextBlock = curFile->HeadBlock;
		}
		curFile->HeadBlock = btmp;
		btmp->offset = 0;
	}
	else if (!curBlock->nextBlock){
		btmp->preBlock = curBlock;
		curBlock->nextBlock = btmp;
		btmp->offset = curBlock->offset + 1;
	}
	else{
		btmp->preBlock = curBlock;
		btmp->nextBlock = curBlock->nextBlock;
		curBlock->nextBlock->preBlock = btmp;
		curBlock->nextBlock = btmp;
		btmp->offset = curBlock->offset + 1;
	}
	//￥”￥≈≈????°’?∏?blockμ?????
#if MACTESTMODE
	char temp[100] = "/Users/mac/Desktop/MiniSQL/test/";
	strcat(temp, curFile->FileName);
	fstream Tfile(temp, ios::in | ios::binary);
#else
	fstream Tfile(curFile->FileName, ios::in | ios::binary);
#endif
	Tfile.seekg(BLOCK_SIZE*btmp->offset, ios::beg);
	Tfile.read(btmp->address, BLOCK_SIZE);
	//≥??oa?≥§??
	btmp->usingSize = *(size_t*)btmp->address;
	//≈–??Ω·?≤
	Tfile.seekg((btmp->offset + 1)*BLOCK_SIZE, ios::beg);
	char tempchar;
	Tfile.read(&tempchar, 1);
	if (Tfile.gcount() == 0)
		btmp->isBottom = true;

	return btmp;

}

void BufferManager::writeBackToDisk(const char *fileName, BlockNode *block){
#if MACTESTMODE
	char temp[100] = "/Users/mac/Desktop/MiniSQL/test/";
	strcat(temp, fileName);
#endif

	if (!block->dirty)//没有被更改过，不用写
		return;
	else{
		//如果不存在这个文件则创建
#if MACTESTMODE
		fstream create(temp, ios::in | ios::binary);
#else
		fstream create(fileName, ios::in | ios::binary);
#endif
		if (!create){
			//        fstream create(fileName, ios::in|ios::binary);
			//            cout << "none" << endl;
#if MACTESTMODE
			fstream create(temp, ios::out | ios::binary);
#else
			fstream create(fileName, ios::out | ios::binary);
#endif
			//          if(create)
			//              cout << "now exist" << endl;
			//          else
			if (!create)
				cout << "fail to create file" << endl;
		}
		create.close();

		//现在开始读写
#if MACTESTMODE
		fstream Tfile(temp, ios::in | ios::out | ios::binary);
#else
		fstream Tfile(fileName, ios::in | ios::out | ios::binary);
#endif
		Tfile.seekp(block->offset*BLOCK_SIZE, ios::beg);
		Tfile.write(block->address, BLOCK_SIZE);//写一个block的空间
		Tfile.close();
	}
}

void BufferManager::writeAllToDisk(){
	BlockNode *btmp;
	for (int i = 0; i<FileNum; i++)
		for (btmp = FilePool[i].HeadBlock; btmp; btmp = btmp->nextBlock)
			writeBackToDisk(FilePool[i].FileName, btmp);
}

BufferManager::BufferManager(){
	HeadFile = NULL;
	FileNum = BlockNum = 0;
	LastReplaced = -1;
	for (int i = 0; i<MAX_BLOCK_NUM; i++){
		BlockPool[i].address = new char[BLOCK_SIZE];
		refresh_block(BlockPool[i]);
	}
	for (int i = 0; i<MAX_FILE_NUM; i++){
		FilePool[i].FileName = new char[MAX_FILE_NAME];
		refresh_file(FilePool[i]);
	}
}

BufferManager::~BufferManager(){
	writeAllToDisk();
	for (int i = 0; i<MAX_BLOCK_NUM; i++)
		delete[] BlockPool[i].address;
	for (int i = 0; i<MAX_FILE_NUM; i++)
		delete[] FilePool[i].FileName;
}

//因为delete_file，一定要加上链表的操作TAT
//好像又不用了，delete的时候把后面的移到前面
FileNode* BufferManager::getFile(const char*fileName){
	if (strlen(fileName)>MAX_FILE_NAME - 1){
		cout << "file name out of range" << endl;
		exit(2);
	}
	//已经存在这个file
	for (int i = 0; i<FileNum; i++)
		if (0 == strcmp(fileName, FilePool[i].FileName))
			return &FilePool[i];
	//如果当前不存在
	//buffer中允许存在更多的的file
	if (FileNum<MAX_FILE_NUM){
		strcpy(FilePool[FileNum].FileName, fileName);
		return &FilePool[FileNum++];
	}
	//buffer中不允许存在更多file,需要替换
	else{
		if (FileNum != MAX_FILE_NUM)
			cout << "Bug exists here" << endl;
		int FitPos;
		for (FitPos = 0; FitPos<FileNum; FitPos++){
			if (FilePool[FitPos].pin)
				continue;
			FileNode *thefile = &FilePool[FitPos];
			BlockNode *next;
			for (BlockNode *btemp = thefile->HeadBlock; btemp; btemp = next){
				next = btemp->nextBlock;
				writeBackToDisk(thefile->FileName, btemp);
				refresh_block(*btemp);
				BlockNum--;
			}
			refresh_file(*thefile);
			strcpy(thefile->FileName, fileName);
			thefile->HeadBlock = NULL;
			thefile->pin = false;
			return thefile;
		}
	}
	cout << "not enough space for new file" << endl;
	return NULL;
}

BlockNode* BufferManager::getBlockHead(FileNode* file){
	if (file->HeadBlock && file->HeadBlock->offset == 0)
		return file->HeadBlock;
	else
		return getBlock(file, NULL);
}

BlockNode* BufferManager::getNextBlock(FileNode *file, BlockNode *block){
	if (!block->nextBlock){
		if (block->isBottom)
			block->isBottom = false;//原来在文件中这个block是最后一个的，后面申请了一个后就不是最后一个了
		return getBlock(file, block);
	}
	else if (block->nextBlock->offset != block->offset + 1){
		return getBlock(file, block);
		//在这个文件的链表中，有些block可能被删掉了，所以offset中间有些缺失，nextblock并不是文件中真正的后一个block
	}
	else
		return block->nextBlock;
}

BlockNode* BufferManager::getBlockByOffset(FileNode* file, int offset){
	BlockNode *btmp;
	for (btmp = getBlockHead(file); offset; offset--)
		btmp = getNextBlock(file, btmp);
	return btmp;
}

void BufferManager::set_dirty(BlockNode* block){
	block->dirty = true;
}

void BufferManager::set_pin(FileNode *file, bool pin){
	file->pin = pin;
}

void BufferManager::set_pin(BlockNode *block, bool pin){
	block->pin = pin;
}

size_t BufferManager::getUsingSize(BlockNode *block){
	return block->usingSize;
}

void BufferManager::setUsingSize(BlockNode *block, size_t usingSize){
	block->usingSize = usingSize;
	memcpy(block->address, &usingSize, sizeof(size_t));
}

char* BufferManager::getContent(BlockNode* block){
	return (char*)(sizeof(size_t) + block->address);//去掉头信息
}

void BufferManager::delete_fileNode(const char* fileName){
	FileNode* file = getFile(fileName);
	BlockNode* btmp = getBlockHead(file);
	BlockNode* nextemp;
	while (btmp){
		nextemp = btmp->nextBlock;
		refresh_block(*btmp);
		btmp = nextemp;
	}
	refresh_file(*file);

	//把最后一个可用的文件换到这里
	FileNode* ftmp;
	ftmp = &FilePool[FileNum - 1];
	strcpy(file->FileName, ftmp->FileName);
	file->HeadBlock = ftmp->HeadBlock;
	file->pin = ftmp->pin;
	refresh_file(*ftmp);
	FileNum--;
}

int BufferManager::getBlockSize(){
	return BLOCK_SIZE - sizeof(size_t);
}
