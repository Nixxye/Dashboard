#pragma once
#include <crow_all.h>
#include <string>

namespace WindowsInfo {
    class Thread {
    private:
        unsigned int priorityBase;
        unsigned int priorityDelta;
    
    public:
        Thread(unsigned long id, unsigned int priorityBase, unsigned int priorityDelta);
        ~Thread();

        unsigned long id;
        unsigned int getPriorityBase();
        unsigned int getPriorityDelta();
        crow::json::wvalue to_json();
    };
}