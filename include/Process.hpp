#include <iostream>
#include <list>
#include <string>
#include <iostream>

#include "../include/Thread.hpp"

namespace WindowsInfo {
    class Process {
    private:
        std::string name;
        unsigned int threadCount;
        unsigned int priorityBase;
        unsigned int priorityClass;
        std::list<Thread> threads;
        // modules
        
    public:
        Process(std::string name, unsigned long id, unsigned long parentId, unsigned int threadCount, unsigned int priorityBase);
        ~Process();

        unsigned long parentId;
        unsigned long id;
        std::list<Thread> getThreads();
        unsigned int getPriorityBase();
        unsigned int getPriorityClass();
        unsigned int getThreadCount();
    };
}