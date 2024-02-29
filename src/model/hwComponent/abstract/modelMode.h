//
// Created by tanawin on 29/2/2567.
//

#ifndef KATHRYN_MODELMODE_H
#define KATHRYN_MODELMODE_H

namespace kathryn{


    enum ASSIGN_MODE{
        AM_MOD,
        AM_SIM
    };

    ASSIGN_MODE getAssignMode();
    void        setAssignMode(ASSIGN_MODE asMode);

}

#endif //KATHRYN_MODELMODE_H
