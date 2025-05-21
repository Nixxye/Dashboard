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
    };
}