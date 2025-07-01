#pragma once
#include <crow_all.h>
#include "../Model/include/Thread.hpp"
#include "../Model/include/Process.hpp"
#include "../Model/include/System.hpp"
#include "../Model/include/Model.hpp"


namespace WindowsInfo {
    inline crow::json::wvalue to_json(Thread thread) {
        crow::json::wvalue j;
        j["id"] = thread.id;
        j["priorityBase"] = thread.getPriorityBase();
        j["priorityDelta"] = thread.getPriorityDelta();
        return j;
    }
    inline crow::json::wvalue to_json(System system) {
        crow::json::wvalue j;
        j["cpuCount"] = system.cpuCount;

        j["idleTime"] = system.calculateIdleTime();
        j["memoryUsage"] = system.calculateMemoryUsage();
        j["diskUsage"] = system.calculateDiskUsage();

        auto perCpu = system.calculatePerCpuUsage();
        crow::json::wvalue perCpuJson;
        for (size_t i = 0; i < perCpu.size(); ++i) {
            perCpuJson[std::to_string(i)] = perCpu[i];
        }
        j["perCpuUsage"] = std::move(perCpuJson);
        j["cpuUsage"] = system.getCpuUsage();
        j["usedMemory"] = system.getUsedMemory();
        return j;
    }
    inline crow::json::wvalue to_json(Process process) {
        crow::json::wvalue j;
        j["id"] = process.id;
        j["parentId"] = process.parentId;
        j["name"] = process.name;
        j["threadCount"] = process.getThreadCount();
        j["priorityBase"] = process.getPriorityBase();
        j["priorityClass"] = process.getPriorityClass();
        j["userName"] = process.getUserName();

        j["memoryWorkingSet"] = process.getMemoryWorkingSet();
        j["memoryCommitted"] = process.getMemoryCommitted();
        j["privateMemoryCommitted"] = process.getPrivateMemoryCommitted();
        j["memoryReserved"] = process.getMemoryReserved();
        j["memoryHeap"] = process.getMemoryHeap();
        j["memoryStack"] = process.getMemoryStack();
        j["memoryCode"] = process.getMemoryCode();
        j["numberOfPages"] = process.getNumberOfPages();

        // Serializa a lista de threads
        crow::json::wvalue threadsJson = crow::json::wvalue::list();
        int index = 0;
        for ( auto& thread : process.getThreads()) {
            threadsJson[index++] = to_json(thread);
        }
        j["threads"] = std::move(threadsJson);

        return j;
    }

    inline crow::json::wvalue to_json(Model model) {
        crow::json::wvalue j;

        // Serializa a lista de processos
        crow::json::wvalue processesJson = crow::json::wvalue::list();
        int index = 0;
        for (auto& process : model.getProcesses()) {
            processesJson[index++] = to_json(process);
        }
        j["processes"] = std::move(processesJson);

        // Serializa o objeto System
        j["systemInfo"] = to_json(model.getSystemInfo());

        // Opcional: adicionar contagem total de threads
        j["threadCount"] = model.getThreadCount();

        return j;
    }
}
