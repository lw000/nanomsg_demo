#include "FileUtils.h"
#include <queue>
#include <io.h>
#include <Shlobj.h>
#include <assert.h>
#include <sys/stat.h>
#include <regex>

#define HN_MAX_PATH		260

typedef int(__stdcall *P_WALKDIR_CALLBACK)(std::vector<std::string>& vecFiles, const char *filePath);

// The root path of resources, the character encoding is UTF-8.
// UTF-8 is the only encoding supported by cocos2d-x API.
static std::string s_resourcePath = "";

// D:\aaa\bbb\ccc\ddd\abc.txt --> D:/aaa/bbb/ccc/ddd/abc.txt
static inline std::string convertPathFormatToUnixStyle(const std::string& path)
{
	std::string ret = path;
	int len = ret.length();
	for (int i = 0; i < len; ++i)
	{
		if (ret[i] == '\\')
		{
			ret[i] = '/';
		}
	}
	return ret;
}

static void _checkPath()
{
	if (0 == s_resourcePath.length())
	{
		WCHAR utf16Path[HN_MAX_PATH] = { 0 };
		GetCurrentDirectoryW(sizeof(utf16Path) - 1, utf16Path);

		char utf8Path[HN_MAX_PATH] = { 0 };
		int nNum = ::WideCharToMultiByte(CP_UTF8, 0, utf16Path, -1, utf8Path, sizeof(utf8Path), nullptr, nullptr);

		s_resourcePath = convertPathFormatToUnixStyle(utf8Path);
		s_resourcePath.append("/");
	}
}

static int __stdcall WalkDirCallBack(std::vector<std::string>& vecFiles, const char *filePath)
{
	if (nullptr != filePath)
	{
		vecFiles.push_back(filePath);
	}
	return 0;
}

int WalkDirEx(const char *dir, std::vector<std::string>& vecFiles, P_WALKDIR_CALLBACK pfunCallBack)
{
	int ret = 0;
	std::string root;
	std::queue<std::string> directory;

	if (nullptr == dir || nullptr == pfunCallBack)
	{
		ret = -1;
		return ret;
	}

	root = dir;
	if (root.empty())
	{
		ret = -2;
		return ret;
	}

	char cRootBack = root.back();
	if (cRootBack != '\\' && cRootBack != '/')
	{
		root += '\\';
	}

	directory.push(root);

	do
	{
		std::string strDirForWalk = directory.front();
		directory.pop();

		_finddata_t findData = { 0 };
		intptr_t fileHandle = _findfirst((strDirForWalk + "*").c_str(), &findData);

		if (-1 == fileHandle)
		{
			continue;
		}

		if (strcmp(findData.name, ".") != 0 && strcmp(findData.name, "..") != 0)
		{
			if (findData.attrib & _A_SUBDIR)
			{
				directory.push(strDirForWalk + findData.name + "\\");
			}
			else
			{
				pfunCallBack(vecFiles, (strDirForWalk + findData.name).c_str());
			}
		}

		while (0 == _findnext(fileHandle, &findData))
		{
			if (strcmp(findData.name, ".") != 0 && strcmp(findData.name, "..") != 0)
			{
				if (findData.attrib & _A_SUBDIR)
				{
					directory.push(strDirForWalk + findData.name + "\\");
				}
				else
				{
					//std::transform(std::begin(findData.name), std::end(findData.name), std::begin(findData.name), tolower);
					pfunCallBack(vecFiles, (strDirForWalk + findData.name).c_str());
				}
			}
		}

		if (-1 != fileHandle)
		{
			_findclose(fileHandle);
		}

	} while (!directory.empty());

	return ret;
}

static bool WalkDir(const char* path, std::vector<std::string>& vecFiles)
{
#define BUFFER_LEN	512
	char filePathName[BUFFER_LEN];
	char fullPathName[BUFFER_LEN];
	strcpy_s(filePathName, BUFFER_LEN, path);
	strcat_s(filePathName, BUFFER_LEN, "\\*.*");

	_finddata_t findData = { 0 };
	intptr_t fileHandle = _findfirst(filePathName, &findData);

	if (-1 == fileHandle)
	{
		return 0;
	}

	while (0 == _findnext(fileHandle, &findData))
	{
		if (strcmp(findData.name, ".") == 0 && strcmp(findData.name, "..") == 0)
		{
			continue;
		}

		std::transform(std::begin(findData.name), std::end(findData.name), std::begin(findData.name), tolower);
		sprintf_s(fullPathName, BUFFER_LEN, "%s\\%s", path, findData.name);

		if (findData.attrib & _A_SUBDIR)
		{
			WalkDir(fullPathName, vecFiles);
		}
		else
		{
			vecFiles.push_back(fullPathName);
		}
	}
	return 0;
}

static int writeData(const std::string& filename, unsigned char* data, unsigned int len, bool forString)
{
	if (filename.empty())
	{
		return -1;
	}

	size_t writesize = 0;
	const char* mode = nullptr;

	if (forString)
		mode = "wt+";
	else
		mode = "wb+";

	do
	{
		// Read the file from hardware
		std::string fullPath = filename;
		FILE *fp = nullptr;
		fopen_s(&fp, fullPath.c_str(), mode);
		if (!fp) break;
		fseek(fp, 0, SEEK_SET);
		writesize = fwrite(data, sizeof(unsigned char), len, fp);
		fclose(fp);

	} while (0);

	return writesize;
}

static Data getData(const std::string& filename, bool forString)
{
	if (filename.empty())
	{
		return Data::Null;
	}

	Data ret;
	unsigned char* buffer = nullptr;
	size_t size = 0;
	size_t readsize;
	const char* mode = nullptr;

	if (forString)
		mode = "rt";
	else
		mode = "rb";

	do
	{
		// Read the file from hardware
		std::string fullPath = filename;
		FILE *fp = nullptr;
		fopen_s(&fp, fullPath.c_str(), mode);
		if (!fp) break;
		fseek(fp, 0, SEEK_END);
		size = ftell(fp);
		fseek(fp, 0, SEEK_SET);

		if (forString)
		{
			buffer = (unsigned char*)malloc(sizeof(unsigned char) * (size + 1));
			buffer[size] = '\0';
		}
		else
		{
			buffer = (unsigned char*)malloc(sizeof(unsigned char) * size);
		}

		readsize = fread(buffer, sizeof(unsigned char), size, fp);
		fclose(fp);

		if (forString && readsize < size)
		{
			buffer[readsize] = '\0';
		}
	} while (0);

	if (nullptr == buffer || 0 == readsize)
	{
		std::string msg = "Get data from file(";
		msg.append(filename).append(") failed!");
		printf("%s", msg.c_str());
	}
	else
	{
		ret.fastSet(buffer, readsize);
	}

	return ret;
}

FileUtils* s_FileUtils = nullptr;

FileUtils* FileUtils::getInstance()
{
	if (s_FileUtils == nullptr)
	{
		s_FileUtils = new FileUtils();
	}
	return s_FileUtils;
}

FileUtils::FileUtils(void)
{
}

FileUtils::~FileUtils(void)
{
}

std::string FileUtils::getStringFromFile(const std::string& filename)
{
	Data data = getData(filename, true);
	if (data.isNull())
		return "";

	std::string ret((const char*)data.getBytes());
	return ret;
}

Data FileUtils::getDataFromFile(const std::string& filename)
{
	return getData(filename, false);
}

int FileUtils::setStringToFile(const std::string& filename, const std::string& data)
{
	int r = writeData(filename, (unsigned char*)data.c_str(), data.size(), true);
	return r;
}

int FileUtils::setDataToFile(const std::string& filename, unsigned char* data, unsigned int len)
{
	int r = writeData(filename, data, len, false);
	return r;
}

std::vector<std::string> FileUtils::walkDir(const std::string& dir)
{
	std::vector<std::string> vecFiles;
	walkDir(dir, vecFiles);
	return vecFiles;
}

void FileUtils::walkDir(const std::string& dir, std::vector<std::string>& vecFiles)
{
	WalkDirEx(dir.c_str(), vecFiles, WalkDirCallBack);
}

std::string FileUtils::getRootPath() const
{
	// Get full path of executable, e.g. c:\Program Files (x86)\My Game Folder\MyGame.exe
	char full_path[HN_MAX_PATH + 1];
	::GetModuleFileNameA(nullptr, full_path, HN_MAX_PATH + 1);
#if 1
	{
		std::string ret(full_path);

		ret = ret.substr(0, ret.rfind("\\"));
		ret += "\\";

		// Create directory
		if (SUCCEEDED(SHCreateDirectoryExA(nullptr, ret.c_str(), nullptr)))
		{
			return convertPathFormatToUnixStyle(ret);
		}
	}	
#else
	// Debug app uses executable directory; Non-debug app uses local app data directory
	//#ifndef _DEBUG
	// Get filename of executable only, e.g. MyGame.exe
	char *base_name = strrchr(full_path, '\\');

	if (base_name)
	{
		char app_data_path[HN_MAX_PATH + 1];

		// Get local app data directory, e.g. C:\Documents and Settings\username\Local Settings\Application Data
		if (SUCCEEDED(SHGetFolderPathA(nullptr, CSIDL_LOCAL_APPDATA, nullptr, SHGFP_TYPE_CURRENT, app_data_path)))
		{
			std::string ret((char*)app_data_path);

			// Adding executable filename, e.g. C:\Documents and Settings\username\Local Settings\Application Data\MyGame.exe
			ret += base_name;

			// Remove ".exe" extension, e.g. C:\Documents and Settings\username\Local Settings\Application Data\MyGame
			ret = ret.substr(0, ret.rfind("."));

			ret += "\\";

			// Create directory
			if (SUCCEEDED(SHCreateDirectoryExA(nullptr, ret.c_str(), nullptr)))
			{
				return convertPathFormatToUnixStyle(ret);
			}
		}
	}
	//#endif // not defined _DEBUG
#endif

	// If fetching of local app data directory fails, use the executable one
	std::string ret((char*)full_path);

	// remove xxx.exe
	ret = ret.substr(0, ret.rfind("\\") + 1);

	ret = convertPathFormatToUnixStyle(ret);

	return ret;
}

bool FileUtils::isFileExistInternal(const std::string& strFilePath) const
{
	if (0 == strFilePath.length())
	{
		return false;
	}

	std::string strPath = strFilePath;
	if (!isAbsolutePath(strPath))
	{ // Not absolute path, add the default root path at the beginning.
		strPath.insert(0, _defaultResRootPath);
	}

	WCHAR utf16Buf[HN_MAX_PATH] = { 0 };
	::MultiByteToWideChar(CP_UTF8, 0, strPath.c_str(), -1, utf16Buf, sizeof(utf16Buf) / sizeof(utf16Buf[0]));

	DWORD attr = GetFileAttributesW(utf16Buf);
	if (attr == INVALID_FILE_ATTRIBUTES || (attr & FILE_ATTRIBUTE_DIRECTORY))
		return false;   //  not a file
	return true;
}

bool FileUtils::isAbsolutePath(const std::string& strPath) const
{
	if (strPath.length() > 2
		&& ((strPath[0] >= 'a' && strPath[0] <= 'z') || (strPath[0] >= 'A' && strPath[0] <= 'Z'))
		&& strPath[1] == ':')
	{
		return true;
	}
	return false;
}

bool FileUtils::isFileExist(const std::string& filename) const
{
	if (isAbsolutePath(filename))
	{
		return isFileExistInternal(filename);
	}
	else
	{
		return false;
	}
}

bool FileUtils::isDirectoryExistInternal(const std::string& dirPath) const
{
	unsigned long fAttrib = GetFileAttributesA(dirPath.c_str());
	if (fAttrib != INVALID_FILE_ATTRIBUTES &&
		(fAttrib & FILE_ATTRIBUTE_DIRECTORY))
	{
		return true;
	}
	return false;
}

bool FileUtils::isDirectoryExist(const std::string& dirPath)
{
	assert(!dirPath.empty());

	if (isAbsolutePath(dirPath))
	{
		return isDirectoryExistInternal(dirPath);
	}

	return false;
}

bool FileUtils::createDirectory(const std::string& path)
{
	assert(!path.empty());

	if (isDirectoryExist(path))
		return true;

	// Split the path
	size_t start = 0;
	size_t found = path.find_first_of("/\\", start);
	std::string subpath;
	std::vector<std::string> dirs;

	if (found != std::string::npos)
	{
		while (true)
		{
			subpath = path.substr(start, found - start + 1);
			if (!subpath.empty())
				dirs.push_back(subpath);
			start = found + 1;
			found = path.find_first_of("/\\", start);
			if (found == std::string::npos)
			{
				if (start < path.length())
				{
					dirs.push_back(path.substr(start));
				}
				break;
			}
		}
	}

	if ((GetFileAttributesA(path.c_str())) == INVALID_FILE_ATTRIBUTES)
	{
		subpath = "";
		for (unsigned int i = 0; i < dirs.size(); ++i)
		{
			subpath += dirs[i];
			if (!isDirectoryExist(subpath))
			{
				BOOL ret = CreateDirectoryA(subpath.c_str(), NULL);
				if (!ret && ERROR_ALREADY_EXISTS != GetLastError())
				{
					return false;
				}
			}
		}
	}
	return true;
}

bool FileUtils::removeDirectory(const std::string& path)
{
	if (path.size() > 0 && path[path.size() - 1] != '/')
	{
		printf("Fail to remove directory, path must termniate with '/': %s", path.c_str());
		return false;
	}

	std::string command = "cmd /c rd /s /q ";
	// Path may include space.
	command += "\"" + path + "\"";

	if (WinExec(command.c_str(), SW_HIDE) > 31)
		return true;
	else
		return false;
}

bool FileUtils::removeFile(const std::string &path)
{
	
	std::string command = "cmd /c del /q ";
	std::string win32path = path;
	int len = win32path.length();
	for (int i = 0; i < len; ++i)
	{
		if (win32path[i] == '/')
		{
			win32path[i] = '\\';
		}
	}
	command += win32path;

	if (WinExec(command.c_str(), SW_HIDE) > 31)
		return true;
	else
		return false;
}

bool FileUtils::renameFile(const std::string &path, const std::string &oldname, const std::string &name)
{
	assert(!path.empty());
	std::string oldPath = path + oldname;
	std::string newPath = path + name;

	std::regex pat("\\/");
	std::string _old = std::regex_replace(oldPath, pat, "\\");
	std::string _new = std::regex_replace(newPath, pat, "\\");

	if (FileUtils::getInstance()->isFileExist(_new))
	{
		DeleteFileA(_new.c_str());
	}

	MoveFileA(_old.c_str(), _new.c_str());

	if (0 == GetLastError())
		return true;
	else
		return false;
}

long FileUtils::getFileSize(const std::string &filepath)
{
	assert(!filepath.empty());

	std::string fullpath = filepath;
	if (!isAbsolutePath(filepath))
	{
		return 0;
	}

	struct stat info;
	// Get data associated with "crt_stat.c":
	int result = stat(fullpath.c_str(), &info);

	// Check if statistics are valid:
	if (result != 0)
	{
		// Failed
		return -1;
	}
	else
	{
		return (long)(info.st_size);
	}
}
