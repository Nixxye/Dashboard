#pragma once
#define UNICODE
#ifdef _WIN32
#include <winsock2.h>
#include <windows.h>
#endif
#include <crow_all.h>
#include <string>

namespace WindowsInfo {
    class Handle {
    public:
        HANDLE handleValue;
        std::wstring type;
        std::wstring name;
        std::string path;

        Handle(HANDLE handleValue);
        crow::json::wvalue to_json() const;
    };
}
