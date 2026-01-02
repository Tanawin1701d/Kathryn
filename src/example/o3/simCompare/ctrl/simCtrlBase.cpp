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
        lastDmemWData    = 0;
    }

    bool O3SimCtrlBase::compareMemOp(O3SimCtrlBase& rhs){

        if (!lastDmemEnable){return true;}
        bool result = true;

        if (lastDmemRead != rhs.lastDmemRead){
            std::cout << TC_RED
                      << "Dmem read misMatch Kride got: "
                      << lastDmemRead
                      << "    Ride got: "
                      << rhs.lastDmemRead
                      << std::endl;
            result = false;
        }
        if (lastDmemAddr != rhs.lastDmemAddr){
            std::cout << TC_RED
                      << "Dmem addr misMatch Kride got: "
                      << cvtNum2HexStr(lastDmemAddr)
                      << "    Ride got: "
                      << cvtNum2HexStr(rhs.lastDmemAddr)
                      << std::endl;
            result = false;
        }

        if (!lastDmemRead){ ///// it is write
            if (lastDmemWData != rhs.lastDmemWData){
                std::cout << TC_RED
                          << "Dmem write Data misMatch Kride got: "
                          << cvtNum2HexStr(lastDmemWData)
                          << "    Ride got: "
                          << cvtNum2HexStr(rhs.lastDmemWData)
                          << std::endl;
            }
        }
        return result;

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