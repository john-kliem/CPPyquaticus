#ifndef PID_H
#define PID_H

#include <limits>  // For std::numeric_limits

class PID {
public:
    PID(double dt, double kp, double ki, double kd, double integral_max = std::numeric_limits<double>::infinity());
    PID(const PID& other);
    PID& operator=(const PID& other);
    double operator()(double error);


private:
    double dt_, kp_, ki_, kd_, integral_max_;
    double integral_;
    double prev_error_;
};

#endif  // PID_H
