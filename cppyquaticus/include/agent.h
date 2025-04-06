#ifndef AGENT_H
#define AGENT_H

#include "pid.h"
#include <iostream>
#include <vector>
#include <unordered_map>
#include <array>
#include <algorithm>
#include <limits>
#include <cmath>
#include <tuple>

// Function to constrain angles between -180 and 180 degrees
float angle180(float deg);

// Function for linear interpolation
double linearInterp(double x, const std::vector<double>& x_vals, const std::vector<double>& y_vals);

class Agent {
public:
    float x, y, heading, radius, thrust, rudder, max_thrust, max_rudder;
    float prev_x, prev_y, prev_heading;
    int team_id;
    float tag_cooldown;
    float max_cooldown;
    float speed, max_speed, turn_loss, turn_rate, max_acc, max_dec, rotate_speed, dt;
    int on_side; 
    bool is_tagged;
    float has_flag;
    float tag_range, capture_range, agent_radius;
    std::array<std::array<double, 7>, 2> thrust_map;
    PID heading_pid, speed_pid;
    float speed_factor;

    Agent(float x_pos, float y_pos, float max_speeds, float heading_angle, float agent_radius,
          float max_thrust_val, float max_rudder_val, float turn_loss_val, float turn_rate_val,
          float max_acc_val, float max_dec_val, float rotate_speed_val,
          std::array<std::array<double, 7>, 2> thrust_map_val,
          double pid_dts, double pid_kps, double pid_kis, double pid_kds, double integral_maxs,
          double pid_dth, double pid_kph, double pid_kih, double pid_kdh, double integral_maxh, int team_id);
    Agent(const Agent& other);
    Agent& operator=(const Agent& other);
    void move_agent(int action);
    void move_agent(double speed, double heading);
    void _move_agent(float desired_speed, float heading_error);
    void reset();
    void rotate(float theta=180.0);
    float get_max_speed();
    void set_desired_thrust(float desired_speed);
    void set_desired_rudder(float heading_error);
    float propagate_speed(float thrust, float rudder);
    float propagate_heading(float speed, float thrust, float rudder);
    std::tuple<float, float, float> propagate_pos(float new_speed, float new_heading);


    virtual ~Agent() = default;
};

class Surveyor : public Agent {
public:
    static constexpr std::array<std::array<double, 7>, 2> fixed_thrust_map = {{
        {-100.0, 0.0, 20.0, 40.0, 60.0, 70.0, 100.0},
        {-2.0, 0.0, 1.0, 1.5, 2.0, 2.25, 3.0}
    }};

    Surveyor(float x_pos, float y_pos, float heading_angle, float radius, int team_id);
    Surveyor(const Surveyor& other);
    Surveyor& operator=(const Surveyor& other);
};

#endif // AGENT_H
