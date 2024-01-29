#include <iostream>

#include "test/simTest/simTest4.h"


using namespace kathryn;


int main() {

     auto t = test1();
     t.test();


    /**debugger*/
    initMdDebugger();
    initMfDebugger();
    finalizeMdDebugger();
    finalizeMfDebugger();
    ////////////////////////////////////


}
