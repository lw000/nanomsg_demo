#include "SQLMgr.h"

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

SQLMgr::SQLMgr()
{
	_driver = (sql::Driver*)get_driver_instance();
}

SQLMgr::~SQLMgr()
{
	delete _con;
}

bool SQLMgr::connect(const std::string& hostName, const std::string& userName, const std::string& password)
{
	try
	{
		_con = _driver->connect(hostName, userName, password);
	}
	catch (sql::SQLException& e)
	{
		__catch_info(e);
	}

	return _con != NULL;
}

bool SQLMgr::connect(const std::string& hostName, const std::string& userName, const std::string& password, const std::string& schema)
{
	try
	{
		sql::ConnectOptionsMap connection_properties;
		connection_properties["hostName"] = hostName;
		connection_properties["userName"] = userName;
		connection_properties["password"] = password;
		connection_properties["schema"] = schema;
		connection_properties["port"] = 3306;
		connection_properties["OPT_RECONNECT"] = true;
		_con = _driver->connect(connection_properties);
	}
	catch (sql::SQLException& e)
	{
		__catch_info(e);
	}

	return _con != NULL;
}

void SQLMgr::useSchema(const std::string& schema)
{
	if (_con == NULL) return;
	if (_con->isClosed()) return;
	
	_con->setSchema(schema);
}

sql::Connection* SQLMgr::getConnection()
{
	return _con;
}

void SQLMgr::print()
{
	if (_con == NULL) return;
	if (_con->isClosed()) return;

	sql::DatabaseMetaData *dbMeta = _con->getMetaData();
	printf("Database Product Name: %s \n", dbMeta->getDatabaseProductName().c_str());
	printf("Database Product Version: %s \n", dbMeta->getDatabaseProductVersion().c_str());
	printf("Database User name: %s \n", dbMeta->getUserName().c_str());
}

////////////////////////////////////////////////////////////////////////////////////////////////////

SQLResult::SQLResult(SQLMgr* mgr) : _mgr(mgr), _res(nullptr), _stmt(nullptr), _pstmt(nullptr)
{
	this->_onResultFunc = nullptr;
	this->_onErrorFunc = nullptr;

	if (_conn == NULL)
	{
		_conn = _mgr->getConnection();
	}
}

SQLResult::SQLResult(sql::Connection* conn) : _mgr(NULL), _conn(conn), _res(nullptr), _stmt(nullptr), _pstmt(nullptr)
{

}

SQLResult::~SQLResult()
{
	
}

sql::ResultSet* SQLResult::getResultSet() const
{
	return _res;
}

sql::Statement* SQLResult::getStatement() const
{
	return _stmt;
}

sql::PreparedStatement* SQLResult::getPreparedStatement() const
{
	return _pstmt;
}

bool SQLResult::createStatement()
{
	try
	{
		if (_stmt == nullptr)
		{
			if (_conn == NULL)
			{
				_conn = _mgr->getConnection();
			}
			_stmt = _conn->createStatement();
		}
	}
	catch (sql::SQLException& e)
	{
		std::string s = __catch_info(e);
		onError(s.c_str());
	}

	return (_stmt != nullptr);
}

bool SQLResult::executeQuery(const std::string& sqlString, std::function<void(sql::ResultSet*)> onResultFunc,
	std::function<void(const std::string&)> onErrorFunc)
{
	try
	{	
		this->_onResultFunc = onResultFunc;
		this->_onErrorFunc = onErrorFunc;

		if (_stmt == nullptr)
		{
			createStatement();
		}

		_res = _stmt->executeQuery(sqlString);
		
		if (this->_onResultFunc != nullptr)
		{
			this->_onResultFunc(_res);
		}

		onResult(_res);
	}
	catch (sql::SQLException& e)
	{
		std::string s = __catch_info(e);
		
		if (this->_onErrorFunc != NULL)
		{
			this->_onErrorFunc(s);
		}

		onError(s.c_str());	
	}

	return (_res != nullptr);
}


sql::PreparedStatement* SQLResult::prepareStatement(const std::string& sql)
{
	try
	{
		if (_pstmt != nullptr)
		{
			delete _pstmt;
			_pstmt = nullptr;
		}
		if (_conn == NULL)
		{
			_conn = _mgr->getConnection();
		}
		_pstmt = _conn->prepareStatement(sql);
	}
	catch (sql::SQLException& e)
	{
		std::string s = __catch_info(e);
		onError(s.c_str());
	}

	return (_pstmt);
}

bool SQLResult::executeQuery(std::function<void(sql::ResultSet*)> onResultFunc,
	std::function<void(const std::string&)> onErrorFunc)
{
	try
	{
		this->_onResultFunc = onResultFunc;
		this->_onErrorFunc = onErrorFunc;

		_res = _pstmt->executeQuery();

		int row = _res->rowsCount();

		if (this->_onResultFunc != nullptr)
		{
			this->_onResultFunc(_res);
		}

		onResult(_res);
	}
	catch (sql::SQLException& e)
	{
		std::string s = __catch_info(e);
		
		if (this->_onErrorFunc != nullptr)
		{
			this->_onErrorFunc(s);
		}

		onError(s.c_str());
	}

	return (_res != nullptr);
}

void SQLResult::reset()
{
 	this->_onResultFunc = nullptr;
 	this->_onErrorFunc = nullptr;

	if (_res != nullptr)
	{
		delete _res;
		_res = nullptr;
	}

	if (_stmt != nullptr)
	{
		delete _stmt;
		_stmt = nullptr;
	}

	if (_pstmt != nullptr)
	{
		delete _pstmt;
		_pstmt = nullptr;
	}

	onReset();
}
