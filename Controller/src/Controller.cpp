#include "../include/Controller.hpp"
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <string>

namespace WindowsInfo {
    Controller::Controller() {
        buffer = "Inicial";
        data_ready = false;

        // Inicia a thread de atualização em background
        updater_thread = std::thread(&Controller::updatePipes, this);

        // Inicia o servidor HTTP
        initServer();
    }

    Controller::~Controller() {
        if (updater_thread.joinable()) {
            updater_thread.detach(); // ou implemente flag para parar e faça join
        }
    }

    void Controller::initServer() {
        using namespace std::placeholders;
        app.get_middleware<CORSHandler>();
        CROW_ROUTE(app, "/")([]() {
            return "Hello, World";
        });

        // Endpoint que retorna o conteúdo do buffer protegido
        CROW_ROUTE(app, "/buffer").methods(crow::HTTPMethod::Get)([this]() {
            std::lock_guard<std::mutex> lock(mtx);
            return modelBuffer;
        });

        // Endpoint que retorna o JSON de um processo específico pelo ID
        CROW_ROUTE(app, "/process/<int>").methods(crow::HTTPMethod::Get)([this](int pid) {
            std::lock_guard<std::mutex> lock(mtx);
            return model.get_process_json(pid);
        });
        app.port(8000).multithreaded().run();
    }

    void Controller::updatePipes() {
        int counter = 0;
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(2));
            crow::json::wvalue buffer = model.to_json();
            {
                std::lock_guard<std::mutex> lock(mtx);
                modelBuffer = crow::json::wvalue(buffer);  // força nova instância (cópia)
                data_ready = true;
            }
            cv.notify_all();
        }
    }

}
