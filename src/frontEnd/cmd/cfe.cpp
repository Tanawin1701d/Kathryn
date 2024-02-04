//
// Created by tanawin on 4/2/2567.
//

#include <cassert>
#include "cfe.h"


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
            std::cout << "test error no specify argument";
            assert(false);
        }

        if (args[idx] == "all"){

        }else{
            try{

            }catch (std::exception& e){

            }
        }


    }



}