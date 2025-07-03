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
#include <codecvt>
#include <locale>
#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <strsafe.h>
#pragma comment(lib, "User32.lib")

#include "../include/Process.hpp"
#include "../include/System.hpp"
#include "VolumeInfo.hpp"

namespace WindowsInfo
{
    class Model
    {
    private:
        std::list<WindowsInfo::Process> processes;
        WindowsInfo::System systemInfo;
        std::vector<FileSystemInfo::VolumeInfo> volumesInfo;
        std::vector<FileSystemInfo::FileInfo> filesInfo;

    public:
        Model();
        ~Model();
        // Ajustar o retorno para o Component
        std::list<WindowsInfo::Process> getProcesses();
        WindowsInfo::System getSystemInfo();
        int getThreadCount();
        void updateProcesses();
        crow::json::wvalue to_json();
        std::vector<FileSystemInfo::VolumeInfo> GetVolumesInfo();
        int GetVolumeInfo(FileSystemInfo::VolumeInfo &volumeInfo);
        bool GetDirectoryInfo(std::string path, std::vector<FileSystemInfo::FileInfo>& filesInfo);
        std::chrono::system_clock::time_point FileTimeToSystemClock(const FILETIME &ft);
    };
}