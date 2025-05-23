#include <windows.h>
#include <tlhelp32.h>
#include <tchar.h>
#include <stdio.h>
#include <iostream>
#include <list>
#include <vector>

#include "../include/Process.hpp"
#include "../include/System.hpp"

void printError( TCHAR const* msg )
{
  DWORD eNum;
  TCHAR sysMsg[256];
  TCHAR* p;

  eNum = GetLastError( );
  FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
         NULL, eNum,
         MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
         sysMsg, 256, NULL );

  // Trim the end of the line and terminate it with a null
  p = sysMsg;
  while( ( *p > 31 ) || ( *p == 9 ) )
    ++p;
  do { *p-- = 0; } while( ( p >= sysMsg ) &&
                          ( ( *p == '.' ) || ( *p < 33 ) ) );

  // Display the message
  printf( (char*)TEXT("\n  WARNING: %s failed with error %d (%s)"), msg, eNum, sysMsg );
}

int main(void) {
    HANDLE hProcessSnap;
    HANDLE hProcess;
    PROCESSENTRY32 pe32;
    DWORD dwPriorityClass;

    std::list<WindowsInfo::Process> processes;
    WindowsInfo::System systemInfo;
    systemInfo.calculateCpuUsage();
    Sleep(1000);
    std::vector<double> cpuUsages = systemInfo.calculatePerCpuUsage();
    for (int i = 0; i < systemInfo.cpuCount; ++i) {
        std::cout << "CPU " << i << " Usage: " << cpuUsages[i] << "%" << std::endl;
    }
    std::cout << "Idle Time: " << systemInfo.calculateIdleTime() << "%" << std::endl;
    std::cout << "CPU Usage: " << systemInfo.calculateCpuUsage() << "%" << std::endl;
    std::cout << "Memory Usage: " << systemInfo.calculateMemoryUsage() << "%" << std::endl;
    return 0;
    // Take a snapshot of all processes in the system.
    hProcessSnap = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );
    if( hProcessSnap == INVALID_HANDLE_VALUE )
    {
    printError( TEXT("CreateToolhelp32Snapshot (of processes)") );
    return( FALSE );
    }

    // Set the size of the structure before using it.
    pe32.dwSize = sizeof( PROCESSENTRY32 );

    // Retrieve information about the first process,
    // and exit if unsuccessful
    if( !Process32First( hProcessSnap, &pe32 ) )
    {
    printError( TEXT("Process32First") ); // show cause of failure
    CloseHandle( hProcessSnap );          // clean the snapshot object
    return( FALSE );
    }

    // Now walk the snapshot of processes, and
    // display information about each process in turn
    do
    {
        std::wstring ws(pe32.szExeFile);
        std::string name(ws.begin(), ws.end());
        processes.push_back(WindowsInfo::Process(name,
             (unsigned long) pe32.th32ProcessID,
             (unsigned long) pe32.th32ParentProcessID,
             (unsigned int) pe32.cntThreads,
             (unsigned int) pe32.pcPriClassBase));
    // _tprintf( TEXT("\n\n=====================================================" ));
    // _tprintf( TEXT("\nPROCESS NAME:  %s"), pe32.szExeFile );
    // _tprintf( TEXT("\n-------------------------------------------------------" ));

    // Retrieve the priority class.
    dwPriorityClass = 0;
    hProcess = OpenProcess( PROCESS_ALL_ACCESS, FALSE, pe32.th32ProcessID );
    if( hProcess == NULL )
        printError( TEXT("OpenProcess") );
    else
    {
        dwPriorityClass = GetPriorityClass( hProcess );
        if( !dwPriorityClass )
        printError( TEXT("GetPriorityClass") );
        CloseHandle( hProcess );
    }

    // _tprintf( TEXT("\n  Process ID        = 0x%08X"), pe32.th32ProcessID );
    // _tprintf( TEXT("\n  Thread count      = %d"),   pe32.cntThreads );
    // _tprintf( TEXT("\n  Parent process ID = 0x%08X"), pe32.th32ParentProcessID );
    // _tprintf( TEXT("\n  Priority base     = %d"), pe32.pcPriClassBase );
    // if( dwPriorityClass )
    //     _tprintf( TEXT("\n  Priority class    = %d"), dwPriorityClass );

    // List the modules and threads associated with this process
    // ListProcessModules( pe32.th32ProcessID );
    // ListProcessThreads( pe32.th32ProcessID );

    } while( Process32Next( hProcessSnap, &pe32 ) );

    CloseHandle( hProcessSnap );


    int threadCount = 0;
    for (WindowsInfo::Process process : processes) {
        std::cout << "Process Name: " << process.name << std::endl;
        std::cout << "Process ID: " << process.id << std::endl;
        std::cout << "Parent Process ID: " << process.parentId << std::endl;
        std::cout << "Thread Count: " << process.getThreadCount() << std::endl;
        std::cout << "Priority Base: " << process.getPriorityBase() << std::endl;
        std::cout << "Priority Class: " << process.getPriorityClass() << std::endl;
        std::cout << "User Name: " << process.getUserName() << std::endl;
        std::cout << "Memory Committed: " << process.getMemoryCommitted() << std::endl;
        std::cout << "Memory Reserved: " << process.getMemoryReserved() << std::endl;
        std::cout << "Memory Heap: " << process.getMemoryHeap() << std::endl;
        std::cout << "Memory Stack: " << process.getMemoryStack() << std::endl;
        std::cout << "Memory Code: " << process.getMemoryCode() << std::endl;
        std::cout << "Number of Pages: " << process.getNumberOfPages() << std::endl;
        std::cout << "-------------------------------------------------------" << std::endl;
        threadCount += process.getThreadCount();
    }
    std::cout << "Total number of threads: " << threadCount << std::endl;
    return( TRUE );
}