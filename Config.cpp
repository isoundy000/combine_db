#include "Config.h"

Config *Config::m_conf_ = NULL;

void table_info::init(Xml &xml, TiXmlNode *node) {
	name = xml.get_attr_str(node, "name");
	TiXmlNode *subnode = xml.enter_node(node, "table");
	if(subnode && xml.has_key(subnode, "filter")){
		XML_LOOP_BEGIN(subnode)
			table_filter *filter = new table_filter();
			filter->key = xml.get_attr_str(subnode, "key");
			std::string opt = xml.get_attr_str(subnode, "opt");
			filter->opt = opt == ">" ? 1 : 
							opt == ">=" ? 2 :
							opt == "<" ? 3 :
							opt == "<=" ? 4 :
							opt == "==" ? 5 :
							opt == "!=" ? 6 : 0;
			filter->val = xml.get_attr_int(subnode, "val");
			filter_vec.push_back(filter);
		XML_LOOP_END(subnode)
	}
}

Config::Config()
{

}

Config::~Config() {

}

void Config::init() {
	Xml xml("config/config.xml");
	TiXmlNode *node = xml.get_root_node();
	XML_LOOP_BEGIN(node)
		std::string key = xml.get_key(node);
		if(!strcmp("client_info", key.c_str())){
			strcpy(conf_.dbtype, xml.get_attr_str(node, "dbtype").c_str());
			strcpy(conf_.srcip, xml.get_attr_str(node, "srcip").c_str());
			conf_.srcport = xml.get_attr_int(node, "srcport");
			strcpy(conf_.srcuser, xml.get_attr_str(node, "srcuser").c_str());
			strcpy(conf_.srcpasswd, xml.get_attr_str(node, "srcpasswd").c_str());
			strcpy(conf_.dstip, xml.get_attr_str(node, "dstip").c_str());
			conf_.dstport = xml.get_attr_int(node, "dstport");
			strcpy(conf_.dstuser, xml.get_attr_str(node, "dstuser").c_str());
			strcpy(conf_.dstpasswd, xml.get_attr_str(node, "dstpasswd").c_str());
			strcpy(conf_.struct_path, xml.get_attr_str(node, "struct_path").c_str());
			conf_.table_limit = xml.get_attr_int(node, "table_limit");
			conf_.thread_num = MAX(xml.get_attr_int(node, "thread_num"), 1);
		}
		else if(!strcmp("table", key.c_str())){
			table_info *info = new table_info();
			info->init(xml, node);
			table_info_[info->name] = info;
		}
	XML_LOOP_END(node)
	
	Xml dbxml(conf_.struct_path);
	node = dbxml.get_root_node();
	XML_LOOP_BEGIN(node)
		DB_Struct *db_s = new DB_Struct(dbxml, node);
		db_struct_map_[db_s->struct_name()] = db_s;
		DEBUG_LOG("load struct %s", db_s->struct_name().c_str());
	XML_LOOP_END(node)	
}

DB_Struct *Config::find_db_struct(std::string name) {
	for(DB_STRUCT_MAP::iterator iter = db_struct_map_.begin();
		iter != db_struct_map_.end(); iter++){
		if(iter->second->table_name() == name)
			return iter->second;
	}
	return NULL;
}

DB_Struct *Config::find_game_struct(std::string name) {
	DB_STRUCT_MAP::iterator iter = db_struct_map_.find(name);
	if(iter != db_struct_map_.end())
		return iter->second;
	return NULL;
}

Config *Config::instance() {
	if(m_conf_ == NULL)
		m_conf_ = new Config();
	return m_conf_;
}

