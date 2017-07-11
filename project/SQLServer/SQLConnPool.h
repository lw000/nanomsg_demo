#ifndef __SQLConnPool_H__
#define __SQLConnPool_H__

#include "mysql_connection.h"
#include "mysql_driver.h"
#include "mysql_error.h"
#include "cppconn/driver.h"
#include "cppconn/exception.h"
#include "cppconn/resultset.h"
#include "cppconn/statement.h"
#include "cppconn/metadata.h"
#include "cppconn/prepared_statement.h"

#include <list>

#include <pthread.h>

class SQLConnPool
{
public:
	class CONNECT
	{
	public:
		CONNECT()
		{
			_conn = SQLConnPool::getInstance()->getGetConnection();
			_conn->getDriver()->threadInit();
		}

		~CONNECT()
		{
			SQLConnPool::getInstance()->recycleConnection(_conn);
			_conn->getDriver()->threadEnd();
		}

		sql::Connection* operator ->()
		{
			return _conn;
		}

		sql::Connection* operator &()
		{
			return _conn;
		}

	public:
		void* operator new(size_t size)
		{
			throw("new error");
			return NULL;
		}
			
		void operator delete(void* pp) {}

	private:
		sql::Connection* _conn;
	};

public:
	SQLConnPool();
	~SQLConnPool();

public:
	static SQLConnPool* getInstance();
	int createSqlConnPool(const std::string& hostName, const std::string& userName, const std::string& password,
		const std::string& schema, int maxsize);
	void destoryConnPool();

public:
	sql::Connection* getGetConnection();
	void recycleConnection(sql::Connection * conn);

private:
	int initConnection(int maxConnectionCount);
	sql::Connection* createConnection();
	void destoryConnection(sql::Connection* conn);

public:
	sql::Driver* _driver;
	std::list < sql::Connection* > _work_pool;
	std::list < sql::Connection* > _working_pool;
	int _maxsize;
	pthread_mutex_t _lock;
	std::string _hostname;
	std::string _username;
	std::string _password;
	std::string _schema;
};

#endif