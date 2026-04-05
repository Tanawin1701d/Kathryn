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
                                  std::string profileFilePath,
                                  SimProxyBuildMode simProxyBuildMode,
                                  bool reqInline = true,
                                  int  opLevel = 3,
                                  const std::string& genPref = "" ////// (optional)
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


    };

}

#endif //KATHRYN_SIMAUTOINTERFACE_H
