#include "../include/VolumeInfo.hpp"

namespace FileSystemInfo
{

    std::string TimePointToISO8601(const std::chrono::system_clock::time_point &tp)
    {
        auto time_t = std::chrono::system_clock::to_time_t(tp);
        std::stringstream ss;
        std::tm tm_utc;
        gmtime_s(&tm_utc, &time_t);

        ss << std::put_time(&tm_utc, "%Y-%m-%dT%H:%M:%SZ");
        return ss.str();
    }

    crow::json::wvalue SerializeVolumesToJson(const std::vector<VolumeInfo> &volumes)
    {
        // Cria um objeto JSON do tipo lista (array)
        crow::json::wvalue json_array = crow::json::wvalue::list();

        int index = 0;
        for (const auto &volume : volumes)
        {
            crow::json::wvalue json_volume;
            json_volume["guid"] = volume.Guid;
            json_volume["path"] = volume.Path;
            json_volume["volumeName"] = volume.VolumeName;
            json_volume["serialNumber"] = volume.SerialNumber;
            json_volume["freeMemory"] = volume.FreeMemory;
            json_volume["usedMemory"] = volume.UsedMemory;
            json_volume["totalMemory"] = volume.TotalMemory;

            json_array[index++] = std::move(json_volume);
        }

        return json_array;
    }

    crow::json::wvalue SerializeFilesInfoToJson(const std::vector<FileInfo> &filesInfo)
    {
        crow::json::wvalue json_array = crow::json::wvalue::list();

        int index = 0;
        for (const auto &info : filesInfo)
        {
            crow::json::wvalue json_object;
            json_object["creationTime"] = TimePointToISO8601(info.CreationTime);
            json_object["lastAccessTime"] = TimePointToISO8601(info.LastAccessTime);
            json_object["lastWriteTime"] = TimePointToISO8601(info.LastWriteTime);
            json_object["fileSize"] = info.FileSize;
            json_object["fileName"] = info.FileName;

            json_array[index++] = std::move(json_object);
        }

        return json_array;
    }

    VolumeInfo::VolumeInfo()
    {
    }

    VolumeInfo::~VolumeInfo()
    {
    }


    FileInfo::FileInfo()
    {
    }

    FileInfo::~FileInfo()
    {
    }
}