

///#include "test/sim_test/sim_test11.h"

#include "kathryn.h"
#include "front_end/cmd/cfe.h"

using namespace kathryn;


int main(int argc, char* argv[]) {

//     auto t = test1();
//     t.test();


    /**debugger*/
    // initMdDebugger();
    // initMfDebugger();

    if (argc < 2){
        std::cout << "there is no argument value" << std::endl;
    }
    auto params = read_param_kathryn(argv[1]);


    /***** model and simulation start here*/
    start(params);
    /*** finalize system*/

    // finalizeMdDebugger();
    // finalizeMfDebugger();
    ////////////////////////////////////


}
