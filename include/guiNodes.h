#ifndef GUI_NODES_H
#define GUI_NODES_H

#include <cmath>
#include <string>
#include <functional>
#include <imgui.h>

class ValueGeneratorNode {
protected:
    float value = 0.0f;
    float time = 0.0f;
    std::string name;
    
public:
    ValueGeneratorNode(const std::string& nodeName) : name(nodeName) {}
    virtual ~ValueGeneratorNode() = default;
    
    // Pure virtual function that derived classes must implement
    virtual void update(float deltaTime) = 0;
    
    // Virtual function for rendering UI controls - derived classes should override
    virtual void renderUI() = 0;
    
    // Common getters/setters
    virtual float getValue() const { return value; }
    virtual const std::string& getName() const { return name; }
    float getTime() const { return time; }
    void setTime(float t) { time = t; }
};

// Sinusoid wave generator node
class SinusoidNode : public ValueGeneratorNode {
private:
    float frequency = 1.0f;
    float amplitude = 1.0f;
    float phase = 0.0f;
    
public:
    SinusoidNode() : ValueGeneratorNode("Sinusoid") {}
    
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
    SquareNode() : ValueGeneratorNode("Square") {}
    
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
    TriangleNode() : ValueGeneratorNode("Triangle") {}
    
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
    SawtoothNode() : ValueGeneratorNode("Sawtooth") {}
    
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
    NoiseNode() : ValueGeneratorNode("Noise") {}
    
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
    ConstantNode() : ValueGeneratorNode("Constant") {}
    
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

#endif // GUI_NODES_H 