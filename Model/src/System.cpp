//https://stackoverflow.com/questions/62898131/correct-way-to-get-windows-cpu-utilization-for-multiprocessor

#include <windows.h>
#include <winternl.h>
#include <iostream>
#include <vector>
#include "../include/System.hpp"

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
    // https://web.archive.org/web/20150215064443/http://support.microsoft.com/kb/287158
    //https://learn.microsoft.com/en-us/windows/win32/api/winternl/nf-winternl-ntquerysysteminformation
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

        SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION* data = new SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION [cpuCount];

        if (NtQuerySystemInformation(SystemProcessorPerformanceInformation, data, sizeof(SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION) * cpuCount, nullptr) != 0) {
            delete[] data;
            FreeLibrary(hNtDll);
            return usages;
        }

        for (int i = 0; i < cpuCount; ++i) {
            LONGLONG current_idle = data[i].IdleTime.QuadPart;
            LONGLONG current_kernel = data[i].KernelTime.QuadPart;
            LONGLONG current_user = data[i].UserTime.QuadPart;

            LONGLONG delta_idle = current_idle - prev_idle[i];
            LONGLONG  delta_kernel = current_kernel - prev_kernel[i];
            LONGLONG  delta_user = current_user - prev_user[i];



            LONGLONG  active = (delta_kernel + delta_user - delta_idle);
            LONGLONG  total = (delta_kernel + delta_user);



            usages[i] = (double) (active * 100.0) / (double) total;

            // if (i == 0) {
            //     std::cout << "CPU " << i << ": " << usages[i] << std::endl;
            //     std::cout << "KernelTime: " << current_kernel << std::endl;
            //     std::cout << "UserTime: " << current_user << std::endl;
            //     std::cout << "IdleTime: " << current_idle << std::endl;
            //     std::cout << "Delta Idle: " << delta_idle << std::endl;
            //     std::cout << "Delta Kernel: " << delta_kernel << std::endl;
            //     std::cout << "Delta User: " << delta_user << std::endl;
            //     std::cout << "Active: " << active << std::endl;
            //     std::cout << "Total: " << total << std::endl;
            //     std::cout << "Prev Idle: " << prev_idle[i] << std::endl;
            //     std::cout << "Prev Kernel: " << prev_kernel[i] << std::endl;
            //     std::cout << "Prev User: " << prev_user[i] << std::endl;
            //     std::cout << "-------------------------" << std::endl;
            // }
            prev_idle[i] = current_idle;
            prev_kernel[i] = current_kernel;
            prev_user[i] = current_user;
        }



        delete[] data;
        FreeLibrary(hNtDll);

        double sum = 0.0;
        for (double u : usages) sum += u;
        totalCpuUsage = sum / cpuCount;
        // std::cout << "Total CPU Usage: " << totalCpuUsage << std::endl;
        // std::cout << "-------------------------" << std::endl;

        return usages;
    }

    double System::getCpuUsage() {
        return totalCpuUsage;
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

