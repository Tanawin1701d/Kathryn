

///#include "test/simTest/simTest11.h"

#include "kathryn.h"
#include "frontEnd/cmd/cfe.h"

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
    auto params = readParamKathryn(argv[1]);


    /***** model and simulation start here*/
    start(params);
    /*** finalize system*/

    // finalizeMdDebugger();
    // finalizeMfDebugger();
    ////////////////////////////////////


}
