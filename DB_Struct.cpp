#include "DB_Struct.h"
#include "Config.h"

DB_Struct::DB_Struct(Xml &xml, TiXmlNode *node):
	struct_name_(),
	table_name_(),
	index_name_(),
	type_vec_()
{
	if(node){
		struct_name_ = xml.get_key(node);
		TiXmlNode* sub = xml.enter_node(node, struct_name_.c_str());
		XML_LOOP_BEGIN(sub)
			if(xml.get_key(sub) == "head"){
				std::string table_name = xml.get_attr_str(sub, "table_name");
				table_name_ = "game." + table_name;
				index_name_ = xml.get_attr_str(sub, "index_name");
			}
			else {
				DB_Type_Description des;
				std::string label_name = xml.get_key(sub);
				if(label_name == "map")
					label_name = "vector";
				des.label_name = label_name;
				des.type_name = xml.get_attr_str(sub, "type");
				des.value_name = xml.get_attr_str(sub, "name");
				type_vec_.push_back(des);
			}
		XML_LOOP_END(sub)
	}
}

DB_Struct::~DB_Struct(){
	struct_name_.clear();
	table_name_.clear();
	index_name_.clear();
	type_vec_.clear();
}

void DB_Struct::mongo_read_data_from_buffer(BSONObjBuilder &set_builder, Byte_Buffer &buffer){
	for(std::vector<DB_Type_Description>::iterator iter = type_vec_.begin();
			iter != type_vec_.end(); iter++){
		if((*iter).label_name == "arg"){
			mongo_build_bson_obj_arg(*iter, buffer, set_builder);
		}
		else if((*iter).label_name == "vector"){
			mongo_build_bson_obj_vector(*iter, buffer, set_builder);
		}
		else if((*iter).label_name == "struct"){
			mongo_build_bson_obj_struct(*iter, buffer, set_builder);
		}
	}
}

void DB_Struct::mongo_write_data_to_buffer(BSONObj &obj, Byte_Buffer &buffer){
	for(std::vector<DB_Type_Description>::iterator iter = type_vec_.begin();
			iter != type_vec_.end(); iter++){
		if((*iter).label_name == "arg"){
			mongo_build_buffer_arg(*iter, buffer, obj);
		}
		else if((*iter).label_name == "vector"){
			mongo_build_buffer_vector(*iter, buffer, obj);
		}
		else if((*iter).label_name == "struct"){
			mongo_build_buffer_struct(*iter, buffer, obj);
		}
	}
}

void DB_Struct::mongo_build_bson_obj_arg(DB_Type_Description &des, Byte_Buffer &buffer, BSONObjBuilder &builder){
	if(des.type_name == "int8"){
		int8_t value = 0;
		buffer.read_int8(value);
		builder << des.value_name << (int)value;
	}
	else if(des.type_name == "int16"){
		int16_t value = 0;
		buffer.read_int16(value);
		builder << des.value_name << (int)value;
	}
	else if(des.type_name == "int32"){
		int32_t value = 0;
		buffer.read_int32(value);
		builder << des.value_name << (int)value;
	}
	else if(des.type_name == "int64"){
		int64_t value = 0;
		buffer.read_int64(value);
		builder << des.value_name << (long long int)value;
	}
	else if(des.type_name == "double"){
		double value = 0;
		buffer.read_double(value);
		builder << des.value_name << value;
	}
	else if(des.type_name == "string"){
		std::string value = "";
		buffer.read_string(value);
		builder << des.value_name << value;
	}
	else if(des.type_name == "bool"){
		bool value = 0;
		buffer.read_bool(value);
		builder << des.value_name << (int)value;
	}
	else {
		DEBUG_LOG("Can not find the type %s", des.type_name.c_str());
	}
}

void DB_Struct::mongo_build_bson_obj_vector(DB_Type_Description &des, Byte_Buffer &buffer, BSONObjBuilder &builder){
	std::vector<BSONObj> bson_vec;
	uint16_t vec_size = 0;
	buffer.read_uint16(vec_size);

	if(is_struct_type(des.type_name)){
		for(uint16_t i = 0; i < vec_size; ++i) {
			BSONObjBuilder obj_builder;
			mongo_build_bson_obj_struct(des, buffer, obj_builder);
			bson_vec.push_back(obj_builder.obj());
		}
	}
	else{
		for(uint16_t i = 0; i < vec_size; ++i) {
			BSONObjBuilder obj_builder;
			mongo_build_bson_obj_arg(des, buffer, obj_builder);
			bson_vec.push_back(obj_builder.obj());
		}
	}

	builder << des.value_name << bson_vec;
}

void DB_Struct::mongo_build_bson_obj_struct(DB_Type_Description &des, Byte_Buffer &buffer, BSONObjBuilder &builder){
	DB_Struct *dbs = CONFIG->find_game_struct(des.type_name);
	if(dbs == NULL){
		DEBUG_LOG("Can not find the module %s", des.type_name.c_str());
		return;
	}

	std::vector<DB_Type_Description> type_vec = dbs->type_vec();
	BSONObjBuilder obj_builder;
	for(std::vector<DB_Type_Description>::iterator iter = type_vec.begin();
			iter != type_vec.end(); iter++){
		if((*iter).label_name == "arg"){
			mongo_build_bson_obj_arg(*iter, buffer, obj_builder);
		}
		else if((*iter).label_name == "vector"){
			mongo_build_bson_obj_vector(*iter, buffer, obj_builder);
		}
		else if((*iter).label_name == "struct"){
			mongo_build_bson_obj_struct(*iter, buffer, obj_builder);
		}
	}
	builder << des.type_name << obj_builder.obj();
}

void DB_Struct::mongo_build_buffer_arg(DB_Type_Description &des, Byte_Buffer &buffer, BSONObj &bsonobj) {
	if(des.type_name == "int8"){
		int8_t value = bsonobj[des.value_name].numberInt();
		buffer.write_int8(value);
	}
	else if(des.type_name == "int16"){
		int16_t value = bsonobj[des.value_name].numberInt();
		buffer.write_int16(value);
	}
	else if(des.type_name == "int32"){
		int32_t value = bsonobj[des.value_name].numberInt();
		buffer.write_int32(value);
	}
	else if(des.type_name == "int64"){
		int64_t value = bsonobj[des.value_name].numberLong();
		buffer.write_int64(value);
	}
	else if(des.type_name == "double"){
		uint64_t value = bsonobj[des.value_name].numberDouble();
		buffer.write_double(value);
	}
	else if(des.type_name == "string"){
		std::string value = bsonobj[des.value_name].valuestrsafe();
		buffer.write_string(value);
	}
	else if(des.type_name == "bool"){
		bool value = bsonobj[des.value_name].numberInt();
		buffer.write_bool(value);
	}
	else {
		DEBUG_LOG("Can not find the type %s", des.type_name.c_str());
	}
}

void DB_Struct::mongo_build_buffer_vector(DB_Type_Description &des, Byte_Buffer &buffer, BSONObj &bsonobj) {
	BSONObj fieldobj = bsonobj.getObjectField(des.value_name);
	uint16_t count = fieldobj.nFields();
	buffer.write_uint16(count);

	BSONObjIterator field_iter(fieldobj);
	BSONObj obj;
	if(is_struct_type(des.type_name)){
		while (field_iter.more()) {
			obj = field_iter.next().embeddedObject();
			mongo_build_buffer_struct(des, buffer, obj);
		}
	}
	else{
		while (field_iter.more()) {
			obj = field_iter.next().embeddedObject();
			mongo_build_buffer_arg(des, buffer, obj);
		}
	}
}

void DB_Struct::mongo_build_buffer_struct(DB_Type_Description &des, Byte_Buffer &buffer, BSONObj &bsonobj) {
	DB_Struct *dbs = CONFIG->find_game_struct(des.type_name);
	if(dbs == NULL){
		DEBUG_LOG("Can not find the module %s", des.type_name.c_str());
		return;
	}

	BSONObj fieldobj = bsonobj.getObjectField(des.type_name);
	std::vector<DB_Type_Description> type_vec = dbs->type_vec();
	for(std::vector<DB_Type_Description>::iterator iter = type_vec.begin();
			iter != type_vec.end(); iter++){
		if((*iter).label_name == "arg"){
			mongo_build_buffer_arg(*iter, buffer, fieldobj);
		}
		else if((*iter).label_name == "vector"){
			mongo_build_buffer_vector(*iter, buffer, fieldobj);
		}
		else if((*iter).label_name == "struct"){
			mongo_build_buffer_struct(*iter, buffer, fieldobj);
		}
	}
}

PreparedStatement *DB_Struct::mysql_read_data_from_buffer(Connection *dbdst, Byte_Buffer &buffer) {
	char update_sql[512] = {0};
	int64_t index = 0;
	std::string update_value;
	std::stringstream update_stream;
	for(std::vector<DB_Type_Description>::iterator iter = type_vec_.begin();
			iter != type_vec_.end(); iter++) {
		update_stream.str("");
		update_stream << (*iter).value_name << "=?,";
		update_value += update_stream.str();
	}
	update_value = update_value.substr(0, update_value.length()-1);
	buffer.peek_int64(index);
	sprintf(update_sql, "UPDATE %s", update_value.c_str());
	
	char insert_sql[1048] = {0};
	std::string insert_name;
	std::string insert_value;
	std::stringstream insert_stream;
	for(std::vector<DB_Type_Description>::iterator iter = type_vec_.begin();
			iter != type_vec_.end(); iter++) {
		insert_stream.str("");
		insert_stream << (*iter).value_name << ",";
		insert_name += insert_stream.str();
		insert_stream.str("");
		insert_stream << "?,";
		insert_value += insert_stream.str();
	}
	insert_name = insert_name.substr(0, insert_name.length()-1);
	insert_value = insert_value.substr(0, insert_value.length()-1);
	sprintf(insert_sql, "INSERT INTO %s (%s) VALUES (%s)", table_name().c_str(), insert_name.c_str(), insert_value.c_str());
	sprintf(insert_sql + strlen(insert_sql), " ON DUPLICATE KEY %s", update_sql);
	
	PreparedStatement* pstmt = dbdst->prepareStatement(insert_sql);
	if (!pstmt) {
		DEBUG_LOG("create pstmt error, sql:%s", insert_sql);
		return NULL;
	}
	int param_index = 0;
	int param_len = type_vec_.size();
	for(std::vector<DB_Type_Description>::iterator iter = type_vec_.begin();
		iter != type_vec_.end(); iter++){
		param_index++;
		if((*iter).label_name == "arg") {
			mysql_build_sql_arg(*iter, buffer, pstmt, param_index, param_len);
		}
		else if((*iter).label_name == "vector"){
			mysql_build_sql_vector(buffer, pstmt, param_index, param_len);
		}
		else if((*iter).label_name == "struct"){
			mysql_build_sql_struct(buffer, pstmt, param_index, param_len);
		}
	}
	return pstmt;
}

void DB_Struct::mysql_write_data_to_buffer(ResultSet *result, Byte_Buffer &buffer) {
	for(std::vector<DB_Type_Description>::iterator iter = type_vec_.begin();
			iter != type_vec_.end(); iter++){
		if((*iter).label_name == "arg"){
			mysql_build_buffer_arg(*iter, buffer, result);
		}
		else if((*iter).label_name == "vector") {
			mysql_build_buffer_vector(*iter, buffer, result);
		}
		else if((*iter).label_name == "struct") {
			mysql_build_buffer_struct(*iter, buffer, result);
		}
	}
}

void DB_Struct::mysql_build_sql_arg(DB_Type_Description &des, Byte_Buffer &buffer, PreparedStatement *pstmt, int param_index, int param_len) {
	if(des.type_name == "int8") {
		int8_t value = 0;
		buffer.read_int8(value);
		pstmt->setInt(param_index, value);
		pstmt->setInt(param_index + param_len, value);
	}
	else if(des.type_name == "int16") {
		int16_t value = 0;
		buffer.read_int16(value);
		pstmt->setInt(param_index, value);
		pstmt->setInt(param_index + param_len, value);
	}
	else if(des.type_name == "int32") {
		int32_t value = 0;
		buffer.read_int32(value);
		pstmt->setInt(param_index, value);
		pstmt->setInt(param_index + param_len, value);
	}
	else if(des.type_name == "int64") {
		int64_t value = 0;
		buffer.read_int64(value);
		pstmt->setInt64(param_index, value);
		pstmt->setInt64(param_index + param_len, value);
	}
	else if(des.type_name == "double") {
		double value = 0;
		buffer.read_double(value);
		pstmt->setDouble(param_index, value);
		pstmt->setDouble(param_index + param_len, value);
	}
	else if(des.type_name == "bool") {
		bool value = false;
		buffer.read_bool(value);
		pstmt->setBoolean(param_index, value);
		pstmt->setBoolean(param_index + param_len, value);
	}
	else if(des.type_name == "string") {
		std::string value = "";
		buffer.read_string(value);
		pstmt->setString(param_index, value);
		pstmt->setString(param_index + param_len, value);
	}
	else {
		DEBUG_LOG("Can not find the type:%s", des.type_name.c_str());
	}
}

void DB_Struct::mysql_build_sql_vector(Byte_Buffer &buffer, PreparedStatement *pstmt, int param_index, int param_len) {
	std::string blob_str;
	buffer.read_string(blob_str);
	pstmt->setString(param_index, blob_str);
	pstmt->setString(param_index + param_len, blob_str);
}

void DB_Struct::mysql_build_sql_struct(Byte_Buffer &buffer, PreparedStatement *pstmt, int param_index, int param_len) {
	std::string blob_str;
	buffer.read_string(blob_str);
	pstmt->setString(param_index, blob_str);
	pstmt->setString(param_index + param_len, blob_str);
}

void DB_Struct::mysql_build_buffer_arg(DB_Type_Description &des, Byte_Buffer &buffer, ResultSet *result) {
	if(des.type_name == "int8"){
		int8_t value = result->getInt(des.value_name);
		buffer.write_int8(value);
	}
	else if(des.type_name == "int16"){
		int16_t value = result->getInt(des.value_name);
		buffer.write_int16(value);
	}
	else if(des.type_name == "int32"){
		int32_t value = result->getInt(des.value_name);
		buffer.write_int32(value);
	}
	else if(des.type_name == "int64"){
		int64_t value = result->getInt64(des.value_name);
		buffer.write_int64(value);
	}
	else if(des.type_name == "double"){
		double value = result->getDouble(des.value_name);
		buffer.write_double(value);
	}
	else if(des.type_name == "string"){
		std::string value = result->getString(des.value_name);
		buffer.write_string(value);
	}
	else if(des.type_name == "bool"){
		bool value = result->getBoolean(des.value_name);
		buffer.write_bool(value);
	}
	else {
		DEBUG_LOG("Can not find the type %s", des.type_name.c_str());
	}
}

void DB_Struct::mysql_build_buffer_vector(DB_Type_Description &des, Byte_Buffer &buffer, ResultSet *result) {
	std::string blob_str = result->getString(des.value_name);
	buffer.write_string(blob_str);
}

void DB_Struct::mysql_build_buffer_struct(DB_Type_Description &des, Byte_Buffer &buffer, ResultSet *result) {
	std::string blob_str = result->getString(des.value_name);
	buffer.write_string(blob_str);
}

bool DB_Struct::is_struct_type(std::string type){
	if(type == "int8" || type == "int16" || type == "int32" || type == "int64" ||type == "uint8" ||
			type == "uint16" || type == "uint32" || type == "uint64" || type == "double" || type == "bool" ||
			type == "string")
		return false;
	return true;
}

