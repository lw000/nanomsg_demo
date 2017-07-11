#ifndef __SQLTABLE_H__
#define __SQLTABLE_H__

#include "SQLMgr.h"

#include "lwutil.h"

#include <vector>

class TableBase
{
public:
	virtual void reset() = 0;

public:
	virtual void print() { }
};

class TableConfig : public TableBase
{
public:
	double proportion;			// (工作日/月)
	double meal_supplement;		// (餐补/日)
	double overtime;			// (加班费/日)
	double finance;				// (活动经费/月)
	double work_day;			// (工作日/月)

public:
	TableConfig()
	{
		reset();
	}

	virtual void reset() override
	{
		proportion = 0.0;
		meal_supplement = 0.0;
		overtime = 0.0;
		finance = 0.0;
		work_day = 0.0;
	}

	virtual void print() override
	{
		printf("# proportion : %f, meal_supplement : %f, overtime : %f, finance : %f, work_day : %d\n",
			this->proportion, this->meal_supplement, this->overtime, this->finance, this->work_day);
	}
};

class TableUser : public TableBase
{
public:
	int id;
	int sex;
	int position;
	double wages;
	double average_wages;
	int department;
	std::string name;

public:
	TableUser()
	{
		reset();
	}

public:
	virtual void reset() override
	{
		id = 0;
		sex = 0;
		position = 0;
		wages = 0.0;
		average_wages = 0.0;
		department = 0;
		name.clear();
	}

	virtual void print() override
	{
		fprintf(stdout, "# id : %d, name : %s, sex : %d, position : %d, wages : %.2f, average_wages : %.2f, department : %d\n",
			this->id, this->name.c_str(), this->sex, this->position, this->wages, this->average_wages, this->department);
	}
};

class TableQuotation : public TableBase
{
public:
	std::string name;
	std::string sale_name;
	std::string quotation_number;
	std::string create_time;

public:
	TableQuotation()
	{
		reset();
	}

public:
	virtual void reset() override
	{
		name.clear();
		sale_name.clear();
		quotation_number.clear();
		create_time.clear();
	}

	virtual void print() override
	{
		fprintf(stdout, "# name : %s, sale_name : %s, quotation_number : %s, create_time : %s\n",
			this->name.c_str(), this->sale_name.c_str(), this->quotation_number.c_str(), this->create_time.c_str());
	}
};

////////////////////////////////////////////////////////////////////////////////////////////////////

class SQLTableConfig : public SQLResult
{
public:
	TableConfig config;

public:
	SQLTableConfig(SQLMgr* mgr) : SQLResult(mgr)
	{
	}

	SQLTableConfig(sql::Connection* conn) : SQLResult(conn)
	{
	}

	~SQLTableConfig()
	{
	}

public:
	virtual void onReset() override
	{
		config.reset();
	}

	virtual void onResult(sql::ResultSet* res) override
	{
		uint32_t  c0 = res->findColumn("proportion");
		uint32_t  c1 = res->findColumn("meal_supplement");
		uint32_t  c2 = res->findColumn("overtime");
		uint32_t  c3 = res->findColumn("finance");
		uint32_t  c4 = res->findColumn("work_day");

		int row = res->rowsCount();

		while (res->next())
		{
			config.proportion = res->getDouble(1);
			config.meal_supplement = res->getDouble(2);
			config.overtime = res->getDouble(3);
			config.finance = res->getDouble(4);
			config.work_day = res->getInt(5);
			config.print();
		}
	}

	virtual void onError(const std::string& error) override
	{

	}
};

class SQLTableUser : public SQLResult
{
public:
	std::vector<TableUser> vtUser;

public:
	SQLTableUser(SQLMgr* mgr) : SQLResult(mgr)
	{
	}

	SQLTableUser(sql::Connection* conn) : SQLResult(conn)
	{
	}

	~SQLTableUser()
	{
	}

public:
	virtual void onReset() override
	{
		vtUser.clear();
	}

	virtual void onResult(sql::ResultSet* res) override
	{
		int row = res->rowsCount();

		while (res->next())
		{
			TableUser user;

			user.id = res->getInt("id");
			user.name = utf8_to_gbk(res->getString("name").c_str());
			user.sex = res->getInt("sex");
			user.position = res->getInt("position");
			user.wages = res->getDouble("wages");
			user.average_wages = res->getDouble("average_wages");
			user.department = res->getInt("department");
			
			user.print();

			vtUser.push_back(user);
		}
	}

	virtual void onError(const std::string& error) override
	{

	}

};

class SQLTableQuotation : public SQLResult
{
public:
	std::vector<TableQuotation> vtQuotation;

public:
	SQLTableQuotation(SQLMgr* mgr) : SQLResult(mgr)
	{
	}

	SQLTableQuotation(sql::Connection* conn) : SQLResult(conn)
	{
	}

	~SQLTableQuotation()
	{
	}

public:
	virtual void onReset() override
	{
		vtQuotation.clear();
	}

	virtual void onResult(sql::ResultSet* res) override
	{
		int row = res->rowsCount();

		while (res->next())
		{
			TableQuotation  quotation;
			quotation.name = utf8_to_gbk(res->getString("name").c_str());
			quotation.sale_name = utf8_to_gbk(res->getString("sale_name").c_str());
			quotation.quotation_number = utf8_to_gbk(res->getString("quotation_number").c_str());
			quotation.create_time = utf8_to_gbk(res->getString("create_time").c_str());
			
			quotation.print();

			vtQuotation.push_back(quotation);
		}
	}

	virtual void onError(const std::string& error) override
	{

	}

};


#endif	//__SQLTABLE_H__
