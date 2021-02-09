#include "wheel_controller.hpp"
#include <driver/imu.hpp>
#include <peripheral/vector_controller.hpp>
#include "centralized_monitor.hpp"
#include "shared_memory.hpp"
#include <math.h>

void WheelController::StartControl(void) {
    VectorController::SetGainP(3500);
    VectorController::SetGainI(500);
    VectorController::ClearFault();
    VectorController::SetCurrentReferenceQ(1, 0);
    VectorController::SetCurrentReferenceQ(2, 0);
    VectorController::SetCurrentReferenceQ(3, 0);
    VectorController::SetCurrentReferenceQ(4, 0);
    memset(_LastSpeedError, 0, sizeof(_LastSpeedError));
    memset(_LastCurrentReference, 0, sizeof(_LastCurrentReference));
}

void WheelController::StopControl(void) {
    VectorController::SetFault();
    VectorController::SetCurrentReferenceQ(1, 0);
    VectorController::SetCurrentReferenceQ(2, 0);
    VectorController::SetCurrentReferenceQ(3, 0);
    VectorController::SetCurrentReferenceQ(4, 0);
    VectorController::SetGainP(0);
    VectorController::SetGainI(0);
}

void WheelController::Update(bool new_parameters) {
    if (VectorController::IsFault() == false) {
        int speed_meas_i[4];
        speed_meas_i[0] = VectorController::GetEncoderValue(1);
        speed_meas_i[1] = VectorController::GetEncoderValue(2);
        speed_meas_i[2] = VectorController::GetEncoderValue(3);
        speed_meas_i[3] = VectorController::GetEncoderValue(4);
        for (int index = 0; index < 4; index++) {
            float speed_ref = SharedMemory::GetParameters().wheel_speed[index];
            if (fabsf(speed_ref) <= MaxSpeedReference) {
                float speed_meas = speed_meas_i[index];
                float error = speed_ref - speed_meas;
                float value_i = _SpeedGainI * error;
                float value_p = _SpeedGainP * (error - _LastSpeedError[index]);
                _LastSpeedError[index] = error;

                float current_ref = _LastCurrentReference[index];
                current_ref += (value_p + value_i) * 0.0000152587890625f;
                current_ref = fmaxf(-MaxCurrentReference, fminf(current_ref, MaxCurrentReference));
                _LastCurrentReference[index] = current_ref;
            }
            else {
                _LastCurrentReference[index] = 0.0f;
                _LastSpeedError[index] = 0.0f;
            }
        }
        VectorController::SetCurrentReferenceQ(1, static_cast<int>(_LastCurrentReference[0] * 1977.0f));
        VectorController::SetCurrentReferenceQ(2, static_cast<int>(_LastCurrentReference[1] * 1977.0f));
        VectorController::SetCurrentReferenceQ(3, static_cast<int>(_LastCurrentReference[2] * 1977.0f));
        VectorController::SetCurrentReferenceQ(4, static_cast<int>(_LastCurrentReference[3] * 1977.0f));
    }
    else if (new_parameters == true) {
        StartControl();
    }
}

void WheelController::SetGains(float kp, float ki) {
    /*auto clamp = [](float value) -> int {
     value = (0.0f <= value) ? value : 0.0f;
     value = (value * (1u << VectorController::GainScale)) + 0.5f;
     value = (value <= 65535.0f) ? value : 65535.0f;
     return static_cast<int>(value);
     };
     VectorController::SetGainP(clamp(kp));
     VectorController::SetGainI(clamp(ki));*/
    _SpeedGainP = fmaxf(0.0f, fminf(kp, 10000.0f));
    _SpeedGainI = fmaxf(0.0f, fminf(ki, 10000.0f));
}

void WheelController::GetGains(float *kp, float *ki) {
    //*kp = static_cast<float>(VectorController::GetGainP()) * (1.0f / (1u << VectorController::GainScale));
    //*ki = static_cast<float>(VectorController::GetGainI()) * (1.0f / (1u << VectorController::GainScale));
    if (kp != nullptr) {
        *kp = _SpeedGainP;
    }
    if (ki != nullptr) {
        *ki = _SpeedGainI;
    }
}

float WheelController::_SpeedGainP;
float WheelController::_SpeedGainI;
float WheelController::_LastSpeedError[4];
float WheelController::_LastCurrentReference[4];
