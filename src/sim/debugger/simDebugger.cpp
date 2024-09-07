//
// Created by tanawin on 7/9/2024.
//

#include "simDebugger.h"

#include <sim/controller/simController.h>
#include <util/termColor/termColor.h>

namespace kathryn{


    void smAssert(bool valid, const std::string& msg){

        if (valid){
            return;
        }

        std::cout << "[SIM ERROR] " << msg << std::endl;
        ///// -----------------------------------------------------------
        std::cout << "simulation is at CYCLE: "
                  << std::to_string(getSimController()->getCurCycle())
                  << std::endl;
        ///// -----------------------------------------------------------
        std::cout << "there is "
                  << std::to_string(getSimController()->getAmtCycle_force())
                  << std::endl;
        ///// -----------------------------------------------------------

        assert(false);
    }


    void smWarn(bool valid, const std::string& msg){
        if (valid){
            return;
        }
        std::cout << TC_YELLOW <<"[SIM WARN] " << msg << TC_DEF << std::endl;
    }

}
