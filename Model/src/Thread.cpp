#include "../include/Thread.hpp"

namespace WindowsInfo {
    Thread::Thread(unsigned long id, unsigned int priorityBase, unsigned int priorityDelta) :
        id(id),
        priorityBase(priorityBase),
        priorityDelta(priorityDelta) {
    }
    Thread::~Thread() {
        
    }
    unsigned int Thread::getPriorityBase() const{
        return priorityBase;
    }
    unsigned int Thread::getPriorityDelta() const{
        return priorityDelta;
    }
}