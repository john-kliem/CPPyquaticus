#include "pid.h"
#include <iostream>
#include <vector>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <array>
#include <limits>

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
class Agent;  // Ensure the compiler knows about Agent before usage

class Agent {
public:
    float x;
    float y;
    float prev_x;
    float prev_y;
    float heading;
    float radius;
    float thrust;
    float rudder;
    float max_thrust;
    float max_rudder;
    float turn_loss;
    float turn_rate;
    float max_acc;
    float max_dec;
    float rotate_speed;
    float speed;
    float max_speed;
    std::array<std::array<double, 7>, 2> thrust_map; // The thrust_map array
    PID heading_pid;
    PID speed_pid;
    float speed_factor;
    float dt;

    // Constructor for Agent class
    Agent(float x_pos, float y_pos, float max_speeds, float heading_angle, float agent_radius, 
        float max_thrust_val, float max_rudder_val, float turn_loss_val, float turn_rate_val, 
        float max_acc_val, float max_dec_val, float rotate_speed_val,
        std::array<std::array<double, 7>, 2> thrust_map_val,
        double pid_dts, double pid_kps, double pid_kis, double pid_kds, double integral_maxs,
        double pid_dth, double pid_kph, double pid_kih, double pid_kdh, double integral_maxh)
        : x(x_pos), 
          y(y_pos), 
          max_speed(max_speeds),
          heading(heading_angle), 
          radius(agent_radius), 
          max_thrust(max_thrust_val), 
          max_rudder(max_rudder_val), 
          turn_loss(turn_loss_val), 
          turn_rate(turn_rate_val), 
          max_acc(max_acc_val), 
          max_dec(max_dec_val), 
          speed(0.0),
          rotate_speed(rotate_speed_val), 
          thrust_map(thrust_map_val), thrust(0.0), rudder(0.0), prev_x(0.0), prev_y(0.0), 
          heading_pid(pid_dth, pid_kph, pid_kih, pid_kdh, integral_maxh), 
          speed_pid(pid_dts, pid_kps, pid_kis, pid_kds, integral_maxs), speed_factor(0.0),
          dt(pid_dts){}
    
    

    void move_agent(int action){
        
    }
    void move_agent(int speed, int heading){
        
    }

    void reset(){
        thrust = 0.0;
        rudder = 0.0;
    }
    void rotate(float theta=180){
        prev_x = x;
        prev_y = y;
        speed = 0.0;
        heading = angle180(heading + theta);
        thrust = 0.0;
        rudder = 0.0;
    }
    float get_max_speed(){
        return max_speed;
    }
    void set_desired_thrust(float desired_speed) {
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
    void set_desired_rudder(float heading_error) {
        float desired_rudder = heading_pid(heading_error);
        rudder = std::clamp(desired_rudder, -max_rudder, max_rudder);
    }
    float propagate_speed(float thrust, float rudder) {
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
    float propagate_heading(float speed, float thrust, float rudder) {
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
    virtual ~Agent() = default;  // Virtual destructor for proper cleanup of derived objects
};

class Surveyor : public Agent {
public:
    // Define a fixed thrust map that the Surveyor class will use
    static constexpr std::array<std::array<double, 7>, 2> fixed_thrust_map = {{
        {-100.0, 0.0, 20.0, 40.0, 60.0, 70.0, 100.0}, 
        {-2.0, 0.0, 1.0, 1.5, 2.0, 2.25, 3.0}
    }};
    
    // Surveyor constructor initializes Agent's constructor
    //double pid_dts, double pid_kps, double pid_kis, double pid_kds, double integral_maxs,
    Surveyor(float x_pos, float y_pos, float heading_angle, float radius)
        : Agent(x_pos, y_pos, 3.0, heading_angle, radius, 100, 100, 0.85, 10, 0.15, 0.25, 0.0, 
        fixed_thrust_map, 0.1, 0.5, 0.0, 0.0, 0.0,
         0.1, 1.2, 0.0, 3.0, 0.0) {}
};