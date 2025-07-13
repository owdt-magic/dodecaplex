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
#include "imnodes.h"

// Project-local
#include "audio.h"
#include "sharedUniforms.h"
#include "window.h"
#include "config.h"

std::vector<std::pair<int, int>> links;

ImVec4 bar_colors[BAND_COUNT] = {
    ImVec4(1.0f, 0.2f, 0.2f, 1.0f),
    ImVec4(1.0f, 0.6f, 0.2f, 1.0f),
    ImVec4(0.2f, 1.0f, 0.2f, 1.0f),
    ImVec4(0.2f, 0.2f, 1.0f, 1.0f)
};

// Helper function for color conversion
auto toImU32 = [](const ImVec4& color, int alpha) {
    return IM_COL32((int)(color.x * 255), (int)(color.y * 255), (int)(color.z * 255), alpha);
};

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

    const char* glsl_version = "#version 410";    
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    ImNodes::CreateContext();
    ImNodes::StyleColorsDark();

    SharedUniforms uniforms(true);

    GLFWmonitor** monitors = glfwGetMonitors(&monitorCount);
    primaryMonitor = 0;

    int instanceCount = 1;
    int selectedDeviceIndex = 0;    

    AudioNest audio_nest(selectedDeviceIndex);
    int previousDeviceIndex = selectedDeviceIndex;

    ma_context context;
    ma_context_config contextConfig = ma_context_config_init();
    ma_context_init(NULL, 0, &contextConfig, &context);
    auto deviceNames = GetInputDeviceNames(&context);

    int tabIndex = 0;

    std::vector<std::string> fragShaders = ListFragmentShaders(FRAG_SHADER_DIR);
    int selectedShaderIndex = 0;

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        if (previousDeviceIndex != selectedDeviceIndex) {
            audio_nest.changeAudioDevice(selectedDeviceIndex);
            previousDeviceIndex = selectedDeviceIndex;
        }
        // Process audio FFT and update shared uniforms
        audio_nest.processFFT();
        uniforms.ApplyRouting(audio_nest.g_bandAmplitudes);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if (ImGui::Begin("Launch")) {
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
            
        }
        ImGui::End();

        if (ImGui::Begin("Sliders")) {
            // Lambda function to handle parameter sliders with audio routing
            auto AddNodeDrivenInput = [&](bool routed, const char* label, float* value, float min_val, float max_val, int param_bit) {
                if (routed) ImGui::PushStyleColor(ImGuiCol_SliderGrab, ImVec4(1.0f, 0.5f, 0.0f, 1.0f));
                bool changed = ImGui::SliderFloat(label, value, min_val, max_val);
                if (routed) ImGui::PopStyleColor();
                if (changed) {
                    for (int band = 0; band < BAND_COUNT; ++band) {
                        if (uniforms.data->audio_routing[band] & (1 << param_bit)) {
                            uniforms.data->audio_routing[band] &= ~(1 << param_bit); // Clear this bit
                            links.erase(std::remove_if(links.begin(), links.end(),
                                [band, param_bit](const std::pair<int, int>& l){ 
                                    return l.first == band * 10 + 1 && l.second == 1000 + param_bit; 
                                }), links.end());
                        }
                    }
                }
            };

            // Check if any audio routing is active for each parameter
            bool routed[PARAM_COUNT] = {false};
            for(int i = 0; i < BAND_COUNT; i++) {
                int routing = uniforms.data->audio_routing[i];
                for(int j = 0; j < PARAM_COUNT; j++) {
                    if (routing & (1 << j)) routed[j] = true;
                }
            }
            
            for(int j = 0; j < PARAM_COUNT; j++) {
                UniformMeta& um = uniforms.metadata[j];
                AddNodeDrivenInput(routed[j], um.name, um.value, um.min, um.max, j);
            }

            ImGui::Separator();
            ImGui::Text("Live FFT:");
            float max_amplitude = 2.0f;

            for(int i = 0; i < BAND_COUNT; i++) {
                float amplitude = uniforms.data->audio_bands[i];
                float normalized = amplitude / max_amplitude;

                ImGui::BeginGroup();
                ImGui::PushStyleColor(ImGuiCol_PlotHistogram, bar_colors[i]);
                ImGui::ProgressBar(normalized, ImVec2(-1, 19), "");
                ImGui::PopStyleColor();
                // Overlay the volume slider on top of the histogram
                ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 23); // Move back up to overlay on the ProgressBar
                
                // Make slider background transparent
                ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
                
                ImGui::SetNextItemWidth(-1); // Make the slider take full width
                ImGui::SliderFloat(("##BandVol" + std::to_string(i)).c_str(), &uniforms.data->band_volumes[i], 0.0f, 2.0f, " x %.2f");
                
                ImGui::PopStyleColor();
                ImGui::EndGroup();
            }
            
            ImGui::Separator();
            ImGui::SliderFloat("Volume", &uniforms.data->volume, 0.0f, 2.0f);
            
        }
        ImGui::End();
        
        ImGui::SetNextWindowSize(ImVec2(600, 500), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("Nodes")) {
            ImGui::Text("Patch FFT Bands to Parameters");
            ImNodes::BeginNodeEditor();
            
            // Static variables to track if nodes have been initialized
            static bool nodes_initialized = false;
            static ImVec2 node_positions[BAND_COUNT+PARAM_COUNT];
            if (!nodes_initialized) {
                float left_x = 300.0f;
                float right_x = 400.0f;
                float start_y = 100.0f;
                float node_spacing = 75.0f;
                for (int i = 0; i < BAND_COUNT; ++i) {
                    node_positions[i] = ImVec2(left_x, start_y + i * node_spacing);
                }
                for (int j = 0; j < PARAM_COUNT; ++j) {
                    node_positions[BAND_COUNT + j] = ImVec2(right_x, start_y + j * node_spacing);
                }                
            }
            // Output nodes (bands) - left justified
            for (int i = 0; i < BAND_COUNT; ++i) {
                int color_alpha = 50 + (int)(std::min(uniforms.data->audio_bands[i] / 2.0f, 1.0f) * 200);
                
                ImU32 bg_color       = toImU32(bar_colors[i], color_alpha);
                ImU32 hover_color    = toImU32(bar_colors[i], color_alpha + 50);
                ImU32 selected_color = toImU32(bar_colors[i], color_alpha + 100);
                ImU32 outline_color  = toImU32(bar_colors[i], 255);
                
                ImNodes::PushColorStyle(ImNodesCol_NodeBackground, bg_color);
                ImNodes::PushColorStyle(ImNodesCol_NodeBackgroundHovered, hover_color);
                ImNodes::PushColorStyle(ImNodesCol_NodeBackgroundSelected, selected_color);
                ImNodes::PushColorStyle(ImNodesCol_NodeOutline, outline_color);
                
                ImNodes::BeginNode(i);
                if (!nodes_initialized) ImNodes::SetNodeScreenSpacePos(i, node_positions[i]);
                
                ImNodes::BeginOutputAttribute(i * 10 + 1);
                ImGui::Text("Band %d", i);
                ImNodes::EndOutputAttribute();
                ImNodes::EndNode();
                
                ImNodes::PopColorStyle();
                ImNodes::PopColorStyle();
                ImNodes::PopColorStyle();
                ImNodes::PopColorStyle();
            }
            
            // Input nodes (parameters) - right justified
            for (int j = 0; j < PARAM_COUNT; ++j) {
                ImNodes::BeginNode(100 + j);
                if (!nodes_initialized) ImNodes::SetNodeScreenSpacePos(100 + j, node_positions[BAND_COUNT + j]);
                ImNodes::BeginInputAttribute(1000 + j);
                ImGui::Text("%s", uniforms.metadata[j].name);                
                ImNodes::EndInputAttribute();
                ImNodes::EndNode();
            }
            // Draw all links with colored splines
            for (const auto& link : links) {
                int band_index = link.first / 10;
                bool valid_band = (band_index >= 0 && band_index < BAND_COUNT);
                
                if (valid_band) {
                    int color_alpha = 100 + (int)(std::min(uniforms.data->audio_bands[band_index] / 2.0f, 1.0f) * 155);
                    
                    ImU32 link_color = toImU32(bar_colors[band_index], color_alpha);

                    ImNodes::PushColorStyle(ImNodesCol_Link, link_color);
                    ImNodes::PushColorStyle(ImNodesCol_LinkHovered, toImU32(bar_colors[band_index], 255));
                    ImNodes::PushColorStyle(ImNodesCol_LinkSelected, toImU32(bar_colors[band_index], 255));
                }
                
                ImNodes::Link(link.first * 10000 + link.second, link.first, link.second);
                
                if (valid_band) {
                    ImNodes::PopColorStyle();
                    ImNodes::PopColorStyle();
                    ImNodes::PopColorStyle();
                }
            }
            nodes_initialized = true;
            ImNodes::EndNodeEditor();

            // Now handle new links
            int start_attr, end_attr;
            if (ImNodes::IsLinkCreated(&start_attr, &end_attr)) {
                if (start_attr < 1000 && end_attr >= 1000) {
                    // Remove any previous link from this band to this parameter
                    links.erase(std::remove_if(links.begin(), links.end(),
                        [start_attr, end_attr](const std::pair<int, int>& l){ 
                            return l.first == start_attr && l.second == end_attr; 
                        }), links.end());
                    links.emplace_back(start_attr, end_attr);
                    int band = start_attr / 10;
                    int param_bit = end_attr - 1000; // 0=Scale, 1=Brightness, etc.
                    uniforms.data->audio_routing[band] |= (1 << param_bit); // Set this bit
                }
            }
            
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
    ImNodes::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
