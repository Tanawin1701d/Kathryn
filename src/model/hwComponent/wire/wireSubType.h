//
// Created by tanawin on 25/6/2024.
//

#ifndef WIRESUBTYPE_H
#define WIRESUBTYPE_H

namespace kathryn{

    //// wire input output auto type
    constexpr int AMT_PHY_WIRE = 7;
    enum WIRE_IO_TYPE{
        ////// this exposed only into the generation frame
        WIRE_IO_AUTO_INPUT  = 0,
        WIRE_IO_AUTO_OUTPUT = 1,
        WIRE_IO_INTER       = 2,
        WIRE_IO_INPUT_GLOB  = 3,
        WIRE_IO_OUTPUT_GLOB = 4,
        WIRE_IO_USER_INPUT  = 5,
        WIRE_IO_USER_OUPUT  = 6,
        ////// 0 - 6 module gen holder The srcture
        WIRE_IO_BCI         = 7,
        WIRE_IO_NORMAL      = 8,

    };

    

}

#endif //WIRESUBTYPE_H
