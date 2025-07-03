#include "../include/Controller.hpp"
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <string>

namespace WindowsInfo
{
    Controller::Controller()
    {
        buffer = "Inicial";
        data_ready = false;

        // Inicia a thread de atualização em background
        updater_thread = std::thread(&Controller::updatePipes, this);

        // Inicia o servidor HTTP
        initServer();
    }

    Controller::~Controller()
    {
        if (updater_thread.joinable())
        {
            updater_thread.detach(); // ou implemente flag para parar e faça join
        }
    }

    void Controller::initServer()
    {
        using namespace std::placeholders;
        app.get_middleware<CORSHandler>();
        CROW_ROUTE(app, "/")([]()
                             { return "Hello, World"; });

        // Endpoint que retorna o conteúdo do buffer protegido
        CROW_ROUTE(app, "/buffer").methods(crow::HTTPMethod::Get)([this]()
                                                                  {
            std::lock_guard<std::mutex> lock(mtx);
            return modelBuffer; });

        CROW_ROUTE(app, "/volumes").methods(crow::HTTPMethod::Get)([this]()
                                                                   { return FileSystemInfo::SerializeVolumesToJson(this->model.GetVolumesInfo()); });

        CROW_ROUTE(app, "/search")
        ([this](const crow::request &req)
         {
        
        // 1. Tenta obter o valor do parâmetro "term" da URL.
        // O método .get() retorna um ponteiro para char (char*) se o parâmetro existir,
        // ou nullptr se não for encontrado.
        char* search_term_ptr = req.url_params.get("term");

        // 2. Verifica se o parâmetro foi realmente fornecido na URL
        if (search_term_ptr)
        {
            // É uma boa prática converter o char* para std::string para manipulação segura.
            std::string search_term(search_term_ptr);

            // Cria uma resposta de sucesso (HTTP 200 OK)
            std::string response_body = "Você pesquisou pelo termo: '" + search_term + "'";
            
            
        std::vector<FileSystemInfo::FileInfo> filesInfo;


            if (this->model.GetDirectoryInfo(search_term, filesInfo)) {
                return crow::response(400);
            } else {
                return crow::response(200, FileSystemInfo::SerializeFilesInfoToJson(
                        filesInfo
            ));
            }

        }
        
        // 3. Se o parâmetro "term" não foi encontrado, retorna um erro.
        // HTTP 400 Bad Request é o código apropriado para uma requisição malformada.
        return crow::response(400, "Erro: O parâmetro 'term' é obrigatório na query string."); });

        app.bindaddr("0.0.0.0").port(8000).multithreaded().run();
    }

    void Controller::updatePipes()
    {
        int counter = 0;
        while (true)
        {
            std::this_thread::sleep_for(std::chrono::seconds(2));
            crow::json::wvalue buffer = model.to_json();
            {
                std::lock_guard<std::mutex> lock(mtx);
                modelBuffer = crow::json::wvalue(buffer); // força nova instância (cópia)
                data_ready = true;
            }
            cv.notify_all();
        }
    }

}
