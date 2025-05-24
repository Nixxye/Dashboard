#include "Model/include/Model.hpp"
#include "View/imgui/backends/imgui_impl_glfw.h"
#include "View/imgui/backends/imgui_impl_opengl3.h"
#include "View/imgui/imgui.h"
#include <Thread>

#include <GLFW/glfw3.h>
#include <chrono>
#include <ratio>
#include <stdio.h>
#include <string>
#include <vector>
#include <winnt.h>
#include <winscard.h>

#define clockInterval 1000 // 5s
#define MegaByte 1024      
#define GigaByte 1'048'576 // 1024*1024

// Estrutura para simular processos
struct MockProcess {
  std::string name;
  unsigned long pid;
  unsigned long threadCount;
  unsigned long priorityBase;
  unsigned long priorityClass;
  unsigned long memoryCommitted;
  unsigned long memoryPrivateCommited;
  unsigned long memoryReserved;
  unsigned long memoryWorkingSet;
  unsigned long numberOfPages;
};
// Buffers duplos para suavizar transição de dados
std::vector<MockProcess> processes[2];
std::vector<std::vector<double>> cpu_usage[2];
float cpu_total_usage[2];
float ram_total_usage[2];
int total_processes[2];
int total_threads[2];
std::vector<double> ram_usage[2];
short bufferSelector = 0;
bool running = true;
// Alterna entre os buffers
void swapViewBuffer() {
  bufferSelector++;
  bufferSelector %= 2;
}
// Função principal da interface com ImGui
void show_gui() {

  ImGui::Begin("Process Monitor");

  ImGui::Text("Running Processes:");
  ImGui::BeginChild("ProcessList", ImVec2(400, 300), true);
  // Exibe lista de processos
  for (auto &proc : processes[bufferSelector]) {
    std::string label =
      std::string(proc.name) + " (PID: " + std::to_string(proc.pid) + ")";
    if (ImGui::TreeNode(label.c_str())) {
      ImGui::Text("-> Process Name: %s", proc.name.c_str());
      ImGui::Text("-> PID: %lu", proc.pid);

      if (proc.memoryWorkingSet > 999'999)
        ImGui::Text("-> Memory Working Set: %.1fGB",
                    (double)proc.memoryWorkingSet / (double)GigaByte);
      else if (proc.memoryWorkingSet > 999)
        ImGui::Text("-> Memory Working Set: %.1fMB",
                    (double)proc.memoryWorkingSet / (double)MegaByte);
      else
        ImGui::Text("-> Memory Working Set: %luKB", proc.memoryWorkingSet);

      if (proc.memoryCommitted > 999'999)
        ImGui::Text("-> Memory Commited: %.1fGB",
                    (double)proc.memoryCommitted / (double)GigaByte);
      else if (proc.memoryCommitted > 999)
        ImGui::Text("-> Memory Commited: %.1fMB",
                    (double)proc.memoryCommitted / (double)MegaByte);
      else
        ImGui::Text("-> Memory Commited: %luKB", proc.memoryCommitted);

      if (proc.memoryPrivateCommited > 999'999)
        ImGui::Text("-> Private Memory Commited: %.1fGB",
                    (double)proc.memoryPrivateCommited / (double)GigaByte);
      else if (proc.memoryPrivateCommited > 999)
        ImGui::Text("-> Private Memory Commited: %.1fMB",
                    (double)proc.memoryPrivateCommited / (double)MegaByte);
      else
        ImGui::Text("-> Private Memory Commited: %luKB", proc.memoryPrivateCommited);

      if (proc.memoryReserved > 999'999)
        ImGui::Text("-> Memory Reserved: %.1fGB",
                    (double)proc.memoryReserved / (double)GigaByte);
      else if (proc.memoryReserved > 999)
        ImGui::Text("-> Memory Reserved: %.1fMB",
                    (double)proc.memoryReserved / (double)MegaByte);
      else
        ImGui::Text("-> Memory Reserved: %luKB", proc.memoryReserved);

      ImGui::Text("-> Number of Pages: %lu", proc.numberOfPages);
      ImGui::Text("-> Priority Base: %lu", proc.priorityBase);
      ImGui::Text("-> Priority Class: %lu", proc.priorityClass);
      ImGui::Text("-> Number of Threads: %lu", proc.threadCount);
      ImGui::TreePop();
    }
  }

  ImGui::EndChild();
  ImGui::Separator();
  // Exibe resumo do sistema
  ImGui::BeginChild("SystemStats", ImVec2(400, 100), true);
  ImGui::Text("System Summary:");
  ImGui::Text("-> Average CPU Usage: %.1f%%", cpu_total_usage[bufferSelector]);
  ImGui::Text("-> Memory Usage: %.2f GB", ram_total_usage[bufferSelector]);
  ImGui::Text("-> Number of Processes: %zu", total_processes[bufferSelector]);
  ImGui::Text("-> Total Threads: %zu", total_threads[bufferSelector]);
  ImGui::EndChild();

  ImGui::Separator();

  // Exibe uso de CPU por núcleo
  const int cpu_count = 12;
  const int columns = 4;

  ImGui::Text("Per-Core CPU Usage:");
  ImGui::Columns(columns, "cpu_columns", false);

  for (int i = 0; i < cpu_count; ++i) {
    std::vector<float> temp;
    for (auto &element : cpu_usage[bufferSelector][i]) {
      temp.push_back(element / 100);
    }
    std::string label = "CPU " + std::to_string(i);
    ImGui::Text("%s", label.c_str());
    ImGui::PlotLines(("##" + label).c_str(), temp.data(),
                      cpu_usage[bufferSelector][i].size(), 0, nullptr, 0.0f,
                      1.0f, ImVec2(-1, 60));
    ImGui::NextColumn();
  }
  ImGui::Columns(1);

  ImGui::Separator();
  // Exibe gráfico de uso de RAM
  ImGui::Text("Total Memory Usage:");
  std::vector<float> temp;
  for (auto &element : ram_usage[bufferSelector]) {
    temp.push_back(element / 100);
  }
  ImGui::PlotLines("##MEM", temp.data(), 50, 0, nullptr, 0.0f, 1.0f, ImVec2(400, 100));

  ImGui::End();
}
// Thread de coleta de dados
using namespace std::chrono_literals;
void teste() {
  WindowsInfo::Model model;
  WindowsInfo::System systemInfo = model.getSystemInfo();
  while (running) {
    int current_index = (bufferSelector + 1) % 2;
    // Copia histórico anterior
    cpu_usage[current_index] = cpu_usage[1 - current_index];
    ram_usage[current_index] = ram_usage[1 - current_index];
    // Atualiza lista de processos
    std::list<WindowsInfo::Process> procList = model.getProcesses();

    std::vector<MockProcess> vet;
    vet.clear();
    total_threads[current_index] = 0;
    for (WindowsInfo::Process p : procList) {
      MockProcess temp;
      temp.pid = p.id;
      temp.name = p.name;
      temp.threadCount = p.getThreadCount();
      temp.priorityBase = p.getPriorityBase();
      temp.priorityClass = p.getPriorityClass();
      temp.memoryCommitted = p.getMemoryCommitted();
      temp.memoryPrivateCommited = p.getPrivateMemoryCommitted();
      temp.memoryReserved = p.getMemoryReserved();
      temp.memoryWorkingSet = p.getMemoryWorkingSet();
      temp.numberOfPages = p.getNumberOfPages();
      vet.push_back(temp);
      total_threads[current_index] += p.getThreadCount();
    }
    processes[current_index] = vet;

    // Atualiza uso por núcleo
    std::vector<double> temp = systemInfo.calculatePerCpuUsage();

    for (int i = 0; i < systemInfo.cpuCount; i++) {
      cpu_usage[current_index][i].push_back((float)temp[i]);
      if (cpu_usage[current_index][i].size() > 50)
        cpu_usage[current_index][i].erase(cpu_usage[current_index][i].begin());
    }

    // Atualiza uso de memória
    ram_usage[current_index].push_back((float)systemInfo.calculateMemoryUsage());
    if (ram_usage[current_index].size() > 50)
      ram_usage[current_index].erase(ram_usage[current_index].begin());

    // Atualiza totais do sistema

    cpu_total_usage[current_index] = ((float)systemInfo.getCpuUsage());
    ram_total_usage[current_index] = (float)systemInfo.getUsedMemory();

    total_processes[current_index] = processes[current_index].size();
     

    swapViewBuffer();
    std::this_thread::sleep_for(std::chrono::milliseconds(clockInterval));
  }
}
int main() {
  if (!glfwInit()) {
    running = false;
    return -1;
  }

  // Inicializa histórico de uso
  WindowsInfo::Model model;
  auto tcpuCount = model.getSystemInfo().cpuCount;
  for (int i = 0; i < tcpuCount; i++) {
    std::vector<double> temp(50, 0.f);
    cpu_usage[0].push_back(temp);
    cpu_usage[1].push_back(temp);
  }
  ram_usage[0].resize(50, 0.f);
  ram_usage[1].resize(50, 0.f);

  // Cria janela GLFW
  GLFWwindow *window = glfwCreateWindow(1280, 720, "Dashboard", NULL, NULL);
  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);

  // Inicializa ImGui
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init("#version 130");

  ImGui::StyleColorsDark();

  // Inicia thread de monitoramento
  std::thread t(teste);

  // Loop principal
  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    show_gui(); // Renderiza a interface

    ImGui::Render();
    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(window);
  }
  
  // Encerra ImGui e GLFW
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
  glfwDestroyWindow(window);
  glfwTerminate();
  running = false;
  return 0;
}
