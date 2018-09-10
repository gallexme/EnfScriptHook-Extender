#include "stdafx.h"

#include <stdio.h>

#include <string>
#include <sstream> 

#include "SDK/inc/main.h"

#include "SDK/inc/curl/curl.h"

#pragma comment(lib, "SDK/lib/libcurl.lib")
#pragma comment(lib, "SDK/lib/xinput1_3.lib")


// Copied from http://www.cplusplus.com/forum/unices/45878/#msg249287
static size_t WriteData(void* buf, size_t size, size_t nmemb, void* userp)
{
	if (userp)
	{
		std::ostream& os = *static_cast<std::ostream*>(userp);
		std::streamsize len = size * nmemb;
		if (os.write(static_cast<char*>(buf), len))
			return len;
	}

	return 0;
}

// Copied from http://www.cplusplus.com/forum/unices/45878/#msg249287
CURLcode CURLReadInternal(const std::string& url, std::ostream& os, long timeout = 30)
{
	CURLcode code(CURLE_FAILED_INIT);
	CURL* curl = curl_easy_init();

	if (curl)
	{
		if (CURLE_OK == (code = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &WriteData))
		 && CURLE_OK == (code = curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L))
		 && CURLE_OK == (code = curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L))
		 && CURLE_OK == (code = curl_easy_setopt(curl, CURLOPT_FILE, &os))
		 && CURLE_OK == (code = curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout))
		 && CURLE_OK == (code = curl_easy_setopt(curl, CURLOPT_URL, url.c_str())))
		{
			code = curl_easy_perform(curl);
		}

		curl_easy_cleanup(curl);
	}

	return code;
}

char* CURLRead(char* url, int timeout)
{
	std::ostringstream oss;
	CURLcode code = CURLReadInternal(url, oss, timeout);
	if (code == CURLE_OK)
	{
		return (char*) oss.str().c_str();
	}
	return (char*)("Error: ");
}

BOOL APIENTRY DllMain( HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
		scriptRegister((char*)"CURLRead", reinterpret_cast<INT64>(&CURLRead));
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

