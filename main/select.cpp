#include <cstdlib>
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include <filesystem>
#include <spawn.h>
#include <signal.h>
#include <sys/wait.h>
extern char **environ;

// ImGui
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

// Project-local
#include "audio.h"
#include "sharedUniforms.h"
#include "window.h"
#include "config.h"

int monitorCount = 1;
int primaryMonitor = 0;

std::vector<pid_t> fragment_pids;

std::vector<std::string> ListFragmentShaders(const std::string& directory) {
    std::vector<std::string> files;
    for (const auto& entry : std::filesystem::directory_iterator(directory)) {
        if (entry.is_regular_file() && entry.path().extension() == ".frag") {
            files.push_back(entry.path().filename().string());
        }
    }
    return files;
}

void launch_fragment(const std::string& cmd, int instanceCount) {
    for (int i = 0; i < instanceCount; ++i) {
        pid_t pid;
        std::vector<std::string> args = {"/bin/sh", "-c", cmd};
        std::vector<char*> argv;
        for (auto& arg : args) argv.push_back(&arg[0]);
        argv.push_back(nullptr);

        int status = posix_spawn(&pid, "/bin/sh", nullptr, nullptr, argv.data(), environ);
        if (status == 0) {
            fragment_pids.push_back(pid);
        } else {
            std::cerr << "Failed to spawn: " << status << std::endl;
        }
    }
}

void kill_fragments() {
    for (pid_t pid : fragment_pids) {
        kill(pid, SIGTERM);
        waitpid(pid, nullptr, 0); // Reap the process
    }
    fragment_pids.clear();
}

void cleanup_spawned_processes() {
    std::vector<std::string> processesToKill = {"game", "fragment", "spin"};
    for (const std::string& proc : processesToKill) {
        std::string cmd = "pkill -f " + proc;
        system(cmd.c_str());
    }
}

template<typename T>
bool dropDown(const std::vector<T>& options, const char* label, int& selectedIndex) {
    bool changed = false;
    if (ImGui::BeginCombo(label, options[selectedIndex].c_str())) {
        for (int i = 0; i < options.size(); ++i) {
            bool is_selected = (selectedIndex == i);
            if (ImGui::Selectable(options[i].c_str(), is_selected)) {
                selectedIndex = i;
                changed = true;
            }
            if (is_selected) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
    return changed;
}

int main() {
    GLFWwindow* window = simplestWindow(800, 600, "Select Ritual Mode");

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGuiIO& io = ImGui::GetIO();
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    colors[ImGuiCol_WindowBg]      = ImVec4(0.04f, 0.01f, 0.06f, 1.00f);
    colors[ImGuiCol_Text]          = ImVec4(0.85f, 0.75f, 0.95f, 1.00f);
    colors[ImGuiCol_TitleBg]       = ImVec4(0.1f, 0.0f, 0.15f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.3f, 0.0f, 0.4f, 1.00f);
    colors[ImGuiCol_FrameBg]       = ImVec4(0.2f, 0.1f, 0.25f, 1.00f);
    colors[ImGuiCol_Button]        = ImVec4(0.25f, 0.10f, 0.30f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.45f, 0.25f, 0.55f, 1.00f);
    colors[ImGuiCol_Border]        = ImVec4(0.4f, 0.0f, 0.6f, 0.4f);
    
    io.Fonts->AddFontFromFileTTF(GLOBAL_FONT, 30.0f);
    
    const char* glsl_version = "#version 410";    
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    SharedUniforms uniforms(true);

    GLFWmonitor** monitors = glfwGetMonitors(&monitorCount);
    primaryMonitor = 0;

    int instanceCount = 1;
    int selectedDeviceIndex = 0;

    ma_context context;
    ma_context_config contextConfig = ma_context_config_init();
    ma_context_init(NULL, 0, &contextConfig, &context);
    auto deviceNames = GetInputDeviceNames(&context);

    int tabIndex = 0;

    std::vector<std::string> fragShaders = ListFragmentShaders(FRAG_SHADER_DIR);
    int selectedShaderIndex = 0;

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Ritual Control");

        if (ImGui::BeginTabBar("MainTabs")) {
            if (ImGui::BeginTabItem("Launch")) {
                if (ImGui::Button("Launch Game")) {
                    system("./game");
                }
                
                ImGui::InputInt("Instances", &instanceCount);
                dropDown(deviceNames, "Audio Input", selectedDeviceIndex);
                
                if (ImGui::Button("Launch Spin")) {
                    std::stringstream ss;
                    ss << "./spin --input " << selectedDeviceIndex;
                    launch_fragment(ss.str(), instanceCount);
                }

                dropDown(fragShaders, "Fragment Shader", selectedShaderIndex);

                if (ImGui::Button("Launch Fragment")) {
                    std::stringstream ss;
                    ss << "./fragment --input " << selectedDeviceIndex
                        << " --shader " << FRAG_SHADER_DIR << "/" << fragShaders[selectedShaderIndex].c_str();
                    launch_fragment(ss.str(), instanceCount);
                }

                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Control")) {
                ImGui::SliderFloat("Scale", &uniforms.data->scale, 0.0f, 1.0f);
                ImGui::SliderFloat("Brightness", &uniforms.data->brightness, 0.0f, 2.0f);
                ImGui::SliderFloat("Speed", &uniforms.data->speed, 0.0f, 2.0f);
                ImGui::SliderFloat("FOV", &uniforms.data->fov, 0.0f, 180.0f);
                ImGui::SliderFloat("Hue Shift", &uniforms.data->hueShift, 0.0f, 360.0f);
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }

        ImGui::End();

        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    kill_fragments();
    ma_context_uninit(&context);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
