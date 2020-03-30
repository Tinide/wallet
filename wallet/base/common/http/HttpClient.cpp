#include "HttpClient.h"
#include <Windows.h>
#include <winhttp.h>
#include "ATW.h"
#include "base/common/markup/Markup.h"
#include "base/common/log/log.h"
#pragma comment(lib, "winhttp")


static HINTERNET hSession = NULL;

CHttpClient::CHttpClient(void)
{
}

CHttpClient::~CHttpClient(void)
{
}

bool CHttpClient::init()
{
	hSession = WinHttpOpen(L"User Agent",WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,WINHTTP_NO_PROXY_NAME,WINHTTP_NO_PROXY_BYPASS,0);
	if(!hSession)
	{
		return false;
	}
	WinHttpSetTimeouts(hSession, 10000, 10000, 10000, 10000);

	return true;
}

void CHttpClient::uinit()
{
	if (hSession) 
	{
		WinHttpCloseHandle(hSession);
		hSession = NULL;
	}
}


bool CHttpClient::get(std::string url,std::string& response)
{
	response.clear();
	std::wstring w_url = AtoW(url);

	URL_COMPONENTS urlComp;

	// Initialize the URL_COMPONENTS structure.
	ZeroMemory(&urlComp, sizeof(urlComp));
	urlComp.dwStructSize = sizeof(urlComp);

	// Set required component lengths to non-zero 
	// so that they are cracked.
	urlComp.dwSchemeLength    = -1;
	urlComp.dwHostNameLength  = -1;
	urlComp.dwUrlPathLength   = -1;
	urlComp.dwExtraInfoLength = -1;

	// Crack the URL.
	const wchar_t* pw_url = w_url.c_str();
	if (!WinHttpCrackUrl( pw_url, DWORD(wcslen(pw_url)), 0, &urlComp))
	{
		return false;
	}

	std::wstring hostName(urlComp.lpszHostName,urlComp.dwHostNameLength);
	std::wstring urlPath(urlComp.lpszUrlPath);

	HINTERNET  hConnect = NULL, hRequest = NULL; 

	BOOL  bResults = FALSE; 

	if(hSession) 
	{ 
		hConnect=WinHttpConnect(hSession,hostName.c_str(),INTERNET_DEFAULT_HTTP_PORT,0); 
	} 
	if(hConnect) 
	{ 
		hRequest=WinHttpOpenRequest(hConnect, L"GET",urlPath.c_str(),L"HTTP/1.1", WINHTTP_NO_REFERER,WINHTTP_DEFAULT_ACCEPT_TYPES,0); 
	} 

	if(hRequest) 
	{ 
		bResults=WinHttpSendRequest(hRequest,WINHTTP_NO_ADDITIONAL_HEADERS, 0,WINHTTP_NO_REQUEST_DATA, 0, 0, 0 ); 
	} 

	if(bResults) 
	{ 
		bResults=WinHttpReceiveResponse(hRequest,NULL); 

	} 

	DWORD dwSize = 0; 
	DWORD dwDownloaded = 0; 
	LPSTR pszOutBuffer = NULL; 
	bool bRet = true;

	if(bResults) 
	{ 
		do
		{ 
			// Check for available data. 
			dwSize = 0; 

			if (!WinHttpQueryDataAvailable( hRequest, &dwSize)) 
			{ 
				bRet = false;
				break;
			} 

			if (!dwSize) 
				break; 

			pszOutBuffer = new char[dwSize+1]; 

			if (!pszOutBuffer) 
			{ 
				bRet = false; 
				break;
			} 

			ZeroMemory(pszOutBuffer, dwSize+1); 

			if (!WinHttpReadData( hRequest, (LPVOID)pszOutBuffer,  dwSize, &dwDownloaded)) 
			{ 
				bRet = false;
				break;
			} 
			else
			{ 
				response.append(pszOutBuffer,dwSize); 
			} 

			delete [] pszOutBuffer;
			pszOutBuffer = NULL;

			if (!dwDownloaded) 
			{
				bRet = false;
				break; 
			}

		} while (dwSize > 0); 
	}
	else
	{
		bRet = false;
	}

	if (hRequest) WinHttpCloseHandle(hRequest); 
	if (hConnect) WinHttpCloseHandle(hConnect); 

	return bRet;
}




bool CHttpClient::post(std::string url, Json::Value req, Json::Value& rsp, bool Authorization)
{
	rsp.clear();
	std::string response;
	std::wstring w_url = AtoW(url);

	URL_COMPONENTS urlComp;

	// Initialize the URL_COMPONENTS structure.
	ZeroMemory(&urlComp, sizeof(urlComp));
	urlComp.dwStructSize = sizeof(urlComp);

	// Set required component lengths to non-zero 
	// so that they are cracked.
	urlComp.dwSchemeLength    = -1;
	urlComp.dwHostNameLength  = -1;
	urlComp.dwUrlPathLength   = -1;
	urlComp.dwExtraInfoLength = -1;

	// Crack the URL.
	const wchar_t* pw_url = w_url.c_str();
	if (!WinHttpCrackUrl( pw_url, DWORD(wcslen(pw_url)), 0, &urlComp))
	{
		return false;
	}

	std::wstring hostName(urlComp.lpszHostName,urlComp.dwHostNameLength);
	std::wstring urlPath(urlComp.lpszUrlPath);

	HINTERNET  hConnect = NULL,hRequest = NULL;

	BOOL  bResults = FALSE;

	if(hSession)
	{
		hConnect = WinHttpConnect(hSession, hostName.c_str(), urlComp.nPort, 0);
	}

	if(hConnect)
	{
		hRequest=WinHttpOpenRequest(hConnect, L"POST",urlPath.c_str(),L"HTTP/1.1", WINHTTP_NO_REFERER,WINHTTP_DEFAULT_ACCEPT_TYPES,0);
	}

	//LPCWSTR header=L"Content-type: application/x-www-form-urlencoded";
	//LPCWSTR header = L"Content-type: text/plain";
	LPCWSTR header = L"Content-type: application/json";
	SIZE_T len = lstrlenW(header);

	WinHttpAddRequestHeaders(hRequest,header,DWORD(len), WINHTTP_ADDREQ_FLAG_ADD);

	if (Authorization)
	{
		LPCWSTR header = L"Authorization: Basic cnVpc2h1bzpydWlzaHVv";

		SIZE_T len = lstrlenW(header);

		WinHttpAddRequestHeaders(hRequest, header, DWORD(len), WINHTTP_ADDREQ_FLAG_ADD);
	}

	if(hRequest)
	{
		std::string request = Json::FastWriter().write(req);
		const void *ss=(const char *)request.c_str();
		bResults=WinHttpSendRequest(hRequest, 0, 0,const_cast<void *>(ss),DWORD(request.length()), DWORD(request.length()), 0 );
	}

	if(bResults)
	{
		bResults=WinHttpReceiveResponse(hRequest,NULL);

	}
	//用于获取headers
	//if (bResults)
	//{
	//	DWORD dwSize = 0;
	//	LPVOID lpOutBuffer = NULL;
	//	WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_RAW_HEADERS_CRLF,
	//		WINHTTP_HEADER_NAME_BY_INDEX, NULL,
	//		&dwSize, WINHTTP_NO_HEADER_INDEX);

	//	// Allocate memory for the buffer.
	//	if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
	//	{
	//		lpOutBuffer = new WCHAR[dwSize / sizeof(WCHAR)];

	//		// Now, use WinHttpQueryHeaders to retrieve the header.
	//		bResults = WinHttpQueryHeaders(hRequest,
	//			WINHTTP_QUERY_RAW_HEADERS_CRLF,
	//			WINHTTP_HEADER_NAME_BY_INDEX,
	//			lpOutBuffer, &dwSize,
	//			WINHTTP_NO_HEADER_INDEX);
	//	}
	//}

	DWORD dwSize = 0;
	DWORD dwDownloaded = 0;
	LPSTR pszOutBuffer = NULL;
	bool bRet = true;

	if(bResults) 
	{ 
		do
		{ 
			// Check for available data. 
			dwSize = 0; 

			if (!WinHttpQueryDataAvailable( hRequest, &dwSize)) 
			{ 
				bRet = false;
				break;
			} 

			if (!dwSize) 
				break; 

			pszOutBuffer = new char[dwSize+1]; 

			if (!pszOutBuffer) 
			{ 
				bRet = false; 
				break;
			} 

			ZeroMemory(pszOutBuffer, dwSize+1); 

			if (!WinHttpReadData( hRequest, (LPVOID)pszOutBuffer,  dwSize, &dwDownloaded)) 
			{ 
				bRet = false;
				break;
			} 
			else
			{ 
				response.append(pszOutBuffer,dwSize); 
			} 

			delete [] pszOutBuffer;
			pszOutBuffer = NULL;

			if (!dwDownloaded) 
			{
				bRet = false;
				break; 
			}

		} while (dwSize > 0); 
		
		int sz = (int)response.size();
		if (sz > 0)
		{
			if (sz < 2000)
			{
				LOG_INFO(response);
			}
			Json::Reader().parse(CMarkup::UTF8ToA(response), rsp);
		}
	}
	else
	{
		bRet = false;
	}

	if (hRequest) WinHttpCloseHandle(hRequest);
	if (hConnect) WinHttpCloseHandle(hConnect);

	return bRet;
}