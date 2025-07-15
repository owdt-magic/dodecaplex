#ifndef GUI_NODES_H
#define GUI_NODES_H

#include <cmath>
#include <string>
#include <functional>
#include <imgui.h>
#include <vector>
#include <memory>
#include <nlohmann/json.hpp>

// Forward declaration
struct UniformMeta;

// Unified interface for all value sources (audio bands, value generators, etc.)
class ValueSource {
protected:
    float value = 0.0f;
    std::string name;
    int sourceId;
    
public:
    ValueSource(const std::string& sourceName, int id) : name(sourceName), sourceId(id) {}
    virtual ~ValueSource() = default;
    
    // Core interface that all value sources must implement
    virtual void update(float deltaTime) = 0;
    virtual void renderUI() = 0;
    virtual float getValue() const { return value; }
    virtual const std::string& getName() const { return name; }
    virtual int getSourceId() const { return sourceId; }
    virtual void setSourceId(int id) { sourceId = id; } // Added for auto-generation
    
    // Optional interface for sources that can be configured
    virtual bool hasConfigurableParameters() const { return false; }
    virtual void setValue(float newValue) { value = newValue; }
    
    // Get processed value that would be applied to parameters (with centering/scaling)
    virtual float getProcessedValue(const UniformMeta& um) const { 
        if (sourceId < BAND_COUNT) {
            // Audio band: positive values, map directly to parameter range
            return um.min + std::min(value / 2.0f, 1.0f) * (um.max - um.min);
        } else {
            // Value generator: zero-mean values, center the mapping
            float range_center = (um.max + um.min) * 0.5f;
            float range_half = (um.max - um.min) * 0.5f;
            float value_scale = 0.5f; // Scale factor for the value
            return range_center + (value / 2.0f) * range_half * value_scale;
        }
    }
    
    // Get normalized value for visual feedback (0-1 range)
    virtual float getNormalizedValue() const {
        if (sourceId < BAND_COUNT) {
            return std::min(value, 1.0f);
        } else {
            // Value generator: normalize to 0-1 (simple approach)
            return std::max(0.0f, std::min((value + 1.0f) / 2.0f, 1.0f));
        }
    }

    // Get the attribute ID for the output of this value source
    virtual int getOutputAttributeId() const {
        return -1; // Default to -1 if not overridden
    }

    virtual nlohmann::json to_json() const { return {}; }
    virtual void from_json(const nlohmann::json& j) {}
};

// Audio band as a value source
class AudioBandSource : public ValueSource {
private:
    int bandIndex;
    float* audioData;
    float volume = 1.0f;
    
public:
    AudioBandSource(int bandIdx, float* audioPtr) 
        : ValueSource("Band " + std::to_string(bandIdx), bandIdx), 
          bandIndex(bandIdx), audioData(audioPtr) {}
    
    void update(float deltaTime) override {
        // Audio bands are updated externally via FFT
        value = *audioData * volume;
    }
    
    void renderUI() override {}
    
    bool hasConfigurableParameters() const override { return true; }
    int getBandIndex() const { return bandIndex; }
    float getVolume() const { return volume; }
    
    // Override to sync with shared uniforms
    void setVolume(float vol) { 
        volume = vol; 
        // Also update the shared uniforms for compatibility
        if (audioData) {
            // This is a bit of a hack - we need to find the band_volumes array
            // For now, we'll assume the audioData points to audio_bands[i]
            // and we can calculate the offset to band_volumes[i]
            // This is not ideal but maintains compatibility
        }
    }

    int getOutputAttributeId() const override {
        return AttributeHelpers::getAudioBandAttributeId(bandIndex);
    }

    nlohmann::json to_json() const override {
        return {
            {"type", "audio_band"},
            {"sourceId", sourceId},
            {"bandIndex", bandIndex},
            {"volume", volume}
        };
    }
    void from_json(const nlohmann::json& j) override {
        // audioData pointer must be set externally after construction
        if (j.contains("volume")) volume = j["volume"].get<float>();
    }
};

// Node factory for creating different types of value generators
class NodeFactory {
public:
    enum class NodeType {
        Sinusoid,
        Square,
        Triangle,
        Sawtooth,
        Noise,
        Constant
    };
    
    static std::string getNodeTypeName(NodeType type) {
        switch (type) {
            case NodeType::Sinusoid: return "Sinusoid";
            case NodeType::Square: return "Square";
            case NodeType::Triangle: return "Triangle";
            case NodeType::Sawtooth: return "Sawtooth";
            case NodeType::Noise: return "Noise";
            case NodeType::Constant: return "Constant";
            default: return "Unknown";
        }
    }
};

// Unified multi-mode value generator that can switch between different wave types
class MultiModeValueGenerator : public ValueSource {
private:
    float time = 0.0f;
    float frequency = 0.1f;
    float amplitude = 1.0f;
    float phase = 0.0f;
    float dutyCycle = 0.5f;
    float speed = 1.0f;
    float lastValue = 0.0f;
    float constantValue = 0.5f;
    
    NodeFactory::NodeType currentMode = NodeFactory::NodeType::Sinusoid;
    int selectedMode = 0; // Instance-specific mode selection for UI
    
public:
    MultiModeValueGenerator() : ValueSource("Value Generator", 0) {} // ID will be set by manager
    
    void update(float deltaTime) override {
        time += deltaTime;
        
        switch (currentMode) {
            case NodeFactory::NodeType::Sinusoid:
                value = amplitude * sin(2.0f * M_PI * frequency * time + phase);
                break;
                
            case NodeFactory::NodeType::Square:
                {
                    float cycle = fmod(frequency * time, 1.0f);
                    value = (cycle < dutyCycle) ? amplitude : -amplitude;
                }
                break;
                
            case NodeFactory::NodeType::Triangle:
                {
                    float cycle = fmod(frequency * time, 1.0f);
                    if (cycle < 0.5f) {
                        value = amplitude * (4.0f * cycle - 1.0f);
                    } else {
                        value = amplitude * (3.0f - 4.0f * cycle);
                    }
                }
                break;
                
            case NodeFactory::NodeType::Sawtooth:
                {
                    float cycle = fmod(frequency * time, 1.0f);
                    value = amplitude * (2.0f * cycle - 1.0f);
                }
                break;
                
            case NodeFactory::NodeType::Noise:
                {
                    // Simple random walk implementation
                    float change = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 2.0f * speed * deltaTime;
                    lastValue += change;
                    lastValue = std::max(-amplitude, std::min(amplitude, lastValue));
                    value = lastValue;
                }
                break;
                
            case NodeFactory::NodeType::Constant:
                value = constantValue;
                break;
        }
    }
    
    void renderUI() override {
        // Mode selection dropdown
        const char* modes[] = {"Sinusoid", "Square", "Triangle", "Sawtooth", "Noise", "Constant"};
        
        // Update selectedMode to match currentMode
        selectedMode = static_cast<int>(currentMode);
        
        ImGui::SetNextItemWidth(120.0f);
        if (ImGui::BeginCombo("Mode", modes[selectedMode])) {
            for (int i = 0; i < 6; ++i) {
                bool is_selected = (selectedMode == i);
                if (ImGui::Selectable(modes[i], is_selected)) {
                    if (selectedMode != i) {
                        selectedMode = i;
                        currentMode = static_cast<NodeFactory::NodeType>(i);
                    }
                }
                if (is_selected) ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
        
        // Common parameters
        ImGui::SetNextItemWidth(100.0f);
        ImGui::SliderFloat("Freq", &frequency, 0.01f, 0.1f, "%.2f");
        ImGui::SetNextItemWidth(100.0f);
        ImGui::SliderFloat("Amp", &amplitude, 0.1f, 5.0f, "%.2f");
        
        // Mode-specific parameters
        switch (currentMode) {
            case NodeFactory::NodeType::Sinusoid:
                ImGui::SetNextItemWidth(100.0f);
                ImGui::SliderFloat("Phase", &phase, -M_PI, M_PI, "%.2f");
                break;
                
            case NodeFactory::NodeType::Square:
                ImGui::SetNextItemWidth(100.0f);
                ImGui::SliderFloat("Duty", &dutyCycle, 0.0f, 1.0f, "%.2f");
                break;
                
            case NodeFactory::NodeType::Noise:
                ImGui::SetNextItemWidth(100.0f);
                ImGui::SliderFloat("Speed", &speed, 0.1f, 10.0f, "%.1f");
                break;
                
            case NodeFactory::NodeType::Constant:
                ImGui::SetNextItemWidth(100.0f);
                ImGui::SliderFloat("Value", &constantValue, -2.0f, 2.0f, "%.2f");
                break;
                
            default:
                break;
        }
        
        ImGui::Text("Value: %.3f", value);
    }
    
    // Getters and setters
    NodeFactory::NodeType getCurrentMode() const { return currentMode; }
    void setMode(NodeFactory::NodeType mode) { 
        currentMode = mode; 
        selectedMode = static_cast<int>(mode);
    }
    
    float getFrequency() const { return frequency; }
    void setFrequency(float freq) { frequency = freq; }
    
    float getAmplitude() const { return amplitude; }
    void setAmplitude(float amp) { amplitude = amp; }
    
    float getPhase() const { return phase; }
    void setPhase(float p) { phase = p; }
    
    float getDutyCycle() const { return dutyCycle; }
    void setDutyCycle(float duty) { dutyCycle = std::max(0.0f, std::min(1.0f, duty)); }
    
    float getSpeed() const { return speed; }
    void setSpeed(float s) { speed = s; }
    
    float getConstantValue() const { return constantValue; }
    void setConstantValue(float val) { constantValue = val; }

    int getOutputAttributeId() const override {
        return AttributeHelpers::getValueGeneratorAttributeId(sourceId);
    }

    nlohmann::json to_json() const override {
        return {
            {"type", "generator"},
            {"sourceId", sourceId},
            {"mode", (int)currentMode},
            {"frequency", frequency},
            {"amplitude", amplitude},
            {"phase", phase},
            {"dutyCycle", dutyCycle},
            {"speed", speed},
            {"constantValue", constantValue}
        };
    }
    void from_json(const nlohmann::json& j) override {
        if (j.contains("mode")) setMode((NodeFactory::NodeType)j["mode"].get<int>());
        if (j.contains("frequency")) frequency = j["frequency"].get<float>();
        if (j.contains("amplitude")) amplitude = j["amplitude"].get<float>();
        if (j.contains("phase")) phase = j["phase"].get<float>();
        if (j.contains("dutyCycle")) dutyCycle = j["dutyCycle"].get<float>();
        if (j.contains("speed")) speed = j["speed"].get<float>();
        if (j.contains("constantValue")) constantValue = j["constantValue"].get<float>();
    }
};

// Unified manager for all value sources
class ValueSourceManager {
private:
    std::vector<std::unique_ptr<ValueSource>> sources;
    std::vector<std::pair<int, int>> links; // sourceId -> parameterIndex
    int nextSourceId = 0; // Auto-generate unique IDs
    
public:
    ValueSourceManager() = default;
    
    // Add a value source with auto-generated ID
    int addSource(std::unique_ptr<ValueSource> source) {
        int newId = nextSourceId++;
        source->setSourceId(newId); // Set the auto-generated ID
        sources.push_back(std::move(source));
        return newId;
    }
    
    // Remove a source by ID
    bool removeSource(int sourceId) {
        // Remove all links to this source first
        links.erase(std::remove_if(links.begin(), links.end(),
            [sourceId](const std::pair<int, int>& link) {
                return link.first == sourceId;
            }), links.end());
        
        // Remove the source
        auto it = std::find_if(sources.begin(), sources.end(),
            [sourceId](const std::unique_ptr<ValueSource>& source) {
                return source->getSourceId() == sourceId;
            });
        
        if (it != sources.end()) {
            sources.erase(it);
            return true;
        }
        return false;
    }
    
    // Get a value source by ID
    ValueSource* getSource(int sourceId) {
        for (auto& source : sources) {
            if (source->getSourceId() == sourceId) {
                return source.get();
            }
        }
        return nullptr;
    }
    
    // Update all sources
    void updateAll(float deltaTime) {
        for (auto& source : sources) {
            source->update(deltaTime);
        }
    }
    
    // Apply values to parameters based on links
    void applyToParameters(UniformMeta* metadata, int paramCount) {
        for (const auto& link : links) {
            int sourceId = link.first;
            int paramIndex = link.second;
            
            if (paramIndex >= 0 && paramIndex < paramCount) {
                ValueSource* source = getSource(sourceId);
                if (source) {
                    UniformMeta& um = metadata[paramIndex];
                    *um.value = source->getProcessedValue(um);
                }
            }
        }
    }
    
    // Link management
    void addLink(int sourceId, int paramIndex) {
        // Check if this exact link already exists
        for (const auto& link : links) {
            if (link.first == sourceId && link.second == paramIndex) {
                return; // Link already exists, don't add duplicate
            }
        }
        
        // Remove any existing link to this parameter (only one source per parameter)
        links.erase(std::remove_if(links.begin(), links.end(),
            [paramIndex](const std::pair<int, int>& link) {
                return link.second == paramIndex;
            }), links.end());
        
        links.emplace_back(sourceId, paramIndex);
    }
    
    void removeLink(int sourceId, int paramIndex) {
        if (sourceId == -1) {
            // Remove all links to this parameter
            links.erase(std::remove_if(links.begin(), links.end(),
                [paramIndex](const std::pair<int, int>& link) {
                    return link.second == paramIndex;
                }), links.end());
        } else {
            // Remove specific link
            links.erase(std::remove_if(links.begin(), links.end(),
                [sourceId, paramIndex](const std::pair<int, int>& link) {
                    return link.first == sourceId && link.second == paramIndex;
                }), links.end());
        }
    }
    
    // Check if a parameter is being driven by any source
    bool isParameterDriven(int paramIndex) const {
        for (const auto& link : links) {
            if (link.second == paramIndex) {
                return true;
            }
        }
        return false;
    }
    
    // Get all sources
    const std::vector<std::unique_ptr<ValueSource>>& getAllSources() const {
        return sources;
    }
    
    // Get all links
    const std::vector<std::pair<int, int>>& getAllLinks() const {
        return links;
    }
    
    // Helper functions for node rendering
    int getSourceCount() const { return sources.size(); }
    
    // Get source by index (for rendering)
    ValueSource* getSourceByIndex(int index) {
        if (index >= 0 && index < sources.size()) {
            return sources[index].get();
        }
        return nullptr;
    }
    
    // Get source ID from attribute ID (for compatibility with existing system)
    int getSourceIdFromAttribute(int attributeId) {
        // Audio bands: attributeId = bandIndex * 10 + 1
        if (attributeId % 10 == 1 && attributeId / 10 < BAND_COUNT) {
            int sourceId = attributeId / 10;
            return sourceId; // Return band index as source ID
        }
        // Value generators: attributeId = 2000 + sourceId
        if (attributeId >= 2000) {
            int sourceId = attributeId - 2000;
            // Verify this source exists
            for (auto& source : sources) {
                if (source->getSourceId() == sourceId) {
                    return sourceId;
                }
            }
        }
        return -1;
    }

    nlohmann::json to_json() const {
        nlohmann::json j;
        j["sources"] = nlohmann::json::array();
        for (const auto& src : sources) {
            j["sources"].push_back(src->to_json());
        }
        j["links"] = nlohmann::json::array();
        for (const auto& link : links) {
            j["links"].push_back({{"sourceId", link.first}, {"paramIndex", link.second}});
        }
        return j;
    }

    void from_json(const nlohmann::json& j, float* audio_bands) {
        sources.clear();
        links.clear();
        nextSourceId = 0;
        if (j.contains("sources")) {
            for (const auto& srcj : j["sources"]) {
                std::string type = srcj["type"].get<std::string>();
                std::unique_ptr<ValueSource> src;
                if (type == "audio_band") {
                    int bandIdx = srcj["bandIndex"].get<int>();
                    src = std::make_unique<AudioBandSource>(bandIdx, &audio_bands[bandIdx]);
                    src->from_json(srcj);
                } else if (type == "generator") {
                    auto gen = std::make_unique<MultiModeValueGenerator>();
                    gen->from_json(srcj);
                    src = std::move(gen);
                }
                if (src) {
                    int sid = srcj["sourceId"].get<int>();
                    src->setSourceId(sid);
                    if (sid >= nextSourceId) nextSourceId = sid + 1;
                    sources.push_back(std::move(src));
                }
            }
        }
        if (j.contains("links")) {
            for (const auto& l : j["links"]) {
                links.emplace_back(l["sourceId"].get<int>(), l["paramIndex"].get<int>());
            }
        }
    }
};

#endif // GUI_NODES_H 