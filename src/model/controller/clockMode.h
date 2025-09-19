//
// Created by tanawin on 19/9/25.
//

#ifndef SRC_MODEL_CONTROLLER_CLOCKMODE_H
#define SRC_MODEL_CONTROLLER_CLOCKMODE_H

namespace kathryn{

enum CLOCK_MODE{
    CM_POSEDGE,
    CM_NEGEDGE
};

extern CLOCK_MODE curClockMode;

void setClockMode(CLOCK_MODE mode);
CLOCK_MODE getClockMode();



}

#endif //SRC_MODEL_CONTROLLER_CLOCKMODE_H