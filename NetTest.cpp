#include <iostream>
#include <windows.h>
#include <stdio.h>
#include <windows.h>
#include <string>
#include <vector>
#include <WinHttp.h>

#pragma comment(lib, "winhttp.lib")

std::wstring BaseHost = L"fapi.binance.com";

std::wstring Requests[] = {
    L"fapi/v1/ticker/bookTicker",
        L"fapi/v1/time",
        L"fapi/v1/depth?symbol=EGLDUSDT&limit=500",
        L"fapi/v1/depth?symbol=SOLUSDT&limit=500",
        L"fapi/v1/klines?symbol=KSMUSDT&interval=5m",
        L"fapi/v1/klines?symbol=SOLUSDT&interval=5m"
};

static bool Terminated = false;

BOOL WINAPI consoleHandler(DWORD signal) {

    if (signal == CTRL_C_EVENT) {
        printf("Ctrl-C handled\n"); // do cleanup
        Terminated = true;
    }
    return TRUE;
}

class HttpRequest
{
private:
    std::wstring _userAgent;
    //std::wstring _proxyIp;
    //std::wstring _proxyPort;
public:
    HttpRequest(const std::wstring&, const std::wstring&, const std::wstring&);
    bool SendRequest(const std::wstring&, const std::wstring&, void*, DWORD);
    std::wstring responseHeader;
    std::vector<BYTE> responseBody;
    DWORD responseCode;
};

HttpRequest::HttpRequest(const std::wstring& userAgent, const std::wstring& proxyIp, const std::wstring& proxyPort) :
    _userAgent(userAgent)
    //,_proxyIp(proxyIp)
    //,_proxyPort(proxyPort)
{
}

bool HttpRequest::SendRequest(const std::wstring& url, const std::wstring& method, void* body, DWORD bodySize)
{
    DWORD dwSize;
    DWORD dwDownloaded;
    DWORD headerSize = 0;
    BOOL  bResults = FALSE;
    HINTERNET hSession;
    HINTERNET hConnect;
    HINTERNET hRequest;

    responseHeader.resize(0);
    responseBody.resize(0);

    hSession = WinHttpOpen(_userAgent.c_str(), WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (hSession) {
        hConnect = WinHttpConnect(hSession, BaseHost.c_str(), INTERNET_DEFAULT_HTTPS_PORT, 0);
    }
    else
    {
        printf("\r\nsession handle failed\n");
        return false;
    }

    if (hConnect) {
        hRequest = WinHttpOpenRequest(hConnect, method.c_str(), url.c_str(), NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
    }
    else 
    {
        printf("\r\nconnect handle failed\n");
        return false;
    }

    if (hRequest) { 
        bResults = WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, body, bodySize, 0, 0); 
    } 
    else 
    {
        printf("\r\nrequest handle failed\n");  
        return false; 
    }


    if (bResults)
        bResults = WinHttpReceiveResponse(hRequest, NULL);
    if (bResults)
    {
        bResults = WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_RAW_HEADERS_CRLF, NULL, WINHTTP_NO_OUTPUT_BUFFER, &headerSize, WINHTTP_NO_HEADER_INDEX);
        if ((!bResults) && (GetLastError() == ERROR_INSUFFICIENT_BUFFER))
        {
            responseHeader.resize(headerSize / sizeof(wchar_t));
            if (responseHeader.empty())
            {
                bResults = TRUE;
            }
            else
            {
                bResults = WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_RAW_HEADERS_CRLF, NULL, &responseHeader[0], &headerSize, WINHTTP_NO_HEADER_INDEX);
                if (!bResults) headerSize = 0;
                responseHeader.resize(headerSize / sizeof(wchar_t));

                DWORD dwSize = sizeof(responseCode);
                bResults = WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER, WINHTTP_HEADER_NAME_BY_INDEX, &responseCode, &dwSize, WINHTTP_NO_HEADER_INDEX);
            }
        }
    }
    if (bResults)
    {
        do
        {
            // Check for available data.
            dwSize = 0;
            bResults = WinHttpQueryDataAvailable(hRequest, &dwSize);
            if (!bResults)
            {
                printf("\r\nError %u in WinHttpQueryDataAvailable.\n", GetLastError());
                break;
            }

            if (dwSize == 0)
                break;

            do
            {
                // Allocate space for the buffer.
                DWORD dwOffset = responseBody.size();
                responseBody.resize(dwOffset + dwSize);

                // Read the data.
                bResults = WinHttpReadData(hRequest, &responseBody[dwOffset], dwSize, &dwDownloaded);
                if (!bResults)
                {
                    printf("\r\nError %u in WinHttpReadData.\n", GetLastError());
                    dwDownloaded = 0;
                }

                responseBody.resize(dwOffset + dwDownloaded);

                if (dwDownloaded == 0)
                    break;

                dwSize -= dwDownloaded;
            } while (dwSize > 0);
        } while (true);
    }

    // Report any errors.
    if (!bResults)
        printf("\r\nError %d has occurred.   <%S> \n ", GetLastError(), url.c_str());

    // Close any open handles.
    if (hRequest) WinHttpCloseHandle(hRequest);
    if (hConnect) WinHttpCloseHandle(hConnect);
    if (hSession) WinHttpCloseHandle(hSession);

    return bResults;
}

tm NowTIme()
{
    time_t rawtime;
    struct tm timeinfo;
    time(&rawtime);
    localtime_s(&timeinfo, &rawtime);
    return timeinfo;
}

// =================================================================



void doNextRequest(const std::wstring& url) {

    HttpRequest Request(L"Example UserAgent/1.0", L"", L"");
    if (Request.SendRequest(url, L"GET", NULL, 0))
    {
        struct tm timeinfo = NowTIme();
        if (Request.responseCode == 200) {
            printf("%02d:%02d:%02d: [%d]  <%S>                                  \r",
                timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec,
                Request.responseCode, url.c_str());
        }
        else {
            printf("\r\n%02d:%02d:%02d: [%d]  <%S> %s                                 \r\n",
                timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec,
                Request.responseCode, url.c_str(), (char*)&Request.responseBody[0]);
        }

    }
}

int main()
{
    if (!SetConsoleCtrlHandler(consoleHandler, TRUE)) {
        std::cout << "SetConsoleCtrlHandler!\n";
        return 1;
    }

    while (!Terminated) {
        for (std::wstring s : Requests)
        { 
            doNextRequest(s);
            Sleep(100);
        }
        Sleep(500);
    }

    std::cout << "DONE!!\n";
    return 0;
}

