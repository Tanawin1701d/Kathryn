//
// Created by tanawin on 4/2/2567.
//

#ifndef KATHRYN_SIMAUTOINTERFACE_H
#define KATHRYN_SIMAUTOINTERFACE_H

#include "sim/interface/sim_interface.h"

namespace kathryn{




    class SimAutoInterface: public SimInterface{
    private:
        int _simId = -1;
    public:

        explicit SimAutoInterface(int sim_id,
                                  CYCLE limit_cycle,
                                  std::string vcd_file_path,
                                  std::string profile_file_path,
                                  SimProxyBuildMode sim_proxy_build_mode,
                                  bool req_inline = true,
                                  int  op_level = 3,
                                  const std::string& gen_pref = "" ////// (optional)
                                  );

        int get_sim_id() const {return _simId;}



        virtual void sim_assert(){};
        virtual void sim_driven(){};

        void describe() override{
            /* drive the signal*/
            set_cycle(0);
            sim_driven();
            set_cycle(0);
            sim_assert();

        }


    };

}

#endif //KATHRYN_SIMAUTOINTERFACE_H
