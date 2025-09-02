# Dodecaplex

For images and videos documenting this project:
https://owdt-magic.org/dodecaplex.html

A sophisticated OpenGL-based graphics engine and shader development platform featuring audio-reactive visualizations, real-time shader editing, and multiple rendering modes. This project has evolved from a simple shader sandbox into a comprehensive graphics programming toolkit. It also contains a simple 4D game engine.

## 🎮 Features

### Multiple Rendering Modes
- **Game Mode** (`./game`) - Interactive 4D projected world with spell casting mechanics and grimoire system
- **Spin Mode** (`./spin`) - Audio-reactive geometric visualizations with real-time music analysis
- **Fragment Mode** (`./fragment`) - Fullscreen fragment shader playground for experimentation
- **Select Mode** (`./select`) - Visual node-based shader editor with ImGui interface (START HERE)

### Audio Integration
- Real-time audio input processing using Miniaudio
- FFT-based frequency band analysis (4-band system)
- Audio-reactive shader uniforms for music visualization

### Graphics Pipeline
- Modern OpenGL 3.3+ with GLAD loader
- GLFW window management with multi-monitor support
- Comprehensive shader management system
- Texture library with PBR materials
- 3D model loading via Assimp

### Development Tools
- Visual node editor for shader composition
- Real-time shader hot-reloading
- Python scripting for automation, generation, and analysis
- Multi-display management and calibration tools

## 🏗️ Architecture

### Core Components
- **GraphicsPipe** - Unified rendering pipeline supporting multiple modes
- **ShaderInterface** - Abstract base for different rendering patterns
- **AudioNest** - Real-time audio processing and FFT analysis
- **ValueSourceManager** - Node-based value generation system
- **TextureLibrary** - Asset management for textures and materials

### Shader System
- Modular shader architecture with shared uniforms
- Geometry, vertex, and fragment shader support
- Real-time uniform updates from audio and user input
- Hot-reloadable shader compilation

## 🚀 Building and Running

### Prerequisites
- CMake 3.19.4+
- OpenGL 3.3+
- GLFW3
- GLM (OpenGL Mathematics)
- Assimp (3D model loading)
- Miniaudio (audio processing)
- ImGui (user interface)

### Compilation
```bash
# Create build directory
mkdir build && cd build

# Configure with CMake
cmake ..

# Build all executables
make

# Run different modes
./game      # Interactive 3D world
./spin      # Audio-reactive visualizations  
./fragment  # Fragment shader playground
./select    # Visual shader editor
```

### Dependencies Installation (macOS)
```bash
# Using Homebrew
brew install cmake glfw glm assimp

# Or using MacPorts
sudo port install cmake glfw glm assimp
```

## 📁 Project Structure

```
dodecaplex/
├── main/           # Executable entry points
├── src/            # Core engine source files
├── include/        # Header files and external libraries
├── shaders/        # GLSL shader files
├── textures/       # Texture assets
├── models/         # 3D model files
├── python/         # Python tools and automation scripts
├── fonts/          # UI fonts
└── build/          # Build artifacts
```

## 🎨 Shader Development

### Node Editor Features
- Visual shader composition
- Real-time parameter adjustment
- Audio input integration
- Multi-display output management

## 🎵 Audio Features

### Real-time Processing
- Live audio input capture
- 4-band frequency analysis
- Audio-reactive shader uniforms
- Support for separated audio stems

## 🔧 Development Workflow

### Shader Development
1. Use `./select` for visual shader editing
2. Hot-reload shaders during development
3. Test with `./fragment` for fullscreen preview
4. Integrate audio with `./spin` for music visualization

### 4D World Development
1. Use `./game` for interactive 4D testing
2. Implement new spells and effects
3. Test collision and physics systems
4. Develop new geometry patterns

## 🎥 Preview

Check out the project in action:
[Preview Video](https://www.youtube.com/watch?v=BYUks-dQ8sE)

## 🌐 Links

- **Website**: [owdt-magic.org](https://owdt-magic.org)
- **YouTube**: [Guy^1,000,000](https://www.youtube.com/@Guytothemillionth)

## 📝 License

This project is developed for educational and experimental purposes in graphics programming and audio-visual synthesis.

---

*Built with OpenGL, GLFW, ImGui, and a passion for real-time graphics programming.*

