#pragma once
#include "../../Model/include/Model.hpp"
#include "../../Include/crow_all.h"
#include "../../Include/ToJson.hpp"
#include "CorsMiddleware.hpp"
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>

namespace WindowsInfo {
    class Controller {
    public:
        Controller();
        ~Controller();

        void initServer();
        void updatePipes();

    private:
        Model model;
        crow::App<CORSHandler> app;
        crow::json::wvalue modelBuffer;
        std::string buffer;
        std::mutex mtx;
        std::condition_variable cv;
        bool data_ready;
        std::thread server_thread;
        std::thread updater_thread;
    };
}