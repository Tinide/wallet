#include "Tool.h"
#include <objbase.h>  
#include <time.h>
#include <mutex>
#include <Iphlpapi.h>
#include <direct.h>
#include <io.h>
#include "base/common/openssl/include/openssl/md5.h"
#include "base/common/openssl/include/openssl/evp.h"
#include "base/common/openssl/include/openssl/bio.h"
#include "base/common/openssl/include/openssl/buffer.h"
#include "base/common/openssl/include/openssl/aes.h"
#include "base/common/markup/Markup.h"
//返回32位uuid
std::string getUuid()
{
	char buffer[64] = { 0 };
	GUID guid;

	CoCreateGuid(&guid);

	_snprintf(buffer, sizeof(buffer),
		"%08X%04X%04x%02X%02X%02X%02X%02X%02X%02X%02X",
		guid.Data1, guid.Data2, guid.Data3,
		guid.Data4[0], guid.Data4[1], guid.Data4[2],
		guid.Data4[3], guid.Data4[4], guid.Data4[5],
		guid.Data4[6], guid.Data4[7]);
	return buffer;
}

bool doubleIsValid(double & d)
{
	if (d > pow(10, 308))
	{
		return false;
	}
	return true;
}

std::string getday(int day)
{
	time_t tt = time(NULL) + day*86400;//这句返回的只是一个时间cuo

	tm* t = localtime(&tt);

	return string_format("%d-%02d-%02d",t->tm_year + 1900,t->tm_mon + 1,t->tm_mday);
}

std::string getNow()
{
	time_t tt = time(NULL);//这句返回的只是一个时间cuo
	tm* t = localtime(&tt);
	return string_format("%d-%02d-%02d %02d:%02d:%02d", t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,t->tm_hour,t->tm_min,t->tm_sec);
}

time_t timeStr2unixtime(std::string timeStr)
{
	if ("" == timeStr)
	{
		return 0;
	}
	tm tm_;

	int year, month, day, hour, minute, second;
	sscanf(timeStr.c_str(), "%d-%d-%d %02d:%02d:%02d", &year, &month, &day, &hour, &minute, &second);
	tm_.tm_year = year - 1900;
	tm_.tm_mon = month - 1;
	tm_.tm_mday = day;
	tm_.tm_hour = hour;
	tm_.tm_min = minute;
	tm_.tm_sec = second;
	tm_.tm_isdst = -1;

	time_t t_ = mktime(&tm_); //已经减了8个时区  
	return t_; //秒时间  
}

std::string toTime(const char * date, const char * time, int msec)
{
	std::string datetime;
	const char* p = strchr(date, '-');
	if (p)
	{
		datetime = date;
	}
	else
	{
		datetime.assign(date, 4);
		datetime += "-";
		datetime.append(date + 4, 2);
		datetime += "-";
		datetime.append(date + 6, 2);
	}

	if (!time)
	{
		return datetime;
	}

	datetime += " ";

	datetime.append(time,8);

	if (msec>=0)
	{
		datetime = string_format("%s.%03d", datetime.c_str(), msec);
	}
	return datetime;
}

std::string ToMD5(const std::string str)
{
	unsigned char md5[16] = { 0 };
	MD5((const unsigned char*)str.c_str(), (int)str.size(), md5);

	std::string md;
	char ss[3];
	for (int i = 0; i<16; i++)
	{
		_snprintf_s(ss, sizeof(ss), "%02X", md5[i]);
		md += ss;
	}
	return md;
}


//利用质数乘积
std::string CreateToken(int login)
{
	GUID guid;
	CoCreateGuid(&guid);
	time_t currentTime = 0;
	time(&currentTime);

	UINT64 temp = 119 * 113 * (currentTime + 60 * 60 * 24 * 7);

	UINT32 i32_1;
	UINT32 i32_2;

	memcpy((void*)&i32_1, (void*)&temp, 4);
	memcpy((void*)&i32_2, (void*)((char*)&temp + 4), 4);
	char buf[64];
	_snprintf_s(buf, sizeof(buf), "%08X-%08X%08X-%04X%08X%02X%02X",
		guid.Data1,
		i32_1,
		login,
		guid.Data3,
		i32_2,
		guid.Data4[0], guid.Data4[1]);

	return std::string(buf);
}

bool verifyToken(std::string token, int login)
{
	if (token.size() != 42)
	{
		return false;
	}
	UINT32 i32_3;
	UINT32 i32_4;
	sscanf(token.substr(9, 8).c_str(), "%x", &i32_3);
	sscanf(token.substr(30, 8).c_str(), "%x", &i32_4);

	int li;
	sscanf(token.substr(17, 8).c_str(), "%x", &li);
	if (li != login)
	{
		return false;
	}

	UINT64 temp2;
	memcpy((void*)&temp2, (void*)(&i32_3), 4);
	memcpy((void*)((char*)&temp2 + 4), (void*)(&i32_4), 4);

	time_t currentTime = 0;
	time(&currentTime);
	double Dividend = 0;
	try {
		Dividend = (double)temp2;
	}
	catch (std::exception& e)
	{
		e.what();
		return false;
	}

	double Remainder = Dividend / (119 * 113);

	if (Remainder >= currentTime)
	{
		if (Remainder - int(Remainder) < 1e-6)
		{
			return true;
		}
	}
	return false;
}

std::string newToken(int login,int expiryTime)
{
	std::string token;
	char sLogin[9] = { 0 };
	_snprintf_s(sLogin, sizeof(sLogin), "%08X", login);

	token = sLogin;

	int t = (int)time(NULL) + expiryTime;
	char sT[9] = { 0 };
	_snprintf_s(sT, sizeof(sT), "%08X", t);

	token += sT;

	token += ToMD5(token);

	int pos = 0;
	while (token[pos] == '0')
	{
		pos++;
	}
	token.erase(0, pos);

	return token;
}

int verifyToken(std::string token)
{
	if (token.size() < 41 || token.size() > 48)
	{
		return 0;
	}
	while (token.size() != 48)
	{
		token = '0' + token;
	}
	std::string sLogin = token.substr(0, 8);
	std::string sT = token.substr(8, 8);
	std::string md = token.substr(16, 32);

	std::string md1 = ToMD5(sLogin + sT);
	if (md != md1)
	{
		return 0;
	}

	int t;
	sscanf(sT.c_str(), "%x", &t);
	if (t < time(NULL))
	{
		return 0;
	}

	int login;
	sscanf(sLogin.c_str(), "%x", &login);

	return login;
}


int gnRequestId;
std::mutex grequestMutex;

int requestNo()
{
	int iRet;
	{
		std::lock_guard<std::mutex> guard(grequestMutex);
		iRet = ++gnRequestId;
	}
	return iRet;
}

std::string unixTime2TimeStr(time_t unixTime)
{
	char buf[32] = { 0 };
	tm* local = localtime(&unixTime); //转为本地时间  
	strftime(buf, 32, "%Y-%m-%d %H:%M:%S", local);
	return buf;
}

std::string unixmilliTime2TimeStr(__time64_t unixTime)
{
	int milli = unixTime % 1000;

	time_t second = unixTime / 1000;
	char buf[32] = { 0 };
	tm* local = localtime(&second); //转为本地时间  
	strftime(buf, 32, "%Y-%m-%d %H:%M:%S", local);

	return string_format("%s.%03d", buf, milli);
}

int getWeekDay(time_t unixTime)
{
	tm* local = localtime(&unixTime); //转为本地时间  
	return local->tm_wday;
}

std::string addOneMin(std::string dateTime)
{
	if ("" == dateTime)
	{
		return "";
	}

	int hour, minute, second;
	sscanf(dateTime.c_str(), "%d:%d:%d", &hour, &minute, &second);

	int sec = 3600 * hour + 60 * minute + second;
	sec += 60;

	hour = sec / 3600;
	minute = sec % 3600 / 60;
	second = sec % 3600 % 60;


	return string_format("%02d:%02d:%02d", hour, minute, second);
}

std::string floorTime(std::string time, int period)
{
	time_t t = timeStr2unixtime(time);
	t = t - t%period;
	return unixTime2TimeStr(t);
}

std::string ceilTime(std::string time, int period)
{
	time_t t = timeStr2unixtime(time);
	t = t - t%period + period;
	return unixTime2TimeStr(t);
}

bool mkDir(std::string dir)
{
	if (_access((dir).c_str(), 02) != 0)
	{
		if (_mkdir((dir).c_str()) != 0)
		{
			return false;
		}
		return true;
	}
	return true;
}

bool equalsDouble(double one, double two)
{
	if (abs(one - two) < 1e-6)
	{
		return true;
	}
	return false;
}

bool doubleIsZero(double d)
{
	if (abs(d) < 1e-6)
	{
		return true;
	}
	return false;
}

bool loopTime(int& n, int loop)
{
	Sleep(1000);
	n++;
	if (n % loop == 0)
	{
		return true;
	}
	return false;
}

std::string Base64Encode(const char * input, int length, bool with_new_line)
{
	BIO * bmem = NULL;
	BIO * b64 = NULL;
	BUF_MEM * bptr = NULL;

	b64 = BIO_new(BIO_f_base64());
	if (!with_new_line) {
		BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
	}
	bmem = BIO_new(BIO_s_mem());
	b64 = BIO_push(b64, bmem);
	BIO_write(b64, input, length);
	BIO_flush(b64);
	BIO_get_mem_ptr(b64, &bptr);

	std::string buf(bptr->data, bptr->length);
	BIO_free_all(b64);
	return buf;
}

char * Base64Decode(char * input, int length, bool with_new_line)
{
	BIO * b64 = NULL;
	BIO * bmem = NULL;
	char * buffer = (char *)malloc(length);
	memset(buffer, 0, length);

	b64 = BIO_new(BIO_f_base64());
	if (!with_new_line) {
		BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
	}
	bmem = BIO_new_mem_buf(input, length);
	bmem = BIO_push(b64, bmem);
	BIO_read(bmem, buffer, length);

	BIO_free_all(bmem);

	return buffer;
}

std::string d2Str_8(double d)
{
	char s[20];
	sprintf(s, "%.8f", d); //将10.8转为字符串
	return std::string(s);
}
#include <random>
unsigned int random()
{
	std::random_device rd;
	return rd();
}

std::string toHex(double d)
{
	return "0x" + (boost::format("%x") % u256(d)).str();
}

std::string toHex(std::string dec)
{
	return "0x" + (boost::format("%x") % u256(dec)).str();
}

std::string toHex(int i)
{
	return "0x" + (boost::format("%x") % u256(i)).str();
}

std::string toLower(const std::string & str)
{
	std::string s = str;
	std::transform(str.begin(), str.end(), s.begin(), std::tolower);
	return s;
}

std::string toUpper(const std::string & str)
{
	std::string s = str;
	std::transform(str.begin(), str.end(), s.begin(), std::toupper);
	return s;
}

std::string decodeUrl(std::string url)
{
	std::string decodeUrl;
	char* str = NULL;
	char s;
	for (int i = 0; i < url.size(); i++)
	{
		if (url[i] == '%')
		{
			std::string hex = url.substr(i + 1, 2);
			s = (char)strtol(hex.c_str(), &str, 16);
			decodeUrl += s;
			i += 2;
		}
		else
		{
			decodeUrl += url[i];
		}
	}

	return CMarkup::UTF8ToA(decodeUrl);
}

std::map<std::string, std::string> praseParams(std::string url)
{
	std::map<std::string, std::string> params;

	std::string decodeUrl = url.substr(url.find('?') + 1);

	while (1)
	{
		int pos = (int)decodeUrl.find('=');
		if (pos > 0)
		{
			std::string key = decodeUrl.substr(0, pos);

			decodeUrl = decodeUrl.substr(pos + 1);
			pos = (int)decodeUrl.find_first_of("&");

			std::string val;
			if (pos == -1)
			{
				val = decodeUrl;
			}
			else
			{
				val = decodeUrl.substr(0, pos);
			}
			params[key] = val;

			if (pos == -1)
			{
				decodeUrl = "";
			}
			else
			{
				decodeUrl = decodeUrl.substr(pos + 1);
			}
		}
		else
		{
			break;
		}
	}
	return params;
}
