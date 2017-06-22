#ifndef __SQLMGR_H__
#define __SQLMGR_H__

#include "mysql_connection.h"
#include "mysql_driver.h"
#include "mysql_error.h"
#include "cppconn/driver.h"
#include "cppconn/exception.h"
#include "cppconn/resultset.h"
#include "cppconn/statement.h"
#include "cppconn/metadata.h"
#include "cppconn/prepared_statement.h"

class SQLResult;
class SQLMgr;

class SQLMgr
{
public:
	SQLMgr();
	~SQLMgr();

public:
	bool connect(const std::string& hostName, const std::string& userName, const std::string& password);
	void useSchema(const std::string& schema);
	sql::Connection* getConnection();

	void print();

private:
	sql::mysql::MySQL_Driver* _driver;
	sql::Connection* _con;
};

class SQLResult
{
public:
	SQLResult(SQLMgr* mgr);
	~SQLResult();

public:
	void reset();

public:
	bool createStatement();
	bool executeQuery(const std::string& sqlString);

public:
	bool executeQuery();
	bool prepareStatement(const std::string& sql);

public:
	void setBigInt(unsigned int parameterIndex, const std::string& value);
	void setBlob(unsigned int parameterIndex, std::istream * blob);
	void setBoolean(unsigned int parameterIndex, bool value);
	void setDateTime(unsigned int parameterIndex, const std::string& value);
	void setDouble(unsigned int parameterIndex, double value);
	void setInt(unsigned int parameterIndex, int32_t value);
	void setUInt(unsigned int parameterIndex, uint32_t value);
	void setInt64(unsigned int parameterIndex, int64_t value);
	void setUInt64(unsigned int parameterIndex, uint64_t value);
	void setNull(unsigned int parameterIndex, int sqlType);
	void setString(unsigned int parameterIndex, const std::string& value);

public:
	virtual void onReset() = 0;
	virtual void onResult(sql::ResultSet* res) = 0;
	virtual void onError(const std::string& error) = 0;

private:


private:
	SQLMgr* _mgr;
	sql::ResultSet* _res;
	sql::Statement * _stmt;
	sql::PreparedStatement *_pstmt;
};


#endif	//__SQLMGR_H__
