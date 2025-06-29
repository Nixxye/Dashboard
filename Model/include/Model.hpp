#pragma once
#define UNICODE
#ifdef _WIN32
#include <winsock2.h>
#include <windows.h>
#endif
#include <crow_all.h>
#include <tlhelp32.h>
#include <tchar.h>
#include <stdio.h>
#include <iostream>
#include <list>
#include <vector>

#include "../include/Process.hpp"
#include "../include/System.hpp"

namespace WindowsInfo {
    class Model {
    private:
        std::list<WindowsInfo::Process> processes;
        WindowsInfo::System systemInfo;
    public:
        Model();
        ~Model();
        //Ajustar o retorno para o Component
        std::list<WindowsInfo::Process> getProcesses();
        WindowsInfo::System getSystemInfo();
        int getThreadCount();
        void updateProcesses();
        crow::json::wvalue to_json();
    };
}