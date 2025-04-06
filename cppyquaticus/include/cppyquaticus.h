#ifndef CPPYQUATICUS_H
#define CPPYQUATICUS_H

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
#include "renderer.h"



inline float to_360(float angle) {
    return (angle < 0) ? angle + 360.0f : angle;
}
//Use Pybind to import packages
 /**
  * @brief This is an implementation of pyquaticus's pettingzoo environment written in c++ to c
  * 
  * 
  */
class CTFpyquaticus {
public:
    nlohmann::json config;
    //std::unordered_map<int, int> agent_ids;
    std::vector<Agent> agents;
    std::unordered_map<std::string, int> team_ids;
    std::vector<Team> gteams;
    //TODO: Implement Obstacles
    std::vector<std::string> obstacles;
    int cur_step;

    /**
      * @brief Initializes a capture the flag game along with agents.
      * @param num_teams number of teams in the game currently only supports two teams
      * @param num_agents number of agents per team currently in the game.
      * @param num_possible number of agents possible on each team.
      * @param config game configuration.
      * @return Returns a maritime capture the flag env object.
    */

    CTFpyquaticus(const std::string& user_config);
    CTFpyquaticus(const CTFpyquaticus& other);
    CTFpyquaticus& operator=(const CTFpyquaticus& other);
    void reset();
    void step_agents(std::vector<std::vector<int>> actions);
    void step_agent(int agent_id, int action);
    void step_agent(int agent_id, std::vector<double> action);
    void check_game_events();
    void load_from_config();
    void drive_home(int agent_id);
    void drive_towards(int agent_id, point p);

};


#endif  
