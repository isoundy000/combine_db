#ifndef DB_CONNECTOR_H
#define DB_CONNECTOR_H

#include <semaphore.h>
#include <pthread.h>
#include "Byte_Buffer.h"
#include "Common.h"
#include "Config.h"

struct DB_Query_Result {
	ResultSet *mysql_result;
	std::auto_ptr<DBClientCursor> mongo_result;
};

class DB_Connector {
public:
	typedef std::list<Buffer_Wrap> DATA_LIST;
	
	DB_Connector();
	virtual ~DB_Connector(){}
	virtual int connect_to_db(const char *ip, int port, const char *user, const char *passwd){
								DEBUG_LOG("connect_to_db should implement in subclass");
								return -1;
							}
	virtual int get_record_count(const char *table_name){DEBUG_LOG("get_record_count should implement in subclass");return -1;}
	virtual DB_Query_Result read_from_db(const char *table_name, int last_offset, int limit){ 
		DEBUG_LOG("read_from_db should implement in subclass");
		DB_Query_Result result;
		return result;
	}
	virtual int write_to_db(){DEBUG_LOG("write_to_db should implement in subclass");return -1;}
	
	void push_data(Buffer_Wrap wrap);

	void start_thread();
	void stop_thread();
	void wait();
	static void *worker(void *arg);
protected:
	pthread_t tid_;
	DATA_LIST data_list_;
	pthread_mutex_t mutex_;
	sem_t sem_;
	bool stop_;
};

class Mongo_Connector : public DB_Connector {
public:
	Mongo_Connector();
	~Mongo_Connector();

	virtual int connect_to_db(const char *ip, int port, const char *user, const char *passwd);
	virtual int get_record_count(const char *table_name);
	virtual DB_Query_Result read_from_db(const char *table_name, int last_offset, int limit);
	virtual int write_to_db();
private:
	DBClientConnection *dbcon_;
};

class Mysql_Connector : public DB_Connector {
public:
	Mysql_Connector();
	~Mysql_Connector();
	
	virtual int connect_to_db(const char *ip, int port, const char *user, const char *passwd);
	virtual int get_record_count(const char *table_name);
	virtual DB_Query_Result read_from_db(const char *table_name, int last_offset, int limit);
	virtual int write_to_db();
private:
	Connection *dbcon_;
	Statement *dbstate_;
};

#endif

