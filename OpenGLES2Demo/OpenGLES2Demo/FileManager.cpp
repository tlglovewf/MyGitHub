#include "FileManager.h"
#include <fstream>
#include <assert.h>

FileManager::FileManager()
{
}


FileManager::~FileManager()
{
}

FileManager& FileManager::getSingleton(void)
{
	static FileManager singleton;
	return singleton;
}

char* FileManager::loadFile(const char *file)
{
	assert(NULL != file);
	char fullpath[MAXFILELEN] = { 0 };
	sprintf_s(fullpath, "%s\\%s", "./", file);
	static char *info = NULL;
	std::fstream fs;
	try
	{
		fs.open(fullpath, std::ios::in | std::ios::ate);

		if (!fs.is_open())
		{
			printf("%d", fs.goodbit);
			return NULL;
		}
		int len = fs.tellg();
		info = new char[len];
		memset(info, 0, len);
		fs.seekg(std::ios::beg);
		fs.read(info, len);
	}
	catch (...)
	{
		if (NULL != info)
		{
			delete info;
			info = NULL;
		}
		fs.close();
	}
	fs.close();
	return info;
}
