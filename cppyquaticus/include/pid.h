#ifndef PID_H
#define PID_H

class PID {
public:
    PID(double dt, double kp, double ki, double kd, double integral_max = std::numeric_limits<double>::infinity());

    double operator()(double error);

private:
    double dt_;
    double kp_;
    double ki_;
    double kd_;
    double integral_max_;
    double prev_error_;
    double integral_;
};

#endif // PID_H
