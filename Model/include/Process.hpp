#pragma once
#ifdef _WIN32
#include <winsock2.h>
#include <windows.h>
#endif
#include <crow_all.h>
#include <iostream>
#include <list>
#include <string>
#include <iostream>

#include "../include/Thread.hpp"
#include "../include/Handle.hpp"

namespace WindowsInfo {
    class Process {
    private:
        std::list<Thread> threads;
        std::list<Handle> semaphores;
        std::list<Handle> mutexes;
        std::list<Handle> diskFiles;
        std::list<Handle> charFiles;
        std::list<Handle> pipeFiles;
        std::list<Handle> unknownFiles;
        
        std::list<Handle> directories;
        std::list<Handle> devices;
        std::string userName;
        unsigned int threadCount;
        unsigned int priorityBase;
        unsigned int priorityClass;
        unsigned long long memoryCommitted;
        unsigned long long memoryReserved;
        unsigned long long memoryHeap;
        unsigned long long memoryStack;
        unsigned long long memoryCode;
        unsigned long long numberOfPages;
        unsigned long long memoryWorkingSet;
        unsigned long long memoryPrivateCommited;

        void loadMemoryInfo();
    public:
        void updateInfo();
        Process(std::string name, unsigned long id, unsigned long parentId, unsigned int threadCount, unsigned int priorityBase);
        ~Process();
    
        std::string name;
        unsigned long parentId;
        unsigned long id;
        std::list<Thread> getThreads();
        unsigned int getPriorityBase();
        unsigned int getPriorityClass();
        unsigned int getThreadCount();
        std::string getUserName();
        void loadHandles();
        crow::json::wvalue to_json();
        crow::json::wvalue to_json_simple();
        unsigned long long getMemoryWorkingSet() {
            if (memoryWorkingSet == 0) loadMemoryInfo();
            return memoryWorkingSet;
        }
        unsigned long long getMemoryCommitted() {
            if (memoryCommitted == 0) loadMemoryInfo();
            return memoryCommitted;
        }
        unsigned long long getPrivateMemoryCommitted() {
            if (memoryPrivateCommited == 0) loadMemoryInfo();
            return memoryPrivateCommited;
        }
        unsigned long long getMemoryReserved() {
            if (memoryReserved == 0) loadMemoryInfo();
            return memoryReserved;
        }
        unsigned long long getMemoryHeap() {
            if (memoryHeap == 0) loadMemoryInfo();
            return memoryHeap;
        }
        unsigned long long getMemoryStack() {
            if (memoryStack == 0) loadMemoryInfo();
            return memoryStack;
        }
        unsigned long long getMemoryCode() {
            if (memoryCode == 0) loadMemoryInfo();
            return memoryCode;
        }
        unsigned long long getNumberOfPages() {
            if (numberOfPages == 0) loadMemoryInfo();
            return numberOfPages;
        }
    };
}