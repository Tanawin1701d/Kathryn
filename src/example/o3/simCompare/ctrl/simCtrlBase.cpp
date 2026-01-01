//
// Created by tanawin on 25/12/25.
//

#include "simCtrlBase.h"


namespace kathryn::o3{

    O3SimCtrlBase::O3SimCtrlBase(CYCLE                    limitCycle,
                                 const std::string&       prefix,
                                 std::vector<std::string> testTypes,
                                 SimProxyBuildMode        buildMode,
                                 SlotWriterBase&          slotWriter,
                                 SimState&                state):


    _prefixFolder(prefix),
    _testTypes   (testTypes),
    _slotWriter  (slotWriter),
    _state       (state){}



    void O3SimCtrlBase::resetDmem(){
        std::memset(_dmem, 0, sizeof(_dmem));
        lastDmemEnable  = false;
        lastDmemRead    = true;
        lastDmemAddr    = 0;
        lastDmemData    = 0;
    }


    void O3SimCtrlBase::readAssembly(const std::string& filePath){
        ////////// initialize file
        std::ifstream asmFile(filePath, std::ios::binary);
        if (!asmFile.is_open()){assert(false);}
        asmFile.seekg(0, std::ios::end);
        std::streampos fileSize = asmFile.tellg();
        assert((fileSize % 4) == 0);
        asmFile.seekg(0, std::ios::beg);

        /** read instruction from file and write it to memory block*/
        uint32_t writeAddr = 0;
        uint32_t instr;
        while(asmFile.read(reinterpret_cast<char*>(&instr), sizeof instr)){
            assert((instr & 0b11) == 0b11); ////// check instruction
            _imem[writeAddr] = instr;
            writeAddr++;
        }
        asmFile.close();

        ///// fill all with zero
        std::fill(_imem + writeAddr, _imem + IMEM_ROW, 0);

        std::cout << TC_GREEN << "initialize mem finish" << TC_DEF << std::endl;
    }

    void O3SimCtrlBase::readAssertVal(const std::string& filePath){

        std::vector<std::string> rawVals;
        FileReaderBase reader(filePath);
        rawVals = reader.readLines();

        assert(rawVals.size() == REG_NUM);

        for (int regIdx = 0; regIdx < REG_NUM; regIdx++){
            _regTestVal[regIdx] = stoul(rawVals[regIdx]);
        }

    }

}