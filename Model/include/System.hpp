#include <stdint.h>
#include <vector>
namespace WindowsInfo {
    class System {
    public:
        System();
        ~System();

        std::vector<double> calculatePerCpuUsage();
        double getCpuUsage();
        double calculateIdleTime();
        double calculateMemoryUsage();
        double calculateDiskUsage();
        int cpuCount;

    private:
        std::vector<uint64_t> prev_idle = {0};
        std::vector<uint64_t> prev_kernel = {0};
        std::vector<uint64_t> prev_user = {0};
        std::vector<uint64_t> cur_idle = {0};
        std::vector<uint64_t> cur_ker = {0};
        std::vector<uint64_t> cur_user = {0};

        double cpuUsage = 0.0;
        double idleTime = 0.0;
    };
}
