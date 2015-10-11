#ifndef FILEMANAGER_H_H
#define FILEMANAGER_H_H
#define MAXFILELEN  1024
class FileManager
{
public:
	FileManager();
	~FileManager();


	static FileManager& getSingleton(void);

	char* loadFile(const char *file);
	
};

#endif