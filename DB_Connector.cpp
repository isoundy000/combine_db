#include "DB_Connector.h"

DB_Connector::DB_Connector():
	tid_(-1),
	data_list_(),
	mutex_(),
	sem_(),
	stop_(false)
{
	pthread_mutex_init(&mutex_, NULL);
	sem_init(&sem_, 0, 0);
}

void DB_Connector::push_data(Buffer_Wrap wrap) {
	pthread_mutex_lock(&mutex_);
	data_list_.push_back(wrap);
	pthread_mutex_unlock(&mutex_);
	sem_post(&sem_);
}

void *DB_Connector::worker(void *arg) {
	DB_Connector *p = (DB_Connector *)arg;
	while(0 == p->write_to_db());
	return arg;
}

void DB_Connector::start_thread() {
	pthread_create(&tid_, NULL, worker, this);
}

void DB_Connector::stop_thread() {
	pthread_mutex_lock(&mutex_);
	stop_ = true;
	pthread_mutex_unlock(&mutex_);
	sem_post(&sem_);
}

void DB_Connector::wait() {
	if(tid_ < 0)
		return;
	pthread_join(tid_, NULL);
}

Mongo_Connector::Mongo_Connector():
	DB_Connector(),
	dbcon_()
{

}

Mongo_Connector::~Mongo_Connector() {

}

int Mongo_Connector::connect_to_db(const char *ip, int port, const char *user, const char *passwd) {
	dbcon_ = new DBClientConnection();
	std::string err_msg;
	std::stringstream stream;
	
	stream << ip;
	stream << ":" << port;
	
	if(!dbcon_->connect(stream.str().c_str(), err_msg)){
		DEBUG_LOG(("connect to mongodb error: " + err_msg).c_str());
		return -1;
	}
	DEBUG_LOG("connected to mongodb %s:%d success", ip, port);
	return 0;
}

int Mongo_Connector::get_record_count(const char *table_name) {
	return dbcon_->count(table_name);
}

DB_Query_Result Mongo_Connector::read_from_db(const char *table_name, int last_offset, int limit) {
	DB_Query_Result result;
	result.mongo_result = dbcon_->query(table_name, Query(), limit, last_offset);
	return result;
}

int Mongo_Connector::write_to_db() {
	sem_wait(&sem_);
	pthread_mutex_lock(&mutex_);
	if(stop_ && data_list_.empty()){
		pthread_mutex_unlock(&mutex_);
		return 1;
	}
	Buffer_Wrap wrap = data_list_.front();
	data_list_.pop_front();
	pthread_mutex_unlock(&mutex_);

	int64_t index_value = 0;
	wrap.buffer.peek_int64(index_value);
	BSONObjBuilder set_builder;
	wrap.dbs->mongo_read_data_from_buffer(set_builder, wrap.buffer);
	dbcon_->update(wrap.dbs->table_name(), MONGO_QUERY(wrap.dbs->index_name() << (long long int)index_value),
		BSON("$set" << set_builder.obj()), true);
	DEBUG_LOG("save %s data index : %ld", wrap.dbs->table_name().c_str(), index_value);
	return 0;
}

Mysql_Connector::Mysql_Connector():
	DB_Connector(),
	dbcon_(),
	dbstate_()
{

}

Mysql_Connector::~Mysql_Connector() {

}

int Mysql_Connector::connect_to_db(const char *ip, int port, const char *user, const char *passwd) {
	mysql::MySQL_Driver *driver;
	std::stringstream stream;
	
	stream << ip;
	stream << ":" << port;
	
	driver = sql::mysql::get_mysql_driver_instance(); 	

	try{
		dbcon_ = driver->connect(stream.str().c_str(), user, passwd);
    	dbstate_ = dbcon_->createStatement();
		DEBUG_LOG("connected to src mysql %s:%d success", ip, port);
	}
	catch(...){
		DEBUG_LOG("connect mysql %s:%d error", ip, port);
		return -1;
	}

	return 0;
}

int Mysql_Connector::get_record_count(const char *table_name) {
	int count = 0;
	char count_sql[128] = {};
	sprintf(count_sql, "select count(1) from %s", table_name);
	try {
		ResultSet *res_count = dbstate_->executeQuery(count_sql);
		if(!res_count->next())
			return -1;
		count = res_count->getInt("count(1)");
	}
	catch(...){
		DEBUG_LOG("SQL ERROR:%s", count_sql);
		return -1;
	}
	return count;
}

DB_Query_Result Mysql_Connector::read_from_db(const char *table_name, int last_offset, int limit) {
	DB_Query_Result result;
	char query_sql[128] = {};
	sprintf(query_sql, "select * from %s limit %d,%d", table_name, last_offset, limit);
	try {
		result.mysql_result = dbstate_->executeQuery(query_sql);
	}
	catch(...) {
		DEBUG_LOG("SQL ERROR:%s");
	}
	return result;
}

int Mysql_Connector::write_to_db() {
	sem_wait(&sem_);
	pthread_mutex_lock(&mutex_);
	if(stop_ && data_list_.empty()){
		pthread_mutex_unlock(&mutex_);
		return 1;
	}
	Buffer_Wrap wrap = data_list_.front();
	data_list_.pop_front();
	pthread_mutex_unlock(&mutex_);
	
	int64_t index_value = 0;
	wrap.buffer.peek_int64(index_value);
	DEBUG_LOG("save %s data index : %ld", wrap.dbs->table_name().c_str(), index_value);
	PreparedStatement* pstmt = 	wrap.dbs->mysql_read_data_from_buffer(dbcon_, wrap.buffer);
	pstmt->execute();
	delete pstmt;
	return 0;
}

