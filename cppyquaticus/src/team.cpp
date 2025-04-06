#include <iostream>
#include <vector>
#include <unordered_map>
#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>
#include <cmath>
#include "team.h"
//pybind11

double calculateDistance(double x1, double y1, double x2, double y2) {
    return sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2));
}

long long cross_product(point a, point b){
    return a.x*b.y-b.x*a.y;
}

long long sq_dist(point a, point b){
    return (a.x-b.x)*(a.x-b.x)+(a.y-b.y)*(a.y-b.y);
}

Flag::Flag(double x_pos, double y_pos) : x(x_pos), y(y_pos), is_grabbed(false), grabbed_by(-1) {}

Flag::Flag(const Flag& other) : x(other.x), y(other.y), is_grabbed(other.is_grabbed), grabbed_by(other.grabbed_by) {}

// Copy Assignment Operator (Deep Copy)
Flag& Flag::operator=(const Flag& other) {
    if (this == &other) return *this; // Prevent self-assignment
    // Copy primitive and string members
    x = other.x;
    y = other.y;
    is_grabbed = other.is_grabbed;
    grabbed_by = other.grabbed_by;
    return *this;
}

void Flag::displayPosition() const {
    std::cout << "Flag Position -> X: " << x << ", Y: " << y << std::endl;
}
void Flag::reset()
{
    this->is_grabbed = false;
    this->grabbed_by = -1;
}
void Flag::set_name(int agent){
    this->grabbed_by = agent;
}

Area::Area(std::vector<std::vector<double>> points_) {
    if(points_.size() < 3){
        std::cout << "ERROR an area must have 3 or more points!" << std::endl;
        return;
    }
    for(size_t i = 0; i < points_.size(); i++){
        points.push_back({points_[i][0],points_[i][1]});
    }
}

Area::Area(const Area& other) {
    for (const auto& p : other.points) {
        points.push_back(p);
    }
}
Area& Area::operator=(const Area& other) {
    if (this == &other) return *this;
    points.clear();
    for (const auto& p : other.points) {
        points.push_back(p);
    }
    return *this;
}

// bool Area::in_area(point p){

//     point p1 = {points[points.size()-1].x-points[0].x, points[points.size()-1].y-points[0].y};
//     point p2 = {points[1].x-points[0].x, points[1].y-points[0].y};
//     point pq = {p.x-points[0].x,p.y-points[0].y};
//     if(!(cross_product(p1,pq)<=0 && cross_product(p2,pq)>=0)) return false;
//     int l = 0, r = points.size();
//     while(r-l > 1){
//         int mid = (l+r) / 2;
//         point cur = {points[mid].x-points[0].x, points[mid].y-points[0].y};
//         if(cross_product(cur,pq)<0){
//             r = mid;
//         }
//         else{
//             l = mid;
//         }
//     }
//     if(l == points.size()-1){
//         return sq_dist(points[0],{p.x,p.y}) <= sq_dist(points[0],points[l]);
//     }
//     else{
//         point l_l1 = {points[l+1].x-points[l].x, points[l+1].y-points[l].y};
//         point lq = {p.x-points[l].x, p.y-points[l].y};
//         return (cross_product(l_l1,lq) >= 0);
//     }
    
// }
bool Area::in_area(point p){
    int intersections = 0;
    for(size_t i = 0; i < points.size(); i++){
        point p1 = points[i];
        point p2 = points[(i+1)%points.size()];
        if (((p.y < p1.y) != (p.y < p2.y)) && (p.x<((p2.x-p1.x)*(p.y-p1.y)/(p2.y-p1.y)+p1.x))){
            intersections++;
        }
    }
    return intersections % 2 == 1;
}

Team::Team(int team_num, std::unordered_map<int, bool>& agents_, std::vector<Flag>& flag_pos, std::vector<Area>& areas_, std::vector<std::vector<double>>& home_bases_)
: team_id(team_num),home_bases(home_bases_), agents(agents_), areas(areas_), flags(flag_pos){
    // For configs with more than one flag add to game
    // for (size_t i = 0; i < flag_pos.size(); ++i) {
    //     flags.push_back(Flag(flag_pos[i][0],flag_pos[i][1]));
    // }
    // for (size_t i = 0; i< areas_.size(); ++i) {
    //     areas.push_back(areas_[i]);
    // }
}


Team::Team(const Team& other): team_id(other.team_id), agents(other.agents), areas(other.areas), home_bases(other.home_bases) {
    
    areas.clear();
    for (const auto& area : other.areas){
        areas.emplace_back(area);
    }

    flags.clear();  // Ensure empty before deep copying
    for (const auto& flag : other.flags) {
        flags.emplace_back(flag);  // Calls Flag's copy constructor
    }
}

Team& Team::operator=(const Team& other) {
    if (this == &other) return *this; // Prevent self-assignment

    // Copy fields
    team_id = other.team_id;
    agents = other.agents;
    home_bases = other.home_bases;

    // Deep copy flags
    flags.clear();
    for (const auto& flag : other.flags) {
        flags.emplace_back(flag);
    }
    // Deep copy Areas
    areas.clear();
    for (const auto& area : other.areas){
        areas.emplace_back(area);
    }
    return *this;
}
void Team::check_onside(Agent* player){
    // Loop through to determine if in area
    //player->on_side = false;
    for (size_t i = 0; i< areas.size(); i++){
        if (areas[i].in_area({player->x, player->y})){
            player->on_side = team_id;


        }
    }
    return;
}


// Returns true if agent is on team
bool Team::agent_on_team(int agent_id){
    if (agents.find(agent_id) != agents.end()) {
        return true;
    }
    return false;
}
void Team::reset_grab(int agent_id){
    for (size_t i = 0; i < flags.size(); i++){
        if(flags[i].grabbed_by == agent_id){
            flags[i].reset();
        }
    }
}
// // Check if flag could have been grabbed by an agent
// bool Team::check_home(int agent_id, Agent* opp_agent){
//     //Loops through all agents then determines if the agent is in any of the teams areas
//     if (opp_agent->is_tagged ||(opp_agent->has_flag > -1) || !(opp_agent->on_side==team_id)){
//         return false;
//     }
//     for (size_t i = 0; i < home_bases.size(); i++){
//         if(!flags[i].is_grabbed){
//             double dist = calculateDistance(opp_agent->x, opp_agent->y, home_bases[i][0], home_bases[i][0]);
//             if (dist < opp_agent->capture_range){
                
//                 opp_agent->has_flag = team_id;
//                 flags[i].is_grabbed = true;
//                 flags[i].set_name(agent_id);
//                 return true;
//             }
//         }
//     }
//     return false;
// }
bool Team::check_grab(int agent_id, Agent* opp_agent){
    //Loops through all agents then determines if the agent is in any of the teams areas
    if (opp_agent->is_tagged ||(opp_agent->has_flag > -1) || !(opp_agent->on_side==team_id)){
        return false;
    }
    for (size_t i = 0; i < flags.size(); i++){
        if(!flags[i].is_grabbed){
            double dist = calculateDistance(opp_agent->x, opp_agent->y, flags[i].x, flags[i].y);
            if (dist < opp_agent->capture_range && !flags[i].is_grabbed){
                
                opp_agent->has_flag = team_id;
                flags[i].is_grabbed = true;
                flags[i].set_name(agent_id);
                return true;
            }
        }
    }
    return false;
}