

///#include "test/simTest/simTest11.h"

#include "kathryn.h"
#include "frontEnd/cmd/cfe.h"

using namespace kathryn;


int main(int argc, char* argv[]) {

//     auto t = test1();
//     t.test();


    /**debugger*/
    initMdDebugger();
    initMfDebugger();

    if (argc < 2){
        std::cout << "there is no argument value" << std::endl;
    }

    start(std::string(argv[1]));

    finalizeMdDebugger();
    finalizeMfDebugger();
    ////////////////////////////////////


}
