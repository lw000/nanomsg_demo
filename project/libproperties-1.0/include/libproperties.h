/*
  Project: libproperties
  Written by harite
  mailto:Harite.K@gmail.com
  date:2007/10/05
  libproperties's homepage:
    http://code.google.com/p/libproperties

  libproperties is released under the zlib license:

This software is provided 'as-is', without any express or implied 
warranty. In no event will the authors be held liable for any 
damages arising from the use of this software.

Permission is granted to anyone to use this software for any 
purpose, including commercial applications, and to alter it and 
redistribute it freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must 
not claim that you wrote the original software. If you use this 
software in a product, an acknowledgment in the product documentation 
would be appreciated but is not required.

2. Altered source versions must be plainly marked as such, and 
must not be misrepresented as being the original software.

3. This notice may not be removed or altered from any source 
distribution.

*/

#ifndef LIBPROPERTIES_H
#define LIBPROPERTIES_H

/**
*
* 类似java中的类Properties，其xml文件格式一致。
* we give a c++ class as is class "Properties" in java
* And they can use the xml file each other.
* The lib have only one class, so You can use the lib
* very simple.
*
* Authors: harite  Harite.K@gmail.com
* DataTime: 2007/10/03
* Project: http://code.google.com/p/libproperties
*/

//#include <map>
#include <unordered_map>
#include <string>

//using std::map;
using std::unordered_map;
using std::string;

/*
 * 实际上tinyxml2.4.3对线程是不安全的
*/
// tinyxml2.4.3 is NOT thread safe in fact, so ...
//#define WIN32_THREAD_SAFE
/*
#ifdef WIN32_THREAD_SAFE
#include <windows.h>
#endif
*/

//如果想要编译成dll，使用时需要一个导出库lib
/*
 if you want a dll in win32, this macro can help you.
 it will create a dll and its export lib.
*/
#ifdef LIBPROPERTIES_BUILDING_DLL

#include <windows.h>
#define LIBPROPERTIES_DLLIMPORT __declspec (dllexport)

#else /* Not DLL */

//#define LIBPROPERTIES_DLLIMPORT __declspec (dllimport)
#define LIBPROPERTIES_DLLIMPORT

#endif

// 使用stl
// use the stl for tinyxml
#ifndef TIXML_USE_STL
#define TIXML_USE_STL
#endif

#include "tinyxml.h"
#include "libproperties_tinyxml_ext.h"

// 调试
// just for debug
//#define DEBUG_PROPERTIES_H

#ifdef DEBUG_PROPERTIES_H
#include <iostream>
#endif

using namespace std;

/*
 libproperties供外界使用的只有Properties类
 因此，你可以很简单的使用它。
 The libproperties has only one class, so You can use the lib
 very simple.
*/
class LIBPROPERTIES_DLLIMPORT Properties
{
public:

// 	typedef map<string, string>::iterator iterator;
// 	typedef map<string, string>::const_iterator const_iterator;

	typedef unordered_map<string, string>::iterator iterator;
	typedef unordered_map<string, string>::const_iterator const_iterator;

	Properties(void)
	{
		/*
		#ifdef WIN32_THREAD_SAFE
	    InitializeCriticalSection(&CriticalSection);
        #endif
		*/
	}

	virtual ~Properties(void)
	{
		/*
		#ifdef WIN32_THREAD_SAFE
	    DeleteCriticalSection(&CriticalSection);
        #endif
		*/
	}

	/**
	* 设置一个“键－值”对,若键已经存在则将其替换成新值
	* set a "key-value" pair, if the key exist, update its value
	* @param key 要设置的键 the key to set
	* @param value 要设置的值 the value to set
	*/
	inline void setProperty(string key, string value)
	{
		iterator it = key_value.find(key);
		/*
		#ifdef WIN32_THREAD_SAFE
		EnterCriticalSection(&CriticalSection);
        #endif
		*/
		if(it!=key_value.end())//对于已经存在的
		{
			(*it).second = value;
		}
		else
		{
			key_value[key] = value;
		}
		/*
		#ifdef WIN32_THREAD_SAFE
		LeaveCriticalSection(&CriticalSection);
        #endif
		*/
	}

	/**
	* 读取一个键的值，如果不存在，返回默认值defaultvalue
	* get value of a key, if the key is inexistence then return default value
	* @param key 想要获得值的键 the key of you want value
	* @param defaultvalue 如果键不存在则返回此参数 the defaultvalue
	* @return 若键存在，返回其对应的值；若不存在，返回defaultvalue
	*/
	inline string getProperty(string key, string defaultvalue) const
	{
		const_iterator it = key_value.find(key);
		if(it==key_value.end())
		{
			return defaultvalue;
		}
		else
		{
			return (*it).second;
		}
	}

	/**
	* 从xml文件中读取“键－值”对 load "key-value" from xml file
	* @param filename 读取的xml文件，必须是按照规定的格式
	* @return 布尔值 读取成功与否
	*/
	inline bool loadFromXML(const string filename)
	{
		TiXmlDocument doc(filename.c_str());
		bool loadOkay = doc.LoadFile();//以utf-8格式读取xml文件
		if (!loadOkay)
		{
			return false;
		}
		TiXmlNode *node = doc.FirstChild("properties");
        #ifdef DEBUG_PROPERTIES_H
		node->Print(stdout, 1);
		cout << endl;
        #endif
		TiXmlElement* propertiesElement = node->ToElement();
		for(TiXmlElement *it = propertiesElement->FirstChildElement("entry")
			; it!=NULL ;
			it = it->NextSiblingElement("entry")
			)
		{
			TiXmlAttribute *att = it->FirstAttribute();
			this->setProperty(att->Value(), it->GetText());
			#ifdef DEBUG_PROPERTIES_H
			cout << "[" << att->Name() << ":" << att->Value() << "->" << it->GetText() << "]" << endl;
            #endif
		}
		return true;
	}

	/**
	* 将键值对按照类似java中Properties类的xml格式存储 store "key-value" to xml file
	* @param filename 要保存的xml文件名
	* @return 布尔值,代表操作执行成功与否
	*/
	inline bool storeToXML(const string filename) const
	{
		TiXmlDocument doc;
		
		TiXmlDeclarationWithoutStandalone *decl = 
			new TiXmlDeclarationWithoutStandalone("1.0", "UTF-8");
		doc.LinkEndChild(decl);

		TiXmlDoctype *doctype = new TiXmlDoctype();
		doctype->SetValue("properties SYSTEM \"http://java.sun.com/dtd/properties.dtd\"");
		doc.LinkEndChild(doctype);
		
		TiXmlComment *comment1 = new TiXmlComment();
		comment1->SetValue("This file is created by libproperties");
		doc.LinkEndChild(comment1);
		
		TiXmlComment *comment2 = new TiXmlComment();
		comment2->SetValue("read more from http://code.google.com/p/libproperties");
		doc.LinkEndChild(comment2);

		TiXmlElement *propertiesElement = new TiXmlElement("properties");
		doc.LinkEndChild(propertiesElement);
 
		for (Properties::const_iterator it = key_value.begin()
			; it!=key_value.end() ;
			++it
			)
		{
			TiXmlElement *entryElement = new TiXmlElement("entry");
			entryElement->LinkEndChild(new TiXmlText((*it).second));
			#ifdef DEBUG_PROPERTIES_H
		    propertiesElement->Print(stdout, 1);
		    cout << endl;
            #endif
			propertiesElement->LinkEndChild(entryElement);
			entryElement->SetAttribute("key", (*it).first);
		}
		
		bool result = doc.SaveFile(filename);
		doc.Clear();
		return result;
	}

	/**
	* 清空键值对 clear "key-value"
	*/
	inline void clear()
	{
		/*
		#ifdef WIN32_THREAD_SAFE
		EnterCriticalSection(&CriticalSection);
        #endif
		*/
		key_value.clear();
		/*
		#ifdef WIN32_THREAD_SAFE
		LeaveCriticalSection(&CriticalSection);
        #endif
		*/
	}

	/**
	* 返回指向第一对键值的iterator
	* return the const_iterator of first "key-value" pair
	*/
	inline const_iterator begin() const
	{
		return key_value.begin();
	}

	/**
	* 返回指向最后一对键值之后的iterator
	* return the next const_iterator of end "key-value" pair
	*/
	inline const_iterator end() const
	{
		return key_value.end();
	}

private:
	/**
	* stl中的map，盛放键值对
	*/
//	map<string, string> key_value;
	unordered_map<string, string> key_value;

	/**
	* 临界区
	*/
	/*
	#ifdef WIN32_THREAD_SAFE
	CRITICAL_SECTION CriticalSection;
    #endif
	*/
};

#endif

