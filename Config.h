#ifndef _CONFIG_H_
#define _CONFIG_H_

#include "xml/xml.h"
#include "Common.h"
#include "DB_Struct.h"

struct client_info {
	char dbtype[32];
	char srcip[16];
	int srcport;
	char srcuser[32];
	char srcpasswd[32];
	char dstip[16];
	int dstport;
	char dstuser[32];
	char dstpasswd[32];
	char struct_path[64];
	int table_limit;
	int thread_num;
};

struct table_filter {
	std::string key;
	int opt;
	int val;
};

struct table_info {
	std::string name;
	std::vector<table_filter *> filter_vec;
	void init(Xml &xml, TiXmlNode *node);
};

typedef std::map<std::string, table_info *> TABLE_INFO_VEC;

typedef std::map<std::string, DB_Struct *> DB_STRUCT_MAP;

class Config {
private:
	Config();
	~Config();
public:
	void init();
	DB_Struct *find_db_struct(std::string name);
	DB_Struct *find_game_struct(std::string name);
	static Config *instance();
	
	inline char *dbtype(){return conf_.dbtype;}
	inline char *srcip(){return conf_.srcip;}
	inline int srcport(){return conf_.srcport;}
	inline char *srcuser(){return conf_.srcuser;}
	inline char *srcpasswd(){return conf_.srcpasswd;}
	inline char *dstip(){return conf_.dstip;}
	inline int dstport(){return conf_.dstport;}
	inline char *dstuser(){return conf_.dstuser;}
	inline char *dstpasswd(){return conf_.dstpasswd;}
	inline char *struct_path(){return conf_.struct_path;}
	inline int table_limit(){return conf_.table_limit;}
	inline int thread_num(){return conf_.thread_num;}
	inline TABLE_INFO_VEC &table_info_vec(){return table_info_;}
private:
	static Config *m_conf_;
	client_info conf_;
	TABLE_INFO_VEC table_info_;
	DB_STRUCT_MAP db_struct_map_;
};

#define CONFIG Config::instance()

#endif

