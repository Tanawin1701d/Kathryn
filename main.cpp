#include <iostream>

#include "test/modelTest3.h"
#include "util/logger/logger.h"


using namespace kathryn;


int main() {

    auto t = test3();
    t.test();



    /**debugger*/
    initMdDebugger();
    initMfDebugger();
    finalizeMdDebugger();
    finalizeMfDebugger();
    ////////////////////////////////////


}
