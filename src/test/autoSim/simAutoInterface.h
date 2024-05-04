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
                                  std::string vcdFilePath,
                                  std::string profileFilePath
                                  );

        int getSimId() const {return _simId;}



        virtual void simAssert(){};
        virtual void simDriven(){};

        void describe() override{
            /* drive the signal*/
            setCycle(0);
            simDriven();
            setCycle(0);
            simAssert();

        }

        void flush(){
            _vcdWriter->flush();
        }


    };

}

#endif //KATHRYN_SIMAUTOINTERFACE_H
