

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

    std::vector<std::string> cmd; ///// = {"test", "all"};

    if (argc < 2){
        std::cout << "there is no argument value" << std::endl;
    }

    for (int i = 1; i < argc; i++){
        std::string myarg = std::string(argv[i]);
        cmd.push_back(myarg);
    }


    start(cmd);

    finalizeMdDebugger();
    finalizeMfDebugger();
    ////////////////////////////////////


}
