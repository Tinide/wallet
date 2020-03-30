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

//��ȡJson��boolֵ
bool GetJsonBoolean(const Json::Value& value,std::string key);

//��ȡJson��intֵ����ֹ����string�������
int GetJsonInt(const Json::Value& value);
//��ȡJson��intֵ
int GetJsonInt(const Json::Value& value,std::string key);

double GetJsonDouble(const Json::Value& value, std::string key);
double GetJsonDouble(const Json::Value& value);

//��ȡJson��stringֵ����ֹ����int�������
std::string GetJsonStr(const Json::Value& value);
//�ݹ��ȡJson�е�string
std::string GetJsonStr(const Json::Value& value,std::string key);	

//��ȡ����
const Json::Value& GetJsonArr(const Json::Value& value);
//��ȡkey��ȡ����
const Json::Value& GetJsonArr(const Json::Value& value,std::string key);
//����size��ȡ����
const Json::Value& GetJsonArrPos(const Json::Value& value,int size);

//��ȡJson
const Json::Value& GetJsonVal(const Json::Value& value);
//����Key��ȡJson
const Json::Value& GetJsonVal(const Json::Value& value,std::string key);
