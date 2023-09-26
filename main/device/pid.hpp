#include <Arduino.h>
#include <cmath>
#include <cstdint>

class PID {
private:
    float _kp, _ki, _kd, _integrator, _imax;
    float _last_error, _last_derivative;
    uint32_t _last_t;
    const float _RC = 1 / (2 * M_PI * 20);

public:
    PID(float p = 0, float i = 0, float d = 0, float imax = 0) 
        : _kp(p), _ki(i), _kd(d), _integrator(0), _imax(std::abs(imax)), 
          _last_error(0), _last_derivative(std::nan("1")), _last_t(0) {}

    float get_pid(float error, float scaler) {
        uint32_t tnow = millis();
        uint32_t dt = tnow - _last_t;
        float output = 0;

        if (_last_t == 0 || dt > 1000) {
            dt = 0;
            reset_I();
        }

        _last_t = tnow;
        float delta_time = dt / 1000.0;

        output += error * _kp;

        if (std::abs(_kd) > 0 && dt > 0) {
            float derivative = 0;
            if (std::isnan(_last_derivative)) {
                derivative = 0;
                _last_derivative = 0;
            } else {
                derivative = (error - _last_error) / delta_time;
            }

            derivative = _last_derivative + 
                            ((delta_time / (_RC + delta_time)) * 
                            (derivative - _last_derivative));

            _last_error = error;
            _last_derivative = derivative;

            output += _kd * derivative;
        }

        output *= scaler;

        if (std::abs(_ki) > 0 && dt > 0) {
            _integrator += (error * _ki) * scaler * delta_time;
            if (_integrator < -_imax) _integrator = -_imax;
            else if (_integrator > _imax) _integrator = _imax;

            output += _integrator;
        }

        return output;
    }

    void reset_I() {
        _integrator = 0;
        _last_derivative = std::nan("1");
    }
};
