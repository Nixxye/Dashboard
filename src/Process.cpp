#include <windows.h>
#include <tlhelp32.h>
#include <tchar.h>
#include <iostream>

#include "../include/Process.hpp"

namespace WindowsInfo {
    Process::Process(std::string name, unsigned long id, unsigned long parentId, unsigned int threadCount, unsigned int priorityBase) :
    name(name),
    id(id),
    parentId(parentId),
    threadCount(threadCount),
    priorityBase(priorityBase),
    priorityClass(0),
    threads() {
        HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, id);
        if( hProcess == NULL )
            std::cerr << "OpenProcess failed: " << GetLastError() << std::endl;
            // printError( TEXT("OpenProcess") );
        else {
            priorityClass = GetPriorityClass( hProcess );
            if( !priorityClass )
                std::cerr << "GetPriorityClass failed: " << GetLastError() << std::endl;
                // printError( TEXT("GetPriorityClass") );
            CloseHandle( hProcess );
        }
    }
    Process::~Process() {
        
    }
    std::list<Thread> Process::getThreads() {
        if (threads.size() > 0) {
            return threads;
        }
        HANDLE hThreadSnap = INVALID_HANDLE_VALUE; 
        THREADENTRY32 te32; 
        // Take a snapshot of all running threads  
        hThreadSnap = CreateToolhelp32Snapshot( TH32CS_SNAPTHREAD, 0 ); 
        if( hThreadSnap == INVALID_HANDLE_VALUE ) 
            return std::list<Thread>(); 
        
        // Fill in the size of the structure before using it. 
        te32.dwSize = sizeof(THREADENTRY32); 
        
        // Retrieve information about the first thread,
        // and exit if unsuccessful
        if( !Thread32First( hThreadSnap, &te32 ) ) 
        {
            std::cerr << "Thread32First failed: " << GetLastError() << std::endl;
            // printError( TEXT("Thread32First") ); // show cause of failure
            CloseHandle( hThreadSnap );          // clean the snapshot object
            return std::list<Thread>(); 
        }

        // Now walk the thread list of the system,
        // and display information about each thread
        // associated with the specified process
        do 
        { 
            if( te32.th32OwnerProcessID == id )
            {
                Thread thread = Thread(te32.th32ThreadID, te32.tpBasePri, te32.tpDeltaPri);
                threads.push_back(thread);
            }
        } while( Thread32Next(hThreadSnap, &te32 ) ); 

        CloseHandle( hThreadSnap );
        return threads;
    }
    unsigned int Process::getPriorityBase() {
        return priorityBase;
    }
    unsigned int Process::getPriorityClass() {
        return priorityClass;
    }
    unsigned int Process::getThreadCount() {
        return threadCount;
    }
}