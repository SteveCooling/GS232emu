#ifndef MotorThreePhase_h
#define MotorThreePhase_h

#include "Arduino.h"

class MotorThreePhase
{
    private:
        const bool _stop[3] = { LOW, LOW, LOW };
        const bool _drive[6][3] = {
            { HIGH, LOW,  LOW  },
            { HIGH, HIGH, LOW  },
            { LOW,  HIGH, LOW  },
            { LOW,  HIGH, HIGH },
            { LOW,  LOW,  HIGH },
            { HIGH, LOW,  HIGH },
        };

        int _pin [3] = { 0, 0, 0 };
        unsigned char _motion = ROT_STOP;
        signed int _motion_step = 0;
        unsigned long _last_mircos = 0;
        unsigned long _step_mircos = 9;
        // Gets called every full motor step
        void (*callback)(unsigned int motion);
    public:
        static const int ROT_STOP  = 0;
        static const int ROT_LEFT  = 1;
        static const int ROT_RIGHT = 2;
        MotorThreePhase(int pin1, int pin2, int pin3);
        void update(unsigned long mircos);
        void setSpeed(unsigned int hz);
        void rotateLeft();
        void rotateRight();
        void stop();
        void setCallback(void (*callback)(unsigned int motion));
};

#endif
