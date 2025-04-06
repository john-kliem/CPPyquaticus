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
#include "cppyquaticus.h"




//Use Pybind to import packages
 /**
  * @brief This is an implementation of pyquaticus's pettingzoo environment written in c++ to c
  * 
  * 
  */

    CTFpyquaticus::CTFpyquaticus(const std::string& user_config) {
        cur_step = 0;

        std::ifstream input_file(user_config);
        if (input_file.is_open()) {
            input_file >> config;  // Parse the JSON content
            load_from_config();
        }
    }

    CTFpyquaticus::CTFpyquaticus(const CTFpyquaticus& other)
        : config(other.config),
          //agent_ids(other.agent_ids),
          team_ids(other.team_ids),
          obstacles(other.obstacles), cur_step(other.cur_step){
        agents.clear();
        for (const auto& agent : other.agents) {
            agents.push_back(agent);  // Assuming Agent has a proper copy constructor
        }
        gteams.clear();
        for (const auto& team : other.gteams) {
            gteams.push_back(team);  // Assuming Team has a proper copy constructor
        }
    }

    CTFpyquaticus& CTFpyquaticus::operator=(const CTFpyquaticus& other) {
        if (this == &other) return *this; // Prevent self-assignment
        config = other.config;
        //agent_ids = other.agent_ids;
        team_ids = other.team_ids;
        obstacles = other.obstacles;
        cur_step = other.cur_step;
        agents.clear();
        for (const auto& agent : other.agents) {
            agents.push_back(agent);  
        }
        gteams.clear();
        for (const auto& team : other.gteams) {
            gteams.push_back(team); 
        }
        return *this;
    }
    void CTFpyquaticus::reset() {
        //agent_ids.clear();
        agents.clear();
        team_ids.clear();
        gteams.clear();
        obstacles.clear();
        load_from_config();
        return;
    }

    void CTFpyquaticus::step_agents(std::vector<std::vector<int>> actions){
        for(size_t i = 0; i < actions.size(); i++){
            if(agents[actions[i][0]].is_tagged){
                drive_home(actions[i][0]);
            }
            else{
                step_agent(actions[i][0], actions[i][1]);
            }
        }
        check_game_events();
    }
    void CTFpyquaticus::step_agent(int agent_id, int action){
        agents[agent_id].move_agent(action);
    }
    void CTFpyquaticus::step_agent(int agent_id, std::vector<double> action){
        agents[agent_id].move_agent(action[0], action[1]);
    }

    // TODO: Add Collision with obstacles and other players
    // void check_collisions(){

    // }
    void CTFpyquaticus::check_game_events(){
        for(size_t a = 0; a < agents.size(); a++){
            for(size_t o = 0; o < agents.size(); o++){
                //Update if player is onside only on first loop through agents
                if(a == 0){
                    agents[o].on_side = -1;
                    for (auto& team : gteams) {
                        team.check_onside(&agents[o]);
                    }
                    // std::cout << "Agent: "<< o << " onside: "<< agents[o].on_side<<std::endl;
                    if (agents[o].on_side == -1)
                    {
                        agents[o].is_tagged = true;
                    }
                }
                if (a == o){
                    continue;
                }
                double dist = calculateDistance(agents[a].x, agents[a].y, agents[o].x, agents[o].y);
                //Check Agent a has tagged agent o
                if(agents[a].tag_cooldown == 0.0 && agents[a].on_side==agents[a].team_id && agents[o].on_side==agents[a].team_id && !agents[o].is_tagged && dist < agents[a].tag_range){
                    agents[a].tag_cooldown = agents[a].max_cooldown;
                    agents[o].is_tagged = true;
                    if (agents[o].has_flag > -1){
                        gteams[agents[o].has_flag].reset_grab(o);
                    }
                    agents[o].has_flag = -1;
                }
            }
            //Check to see if agent captured a flag or should be untagged
            // std::cout << "Check Tag and captures"<<std::endl;
            if((agents[a].is_tagged || agents[a].has_flag > -1) && agents[a].on_side==agents[a].team_id){
                for(size_t hb = 0; hb < gteams[agents[a].team_id].home_bases.size(); hb++){
                    double dist = calculateDistance(agents[a].x, agents[a].y, gteams[agents[a].team_id].home_bases[hb][0], gteams[agents[a].team_id].home_bases[hb][1]);
                    if(agents[a].is_tagged && dist < agents[a].tag_range){
                        agents[a].is_tagged = false;
                    }
                    //Check to see if agent should capture flag
                    //TODO change has_flag to be value of teams flag grabbed
                    if(agents[a].on_side==agents[a].team_id && (agents[a].has_flag > -1) && dist < agents[a].capture_range)
                    {
                        gteams[agents[a].has_flag].reset_grab(a);
                        agents[a].has_flag = -1;

                    }
                }
            }
            //Check to see if agent should grab Flag
            // std::cout << "Chk Grab "<< agents[a].has_flag << " 2: "<< !(agents[a].on_side==agents[a].team_id)<<std::endl;
            if((agents[a].has_flag == -1) && (agents[a].on_side!=agents[a].team_id) && !agents[a].is_tagged){
                for (auto& team : gteams) {
                    bool res = team.check_grab(a, &agents[a]);
                    if(res){
                        //Update Grab/Capture State
                        break;
                    }
                }
            }
            
        }

    }

    void CTFpyquaticus::load_from_config(){
        if (config.contains("agents")) {
            // Initialize Agents In Game
            const nlohmann::json& agents_json = config["agents"];  // Reference to avoid repeated lookups
            const nlohmann::json& agent_starts = config["agent_starts"];
            for (size_t i = 0; i < agents_json.size(); i++) {
                if (agents_json[i][1] == "surveyor"){
                    // agent_ids[agents_json[i][0]] = i;
                    agents.push_back(Surveyor((float)agent_starts[i][0], (float)agent_starts[i][1], (float)agent_starts[i][2], 2.0, i));
                }
            }
        }
        // Initialize Teams
        if (config.contains("teams")){
            const nlohmann::json& teams = config["teams"];  // Reference to avoid repeated lookups
            if (teams.empty()){
                std::cout << "Scenario config should include atleast one team" << std::endl;
            }
            for(auto it = teams.begin(); it != teams.end(); ++it){
                //Initialize Team Variables
                std::string team_name = it.key();

                int team_id;
                std::unordered_map<int, bool> agents_on_team;
                std::vector<Area> areas;
                std::vector<Flag> flags;
                std::vector<std::vector<double>> home_base;
                const nlohmann::json& team_info = it.value();
                //Assign Team ID (Int)
                if (team_info.contains("team_id")){
                    team_id = team_info["team_id"];
                    team_ids[it.key()] = team_id;

                }
                else{
                    std::cout << "Team Config should include team_id" << std::endl;
                    return;
                }
                //Assign Agents on Team if it exists otherwise will be an empty array
                if (team_info.contains("agents_on_team")){
                    for(size_t i = 0; i < team_info["agents_on_team"].size(); i++)
                    {
                        agents_on_team[team_info["agents_on_team"][i]] = true;
                    }
                }

                if (team_info.contains("areas")){
                    for(size_t i = 0; i < team_info["areas"].size(); i++)
                    {
                        areas.push_back(Area(team_info["areas"][i]));
                    }
                }
                else{
                    std::cout << "Team should control atleast one area" << std::endl;
                    return;
                }
                
                if (team_info.contains("flags")){
                    for(size_t i = 0; i < team_info["flags"].size(); i++)
                    {
                        flags.push_back(Flag(team_info["flags"][i][0],team_info["flags"][i][1]));
                    }
                }
                else{
                    std::cout << "Team should have atleast one flag" << std::endl;
                    return;
                }
                if (team_info.contains("home_base")){

                    home_base = team_info["home_base"];
                }
                else{
                    std::cout << "Team should have atleast one home base" << std::endl;
                    return;
                }
                gteams.push_back(Team(team_id, agents_on_team, flags, areas, home_base));
            }
        }
    }

    void CTFpyquaticus::drive_home(int agent_id){
        Team team  = gteams[agents[agent_id].team_id];
        point hb;
        double min_dist = 0.0;
        for(size_t i = 0; i < team.home_bases.size(); i++){
            double dist = calculateDistance(agents[agent_id].x, agents[agent_id].y, team.home_bases[i][0], team.home_bases[i][1]);
            if(i == 0){
                min_dist = dist;
                hb.x = team.home_bases[i][0];
                hb.y = team.home_bases[i][1];
            }
            else if(dist < min_dist){
                min_dist = dist;
                hb.x = team.home_bases[i][0];
                hb.y = team.home_bases[i][1];
            }
        }

        float cur_hdg = agents[agent_id].heading;
        float x  = agents[agent_id].x;
        float y  = agents[agent_id].y;
        float x2 = hb.x;
        float y2 = hb.y;
        float diff_y = y2-y;
        float diff_x = x2-x;
        float smallest_angle = (atan2((y2-y) ,(x2-x))*180.0/M_PI);
        float atanval = atan((y2-y)/ (x2-x))*180.0f/M_PI;
        float angle_diff;
        angle_diff = fmod(450-to_360(smallest_angle),360) - to_360(cur_hdg);
        float angle_diff2 = fmod(450-to_360(smallest_angle),360) - 360+ to_360(cur_hdg);
        float heading_diff;

        if (fabs(angle_diff) < 10.0f ){
            heading_diff = angle_diff;
        }
        else{

            heading_diff = (angle_diff < 0) ? -175.0f : 175.0f;
        }
        agents[agent_id].move_agent(agents[agent_id].get_max_speed()/2.0f, heading_diff);
    }
    void CTFpyquaticus::drive_towards(int agent_id, point p){
        if (agents[agent_id].is_tagged){
            drive_home(agent_id);
            check_game_events();
            return;
        }
        Team team  = gteams[agents[agent_id].team_id];
        point hb = p;
        float cur_hdg = agents[agent_id].heading;
        float x  = agents[agent_id].x;
        float y  = agents[agent_id].y;
        float x2 = hb.x;
        float y2 = hb.y;
        float diff_y = y2-y;
        float diff_x = x2-x;
        float smallest_angle = (atan2((y2-y) ,(x2-x))*180.0/M_PI);
        float atanval = atan((y2-y)/ (x2-x))*180.0f/M_PI;
        float angle_diff;
        angle_diff = fmod(450-to_360(smallest_angle),360) - to_360(cur_hdg);
        float angle_diff2 = fmod(450-to_360(smallest_angle),360) - 360+ to_360(cur_hdg);
        float heading_diff;

        if (fabs(angle_diff) < 10.0f ){
            heading_diff = angle_diff;
        }
        else{

            heading_diff = (angle_diff < 0) ? -175.0f : 175.0f;
        }
        agents[agent_id].move_agent(agents[agent_id].get_max_speed()/2.0f, heading_diff);
        check_game_events();
    }






// // Add this Renderer






// PYBIND11_MODULE(pyquaticus, m) {
//     py::class_<CTFpyquaticus>(m, "CTFpyquaticus")
//         .def(py::init<const std::string&>())
//         .def("reset", &CTFpyquaticus::reset)
//         .def("step_agent", py::overload_cast<const std::string&, int>(&CTFpyquaticus::step_agent))
//         .def("step_agent", py::overload_cast<const std::string&, std::vector<double>>(&CTFpyquaticus::step_agent));

//     py::class_<Renderer>(m, "Renderer")
//         .def(py::init<>())
//         .def("init", &Renderer::Init)
//         .def("draw_field", &Renderer::DrawField)
//         .def("close", &Renderer::Close);
// }






int main() {
    CTFpyquaticus env("./config_1v1.json");
    // env.step({{"agent_0", 16}, {"agent_1",16}});
    Renderer render;
    render.Init();
    for(size_t i = 0; i < 6000; i++){
        // env.step_agents({{0,16},{1,16}});
        if(i <= 2500){
            env.drive_towards(0, {155.0, 65.0});
        }
        else{
            env.drive_towards(0, {15,40});
        }
        //std::cout << i<<std::endl;
        // env.step_agent(1, 0);
        //std::cout << "Agent 0 Pos: " << env.agents[0].x << ", "<<env.agents[0].y << std::endl;
        //std::cout << "Agent 1 Pos: " << env.agents[0].x << ", "<<env.agents[1].y << std::endl;
        render.DrawField(env);
        std::this_thread::sleep_for(std::chrono::milliseconds(20));

    }
    render.Close();
    // env.render();

    // for (int i = 0; i < 5; ++i) {
    //     std::unordered_map<std::string, int> actions = {
    //         {"agent_1", 1}, {"agent_2", -1}, {"agent_3", 2}
    //     };
    //     env.step(actions);
    //     env.render();
    //     if (env.done) break;
    // }

    return 0;
}