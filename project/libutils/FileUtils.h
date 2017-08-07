#ifndef __HN_FILEUTILS_H__
#define __HN_FILEUTILS_H__

#include "Data.h"
#include <string>
#include <vector>

class FileUtils final
{
	std::string _defaultResRootPath;

public:
	static FileUtils* getInstance();

public:
	std::string getStringFromFile(const std::string& filename);
	Data getDataFromFile(const std::string& filename);

	int setStringToFile(const std::string& filename, const std::string& data);
	int setDataToFile(const std::string& filename, unsigned char* data, unsigned int len);

public:
	std::vector<std::string> walkDir(const std::string& dir);
	void walkDir(const std::string& dir, std::vector<std::string>& vecFiles);

public:
	std::string getRootPath() const;

	bool isFileExistInternal(const std::string& strFilePath) const;
	bool isDirectoryExistInternal(const std::string& dirPath) const;
		
	virtual bool isFileExist(const std::string& filename) const;

	/**
	*  Checks whether the path is an absolute path.
	*
	*  @note On Android, if the parameter passed in is relative to "assets/", this method will treat it as an absolute path.
	*        Also on Blackberry, path starts with "app/native/Resources/" is treated as an absolute path.
	*
	*  @param strPath The path that needs to be checked.
	*  @return true if it's an absolute path, otherwise it will return false.
	*/
	virtual bool isAbsolutePath(const std::string& path) const;

	/**
	*  Checks whether the path is a directory
	*
	*  @param dirPath The path of the directory, it could be a relative or an absolute path.
	*  @return true if the directory exists, otherwise it will return false.
	*/
	virtual bool isDirectoryExist(const std::string& dirPath);

	/**
	*  Creates a directory
	*
	*  @param dirPath The path of the directory, it must be an absolute path.
	*  @return true if the directory have been created successfully, otherwise it will return false.
	*/
	virtual bool createDirectory(const std::string& dirPath);

	/**
	*  Remove a directory
	*
	*  @param dirPath  The full path of the directory, it must be an absolute path.
	*  @return true if the directory have been removed successfully, otherwise it will return false.
	*/
	virtual bool removeDirectory(const std::string& dirPath);

	/**
	*  Remove a file
	*
	*  @param filepath The full path of the file, it must be an absolute path.
	*  @return true if the file have been removed successfully, otherwise it will return false.
	*/
	virtual bool removeFile(const std::string &filepath);

	/**
	*  Rename a file under the given directory
	*
	*  @param path     The parent directory path of the file, it must be an absolute path.
	*  @param oldname  The current name of the file.
	*  @param name     The new name of the file.
	*  @return true if the file have been renamed successfully, otherwise it will return false.
	*/
	virtual bool renameFile(const std::string &path, const std::string &oldname, const std::string &name);

	/**
	*  Retrieve the file size
	*
	*  @note If a relative path was passed in, it will be inserted a default root path at the beginning.
	*  @param filepath The path of the file, it could be a relative or absolute path.
	*  @return The file size.
	*/
	virtual long getFileSize(const std::string &filepath);
protected:
	FileUtils(void);
	~FileUtils(void);
};

#endif // __HN_FILEUTILS_H__
