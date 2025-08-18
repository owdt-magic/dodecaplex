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
#include <cmath>
#include <fstream>
#include <nlohmann/json.hpp>
extern char **environ;


// ImGui
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include "imnodes.h"
#include "graphicsPipe.h"
// Project-local
#include "audio.h"
#include "sharedUniforms.h"
#include "config.h"
#include "attributeSystem.h"
#include "guiNodes.h"

using namespace AttributeHelpers;

ImVec4 bar_colors[BAND_COUNT] = {
    ImVec4(1.0f, 0.2f, 0.2f, 1.0f),
    ImVec4(1.0f, 0.6f, 0.2f, 1.0f),
    ImVec4(0.2f, 1.0f, 0.2f, 1.0f),
    ImVec4(0.2f, 0.2f, 1.0f, 1.0f)
};

// Value generator node color
ImVec4 value_generator_color = ImVec4(0.8f, 0.2f, 0.8f, 1.0f); // Purple/magenta

// Unified value source manager
ValueSourceManager valueManager;

// Deferred removal system
std::vector<int> sourcesToRemove;

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

// Helper functions for saving/loading ImNodes layout
void SaveNodeLayoutSlot(int slot) {
    std::filesystem::create_directories("tmp"); // Ensure tmp/ exists
    size_t data_size = 0;
    const char* ini_str = ImNodes::SaveCurrentEditorStateToIniString(&data_size);
    if (!ini_str || data_size == 0) return;
    std::string ini_filename = "tmp/nodes_config_slot" + std::to_string(slot) + ".ini";
    std::ofstream ofs(ini_filename, std::ios::binary);
    if (ofs) {
        ofs.write(ini_str, data_size);
        ofs.close();
    }
    // Save node graph as JSON
    std::string json_filename = "tmp/nodes_config_slot" + std::to_string(slot) + ".json";
    nlohmann::json j = valueManager.to_json();
    std::ofstream jfs(json_filename);
    if (jfs) {
        jfs << j.dump(2);
        jfs.close();
    }
}

void LoadNodeLayoutSlot(int slot, SharedUniforms& uniforms) {
    // Load node graph from JSON
    std::string json_filename = "tmp/nodes_config_slot" + std::to_string(slot) + ".json";
    std::ifstream jfs(json_filename);
    if (jfs) {
        nlohmann::json j;
        jfs >> j;
        valueManager.from_json(j, uniforms.data->audio_bands);
        jfs.close();
    }
    // Load ImNodes layout
    std::string ini_filename = "tmp/nodes_config_slot" + std::to_string(slot) + ".ini";
    std::ifstream ifs(ini_filename, std::ios::binary);
    if (ifs) {
        std::string data((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
        if (!data.empty()) {
            ImNodes::LoadCurrentEditorStateFromIniString(data.c_str(), data.size());
        }
        ifs.close();
    }
}

int main() {
    GLFWwindow* window = initializeWindow(800, 600, "Select Ritual Mode", false, 0);

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    // Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

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

    // Initialize unified value source system
    // Add audio band sources
    for (int i = 0; i < BAND_COUNT; ++i) {
        valueManager.addSource(std::make_unique<AudioBandSource>(i, &uniforms.data->audio_bands[i]));
    }
    
    // Add initial value generator
    valueManager.addSource(std::make_unique<MultiModeValueGenerator>());

    // Time tracking
    static float lastTime = 0.0f;
    float currentTime = glfwGetTime();
    float deltaTime = currentTime - lastTime;
    lastTime = currentTime;
    
   GraphicsPipe* graphicsPipe = nullptr;

    while (!glfwWindowShouldClose(window)) {
        if (graphicsPipe != nullptr) {            
            graphicsPipe->renderNextFrame(false);
        }

        // Update time
        glfwPollEvents();
        
        // Update unified value source system
        valueManager.updateAll(deltaTime);

        if (previousDeviceIndex != selectedDeviceIndex) {
            audio_nest.changeAudioDevice(selectedDeviceIndex);
            previousDeviceIndex = selectedDeviceIndex;
        }
        // Process audio FFT and update shared uniforms
        audio_nest.processFFT();
        uniforms.ApplyRouting(audio_nest.g_bandAmplitudes);
        
        // Apply unified value sources to parameters
        valueManager.applyToParameters(uniforms.metadata, PARAM_COUNT);

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
                 CLAs clas;
                int instancesLeft = instanceCount;
                if (graphicsPipe == nullptr) {
                    clas.fullscreen = false;
                    clas.monitorIndex = 0;
                    clas.audioIndex = selectedDeviceIndex;
                    graphicsPipe = new GraphicsPipe(PipeType::SPIN, clas);
                    graphicsPipe->initHere(window);
                    graphicsPipe->establishShaders();
                    instancesLeft -= 1;
                }
                std::stringstream ss;
                ss << "./spin --input " << selectedDeviceIndex;
                launch_fragment(ss.str(), instancesLeft);
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
            // Lambda function to handle parameter sliders with unified routing
            auto AddNodeDrivenInput = [&](bool routed, const char* label, float* value, float min_val, float max_val, int param_bit) {
                if (routed) ImGui::PushStyleColor(ImGuiCol_SliderGrab, ImVec4(1.0f, 0.5f, 0.0f, 1.0f));
                bool changed = ImGui::SliderFloat(label, value, min_val, max_val);
                if (routed) ImGui::PopStyleColor();
                if (changed) {
                    // Override any node linking with the value selected
                    valueManager.removeLink(-1, param_bit); // Remove all links to this parameter
                }
            };
            
            // Use unified system to check if parameters are driven
            for(int j = 0; j < PARAM_COUNT; j++) {
                UniformMeta& um = uniforms.metadata[j];
                bool isDriven = valueManager.isParameterDriven(j);
                AddNodeDrivenInput(isDriven, um.name, um.value, um.min, um.max, j);
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
            // Add Generator button
            if (ImGui::Button("Add Generator")) {
                valueManager.addSource(std::make_unique<MultiModeValueGenerator>());
            }
            
            ImNodes::BeginNodeEditor();
            
            // Static variables to track if nodes have been initialized
            static bool nodes_initialized = false;
            static ImVec2 node_positions[100]; // Large enough for all sources + parameters
            if (!nodes_initialized) {
                float left_x = 300.0f;
                float right_x = 400.0f;
                float start_y = 100.0f;
                float node_spacing = 75.0f;
                
                // Position all value sources on the left
                for (int i = 0; i < valueManager.getSourceCount(); ++i) {
                    node_positions[i] = ImVec2(left_x, start_y + i * node_spacing);
                }
                
                const int rows = 4;
                float param_spacing_x = 120.0f;
                float param_spacing_y = 75.0f;
                
                for (int j = 0; j < PARAM_COUNT; ++j) {
                    int row = j / rows;
                    int col = j % rows;
                    float x = right_x + row * param_spacing_x;
                    float y = start_y + col * param_spacing_y;
                    node_positions[valueManager.getSourceCount() + j] = ImVec2(x, y);
                }                
            }
            // Output nodes (value sources) - left justified
            for (int i = 0; i < valueManager.getSourceCount(); ++i) {
                ValueSource* source = valueManager.getSourceByIndex(i);
                if (!source) continue;
                
                int s_id = source->getSourceId();
                ImVec4 sourceColor = (s_id < BAND_COUNT) ? bar_colors[s_id] : value_generator_color;                
                int color_alpha = 50 + (int)(source->getNormalizedValue() * 200);
                
                ImU32 bg_color       = toImU32(sourceColor, color_alpha);
                ImU32 hover_color    = toImU32(sourceColor, color_alpha + 50);
                ImU32 selected_color = toImU32(sourceColor, color_alpha + 100);
                ImU32 outline_color  = toImU32(sourceColor, 255);
                
                ImNodes::PushColorStyle(ImNodesCol_NodeBackground, bg_color);
                ImNodes::PushColorStyle(ImNodesCol_NodeBackgroundHovered, hover_color);
                ImNodes::PushColorStyle(ImNodesCol_NodeBackgroundSelected, selected_color);
                ImNodes::PushColorStyle(ImNodesCol_NodeOutline, outline_color);
                
                ImNodes::BeginNode(source->getSourceId());
                if (!nodes_initialized) ImNodes::SetNodeScreenSpacePos(source->getSourceId(), node_positions[i]);
                
                // Add X button for value generators (not audio bands)
                if (source->getSourceId() >= BAND_COUNT) {                    
                    if (ImGui::Button(("X##" + std::to_string(source->getSourceId())).c_str())) {
                        sourcesToRemove.push_back(source->getSourceId());
                    }
                    ImGui::SameLine();
                }
                // Render source-specific UI                
                source->renderUI();

                // Create output attribute
                if (source->getSourceId() < BAND_COUNT) {
                    // Audio band
                    ImNodes::BeginOutputAttribute(getAudioBandAttributeId(source->getSourceId()));
                    ImGui::Text("Band %d", source->getSourceId());
                    ImNodes::EndOutputAttribute();
                } else {
                    // Value generator - use unique output attribute ID
                    int outputAttrId = 2000 + source->getSourceId(); // 2000 + sourceId for unique output
                    ImNodes::BeginOutputAttribute(outputAttrId);
                    ImGui::Text("Output");
                    ImNodes::EndOutputAttribute();
                }
                
                ImNodes::EndNode();
                
                ImNodes::PopColorStyle();
                ImNodes::PopColorStyle();
                ImNodes::PopColorStyle();
                ImNodes::PopColorStyle();
            }
            
            // Input nodes (parameters) - right justified
            for (int j = 0; j < PARAM_COUNT; ++j) {
                ImNodes::BeginNode(getParameterNodeId(j));
                if (!nodes_initialized) ImNodes::SetNodeScreenSpacePos(getParameterNodeId(j), node_positions[valueManager.getSourceCount() + j]);
                ImNodes::BeginInputAttribute(getParameterAttributeId(j));
                ImGui::Text("%s", uniforms.metadata[j].name);                
                ImNodes::EndInputAttribute();
                ImNodes::EndNode();
            }
            // Draw all links with colored splines
            int color_alpha = 100;
            ImVec4 link_color = ImVec4(0.5, 0.5, 0.5, 1.0);
            for (const auto& link : valueManager.getAllLinks()) {
                int sourceId = link.first;
                ValueSource* source = valueManager.getSource(sourceId);
                if (source) {
                    color_alpha = 100 + (int)(source->getNormalizedValue() * 155);
                    link_color = (sourceId < BAND_COUNT) ? bar_colors[sourceId] : value_generator_color;
                }
                
                int startAttr, endAttr;
                startAttr = source->getOutputAttributeId();                
                endAttr = getParameterAttributeId(link.second);
                
                ImNodes::PushColorStyle(ImNodesCol_Link, toImU32(link_color, color_alpha));
                ImNodes::PushColorStyle(ImNodesCol_LinkHovered, toImU32(link_color, 255));
                ImNodes::PushColorStyle(ImNodesCol_LinkSelected, toImU32(link_color, 255));
                
                ImNodes::Link(generateLinkId(startAttr, endAttr), startAttr, endAttr);
                
                ImNodes::PopColorStyle();
                ImNodes::PopColorStyle();
                ImNodes::PopColorStyle();
            }
            nodes_initialized = true;
            
            // Add mini-map for navigation overview
            ImNodes::MiniMap(0.2f, ImNodesMiniMapLocation_TopRight);
            
            ImNodes::EndNodeEditor();

            // Handle deferred removals
            for (int sourceId : sourcesToRemove) {
                valueManager.removeSource(sourceId);
            }
            sourcesToRemove.clear();

            // Now handle new links
            int start_attr, end_attr;
            if (ImNodes::IsLinkCreated(&start_attr, &end_attr)) {
                if (isValidOutputAttribute(start_attr) && isValidInputAttribute(end_attr)) {
                    int sourceId = valueManager.getSourceIdFromAttribute(start_attr);
                    int paramIndex = getParameterIndexFromAttributeId(end_attr);
                    if (sourceId >= 0 && paramIndex >= 0) {
                        valueManager.addLink(sourceId, paramIndex);
                    }
                }
            }
            
        }
        ImGui::End();
        // Floating Save/Load panel in bottom right
        if (ImGui::Begin("Save/Load Layout")){
            ImGui::Text("Save/Load Layout Slots:");
            for (int slot = 1; slot <= 3; ++slot) {
                ImGui::PushID(slot);
                if (ImGui::Button((std::string("Save ") + std::to_string(slot)).c_str())) {
                    SaveNodeLayoutSlot(slot);
                }
                ImGui::SameLine();
                if (ImGui::Button((std::string("Load ") + std::to_string(slot)).c_str())) {
                    LoadNodeLayoutSlot(slot, uniforms);
                }
                ImGui::PopID();
            }
        }
        ImGui::End();

        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        
        if (graphicsPipe == nullptr) {
            glClearColor(0, 0, 0, 1);
            glClear(GL_COLOR_BUFFER_BIT);
        }
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
