#include "pid.h"
#include <algorithm>
#include <limits>


PID::PID(double dt, double kp, double ki, double kd, double integral_max)
        : dt_(dt), kp_(kp), ki_(ki), kd_(kd), integral_max_(integral_max), prev_error_(0.0), integral_(0.0) {}
PID::PID(const PID& other)
    : dt_(other.dt_), kp_(other.kp_), ki_(other.ki_), kd_(other.kd_),
      integral_max_(other.integral_max_), prev_error_(other.prev_error_), integral_(other.integral_) {}

PID& PID::operator=(const PID& other) {
    if (this == &other) return *this;
    dt_ = other.dt_;
    kp_ = other.kp_;
    ki_ = other.ki_;
    kd_ = other.kd_;
    integral_max_ = other.integral_max_;
    prev_error_ = other.prev_error_;
    integral_ = other.integral_;

    return *this;
}
double PID::operator()(double error) {
    integral_ = std::min(integral_ + error * dt_, integral_max_);
    double deriv = (error - prev_error_) / dt_;

    prev_error_ = error;

    double p = kp_ * error;
    double i = ki_ * integral_;
    double d = kd_ * deriv;

    return p + i + d;
}
