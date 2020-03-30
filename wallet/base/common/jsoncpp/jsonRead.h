#pragma once
#include <string>
#include "include/json/json.h"
#include <memory>
typedef std::shared_ptr<Json::Value> JsonPtr;
const Json::Value JSON_NULL = Json::Value(Json::nullValue);

#define JsonIntEx(Value) GetJsonInt(Value)
#define JsonStrEx(Value) GetJsonStr(Value)
#define JsonArrEx(Value) GetJsonArr(Value)
#define JsonValEx(Value) GetJsonVal(Value)
#define JsonDoubleEx(Value) GetJsonDouble(Value)

#define JsonBool(Value,Key) GetJsonBoolean(Value,Key)
#define JsonVal(Value,Key) GetJsonVal(Value,Key)
#define JsonInt(Value,Key) GetJsonInt(Value,Key)
#define JsonArr(Value,Key) GetJsonArr(Value,Key)
#define JsonStr(Value,Key) GetJsonStr(Value,Key)
#define JsonDouble(Value,Key) GetJsonDouble(Value,Key)

#define JsonPos(Value,Size) GetJsonArrPos(Value,Size)

//#define JsonRootInt(Key) GetJsonInt(root[Key])
//#define JsonRootStr(Key) GetJsonStr(root[Key])
//#define JsonRootArr(key) GetJsonArr(root[key])
//#define JsonRootVal(Key) GetJsonVal(root[Key])

//获取Json中bool值
bool GetJsonBoolean(const Json::Value& value,std::string key);

//获取Json中int值，防止传入string引起错误
int GetJsonInt(const Json::Value& value);
//获取Json中int值
int GetJsonInt(const Json::Value& value,std::string key);

double GetJsonDouble(const Json::Value& value, std::string key);
double GetJsonDouble(const Json::Value& value);

//获取Json中string值，防止传入int引起错误
std::string GetJsonStr(const Json::Value& value);
//递归获取Json中的string
std::string GetJsonStr(const Json::Value& value,std::string key);	

//获取数组
const Json::Value& GetJsonArr(const Json::Value& value);
//获取key获取数组
const Json::Value& GetJsonArr(const Json::Value& value,std::string key);
//根据size获取数组
const Json::Value& GetJsonArrPos(const Json::Value& value,int size);

//获取Json
const Json::Value& GetJsonVal(const Json::Value& value);
//根据Key获取Json
const Json::Value& GetJsonVal(const Json::Value& value,std::string key);
