#ifndef GUI_NODES_H
#define GUI_NODES_H

#include <cmath>
#include <string>
#include <functional>
#include <imgui.h>
#include <vector>
#include <memory>

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
};

class ValueGeneratorNode : public ValueSource {
protected:
    float time = 0.0f;
    
public:
    ValueGeneratorNode(const std::string& nodeName, int nodeId) 
        : ValueSource(nodeName, nodeId) {} // Use unique ID for each value generator
    virtual ~ValueGeneratorNode() = default;
    
    // Pure virtual function that derived classes must implement
    virtual void update(float deltaTime) override = 0;
    
    // Virtual function for rendering UI controls - derived classes should override
    virtual void renderUI() override = 0;
    
    // Common getters/setters
    float getTime() const { return time; }
    void setTime(float t) { time = t; }
    bool hasConfigurableParameters() const override { return true; }
};

// Sinusoid wave generator node
class SinusoidNode : public ValueGeneratorNode {
private:
    float frequency = 1.0f;
    float amplitude = 1.0f;
    float phase = 0.0f;
    
public:
    SinusoidNode() : ValueGeneratorNode("Sinusoid", 200) {}
    
    void update(float deltaTime) override {
        time += deltaTime;
        value = amplitude * sin(2.0f * M_PI * frequency * time + phase);
    }
    
    void renderUI() override {
        ImGui::SetNextItemWidth(100.0f);
        ImGui::SliderFloat("Freq", &frequency, 0.1f, 10.0f, "%.1f");
        ImGui::SetNextItemWidth(100.0f);
        ImGui::SliderFloat("Amp", &amplitude, 0.1f, 5.0f, "%.1f");
        ImGui::SetNextItemWidth(100.0f);
        ImGui::SliderFloat("Phase", &phase, -M_PI, M_PI, "%.2f");
    }
    
    // Getters and setters for sinusoid-specific parameters
    float getFrequency() const { return frequency; }
    void setFrequency(float freq) { frequency = freq; }
    
    float getAmplitude() const { return amplitude; }
    void setAmplitude(float amp) { amplitude = amp; }
    
    float getPhase() const { return phase; }
    void setPhase(float p) { phase = p; }
};

// Square wave generator node
class SquareNode : public ValueGeneratorNode {
private:
    float frequency = 1.0f;
    float amplitude = 1.0f;
    float dutyCycle = 0.5f; // 0.0 to 1.0
    
public:
    SquareNode() : ValueGeneratorNode("Square", 201) {}
    
    void update(float deltaTime) override {
        time += deltaTime;
        float cycle = fmod(frequency * time, 1.0f);
        value = (cycle < dutyCycle) ? amplitude : -amplitude;
    }
    
    void renderUI() override {
        ImGui::SetNextItemWidth(100.0f);
        ImGui::SliderFloat("Freq", &frequency, 0.1f, 10.0f, "%.1f");
        ImGui::SetNextItemWidth(100.0f);
        ImGui::SliderFloat("Amp", &amplitude, 0.1f, 5.0f, "%.1f");
        ImGui::SetNextItemWidth(100.0f);
        ImGui::SliderFloat("Duty", &dutyCycle, 0.0f, 1.0f, "%.2f");
    }
    
    float getFrequency() const { return frequency; }
    void setFrequency(float freq) { frequency = freq; }
    
    float getAmplitude() const { return amplitude; }
    void setAmplitude(float amp) { amplitude = amp; }
    
    float getDutyCycle() const { return dutyCycle; }
    void setDutyCycle(float duty) { dutyCycle = std::max(0.0f, std::min(1.0f, duty)); }
};

// Triangle wave generator node
class TriangleNode : public ValueGeneratorNode {
private:
    float frequency = 1.0f;
    float amplitude = 1.0f;
    
public:
    TriangleNode() : ValueGeneratorNode("Triangle", 202) {}
    
    void update(float deltaTime) override {
        time += deltaTime;
        float cycle = fmod(frequency * time, 1.0f);
        if (cycle < 0.5f) {
            value = amplitude * (4.0f * cycle - 1.0f);
        } else {
            value = amplitude * (3.0f - 4.0f * cycle);
        }
    }
    
    void renderUI() override {
        ImGui::SetNextItemWidth(100.0f);
        ImGui::SliderFloat("Freq", &frequency, 0.1f, 10.0f, "%.1f");
        ImGui::SetNextItemWidth(100.0f);
        ImGui::SliderFloat("Amp", &amplitude, 0.1f, 5.0f, "%.1f");
    }
    
    float getFrequency() const { return frequency; }
    void setFrequency(float freq) { frequency = freq; }
    
    float getAmplitude() const { return amplitude; }
    void setAmplitude(float amp) { amplitude = amp; }
};

// Sawtooth wave generator node
class SawtoothNode : public ValueGeneratorNode {
private:
    float frequency = 1.0f;
    float amplitude = 1.0f;
    
public:
    SawtoothNode() : ValueGeneratorNode("Sawtooth", 203) {}
    
    void update(float deltaTime) override {
        time += deltaTime;
        float cycle = fmod(frequency * time, 1.0f);
        value = amplitude * (2.0f * cycle - 1.0f);
    }
    
    void renderUI() override {
        ImGui::SetNextItemWidth(100.0f);
        ImGui::SliderFloat("Freq", &frequency, 0.1f, 10.0f, "%.1f");
        ImGui::SetNextItemWidth(100.0f);
        ImGui::SliderFloat("Amp", &amplitude, 0.1f, 5.0f, "%.1f");
    }
    
    float getFrequency() const { return frequency; }
    void setFrequency(float freq) { frequency = freq; }
    
    float getAmplitude() const { return amplitude; }
    void setAmplitude(float amp) { amplitude = amp; }
};

// Noise generator node (simple random walk)
class NoiseNode : public ValueGeneratorNode {
private:
    float amplitude = 1.0f;
    float speed = 1.0f;
    float lastValue = 0.0f;
    
public:
    NoiseNode() : ValueGeneratorNode("Noise", 204) {}
    
    void update(float deltaTime) override {
        time += deltaTime;
        // Simple random walk implementation
        float change = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 2.0f * speed * deltaTime;
        lastValue += change;
        lastValue = std::max(-amplitude, std::min(amplitude, lastValue));
        value = lastValue;
    }
    
    void renderUI() override {
        ImGui::SetNextItemWidth(100.0f);
        ImGui::SliderFloat("Amp", &amplitude, 0.1f, 5.0f, "%.1f");
        ImGui::SetNextItemWidth(100.0f);
        ImGui::SliderFloat("Speed", &speed, 0.1f, 10.0f, "%.1f");
    }
    
    float getAmplitude() const { return amplitude; }
    void setAmplitude(float amp) { amplitude = amp; }
    
    float getSpeed() const { return speed; }
    void setSpeed(float s) { speed = s; }
};

// Constant value generator node
class ConstantNode : public ValueGeneratorNode {
private:
    float constantValue = 0.5f;
    
public:
    ConstantNode() : ValueGeneratorNode("Constant", 205) {}
    
    void update(float deltaTime) override {
        // Constant value doesn't change over time
        value = constantValue;
    }
    
    void renderUI() override {
        ImGui::SetNextItemWidth(100.0f);
        ImGui::SliderFloat("Value", &constantValue, -2.0f, 2.0f, "%.2f");
    }
    
    float getConstantValue() const { return constantValue; }
    void setConstantValue(float val) { constantValue = val; }
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
    
    static ValueGeneratorNode* createNode(NodeType type) {
        switch (type) {
            case NodeType::Sinusoid:
                return new SinusoidNode();
            case NodeType::Square:
                return new SquareNode();
            case NodeType::Triangle:
                return new TriangleNode();
            case NodeType::Sawtooth:
                return new SawtoothNode();
            case NodeType::Noise:
                return new NoiseNode();
            case NodeType::Constant:
                return new ConstantNode();
            default:
                return new SinusoidNode(); // Default fallback
        }
    }
    
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
    float frequency = 1.0f;
    float amplitude = 1.0f;
    float phase = 0.0f;
    float dutyCycle = 0.5f;
    float speed = 1.0f;
    float lastValue = 0.0f;
    float constantValue = 0.5f;
    
    NodeFactory::NodeType currentMode = NodeFactory::NodeType::Sinusoid;
    
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
        static int selectedMode = 0;
        const char* modes[] = {"Sinusoid", "Square", "Triangle", "Sawtooth", "Noise", "Constant"};
        
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
        ImGui::SliderFloat("Freq", &frequency, 0.1f, 10.0f, "%.1f");
        ImGui::SetNextItemWidth(100.0f);
        ImGui::SliderFloat("Amp", &amplitude, 0.1f, 5.0f, "%.1f");
        
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
    void setMode(NodeFactory::NodeType mode) { currentMode = mode; }
    
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
                    float sourceValue = source->getValue();
                    
                    if (sourceId < BAND_COUNT) {
                        // Audio band: positive values, map directly to parameter range
                        // Audio bands typically range from 0 to ~2.0, map to parameter range
                        float normalizedValue = std::min(sourceValue / 2.0f, 1.0f); // Normalize to 0-1
                        *um.value = um.min + normalizedValue * (um.max - um.min);
                    } else {
                        // Value generator: zero-mean values, center the mapping
                        float range_center = (um.max + um.min) * 0.5f;
                        float range_half = (um.max - um.min) * 0.5f;
                        float value_scale = 0.5f; // Scale factor for the value
                        *um.value = range_center + (sourceValue / 2.0f) * range_half * value_scale;
                    }
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
};

#endif // GUI_NODES_H 