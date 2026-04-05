//
// Created by tanawin on 25/6/2024.
//

#ifndef WIRESUBTYPE_H
#define WIRESUBTYPE_H

namespace kathryn{

    enum WIRE_AUTO_GEN_TYPE{
        WIRE_AUTO_GEN_INPUT       = 0,
        WIRE_AUTO_GEN_OUTPUT      = 1,
        WIRE_AUTO_GEN_INTER       = 2,
        WIRE_AUTO_GEN_GLOB_INPUT  = 3,
        WIRE_AUTO_GEN_GLOB_OUTPUT = 4,
        WIRE_AUTO_GEN_CNT         = 6
    };

}

#endif //WIRESUBTYPE_H
