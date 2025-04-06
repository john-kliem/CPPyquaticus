#ifndef TEAM_H
#define TEAM_H

#include <iostream>
#include <vector>
#include <unordered_map>
#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>
#include <cmath>
#include "agent.h"
struct point {
    double x, y;
};
double calculateDistance(double x1, double y1, double x2, double y2);
long long cross_product(point a, point b);
long long sq_dist(point a, point b);

class Flag {
  public:
      double x, y;
      bool is_grabbed;
      int grabbed_by;
      Flag(double x_pos, double y_pos);
      Flag(const Flag& other);
      Flag& operator=(const Flag& other);
      void displayPosition() const ;
      void reset();
      void set_name(int agent);
};

class Area{
  public:
    std::vector<point> points; // List of points [[x,y],[x1,y1], ..., [xn,yn]]
    Area(std::vector<std::vector<double>> points_);
    Area(const Area& other);
    Area& operator=(const Area& other);
    bool in_area(point p);
};

class Team{
  public:
    int team_id;
    std::unordered_map<int, bool> agents;
    std::vector<Flag> flags;
    std::vector<Area> areas;
    std::vector<std::vector<double>> home_bases;
    Team(int team_num, std::unordered_map<int, bool>& agents_, std::vector<Flag>& flag_pos, std::vector<Area>& areas_, std::vector<std::vector<double>>& home_bases_);
    Team(const Team& other);
    Team& operator=(const Team& other);
    void check_onside(Agent* teammate);
    bool agent_on_team(int agent_id);
    void reset_grab(int agent_id);
    bool check_grab(int agent_id, Agent* opp_agent);
};

#endif // TEAM_H 
