#ifndef __SQLConnPool_H__
#define __SQLConnPool_H__

#include <mysql_connection.h>
#include <mysql_driver.h>
#include <mysql_error.h>
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/metadata.h>
#include <cppconn/prepared_statement.h>

#include <string>
#include <list>
#include <pthread.h>

class SQLConnPool final
{
public:
	class CONNECT
	{
	public:
		CONNECT()
		{
			_conn = SQLConnPool::getInstance()->getConnection();
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

public:
	int createConnPool(const std::string& hostName, const std::string& userName, const std::string& password,
		const std::string& schema, int maxsize = 10);
	void destoryConnPool();

public:
	sql::Connection* getConnection();
	void recycleConnection(sql::Connection * conn);

private:
	int initConnection(int maxConnectionCount);
	sql::Connection* createConnection();
	void destoryConnection(sql::Connection* conn);

private:
	sql::Driver* _driver;

	std::list < sql::Connection* > _work_pool;
	std::list < sql::Connection* > _working_pool;

private:
	int _maxsize;
	pthread_mutex_t _lock;

private:
	std::string _hostname;
	std::string _username;
	std::string _password;
	std::string _schema;
};

#endif