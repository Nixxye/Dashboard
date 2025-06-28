#pragma once

#include <stdint.h>
#include <vector>
namespace WindowsInfo {
    class System {
    public:
        System();
        ~System();

        std::vector<double> calculatePerCpuUsage() ;
        double getCpuUsage() ;
        double calculateIdleTime() ;
        double calculateMemoryUsage();
        float getUsedMemory() ;
        double calculateDiskUsage() ;
        int cpuCount;
        float usedMemory;
    private:
        std::vector<LONGLONG> prev_idle;
        std::vector<LONGLONG> prev_kernel;
        std::vector<LONGLONG> prev_user;
        std::vector<LONGLONG> cur_idle;
        std::vector<LONGLONG> cur_ker;
        std::vector<LONGLONG> cur_user;

        double totalCpuUsage = 0.0;
        double idleTime = 0.0;
    };
}
