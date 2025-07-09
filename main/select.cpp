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
int band_to_param[4] = {0, 0, 0, 0};
const char* param_names[] = {"Scale", "Brightness", "Speed", "FOV", "Hue Shift"};

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

    ImNodes::CreateContext();
    ImNodes::StyleColorsDark();

    SharedUniforms uniforms(true);

    GLFWmonitor** monitors = glfwGetMonitors(&monitorCount);
    primaryMonitor = 0;

    int instanceCount = 1;
    int selectedDeviceIndex = 0;    
    // Audio processing
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
        for(int i = 0; i < 4; i++) {
            uniforms.data->audio_bands[i] = audio_nest.g_bandAmplitudes[i];
        }
        
        // Apply audio routing to control parameters
        for(int i = 0; i < 4; i++) {
            int routing = uniforms.data->audio_routing[i];
            float audio_value = uniforms.data->audio_bands[i];
            
            switch(routing) {
                case 1: // Scale
                    uniforms.data->scale = audio_value;
                    break;
                case 2: // Brightness
                    uniforms.data->brightness = audio_value * 2.0f; // Scale to 0-2 range
                    break;
                case 3: // Speed
                    uniforms.data->speed = audio_value * 2.0f; // Scale to 0-2 range
                    break;
                case 4: // FOV
                    uniforms.data->fov = audio_value * 180.0f; // Scale to 0-180 range
                    break;
                case 5: // Hue Shift
                    uniforms.data->hueShift = audio_value * 360.0f; // Scale to 0-360 range
                    break;
            }
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        //ImGui::Begin("Ritual Control");

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

        if (ImGui::Begin("Control")) {
            // Check if any audio routing is active for each parameter
            bool scale_routed = false, brightness_routed = false, speed_routed = false;
            bool fov_routed = false, hue_routed = false;
            
            for(int i = 0; i < 4; i++) {
                switch(uniforms.data->audio_routing[i]) {
                    case 1: scale_routed = true; break;
                    case 2: brightness_routed = true; break;
                    case 3: speed_routed = true; break;
                    case 4: fov_routed = true; break;
                    case 5: hue_routed = true; break;
                }
            }
            
            // Scale
            if (scale_routed) ImGui::PushStyleColor(ImGuiCol_SliderGrab, ImVec4(1.0f, 0.5f, 0.0f, 1.0f));
            bool scale_changed = ImGui::SliderFloat("Scale", &uniforms.data->scale, 0.0f, 1.0f);
            if (scale_routed) ImGui::PopStyleColor();
            if (scale_changed) {
                for (int band = 0; band < 4; ++band) {
                    if (band_to_param[band] == 1) {
                        band_to_param[band] = 0;
                        uniforms.data->audio_routing[band] = 0;
                        links.erase(std::remove_if(links.begin(), links.end(),
                            [band](const std::pair<int, int>& l){ return l.first == band * 10 + 1; }), links.end());
                    }
                }
            }
            // Brightness
            if (brightness_routed) ImGui::PushStyleColor(ImGuiCol_SliderGrab, ImVec4(1.0f, 0.5f, 0.0f, 1.0f));
            bool brightness_changed = ImGui::SliderFloat("Brightness", &uniforms.data->brightness, 0.0f, 2.0f);
            if (brightness_routed) ImGui::PopStyleColor();
            if (brightness_changed) {
                for (int band = 0; band < 4; ++band) {
                    if (band_to_param[band] == 2) {
                        band_to_param[band] = 0;
                        uniforms.data->audio_routing[band] = 0;
                        links.erase(std::remove_if(links.begin(), links.end(),
                            [band](const std::pair<int, int>& l){ return l.first == band * 10 + 1; }), links.end());
                    }
                }
            }
            // Speed
            if (speed_routed) ImGui::PushStyleColor(ImGuiCol_SliderGrab, ImVec4(1.0f, 0.5f, 0.0f, 1.0f));
            bool speed_changed = ImGui::SliderFloat("Speed", &uniforms.data->speed, 0.0f, 2.0f);
            if (speed_routed) ImGui::PopStyleColor();
            if (speed_changed) {
                for (int band = 0; band < 4; ++band) {
                    if (band_to_param[band] == 3) {
                        band_to_param[band] = 0;
                        uniforms.data->audio_routing[band] = 0;
                        links.erase(std::remove_if(links.begin(), links.end(),
                            [band](const std::pair<int, int>& l){ return l.first == band * 10 + 1; }), links.end());
                    }
                }
            }
            // FOV
            if (fov_routed) ImGui::PushStyleColor(ImGuiCol_SliderGrab, ImVec4(1.0f, 0.5f, 0.0f, 1.0f));
            bool fov_changed = ImGui::SliderFloat("FOV", &uniforms.data->fov, 0.0f, 180.0f);
            if (fov_routed) ImGui::PopStyleColor();
            if (fov_changed) {
                for (int band = 0; band < 4; ++band) {
                    if (band_to_param[band] == 4) {
                        band_to_param[band] = 0;
                        uniforms.data->audio_routing[band] = 0;
                        links.erase(std::remove_if(links.begin(), links.end(),
                            [band](const std::pair<int, int>& l){ return l.first == band * 10 + 1; }), links.end());
                    }
                }
            }
            // Hue Shift
            if (hue_routed) ImGui::PushStyleColor(ImGuiCol_SliderGrab, ImVec4(1.0f, 0.5f, 0.0f, 1.0f));
            bool hue_changed = ImGui::SliderFloat("Hue Shift", &uniforms.data->hueShift, 0.0f, 360.0f);
            if (hue_routed) ImGui::PopStyleColor();
            if (hue_changed) {
                for (int band = 0; band < 4; ++band) {
                    if (band_to_param[band] == 5) {
                        band_to_param[band] = 0;
                        uniforms.data->audio_routing[band] = 0;
                        links.erase(std::remove_if(links.begin(), links.end(),
                            [band](const std::pair<int, int>& l){ return l.first == band * 10 + 1; }), links.end());
                    }
                }
            }
                        // Sync with sharedUniforms
            for (int i = 0; i < 4; ++i) {
                uniforms.data->audio_routing[i] = band_to_param[i];
            }
            ImGui::Separator();
            ImGui::Text("Live FFT:");
            float max_amplitude = 2.0f;
            ImVec4 bar_colors[] = {
                ImVec4(1.0f, 0.2f, 0.2f, 1.0f),
                ImVec4(1.0f, 0.6f, 0.2f, 1.0f),
                ImVec4(0.2f, 1.0f, 0.2f, 1.0f),
                ImVec4(0.2f, 0.2f, 1.0f, 1.0f)
            };
            for(int i = 0; i < 4; i++) {
                float amplitude = uniforms.data->audio_bands[i];
                float normalized = amplitude / max_amplitude;
                ImGui::PushStyleColor(ImGuiCol_PlotHistogram, bar_colors[i]);
                ImGui::ProgressBar(normalized, ImVec2(-1, 20), "");
                ImGui::PopStyleColor();
            }
            
        }
        ImGui::End();
        
        ImGui::SetNextWindowSize(ImVec2(600, 500), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("Audio")) {
            ImGui::Text("Patch FFT Bands to Parameters");
            ImNodes::BeginNodeEditor();
            
            // Static variables to track if nodes have been initialized
            static bool nodes_initialized = false;
            static ImVec2 node_positions[9]; // 4 bands + 5 parameters
            
            // Initialize node positions only once
            if (!nodes_initialized) {
                std::cout << "Initializing nodes" << std::endl;
                
                // Use better coordinates for ImNodes editor
                float left_x = 100.0f;
                float right_x = 400.0f;
                float start_y = 180.0f;
                float node_spacing = 75.0f;
                
                // Set initial positions for output nodes (bands) - left side
                for (int i = 0; i < 4; ++i) {
                    node_positions[i] = ImVec2(left_x, start_y + i * node_spacing);
                }
                
                // Set initial positions for input nodes (parameters) - right side
                for (int j = 0; j < 5; ++j) {
                    node_positions[4 + j] = ImVec2(right_x, start_y + j * node_spacing);
                }
                
            }
            
            // Define FFT colors once (matching the FFT display)
            ImVec4 bar_colors[] = {
                ImVec4(1.0f, 0.2f, 0.2f, 1.0f),  // Red
                ImVec4(1.0f, 0.6f, 0.2f, 1.0f),  // Orange
                ImVec4(0.2f, 1.0f, 0.2f, 1.0f),  // Green
                ImVec4(0.2f, 0.2f, 1.0f, 1.0f)   // Blue
            };
            
            // Output nodes (bands) - left justified
            for (int i = 0; i < 4; ++i) {
                // Set node colors to match FFT display colors BEFORE BeginNode
                ImVec4 color = bar_colors[i];
                
                // Calculate opacity based on FFT amplitude
                float amplitude = uniforms.data->audio_bands[i];
                float max_amplitude = 2.0f; // Same as FFT display
                float normalized_amplitude = std::min(amplitude / max_amplitude, 1.0f);
                int base_alpha = 50; // Minimum opacity
                int amplitude_alpha = (int)(normalized_amplitude * 200); // Additional opacity from amplitude
                int total_alpha = base_alpha + amplitude_alpha;
                
                ImU32 bg_color = IM_COL32((int)(color.x * 255), (int)(color.y * 255), (int)(color.z * 255), total_alpha);
                ImU32 hover_color = IM_COL32((int)(color.x * 255), (int)(color.y * 255), (int)(color.z * 255), total_alpha + 50);
                ImU32 selected_color = IM_COL32((int)(color.x * 255), (int)(color.y * 255), (int)(color.z * 255), total_alpha + 100);
                ImU32 outline_color = IM_COL32((int)(color.x * 255), (int)(color.y * 255), (int)(color.z * 255), 255);
                
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
            for (int j = 0; j < 5; ++j) {
                ImNodes::BeginNode(100 + j);
                if (!nodes_initialized) ImNodes::SetNodeScreenSpacePos(100 + j, node_positions[4 + j]);
                ImNodes::BeginInputAttribute(1000 + j);
                ImGui::Text("%s", param_names[j]);                
                ImNodes::EndInputAttribute();
                ImNodes::EndNode();
            }
            // Draw all links with colored splines
            for (const auto& link : links) {
                // Determine which band this link comes from
                int band_index = link.first / 10; // Extract band index from attribute ID
                
                if (band_index >= 0 && band_index < 4) {
                    // Get the color for this band
                    ImVec4 color = bar_colors[band_index];
                    
                    // Calculate opacity based on FFT amplitude
                    float amplitude = uniforms.data->audio_bands[band_index];
                    float max_amplitude = 2.0f;
                    float normalized_amplitude = std::min(amplitude / max_amplitude, 1.0f);
                    int base_alpha = 100; // Minimum opacity for links
                    int amplitude_alpha = (int)(normalized_amplitude * 155); // Additional opacity from amplitude
                    int total_alpha = base_alpha + amplitude_alpha;
                    
                    ImU32 link_color = IM_COL32((int)(color.x * 255), (int)(color.y * 255), (int)(color.z * 255), total_alpha);
                    
                    // Push link color style
                    ImNodes::PushColorStyle(ImNodesCol_Link, link_color);
                    ImNodes::PushColorStyle(ImNodesCol_LinkHovered, IM_COL32((int)(color.x * 255), (int)(color.y * 255), (int)(color.z * 255), 255));
                    ImNodes::PushColorStyle(ImNodesCol_LinkSelected, IM_COL32((int)(color.x * 255), (int)(color.y * 255), (int)(color.z * 255), 255));
                    
                    ImNodes::Link(link.first * 10000 + link.second, link.first, link.second);
                    
                    // Pop link color styles
                    ImNodes::PopColorStyle();
                    ImNodes::PopColorStyle();
                    ImNodes::PopColorStyle();
                } else {
                    // Fallback for any unexpected links
                    ImNodes::Link(link.first * 10000 + link.second, link.first, link.second);
                }
            }
            nodes_initialized = true;
            ImNodes::EndNodeEditor();

            // Now handle new links
            int start_attr, end_attr;
            if (ImNodes::IsLinkCreated(&start_attr, &end_attr)) {
                if (start_attr < 1000 && end_attr >= 1000) {
                    // Remove any previous link from this band
                    links.erase(std::remove_if(links.begin(), links.end(),
                        [start_attr](const std::pair<int, int>& l){ return l.first == start_attr; }), links.end());
                    links.emplace_back(start_attr, end_attr);
                    int band = start_attr / 10;
                    int param = end_attr - 1000 + 1; // 1=Scale, 2=Brightness, etc.
                    band_to_param[band] = param;
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
