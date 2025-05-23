#include "Front/imgui/imgui.h"
#include "Front/imgui/backends/imgui_impl_glfw.h"
#include "Front/imgui/backends/imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <string>

void show_gui() {
    ImGui::Begin("Process Monitor");

    ImGui::Text("Running Processes:");
    ImGui::BeginChild("ProcessList", ImVec2(400, 300), true);

    // Mock process list
    struct MockProcess {
        const char* name;
        int pid;
        float cpu;
        float mem;
    };

    static MockProcess processes[] = {
        { "chrome.exe", 1234, 0.12f, 0.34f },
        { "explorer.exe", 5678, 0.05f, 0.20f },
        { "code.exe", 9012, 0.25f, 0.45f },
        { "discord.exe", 3456, 0.07f, 0.30f },
        { "steam.exe", 7890, 0.15f, 0.50f },
    };

    for (auto& proc : processes) {
        std::string label = std::string(proc.name) + " (PID: " + std::to_string(proc.pid) + ")";
        if (ImGui::TreeNode(label.c_str())) {
            ImGui::Text("-> Process Name: %s", proc.name);
            ImGui::Text("-> PID: %d", proc.pid);
            ImGui::Text("-> CPU Usage: %.2f%%", proc.cpu * 100);
            ImGui::Text("-> Memory Usage: %.2f%%", proc.mem * 100);
            ImGui::TreePop();
        }
    }

    ImGui::EndChild();
    ImGui::Separator();

    // CPU Usage Grid
    const int cpu_count = 12;               // mock 8 logical CPUs
    const int columns = 4;                 // grid layout: 4 columns
    const int graph_size = 50;             // history length
    static float cpu_usage[cpu_count][graph_size] = {};

    ImGui::Text("Per-Core CPU Usage:");
    ImGui::Columns(columns, "cpu_columns", false);
    for (int i = 0; i < cpu_count; ++i) {
        // Scroll data
        memmove(cpu_usage[i], cpu_usage[i] + 1, sizeof(float) * (graph_size - 1));
        cpu_usage[i][graph_size - 1] = (rand() % 100) / 100.0f;  // mock value

        std::string label = "CPU " + std::to_string(i);
        ImGui::Text("%s", label.c_str());
        ImGui::PlotLines(("##" + label).c_str(), cpu_usage[i], graph_size, 0, nullptr, 0.0f, 1.0f, ImVec2(-1, 60));
        ImGui::NextColumn();
    }
    ImGui::Columns(1); // reset

    ImGui::Separator();

    ImGui::Text("Total Memory Usage:");
    static float mem[50] = {};
    memmove(mem, mem + 1, sizeof(mem) - sizeof(float));
    mem[49] = (rand() % 100) / 100.0f;
    ImGui::PlotLines("##MEM", mem, 50, 0, nullptr, 0.0f, 1.0f, ImVec2(400, 100));

    ImGui::End();
}



int main() {
    if (!glfwInit()) return -1;

    GLFWwindow* window = glfwCreateWindow(1280, 720, "ImGui Example", NULL, NULL);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    ImGui::StyleColorsDark();

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        show_gui();

        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
