#include "../include/Thread.hpp"

namespace WindowsInfo {
    Thread::Thread(unsigned long id, unsigned int priorityBase, unsigned int priorityDelta) :
        id(id),
        priorityBase(priorityBase),
        priorityDelta(priorityDelta) {
    }
    Thread::~Thread() {
        
    }
    unsigned int Thread::getPriorityBase(){
        return priorityBase;
    }
    unsigned int Thread::getPriorityDelta(){
        return priorityDelta;
    }
    crow::json::wvalue Thread::to_json() {
        crow::json::wvalue j;
        j["id"] = id;
        j["priorityBase"] = priorityBase;
        j["priorityDelta"] = priorityDelta;
        return j;
    }
}