#pragma once
#include <string>
#include <crow_all.h>

namespace FileSystemInfo
{
    class VolumeInfo
    {
    public:
        std::string Guid;
        std::string Path;
        std::string VolumeName;
        unsigned int SerialNumber;
        unsigned long long UsedMemory;
        unsigned long long FreeMemory;
        unsigned long long TotalMemory;

        VolumeInfo();
        ~VolumeInfo();
    };

    class FileInfo
    {
    public:
        std::chrono::system_clock::time_point CreationTime;
        std::chrono::system_clock::time_point LastAccessTime;
        std::chrono::system_clock::time_point LastWriteTime;
        unsigned long long FileSize;
        std::string FileName;

        FileInfo();
        ~FileInfo();
    };

    crow::json::wvalue SerializeVolumesToJson(const std::vector<VolumeInfo> &volumes);

    crow::json::wvalue SerializeFilesInfoToJson(const std::vector<FileInfo> &filesInfo);
}