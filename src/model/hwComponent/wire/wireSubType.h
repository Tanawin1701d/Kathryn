//
// Created by tanawin on 25/6/2024.
//

#ifndef WIRESUBTYPE_H
#define WIRESUBTYPE_H

namespace kathryn{

    //// wire input output auto type
    enum WIRE_IO_TYPE{

        ////// this exposed only into the generation frame
        WIRE_IO_AUTO_INPUT,
        WIRE_IO_AUTO_OUTPUT,
        WIRE_IO_INTER,
        WIRE_IO_INPUT_GLOB,
        WIRE_IO_OUTPUT_GLOB,
        ////// this expose to user
        WIRE_IO_NORMAL,
        WIRE_IO_USER_INPUT,
        WIRE_IO_USER_OUPUT,
    };

    

}

#endif //WIRESUBTYPE_H
