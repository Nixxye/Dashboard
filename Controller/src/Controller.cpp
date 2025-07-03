#include "../include/Controller.hpp"
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <string>
#include <thread>

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

std::pair<std::string, int>
Controller::readConfig(std::string filename) {
  std::ifstream file(filename);
  std::string line;
  std::unordered_map<std::string, std::string> config;

  if (!file.is_open()) {
    throw std::runtime_error("Failed to open config file: " + filename);
  }

  while (std::getline(file, line)) {
    std::istringstream iss(line);
    std::string key, value;

    // Only parse lines of the form KEY=VALUE
    if (std::getline(iss, key, '=') && std::getline(iss, value)) {
      config[key] = value;
    }
  }

  file.close();

  if (config.find("IP") == config.end() ||
      config.find("PORT") == config.end()) {
    throw std::runtime_error("Missing IP or PORT in config file");
  }

  std::string ip = config["IP"];
  int port = std::stoi(config["PORT"]);

  return {ip, port};
}

void Controller::initServer() {
  using namespace std::placeholders;
  app.get_middleware<CORSHandler>();
  CROW_ROUTE(app, "/")([]() { return "Hello, World"; });

  // Endpoint que retorna o conteúdo do buffer protegido
  CROW_ROUTE(app, "/buffer").methods(crow::HTTPMethod::Get)([this]() {
    std::lock_guard<std::mutex> lock(mtx);
    return modelBuffer;
  });

  CROW_ROUTE(app, "/volumes").methods(crow::HTTPMethod::Get)([this]() {
    return FileSystemInfo::SerializeVolumesToJson(this->model.GetVolumesInfo());
  });

  CROW_ROUTE(app, "/search")
  ([this](const crow::request &req) {
    // 1. Tenta obter o valor do parâmetro "term" da URL.
    // O método .get() retorna um ponteiro para char (char*) se o parâmetro
    // existir, ou nullptr se não for encontrado.
    char *search_term_ptr = req.url_params.get("term");

    // 2. Verifica se o parâmetro foi realmente fornecido na URL
    if (search_term_ptr) {
      // É uma boa prática converter o char* para std::string para manipulação
      // segura.
      std::string search_term(search_term_ptr);

      // Cria uma resposta de sucesso (HTTP 200 OK)
      std::string response_body =
          "Você pesquisou pelo termo: '" + search_term + "'";

      std::vector<FileSystemInfo::FileInfo> filesInfo;

      if (this->model.GetDirectoryInfo(search_term, filesInfo)) {
        return crow::response(400);
      } else {
        return crow::response(
            200, FileSystemInfo::SerializeFilesInfoToJson(filesInfo));
      }
    }

    // 3. Se o parâmetro "term" não foi encontrado, retorna um erro.
    // HTTP 400 Bad Request é o código apropriado para uma requisição
    // malformada.
    return crow::response(
        400, "Erro: O parâmetro 'term' é obrigatório na query string.");
  });

  // Endpoint que retorna o JSON de um processo específico pelo ID
  CROW_ROUTE(app, "/process/<int>")
      .methods(crow::HTTPMethod::Get)([this](int pid) {
        std::cout << "Requisição recebida para o processo ID: " << pid
                  << std::endl;
        return model.get_process_json(pid);
      });
  auto [address, port] = readConfig("Controller/src/controller.config");
  app.bindaddr(address).port(port).multithreaded().run();
}

void Controller::updatePipes() {
  int counter = 0;
  while (true) {
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
} // namespace WindowsInfo
