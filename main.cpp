

///#include "test/simTest/simTest11.h"

#include "kathryn.h"
#include "frontEnd/cmd/cfe.h"

using namespace kathryn;


int main() {

//     auto t = test1();
//     t.test();


    /**debugger*/
    initMdDebugger();
    initMfDebugger();



    std::vector<std::string> cmd = {"test", "all"};
    start(cmd);


    finalizeMdDebugger();
    finalizeMfDebugger();
    ////////////////////////////////////


}
