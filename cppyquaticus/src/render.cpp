#include "raylib.h"
#include <vector>
#include "Agent.h"
#include "Flag.h"
#include "Team.h"

class Renderer {
public:
    int screenWidth = 800;
    int screenHeight = 600;
    float scaleFactor = 10.0f; // Scale positions for visualization

    void Init() {
        InitWindow(screenWidth, screenHeight, "Field Renderer");
        SetTargetFPS(60);
    }

    void DrawField(const std::vector<CTFpyquaticus>& game) {
        BeginDrawing();
        ClearBackground(RAYWHITE);

        // Loop through each team
        std::vector<Team> teams = game.gteams;
        for (const auto& team : teams) {
            // Draw areas as borders (polygons)
            for (const auto& area : team.areas) {
                int numPoints = area.points.size();  // Number of points in the area
                if (numPoints >= 3) {
                    // Convert area points from {x, y} to Raylib Vector2
                    std::vector<Vector2> polyPoints;
                    for (const auto& point : area.points) {
                        polyPoints.push_back({ point[0] * scaleFactor, point[1] * scaleFactor });
                    }
                    
                    // Draw the polygon border (outline) for the area
                    DrawPolyLines(polyPoints.data(), numPoints, 3, DARKGRAY);  // Adjust line width and color as needed
                }
            }
            // Draw flags
            std::vector<Flag> flags = team.flags;
            for (size_t i = 0; i < flags.size(); i++) {
                Vector2 pos = { flags.x * scaleFactor, flags.y * scaleFactor };
                DrawCircleV(pos, 5, flags.is_grabbed ? DARKGRAY : GREEN);
            }
        }

        std::vector<Agent> agents = game.agents;
        for (size_t i = 0; i < agents.size(); i++) {
            Vector2 pos = { agents[i].x * scaleFactor, agents[i].y * scaleFactor };
            DrawCircleV(pos, 5, team.team_id == 0 ? BLUE : RED);
            
            float headingRad = agents[i].heading * DEG2RAD;
            Vector2 headingVec = { agents[i].x + cos(headingRad) * 10, agents[i].y + sin(headingRad) * 10 };
            DrawLineV(pos, headingVec, BLACK);
        }
        EndDrawing();
    }

    void Close() {
        CloseWindow();
    }
};
