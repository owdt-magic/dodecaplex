#include <iostream>
#include <sstream>
#include <vector>
#include "raylib.h"
#include "audio.h"

void launch_instances_parallel(const std::string& command, int instanceCount) {
    for (int i = 0; i < instanceCount; ++i) {
        std::string fullCommand = command + " &";
        system(fullCommand.c_str());
    }
}

int main() {
    const int screenWidth = 800;
    const int screenHeight = 500;
    InitWindow(screenWidth, screenHeight, "Select Ritual Mode");
    SetTargetFPS(60);

    Rectangle vortexBtn = { 100, 100, 150, 50 };
    Rectangle gameBtn   = { 100, 170, 150, 50 };
    Rectangle spinBtn   = { 100, 240, 150, 50 };
    Rectangle inputBox  = { 100, 310, 150, 40 };

    int instanceCount = 1;
    char instanceInput[4];
    snprintf(instanceInput, sizeof(instanceInput), "%d", instanceCount);

    ma_context context;
    ma_context_config contextConfig = ma_context_config_init();
    ma_context_init(NULL, 0, &contextConfig, &context);
    auto deviceNames = GetInputDeviceNames(&context);
    int selectedDevice = 0;

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);

        DrawText("Select Mode", 100, 40, 30, RAYWHITE);

        DrawRectangleRec(vortexBtn, DARKGRAY);
        DrawRectangleRec(gameBtn, DARKGRAY);
        DrawRectangleRec(spinBtn, DARKGRAY);

        DrawText("Vortex", vortexBtn.x + 20, vortexBtn.y + 15, 20, RAYWHITE);
        DrawText("Game", gameBtn.x + 20, gameBtn.y + 15, 20, RAYWHITE);
        DrawText("Spin", spinBtn.x + 20, spinBtn.y + 15, 20, RAYWHITE);

        DrawText("Instances:", inputBox.x, inputBox.y - 25, 20, RAYWHITE);
        DrawRectangleRec(inputBox, DARKGRAY);
        snprintf(instanceInput, sizeof(instanceInput), "%d", instanceCount);
        DrawText(instanceInput, inputBox.x + 5, inputBox.y + 10, 20, BLACK);

        DrawText("Audio Input:", 400, 100 - 25, 20, RAYWHITE);
        for (size_t i = 0; i < deviceNames.size(); ++i) {
            Color color = (i == selectedDevice) ? LIGHTGRAY : DARKGRAY;
            DrawRectangle(400, 100 + i * 40, 300, 35, color);
            DrawText(deviceNames[i].c_str(), 410, 110 + i * 40, 20, RAYWHITE);
        }

        Vector2 mouse = GetMousePosition();

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            if (CheckCollisionPointRec(mouse, vortexBtn)) {
                CloseWindow();
                std::stringstream ss;
                ss << "./vortex --instances " << instanceCount << " --input " << selectedDevice;
                launch_instances_parallel(ss.str(), instanceCount);
                break;
            }
            if (CheckCollisionPointRec(mouse, gameBtn)) {
                CloseWindow();
                std::stringstream ss;
                ss << "./game --instances " << instanceCount << " --input " << selectedDevice;
                launch_instances_parallel(ss.str(), instanceCount);
                break;
            }
            if (CheckCollisionPointRec(mouse, spinBtn)) {
                CloseWindow();
                std::stringstream ss;
                ss << "./spin --instances " << instanceCount << " --input " << selectedDevice;
                launch_instances_parallel(ss.str(), instanceCount);
                break;
            }

            for (size_t i = 0; i < deviceNames.size(); ++i) {
                Rectangle deviceRect = { 400, (float) 100 + i * 40, 300, 35 };
                if (CheckCollisionPointRec(mouse, deviceRect)) {
                    selectedDevice = i;
                }
            }
        }

        if (IsKeyPressed(KEY_RIGHT)) {
            instanceCount = std::min(instanceCount + 1, 999);
        }
        if (IsKeyPressed(KEY_LEFT)) {
            instanceCount = std::max(instanceCount - 1, 1);
        }

        EndDrawing();
    }

    ma_context_uninit(&context);
    CloseWindow();
    return 0;
}
