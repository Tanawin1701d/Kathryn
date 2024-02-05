//
// Created by tanawin on 4/2/2567.
//

#ifndef KATHRYN_SIMAUTOINTERFACE_H
#define KATHRYN_SIMAUTOINTERFACE_H

#include "sim/interface/simInterface.h"

namespace kathryn{




    class SimAutoInterface: public SimInterface{
    private:
        int _simId = -1;
    public:

        explicit SimAutoInterface(int simId,
                                  CYCLE limitCycle,
                                  std::string vcdFilePath);

        int getSimId() const {return _simId;};


    };

}

#endif //KATHRYN_SIMAUTOINTERFACE_H
