#include "SQLConnPool.h"



class AutoLock
{
public:
	AutoLock(pthread_mutex_t* lock) : _lock(lock)
	{
		pthread_mutex_lock(_lock);
	}

	~AutoLock()
	{
		pthread_mutex_unlock(_lock);
	}

private:
	pthread_mutex_t* _lock;
};


static std::string __catch_info(sql::SQLException& e)
{
	std::string s;
	{
		char buf[2048];
		int c = 0;
		c = c + sprintf(&buf[c], "# ERR: SQLException in %s \n", __FILE__);
		c = c + sprintf(&buf[c], "# (%s) on line (%s) \n", __FUNCTION__, __FILE__);
		c = c + sprintf(&buf[c], "# ERR: (%s) \n", e.what());
		c = c + sprintf(&buf[c], "# (MySQL error code : %d), SQLState: (%s) \n", e.getErrorCode(), e.getSQLState().c_str());

		printf(buf);

		s.append(buf);
	}

	return s;
}

SQLConnPool* SQLConnPool::getInstance()
{
	static SQLConnPool pool;
	return &pool;
}

SQLConnPool::SQLConnPool() : _driver(NULL), _maxsize(0)
{
	pthread_mutex_init(&this->_lock, NULL);
}

SQLConnPool::~SQLConnPool()
{
	pthread_mutex_destroy(&this->_lock);
}

int SQLConnPool::createConnPool(const std::string& hostName, const std::string& userName,
	const std::string& password, const std::string& schema, int maxsize)
{
	this->_maxsize = maxsize;
	this->_hostname = hostName;
	this->_username = userName;
	this->_password = password;
	this->_schema = schema;

	int r = -1;

	try 
	{
		this->_driver = sql::mysql::get_driver_instance();
		this->initConnection(maxsize / 2);
		r = 0;
	}
	catch (sql::SQLException&e)
	{
		perror("驱动连接出错;\n");
	}
	catch (std::runtime_error&e)
	{
		perror("运行出错了\n");
	}

	return r;
}

sql::Connection* SQLConnPool::getConnection()
{
	sql::Connection*con = NULL;

	AutoLock a(&this->_lock);

	if (_work_pool.size() > 0)		// 连接池容器中还有连接
	{
		con = this->_work_pool.front();
		this->_work_pool.pop_front();
		
		if (con->isClosed()) // 如果连接已经被关闭，删除后重新建立一个
		{
			delete con;
			con = this->createConnection();
		}

		if (con != NULL)
		{
			this->_working_pool.push_back(con);
		}

		return con;
	}
	else
	{
		if (this->_working_pool.size() < _maxsize) // 还可以创建新的连接
		{ 
			con = this->createConnection();
			if (con)
			{
				this->_working_pool.push_back(con);
				return con;
			}
			else
			{
				return NULL;
			}
		}
		else // 建立的连接数已经达到 maxsize
		{
			return NULL;
		}
	}

	return NULL;
}

sql::Connection* SQLConnPool::createConnection()
{
	sql::Connection* conn = NULL;
	try
	{
		conn = this->_driver->connect(this->_hostname, this->_username, this->_password);
		conn->setSchema(_schema);
	}
	catch (sql::SQLException& e)
	{
		__catch_info(e);
	}

	return conn;
}

int SQLConnPool::initConnection(int maxConnectionCount)
{
	{
		AutoLock a(&this->_lock);
		sql::Connection*conn = NULL;
		for (int i = 0; i < maxConnectionCount; i++)
		{
			conn = this->createConnection();
			if (conn)
			{
				this->_work_pool.push_back(conn);
			}
			else
			{
				perror("创建CONNECTION出错");
			}
		}
	}
	return 0;
}
 
void SQLConnPool::recycleConnection(sql::Connection * conn)
{
	if (conn)
	{
		AutoLock a(&this->_lock);
		this->_working_pool.remove_if([conn](sql::Connection * con)
		{
			return (conn == con);
		});
		this->_work_pool.push_back(conn);
	}
}

void SQLConnPool::destoryConnPool()
{
	{
		AutoLock a(&_lock);
		{
			std::list<sql::Connection*>::iterator iter_conn;
			for (iter_conn = this->_work_pool.begin(); iter_conn != this->_work_pool.end(); ++iter_conn)
			{
				this->destoryConnection(*iter_conn);
			}
			this->_work_pool.clear();
		}

		{
			std::list<sql::Connection*>::iterator iter_conn;
			for (iter_conn = this->_working_pool.begin(); iter_conn != this->_working_pool.end(); ++iter_conn)
			{
				this->destoryConnection(*iter_conn);
			}
			this->_working_pool.clear();
		}
	}
}

void SQLConnPool::destoryConnection(sql::Connection* conn) 
{
	if (conn)
	{
		try
		{
			conn->close();
		}
		catch (sql::SQLException&e)
		{
			perror(e.what());
		}
		catch (std::exception&e)
		{
			perror(e.what());
		}
		delete conn;
	}
}