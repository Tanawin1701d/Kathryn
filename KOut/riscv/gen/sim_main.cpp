#include<iostream>
#include <fstream>
#include <verilated.h>
#include "Vtop.h"
#include <chrono>


constexpr int AMT_REG = 32;
typedef unsigned long long ull;



inline void doThisClock(Vtop* core){
    assert(core != nullptr);
    core->clk = 1;
    core->eval();
    core->clk = 0;
    core->eval();


}

void resetRegFile(Vtop* core){
    assert(core != nullptr);
    for (int i = 0; i < AMT_REG; i++){
        core->top__DOT__MODULE7_riscCore__DOT__MEM_BLOCK25_regFile[i] = 0;
    }
}


void readFileToMem(Vtop* core, int testCaseIdx){

    assert(core != nullptr);
    std::ifstream asmFile("../../sorter/asm"+  std::to_string(testCaseIdx) + ".out", std::ios::binary);
    /** check file integrity */
    if (!asmFile.is_open()){assert(false);}
    asmFile.seekg(0, std::ios::end);
    std::streampos fileSize = asmFile.tellg();
    assert((fileSize % 4) == 0);
    asmFile.seekg(0, std::ios::beg);

    uint32_t writeAddr = 0;
    uint32_t instr;

    std::cout << "start writing file" << std::endl;
    while(asmFile.read(reinterpret_cast<char*>(&instr), sizeof instr)){
            assert((instr & 0b11) == 0b11); ////// check instruction
            core->top__DOT__MODULE7_riscCore__DOT__MEM_BLOCK26___05FmyMem_SYS[writeAddr]
            = instr;
            writeAddr++;
    }

    asmFile.close();

    int amountPopper =  1 << (10 + testCaseIdx);

        ///////////////////////////////////////// populate data
    for (int i = 0; i < amountPopper; i++){
        core->top__DOT__MODULE7_riscCore__DOT__MEM_BLOCK26___05FmyMem_SYS
        [((1 << 20) + i*4)/4] = (amountPopper-i);
    }
}


int main(int argc, char** argv ){


    Vtop* core = new Vtop();

    
    int AMT_TEST_CASE = 9;

    for (int testCaseIdx = 0; testCaseIdx < AMT_TEST_CASE; testCaseIdx++){

        std::cout << "initialize mem and register" << std::endl;
        resetRegFile (core);
        readFileToMem(core, testCaseIdx);
        std::cout << "finish initialized" << std::endl;

        core->rst = 1;
        core->clk = 1;
        core->eval();
        core->rst = 1;
        core->clk = 0;
        core->eval();
        core->rst = 1;
        core->clk = 1;
        core->eval();


        core->rst = 0;
        ull curCycleUsed = 0;


        std::cout << "start simulation at testcaseId" << std::to_string(testCaseIdx) << std::endl;
        auto start = std::chrono::high_resolution_clock::now();

        while(true){
            if((core->writeIdx  == 31) && (core->valid  ==  1)){
                break;
            }
            doThisClock(core);
            ///std::cout << "pc is " << core->top__DOT__MODULE7_riscCore__DOT__REG8_pc << std::endl;
            curCycleUsed++;
        }


        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = end - start;
        std::cout << "Elapsed time: " << elapsed.count() << " seconds" << std::endl;
        std::cout << "cycle used is "<< curCycleUsed << std::endl;
        std::cout << "core write Idx "<< core->writeIdx << " have value: " << core->writeData  << std::endl;






    }


    return 0;
}