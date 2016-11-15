#include "DB_Client.h"
#include "Config.h"

int main(int argc, char **argv){
	Calculate_Time cal_time;
	CONFIG->init();
	DB_Client *dbclient = NULL;
	if(!strcmp("mongo", CONFIG->dbtype())){
		dbclient = new Mongo_Client();
	}
	else if(!strcmp("mysql", CONFIG->dbtype())){
		dbclient = new Mysql_Client();
	}
	else {
		DEBUG_LOG("Database type %s not exists! Use \"mongo\" or \"mysql\"", CONFIG->dbtype());
		return -1;
	}
	
	if(0 != dbclient->connect())
		return -1;
	dbclient->start_thread();
	for(TABLE_INFO_VEC::iterator iter = CONFIG->table_info_vec().begin();
			iter != CONFIG->table_info_vec().end();iter++){
		if(0 != dbclient->process_table(iter->second->name.c_str()))
			return -1;
	}
	
	dbclient->wait_sub_thread();
	
	return 0;
};

