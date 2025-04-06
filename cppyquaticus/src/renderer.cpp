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



    void Renderer::Init() {
        InitWindow(screenWidth, screenHeight, "Field Renderer");
        SetTargetFPS(60);
    }

    void Renderer::DrawField(const CTFpyquaticus& game) {
        BeginDrawing();
        ClearBackground(RAYWHITE);

        // Loop through each team
        std::vector<Agent> agents = game.agents;
        std::vector<Team> teams = game.gteams;
        drawFieldLines(160,80);
        for (const auto& team : teams) {
            /// Draw Lines based on team areas
            // Draw areas as borders (polygons)
            // for (const auto& area : team.areas) {
            //     int numPoints = area.points.size();  // Number of points in the area
            //     if (numPoints >= 3) {
            //         // Convert area points from {x, y} to Raylib Vector2
            //         std::vector<Vector2> polyPoints;
            //         for (const auto& point : area.points) {
            //             polyPoints.push_back({ static_cast<float>(point[0]) * scaleFactor, static_cast<float>(point[1]) * scaleFactor });
            //         }
                    
            //         // Draw the polygon border (outline) for the area
            //         DrawPolyLinesEx(polyPoints.data(), numPoints, 3.0f, DARKGRAY);
            //     }
            // }

            // Draw flags
            std::vector<Flag> flags = team.flags;
            for (size_t i = 0; i < flags.size(); i++) {

                Vector2 pos = worldToScreen(static_cast<float>(flags[i].x),static_cast<float>(flags[i].y), 160,80);
                DrawCircleV(pos, 10*scaleAvg, team.team_id ? (Color){ 139, 0, 0, 255 } : (Color){ 0, 0, 139, 255 });
                DrawCircleV(pos, 10*scaleAvg-2, (Color){ 255, 255, 255, 255 } );
                if (!flags[i].is_grabbed){
                    DrawCircleV(pos, 3*scaleAvg, team.team_id ? (Color){ 139, 0, 0, 255 } : (Color){ 0, 0, 139, 255 });
                }
            }

            std::unordered_map<int, bool> agent_ids = team.agents;
            for (size_t i = 0; i < agents.size(); i++) {
            Vector2 pos = worldToScreen(static_cast<float>(agents[i].x),static_cast<float>(agents[i].y), 160,80);//{ static_cast<float>(agents[i].x), static_cast<float>(agents[i].y)};
            if(agents[i].is_tagged){
                DrawCircleV(pos, (agents[i].agent_radius)*scaleAvg+1.5, (Color){ 0, 255, 0, 255 });
            }
            DrawCircleV(pos, agents[i].agent_radius*scaleAvg, agents[i].team_id == 0 ? BLUE : RED);

            float headingRad = (fmodf(agents[i].heading+360.0f, 360.0f)-90.0f) * DEG2RAD;//agents[i].heading * DEG2RAD;
            Vector2 headingVec = { pos.x + cos(headingRad) * 2*scaleAvg, pos.y + sin(headingRad) * 2*scaleAvg };
            DrawLineEx(pos, headingVec, 0.25f *scaleAvg, BLACK);
            }
            
        }
        EndDrawing();
    }

    Vector2 Renderer::worldToScreen(float wx, float wy, float field_width, float field_height) {
        
        
        float scaleX = screenWidth / (field_width + 2* FIELD_PADDING);
        float scaleY = screenHeight / (field_height + 2* FIELD_PADDING);
        Vector2 screenPos;
        screenPos.x = (wx + FIELD_PADDING) * scaleX;
        screenPos.y = screenHeight - (wy + FIELD_PADDING) * scaleY;
        return screenPos;
    }
    void Renderer::drawFieldLines(float field_width, float field_height) {
    
        Vector2 start = worldToScreen(0, 0, field_width,field_height);
        Vector2 end = worldToScreen(0, field_height, field_width,field_height);
        DrawLineV(start, end, DARKGRAY);
        start = worldToScreen(field_width, 0, field_width,field_height);
        end = worldToScreen(field_width, field_height, field_width,field_height);
        DrawLineV(start, end, DARKGRAY);

        start = worldToScreen(0, 0, field_width,field_height);
        end = worldToScreen(field_width, 0, field_width,field_height);
        DrawLineV(start, end, DARKGRAY);
        start = worldToScreen(0, field_height, field_width,field_height);
        end = worldToScreen(field_width, field_height, field_width,field_height);
        DrawLineV(start, end, DARKGRAY);

        start = worldToScreen(field_width/2, 0, field_width,field_height);
        end = worldToScreen(field_width/2, field_height, field_width,field_height);
        DrawLineV(start, end, DARKGRAY);
    }
    void Renderer::Close() {
        CloseWindow();
    }