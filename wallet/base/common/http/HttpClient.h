#pragma once
#include <string>
#include "base/common/jsoncpp/jsonRead.h"

class CHttpClient
{
public:
	CHttpClient(void);
	virtual ~CHttpClient(void);

public:
	static bool init();
	static void uinit();
	bool get(std::string url,std::string& response);
	bool post(std::string url,Json::Value req,Json::Value& rsp,bool Authorization =false);
};