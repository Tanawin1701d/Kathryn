//
// Created by tanawin on 4/2/2567.
//

#include <cassert>
#include "cfe.h"
#include "test/autoSim/simMng.h"


namespace kathryn{


    void printWelcomeScreen(){

        std::cout << "------------------------------------------------------------------------\n"
                     "-------- \n"
                     "-------- KATHRYN hardware construction framework with hybrid design flow\n"
                     "-------- \n"
                 <<  "-------- program compiled on " << __DATE__ << " at " << __TIME__ << "\n"
                     "-------- \n"
                     "------------------------------------------------------------------------\n";

    }

    void test(int idx, std::vector<std::string>& args){

        if (idx >= args.size()){
            std::cout << "test error no specify argument"  << std::endl;
            assert(false);
        }

        if (args[idx] == "all"){
            startAutoSimTest();
        }else{
            std::cout << TC_RED << "FOR now we do not support specificTestCase" << TC_DEF << std::endl;
        }

    }

    void start(std::vector<std::string>& args){

        printWelcomeScreen();

        if (args.size() == 0){
            std::cout << "[kathryn] there is no command to run" << std::endl;
        }else if(args[0] == "test"){
            test(1, args);
        }

        std::cout << "[kathryn] exit program" << std::endl;
    }



}