//https://stackoverflow.com/questions/62898131/correct-way-to-get-windows-cpu-utilization-for-multiprocessor

#include <iterator>
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

// https://learn.microsoft.com/pt-br/windows/win32/secauthz/enabling-and-disabling-privileges-in-c--
bool EnableDebugPrivilege() {
    HANDLE hToken;
    LUID luid;
    TOKEN_PRIVILEGES tp;

    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) {
        std::cerr << "OpenProcessToken failed\n";
        return false;
    }

    if (!LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &luid)) {
        std::cerr << "LookupPrivilegeValue failed\n";
        CloseHandle(hToken);
        return false;
    }

    tp.PrivilegeCount = 1;
    tp.Privileges[0].Luid = luid;
    tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    if (!AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(tp), NULL, NULL)) {
        std::cerr << "AdjustTokenPrivileges failed\n";
        CloseHandle(hToken);
        return false;
    }

    CloseHandle(hToken);
    return true;
}


namespace WindowsInfo {
    System::System() {
        SYSTEM_INFO sysInfo;
        GetSystemInfo(&sysInfo);
        cpuCount = sysInfo.dwNumberOfProcessors;
        prev_idle.resize(cpuCount, 0);
        prev_kernel.resize(cpuCount, 0);
        prev_user.resize(cpuCount, 0);
        EnableDebugPrivilege();
    }

    System::~System() {

    }
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
        // Carrega informações de cada núcleo do processador em um vetor 
        if (NtQuerySystemInformation(SystemProcessorPerformanceInformation, data, sizeof(SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION) * cpuCount, nullptr) != 0) {
            delete[] data;
            FreeLibrary(hNtDll);
            return usages;
        }
        // Calcular o uso de CPU para cada núcleo
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
            // Guardar os valores atuais para o próximo cálculo
            prev_idle[i] = current_idle;
            prev_kernel[i] = current_kernel;
            prev_user[i] = current_user;
        }



        delete[] data;
        FreeLibrary(hNtDll);
        // Calcular o uso total de CPU
        double sum = 0.0;
        for (double u : usages) sum += u;
        totalCpuUsage = sum / cpuCount;

        return usages;
    }

    double System::getCpuUsage() {
        return totalCpuUsage;
    }

    double System::calculateIdleTime() {
        return idleTime;
    }
    // https://learn.microsoft.com/pt-br/windows/win32/api/sysinfoapi/nf-sysinfoapi-globalmemorystatusex
    // https://learn.microsoft.com/pt-br/windows/win32/api/sysinfoapi/ns-sysinfoapi-memorystatusex
    double System::calculateMemoryUsage() {
        MEMORYSTATUSEX memStatus;
        memStatus.dwLength = sizeof(memStatus);

        if (GlobalMemoryStatusEx(&memStatus)) {
            // Porcentagem de memória usada
            // Memória física total - memória física disponível
            usedMemory = memStatus.ullTotalPhys - memStatus.ullAvailPhys;
            return (usedMemory * 100.0) / memStatus.ullTotalPhys;
        } else {
            // Se falhar, retorna 0
            // std::cerr << "GlobalMemoryStatusEx failed: " << GetLastError() << std::endl;
            return 0.0;
        }
    }
    float System::getUsedMemory() {
        return usedMemory / 1024 / 1024 / 1024;
    }
    double System::calculateDiskUsage() {
        return 0.0;
    }
}

