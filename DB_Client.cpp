#include "DB_Client.h"
#include "Config.h"

DB_Client::DB_Client():
	last_offset_(0),
	db_con_vec_()
{
}

void DB_Client::start_thread() {
	for(DB_CONNECTOR_VEC::iterator iter = db_con_vec_.begin();
			iter != db_con_vec_.end(); iter++){
		(*iter)->start_thread();
	}
}

void DB_Client::wait_sub_thread(){
	for(DB_CONNECTOR_VEC::iterator iter = db_con_vec_.begin();
			iter != db_con_vec_.end(); iter++){
		(*iter)->stop_thread();
		(*iter)->wait();
	}
}

Mongo_Client::Mongo_Client()
{
}

Mongo_Client::~Mongo_Client(){
}

int Mongo_Client::connect(){
	db_src_con_ = new Mongo_Connector();
	db_src_con_->connect_to_db(CONFIG->srcip(), CONFIG->srcport(), CONFIG->srcuser(), CONFIG->srcpasswd());

	for(int i = 0; i < CONFIG->thread_num(); i++) {
		Mongo_Connector *dbcon = new Mongo_Connector();
		dbcon->connect_to_db(CONFIG->dstip(), CONFIG->dstport(), CONFIG->dstuser(), CONFIG->dstpasswd());
		db_con_vec_.push_back(dbcon);
	}
	return 0;
}

int Mongo_Client::process_table(std::string table) {
	DB_Struct *dbs = CONFIG->find_db_struct(table);
	if(dbs == NULL){
		DEBUG_LOG("DB_Struct %s dosen't exists!", table.c_str());
		return -1;
	}
	std::string table_name = dbs->table_name();
	int limit = CONFIG->table_limit();
	int thread_num = CONFIG->thread_num();
	int count = db_src_con_->get_record_count(table_name.c_str());
	DEBUG_LOG("%s has %d records", table_name.c_str(), count);
	std::auto_ptr<DBClientCursor> cursor;
	while(last_offset_ < count){
		cursor = db_src_con_->read_from_db(table_name.c_str(), last_offset_, limit).mongo_result;
		int seq = 0;
		while(cursor->more()){
			BSONObj obj = cursor->next();
			Buffer_Wrap wrap;
			wrap.dbs = dbs;
			dbs->mongo_write_data_to_buffer(obj, wrap.buffer);
			
			db_con_vec_[seq % thread_num]->push_data(wrap);
			seq++;
		}
		last_offset_ += limit;
	}
	last_offset_ = 0;
	return 0;
}

Mysql_Client::Mysql_Client()
{
}

Mysql_Client::~Mysql_Client(){
}

int Mysql_Client::connect() {
	db_src_con_ = new Mysql_Connector();
	db_src_con_->connect_to_db(CONFIG->srcip(), CONFIG->srcport(), CONFIG->srcuser(), CONFIG->srcpasswd());

	for(int i = 0; i < CONFIG->thread_num(); i++) {
		Mysql_Connector *dbcon = new Mysql_Connector();
		dbcon->connect_to_db(CONFIG->dstip(), CONFIG->dstport(), CONFIG->dstuser(), CONFIG->dstpasswd());
		db_con_vec_.push_back(dbcon);
	}
	return 0;
}

int Mysql_Client::process_table(std::string table) {
	DB_Struct *dbs = CONFIG->find_db_struct(table);
	if(dbs == NULL){
		DEBUG_LOG("DB_Struct %s dosen't exists!", table.c_str());
		return -1;
	}
	std::string table_name = dbs->table_name();
	
	int limit = CONFIG->table_limit();
	int thread_num = CONFIG->thread_num();
	int count = db_src_con_->get_record_count(table_name.c_str());
	if(count == -1)
		return -1;
	DEBUG_LOG("%s has %d records", table_name.c_str(), count);
	ResultSet *result;
	while(last_offset_ < count){
		result = db_src_con_->read_from_db(table_name.c_str(), last_offset_, limit).mysql_result;
		int seq = 0;
		while(result->next())
		{
			Buffer_Wrap wrap;
			wrap.dbs = dbs;
			dbs->mysql_write_data_to_buffer(result, wrap.buffer);
			
			db_con_vec_[seq % thread_num]->push_data(wrap);
			seq++;
		}
		last_offset_ += limit;
	}
	last_offset_ = 0;
	return 0;
}

