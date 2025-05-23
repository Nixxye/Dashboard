//https://stackoverflow.com/questions/62898131/correct-way-to-get-windows-cpu-utilization-for-multiprocessor

#include <windows.h>
#include <iostream>
#include <vector>
#include "../include/System.hpp"

typedef LONG NTSTATUS;
#define NTAPI __stdcall

typedef struct
_SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION_R {
    LARGE_INTEGER IdleTime;
    LARGE_INTEGER KernelTime;
    LARGE_INTEGER UserTime;
    LARGE_INTEGER DpcTime;
    LARGE_INTEGER InterruptTime;
    ULONG InterruptCount;
} SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION_R;

typedef NTSTATUS(NTAPI* NtQuerySystemInformation_t)(
    ULONG SystemInformationClass,
    PVOID SystemInformation,
    ULONG SystemInformationLength,
    PULONG ReturnLength
);

namespace WindowsInfo {

    static long long toInteger(const LARGE_INTEGER& li) {
        return li.QuadPart;
    }

    System::System() {
        SYSTEM_INFO sysInfo;
        GetSystemInfo(&sysInfo);
        cpuCount = sysInfo.dwNumberOfProcessors;
        prev_idle.resize(cpuCount, 0);
        prev_kernel.resize(cpuCount, 0);
        prev_user.resize(cpuCount, 0);
    }

    System::~System() {}

    std::vector<double> System::calculatePerCpuUsage() {
        std::vector<double> usages(cpuCount, 0.0);
        // Carregar dll por conta do mingw:
        HMODULE hNtDll = LoadLibraryW(L"ntdll.dll");
        if (!hNtDll) return usages;

        auto NtQuerySystemInformation = (NtQuerySystemInformation_t)GetProcAddress(hNtDll, "NtQuerySystemInformation");
        if (!NtQuerySystemInformation) {
            FreeLibrary(hNtDll);
            return usages;
        }

        SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION_R* data = new SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION_R[cpuCount];

        if (NtQuerySystemInformation(8, data, sizeof(SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION_R) * cpuCount, nullptr) != 0) {
            delete[] data;
            FreeLibrary(hNtDll);
            return usages;
        }

        uint64_t global_delta_idle = 0;
        uint64_t global_total = 0;

        for (DWORD i = 0; i < cpuCount; ++i) {
            auto& info = data[i];
            uint64_t idle = toInteger(info.IdleTime);
            uint64_t kernel = toInteger(info.KernelTime);
            uint64_t user = toInteger(info.UserTime);

            uint64_t delta_idle = idle - prev_idle[i];
            uint64_t delta_kernel = kernel - prev_kernel[i];
            uint64_t delta_user = user - prev_user[i];
            uint64_t total = delta_kernel + delta_user;

            double usage = 0.0;
            if (total > 0) {
                uint64_t kernel_total = delta_kernel - delta_idle;
                usage = (kernel_total + delta_user) * 100.0 / total;
            }

            usages[i] = usage;
            prev_idle[i] = idle;
            prev_kernel[i] = kernel;
            prev_user[i] = user;

            global_delta_idle += delta_idle;
            global_total += total;
        }

        if (global_total > 0)
            idleTime = (double)global_delta_idle * 100.0 / global_total;


        delete[] data;
        FreeLibrary(hNtDll);

        double sum = 0.0;
        for (double u : usages) sum += u;
        cpuUsage = sum / usages.size();

        return usages;
    }

    double System::getCpuUsage() {
        return cpuUsage;
    }

    double System::calculateIdleTime() {
        return idleTime;
    }

    double System::calculateMemoryUsage() {
        MEMORYSTATUSEX memStatus;
        memStatus.dwLength = sizeof(memStatus);

        if (GlobalMemoryStatusEx(&memStatus)) {
            // Porcentagem de memória usada
            double usedMemory = memStatus.ullTotalPhys - memStatus.ullAvailPhys;
            return (usedMemory * 100.0) / memStatus.ullTotalPhys;
        } else {
            // Se falhar, retorna 0
            // std::cerr << "GlobalMemoryStatusEx failed: " << GetLastError() << std::endl;
            return 0.0;
        }
    }

    double System::calculateDiskUsage() {
        return 0.0;
    }
}

