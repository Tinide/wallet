#pragma once
#include <memory>
#include <iostream>
#include <string>
#include <cstdio>
#include <mutex>
#include <base/common/boost/format.hpp>
#include <base/common/boost/multiprecision/cpp_int.hpp>
#include <map>
using u256 = boost::multiprecision::number<boost::multiprecision::cpp_int_backend<256, 256, boost::multiprecision::unsigned_magnitude, boost::multiprecision::unchecked, void>>;

extern int gnRequestId;
extern std::mutex grequestMutex;

std::string getUuid();

template<typename ...Args>
std::string string_format(const std::string & format, Args ...args)
{
	size_t size = snprintf(nullptr, 0, format.c_str(), args ...) + 1; // Extra space for '\0'
	std::unique_ptr<char[]> buf(new char[size]);
	snprintf(buf.get(), size, format.c_str(), args ...);
	return std::string(buf.get(), buf.get() + size - 1); // We don't want the '\0' inside
}

bool doubleIsValid(double& d);

//获取日期
std::string getday(int day=0);

//获取现在的时间
std::string getNow();

//将时间字符串转化为unix时间戳
time_t timeStr2unixtime(std::string timeStr);

std::string toTime(const char * date, const char * time = NULL, int msec = -1);

std::string ToMD5(const std::string str);

std::string CreateToken(int login);
bool verifyToken(std::string token, int login);

std::string newToken(int login,int expiryTime = 7*86400);
int verifyToken(std::string token);

int requestNo();//获取全局唯一序列号

//将unix时间戳转为时间
std::string unixTime2TimeStr(time_t unixTime);

//将unix毫秒时间戳转为时间
std::string unixmilliTime2TimeStr(__time64_t unixTime);

//获取今天是星期几
int getWeekDay(time_t unixTime);

//增加1分钟
std::string addOneMin(std::string dateTime);

//将time变成整点时间，去尾
std::string floorTime(std::string time,int period);

//将time变成整点时间，进一
std::string ceilTime(std::string time, int period);

bool mkDir(std::string dir);

bool equalsDouble(double one, double two);

bool doubleIsZero(double d);

bool loopTime(int& n,int loop= 1);

std::string Base64Encode(const char * input, int length, bool with_new_line);

char * Base64Decode(char * input, int length, bool with_new_line);

std::string d2Str_8(double d);

unsigned  int random();

std::string toHex(double d);

std::string toHex(std::string dec);

std::string toHex(int i);

std::string toLower(const std::string& str);

std::string toUpper(const std::string& str);

std::string decodeUrl(std::string url);

std::map<std::string, std::string> praseParams(std::string url);