//
// Created by tanawin on 1/8/2024.
//

#ifndef WIREIOUSER_H
#define WIREIOUSER_H
#include "wire.h"


namespace kathryn{

/***
 *
 * this is exposed to user declaration
 *
 */
class WireIoUser: public Wire{

protected:
    WIRE_IO_TYPE _wireIoType;
    void com_init() override;

public:
    WireIoUser(int size, WIRE_IO_TYPE wireIoType);

    [[nodiscard]]
    WIRE_IO_TYPE getWireIoType()const{return _wireIoType;}

    void createLogicGen() override;

    ////// this ensure that the wire is purely assign as it be
    /// no condition no state trigger only assign value
    bool verifyWireLocalization();

};


}

#endif //WIREIOUSER_H
