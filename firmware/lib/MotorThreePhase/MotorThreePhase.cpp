/*
    MotorThreePhase.cpp - Simple three phase motor/stepper driver library
*/
#include "Arduino.h"
#include "MotorThreePhase.h"

MotorThreePhase::MotorThreePhase(int pin1, int pin2, int pin3)
{
    _pin[0] = pin1;
    _pin[1] = pin2;
    _pin[2] = pin3;
    pinMode(_pin[0], OUTPUT);
    pinMode(_pin[1], OUTPUT);
    pinMode(_pin[2], OUTPUT);
}

void MotorThreePhase::update(unsigned long mircos)
{
    if(_motion != ROT_STOP && mircos >= (_last_mircos + _step_mircos)) {
        if(_motion == ROT_RIGHT) {
            _motion_step = (_motion_step + 1 == 6 ? 0 : _motion_step + 1);
        } else {
            _motion_step = (_motion_step - 1 == -1 ? 5 : _motion_step - 1);
        }
        digitalWrite(_pin[0], _drive[_motion_step][0]);
        digitalWrite(_pin[1], _drive[_motion_step][1]);
        digitalWrite(_pin[2], _drive[_motion_step][2]);
        // Callback
        if(_motion_step == 0) (callback)(_motion);
        _last_mircos = mircos;
    } else if (_motion == ROT_STOP) {
        digitalWrite(_pin[0], _stop[0]);
        digitalWrite(_pin[1], _stop[1]);
        digitalWrite(_pin[2], _stop[2]);
    } else if(mircos < _last_mircos) {
        // mircos has rolled over
        _last_mircos = mircos;
    } else {
        // Nothing
        //Serial.print(".");
    }
}

void MotorThreePhase::setSpeed(unsigned int hz)
{
    _step_mircos = 1000000 / (hz * 6);
}

void MotorThreePhase::stop()
{
    _motion = ROT_STOP;
}

void MotorThreePhase::rotateLeft()
{
    _motion = ROT_LEFT;
}

void MotorThreePhase::rotateRight()
{
    _motion = ROT_RIGHT;
}

void MotorThreePhase::setCallback(void (*callback)(unsigned int motion))
{
    this->callback = callback;
}
