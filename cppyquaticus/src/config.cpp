#include <iostream>
#include <vector>
#include <unordered_map>
#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>


struct Flag {
    double x, y; 

    Flag(double x_pos, double y_pos) : x(x_pos), y(y_pos) {}

    void displayPosition() const {
        std::cout << "Flag Position -> X: " << x << ", Y: " << y << std::endl;
    }
};


class Team {
public:
    int team_id;
    //Mapping agent_id to type (surveyor, heron, uav, uuv, etc)
    std::unordered_map<std::string, std::string> agent_types;
    int num_agents;
    
    Team(int team_num, std::unordered_map<std::string, std::string>& agent_types, int agents)
        : team_id(team_num), agent_types(agent_types), num_agents(agents)  {}
};

