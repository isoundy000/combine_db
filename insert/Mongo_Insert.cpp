#include <mongo/client/dbclient.h>
#include <iostream>
#include <string>
#include <ctime>

using namespace std;
using namespace mongo;

string  guild_name[5] = {"guild_name_1", "guild_name_2", "guild_name_3", "guild_name_4", "guild_name_5"};
int64_t chelf_id[5] = {10001000000001, 10001000000002, 10001000000003, 10001000000004, 10001000000005};
int create_time[5] = {1472337081, 1472337082, 1472337083, 1472337084, 1472337085};

int insert_data(DBClientConnection *dbcon);

int main(int argc, char** argv){
	srand(unsigned(time(0)));
	DBClientConnection *dbcon = new DBClientConnection();
	std::string err_msg;
	std::stringstream stream;
	stream << "127.0.0.1";
	stream << ":" << 27017;
	
	if(!dbcon->connect(stream.str().c_str(), err_msg)){
		cout<<"connect to mongodb error: "<<err_msg<<endl;;
		return -1;
	}
	
	insert_data(dbcon);
}

int insert_data(DBClientConnection *dbcon){
	int64_t index_value = 10001000000001;
	for(int i = 0; i < 100000; i++){
		BSONObjBuilder set_builder;
		set_builder << "guild_id" << (long long int)index_value
				<<"guild_name" << guild_name[random() % 5]
				<<"chelf_id" << (long long int)chelf_id[random() %5]
				<<"create_time" << create_time[random() %5];
		std::vector<BSONObj> member_vec;
		BSONObjBuilder member_builder;
		member_builder <<"role_id" << (long long int) index_value
			<<"role_name" << "test_name"
			<<"level" << 1;
		member_vec.push_back(member_builder.obj());
		set_builder << "member_list" << member_vec;
		
		std::vector<BSONObj> app_vec;
		BSONObjBuilder app_builder;
		app_builder <<"role_id" << (long long int) index_value
			<<"role_name" << "app_name"
			<<"level" << 2;
		app_vec.push_back(app_builder.obj());
		set_builder << "app_list" << app_vec;

		dbcon->update("game.guild", MONGO_QUERY("guild_id" << (long long int)index_value),
			BSON("$set" << set_builder.obj()), true);
		index_value++;
		cout<<"insert data "<<index_value<<endl;
	}
	return 0;
}

