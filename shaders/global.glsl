#version 410 core
layout(std140) uniform CameraMatrices {
    mat4 CAMERA;
    mat4 WORLD;
};