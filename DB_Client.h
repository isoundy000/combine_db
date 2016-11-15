#ifndef DB_CLIENT_H
#define DB_CLIENT_H

#include "DB_Connector.h"
#include "Common.h"

class DB_Client {
public:
	typedef std::vector<DB_Connector *> DB_CONNECTOR_VEC;
	
	DB_Client();
	virtual ~DB_Client(){}
	virtual int connect() = 0;
	virtual int process_table(std::string table) = 0;

	void start_thread();
	void wait_sub_thread();
protected:
	int last_offset_;
	DB_Connector *db_src_con_;
	DB_CONNECTOR_VEC db_con_vec_;
};

class Mongo_Client : public DB_Client {
public:
	Mongo_Client();
	~Mongo_Client();

	virtual int connect();
	virtual int process_table(std::string table);
private:
};

class Mysql_Client : public DB_Client {
public:
	Mysql_Client();
	~Mysql_Client();

	virtual int connect();
	virtual int process_table(std::string table);
private:
};

#endif

