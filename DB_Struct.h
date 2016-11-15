#ifndef DB_STRUCT_H_
#define DB_STRUCT_H_

#include <mongo/client/dbclient.h>
#include <mysql_connection.h>
#include <mysql_driver.h>
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
#include "xml/xml.h"
#include "Byte_Buffer.h"
#include "Common.h"

using namespace mongo;
using namespace sql;

class DB_Struct;

struct Buffer_Wrap {
	DB_Struct *dbs;
	Byte_Buffer buffer;
};

struct DB_Type_Description {
	std::string label_name;
	std::string type_name;
	std::string value_name;
};

class DB_Struct {
public:
	DB_Struct(Xml &xml, TiXmlNode *node);
	~DB_Struct();
	void mongo_read_data_from_buffer(BSONObjBuilder &set_builder, Byte_Buffer &buffer);
	void mongo_write_data_to_buffer(BSONObj &obj, Byte_Buffer &buffer);
	
	PreparedStatement *mysql_read_data_from_buffer(Connection *dbdst, Byte_Buffer &buffer);
	void mysql_write_data_to_buffer(ResultSet *result, Byte_Buffer &buffer);
	
	inline std::string struct_name(){return struct_name_;}
	inline std::string table_name(){return table_name_;}
	inline std::string index_name(){return index_name_;}
	inline std::vector<DB_Type_Description>& type_vec(){return type_vec_;}
private:
	//############### MongoDB Operator #########################
	void mongo_build_bson_obj_arg(DB_Type_Description &des, Byte_Buffer &buffer, BSONObjBuilder &builder);
	void mongo_build_bson_obj_vector(DB_Type_Description &des, Byte_Buffer &buffer, BSONObjBuilder &builder);
	void mongo_build_bson_obj_struct(DB_Type_Description &des, Byte_Buffer &buffer, BSONObjBuilder &builder);

	void mongo_build_buffer_arg(DB_Type_Description &des, Byte_Buffer &buffer, BSONObj &bsonobj);
	void mongo_build_buffer_vector(DB_Type_Description &des, Byte_Buffer &buffer, BSONObj &bsonobj);
	void mongo_build_buffer_struct(DB_Type_Description &des, Byte_Buffer &buffer, BSONObj &bsonobj);
	
	//############### Mysql Operator #########################
	void mysql_build_sql_arg(DB_Type_Description &des, Byte_Buffer &buffer, PreparedStatement *pstmt, int param_index, int param_len);
	void mysql_build_sql_vector(Byte_Buffer &buffer, PreparedStatement *pstmt, int param_index, int param_len);
	void mysql_build_sql_struct(Byte_Buffer &buffer, PreparedStatement *pstmt, int param_index, int param_len);

	void mysql_build_buffer_arg(DB_Type_Description &des, Byte_Buffer &buffer, ResultSet *result);
	void mysql_build_buffer_vector(DB_Type_Description &des, Byte_Buffer &buffer, ResultSet *result);
	void mysql_build_buffer_struct(DB_Type_Description &des, Byte_Buffer &buffer, ResultSet *result);

	bool is_struct_type(std::string type);
private:
	std::string struct_name_;
	std::string table_name_;
	std::string index_name_;
	std::vector<DB_Type_Description> type_vec_;
};

#endif
