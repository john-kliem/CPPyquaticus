#ifndef RENDERER_H
#define RENDERER_H

#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <cstdlib>
//Load in packages for config handling
#include <nlohmann/json.hpp>
#include <fstream>  
#include <iostream>
#include "agent.h"
#include "team.h"
#include <raylib.h>
#include <chrono>
#include <iostream>
#include <thread>
// #include <pybind11/pybind11.h>
// #include <pybind11/stl.h>
#include "cppyquaticus.h"

#define FIELD_WIDTH   40.0f  // Logical field width
#define FIELD_HEIGHT  30.0f  // Logical field height

#define WINDOW_WIDTH  800
#define WINDOW_HEIGHT 400
#define FIELD_PADDING 10.0f
#pragma once
class CTFpyquaticus; 

class Renderer {
public:
    int screenWidth = 1600;//800;
    int screenHeight = 800;//400;
    float scaleFactor = 10.0f; // Scale positions for visualization
    float scaleX = screenWidth / (160 + 2* FIELD_PADDING);
    float scaleY = screenHeight / (80 + 2* FIELD_PADDING);
    float scaleAvg = (scaleX + scaleY) / 2.0f;
    void Init();
    void DrawField(const CTFpyquaticus& game);
    Vector2 worldToScreen(float wx, float wy, float field_width, float field_height);
    void drawFieldLines(float field_width, float field_height);
    void Close();
};

#endif  // PID_H
