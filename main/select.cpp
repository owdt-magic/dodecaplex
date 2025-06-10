#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <cstdlib>
#include <algorithm>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "window.h"
#include "audio.h"
#include "sharedUniforms.h"

int monitorCount = 1;
int primaryMonitor = 0;

void launch_instances_parallel(const std::string& command, int instanceCount) {
    int m = 0;
    for (int i = 0; i < instanceCount; ++i) {
        std::stringstream ss;
        if (m == primaryMonitor) m++;
        bool fullscreen = (i < monitorCount - 1);
        ss << command
           << " --monitor " << m++
           << (fullscreen ? " --fullscreen" : "")
           << " &";
        system(ss.str().c_str());
    }
}

void cleanup_spawned_processes() {
    std::vector<std::string> processesToKill = {"game", "vortex", "spin"};
    for (const std::string& proc : processesToKill) {
        std::string cmd = "pkill -f " + proc;
        system(cmd.c_str());
    }
}

int main() {
    GLFWwindow* window = simplestWindow(800, 600, "Select Ritual Mode");

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    
    const char* glsl_version = "#version 410";
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    SharedUniforms uniforms(true);

    GLFWmonitor** monitors = glfwGetMonitors(&monitorCount);
    primaryMonitor = 0;

    int instanceCount = 1;
    int selectedDevice = 0;

    ma_context context;
    ma_context_config contextConfig = ma_context_config_init();
    ma_context_init(NULL, 0, &contextConfig, &context);
    auto deviceNames = GetInputDeviceNames(&context);

    int tabIndex = 0;

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Ritual Control");

        if (ImGui::BeginTabBar("MainTabs")) {
            if (ImGui::BeginTabItem("Launch")) {
                ImGui::Text("Launch Visuals");
                ImGui::InputInt("Instances", &instanceCount);
                ImGui::Text("Audio Input:");
                for (size_t i = 0; i < deviceNames.size(); ++i) {
                    if (ImGui::Selectable(deviceNames[i].c_str(), selectedDevice == i)) {
                        selectedDevice = i;
                    }
                }

                if (ImGui::Button("Launch Vortex")) {
                    std::stringstream ss;
                    ss << "./vortex --input " << selectedDevice;
                    launch_instances_parallel(ss.str(), instanceCount);
                }

                if (ImGui::Button("Launch Game")) {
                    std::stringstream ss;
                    ss << "./game --input " << selectedDevice;
                    launch_instances_parallel(ss.str(), instanceCount);
                }

                if (ImGui::Button("Launch Spin")) {
                    std::stringstream ss;
                    ss << "./spin --input " << selectedDevice;
                    launch_instances_parallel(ss.str(), instanceCount);
                }

                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Control")) {
                ImGui::SliderFloat("Scale", &uniforms.data->scale, 0.0f, 1.0f);
                ImGui::SliderFloat("Brightness", &uniforms.data->brightness, 0.0f, 1.0f);
                ImGui::SliderFloat("Speed", &uniforms.data->speed, 0.0f, 1.0f);
                ImGui::SliderFloat("Warp", &uniforms.data->warp, 0.0f, 1.0f);
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

    cleanup_spawned_processes();
    ma_context_uninit(&context);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
