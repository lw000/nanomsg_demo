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

sql::Driver* SQLMgr::getDriver()
{
	return _driver;
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

SQLResult::SQLResult(SQLMgr* mgr) : _mgr(mgr), _res(NULL), _stmt(NULL), _pstmt(NULL)
{
	
}

SQLResult::~SQLResult()
{
	
}


bool SQLResult::createStatement()
{
	try
	{
		if (_stmt == NULL)
		{
			_stmt = _mgr->getConnection()->createStatement();
		}
	}
	catch (sql::SQLException& e)
	{
		std::string s = __catch_info(e);

		onError(s.c_str());
	}

	return (_stmt != NULL);
}


bool SQLResult::execute()
{

}

bool SQLResult::execute(const std::string& sql)
{

}

bool SQLResult::executeQuery(const std::string& sqlString)
{
	try
	{	
		if (_stmt == NULL)
		{
			createStatement();
		}

		_res = _stmt->executeQuery(sqlString);
		
		int row = _res->rowsCount();

		onResult(_res);
	}
	catch (sql::SQLException& e)
	{
		std::string s = __catch_info(e);

		onError(s.c_str());
	}

	return (_res != NULL);
}


bool SQLResult::prepareStatement(const std::string& sql)
{
	try
	{
		if (_pstmt != NULL)
		{
			delete _pstmt;
			_pstmt = NULL;
		}
		_pstmt = _mgr->getConnection()->prepareStatement(sql);
	}
	catch (sql::SQLException& e)
	{
		std::string s = __catch_info(e);

		onError(s.c_str());
	}

	return _pstmt != NULL;
}

bool SQLResult::executeQuery()
{
	try
	{
		_res = _pstmt->executeQuery();

		int row = _res->rowsCount();

		onResult(_res);
	}
	catch (sql::SQLException& e)
	{
		std::string s = __catch_info(e);

		onError(s.c_str());
	}

	return (_res != NULL);
}

void SQLResult::reset()
{
	if (_res != NULL)
	{
		delete _res;
		_res = NULL;
	}

	if (_stmt != NULL)
	{
		delete _stmt;
		_stmt = NULL;
	}

	if (_pstmt != NULL)
	{
		delete _pstmt;
		_pstmt = NULL;
	}

	onReset();
}

void SQLResult::setBigInt(unsigned int parameterIndex, const std::string& value)
{
	if (_pstmt != NULL)
	{
		_pstmt->setBigInt(parameterIndex, value);
	}
}

void SQLResult::setBlob(unsigned int parameterIndex, std::istream * blob)
{
	if (_pstmt != NULL)
	{
		_pstmt->setBlob(parameterIndex, blob);
	}
}

void SQLResult::setBoolean(unsigned int parameterIndex, bool value)
{
	if (_pstmt != NULL)
	{
		_pstmt->setBoolean(parameterIndex, value);
	}
}

void SQLResult::setDateTime(unsigned int parameterIndex, const std::string& value)
{
	if (_pstmt != NULL)
	{
		_pstmt->setDateTime(parameterIndex, value);
	}
}

void SQLResult::setDouble(unsigned int parameterIndex, double value)
{
	if (_pstmt != NULL)
	{
		_pstmt->setDouble(parameterIndex, value);
	}
}

void SQLResult::setInt(unsigned int parameterIndex, int32_t value)
{
	if (_pstmt != NULL)
	{
		_pstmt->setInt(parameterIndex, value);
	}
}

void SQLResult::setUInt(unsigned int parameterIndex, uint32_t value)
{
	if (_pstmt != NULL)
	{
		_pstmt->setUInt(parameterIndex, value);
	}
}

void SQLResult::setInt64(unsigned int parameterIndex, int64_t value)
{
	if (_pstmt != NULL)
	{
		_pstmt->setInt64(parameterIndex, value);
	}
}

void SQLResult::setUInt64(unsigned int parameterIndex, uint64_t value)
{
	if (_pstmt != NULL)
	{
		_pstmt->setUInt64(parameterIndex, value);
	}
}

void SQLResult::setNull(unsigned int parameterIndex, int sqlType)
{
	if (_pstmt != NULL)
	{
		_pstmt->setNull(parameterIndex, sqlType);
	}
}

void SQLResult::setString(unsigned int parameterIndex, const std::string& value)
{
	if (_pstmt != NULL)
	{
		_pstmt->setString(parameterIndex, value);
	}
}

