#include <iostream>

#include "test/simTest1.h"

#include "util/logger/logger.h"


using namespace kathryn;


int main() {

//    auto t = test3();
//    t.test();
     auto t = test1();
     t.test();


    /**debugger*/
    initMdDebugger();
    initMfDebugger();
    finalizeMdDebugger();
    finalizeMfDebugger();
    ////////////////////////////////////


}
