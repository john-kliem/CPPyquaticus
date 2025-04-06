#include "pid.h"
#include "agent.h"
#include <iostream>
#include <vector>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <array>
#include <limits>
#include <cmath>
#include <tuple>

//TODO: Fix this to work with json config

const int NUM_HEADINGS = 8;
const int NUM_SPEEDS = 2;

const float HEADINGS[NUM_HEADINGS] = {180, 135, 90, 45, 0, 315, 270, 225};
const float SPEEDS[NUM_SPEEDS] = {1.5, 3.0};



float angle180(float deg) {
    while (deg > 180.0f) {
        deg -= 360.0f;
    }
    while (deg < -180.0f) {
        deg += 360.0f;
    }
    return deg;
}

double linearInterp(double x, const std::vector<double>& x_vals, const std::vector<double>& y_vals) {
    if (x_vals.size() != y_vals.size() || x_vals.empty()) {
        throw std::invalid_argument("Input vectors must have the same non-zero size.");
    }

    // If x is out of bounds, clamp it to the nearest boundary
    if (x <= x_vals.front()) return y_vals.front();
    if (x >= x_vals.back()) return y_vals.back();

    // Find the interval [x_i, x_i+1] such that x_i <= x < x_i+1
    auto it = std::upper_bound(x_vals.begin(), x_vals.end(), x);
    size_t idx = std::distance(x_vals.begin(), it) - 1;

    double x0 = x_vals[idx], x1 = x_vals[idx + 1];
    double y0 = y_vals[idx], y1 = y_vals[idx + 1];

    // Linear interpolation formula
    return y0 + (x - x0) * (y1 - y0) / (x1 - x0);
}

std::pair<float, float> getActionMapping(int action) {
    if (action == 16){
        return {0.0,0.0};
    }
    int headingIndex = action % NUM_HEADINGS;
    int speedIndex = action / NUM_HEADINGS;
    
    return {HEADINGS[headingIndex], SPEEDS[speedIndex]};
}


// Constructor for Agent class
Agent::Agent(float x_pos, float y_pos, float max_speeds, float heading_angle, float agent_radius, 
    float max_thrust_val, float max_rudder_val, float turn_loss_val, float turn_rate_val, 
    float max_acc_val, float max_dec_val, float rotate_speed_val,
    std::array<std::array<double, 7>, 2> thrust_map_val,
    double pid_dts, double pid_kps, double pid_kis, double pid_kds, double integral_maxs,
    double pid_dth, double pid_kph, double pid_kih, double pid_kdh, double integral_maxh, int team_id_)
    : x(x_pos), 
      y(y_pos), 
      max_speed(max_speeds),
      heading(heading_angle), 
      prev_heading(heading_angle),
      radius(agent_radius), 
      max_thrust(max_thrust_val), 
      max_rudder(max_rudder_val), 
      turn_loss(turn_loss_val), 
      turn_rate(turn_rate_val), 
      max_acc(max_acc_val), 
      max_dec(max_dec_val), 
      speed(0.0),
      agent_radius(agent_radius),
      team_id(team_id_),
      rotate_speed(rotate_speed_val), 
      thrust_map(thrust_map_val), thrust(0.0), rudder(0.0), prev_x(0.0), prev_y(0.0), 
      heading_pid(pid_dth, pid_kph, pid_kih, pid_kdh, integral_maxh), 
      speed_pid(pid_dts, pid_kps, pid_kis, pid_kds, integral_maxs), speed_factor(0.0),
      dt(pid_dts),
      on_side(team_id_), is_tagged(false), has_flag(-1), tag_range(10.0), tag_cooldown(0.0), max_cooldown(60.0), capture_range(10.0){}

Agent::Agent(const Agent& other)
    : x(other.x), y(other.y), max_speed(other.max_speed), heading(other.heading), prev_heading(other.prev_heading),
      radius(other.radius), max_thrust(other.max_thrust), max_rudder(other.max_rudder), turn_loss(other.turn_loss),
      turn_rate(other.turn_rate), max_acc(other.max_acc), max_dec(other.max_dec), speed(other.speed),
      rotate_speed(other.rotate_speed), thrust(other.thrust), rudder(other.rudder),
      prev_x(other.prev_x), prev_y(other.prev_y), heading_pid(other.heading_pid),
      speed_pid(other.speed_pid), speed_factor(other.speed_factor), dt(other.dt),
      on_side(other.on_side), is_tagged(other.is_tagged), has_flag(other.has_flag), tag_range(other.tag_range),
      thrust_map(other.thrust_map), team_id(other.team_id), agent_radius(other.agent_radius), tag_cooldown(other.tag_cooldown), max_cooldown(other.max_cooldown), capture_range(other.capture_range){}

Agent& Agent::operator=(const Agent& other) {
    if (this == &other) return *this;

    x = other.x;
    y = other.y;
    tag_cooldown = other.tag_cooldown;
    max_cooldown = other.max_cooldown;
    max_speed = other.max_speed;
    heading = other.heading;
    prev_heading = other.prev_heading;
    radius = other.radius;
    max_thrust = other.max_thrust;
    max_rudder = other.max_rudder;
    turn_loss = other.turn_loss;
    turn_rate = other.turn_rate;
    max_acc = other.max_acc;
    max_dec = other.max_dec;
    speed = other.speed;
    rotate_speed = other.rotate_speed;
    thrust = other.thrust;
    rudder = other.rudder;
    prev_x = other.prev_x;
    prev_y = other.prev_y;
    heading_pid = other.heading_pid; // Copy PID objects
    speed_pid = other.speed_pid;
    speed_factor = other.speed_factor;
    dt = other.dt;
    on_side = other.on_side;
    is_tagged = other.is_tagged;
    has_flag = other.has_flag;
    tag_range = other.tag_range;
    team_id = other.team_id;
    agent_radius = other.agent_radius;
    thrust_map = other.thrust_map; // Copy thrust_map array
    capture_range = other.capture_range;

    return *this;
}



// Move Agent - Overloaded Version
void Agent::move_agent(int action) {
    // Convert action into desired speed and heading adjustment
    std::pair<float, float> speed_heading = getActionMapping(action);
    _move_agent(speed_heading.second, speed_heading.first);
}

// Move Agent with Explicit Speed and Heading
void Agent::move_agent(double speed, double heading) {
    _move_agent(static_cast<float>(speed), static_cast<float>(heading));
}

// Core Movement Logic (from _move_agent)
void Agent::_move_agent(float desired_speed, float heading_error) {
    // Set thrust based on desired speed
    set_desired_thrust(desired_speed);

    if (thrust > 0) {
        set_desired_rudder(heading_error);
        rudder = std::clamp(rudder, -100.0f, 100.0f);
    }

    // Propagate Speed, Heading, and Position
    float new_speed = propagate_speed(thrust, rudder);
    float new_heading = propagate_heading(new_speed, thrust, rudder);
    auto [new_x, new_y, new_final_speed] = propagate_pos(new_speed, new_heading);

    // Update Agent's State
    speed = std::clamp(new_final_speed, 0.0f, max_speed);
    tag_cooldown = std::clamp(tag_cooldown - dt, 0.0f, max_cooldown);
    heading = angle180(new_heading);
    prev_x = x;
    prev_y = y;
    x = new_x;
    y = new_y;
}

void Agent::reset(){
    thrust = 0.0;
    rudder = 0.0;
}

void Agent::rotate(float theta){
    prev_x = x;
    prev_y = y;
    speed = 0.0;
    heading = angle180(heading + theta);
    thrust = 0.0;
    rudder = 0.0;
}

float Agent::get_max_speed(){
    return max_speed;
}

void Agent::set_desired_thrust(float desired_speed) {
    float desired_thrust;
    if (speed_factor != 0.0f) {
        desired_thrust = desired_speed * speed_factor;
    } else {
        float speed_error = desired_speed - speed;
        float delta_thrust = speed_pid(speed_error);
        desired_thrust = thrust + delta_thrust;
    }

    if (desired_thrust < 0.01f) {
        desired_thrust = 0.0f;
    }

    thrust = std::clamp(desired_thrust, -max_thrust, max_thrust);
}

void Agent::set_desired_rudder(float heading_error) {
    float desired_rudder = heading_pid(heading_error);
    rudder = std::clamp(desired_rudder, -max_rudder, max_rudder);
}

float Agent::propagate_speed(float thrust, float rudder) {
    std::vector<double> thrust_vals(thrust_map[0].begin(), thrust_map[0].end());
    std::vector<double> speed_vals(thrust_map[1].begin(), thrust_map[1].end());
    
    float next_speed = static_cast<float>(linearInterp(thrust, thrust_vals, speed_vals));

    next_speed *= 1.0f - ((std::abs(rudder) / 100.0f) * turn_loss);

    if ((next_speed - speed) / dt > max_acc) {
        next_speed = speed + max_acc * dt;
    } else if ((speed - next_speed) / dt > max_dec) {
        next_speed = speed - max_dec * dt;
    }
    return next_speed;
}

float Agent::propagate_heading(float speed, float thrust, float rudder) {
    if (speed == 0.0f) {
        rudder = 0.0f;
    }
    float delta_deg = rudder * (turn_rate / 100.0f) * dt;
    delta_deg *= 1.0f + (std::abs(thrust) - 50.0f) / 50.0f;
    if (thrust < 0.0f) {
        delta_deg = -delta_deg;
    }
    delta_deg += rotate_speed * dt;
    
    return angle180(heading + delta_deg);
}

std::tuple<float, float, float> Agent::propagate_pos(float new_speed, float new_heading) {
    // Calculate average speed
    float avg_speed = (new_speed + speed) / 2.0f;

    // Convert headings to radians
    float hdg_rad = heading * M_PI / 180.0f;
    float new_hdg_rad = new_heading * M_PI / 180.0f;

    // Calculate average heading using sine and cosine components
    float s = std::sin(new_hdg_rad) + std::sin(hdg_rad);
    float c = std::cos(new_hdg_rad) + std::cos(hdg_rad);
    float avg_hdg = std::atan2(s, c); // Arctangent to get angle

    // Compute velocity vector
    float vel_x = avg_speed * std::sin(avg_hdg);
    float vel_y = avg_speed * std::cos(avg_hdg);
    float new_speed_final = std::sqrt(vel_x * vel_x + vel_y * vel_y); // Magnitude of velocity

    // Compute new position
    float new_x, new_y;
    
    new_x = x + vel_x * dt;
    new_y = y + vel_y * dt;

    return {new_x, new_y, new_speed_final};
}
// Surveyor constructor initializes Agent's constructor
//double pid_dts, double pid_kps, double pid_kis, double pid_kds, double integral_maxs,
Surveyor::Surveyor(float x_pos, float y_pos, float heading_angle, float radius, int team_id)
    : Agent(x_pos, y_pos, 3.0, heading_angle, radius, 100, 100, 0.85, 10, 0.15, 0.25, 0.0, 
    fixed_thrust_map, 0.1, 0.5, 0.0, 0.0, 0.0,
     0.1, 1.2, 0.0, 3.0, 0.0, team_id) {}
Surveyor::Surveyor(const Surveyor& other) : Agent(other) {}

Surveyor& Surveyor::operator=(const Surveyor& other) {
    if (this == &other) return *this;

    Agent::operator=(other);

    return *this;
}
