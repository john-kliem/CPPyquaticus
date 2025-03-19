#include "PID.h"
#include <algorithm>
#include <limits>

class PID {
public:
    PID(double dt, double kp, double ki, double kd, double integral_max = std::numeric_limits<double>::infinity())
        : dt_(dt), kp_(kp), ki_(ki), kd_(kd), integral_max_(integral_max), prev_error_(0.0), integral_(0.0) {}

    double operator()(double error) {
        integral_ = std::min(integral_ + error * dt_, integral_max_);
        double deriv = (error - prev_error_) / dt_;

        prev_error_ = error;

        double p = kp_ * error;
        double i = ki_ * integral_;
        double d = kd_ * deriv;

        return p + i + d;
    }

private:
    double dt_;
    double kp_;
    double ki_;
    double kd_;
    double integral_max_;
    double prev_error_;
    double integral_;
};