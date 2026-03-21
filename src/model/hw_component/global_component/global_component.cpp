//
// Created by tanawin on 27/1/2567.
//

#include "global_component.h"


namespace kathryn{


    Wire*      rst_wire   = nullptr;
    StartNode* start_node = nullptr;


    void initiate_global_component(){

        rst_wire   = &make_opr_wire_wo_def("rst_wire", 1);
        rst_wire->as_input_glob("rst");
        start_node = new StartNode(rst_wire);
        start_node->assign();
    }

    void reset_global_component(){
         /** rst_wire and start node will be delete by module deconstructor*/
         rst_wire   = nullptr;
         start_node = nullptr;
    }

    Operable& get_reset_signal(){
        return *rst_wire;
    }

}