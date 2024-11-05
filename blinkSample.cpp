#include "kathryn.h"
using namespace kathryn;


////// MODEL
struct BlinkAB: public Module{
    //////// swap blink A and B
    mReg(a, 1); //// register name a 1 bit
    mReg(b, 1); //// register name b 1 bit

    BlinkAB(int x): Module(){}

    void flow() override{
        cwhile(true){ ///// loop forever
            seq{
                par{a <<= 1; b <<= 0;} ///// a is on <-> b is off
                syWait(100); //// wait 100 cycle
                par{a <<= 0; b <<= 1;}
                syWait(100);
            }
        }
    }
};

/////// SIMULATOR
struct BlinkAB_sim: public SimInterface{

    explicit BlinkAB_sim(PARAM& params ):
        SimInterface(100, /// limit cycle
                     params["vcdFile"], /// des VCD file
                     params["profFile"]) /// des prof file
                     {}
};







void genVerilog(PARAM& params){
    /// (#name, #class, #parameter ...)
    mMod(ex, BlinkAB, 0); /// build module
    startModelKathryn(); /// start modeling
    startGenKathryn(params); /// start generate
    resetKathryn(); /// reset system

}

void simulate(PARAM& params){
    mMod(ex, BlinkAB, 0); ///  build module
    startModelKathryn(); /// start modeling
    BlinkAB_sim simulator(params); /// build simulator
    simulator.simStart();
    resetKathryn();
}



int main(int argc, char* argv[]){

    auto params = readParamKathryn(argv[1]);

    int mode;
    std::cout << "simulate press 0 <-> generate press 1" << std::endl;
    std::cin >> mode;
    if (mode == 1){
        genVerilog(params);
    }else if (mode == 0){
        simulate(params);
    }


}